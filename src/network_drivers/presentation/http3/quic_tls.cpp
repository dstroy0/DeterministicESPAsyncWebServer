// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dws_quic_tls.cpp
 * @brief TLS 1.3 server handshake state machine for QUIC (see dws_quic_tls.h).
 */

#include "network_drivers/presentation/http3/quic_tls.h"

#if DWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#if DWS_ENABLE_PQC_KEX
#include "network_drivers/presentation/pqc/mlkem.h" // dws_mlkem768_encaps (X25519MLKEM768 hybrid)
#endif
#include <string.h>

// TLS alert codes we may raise (RFC 8446 sec 6).
struct TlsAlert
{
    static constexpr uint8_t TLS_ALERT_UNEXPECTED_MESSAGE = 10;
    static constexpr uint8_t TLS_ALERT_HANDSHAKE_FAILURE = 40;
    static constexpr uint8_t TLS_ALERT_ILLEGAL_PARAMETER = 47;
    static constexpr uint8_t TLS_ALERT_DECODE_ERROR = 50;
    static constexpr uint8_t TLS_ALERT_DECRYPT_ERROR = 51;
    static constexpr uint8_t TLS_ALERT_PROTOCOL_VERSION = 70;
    static constexpr uint8_t TLS_ALERT_INTERNAL_ERROR = 80;
    static constexpr uint8_t TLS_ALERT_MISSING_EXTENSION = 109;
    static constexpr uint8_t TLS_ALERT_NO_APPLICATION_PROTOCOL = 120;
};

namespace
{
/// Capacity of the encoded-transport-parameters scratch the EncryptedExtensions builder is fed.
constexpr size_t QUIC_TLS_TP_ENC_CAP = 512;

/// Largest EncryptedExtensions this module can emit: handshake header (4) + extensions length (2)
/// + the ALPN "h3" extension (9) + the transport-parameters extension header (4) + the parameters.
constexpr size_t QUIC_TLS_EE_MAX = 4 + 2 + 9 + 4 + QUIC_TLS_TP_ENC_CAP;

// EncryptedExtensions is the FIRST message written into flight_hs (flight_hs_len is reset to 0
// immediately before it), so its emit() cannot overflow - which is why that failure path carries a
// coverage exclusion. That only holds while flight_hs is at least one whole EncryptedExtensions,
// and DWS_H3_CRYPTO_BUF is an overridable macro (ServerConfig.h), so pin the relationship here:
// a build that shrank it would silently make the excluded path reachable.
static_assert(DWS_H3_CRYPTO_BUF >= QUIC_TLS_EE_MAX,
              "DWS_H3_CRYPTO_BUF (QuicTls::flight_hs) must hold a whole EncryptedExtensions: the fixed "
              "512-byte transport-parameter buffer plus the ALPN and extension framing");

void fail(QuicTls *qt, uint8_t alert)
{
    qt->state = QtlsState::QTLS_FAILED;
    qt->alert = alert;
}

// Snapshot the running Transcript-Hash without disturbing it (SshSha256Ctx is copyable state).
void snapshot_hash(const SshSha256Ctx *ctx, uint8_t out[32])
{
    SshSha256Ctx tmp = *ctx;
    ssh_sha256_final(&tmp, out);
}

// Append a handshake message to both the outbound flight buffer and the transcript.
bool emit(QuicTls *qt, uint8_t *flight, size_t cap, size_t *plen, size_t written)
{
    // *plen <= cap is the invariant this append maintains, so cap - *plen never underflows; refuse a
    // write that would run past the flight buffer (each builder already caps to what it was told, so
    // this only fires if that contract is ever broken - it keeps flight+*plen in bounds regardless).
    if (!written || written > cap - *plen) // GCOVR_EXCL_LINE  every caller passes the builder exactly cap - *plen
    {                                      // as its own capacity, so a non-zero return can never exceed it
        fail(qt, TlsAlert::TLS_ALERT_INTERNAL_ERROR);
        return false;
    }
    ssh_sha256_update(&qt->transcript, flight + *plen, written);
    *plen += written;
    return true;
}

#if DWS_ENABLE_PQC_KEX
// Emit a HelloRetryRequest (RFC 8446 §4.1.4) asking the client to retry with an X25519MLKEM768
// key_share, and restart the transcript per §4.4.1: message_hash(Hash(ClientHello1)) || HRR, so the
// eventual transcript is message_hash || HRR || ClientHello2 || ServerHello || ... QUIC does its own
// return-routability (Retry tokens), so the HRR carries no cookie. @p msg is ClientHello1.
bool send_hello_retry(QuicTls *qt, const uint8_t *msg, size_t msg_len, const Tls13ClientHello *ch)
{
    uint8_t ch1_hash[32];
    {
        SshSha256Ctx t;
        ssh_sha256_init(&t);
        ssh_sha256_update(&t, msg, msg_len);
        ssh_sha256_final(&t, ch1_hash);
    }
    ssh_sha256_init(&qt->transcript);
    uint8_t mh[40];
    size_t mhn = dws_tls13_build_message_hash(mh, sizeof(mh), ch1_hash);
    if (!mhn) // GCOVR_EXCL_LINE  mh[40] always fits the 36-byte hash
    {
        fail(qt, TlsAlert::TLS_ALERT_INTERNAL_ERROR); // GCOVR_EXCL_LINE
        return false;                                 // GCOVR_EXCL_LINE
    }
    ssh_sha256_update(&qt->transcript, mh, mhn); // message_hash is transcript-only, never sent

    qt->flight_initial_len = 0;
    size_t n = dws_tls13_build_hello_retry_request(qt->flight_initial, sizeof(qt->flight_initial), ch->session_id,
                                                   ch->session_id_len, TLS_GROUP_X25519MLKEM768, nullptr, 0,
                                                   /*dtls=*/false);
    if (!emit(qt, qt->flight_initial, sizeof(qt->flight_initial), &qt->flight_initial_len, n)) // GCOVR_EXCL_LINE
        return false; // GCOVR_EXCL_LINE  the HRR (~50B) always fits flight_initial (>=256B)
    qt->hrr_sent = true;
    return true; // stay in QTLS_START, awaiting ClientHello2 at the Initial level
}
#endif

bool process_client_hello(QuicTls *qt, const uint8_t *msg, size_t msg_len)
{
    Tls13ClientHello ch;
    if (!dws_tls13_parse_client_hello(msg, msg_len, &ch))
    {
        fail(qt, TlsAlert::TLS_ALERT_DECODE_ERROR);
        return false;
    }
    if (!ch.offers_tls13)
    {
        fail(qt, TlsAlert::TLS_ALERT_PROTOCOL_VERSION);
        return false;
    }
    bool use_hybrid = false;
#if DWS_ENABLE_PQC_KEX
    // Prefer the PQ/T hybrid whenever the client sent a usable X25519MLKEM768 key_share.
    use_hybrid = ch.has_hybrid_share && ch.offers_x25519mlkem768;
    // The client offered X25519MLKEM768 but sent only a classical key_share: ask it (once) to retry with
    // the hybrid share rather than silently downgrading to X25519 (RFC 8446 §4.1.4).
    // GCOVR_EXCL_LINE below: !ch.has_hybrid_share is implied by the two operands before it - use_hybrid is
    // has_hybrid_share && offers_x25519mlkem768, so reaching it with the share present is impossible.
    if (!use_hybrid && ch.offers_x25519mlkem768 && !ch.has_hybrid_share && !qt->hrr_sent) // GCOVR_EXCL_LINE
        return send_hello_retry(qt, msg, msg_len, &ch);
    // A retry that still lacks the hybrid share is fatal - one HRR only, so a client cannot loop us.
    if (qt->hrr_sent && !use_hybrid)
    {
        fail(qt, TlsAlert::TLS_ALERT_HANDSHAKE_FAILURE);
        return false;
    }
#endif
    if (!ch.offers_ed25519 || (!use_hybrid && (!ch.has_key_share || !ch.offers_x25519)))
    {
        fail(qt, TlsAlert::TLS_ALERT_HANDSHAKE_FAILURE);
        return false;
    }
    if (!ch.offers_h3_alpn)
    {
        fail(qt, TlsAlert::TLS_ALERT_NO_APPLICATION_PROTOCOL);
        return false;
    }
    if (!ch.dws_quic_tp)
    {
        fail(qt, TlsAlert::TLS_ALERT_MISSING_EXTENSION);
        return false;
    }
    if (!dws_quic_tp_parse(ch.dws_quic_tp, ch.dws_quic_tp_len, &qt->peer))
    {
        fail(qt, TlsAlert::TLS_ALERT_ILLEGAL_PARAMETER);
        return false;
    }
    qt->have_peer = true;

    // (EC)DHE shared secret + the server's key_share, per negotiated group. The hybrid secret is the
    // 64-byte ML-KEM_secret || X25519_secret (ML-KEM first, per draft-ietf-tls-ecdhe-mlkem).
    uint8_t ecdhe[64];
    size_t ecdhe_len;
    uint16_t group;
    size_t share_len;
#if DWS_ENABLE_PQC_KEX
    uint8_t server_share[MLKEM768_CT_BYTES + 32]; // S_CT2(1088) || Q_S(32) for the hybrid
    if (use_hybrid)
    {
        uint8_t ml_ss[32];
        if (!dws_mlkem768_encaps(ch.client_mlkem_ek, qt->cfg.mlkem_m, server_share, ml_ss))
        {
            fail(qt, TlsAlert::TLS_ALERT_HANDSHAKE_FAILURE); // malformed ML-KEM key
            return false;
        }
        uint8_t x_ss[32];
        uint8_t server_pub[32];
        ssh_x25519(x_ss, qt->cfg.ephemeral_priv, ch.client_x25519);
        ssh_x25519_base(server_pub, qt->cfg.ephemeral_priv);
        memcpy(server_share + MLKEM768_CT_BYTES, server_pub, 32);
        memcpy(ecdhe, ml_ss, 32);
        memcpy(ecdhe + 32, x_ss, 32);
        ecdhe_len = 64;
        share_len = MLKEM768_CT_BYTES + 32;
        group = TLS_GROUP_X25519MLKEM768;
    }
    else
#else
    uint8_t server_share[32];
#endif
    {
        ssh_x25519(ecdhe, qt->cfg.ephemeral_priv, ch.client_x25519);
        ssh_x25519_base(server_share, qt->cfg.ephemeral_priv);
        ecdhe_len = 32;
        share_len = 32;
        group = TLS_GROUP_X25519;
    }

    // Fold the ClientHello into the transcript. On the happy path it is the first message; after a
    // HelloRetryRequest the transcript already holds message_hash || HRR, so this is ClientHello2.
    ssh_sha256_update(&qt->transcript, msg, msg_len);

    // ServerHello (Initial-level flight). The Initial CRYPTO is one contiguous byte stream, so after a
    // HelloRetryRequest the ServerHello is appended after the HRR already in flight_initial - build at the
    // current offset (0 on the happy path, the HRR's end on a retry) and do not reset the length.
    size_t n = dws_tls13_build_server_hello(qt->flight_initial + qt->flight_initial_len,
                                            sizeof(qt->flight_initial) - qt->flight_initial_len, qt->cfg.random,
                                            ch.session_id, ch.session_id_len, server_share, share_len, group);
    if (!emit(qt, qt->flight_initial, sizeof(qt->flight_initial), &qt->flight_initial_len, n)) // GCOVR_EXCL_LINE
        return false; // GCOVR_EXCL_LINE  ServerHello always fits flight_initial (classical <=~160B; the
                      // hybrid's ~1.2 KB share fits the PQC-sized 1400B buffer)

    // Handshake keys from Transcript-Hash(ClientHello..ServerHello).
    uint8_t hash[32];
    snapshot_hash(&qt->transcript, hash);
    dws_tls13_ks_early(&TLS13_KDF, &qt->ks);
    dws_tls13_ks_handshake(&qt->ks, ecdhe, hash, ecdhe_len);
    dws_quic_keys_from_secret(qt->ks.client_hs_traffic, &qt->hs_client);
    dws_quic_keys_from_secret(qt->ks.server_hs_traffic, &qt->hs_server);
    qt->hs_keys_ready = true;

    // Handshake-level flight: EncryptedExtensions, Certificate, CertificateVerify, Finished.
    qt->flight_hs_len = 0;
    uint8_t tp_enc[QUIC_TLS_TP_ENC_CAP];
    size_t tp_len = dws_quic_tp_encode(&qt->cfg.params, tp_enc, sizeof(tp_enc));

    n = dws_tls13_build_encrypted_extensions(qt->flight_hs + qt->flight_hs_len,
                                             sizeof(qt->flight_hs) - qt->flight_hs_len, tp_enc, tp_len);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n)) // GCOVR_EXCL_LINE
        return false; // GCOVR_EXCL_LINE  EncryptedExtensions is the first message written into flight_hs and
                      // DWS_H3_CRYPTO_BUF >= QUIC_TLS_EE_MAX is pinned by the static_assert above

    n = dws_tls13_build_certificate(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len,
                                    qt->cfg.cert_der, qt->cfg.cert_len);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // CertificateVerify signs Transcript-Hash(ClientHello..Certificate).
    snapshot_hash(&qt->transcript, hash);
    n = dws_tls13_build_cert_verify(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len, hash,
                                    qt->cfg.ed25519_seed);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // Server Finished over Transcript-Hash(ClientHello..CertificateVerify).
    snapshot_hash(&qt->transcript, hash);
    uint8_t verify[32];
    dws_tls13_finished_mac(&TLS13_KDF, qt->ks.server_hs_traffic, hash, verify);
    n = dws_tls13_build_finished(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len, verify);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // 1-RTT keys from Transcript-Hash(ClientHello..server Finished); also the hash we verify the
    // client Finished against.
    snapshot_hash(&qt->transcript, qt->hs_finished_hash);
    dws_tls13_ks_master(&qt->ks, qt->hs_finished_hash);
    dws_quic_keys_from_secret(qt->ks.client_ap_traffic, &qt->ap_client);
    dws_quic_keys_from_secret(qt->ks.server_ap_traffic, &qt->ap_server);
    qt->ap_keys_ready = true;

    qt->state = QtlsState::QTLS_WAIT_FINISHED;
    return true;
}

bool process_client_finished(QuicTls *qt, const uint8_t *msg, size_t msg_len)
{
    if (msg[0] != TlsHs::TLS_HS_FINISHED || msg_len != 4 + 32) // GCOVR_EXCL_LINE  process_message only routes a
    {                                                          // Finished here, so the type arm cannot be taken
        fail(qt, TlsAlert::TLS_ALERT_DECODE_ERROR);
        return false;
    }
    uint8_t expected[32];
    dws_tls13_finished_mac(&TLS13_KDF, qt->ks.client_hs_traffic, qt->hs_finished_hash, expected);
    uint8_t diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (uint8_t)(expected[i] ^ msg[4 + i]);
    if (diff)
    {
        fail(qt, TlsAlert::TLS_ALERT_DECRYPT_ERROR);
        return false;
    }
    ssh_sha256_update(&qt->transcript, msg, msg_len);
    qt->complete = true;
    qt->state = QtlsState::QTLS_DONE;
    return true;
}

bool process_message(QuicTls *qt, int level, const uint8_t *msg, size_t msg_len)
{
    if (level == QuicEnc::QUIC_ENC_INITIAL && qt->state == QtlsState::QTLS_START &&
        msg[0] == TlsHs::TLS_HS_CLIENT_HELLO)
        return process_client_hello(qt, msg, msg_len);
    if (level == QuicEnc::QUIC_ENC_HANDSHAKE && qt->state == QtlsState::QTLS_WAIT_FINISHED &&
        msg[0] == TlsHs::TLS_HS_FINISHED)
        return process_client_finished(qt, msg, msg_len);
    fail(qt, TlsAlert::TLS_ALERT_UNEXPECTED_MESSAGE);
    return false;
}
} // namespace

void dws_quic_tls_server_init(QuicTls *qt, const QuicTlsConfig *cfg)
{
    memset(qt, 0, sizeof(*qt));
    qt->cfg = *cfg;
    ssh_sha256_init(&qt->transcript);
    qt->state = QtlsState::QTLS_START;
}

size_t dws_quic_tls_recv_crypto(QuicTls *qt, int level, const uint8_t *data, size_t len)
{
    if (qt->state == QtlsState::QTLS_FAILED)
        return len; // drain; the connection is closing
    size_t off = 0;
    while (off + 4 <= len)
    {
        uint32_t mlen = (uint32_t)((data[off + 1] << 16) | (data[off + 2] << 8) | data[off + 3]);
        size_t total = 4 + mlen;
        if (off + total > len)
            break; // an incomplete trailing message; wait for more bytes
        if (!process_message(qt, level, data + off, total))
            return off + total; // consumed through the offending message; state is FAILED/handled
        off += total;
        if (qt->state == QtlsState::QTLS_DONE)
            break;
    }
    return off;
}

const uint8_t *dws_quic_tls_flight(const QuicTls *qt, int level, size_t *len)
{
    if (level == QuicEnc::QUIC_ENC_INITIAL)
    {
        *len = qt->flight_initial_len;
        return qt->flight_initial;
    }
    if (level == QuicEnc::QUIC_ENC_HANDSHAKE)
    {
        *len = qt->flight_hs_len;
        return qt->flight_hs;
    }
    *len = 0;
    return nullptr;
}

const QuicPacketKeys *dws_quic_tls_keys(const QuicTls *qt, int level, bool is_server)
{
    if (level == QuicEnc::QUIC_ENC_HANDSHAKE && qt->hs_keys_ready)
        return is_server ? &qt->hs_server : &qt->hs_client;
    if (level == QuicEnc::QUIC_ENC_APP && qt->ap_keys_ready)
        return is_server ? &qt->ap_server : &qt->ap_client;
    return nullptr;
}

const QuicTransportParams *dws_quic_tls_peer_params(const QuicTls *qt)
{
    return qt->have_peer ? &qt->peer : nullptr;
}

#endif // DWS_ENABLE_HTTP3
