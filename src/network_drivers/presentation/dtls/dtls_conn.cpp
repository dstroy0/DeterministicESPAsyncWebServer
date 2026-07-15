// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dtls_conn.cpp
 * @brief DTLS 1.3 server handshake state machine (RFC 9147 §5-6). See dtls_conn.h.
 */

#include "network_drivers/presentation/dtls/dtls_conn.h"

#if DETWS_ENABLE_DTLS

#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include <string.h>

namespace
{
// TLS alert codes used here (RFC 8446 §6).
const uint8_t ALERT_UNEXPECTED_MESSAGE = 10;
const uint8_t ALERT_HANDSHAKE_FAILURE = 40;
const uint8_t ALERT_DECODE_ERROR = 50;
const uint8_t ALERT_DECRYPT_ERROR = 51;
const uint8_t ALERT_PROTOCOL_VERSION = 70;
const uint8_t ALERT_INTERNAL_ERROR = 80;

// The record-layer demux (RFC 9147 §4): a first byte 0b001xxxxx is a DTLSCiphertext unified header.
bool is_ciphertext(uint8_t b0)
{
    return (b0 & 0xE0) == 0x20;
}

// On-wire length of a DTLSCiphertext record from its (plaintext) header, so a datagram carrying more
// than one record can be walked. Mirrors the header flags dtls_ciphertext_protect writes.
size_t ciphertext_record_len(const uint8_t *rec, size_t avail)
{
    if (avail < 1)
        return 0;
    uint8_t b0 = rec[0];
    size_t seq_len = (b0 & 0x08) ? 2 : 1; // S bit: 16- vs 8-bit sequence number
    size_t off = 1 + seq_len;
    if (b0 & 0x04) // L bit: explicit length present
    {
        if (off + 2 > avail)
            return 0;
        size_t enc = ((size_t)rec[off] << 8) | rec[off + 1];
        off += 2;
        if (off + enc > avail)
            return 0;
        return off + enc;
    }
    return avail; // no length -> record runs to the end of the datagram
}

int fail(DtlsConn *c, uint8_t alert)
{
    c->state = DtlsConnState::FAILED;
    c->alert = alert;
    return -1;
}

// Finalize a copy of the running transcript without disturbing it (RFC 8446 intermediate hashes).
void snapshot(const SshSha256Ctx *ctx, uint8_t out[SSH_SHA256_DIGEST_LEN])
{
    SshSha256Ctx copy = *ctx;
    ssh_sha256_final(&copy, out);
}

// Wrap one TLS handshake message (@p tls_msg, 4-byte TLS header + body) in a DTLS handshake header
// (message @p msg_seq) and a record for @p epoch, appending it to @p out. Epoch 0 is a DTLSPlaintext
// record; epoch 2 is AEAD-protected with @p keys. One message per record (this phase).
bool emit_handshake(DtlsConn *c, const uint8_t *tls_msg, size_t tls_len, uint16_t msg_seq, uint16_t epoch,
                    const DtlsRecordKeys *keys, uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (tls_len < 4)
        return false;
    uint8_t msg_type = tls_msg[0];
    uint32_t body_len = (uint32_t)(tls_len - 4);
    size_t flen =
        dtls_hs_frag_build(msg_type, msg_seq, body_len, 0, tls_msg + 4, body_len, c->fragbuf, sizeof(c->fragbuf));
    if (!flen)
        return false;
    size_t rn;
    if (epoch == 0)
        rn = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, c->tx_seq_ep0++, c->fragbuf, flen, out + *out_len,
                                  out_cap - *out_len);
    else
        rn = dtls_ciphertext_protect(keys, c->tx_seq_ep2++, DTLS_CT_HANDSHAKE, c->fragbuf, flen, out + *out_len,
                                     out_cap - *out_len);
    if (!rn)
        return false;
    *out_len += rn;
    return true;
}

// Consume a ClientHello and emit the whole server flight (ServerHello + the epoch-2 encrypted
// messages), installing handshake and application keys. Mirrors quic_tls process_client_hello.
int handle_client_hello(DtlsConn *c, const uint8_t *msg, size_t msg_len, uint8_t *out, size_t out_cap, size_t *out_len)
{
    Tls13ClientHello ch;
    if (!tls13_parse_client_hello(msg, msg_len, &ch))
        return fail(c, ALERT_DECODE_ERROR);
    if (!ch.offers_tls13)
        return fail(c, ALERT_PROTOCOL_VERSION);
    if (!ch.offers_ed25519 || !ch.has_key_share || !ch.offers_x25519)
        return fail(c, ALERT_HANDSHAKE_FAILURE);

    // X25519 shared secret and the server's key_share.
    uint8_t ecdhe[32];
    uint8_t server_share[32];
    ssh_x25519(ecdhe, c->cfg.ephemeral_priv, ch.client_x25519);
    ssh_x25519_base(server_share, c->cfg.ephemeral_priv);

    ssh_sha256_update(&c->transcript, msg, msg_len); // transcript: ClientHello

    // ServerHello (epoch 0, plaintext), message_seq 0.
    size_t n = tls13_build_server_hello(c->msgbuf, sizeof(c->msgbuf), c->cfg.server_random, ch.session_id,
                                        ch.session_id_len, server_share, 32, TLS_GROUP_X25519);
    if (!n)
        return fail(c, ALERT_INTERNAL_ERROR);
    ssh_sha256_update(&c->transcript, c->msgbuf, n);
    if (!emit_handshake(c, c->msgbuf, n, 0, 0, nullptr, out, out_cap, out_len))
        return fail(c, ALERT_INTERNAL_ERROR);

    // Handshake-traffic keys from Transcript-Hash(ClientHello..ServerHello).
    uint8_t hash[SSH_SHA256_DIGEST_LEN];
    snapshot(&c->transcript, hash);
    tls13_ks_early(&c->ks);
    tls13_ks_handshake(&c->ks, ecdhe, hash, 32);
    dtls_record_keys_derive(&c->ep2_srv, DtlsCipher::AES_128_GCM_SHA256, 2, c->ks.server_hs_traffic);
    dtls_record_keys_derive(&c->ep2_cli, DtlsCipher::AES_128_GCM_SHA256, 2, c->ks.client_hs_traffic);
    c->ep2_ready = true;

    // EncryptedExtensions, message_seq 1.
    n = tls13_build_encrypted_extensions_empty(c->msgbuf, sizeof(c->msgbuf));
    ssh_sha256_update(&c->transcript, c->msgbuf, n);
    if (!emit_handshake(c, c->msgbuf, n, 1, 2, &c->ep2_srv, out, out_cap, out_len))
        return fail(c, ALERT_INTERNAL_ERROR);

    // Certificate, message_seq 2.
    n = tls13_build_certificate(c->msgbuf, sizeof(c->msgbuf), c->cfg.cert_der, c->cfg.cert_len);
    if (!n)
        return fail(c, ALERT_INTERNAL_ERROR);
    ssh_sha256_update(&c->transcript, c->msgbuf, n);
    if (!emit_handshake(c, c->msgbuf, n, 2, 2, &c->ep2_srv, out, out_cap, out_len))
        return fail(c, ALERT_INTERNAL_ERROR);

    // CertificateVerify signs Transcript-Hash(ClientHello..Certificate), message_seq 3.
    snapshot(&c->transcript, hash);
    n = tls13_build_cert_verify(c->msgbuf, sizeof(c->msgbuf), hash, c->cfg.ed25519_seed);
    if (!n)
        return fail(c, ALERT_INTERNAL_ERROR);
    ssh_sha256_update(&c->transcript, c->msgbuf, n);
    if (!emit_handshake(c, c->msgbuf, n, 3, 2, &c->ep2_srv, out, out_cap, out_len))
        return fail(c, ALERT_INTERNAL_ERROR);

    // Server Finished over Transcript-Hash(ClientHello..CertificateVerify), message_seq 4.
    snapshot(&c->transcript, hash);
    uint8_t verify[SSH_SHA256_DIGEST_LEN];
    tls13_finished_mac(c->ks.server_hs_traffic, hash, verify);
    n = tls13_build_finished(c->msgbuf, sizeof(c->msgbuf), verify);
    ssh_sha256_update(&c->transcript, c->msgbuf, n);
    if (!emit_handshake(c, c->msgbuf, n, 4, 2, &c->ep2_srv, out, out_cap, out_len))
        return fail(c, ALERT_INTERNAL_ERROR);

    // Application-traffic keys from Transcript-Hash(ClientHello..server Finished); this hash also
    // verifies the client's Finished.
    snapshot(&c->transcript, c->hs_finished_hash);
    tls13_ks_master(&c->ks, c->hs_finished_hash);
    dtls_record_keys_derive(&c->ep3_srv, DtlsCipher::AES_128_GCM_SHA256, 3, c->ks.server_ap_traffic);
    dtls_record_keys_derive(&c->ep3_cli, DtlsCipher::AES_128_GCM_SHA256, 3, c->ks.client_ap_traffic);
    c->ep3_ready = true;

    c->state = DtlsConnState::WAIT_FINISHED;
    c->next_recv_msg_seq = 1;
    dtls_hs_reasm_init(&c->reasm, 1, c->reasm_buf + 4, DTLS_CONN_REASM_CAP);
    return 0;
}

// Verify the client's Finished and complete the handshake.
int handle_client_finished(DtlsConn *c, const uint8_t *msg, size_t msg_len)
{
    if (msg[0] != TlsHs::TLS_HS_FINISHED || msg_len != 4 + SSH_SHA256_DIGEST_LEN)
        return fail(c, ALERT_DECODE_ERROR);
    uint8_t expected[SSH_SHA256_DIGEST_LEN];
    tls13_finished_mac(c->ks.client_hs_traffic, c->hs_finished_hash, expected);
    uint8_t diff = 0;
    for (int i = 0; i < SSH_SHA256_DIGEST_LEN; i++)
        diff |= (uint8_t)(expected[i] ^ msg[4 + i]);
    if (diff)
        return fail(c, ALERT_DECRYPT_ERROR);
    ssh_sha256_update(&c->transcript, msg, msg_len);
    c->state = DtlsConnState::DONE;
    return 0;
}

int dispatch_message(DtlsConn *c, const uint8_t *tls_msg, size_t tls_len, uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (c->state == DtlsConnState::START && tls_msg[0] == TlsHs::TLS_HS_CLIENT_HELLO)
        return handle_client_hello(c, tls_msg, tls_len, out, out_cap, out_len);
    if (c->state == DtlsConnState::WAIT_FINISHED && tls_msg[0] == TlsHs::TLS_HS_FINISHED)
        return handle_client_finished(c, tls_msg, tls_len);
    return fail(c, ALERT_UNEXPECTED_MESSAGE);
}

// Parse and reassemble the DTLS handshake fragments carried in one record's payload, dispatching each
// complete TLS message.
int drive_handshake(DtlsConn *c, const uint8_t *payload, size_t plen, uint8_t *out, size_t out_cap, size_t *out_len)
{
    size_t p = 0;
    while (p < plen)
    {
        DtlsHsHeader hh;
        size_t used = dtls_hs_header_parse(payload + p, plen - p, &hh);
        if (!used)
            break;
        p += used;
        int r = dtls_hs_reasm_add(&c->reasm, &hh); // ignores fragments for other message_seqs
        if (r < 0)
            return fail(c, ALERT_DECODE_ERROR);
        if (r == 1)
        {
            // Rebuild the TLS handshake structure (4-byte header + reassembled body) for the transcript.
            c->reasm_buf[0] = c->reasm.msg_type;
            c->reasm_buf[1] = (uint8_t)(c->reasm.length >> 16);
            c->reasm_buf[2] = (uint8_t)(c->reasm.length >> 8);
            c->reasm_buf[3] = (uint8_t)c->reasm.length;
            if (dispatch_message(c, c->reasm_buf, 4 + c->reasm.length, out, out_cap, out_len) < 0)
                return -1;
        }
    }
    return 0;
}
} // namespace

void dtls_conn_init(DtlsConn *c, const DtlsServerConfig *cfg)
{
    memset(c, 0, sizeof(*c));
    c->cfg = *cfg;
    c->state = DtlsConnState::START;
    ssh_sha256_init(&c->transcript);
    dtls_replay_init(&c->replay_ep2);
    c->next_recv_msg_seq = 0;
    dtls_hs_reasm_init(&c->reasm, 0, c->reasm_buf + 4, DTLS_CONN_REASM_CAP);
}

int dtls_conn_process(DtlsConn *c, const uint8_t *dgram, size_t len, uint8_t *out, size_t out_cap)
{
    if (c->state == DtlsConnState::FAILED)
        return -1;
    size_t out_len = 0;
    size_t off = 0;
    while (off < len)
    {
        uint8_t b0 = dgram[off];
        if (is_ciphertext(b0))
        {
            size_t rlen = ciphertext_record_len(dgram + off, len - off);
            if (!rlen)
                break; // malformed header; stop walking the datagram
            if (!c->ep2_ready)
                return fail(c, ALERT_UNEXPECTED_MESSAGE);
            uint8_t inner[DTLS_CONN_REASM_CAP + DTLS_TAG_LEN];
            DtlsCiphertext info;
            uint64_t next = c->replay_ep2.seeded ? c->replay_ep2.highest + 1 : 0;
            if (!dtls_ciphertext_unprotect(&c->ep2_cli, next, dgram + off, rlen, inner, sizeof(inner), &info))
                return fail(c, ALERT_DECRYPT_ERROR);
            off += rlen;
            if (!dtls_replay_check(&c->replay_ep2, info.seq))
                continue; // replay: drop, but keep processing the datagram
            dtls_replay_mark(&c->replay_ep2, info.seq);
            if (info.content_type == DTLS_CT_HANDSHAKE &&
                drive_handshake(c, inner, info.pt_len, out, out_cap, &out_len) < 0)
                return -1;
        }
        else
        {
            DtlsPlaintext pt;
            size_t rlen = dtls_plaintext_parse(dgram + off, len - off, &pt);
            if (!rlen)
                break;
            off += rlen;
            if (pt.content_type == DTLS_CT_HANDSHAKE &&
                drive_handshake(c, pt.fragment, pt.frag_len, out, out_cap, &out_len) < 0)
                return -1;
        }
    }
    return (int)out_len;
}

bool dtls_conn_established(const DtlsConn *c)
{
    return c->state == DtlsConnState::DONE && c->ep3_ready;
}

uint8_t dtls_conn_alert(const DtlsConn *c)
{
    return c->alert;
}

const DtlsRecordKeys *dtls_conn_app_write_keys(const DtlsConn *c)
{
    return c->ep3_ready ? &c->ep3_srv : nullptr;
}

const DtlsRecordKeys *dtls_conn_app_read_keys(const DtlsConn *c)
{
    return c->ep3_ready ? &c->ep3_cli : nullptr;
}

#endif // DETWS_ENABLE_DTLS
