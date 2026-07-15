// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DTLS 1.3 server handshake (RFC 9147 §5-6). A self-consistent proof: the test plays a minimal DTLS
// 1.3 client against dtls_conn - it builds a ClientHello, drives the server, then runs the SAME TLS
// 1.3 key schedule to deprotect the server flight, VERIFIES the server's CertificateVerify signature
// and Finished MAC over the real transcript, sends its own Finished, and confirms both sides install
// identical application-traffic keys. If any transcript byte, epoch, nonce, or key were wrong the
// AEAD open, the signature check, or the key comparison would fail.

#include "network_drivers/presentation/dtls/dtls_conn.h"
#include "network_drivers/presentation/dtls/dtls_handshake.h"
#include "network_drivers/presentation/dtls/dtls_record.h"
#include "network_drivers/presentation/http3/tls13_kdf.h"
#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
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

// Build a ClientHello (TLS message, 4-byte header + body) offering TLS 1.3 / X25519 / Ed25519 and a
// key_share for @p client_pub. Returns its length.
static size_t build_client_hello(uint8_t *out, const uint8_t client_pub[32])
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
    // key_share: x25519 entry
    b16(&b, 0x0033);
    b16(&b, 0x0026); // ext body: client_shares(2) + entry(2+2+32)
    b16(&b, 0x0024); // client_shares length
    b16(&b, 0x001d); // group x25519
    b16(&b, 0x0020); // key length 32
    bmem(&b, client_pub, 32);
    uint16_t ext_len = (uint16_t)(b.n - ext_len_at - 2);
    out[ext_len_at] = (uint8_t)(ext_len >> 8);
    out[ext_len_at + 1] = (uint8_t)ext_len;
    uint32_t body = (uint32_t)(b.n - len_at - 3);
    out[len_at] = (uint8_t)(body >> 16);
    out[len_at + 1] = (uint8_t)(body >> 8);
    out[len_at + 2] = (uint8_t)body;
    return b.n;
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

static size_t ct_record_len(const uint8_t *rec, size_t avail)
{
    size_t seq_len = (rec[0] & 0x08) ? 2 : 1;
    size_t o = 1 + seq_len + 2;
    size_t enc = ((size_t)rec[1 + seq_len] << 8) | rec[1 + seq_len + 1];
    return (o + enc <= avail) ? o + enc : 0;
}

// The full handshake, driven from the client side.
static void test_full_handshake(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);

    DtlsServerConfig cfg;
    cfg.cert_der = server_ed_pub; // the "certificate" is the raw Ed25519 public key for this test
    cfg.cert_len = 32;
    cfg.ed25519_seed = SERVER_ED_SEED;
    cfg.ephemeral_priv = SERVER_X25519_PRIV;
    cfg.server_random = SERVER_RANDOM;

    DtlsConn conn;
    dtls_conn_init(&conn, &cfg);

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
    int fl = dtls_conn_process(&conn, ch_rec, ch_rl, flight, sizeof(flight));
    TEST_ASSERT_TRUE(fl > 0); // server produced its flight

    // --- parse the server flight ---
    size_t off = 0;
    // record 0: ServerHello (DTLSPlaintext, epoch 0)
    DtlsPlaintext pt;
    size_t rl = dtls_plaintext_parse(flight + off, (size_t)fl - off, &pt);
    TEST_ASSERT_TRUE(rl > 0);
    off += rl;
    uint8_t sh[512];
    size_t sh_len = frag_to_tls(pt.fragment, pt.frag_len, sh);
    TEST_ASSERT_TRUE(sh_len > 0);
    ssh_sha256_update(&tr, sh, sh_len);

    uint8_t server_pub[32];
    TEST_ASSERT_TRUE(sh_keyshare(sh, sh_len, server_pub));

    // client handshake key schedule from Transcript-Hash(CH..SH)
    uint8_t ecdhe[32];
    ssh_x25519(ecdhe, CLIENT_X25519_PRIV, server_pub);
    Tls13KeySchedule cks;
    uint8_t h[32];
    SshSha256Ctx tmp = tr;
    ssh_sha256_final(&tmp, h); // H(CH..SH)
    tls13_ks_early(&DTLS13_KDF, &cks);
    tls13_ks_handshake(&cks, ecdhe, h, 32);

    DtlsRecordKeys srv_read; // client reads the server's epoch-2 records
    dtls_record_keys_derive(&srv_read, DtlsCipher::AES_128_GCM_SHA256, 2, cks.server_hs_traffic);

    // records 1..4: EncryptedExtensions, Certificate, CertificateVerify, Finished (DTLSCiphertext)
    uint8_t cert_pub[32];
    bool have_cert = false;
    uint64_t exp_seq = 0;
    int seen_fin = 0;
    while (off < (size_t)fl)
    {
        size_t crl = ct_record_len(flight + off, (size_t)fl - off);
        TEST_ASSERT_TRUE(crl > 0);
        uint8_t inner[512];
        DtlsCiphertext info;
        TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&srv_read, exp_seq, flight + off, crl, inner, sizeof(inner), &info));
        exp_seq = info.seq + 1;
        off += crl;
        TEST_ASSERT_EQUAL_UINT8(DTLS_CT_HANDSHAKE, info.content_type);

        uint8_t msg[512];
        size_t mlen = frag_to_tls(inner, info.pt_len, msg);
        TEST_ASSERT_TRUE(mlen > 0);

        if (msg[0] == 15) // CertificateVerify: verify BEFORE hashing it in, over H(CH..Certificate)
        {
            TEST_ASSERT_TRUE(have_cert);
            uint8_t h_ch_cert[32]; // transcript now holds CH..Certificate (CV not yet hashed in)
            SshSha256Ctx sc = tr;
            ssh_sha256_final(&sc, h_ch_cert);
            uint8_t content[160]; // 64*0x20 + 33-byte context + 0x00 + 32-byte hash = 130
            size_t clen = tls13_cert_verify_content(content, sizeof(content), h_ch_cert, true);
            TEST_ASSERT_TRUE(clen > 0);
            const uint8_t *sig = msg + 4 + 2 + 2; // algorithm(2) + signature length(2)
            TEST_ASSERT_TRUE(ssh_ed25519_verify(cert_pub, content, clen, sig));
        }
        if (msg[0] == 20) // server Finished: verify over H(CH..CertificateVerify)
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

    // --- client Finished over Transcript-Hash(CH..server Finished) ---
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
    size_t cff = dtls_hs_frag_build(cfin[0], 1, (uint32_t)(cfin_len - 4), 0, cfin + 4, (uint32_t)(cfin_len - 4),
                                    cfin_frag, sizeof(cfin_frag));
    uint8_t cfin_rec[128];
    size_t cfr = dtls_ciphertext_protect(&cli_write, 0, DTLS_CT_HANDSHAKE, cfin_frag, cff, cfin_rec, sizeof(cfin_rec));

    uint8_t out2[64];
    int r2 = dtls_conn_process(&conn, cfin_rec, cfr, out2, sizeof(out2));
    TEST_ASSERT_TRUE(r2 > 0); // the server acknowledges the client Finished (RFC 9147 §5.8.3)
    TEST_ASSERT_TRUE(dtls_conn_established(&conn));

    // --- both sides agree on the application-traffic keys ---
    DtlsRecordKeys cli_app_read;  // client reads server app data (from server_ap_traffic)
    DtlsRecordKeys cli_app_write; // client writes app data (from client_ap_traffic)
    dtls_record_keys_derive(&cli_app_read, DtlsCipher::AES_128_GCM_SHA256, 3, cks.server_ap_traffic);
    dtls_record_keys_derive(&cli_app_write, DtlsCipher::AES_128_GCM_SHA256, 3, cks.client_ap_traffic);

    // The ACK is an epoch-3 (application) record of content type 26 that the client can decrypt.
    uint8_t ack_pt[64];
    DtlsCiphertext ackinfo;
    TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&cli_app_read, 0, out2, (size_t)r2, ack_pt, sizeof(ack_pt), &ackinfo));
    TEST_ASSERT_EQUAL_UINT8(DTLS_CT_ACK, ackinfo.content_type);

    const DtlsRecordKeys *srv_app_write = dtls_conn_app_write_keys(&conn); // server->client
    const DtlsRecordKeys *srv_app_read = dtls_conn_app_read_keys(&conn);   // client->server
    TEST_ASSERT_NOT_NULL(srv_app_write);
    TEST_ASSERT_NOT_NULL(srv_app_read);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_read.key, srv_app_write->key, 16);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_read.iv, srv_app_write->iv, 12);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_write.key, srv_app_read->key, 16);
    TEST_ASSERT_EQUAL_MEMORY(cli_app_write.iv, srv_app_read->iv, 12);
}

// A ClientHello that does not offer TLS 1.3 is rejected with a protocol_version alert.
static void test_reject_no_tls13(void)
{
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_X25519_PRIV);
    uint8_t server_ed_pub[32];
    ssh_ed25519_pubkey(server_ed_pub, SERVER_ED_SEED);
    DtlsServerConfig cfg = {server_ed_pub, 32, SERVER_ED_SEED, SERVER_X25519_PRIV, SERVER_RANDOM};
    DtlsConn conn;
    dtls_conn_init(&conn, &cfg);

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
    TEST_ASSERT_EQUAL_INT(-1, dtls_conn_process(&conn, rec, rl, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(70, dtls_conn_alert(&conn)); // protocol_version
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_full_handshake);
    RUN_TEST(test_reject_no_tls13);
    return UNITY_END();
}
