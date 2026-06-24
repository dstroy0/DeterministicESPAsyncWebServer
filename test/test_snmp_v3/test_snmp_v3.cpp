// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SNMPv3 USM layer. The test acts as a full SNMP manager:
// it derives USM keys, builds authenticated/encrypted v3 messages, runs them
// through the agent, and verifies the authenticated/encrypted responses.
//
// Crypto is grounded against independent vectors: the SHA-256 key-localization
// expected value is computed with Python hashlib (RFC 3414 §2.6 / RFC 7860), and
// the AES-128 block cipher is checked against the FIPS-197 Appendix C.1 KAT.

#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
#include "services/snmp/snmp_agent.h"
#include "services/snmp/snmp_ber.h"
#include "services/snmp/snmp_crypto.h"
#include "services/snmp/snmp_v3.h"
#include <string.h>
#include <unity.h>

static const uint32_t OID_SYSDESCR[] = {1, 3, 6, 1, 2, 1, 1, 1, 0};
static const char *SYSDESCR_VAL = "DetWS v3 agent";

void setUp()
{
    snmp_agent_init("public");
    snmp_agent_set_system(SYSDESCR_VAL, "admin", "esp32", "lab", 72);
    snmp_v3_init(nullptr, 0); // built-in default engine ID
    snmp_v3_set_boots(1);
    snmp_v3_set_user("myuser", "authpass12", "privpass12");
}
void tearDown()
{
}

// ==================== crypto known-answer vectors ====================

void test_localize_key_sha256_vector()
{
    // password "maplesyrup", engineID 80 00 C0 DE 05 01 02 03 04 (the agent default).
    // Expected localized key computed independently with Python hashlib.
    const uint8_t engine[] = {0x80, 0x00, 0xC0, 0xDE, 0x05, 0x01, 0x02, 0x03, 0x04};
    const uint8_t expect[32] = {0xb8, 0xa5, 0x56, 0xfc, 0xb1, 0xcd, 0xee, 0x18, 0x79, 0x22, 0x24,
                                0x79, 0x69, 0xaf, 0xe3, 0x90, 0x37, 0x34, 0x07, 0xd6, 0x52, 0x9a,
                                0x98, 0x08, 0x41, 0x8a, 0xef, 0x8e, 0xb0, 0xbf, 0x45, 0x86};
    uint8_t key[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("maplesyrup", engine, sizeof(engine), key);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, key, 32);
}

void test_aes128_fips197_vector()
{
    // FIPS-197 C.1. CFB with IV = plaintext and zero input yields E_key(plaintext).
    const uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                             0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    const uint8_t pt[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    const uint8_t expect[16] = {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                                0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
    uint8_t zero[16] = {0};
    uint8_t out[16];
    snmp_aes128_cfb(key, pt, zero, out, 16, true); // out = 0 XOR E(IV=pt) = E(pt)
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 16);
}

void test_aes_cfb_roundtrip_partial_block()
{
    const uint8_t key[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    const uint8_t iv[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    const char *msg = "SNMPv3 scopedPDU spanning more than one AES block plus a tail.";
    size_t n = strlen(msg);
    uint8_t ct[128], pt[128];
    snmp_aes128_cfb(key, iv, (const uint8_t *)msg, ct, n, true);
    snmp_aes128_cfb(key, iv, ct, pt, n, false);
    TEST_ASSERT_EQUAL_MEMORY(msg, pt, n);
    TEST_ASSERT_TRUE(memcmp(msg, ct, n) != 0); // actually encrypted
}

// ==================== v3 client (manager) helpers ====================

static uint8_t g_dec[1024]; // scratch for response decryption

struct V3View
{
    uint8_t engine_id[64];
    size_t engine_id_len;
    long boots, time;
    uint8_t flags;
    uint8_t pdu_tag;
    long request_id, err_status;
    uint32_t oid[SNMP_MAX_OID_LEN];
    size_t oid_len;
    uint8_t val_tag;
    char str[64];
    size_t str_len;
};

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}

// Build a v3 GET request for a single OID.
static size_t build_get(uint8_t *out, size_t cap, bool auth, bool priv, const uint8_t *eid, size_t eid_len, long boots,
                        long time, const char *user, const uint8_t *authkey, const uint8_t *privkey, long msg_id,
                        long req_id, const uint32_t *oid, size_t oid_len)
{
    // inner GET PDU
    uint8_t pdu[256];
    BerEnc pe;
    ber_enc_init(&pe, pdu, sizeof(pdu));
    size_t pp = ber_seq_begin(&pe, SNMP_PDU_GET);
    ber_put_integer(&pe, req_id);
    ber_put_integer(&pe, 0);
    ber_put_integer(&pe, 0);
    size_t vbl = ber_seq_begin(&pe, BER_SEQUENCE);
    size_t vb = ber_seq_begin(&pe, BER_SEQUENCE);
    ber_put_oid(&pe, oid, oid_len);
    ber_put_null(&pe);
    ber_seq_end(&pe, vb);
    ber_seq_end(&pe, vbl);
    ber_seq_end(&pe, pp);

    // scopedPDU
    uint8_t scoped[320];
    BerEnc se;
    ber_enc_init(&se, scoped, sizeof(scoped));
    size_t ss = ber_seq_begin(&se, BER_SEQUENCE);
    ber_put_octet_string(&se, BER_OCTET_STRING, eid, eid_len);
    ber_put_octet_string(&se, BER_OCTET_STRING, nullptr, 0);
    ber_put_raw(&se, pdu, pe.len);
    ber_seq_end(&se, ss);

    // optional privacy
    uint8_t salt[8] = {0, 0, 0, 0, 0, 0, 0, 7};
    uint8_t cipher[320];
    const uint8_t *data = scoped;
    size_t data_len = se.len;
    if (priv)
    {
        uint8_t iv[16];
        put_be32(iv, (uint32_t)boots);
        put_be32(iv + 4, (uint32_t)time);
        memcpy(iv + 8, salt, 8);
        snmp_aes128_cfb(privkey, iv, scoped, cipher, se.len, true);
        data = cipher;
    }

    // secparams
    uint8_t secp[128];
    BerEnc se2;
    ber_enc_init(&se2, secp, sizeof(secp));
    size_t s2 = ber_seq_begin(&se2, BER_SEQUENCE);
    ber_put_octet_string(&se2, BER_OCTET_STRING, eid, eid_len);
    ber_put_integer(&se2, boots);
    ber_put_integer(&se2, time);
    ber_put_octet_string(&se2, BER_OCTET_STRING, (const uint8_t *)user, strlen(user));
    size_t auth_off = 0;
    if (auth)
    {
        auth_off = se2.len + 2;
        uint8_t z[SNMP_V3_AUTH_PARAM_LEN] = {0};
        ber_put_octet_string(&se2, BER_OCTET_STRING, z, SNMP_V3_AUTH_PARAM_LEN);
    }
    else
        ber_put_octet_string(&se2, BER_OCTET_STRING, nullptr, 0);
    if (priv)
        ber_put_octet_string(&se2, BER_OCTET_STRING, salt, 8);
    else
        ber_put_octet_string(&se2, BER_OCTET_STRING, nullptr, 0);
    ber_seq_end(&se2, s2);

    // message
    BerEnc e;
    ber_enc_init(&e, out, cap);
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, SNMP_V3);
    size_t hdr = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, msg_id);
    ber_put_integer(&e, 65507);
    uint8_t fl = (uint8_t)((auth ? 1 : 0) | (priv ? 2 : 0) | (auth ? 0 : 4)); // reportable on discovery
    ber_put_octet_string(&e, BER_OCTET_STRING, &fl, 1);
    ber_put_integer(&e, 3);
    ber_seq_end(&e, hdr);
    ber_put_octet_string(&e, BER_OCTET_STRING, secp, se2.len);
    size_t sec_value_pos = e.len - se2.len;
    if (priv)
        ber_put_octet_string(&e, BER_OCTET_STRING, data, data_len);
    else
        ber_put_raw(&e, data, data_len);
    ber_seq_end(&e, msg);

    if (auth)
    {
        uint8_t mac[SSH_HMAC_SHA256_LEN];
        ssh_hmac_sha256(authkey, SNMP_USM_KEY_LEN, out, e.len, mac);
        memcpy(out + sec_value_pos + auth_off, mac, SNMP_V3_AUTH_PARAM_LEN);
    }
    return e.ok ? e.len : 0;
}

static bool parse_v3(const uint8_t *buf, size_t len, const uint8_t *privkey, V3View *v)
{
    memset(v, 0, sizeof(*v));
    BerDec d;
    ber_dec_init(&d, buf, len);
    uint8_t tag;
    size_t l;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE)
        return false;
    long version;
    if (!ber_read_integer(&d, &version))
        return false;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE) // global data
        return false;
    long msgid, maxsize, secmodel;
    if (!ber_read_integer(&d, &msgid) || !ber_read_integer(&d, &maxsize))
        return false;
    size_t fl;
    if (!ber_read_header(&d, &tag, &fl) || tag != BER_OCTET_STRING)
        return false;
    v->flags = d.buf[d.pos];
    d.pos += fl;
    if (!ber_read_integer(&d, &secmodel))
        return false;
    // secparams
    size_t seclen;
    if (!ber_read_header(&d, &tag, &seclen) || tag != BER_OCTET_STRING)
        return false;
    BerDec sd;
    ber_dec_init(&sd, d.buf + d.pos, seclen);
    d.pos += seclen;
    if (!ber_read_header(&sd, &tag, &l) || tag != BER_SEQUENCE)
        return false;
    size_t eidl;
    if (!ber_read_header(&sd, &tag, &eidl) || tag != BER_OCTET_STRING)
        return false;
    memcpy(v->engine_id, sd.buf + sd.pos, eidl < sizeof(v->engine_id) ? eidl : sizeof(v->engine_id));
    v->engine_id_len = eidl;
    sd.pos += eidl;
    if (!ber_read_integer(&sd, &v->boots) || !ber_read_integer(&sd, &v->time))
        return false;
    size_t ul;
    if (!ber_read_header(&sd, &tag, &ul) || tag != BER_OCTET_STRING) // user
        return false;
    sd.pos += ul;
    size_t al;
    if (!ber_read_header(&sd, &tag, &al) || tag != BER_OCTET_STRING) // authparams
        return false;
    sd.pos += al;
    size_t pl;
    if (!ber_read_header(&sd, &tag, &pl) || tag != BER_OCTET_STRING) // privparams
        return false;
    const uint8_t *privparm = sd.buf + sd.pos;

    const uint8_t *scoped;
    size_t scoped_len;
    if (v->flags & 0x02)
    {
        size_t ctl;
        if (!ber_read_header(&d, &tag, &ctl) || tag != BER_OCTET_STRING)
            return false;
        uint8_t iv[16];
        put_be32(iv, (uint32_t)v->boots);
        put_be32(iv + 4, (uint32_t)v->time);
        memcpy(iv + 8, privparm, 8);
        snmp_aes128_cfb(privkey, iv, d.buf + d.pos, g_dec, ctl, false);
        scoped = g_dec;
        scoped_len = ctl;
    }
    else
    {
        scoped = d.buf + d.pos;
        scoped_len = len - d.pos;
    }

    // scopedPDU -> inner PDU
    BerDec cd;
    ber_dec_init(&cd, scoped, scoped_len);
    if (!ber_read_header(&cd, &tag, &l) || tag != BER_SEQUENCE)
        return false;
    if (!ber_read_header(&cd, &tag, &l) || tag != BER_OCTET_STRING) // ctxEngineID
        return false;
    cd.pos += l;
    if (!ber_read_header(&cd, &tag, &l) || tag != BER_OCTET_STRING) // ctxName
        return false;
    cd.pos += l;
    if (!ber_read_header(&cd, &v->pdu_tag, &l)) // PDU
        return false;
    if (!ber_read_integer(&cd, &v->request_id) || !ber_read_integer(&cd, &v->err_status))
        return false;
    long erridx;
    if (!ber_read_integer(&cd, &erridx))
        return false;
    if (!ber_read_header(&cd, &tag, &l) || tag != BER_SEQUENCE) // varbind list
        return false;
    if (cd.pos < cd.len)
    {
        if (!ber_read_header(&cd, &tag, &l) || tag != BER_SEQUENCE) // first varbind
            return false;
        if (!ber_read_oid(&cd, v->oid, SNMP_MAX_OID_LEN, &v->oid_len))
            return false;
        size_t vl;
        if (!ber_read_header(&cd, &v->val_tag, &vl))
            return false;
        if (v->val_tag == BER_OCTET_STRING)
        {
            size_t c = vl < sizeof(v->str) - 1 ? vl : sizeof(v->str) - 1;
            memcpy(v->str, cd.buf + cd.pos, c);
            v->str[c] = '\0';
            v->str_len = vl;
        }
    }
    return cd.ok;
}

// Discover the agent's authoritative engineID / boots / time via a probe.
static void discover(V3View *v)
{
    uint8_t req[256], resp[512];
    uint32_t empty_oid[] = {1, 3, 6, 1, 2, 1, 1, 1, 0};
    size_t rl = build_get(req, sizeof(req), false, false, nullptr, 0, 0, 0, "", nullptr, nullptr, 100, 1, empty_oid, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(parse_v3(resp, n, nullptr, v));
}

// ==================== v3 message-flow tests ====================

void test_discovery_reports_engine_id()
{
    V3View v;
    discover(&v);
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_REPORT, v.pdu_tag);
    TEST_ASSERT_TRUE(v.engine_id_len >= 5); // a real authoritative engine ID
    // usmStatsUnknownEngineIDs = 1.3.6.1.6.3.15.1.1.4.0
    TEST_ASSERT_EQUAL_UINT(11, v.oid_len);
    TEST_ASSERT_EQUAL_UINT32(15u, v.oid[6]);
    TEST_ASSERT_EQUAL_UINT32(4u, v.oid[9]);
}

void test_authnopriv_get()
{
    V3View disc;
    discover(&disc);
    uint8_t authkey[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("authpass12", disc.engine_id, disc.engine_id_len, authkey);

    uint8_t req[300], resp[512];
    size_t rl = build_get(req, sizeof(req), true, false, disc.engine_id, disc.engine_id_len, disc.boots, disc.time,
                          "myuser", authkey, nullptr, 200, 42, OID_SYSDESCR, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    V3View v;
    TEST_ASSERT_TRUE(parse_v3(resp, n, nullptr, &v));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_RESPONSE, v.pdu_tag);
    TEST_ASSERT_EQUAL_INT(42, v.request_id);
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, v.err_status);
    TEST_ASSERT_EQUAL_HEX8(BER_OCTET_STRING, v.val_tag);
    TEST_ASSERT_EQUAL_STRING(SYSDESCR_VAL, v.str);
    TEST_ASSERT_EQUAL_UINT8(0x01, v.flags & 0x03); // response is authNoPriv
}

void test_authpriv_get()
{
    V3View disc;
    discover(&disc);
    uint8_t authkey[SNMP_USM_KEY_LEN], privkey[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("authpass12", disc.engine_id, disc.engine_id_len, authkey);
    snmp_usm_localize_key("privpass12", disc.engine_id, disc.engine_id_len, privkey);

    uint8_t req[300], resp[512];
    size_t rl = build_get(req, sizeof(req), true, true, disc.engine_id, disc.engine_id_len, disc.boots, disc.time,
                          "myuser", authkey, privkey, 201, 77, OID_SYSDESCR, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    V3View v;
    TEST_ASSERT_TRUE(parse_v3(resp, n, privkey, &v));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_RESPONSE, v.pdu_tag);
    TEST_ASSERT_EQUAL_INT(77, v.request_id);
    TEST_ASSERT_EQUAL_STRING(SYSDESCR_VAL, v.str);
    TEST_ASSERT_EQUAL_UINT8(0x03, v.flags & 0x03); // response is authPriv
}

void test_wrong_auth_password_reports_wrong_digest()
{
    V3View disc;
    discover(&disc);
    uint8_t badkey[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("wrongpass99", disc.engine_id, disc.engine_id_len, badkey);

    uint8_t req[300], resp[512];
    size_t rl = build_get(req, sizeof(req), true, false, disc.engine_id, disc.engine_id_len, disc.boots, disc.time,
                          "myuser", badkey, nullptr, 202, 5, OID_SYSDESCR, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    V3View v;
    TEST_ASSERT_TRUE(parse_v3(resp, n, nullptr, &v));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_REPORT, v.pdu_tag);
    // usmStatsWrongDigests = ...15.1.1.5.0
    TEST_ASSERT_EQUAL_UINT32(5u, v.oid[9]);
}

void test_unknown_user_reports()
{
    V3View disc;
    discover(&disc);
    uint8_t authkey[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("authpass12", disc.engine_id, disc.engine_id_len, authkey);

    uint8_t req[300], resp[512];
    size_t rl = build_get(req, sizeof(req), true, false, disc.engine_id, disc.engine_id_len, disc.boots, disc.time,
                          "nobody", authkey, nullptr, 203, 9, OID_SYSDESCR, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    V3View v;
    TEST_ASSERT_TRUE(parse_v3(resp, n, nullptr, &v));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_REPORT, v.pdu_tag);
    TEST_ASSERT_EQUAL_UINT32(3u, v.oid[9]); // usmStatsUnknownUserNames
}

void test_not_in_time_window_reports()
{
    V3View disc;
    discover(&disc);
    uint8_t authkey[SNMP_USM_KEY_LEN];
    snmp_usm_localize_key("authpass12", disc.engine_id, disc.engine_id_len, authkey);

    // Send a far-future engineTime to fall outside the +/-150s window.
    uint8_t req[300], resp[512];
    size_t rl = build_get(req, sizeof(req), true, false, disc.engine_id, disc.engine_id_len, disc.boots,
                          disc.time + 100000, "myuser", authkey, nullptr, 204, 11, OID_SYSDESCR, 9);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    V3View v;
    TEST_ASSERT_TRUE(parse_v3(resp, n, nullptr, &v));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_REPORT, v.pdu_tag);
    TEST_ASSERT_EQUAL_UINT32(2u, v.oid[9]);        // usmStatsNotInTimeWindows
    TEST_ASSERT_EQUAL_UINT8(0x01, v.flags & 0x03); // time reports are authenticated
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_localize_key_sha256_vector);
    RUN_TEST(test_aes128_fips197_vector);
    RUN_TEST(test_aes_cfb_roundtrip_partial_block);
    RUN_TEST(test_discovery_reports_engine_id);
    RUN_TEST(test_authnopriv_get);
    RUN_TEST(test_authpriv_get);
    RUN_TEST(test_wrong_auth_password_reports_wrong_digest);
    RUN_TEST(test_unknown_user_reports);
    RUN_TEST(test_not_in_time_window_reports);
    return UNITY_END();
}
