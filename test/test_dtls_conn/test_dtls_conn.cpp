// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DTLS 1.3 server handshake (RFC 9147 §5-6). A self-consistent proof: the test plays a minimal DTLS
// 1.3 client against dtls_conn - it builds a ClientHello, drives the server, then runs the SAME TLS
// 1.3 key schedule to deprotect the server flight, VERIFIES the server's CertificateVerify signature
// and Finished MAC over the real transcript, sends its own Finished, and confirms both sides install
// identical application-traffic keys. If any transcript byte, epoch, nonce, or key were wrong the
// AEAD open, the signature check, or the key comparison would fail. A second case drives the
// HelloRetryRequest path (RFC 9147 §5.1): a ClientHello with no X25519 key_share triggers an HRR with
// a cookie, and the retry that echoes it completes the same full handshake.

#include "network_drivers/presentation/dtls/dtls_conn.h"
#include "network_drivers/presentation/dtls/dtls_handshake.h"
#include "network_drivers/presentation/dtls/dtls_record.h"
#include "network_drivers/presentation/http3/tls13_kdf.h"
#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include "services/clock.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

// Controllable clock so the retransmission-timer tests can advance time deterministically.
static uint32_t g_ms = 0;
static uint32_t test_clock()
{
    return g_ms;
}

void setUp()
{
    g_ms = 0;
    dws_set_clock(test_clock, 1000); // 1000 ticks/s -> dws_millis() == g_ms
}
void tearDown()
{
}

// ---- fixed test key material (deterministic) ----
static const uint8_t SERVER_ED_SEED[32] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                                           17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
static const uint8_t SERVER_X25519_PRIV[32] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                                               0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                                               0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
static const uint8_t SERVER_RANDOM[32] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
                                          0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,
                                          0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF};
static const uint8_t CLIENT_X25519_PRIV[32] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
                                               0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
                                               0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
static const uint8_t SERVER_COOKIE_KEY[32] = {0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
                                              0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71,
                                              0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b};
static const uint8_t TEST_PEER_ADDR[6] = {192, 168, 1, 50, 0xC3, 0x50}; // IPv4 192.168.1.50 : port 50000

// ---- a tiny byte writer ----
struct Buf
{
    uint8_t *p;
    size_t n;
};
static void b8(Buf *b, uint8_t v)
{
    b->p[b->n++] = v;
}
static void b16(Buf *b, uint16_t v)
{
    b8(b, (uint8_t)(v >> 8));
    b8(b, (uint8_t)v);
}
static void bmem(Buf *b, const uint8_t *m, size_t k)
{
    memcpy(b->p + b->n, m, k);
    b->n += k;
}

// Build a ClientHello (TLS message, 4-byte header + body) offering TLS 1.3 / X25519 / Ed25519. When
// @p with_keyshare is true it carries an X25519 key_share for @p client_pub; otherwise it advertises
// X25519 in supported_groups but sends no share (the HelloRetryRequest trigger). A non-NULL @p cookie
// is echoed in a cookie extension (RFC 8446 §4.2.2), as a client does on its post-HRR retry.
static size_t build_client_hello_ex(uint8_t *out, const uint8_t client_pub[32], bool with_keyshare,
                                    const uint8_t *cookie, size_t cookie_len, const uint8_t *cid = nullptr,
                                    size_t cid_len = 0)
{
    Buf b = {out, 0};
    b8(&b, 0x01); // client_hello
    size_t len_at = b.n;
    b8(&b, 0);
    b8(&b, 0);
    b8(&b, 0);       // 24-bit body length (patched)
    b16(&b, 0x0303); // legacy_version
    for (int i = 0; i < 32; i++)
        b8(&b, (uint8_t)(0x30 + i)); // random
    b8(&b, 0x00);                    // session_id: empty
    b8(&b, 0x00);                    // legacy_cookie: empty (DTLS ClientHello, RFC 9147 §5.3)
    b16(&b, 0x0002);
    b16(&b, 0x1301); // cipher_suites: TLS_AES_128_GCM_SHA256
    b8(&b, 0x01);
    b8(&b, 0x00); // compression: null
    size_t ext_len_at = b.n;
    b16(&b, 0); // extensions length (patched)
    // supported_versions: DTLS 1.3 (0xFEFC), RFC 9147
    b16(&b, 0x002b);
    b16(&b, 0x0003);
    b8(&b, 0x02);
    b16(&b, 0xFEFC);
    // supported_groups: x25519
    b16(&b, 0x000a);
    b16(&b, 0x0004);
    b16(&b, 0x0002);
    b16(&b, 0x001d);
    // signature_algorithms: ed25519
    b16(&b, 0x000d);
    b16(&b, 0x0004);
    b16(&b, 0x0002);
    b16(&b, 0x0807);
    if (with_keyshare)
    {
        // key_share: x25519 entry
        b16(&b, 0x0033);
        b16(&b, 0x0026); // ext body: client_shares(2) + entry(2+2+32)
        b16(&b, 0x0024); // client_shares length
        b16(&b, 0x001d); // group x25519
        b16(&b, 0x0020); // key length 32
        bmem(&b, client_pub, 32);
    }
    if (cookie && cookie_len)
    {
        // cookie { opaque cookie<1..2^16-1> } (RFC 8446 §4.2.2)
        b16(&b, 0x002c);
        b16(&b, (uint16_t)(cookie_len + 2));
        b16(&b, (uint16_t)cookie_len);
        bmem(&b, cookie, cookie_len);
    }
    if (cid)
    {
        // connection_id { opaque cid<0..2^8-1> } (RFC 9146 §3): the CID the server must place in records
        // it sends to this client.
        b16(&b, 0x0036);
        b16(&b, (uint16_t)(1 + cid_len));
        b8(&b, (uint8_t)cid_len);
        bmem(&b, cid, cid_len);
    }
    uint16_t ext_len = (uint16_t)(b.n - ext_len_at - 2);
    out[ext_len_at] = (uint8_t)(ext_len >> 8);
    out[ext_len_at + 1] = (uint8_t)ext_len;
    uint32_t body = (uint32_t)(b.n - len_at - 3);
    out[len_at] = (uint8_t)(body >> 16);
    out[len_at + 1] = (uint8_t)(body >> 8);
    out[len_at + 2] = (uint8_t)body;
    return b.n;
}

static size_t build_client_hello(uint8_t *out, const uint8_t client_pub[32])
{
    return build_client_hello_ex(out, client_pub, /*with_keyshare=*/true, nullptr, 0);
}

// Pull the 32-byte X25519 key_share out of a ServerHello (walks its extensions for type 0x0033).
static bool sh_keyshare(const uint8_t *sh, size_t len, uint8_t pub[32])
{
    if (len < 44)
        return false;
    size_t o = 4 + 2 + 32; // hdr + legacy_version + random
    uint8_t sid = sh[o++];
    o += sid;   // session_id echo
    o += 2 + 1; // cipher_suite + compression
    if (o + 2 > len)
        return false;
    size_t ext_end = o + 2 + ((sh[o] << 8) | sh[o + 1]);
    o += 2;
    while (o + 4 <= ext_end && ext_end <= len)
    {
        uint16_t et = (uint16_t)((sh[o] << 8) | sh[o + 1]);
        uint16_t el = (uint16_t)((sh[o + 2] << 8) | sh[o + 3]);
        o += 4;
        if (et == 0x0033 && el >= 4 + 32)
        {
            memcpy(pub, sh + o + 4, 32); // group(2) + klen(2) then the key
            return true;
        }
        o += el;
    }
    return false;
}

// Pull the cookie out of a HelloRetryRequest (a ServerHello walked for the 0x002c cookie extension,
// whose body is opaque cookie<1..2^16-1>). Returns the inner cookie bytes and length.
static bool hrr_cookie(const uint8_t *sh, size_t len, uint8_t *cookie_out, size_t *cookie_len)
{
    if (len < 44)
        return false;
    size_t o = 4 + 2 + 32;
    uint8_t sid = sh[o++];
    o += sid;
    o += 2 + 1;
    if (o + 2 > len)
        return false;
    size_t ext_end = o + 2 + ((sh[o] << 8) | sh[o + 1]);
    o += 2;
    while (o + 4 <= ext_end && ext_end <= len)
    {
        uint16_t et = (uint16_t)((sh[o] << 8) | sh[o + 1]);
        uint16_t el = (uint16_t)((sh[o + 2] << 8) | sh[o + 3]);
        o += 4;
        if (et == 0x002c && el >= 2)
        {
            size_t cl = (size_t)((sh[o] << 8) | sh[o + 1]); // inner cookie length
            if (cl + 2 > el || o + 2 + cl > len)
                return false;
            memcpy(cookie_out, sh + o + 2, cl);
            *cookie_len = cl;
            return true;
        }
        o += el;
    }
    return false;
}

// One DTLS handshake message per record in this phase: strip the 12-byte DTLS handshake header from a
// record payload and rebuild the TLS handshake message (4-byte header + body). Returns TLS msg length.
static size_t frag_to_tls(const uint8_t *payload, size_t plen, uint8_t *tls_out)
{
    DtlsHsHeader hh;
    if (!dtls_hs_header_parse(payload, plen, &hh) || hh.frag_offset != 0 || hh.frag_length != hh.length)
        return 0;
    tls_out[0] = hh.msg_type;
    tls_out[1] = (uint8_t)(hh.length >> 16);
    tls_out[2] = (uint8_t)(hh.length >> 8);
    tls_out[3] = (uint8_t)hh.length;
    memcpy(tls_out + 4, hh.fragment, hh.length);
    return 4 + hh.length;
}

static size_t ct_record_len(const uint8_t *rec, size_t avail, size_t cid_len = 0)
{
    size_t pre = 1 + ((rec[0] & 0x10) ? cid_len : 0); // byte0 + optional connection id
    size_t seq_len = (rec[0] & 0x08) ? 2 : 1;
    size_t len_off = pre + seq_len;
    size_t o = len_off + 2;
    size_t enc = ((size_t)rec[len_off] << 8) | rec[len_off + 1];
    return (o + enc <= avail) ? o + enc : 0;
}

// Pull the server's connection_id (extension 0x0036, RFC 9146) out of a ServerHello. Returns its length
// (0 if absent) and copies the CID bytes to @p cid_out.
static size_t sh_conn_id(const uint8_t *sh, size_t len, uint8_t *cid_out)
{
    if (len < 44)
        return 0;
    size_t o = 4 + 2 + 32;
    uint8_t sid = sh[o++];
    o += sid;
    o += 2 + 1;
    if (o + 2 > len)
        return 0;
    size_t ext_end = o + 2 + ((sh[o] << 8) | sh[o + 1]);
    o += 2;
    while (o + 4 <= ext_end && ext_end <= len)
    {
        uint16_t et = (uint16_t)((sh[o] << 8) | sh[o + 1]);
        uint16_t el = (uint16_t)((sh[o + 2] << 8) | sh[o + 3]);
        o += 4;
        if (et == 0x0036 && el >= 1)
        {
            size_t cl = sh[o]; // 1-byte CID length
            if (1 + cl > el || o + 1 + cl > len)
                return 0;
            memcpy(cid_out, sh + o + 1, cl);
            return cl;
        }
        o += el;
    }
    return 0;
}

// Given the server's flight (its response to the client's final ClientHello) and the client transcript
// @p tr already updated through that final ClientHello, deprotect and verify the whole flight, send the
// client Finished (handshake message_seq @p cfin_msg_seq), and assert both sides install identical
// application-traffic keys. @p tr is taken by value so the caller's copy is untouched. Shared by the
// one-round-trip and HelloRetryRequest paths (they differ only in the transcript prefix and message_seq).
static void complete_handshake_from_flight(DtlsConn *conn, SshSha256Ctx tr, uint16_t cfin_msg_seq,
                                           const uint8_t *flight, size_t fl, const uint8_t *client_cid = nullptr,
                                           size_t client_cid_len = 0)
{
    size_t off = 0;
    // record 0: ServerHello (DTLSPlaintext, epoch 0)
    DtlsPlaintext pt;
    size_t rl = dtls_plaintext_parse(flight + off, fl - off, &pt);
    TEST_ASSERT_TRUE(rl > 0);
    off += rl;
    uint8_t sh[512];
    size_t sh_len = frag_to_tls(pt.fragment, pt.frag_len, sh);
    TEST_ASSERT_TRUE(sh_len > 0);
    ssh_sha256_update(&tr, sh, sh_len);

    uint8_t server_pub[32];
    TEST_ASSERT_TRUE(sh_keyshare(sh, sh_len, server_pub));

    // Connection ids (RFC 9146 / RFC 9147 §9): when we offered a CID, the ServerHello carries the
    // server's CID (which we place in records we send), and every protected record the server sends us
    // carries our CID (@p client_cid). scid = the server's CID; empty when CID was not negotiated.
    uint8_t scid[DTLS_CID_MAX];
    size_t scid_len = client_cid_len ? sh_conn_id(sh, sh_len, scid) : 0;
    if (client_cid_len)
        TEST_ASSERT_TRUE(scid_len > 0); // the server must echo a connection_id extension

    // client handshake key schedule from Transcript-Hash(..SH)
    uint8_t ecdhe[32];
    ssh_x25519(ecdhe, CLIENT_X25519_PRIV, server_pub);
    Tls13KeySchedule cks;
    uint8_t h[32];
    SshSha256Ctx tmp = tr;
    ssh_sha256_final(&tmp, h);
    tls13_ks_early(&DTLS13_KDF, &cks);
    tls13_ks_handshake(&cks, ecdhe, h, 32);

    DtlsRecordKeys srv_read; // client reads the server's epoch-2 records
    dtls_record_keys_derive(&srv_read, DtlsCipher::AES_128_GCM_SHA256, 2, cks.server_hs_traffic);

    // records 1..4: EncryptedExtensions, Certificate, CertificateVerify, Finished (DTLSCiphertext)
    uint8_t cert_pub[32];
    bool have_cert = false;
    uint64_t exp_seq = 0;
    int seen_fin = 0;
    while (off < fl)
    {
        size_t crl = ct_record_len(flight + off, fl - off, client_cid_len);
        TEST_ASSERT_TRUE(crl > 0);
        uint8_t inner[512];
        DtlsCiphertext info;
        TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&srv_read, exp_seq, flight + off, crl, inner, sizeof(inner), &info,
                                                   client_cid, client_cid_len));
        exp_seq = info.seq + 1;
        off += crl;
        TEST_ASSERT_EQUAL_UINT8(DTLS_CT_HANDSHAKE, info.content_type);

        uint8_t msg[512];
        size_t mlen = frag_to_tls(inner, info.pt_len, msg);
        TEST_ASSERT_TRUE(mlen > 0);

        if (msg[0] == 15) // CertificateVerify: verify BEFORE hashing it in, over H(..Certificate)
        {
            TEST_ASSERT_TRUE(have_cert);
            uint8_t h_ch_cert[32];
            SshSha256Ctx sc = tr;
            ssh_sha256_final(&sc, h_ch_cert);
            uint8_t content[160]; // 64*0x20 + 33-byte context + 0x00 + 32-byte hash = 130
            size_t clen = tls13_cert_verify_content(content, sizeof(content), h_ch_cert, true);
            TEST_ASSERT_TRUE(clen > 0);
            const uint8_t *sig = msg + 4 + 2 + 2; // algorithm(2) + signature length(2)
            TEST_ASSERT_TRUE(ssh_ed25519_verify(cert_pub, content, clen, sig));
        }
        if (msg[0] == 20) // server Finished: verify over H(..CertificateVerify)
        {
            uint8_t hcv[32];
            SshSha256Ctx s = tr;
            ssh_sha256_final(&s, hcv);
            uint8_t expect[32];
            tls13_finished_mac(&DTLS13_KDF, cks.server_hs_traffic, hcv, expect);
            TEST_ASSERT_EQUAL_MEMORY(expect, msg + 4, 32);
            seen_fin = 1;
        }

        ssh_sha256_update(&tr, msg, mlen);

        if (msg[0] == 11) // Certificate: the cert_data is the raw Ed25519 pubkey (this test's profile)
        {
            memcpy(cert_pub, msg + 4 + 1 + 3 + 3, 32);
            have_cert = true;
        }
    }
    TEST_ASSERT_TRUE(seen_fin);

    // --- client Finished over Transcript-Hash(..server Finished) ---
    uint8_t h_sfin[32];
    SshSha256Ctx s2 = tr;
    ssh_sha256_final(&s2, h_sfin);
    tls13_ks_master(&cks, h_sfin);

    uint8_t cfin_verify[32];
    tls13_finished_mac(&DTLS13_KDF, cks.client_hs_traffic, h_sfin, cfin_verify);
    uint8_t cfin[64];
    size_t cfin_len = tls13_build_finished(cfin, sizeof(cfin), cfin_verify);

    DtlsRecordKeys cli_write; // client writes its epoch-2 Finished
    dtls_record_keys_derive(&cli_write, DtlsCipher::AES_128_GCM_SHA256, 2, cks.client_hs_traffic);

    uint8_t cfin_frag[80];
    size_t cff = dtls_hs_frag_build(cfin[0], cfin_msg_seq, (uint32_t)(cfin_len - 4), 0, cfin + 4,
                                    (uint32_t)(cfin_len - 4), cfin_frag, sizeof(cfin_frag));
    uint8_t cfin_rec[128];
    size_t cfr = dtls_ciphertext_protect(&cli_write, 0, DTLS_CT_HANDSHAKE, cfin_frag, cff, cfin_rec, sizeof(cfin_rec),
                                         scid_len ? scid : nullptr, scid_len);

    uint8_t out2[64];
    int r2 = dws_dtls_conn_process(conn, cfin_rec, cfr, out2, sizeof(out2));
    TEST_ASSERT_TRUE(r2 > 0); // the server acknowledges the client Finished (RFC 9147 §5.8.3)
    TEST_ASSERT_TRUE(dws_dtls_conn_established(conn));

    // --- both sides agree on the application-traffic keys ---
    DtlsRecordKeys cli_app_read;  // client reads server app data (from server_ap_traffic)
    DtlsRecordKeys cli_app_write; // client writes app data (from client_ap_traffic)
    dtls_record_keys_derive(&cli_app_read, DtlsCipher::AES_128_GCM_SHA256, 3, cks.server_ap_traffic);
    dtls_record_keys_derive(&cli_app_write, DtlsCipher::AES_128_GCM_SHA256, 3, cks.client_ap_traffic);

    // The ACK is an epoch-3 (application) record of content type 26 that the client can decrypt.
    uint8_t ack_pt[64];
    DtlsCiphertext ackinfo;
    TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&cli_app_read, 0, out2, (size_t)r2, ack_pt, sizeof(ack_pt), &ackinfo,
                                               client_cid, client_cid_len));
    TEST_ASSERT_EQUAL_UINT8(DTLS_CT_ACK, ackinfo.content_type);

    const DtlsRecordKeys *srv_app_write = dws_dtls_conn_app_write_keys(conn); // server->client
    const DtlsRecordKeys *srv_app_read = dws_dtls_conn_app_read_keys(conn);   // client->server
    TEST_ASSERT_NOT_NULL(srv_app_write);
    TEST_ASSERT_NOT_NULL(srv_app_read);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_read.key, srv_app_write->key, 16);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_read.iv, srv_app_write->iv, 12);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_write.key, srv_app_read->key, 16);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_write.iv, srv_app_read->iv, 12);

    // A retransmitted client Finished (its ACK was lost) must draw a fresh ACK, not a fatal error
    // (RFC 9147 §5.8.3): re-send the same Finished in a new record and expect another ACK.
    uint8_t cfin_rec2[128];
    size_t cfr2 = dtls_ciphertext_protect(&cli_write, 1, DTLS_CT_HANDSHAKE, cfin_frag, cff, cfin_rec2,
                                          sizeof(cfin_rec2), scid_len ? scid : nullptr, scid_len);
    uint8_t out3[64];
    int r3 = dws_dtls_conn_process(conn, cfin_rec2, cfr2, out3, sizeof(out3));
    TEST_ASSERT_TRUE(r3 > 0);
    TEST_ASSERT_TRUE(dws_dtls_conn_established(conn));
    uint8_t ack_pt3[64];
    DtlsCiphertext ackinfo3;
    TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&cli_app_read, 1, out3, (size_t)r3, ack_pt3, sizeof(ack_pt3), &ackinfo3,
                                               client_cid, client_cid_len));
    TEST_ASSERT_EQUAL_UINT8(DTLS_CT_ACK, ackinfo3.content_type);
}

static void server_cfg(DtlsServerConfig *cfg, const uint8_t server_ed_pub[32])
{
    cfg->cert_der = server_ed_pub; // the "certificate" is the raw Ed25519 public key for this test
    cfg->cert_len = 32;
    cfg->ed25519_seed = SERVER_ED_SEED;
    cfg->ephemeral_priv = SERVER_X25519_PRIV;
    cfg->server_random = SERVER_RANDOM;
    cfg->cookie_key = SERVER_COOKIE_KEY;
}

// The full one-round-trip handshake, driven from the client side.
static void test_full_handshake(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);

    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    dws_dtls_conn_init(&conn, &cfg, nullptr, 0); // no HRR expected on the happy path

    // --- client flight 1: ClientHello (epoch 0) ---
    uint8_t ch[256];
    size_t ch_len = build_client_hello(ch, client_pub);

    SshSha256Ctx tr; // client transcript
    ssh_sha256_init(&tr);
    ssh_sha256_update(&tr, ch, ch_len);

    uint8_t ch_frag[300];
    size_t ch_fl = dtls_hs_frag_build(ch[0], 0, (uint32_t)(ch_len - 4), 0, ch + 4, (uint32_t)(ch_len - 4), ch_frag,
                                      sizeof(ch_frag));
    uint8_t ch_rec[320];
    size_t ch_rl = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, ch_frag, ch_fl, ch_rec, sizeof(ch_rec));

    uint8_t flight[2048];
    int fl = dws_dtls_conn_process(&conn, ch_rec, ch_rl, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0); // server produced its flight

    complete_handshake_from_flight(&conn, tr, /*cfin_msg_seq=*/1, flight, (size_t)fl);
}

// Connection id negotiation (RFC 9146 / RFC 9147 §9): the ClientHello offers a connection_id, so the
// server echoes its own CID in the ServerHello, protects its epoch-2 flight with the client's CID, and
// accepts the client's Finished carrying the server's CID - the whole handshake completes with CIDs.
static void test_cid_handshake(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);

    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    dws_dtls_conn_init(&conn, &cfg, nullptr, 0);

    // ClientHello offering a 3-byte connection id (the CID the server must place in records it sends us).
    const uint8_t client_cid[3] = {0xC1, 0xC2, 0xC3};
    uint8_t ch[256];
    size_t ch_len =
        build_client_hello_ex(ch, client_pub, /*with_keyshare=*/true, nullptr, 0, client_cid, sizeof(client_cid));
    SshSha256Ctx tr;
    ssh_sha256_init(&tr);
    ssh_sha256_update(&tr, ch, ch_len);

    uint8_t ch_frag[300];
    size_t ch_fl = dtls_hs_frag_build(ch[0], 0, (uint32_t)(ch_len - 4), 0, ch + 4, (uint32_t)(ch_len - 4), ch_frag,
                                      sizeof(ch_frag));
    uint8_t ch_rec[320];
    size_t ch_rl = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, ch_frag, ch_fl, ch_rec, sizeof(ch_rec));

    uint8_t flight[2048];
    int fl = dws_dtls_conn_process(&conn, ch_rec, ch_rl, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0);

    // The server's first epoch-2 record (after the plaintext ServerHello) must carry the C bit + our CID.
    DtlsPlaintext sh_pt;
    size_t shrl = dtls_plaintext_parse(flight, (size_t)fl, &sh_pt);
    TEST_ASSERT_TRUE(shrl > 0);
    const uint8_t *ep2 = flight + shrl;
    TEST_ASSERT_TRUE((ep2[0] & 0x10) != 0); // C bit set on the encrypted flight
    TEST_ASSERT_EQUAL_MEMORY(client_cid, ep2 + 1, sizeof(client_cid));

    // The rest completes with CIDs threaded both directions (this also checks the ServerHello carries the
    // server's connection_id, the client's CID-bearing Finished is accepted, and both derive matching keys).
    complete_handshake_from_flight(&conn, tr, /*cfin_msg_seq=*/1, flight, (size_t)fl, client_cid, sizeof(client_cid));
}

// The HelloRetryRequest path (RFC 9147 §5.1): a ClientHello that offers X25519 but sends no key_share
// draws an HRR with a cookie; the retry echoes the cookie and its X25519 share, and the same full
// handshake completes over the message_hash || HRR || ClientHello2 || ... transcript (RFC 8446 §4.4.1).
static void test_hrr_group_renegotiation(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);

    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    dws_dtls_conn_init(&conn, &cfg, TEST_PEER_ADDR, sizeof(TEST_PEER_ADDR));

    // --- client flight 1: ClientHello with NO key_share (message_seq 0) ---
    uint8_t ch1[256];
    size_t ch1_len = build_client_hello_ex(ch1, client_pub, /*with_keyshare=*/false, nullptr, 0);
    uint8_t f1[300];
    size_t f1l =
        dtls_hs_frag_build(ch1[0], 0, (uint32_t)(ch1_len - 4), 0, ch1 + 4, (uint32_t)(ch1_len - 4), f1, sizeof(f1));
    uint8_t r1[320];
    size_t r1l = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, f1, f1l, r1, sizeof(r1));

    uint8_t hrr_flight[512];
    int hf = dws_dtls_conn_process(&conn, r1, r1l, hrr_flight, sizeof(hrr_flight));
    TEST_ASSERT_TRUE(hf > 0);
    TEST_ASSERT_FALSE(dws_dtls_conn_established(&conn)); // just an HRR so far

    // --- the server flight is a single epoch-0 plaintext HelloRetryRequest ---
    DtlsPlaintext pt;
    size_t rl = dtls_plaintext_parse(hrr_flight, (size_t)hf, &pt);
    TEST_ASSERT_TRUE(rl > 0);
    uint8_t hrr[512];
    size_t hrr_len = frag_to_tls(pt.fragment, pt.frag_len, hrr);
    TEST_ASSERT_TRUE(hrr_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x02, hrr[0]);                       // ServerHello handshake type
    TEST_ASSERT_EQUAL_MEMORY(tls13_hrr_random, hrr + 4 + 2, 32); // the HRR magic random marks it an HRR
    uint8_t cookie[DTLS_COOKIE_MAX];
    size_t cookie_len = 0;
    TEST_ASSERT_TRUE(hrr_cookie(hrr, hrr_len, cookie, &cookie_len));
    TEST_ASSERT_TRUE(cookie_len > 0);

    // --- client transcript for the HRR path: message_hash(Hash(CH1)) || HRR || CH2 (RFC 8446 §4.4.1) ---
    uint8_t ch1_hash[32];
    SshSha256Ctx h1;
    ssh_sha256_init(&h1);
    ssh_sha256_update(&h1, ch1, ch1_len);
    ssh_sha256_final(&h1, ch1_hash);

    SshSha256Ctx tr;
    ssh_sha256_init(&tr);
    uint8_t mh[36];
    size_t mhl = tls13_build_message_hash(mh, sizeof(mh), ch1_hash);
    TEST_ASSERT_TRUE(mhl > 0);
    ssh_sha256_update(&tr, mh, mhl);
    ssh_sha256_update(&tr, hrr, hrr_len);

    // --- client flight 2: ClientHello with the X25519 share and the echoed cookie (message_seq 1) ---
    uint8_t ch2[320];
    size_t ch2_len = build_client_hello_ex(ch2, client_pub, /*with_keyshare=*/true, cookie, cookie_len);
    ssh_sha256_update(&tr, ch2, ch2_len);

    uint8_t f2[380];
    size_t f2l =
        dtls_hs_frag_build(ch2[0], 1, (uint32_t)(ch2_len - 4), 0, ch2 + 4, (uint32_t)(ch2_len - 4), f2, sizeof(f2));
    uint8_t r2[420];
    size_t r2l = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 1, f2, f2l, r2, sizeof(r2));

    uint8_t flight[2048];
    int fl = dws_dtls_conn_process(&conn, r2, r2l, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0); // the full server flight

    complete_handshake_from_flight(&conn, tr, /*cfin_msg_seq=*/2, flight, (size_t)fl);
}

// After an HRR, a retry that carries the X25519 share but omits (or corrupts) the cookie is rejected:
// the server refuses to spend the handshake before the client's address is proven (RFC 9147 §5.1).
static void test_hrr_retry_without_cookie_rejected(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);

    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    dws_dtls_conn_init(&conn, &cfg, TEST_PEER_ADDR, sizeof(TEST_PEER_ADDR));

    // CH1 without a key_share -> HRR.
    uint8_t ch1[256];
    size_t ch1_len = build_client_hello_ex(ch1, client_pub, /*with_keyshare=*/false, nullptr, 0);
    uint8_t f1[300];
    size_t f1l =
        dtls_hs_frag_build(ch1[0], 0, (uint32_t)(ch1_len - 4), 0, ch1 + 4, (uint32_t)(ch1_len - 4), f1, sizeof(f1));
    uint8_t r1[320];
    size_t r1l = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, f1, f1l, r1, sizeof(r1));
    uint8_t hrr_flight[512];
    TEST_ASSERT_TRUE(dws_dtls_conn_process(&conn, r1, r1l, hrr_flight, sizeof(hrr_flight)) > 0);

    // CH2 with a key_share but NO cookie (message_seq 1) -> handshake_failure.
    uint8_t ch2[320];
    size_t ch2_len = build_client_hello_ex(ch2, client_pub, /*with_keyshare=*/true, nullptr, 0);
    uint8_t f2[380];
    size_t f2l =
        dtls_hs_frag_build(ch2[0], 1, (uint32_t)(ch2_len - 4), 0, ch2 + 4, (uint32_t)(ch2_len - 4), f2, sizeof(f2));
    uint8_t r2[420];
    size_t r2l = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 1, f2, f2l, r2, sizeof(r2));
    uint8_t out[2048];
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_process(&conn, r2, r2l, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(40, dws_dtls_conn_alert(&conn)); // handshake_failure
}

// A ClientHello that does not offer TLS 1.3 is rejected with a protocol_version alert.
static void test_reject_no_tls13(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);
    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    dws_dtls_conn_init(&conn, &cfg, nullptr, 0);

    uint8_t ch[256];
    size_t ch_len = build_client_hello(ch, client_pub);
    // Corrupt the supported_versions value 0xFEFC (DTLS 1.3) -> 0xFEFD (DTLS 1.2) so offers_tls13 is false.
    for (size_t i = 0; i + 1 < ch_len; i++)
        if (ch[i] == 0xFE && ch[i + 1] == 0xFC)
        {
            ch[i + 1] = 0xFD;
            break;
        }
    uint8_t frag[300], rec[320], out[1024];
    size_t fl =
        dtls_hs_frag_build(ch[0], 0, (uint32_t)(ch_len - 4), 0, ch + 4, (uint32_t)(ch_len - 4), frag, sizeof(frag));
    size_t rl = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, frag, fl, rec, sizeof(rec));
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_process(&conn, rec, rl, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(70, dws_dtls_conn_alert(&conn)); // protocol_version
}

// Drive a fresh connection from ClientHello to WAIT_FINISHED; return the server flight and seed the
// client transcript @p tr through the ClientHello. Shared by the retransmission-timer tests.
static int drive_server_flight(DtlsConn *conn, DtlsServerConfig *cfg, SshSha256Ctx *tr, uint8_t *flight,
                               size_t flight_cap)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    dws_dtls_conn_init(conn, cfg, nullptr, 0);
    uint8_t ch[256];
    size_t ch_len = build_client_hello(ch, client_pub);
    ssh_sha256_init(tr);
    ssh_sha256_update(tr, ch, ch_len);
    uint8_t ch_frag[300];
    size_t ch_fl = dtls_hs_frag_build(ch[0], 0, (uint32_t)(ch_len - 4), 0, ch + 4, (uint32_t)(ch_len - 4), ch_frag,
                                      sizeof(ch_frag));
    uint8_t ch_rec[320];
    size_t ch_rl = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0, ch_frag, ch_fl, ch_rec, sizeof(ch_rec));
    return dws_dtls_conn_process(conn, ch_rec, ch_rl, flight, flight_cap);
}

// The retransmission timer (RFC 9147 §5.8): after the server flight, the timer is armed at the initial
// PTO; once it elapses the whole flight is re-sent with fresh record sequence numbers, and the client
// completes the handshake from that retransmission.
static void test_pto_retransmit_and_recovery(void)
{
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);
    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    SshSha256Ctx tr;
    uint8_t flight[2048];
    int fl = drive_server_flight(&conn, &cfg, &tr, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0);

    TEST_ASSERT_EQUAL_INT((int)DTLS_PTO_INITIAL_MS, dws_dtls_conn_timeout_ms(&conn)); // armed at the initial PTO

    uint8_t rflight[2048];
    TEST_ASSERT_EQUAL_INT(0, dws_dtls_conn_on_timeout(&conn, rflight, sizeof(rflight))); // not due yet -> no-op

    g_ms += DTLS_PTO_INITIAL_MS;
    int rfl = dws_dtls_conn_on_timeout(&conn, rflight, sizeof(rflight));
    TEST_ASSERT_TRUE(rfl > 0); // whole flight retransmitted
    TEST_ASSERT_EQUAL_INT((int)(DTLS_PTO_INITIAL_MS * 2), dws_dtls_conn_timeout_ms(&conn)); // backed off to 2x

    // The retransmission is a valid, completable server flight (fresh record seqs); completing it also
    // disarms the timer.
    complete_handshake_from_flight(&conn, tr, 1, rflight, (size_t)rfl);
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_timeout_ms(&conn));
}

// The timer doubles each retransmission up to the cap, and the handshake is abandoned after the
// retransmission ceiling (RFC 9147 §5.8.1).
static void test_pto_backoff_and_giveup(void)
{
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);
    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    SshSha256Ctx tr;
    uint8_t flight[2048], rflight[2048];
    TEST_ASSERT_TRUE(drive_server_flight(&conn, &cfg, &tr, flight, sizeof(flight)) > 0);

    uint32_t expect = DTLS_PTO_INITIAL_MS;
    TEST_ASSERT_EQUAL_INT((int)expect, dws_dtls_conn_timeout_ms(&conn));
    for (int i = 0; i < DTLS_MAX_RETRANSMITS; i++)
    {
        g_ms += DTLS_PTO_MAX_MS + 1000; // well past any PTO
        TEST_ASSERT_TRUE(dws_dtls_conn_on_timeout(&conn, rflight, sizeof(rflight)) > 0);
        expect = expect >= DTLS_PTO_MAX_MS / 2 ? DTLS_PTO_MAX_MS : expect * 2;
        TEST_ASSERT_EQUAL_INT((int)expect, dws_dtls_conn_timeout_ms(&conn)); // doubled, capped at the max
    }
    g_ms += DTLS_PTO_MAX_MS + 1000;
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_on_timeout(&conn, rflight, sizeof(rflight)));          // ceiling: give up
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_timeout_ms(&conn));                                    // FAILED: no timer
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_process(&conn, rflight, 1, rflight, sizeof(rflight))); // and rejects input
}

// A client ACK covering the whole server flight stops retransmission (RFC 9147 §5.8.3).
static void test_pto_ack_cancels_retransmit(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);
    DtlsServerConfig cfg;
    server_cfg(&cfg, server_ed_pub);
    DtlsConn conn;
    SshSha256Ctx tr;
    uint8_t flight[2048];
    int fl = drive_server_flight(&conn, &cfg, &tr, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0);
    TEST_ASSERT_TRUE(dws_dtls_conn_timeout_ms(&conn) >= 0); // armed

    // Derive the client's epoch-2 write keys from the ServerHello, as a real client would.
    DtlsPlaintext pt;
    TEST_ASSERT_TRUE(dtls_plaintext_parse(flight, (size_t)fl, &pt) > 0);
    uint8_t sh[512];
    size_t sh_len = frag_to_tls(pt.fragment, pt.frag_len, sh);
    TEST_ASSERT_TRUE(sh_len > 0);
    uint8_t server_pub[32];
    TEST_ASSERT_TRUE(sh_keyshare(sh, sh_len, server_pub));
    uint8_t ch[256];
    size_t ch_len = build_client_hello(ch, client_pub);
    SshSha256Ctx t;
    ssh_sha256_init(&t);
    ssh_sha256_update(&t, ch, ch_len);
    ssh_sha256_update(&t, sh, sh_len);
    uint8_t h[32];
    ssh_sha256_final(&t, h);
    uint8_t ecdhe[32];
    ssh_x25519(ecdhe, CLIENT_X25519_PRIV, server_pub);
    Tls13KeySchedule cks;
    tls13_ks_early(&DTLS13_KDF, &cks);
    tls13_ks_handshake(&cks, ecdhe, h, 32);
    DtlsRecordKeys cli_write;
    dtls_record_keys_derive(&cli_write, DtlsCipher::AES_128_GCM_SHA256, 2, cks.client_hs_traffic);

    // ACK the whole flight: ServerHello (epoch 0, seq 0) + the four epoch-2 messages (seq 0..3).
    DtlsRecordNumber rns[5] = {{0, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3}};
    uint8_t ack_body[2 + 5 * 16];
    size_t bl = dtls_ack_build(rns, 5, ack_body, sizeof(ack_body));
    TEST_ASSERT_TRUE(bl > 0);
    uint8_t ack_rec[160];
    size_t ar = dtls_ciphertext_protect(&cli_write, 0, DTLS_CT_ACK, ack_body, bl, ack_rec, sizeof(ack_rec));
    TEST_ASSERT_TRUE(ar > 0);

    uint8_t out[64];
    dws_dtls_conn_process(&conn, ack_rec, ar, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(-1, dws_dtls_conn_timeout_ms(&conn)); // the ACK stopped the retransmission timer
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_full_handshake);
    RUN_TEST(test_cid_handshake);
    RUN_TEST(test_hrr_group_renegotiation);
    RUN_TEST(test_hrr_retry_without_cookie_rejected);
    RUN_TEST(test_pto_retransmit_and_recovery);
    RUN_TEST(test_pto_backoff_and_giveup);
    RUN_TEST(test_pto_ack_cancels_retransmit);
    RUN_TEST(test_reject_no_tls13);
    return UNITY_END();
}
