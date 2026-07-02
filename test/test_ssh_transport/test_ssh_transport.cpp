// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH transport handshake tests (RFC 4253): identification-string exchange and
// KEXINIT algorithm negotiation.

#include "network_drivers/presentation/ssh/ssh_dh.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include "network_drivers/presentation/ssh/ssh_rsa.h"
#include "network_drivers/presentation/ssh/ssh_sha256.h"
#include "network_drivers/presentation/ssh/ssh_transport.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

// Native RSA test fixture (defined in ssh_rsa.cpp native path).
extern uint8_t _test_rsa_n[256];
extern uint8_t _test_rsa_d[256];
extern uint8_t _test_rsa_e[4];

void setUp()
{
    ssh_transport_init(0);
}
void tearDown()
{
}

// ---- name-list / KEXINIT builder for crafting client messages -------------

static size_t put_namelist(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, s, n);
    return 4 + n;
}

// Build a client KEXINIT with the given algorithm lists.
static size_t build_client_kexinit(uint8_t *out, const char *kex, const char *hostkey, const char *cipher,
                                   const char *mac, const char *comp)
{
    size_t o = 0;
    out[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        out[o++] = (uint8_t)j; // cookie
    o += put_namelist(out + o, kex);
    o += put_namelist(out + o, hostkey);
    o += put_namelist(out + o, cipher); // enc c2s
    o += put_namelist(out + o, cipher); // enc s2c
    o += put_namelist(out + o, mac);    // mac c2s
    o += put_namelist(out + o, mac);    // mac s2c
    o += put_namelist(out + o, comp);   // comp c2s
    o += put_namelist(out + o, comp);   // comp s2c
    o += put_namelist(out + o, "");     // lang c2s
    o += put_namelist(out + o, "");     // lang s2c
    out[o++] = 0;                       // first_kex_packet_follows
    for (int j = 0; j < 4; j++)
        out[o++] = 0; // reserved
    return o;
}

// ---- banner ---------------------------------------------------------------

void test_server_banner_format()
{
    uint8_t buf[64];
    size_t n = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_transport_server_banner(buf, &n, sizeof(buf)));
    TEST_ASSERT_TRUE(n > 8);
    TEST_ASSERT_EQUAL_MEMORY("SSH-2.0-", buf, 8);
    TEST_ASSERT_EQUAL('\r', buf[n - 2]);
    TEST_ASSERT_EQUAL('\n', buf[n - 1]);
}

void test_recv_banner_complete()
{
    const char *banner = "SSH-2.0-OpenSSH_9.6\r\n";
    size_t consumed = 0;
    int rc = ssh_transport_recv_banner(0, (const uint8_t *)banner, strlen(banner), &consumed);
    TEST_ASSERT_EQUAL_INT(1, rc);
    TEST_ASSERT_EQUAL_size_t(strlen(banner), consumed);
    TEST_ASSERT_EQUAL_STRING("SSH-2.0-OpenSSH_9.6", ssh_sess[0].v_c);
    TEST_ASSERT_EQUAL(SSH_PHASE_KEXINIT, ssh_sess[0].phase);
}

void test_recv_banner_bare_lf()
{
    const char *banner = "SSH-2.0-x\n"; // some clients omit CR
    size_t consumed = 0;
    TEST_ASSERT_EQUAL_INT(1, ssh_transport_recv_banner(0, (const uint8_t *)banner, strlen(banner), &consumed));
    TEST_ASSERT_EQUAL_STRING("SSH-2.0-x", ssh_sess[0].v_c);
}

void test_recv_banner_split_across_reads()
{
    const char *p1 = "SSH-2.0-Cli";
    const char *p2 = "ent_1\r\n";
    size_t consumed = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_transport_recv_banner(0, (const uint8_t *)p1, strlen(p1), &consumed));
    TEST_ASSERT_EQUAL_INT(1, ssh_transport_recv_banner(0, (const uint8_t *)p2, strlen(p2), &consumed));
    TEST_ASSERT_EQUAL_STRING("SSH-2.0-Client_1", ssh_sess[0].v_c);
}

void test_recv_banner_skips_preamble_lines()
{
    // RFC 4253 §4.2 allows lines before the SSH identification string.
    const char *data = "hello from server\r\nnotice: terms\r\nSSH-2.0-Real\r\n";
    size_t consumed = 0;
    TEST_ASSERT_EQUAL_INT(1, ssh_transport_recv_banner(0, (const uint8_t *)data, strlen(data), &consumed));
    TEST_ASSERT_EQUAL_STRING("SSH-2.0-Real", ssh_sess[0].v_c);
}

// ---- KEXINIT --------------------------------------------------------------

void test_kexinit_build_starts_with_msg_and_stores_is()
{
    uint8_t buf[SSH_KEXINIT_MAX];
    size_t n = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_build(0, buf, &n, sizeof(buf)));
    TEST_ASSERT_EQUAL(SSH_MSG_KEXINIT, buf[0]);
    TEST_ASSERT_EQUAL_size_t(n, ssh_sess[0].i_s_len);
    TEST_ASSERT_EQUAL_MEMORY(buf, ssh_sess[0].i_s, n);
}

void test_kexinit_parse_accepts_supported()
{
    ssh_sess[0].phase = SSH_PHASE_KEXINIT;
    uint8_t buf[SSH_KEXINIT_MAX];
    size_t n = build_client_kexinit(buf, "diffie-hellman-group14-sha256", "rsa-sha2-256", "aes256-ctr", "hmac-sha2-256",
                                    "none");
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, buf, n));
    TEST_ASSERT_EQUAL(SSH_PHASE_DH_INIT, ssh_sess[0].phase);
    TEST_ASSERT_EQUAL_size_t(n, ssh_sess[0].i_c_len);
}

void test_kexinit_parse_accepts_when_ours_listed_among_others()
{
    uint8_t buf[SSH_KEXINIT_MAX];
    size_t n =
        build_client_kexinit(buf, "curve25519-sha256,diffie-hellman-group14-sha256", "ssh-ed25519,rsa-sha2-256",
                             "chacha20-poly1305@openssh.com,aes256-ctr", "hmac-sha2-512,hmac-sha2-256", "none,zlib");
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, buf, n));
}

void test_kexinit_parse_rejects_missing_kex()
{
    uint8_t buf[SSH_KEXINIT_MAX];
    size_t n = build_client_kexinit(buf, "curve25519-sha256", "rsa-sha2-256", "aes256-ctr", "hmac-sha2-256", "none");
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, n));
}

void test_kexinit_parse_rejects_missing_cipher()
{
    uint8_t buf[SSH_KEXINIT_MAX];
    size_t n = build_client_kexinit(buf, "diffie-hellman-group14-sha256", "rsa-sha2-256",
                                    "chacha20-poly1305@openssh.com", "hmac-sha2-256", "none");
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, n));
}

void test_kexinit_parse_rejects_truncated()
{
    uint8_t buf[8] = {SSH_MSG_KEXINIT, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, sizeof(buf)));
}

// ---- exchange hash (RFC 4253 §8) ------------------------------------------

// Independent SSH string / mpint encoders for cross-checking the production
// exchange-hash assembly.
static size_t put_string(uint8_t *p, const uint8_t *d, size_t n)
{
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, d, n);
    return 4 + n;
}

static size_t put_mpint(uint8_t *p, const uint8_t *be, size_t len)
{
    size_t off = 0;
    while (off < len && be[off] == 0)
        off++;
    if (off == len)
        return put_string(p, nullptr, 0);
    bool pad = (be[off] & 0x80u) != 0;
    size_t mlen = (len - off) + (pad ? 1 : 0);
    p[0] = (uint8_t)(mlen >> 24);
    p[1] = (uint8_t)(mlen >> 16);
    p[2] = (uint8_t)(mlen >> 8);
    p[3] = (uint8_t)mlen;
    size_t o = 4;
    if (pad)
        p[o++] = 0x00;
    memcpy(p + o, be + off, len - off);
    return o + (len - off);
}

void test_exchange_hash_matches_independent_assembly()
{
    // Populate the session fields the hash reads.
    SshSession *s = &ssh_sess[0];
    const char *vc = "SSH-2.0-TestClient";
    strcpy(s->v_c, vc);
    s->v_c_len = (uint16_t)strlen(vc);
    for (int j = 0; j < 40; j++)
        s->i_c[j] = (uint8_t)(j + 1);
    s->i_c_len = 40;
    for (int j = 0; j < 50; j++)
        s->i_s[j] = (uint8_t)(100 + j);
    s->i_s_len = 50;

    uint8_t e_be[256], f_be[256], k_be[256], ks[64];
    memset(e_be, 0, sizeof(e_be));
    memset(f_be, 0, sizeof(f_be));
    memset(k_be, 0, sizeof(k_be));
    e_be[0] = 0x80; // high bit set → mpint padding exercised
    e_be[255] = 0x11;
    f_be[1] = 0x01; // leading zero byte stripped
    f_be[255] = 0x22;
    k_be[255] = 0x05; // small value, many leading zeros
    for (int j = 0; j < 64; j++)
        ks[j] = (uint8_t)(0xA0 + j);

    uint8_t got[SSH_SHA256_DIGEST_LEN];
    TEST_ASSERT_EQUAL_INT(0, ssh_kex_exchange_hash(0, e_be, f_be, k_be, ks, sizeof(ks), got));

    // Build the same pre-image independently and hash it.
    static uint8_t pre[2048];
    size_t o = 0;
    o += put_string(pre + o, (const uint8_t *)vc, strlen(vc));
    o += put_string(pre + o, (const uint8_t *)SSH_SERVER_VERSION, strlen(SSH_SERVER_VERSION));
    o += put_string(pre + o, s->i_c, s->i_c_len);
    o += put_string(pre + o, s->i_s, s->i_s_len);
    o += put_string(pre + o, ks, sizeof(ks));
    o += put_mpint(pre + o, e_be, 256);
    o += put_mpint(pre + o, f_be, 256);
    o += put_mpint(pre + o, k_be, 256);

    uint8_t expected[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(pre, o, expected);

    TEST_ASSERT_EQUAL_MEMORY(expected, got, SSH_SHA256_DIGEST_LEN);
}

void test_exchange_hash_changes_with_input()
{
    SshSession *s = &ssh_sess[0];
    strcpy(s->v_c, "SSH-2.0-A");
    s->v_c_len = 9;
    s->i_c_len = 0;
    s->i_s_len = 0;

    uint8_t e_be[256] = {0}, f_be[256] = {0}, k_be[256] = {0}, ks[4] = {1, 2, 3, 4};
    k_be[255] = 1;
    uint8_t h1[SSH_SHA256_DIGEST_LEN], h2[SSH_SHA256_DIGEST_LEN];
    ssh_kex_exchange_hash(0, e_be, f_be, k_be, ks, sizeof(ks), h1);
    k_be[255] = 2; // different shared secret
    ssh_kex_exchange_hash(0, e_be, f_be, k_be, ks, sizeof(ks), h2);
    TEST_ASSERT_NOT_EQUAL(0, memcmp(h1, h2, SSH_SHA256_DIGEST_LEN));
}

// ---- KEXDH (RFC 4253 §8) --------------------------------------------------

void test_kexdh_parse_init_extracts_e_with_padding()
{
    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[0] = 0x80; // high bit set → mpint carries a 0x00 sign byte
    e_be[255] = 0xAB;

    uint8_t pkt[300];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    size_t n = 1 + put_mpint(pkt + 1, e_be, 256);

    uint8_t got[256];
    TEST_ASSERT_EQUAL_INT(0, ssh_kexdh_parse_init(pkt, n, got));
    TEST_ASSERT_EQUAL_MEMORY(e_be, got, 256);
}

void test_kexdh_parse_init_extracts_small_e()
{
    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x05; // small value → many leading zeros stripped on the wire

    uint8_t pkt[300];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    size_t n = 1 + put_mpint(pkt + 1, e_be, 256);

    uint8_t got[256];
    TEST_ASSERT_EQUAL_INT(0, ssh_kexdh_parse_init(pkt, n, got));
    TEST_ASSERT_EQUAL_MEMORY(e_be, got, 256);
}

void test_kexdh_parse_init_rejects_wrong_type()
{
    uint8_t pkt[8] = {99, 0, 0, 0, 1, 0x05};
    uint8_t got[256];
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_parse_init(pkt, sizeof(pkt), got));
}

void test_kexdh_parse_init_rejects_oversized_e()
{
    // mpint with 300 magnitude bytes → exceeds 2048 bits.
    uint8_t pkt[320];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    pkt[1] = 0;
    pkt[2] = 0;
    pkt[3] = 0x01;
    pkt[4] = 0x2C; // length = 300
    for (int j = 0; j < 300; j++)
        pkt[5 + j] = 0x01;
    uint8_t got[256];
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_parse_init(pkt, 5 + 300, got));
}

void test_kexdh_build_reply_structure()
{
    uint8_t ks[12];
    for (int j = 0; j < 12; j++)
        ks[j] = (uint8_t)(0x30 + j);
    uint8_t f_be[256];
    memset(f_be, 0, sizeof(f_be));
    f_be[255] = 0x09;
    uint8_t sig[256];
    for (int j = 0; j < 256; j++)
        sig[j] = (uint8_t)j;

    uint8_t out[1024];
    size_t n = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_kexdh_build_reply(ks, sizeof(ks), f_be, sig, sizeof(sig), out, &n, sizeof(out)));

    TEST_ASSERT_EQUAL(SSH_MSG_KEXDH_REPLY, out[0]);
    uint32_t kslen = ((uint32_t)out[1] << 24) | ((uint32_t)out[2] << 16) | ((uint32_t)out[3] << 8) | out[4];
    TEST_ASSERT_EQUAL_UINT32(12, kslen);
    TEST_ASSERT_EQUAL_MEMORY(ks, out + 5, 12);

    bool alg = false;
    for (size_t k = 0; k + 12 <= n; k++)
        if (memcmp(out + k, "rsa-sha2-256", 12) == 0)
        {
            alg = true;
            break;
        }
    TEST_ASSERT_TRUE(alg);

    bool found_sig = false;
    for (size_t k = 0; k + 256 <= n; k++)
        if (memcmp(out + k, sig, 256) == 0)
        {
            found_sig = true;
            break;
        }
    TEST_ASSERT_TRUE(found_sig);
}

// ---- KEXDH orchestration (compute K, sign H, reply, derive keys) ----------

static void setup_rsa_fixture()
{
    memset(_test_rsa_n, 0, 256);
    _test_rsa_n[0] = 0xFF; // 2048-bit modulus, larger than any PKCS#1 em
    _test_rsa_n[255] = 0x01;
    memset(_test_rsa_d, 0, 256);
    _test_rsa_d[255] = 0x01; // d = 1 (native test stub)
    _test_rsa_e[0] = 0x00;
    _test_rsa_e[1] = 0x01;
    _test_rsa_e[2] = 0x00;
    _test_rsa_e[3] = 0x01; // e = 65537
    ssh_rsa_load_pubkey();
}

void test_kexdh_handle_produces_reply_and_installs_keys()
{
    ssh_transport_init(0);
    setup_rsa_fixture();

    SshSession *s = &ssh_sess[0];
    const char *vc = "SSH-2.0-TestClient";
    strcpy(s->v_c, vc);
    s->v_c_len = (uint16_t)strlen(vc);
    for (int j = 0; j < 30; j++)
    {
        s->i_c[j] = (uint8_t)(j + 1);
        s->i_s[j] = (uint8_t)(j + 100);
    }
    s->i_c_len = 30;
    s->i_s_len = 30;

    // Generate the server ephemeral (y, f). One 2048-bit exponentiation.
    TEST_ASSERT_EQUAL_INT(0, ssh_dh_generate(0));

    // Client KEXDH_INIT with e = 2 (a valid public value: 1 < 2 < p-1).
    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x02;
    uint8_t pkt[300];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    size_t n = 1 + put_mpint(pkt + 1, e_be, 256);

    uint8_t reply[1024];
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_kexdh_handle(0, pkt, n, reply, &rlen, sizeof(reply)));

    TEST_ASSERT_EQUAL(SSH_MSG_KEXDH_REPLY, reply[0]);
    TEST_ASSERT_TRUE(ssh_sess[0].have_session_id);
    TEST_ASSERT_EQUAL(SSH_PHASE_NEWKEYS, ssh_sess[0].phase);
    TEST_ASSERT_TRUE(ssh_keys[0].active);

    bool alg = false;
    for (size_t k = 0; k + 12 <= rlen; k++)
        if (memcmp(reply + k, "rsa-sha2-256", 12) == 0)
        {
            alg = true;
            break;
        }
    TEST_ASSERT_TRUE(alg);

    // NEWKEYS activates encryption and advances to the service phase.
    ssh_newkeys_complete(0);
    TEST_ASSERT_TRUE(ssh_pkt[0].encrypted);
    TEST_ASSERT_EQUAL(SSH_PHASE_SERVICE, ssh_sess[0].phase);
}

void test_kexdh_handle_rejects_invalid_e()
{
    ssh_transport_init(0);
    setup_rsa_fixture();
    ssh_sess[0].v_c_len = 0;
    ssh_sess[0].i_c_len = 0;
    ssh_sess[0].i_s_len = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_dh_generate(0));

    // e = 1 is invalid (must be > 1).
    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x01;
    uint8_t pkt[300];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    size_t n = 1 + put_mpint(pkt + 1, e_be, 256);

    uint8_t reply[1024];
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_handle(0, pkt, n, reply, &rlen, sizeof(reply)));
}

// ---- rekey (RFC 4253 §9) --------------------------------------------------

void test_derive_keys_session_id_affects_output()
{
    uint8_t K[256];
    memset(K, 0, sizeof(K));
    K[255] = 7;
    uint8_t H[32], sid[32];
    for (int j = 0; j < 32; j++)
    {
        H[j] = (uint8_t)j;
        sid[j] = (uint8_t)(0xF0 - j); // distinct from H
    }

    ssh_dh_derive_keys_sid(0, K, H, H);
    uint8_t a[32];
    memcpy(a, ssh_keys[0].mac_key_c2s, 32);

    ssh_dh_derive_keys_sid(0, K, H, sid);
    TEST_ASSERT_NOT_EQUAL(0, memcmp(a, ssh_keys[0].mac_key_c2s, 32));

    // Deterministic: same inputs reproduce the same key.
    ssh_dh_derive_keys_sid(0, K, H, H);
    TEST_ASSERT_EQUAL_MEMORY(a, ssh_keys[0].mac_key_c2s, 32);
}

void test_rekey_needed_threshold()
{
    ssh_transport_init(0);
    ssh_pkt[0].seq_no_send = 0;
    ssh_pkt[0].seq_no_recv = 0;
    TEST_ASSERT_FALSE(ssh_rekey_needed(0));
    ssh_pkt[0].seq_no_send = SSH_REKEY_PACKET_THRESHOLD;
    TEST_ASSERT_TRUE(ssh_rekey_needed(0));
}

void test_begin_rekey_preserves_session_and_auth()
{
    ssh_transport_init(0);
    setup_rsa_fixture();
    ssh_sess[0].have_session_id = true;
    for (int j = 0; j < 32; j++)
        ssh_sess[0].session_id[j] = (uint8_t)j;
    ssh_sess[0].authed = true;
    TEST_ASSERT_EQUAL_INT(0, ssh_dh_generate(0));

    uint8_t out[1024];
    size_t n = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_transport_begin_rekey(0, out, &n, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_KEXINIT, out[0]);
    TEST_ASSERT_EQUAL(SSH_PHASE_KEXINIT, ssh_sess[0].phase);
    TEST_ASSERT_TRUE(ssh_sess[0].have_session_id);
    TEST_ASSERT_TRUE(ssh_sess[0].authed);

    // After the re-key completes, an authenticated connection resumes OPEN.
    ssh_newkeys_complete(0);
    TEST_ASSERT_EQUAL(SSH_PHASE_OPEN, ssh_sess[0].phase);
}

// Build a KEXINIT with eight independently-chosen algorithm name-lists.
static size_t build_kexinit8(uint8_t *out, const char *const n[8])
{
    size_t o = 0;
    out[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        out[o++] = (uint8_t)j; // cookie
    for (int j = 0; j < 8; j++)
        o += put_namelist(out + o, n[j]);
    o += put_namelist(out + o, ""); // lang c2s
    o += put_namelist(out + o, ""); // lang s2c
    out[o++] = 0;                   // first_kex_packet_follows
    for (int j = 0; j < 4; j++)
        out[o++] = 0; // reserved
    return o;
}

// Every transport entry point rejects an out-of-range slot.
void test_transport_index_guards()
{
    uint8_t out[512];
    size_t olen = 0, consumed = 0;
    ssh_transport_init(MAX_SSH_CONNS); // no-op, no crash
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_recv_banner(MAX_SSH_CONNS, (const uint8_t *)"x", 1, &consumed));
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_build(MAX_SSH_CONNS, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(MAX_SSH_CONNS, out, 20));
    uint8_t h[SSH_SHA256_DIGEST_LEN];
    TEST_ASSERT_EQUAL_INT(-1, ssh_kex_exchange_hash(MAX_SSH_CONNS, nullptr, nullptr, nullptr, nullptr, 0, h));
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_handle(MAX_SSH_CONNS, out, 10, out, &olen, sizeof(out)));
    ssh_newkeys_complete(MAX_SSH_CONNS); // no-op
    TEST_ASSERT_FALSE(ssh_rekey_needed(MAX_SSH_CONNS));
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_begin_rekey(MAX_SSH_CONNS, out, &olen, sizeof(out)));
}

// Banner and builder cap checks, and a banner line that exceeds the maximum.
void test_banner_and_build_caps()
{
    uint8_t small[4];
    size_t l = 0, consumed = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_server_banner(small, &l, 4)); // banner does not fit

    // An "SSH-" line of exactly SSH_VERSION_MAX chars, then LF: too long.
    static uint8_t long_ssh[SSH_VERSION_MAX + 2];
    memcpy(long_ssh, "SSH-", 4);
    memset(long_ssh + 4, 'x', SSH_VERSION_MAX - 4);
    long_ssh[SSH_VERSION_MAX] = '\n';
    ssh_transport_init(0);
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_recv_banner(0, long_ssh, SSH_VERSION_MAX + 1, &consumed));

    // A line with no terminator that exceeds the maximum length.
    static uint8_t long_line[SSH_VERSION_MAX + 4];
    memset(long_line, 'y', sizeof(long_line));
    ssh_transport_init(0);
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_recv_banner(0, long_line, sizeof(long_line), &consumed));

    ssh_transport_init(0);
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_build(0, small, &l, 4)); // writer overflow
    TEST_ASSERT_EQUAL_INT(-1, ssh_extinfo_build(small, &l, 4));    // writer overflow
    uint8_t ks[8] = {0}, f[256] = {0}, sig[8] = {0};
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_build_reply(ks, sizeof(ks), f, sig, sizeof(sig), small, &l, 4));
    TEST_ASSERT_EQUAL_INT(-1, ssh_transport_begin_rekey(0, small, &l, 4)); // inner kexinit_build overflow
}

// KEXINIT parsing rejects an unusable name-list at every negotiated field, an
// over-long payload, and truncated / over-claimed name-list fields.
void test_kexinit_parse_field_and_trunc()
{
    ssh_transport_init(0);
    uint8_t buf[3000];
    const char *K = "diffie-hellman-group14-sha256", *H = "rsa-sha2-256", *C = "aes256-ctr", *M = "hmac-sha2-256",
               *P = "none";

    const char *rows[6][8] = {
        {K, "", C, C, M, M, P, P}, // host-key
        {K, H, C, "", M, M, P, P}, // encryption s2c
        {K, H, C, C, "", M, P, P}, // mac c2s
        {K, H, C, C, M, "", P, P}, // mac s2c
        {K, H, C, C, M, M, "", P}, // compression c2s
        {K, H, C, C, M, M, P, ""}, // compression s2c
    };
    for (int r = 0; r < 6; r++)
    {
        size_t n = build_kexinit8(buf, rows[r]);
        TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, n));
    }

    // Over-long payload (> SSH_KEXINIT_MAX).
    memset(buf, 0, sizeof(buf));
    buf[0] = SSH_MSG_KEXINIT;
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, SSH_KEXINIT_MAX + 8));

    // Truncated mid-field: the host-key name-list length header runs past the buffer.
    const char *good[8] = {K, H, C, C, M, M, P, P};
    size_t full = build_kexinit8(buf, good);
    size_t f2 = 1 + 16 + (4 + strlen(K)); // start of the host-key field
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, f2 + 2));

    // Over-claimed field length: host-key name-list claims ~4 GiB.
    build_kexinit8(buf, good);
    buf[f2] = buf[f2 + 1] = buf[f2 + 2] = buf[f2 + 3] = 0xFF;
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, full));
}

// KEXDH parsing rejects a wrong type, an mpint that runs past the payload, and a
// payload that fails to parse in the full handler.
void test_kexdh_parse_and_handle_errors()
{
    uint8_t e_be[256];
    uint8_t bad[8] = {99, 0, 0, 0, 1, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_parse_init(bad, 8, e_be)); // wrong type
    uint8_t over[8] = {SSH_MSG_KEXDH_INIT, 0, 0, 0, 100, 1, 2, 3}; // e claims 100 bytes, 3 present
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_parse_init(over, 8, e_be));

    uint8_t out[512];
    size_t olen = 0;
    ssh_transport_init(0);
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_handle(0, bad, 8, out, &olen, sizeof(out))); // parse failure

    // A full, valid handshake whose reply does not fit the output buffer fails closed
    // after the DH/exchange-hash/sign steps succeed.
    ssh_transport_init(0);
    setup_rsa_fixture();
    SshSession *s = &ssh_sess[0];
    strcpy(s->v_c, "SSH-2.0-TestClient");
    s->v_c_len = (uint16_t)strlen(s->v_c);
    for (int j = 0; j < 30; j++)
    {
        s->i_c[j] = (uint8_t)(j + 1);
        s->i_s[j] = (uint8_t)(j + 100);
    }
    s->i_c_len = 30;
    s->i_s_len = 30;
    TEST_ASSERT_EQUAL_INT(0, ssh_dh_generate(0));
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x02; // valid e
    uint8_t pkt[300];
    pkt[0] = SSH_MSG_KEXDH_INIT;
    size_t n = 1 + put_mpint(pkt + 1, e_be, 256);
    uint8_t reply[8];
    size_t rlen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexdh_handle(0, pkt, n, reply, &rlen, sizeof(reply)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_transport_index_guards);
    RUN_TEST(test_banner_and_build_caps);
    RUN_TEST(test_kexinit_parse_field_and_trunc);
    RUN_TEST(test_kexdh_parse_and_handle_errors);
    RUN_TEST(test_server_banner_format);
    RUN_TEST(test_recv_banner_complete);
    RUN_TEST(test_recv_banner_bare_lf);
    RUN_TEST(test_recv_banner_split_across_reads);
    RUN_TEST(test_recv_banner_skips_preamble_lines);
    RUN_TEST(test_kexinit_build_starts_with_msg_and_stores_is);
    RUN_TEST(test_kexinit_parse_accepts_supported);
    RUN_TEST(test_kexinit_parse_accepts_when_ours_listed_among_others);
    RUN_TEST(test_kexinit_parse_rejects_missing_kex);
    RUN_TEST(test_kexinit_parse_rejects_missing_cipher);
    RUN_TEST(test_kexinit_parse_rejects_truncated);
    RUN_TEST(test_exchange_hash_matches_independent_assembly);
    RUN_TEST(test_exchange_hash_changes_with_input);
    RUN_TEST(test_kexdh_parse_init_extracts_e_with_padding);
    RUN_TEST(test_kexdh_parse_init_extracts_small_e);
    RUN_TEST(test_kexdh_parse_init_rejects_wrong_type);
    RUN_TEST(test_kexdh_parse_init_rejects_oversized_e);
    RUN_TEST(test_kexdh_build_reply_structure);
    RUN_TEST(test_kexdh_handle_produces_reply_and_installs_keys);
    RUN_TEST(test_kexdh_handle_rejects_invalid_e);
    RUN_TEST(test_derive_keys_session_id_affects_output);
    RUN_TEST(test_rekey_needed_threshold);
    RUN_TEST(test_begin_rekey_preserves_session_and_auth);
    return UNITY_END();
}
