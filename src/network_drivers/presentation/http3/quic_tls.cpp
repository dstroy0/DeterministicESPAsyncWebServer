// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_tls.cpp
 * @brief TLS 1.3 server handshake state machine for QUIC (see quic_tls.h).
 */

#include "network_drivers/presentation/http3/quic_tls.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include <string.h>

// TLS alert codes we may raise (RFC 8446 sec 6).
enum
{
    TLS_ALERT_UNEXPECTED_MESSAGE = 10,
    TLS_ALERT_HANDSHAKE_FAILURE = 40,
    TLS_ALERT_ILLEGAL_PARAMETER = 47,
    TLS_ALERT_DECODE_ERROR = 50,
    TLS_ALERT_DECRYPT_ERROR = 51,
    TLS_ALERT_PROTOCOL_VERSION = 70,
    TLS_ALERT_INTERNAL_ERROR = 80,
    TLS_ALERT_MISSING_EXTENSION = 109,
    TLS_ALERT_NO_APPLICATION_PROTOCOL = 120,
};

namespace
{
void fail(QuicTls *qt, uint8_t alert)
{
    qt->state = QTLS_FAILED;
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
    if (!written || written > cap - *plen)
    {
        fail(qt, TLS_ALERT_INTERNAL_ERROR);
        return false;
    }
    ssh_sha256_update(&qt->transcript, flight + *plen, written);
    *plen += written;
    return true;
}

bool process_client_hello(QuicTls *qt, const uint8_t *msg, size_t msg_len)
{
    Tls13ClientHello ch;
    if (!tls13_parse_client_hello(msg, msg_len, &ch))
    {
        fail(qt, TLS_ALERT_DECODE_ERROR);
        return false;
    }
    if (!ch.offers_tls13)
    {
        fail(qt, TLS_ALERT_PROTOCOL_VERSION);
        return false;
    }
    if (!ch.has_key_share || !ch.offers_x25519 || !ch.offers_ed25519)
    {
        fail(qt, TLS_ALERT_HANDSHAKE_FAILURE);
        return false;
    }
    if (!ch.offers_h3_alpn)
    {
        fail(qt, TLS_ALERT_NO_APPLICATION_PROTOCOL);
        return false;
    }
    if (!ch.quic_tp)
    {
        fail(qt, TLS_ALERT_MISSING_EXTENSION);
        return false;
    }
    if (!quic_tp_parse(ch.quic_tp, ch.quic_tp_len, &qt->peer))
    {
        fail(qt, TLS_ALERT_ILLEGAL_PARAMETER);
        return false;
    }
    qt->have_peer = true;

    // ECDHE shared secret and the server's public key share.
    uint8_t ecdhe[32], server_pub[32];
    ssh_x25519(ecdhe, qt->cfg.ephemeral_priv, ch.client_x25519);
    ssh_x25519_base(server_pub, qt->cfg.ephemeral_priv);

    // Transcript starts with the ClientHello.
    ssh_sha256_update(&qt->transcript, msg, msg_len);

    // ServerHello (Initial-level flight).
    size_t n = tls13_build_server_hello(qt->flight_initial, sizeof(qt->flight_initial), qt->cfg.random, ch.session_id,
                                        ch.session_id_len, server_pub);
    qt->flight_initial_len = 0;
    if (!emit(qt, qt->flight_initial, sizeof(qt->flight_initial), &qt->flight_initial_len, n))
        return false; // GCOVR_EXCL_LINE  ServerHello (<=~160B: 32B session_id cap + key share) always fits
                      // flight_initial[256]

    // Handshake keys from Transcript-Hash(ClientHello..ServerHello).
    uint8_t hash[32];
    snapshot_hash(&qt->transcript, hash);
    tls13_ks_early(&qt->ks);
    tls13_ks_handshake(&qt->ks, ecdhe, hash);
    quic_keys_from_secret(qt->ks.client_hs_traffic, &qt->hs_client);
    quic_keys_from_secret(qt->ks.server_hs_traffic, &qt->hs_server);
    qt->hs_keys_ready = true;

    // Handshake-level flight: EncryptedExtensions, Certificate, CertificateVerify, Finished.
    qt->flight_hs_len = 0;
    uint8_t tp_enc[512];
    size_t tp_len = quic_tp_encode(&qt->cfg.params, tp_enc, sizeof(tp_enc));

    n = tls13_build_encrypted_extensions(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len,
                                         tp_enc, tp_len);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false; // GCOVR_EXCL_LINE  EncryptedExtensions (tp <= tp_enc[512]) always fits flight_hs[2048]

    n = tls13_build_certificate(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len,
                                qt->cfg.cert_der, qt->cfg.cert_len);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // CertificateVerify signs Transcript-Hash(ClientHello..Certificate).
    snapshot_hash(&qt->transcript, hash);
    n = tls13_build_cert_verify(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len, hash,
                                qt->cfg.ed25519_seed);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // Server Finished over Transcript-Hash(ClientHello..CertificateVerify).
    snapshot_hash(&qt->transcript, hash);
    uint8_t verify[32];
    tls13_finished_mac(qt->ks.server_hs_traffic, hash, verify);
    n = tls13_build_finished(qt->flight_hs + qt->flight_hs_len, sizeof(qt->flight_hs) - qt->flight_hs_len, verify);
    if (!emit(qt, qt->flight_hs, sizeof(qt->flight_hs), &qt->flight_hs_len, n))
        return false;

    // 1-RTT keys from Transcript-Hash(ClientHello..server Finished); also the hash we verify the
    // client Finished against.
    snapshot_hash(&qt->transcript, qt->hs_finished_hash);
    tls13_ks_master(&qt->ks, qt->hs_finished_hash);
    quic_keys_from_secret(qt->ks.client_ap_traffic, &qt->ap_client);
    quic_keys_from_secret(qt->ks.server_ap_traffic, &qt->ap_server);
    qt->ap_keys_ready = true;

    qt->state = QTLS_WAIT_FINISHED;
    return true;
}

bool process_client_finished(QuicTls *qt, const uint8_t *msg, size_t msg_len)
{
    if (msg[0] != TLS_HS_FINISHED || msg_len != 4 + 32)
    {
        fail(qt, TLS_ALERT_DECODE_ERROR);
        return false;
    }
    uint8_t expected[32];
    tls13_finished_mac(qt->ks.client_hs_traffic, qt->hs_finished_hash, expected);
    uint8_t diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (uint8_t)(expected[i] ^ msg[4 + i]);
    if (diff)
    {
        fail(qt, TLS_ALERT_DECRYPT_ERROR);
        return false;
    }
    ssh_sha256_update(&qt->transcript, msg, msg_len);
    qt->complete = true;
    qt->state = QTLS_DONE;
    return true;
}

bool process_message(QuicTls *qt, int level, const uint8_t *msg, size_t msg_len)
{
    if (level == QUIC_ENC_INITIAL && qt->state == QTLS_START && msg[0] == TLS_HS_CLIENT_HELLO)
        return process_client_hello(qt, msg, msg_len);
    if (level == QUIC_ENC_HANDSHAKE && qt->state == QTLS_WAIT_FINISHED && msg[0] == TLS_HS_FINISHED)
        return process_client_finished(qt, msg, msg_len);
    fail(qt, TLS_ALERT_UNEXPECTED_MESSAGE);
    return false;
}
} // namespace

void quic_tls_server_init(QuicTls *qt, const QuicTlsConfig *cfg)
{
    memset(qt, 0, sizeof(*qt));
    qt->cfg = *cfg;
    ssh_sha256_init(&qt->transcript);
    qt->state = QTLS_START;
}

size_t quic_tls_recv_crypto(QuicTls *qt, int level, const uint8_t *data, size_t len)
{
    if (qt->state == QTLS_FAILED)
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
        if (qt->state == QTLS_DONE)
            break;
    }
    return off;
}

const uint8_t *quic_tls_flight(const QuicTls *qt, int level, size_t *len)
{
    if (level == QUIC_ENC_INITIAL)
    {
        *len = qt->flight_initial_len;
        return qt->flight_initial;
    }
    if (level == QUIC_ENC_HANDSHAKE)
    {
        *len = qt->flight_hs_len;
        return qt->flight_hs;
    }
    *len = 0;
    return nullptr;
}

const QuicPacketKeys *quic_tls_keys(const QuicTls *qt, int level, bool is_server)
{
    if (level == QUIC_ENC_HANDSHAKE && qt->hs_keys_ready)
        return is_server ? &qt->hs_server : &qt->hs_client;
    if (level == QUIC_ENC_APP && qt->ap_keys_ready)
        return is_server ? &qt->ap_server : &qt->ap_client;
    return nullptr;
}

const QuicTransportParams *quic_tls_peer_params(const QuicTls *qt)
{
    return qt->have_peer ? &qt->peer : nullptr;
}

#endif // DETWS_ENABLE_HTTP3
