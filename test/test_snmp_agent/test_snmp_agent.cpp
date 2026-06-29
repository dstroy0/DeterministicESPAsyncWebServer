// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SNMP v1/v2c agent core (snmp_agent_process). Each test
// builds a real request datagram with the BER encoder, runs it through the
// agent, and decodes the response - no sockets, no heap.

#include "services/snmp/snmp_agent.h"
#include "services/snmp/snmp_ber.h"
#include <string.h>
#include <unity.h>

// ---------------------------------------------------------------------------
// Test MIB
// ---------------------------------------------------------------------------

static const uint32_t OID_SYSDESCR[] = {1, 3, 6, 1, 2, 1, 1, 1, 0};
static const uint32_t OID_SYSUPTIME[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
static const uint32_t OID_SYSPREFIX[] = {1, 3, 6, 1, 2, 1, 1}; // for GetNext
static const uint32_t OID_RW[] = {1, 3, 6, 1, 4, 1, 49374, 1, 0};
static const uint32_t OID_RO[] = {1, 3, 6, 1, 4, 1, 49374, 2, 0};
static const uint32_t OID_CTR[] = {1, 3, 6, 1, 4, 1, 49374, 3, 0};
static const uint32_t OID_PAST_END[] = {1, 3, 6, 1, 4, 1, 49374, 99, 0};
static const uint32_t OID_UNKNOWN[] = {1, 3, 6, 1, 2, 1, 99, 0};
// sysDescr exists as object 1.3.6.1.2.1.1.1 but only instance .0; ask for .5.
static const uint32_t OID_SYSDESCR_BADINST[] = {1, 3, 6, 1, 2, 1, 1, 1, 5};

static const char *SYSDESCR_VAL = "DetWS test agent";

static bool g_set_called = false;
static long g_set_value = 0;

static bool rw_setter(const SnmpValue *in)
{
    g_set_called = true;
    if (in->type != BER_INTEGER)
        return false;
    g_set_value = in->ival;
    return true;
}

static bool ctr_getter(SnmpValue *out)
{
    out->type = SNMP_COUNTER32;
    out->uval = 12345u;
    return true;
}

void setUp()
{
    snmp_agent_init("public");
    snmp_agent_set_rw_community("private");
    snmp_agent_set_system(SYSDESCR_VAL, "admin", "esp32", "lab", 72);
    snmp_agent_add_integer(OID_RW, 9, 42, rw_setter);
    snmp_agent_add_integer(OID_RO, 9, 7); // read-only (no setter)
    snmp_agent_add_dynamic(OID_CTR, 9, SNMP_COUNTER32, ctr_getter);
    g_set_called = false;
    g_set_value = 0;
}

void tearDown()
{
}

// ---------------------------------------------------------------------------
// Request builder / response parser
// ---------------------------------------------------------------------------

static size_t build_req(uint8_t *buf, size_t cap, long version, const char *comm, uint8_t pdu, long reqid, long f2,
                        long f3, const uint32_t *oid, size_t oidn, const SnmpValue *setval)
{
    BerEnc e;
    ber_enc_init(&e, buf, cap);
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, version);
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)comm, strlen(comm));
    size_t pdus = ber_seq_begin(&e, pdu);
    ber_put_integer(&e, reqid);
    ber_put_integer(&e, f2);
    ber_put_integer(&e, f3);
    size_t vbl = ber_seq_begin(&e, BER_SEQUENCE);
    size_t vb = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_oid(&e, oid, oidn);
    if (setval && setval->type == BER_INTEGER)
        ber_put_integer(&e, setval->ival);
    else if (setval && setval->type == BER_OCTET_STRING)
        ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)setval->str, setval->str_len);
    else
        ber_put_null(&e);
    ber_seq_end(&e, vb);
    ber_seq_end(&e, vbl);
    ber_seq_end(&e, pdus);
    ber_seq_end(&e, msg);
    return e.ok ? e.len : 0;
}

struct RespView
{
    long version;
    uint8_t pdu_tag;
    long request_id;
    long err_status;
    long err_index;
    size_t nvb;
    uint32_t oid[SNMP_MAX_OID_LEN]; // first varbind
    size_t oid_len;
    uint8_t val_tag;
    long ival;
    uint32_t uval;
    char str[64];
    size_t str_len;
};

static bool parse_resp(const uint8_t *buf, size_t len, RespView *rv)
{
    memset(rv, 0, sizeof(*rv));
    BerDec d;
    ber_dec_init(&d, buf, len);
    uint8_t tag;
    size_t l;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE)
        return false;
    if (!ber_read_integer(&d, &rv->version))
        return false;
    uint8_t ctag;
    size_t cl;
    if (!ber_read_header(&d, &ctag, &cl))
        return false;
    d.pos += cl; // skip community
    if (!ber_read_header(&d, &rv->pdu_tag, &l))
        return false;
    if (!ber_read_integer(&d, &rv->request_id) || !ber_read_integer(&d, &rv->err_status) ||
        !ber_read_integer(&d, &rv->err_index))
        return false;
    uint8_t vt;
    size_t vl;
    if (!ber_read_header(&d, &vt, &vl) || vt != BER_SEQUENCE)
        return false;
    size_t vend = d.pos + vl;
    bool first = true;
    while (d.pos < vend && d.ok)
    {
        uint8_t st;
        size_t sl;
        if (!ber_read_header(&d, &st, &sl) || st != BER_SEQUENCE)
            return false;
        size_t vbend = d.pos + sl;
        uint32_t oid[SNMP_MAX_OID_LEN];
        size_t on;
        if (!ber_read_oid(&d, oid, SNMP_MAX_OID_LEN, &on))
            return false;
        size_t save = d.pos;
        uint8_t valtag;
        size_t vallen;
        if (!ber_read_header(&d, &valtag, &vallen))
            return false;
        if (first)
        {
            memcpy(rv->oid, oid, on * sizeof(uint32_t));
            rv->oid_len = on;
            rv->val_tag = valtag;
            if (valtag == BER_INTEGER)
            {
                d.pos = save;
                ber_read_integer(&d, &rv->ival);
            }
            else if (valtag == BER_OCTET_STRING)
            {
                size_t cpy = vallen < sizeof(rv->str) - 1 ? vallen : sizeof(rv->str) - 1;
                memcpy(rv->str, d.buf + d.pos, cpy);
                rv->str[cpy] = '\0';
                rv->str_len = vallen;
            }
            else if (valtag == SNMP_TIMETICKS || valtag == SNMP_COUNTER32 || valtag == SNMP_GAUGE32)
            {
                uint32_t a = 0;
                for (size_t i = 0; i < vallen; i++)
                    a = (a << 8) | d.buf[d.pos + i];
                rv->uval = a;
            }
            first = false;
        }
        d.pos = vbend;
        rv->nvb++;
    }
    return d.ok;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_get_string_v2c()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GET, 111, 0, 0, OID_SYSDESCR, 9, nullptr);
    TEST_ASSERT_TRUE(rl > 0);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_RESPONSE, rv.pdu_tag);
    TEST_ASSERT_EQUAL_INT(111, rv.request_id);
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_HEX8(BER_OCTET_STRING, rv.val_tag);
    TEST_ASSERT_EQUAL_STRING(SYSDESCR_VAL, rv.str);
}

void test_get_unknown_v2c_exception()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GET, 7, 0, 0, OID_UNKNOWN, 8, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_HEX8(SNMP_NO_SUCH_OBJECT, rv.val_tag);
}

// RFC 3416 4.2.1: a known object (sysDescr) but a nonexistent instance (.5) must
// report noSuchInstance, distinct from the noSuchObject above for an unknown OID.
void test_get_bad_instance_v2c_nosuchinstance()
{
    uint8_t req[256], resp[256];
    size_t rl =
        build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GET, 8, 0, 0, OID_SYSDESCR_BADINST, 9, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_HEX8(SNMP_NO_SUCH_INSTANCE, rv.val_tag);
}

void test_get_unknown_v1_error()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V1, "public", SNMP_PDU_GET, 7, 0, 0, OID_UNKNOWN, 8, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_SUCH_NAME, rv.err_status);
    TEST_ASSERT_EQUAL_INT(1, rv.err_index);
}

void test_getnext_walks_to_first()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GETNEXT, 5, 0, 0, OID_SYSPREFIX, 7, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_HEX8(BER_OCTET_STRING, rv.val_tag);
    TEST_ASSERT_EQUAL_UINT(9, rv.oid_len);
    TEST_ASSERT_EQUAL_UINT32(1u, rv.oid[7]); // sysDescr index .1
    TEST_ASSERT_EQUAL_STRING(SYSDESCR_VAL, rv.str);
}

void test_getnext_past_end_endofmibview()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GETNEXT, 9, 0, 0, OID_PAST_END, 9, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_HEX8(SNMP_END_OF_MIB_VIEW, rv.val_tag);
}

void test_set_without_rw_community_denied()
{
    uint8_t req[256], resp[256];
    SnmpValue sv;
    memset(&sv, 0, sizeof(sv));
    sv.type = BER_INTEGER;
    sv.ival = 99;
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_SET, 3, 0, 0, OID_RW, 9, &sv);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ACCESS, rv.err_status);
    TEST_ASSERT_FALSE(g_set_called);
}

void test_set_with_rw_community_invokes_setter()
{
    uint8_t req[256], resp[256];
    SnmpValue sv;
    memset(&sv, 0, sizeof(sv));
    sv.type = BER_INTEGER;
    sv.ival = 99;
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "private", SNMP_PDU_SET, 3, 0, 0, OID_RW, 9, &sv);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_TRUE(g_set_called);
    TEST_ASSERT_EQUAL_INT(99, g_set_value);
}

void test_set_readonly_not_writable()
{
    uint8_t req[256], resp[256];
    SnmpValue sv;
    memset(&sv, 0, sizeof(sv));
    sv.type = BER_INTEGER;
    sv.ival = 1;
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "private", SNMP_PDU_SET, 3, 0, 0, OID_RO, 9, &sv);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NOT_WRITABLE, rv.err_status);
}

void test_getbulk_returns_multiple()
{
    uint8_t req[512], resp[512];
    // non-repeaters=0, max-repetitions=3, one repeater starting at the system prefix.
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GETBULK, 1, 0, 3, OID_SYSPREFIX, 7, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_INT(SNMP_ERR_NO_ERROR, rv.err_status);
    TEST_ASSERT_EQUAL_UINT(3, rv.nvb);
    TEST_ASSERT_EQUAL_HEX8(BER_OCTET_STRING, rv.val_tag); // first = sysDescr
}

void test_dynamic_counter_value()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GET, 2, 0, 0, OID_CTR, 9, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_HEX8(SNMP_COUNTER32, rv.val_tag);
    TEST_ASSERT_EQUAL_UINT32(12345u, rv.uval);
}

void test_uptime_is_timeticks()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "public", SNMP_PDU_GET, 2, 0, 0, OID_SYSUPTIME, 9, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    RespView rv;
    TEST_ASSERT_TRUE(parse_resp(resp, n, &rv));
    TEST_ASSERT_EQUAL_HEX8(SNMP_TIMETICKS, rv.val_tag);
}

void test_unknown_community_no_response()
{
    uint8_t req[256], resp[256];
    size_t rl = build_req(req, sizeof(req), SNMP_V2C, "wrongcomm", SNMP_PDU_GET, 1, 0, 0, OID_SYSDESCR, 9, nullptr);
    size_t n = snmp_agent_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_EQUAL_UINT(0, n);
}

void test_v3_message_dropped()
{
    uint8_t req[64];
    BerEnc e;
    ber_enc_init(&e, req, sizeof(req));
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, SNMP_V3);
    ber_seq_end(&e, msg);
    uint8_t resp[64];
    size_t n = snmp_agent_process(req, e.len, resp, sizeof(resp));
    TEST_ASSERT_EQUAL_UINT(0, n);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_get_string_v2c);
    RUN_TEST(test_get_unknown_v2c_exception);
    RUN_TEST(test_get_bad_instance_v2c_nosuchinstance);
    RUN_TEST(test_get_unknown_v1_error);
    RUN_TEST(test_getnext_walks_to_first);
    RUN_TEST(test_getnext_past_end_endofmibview);
    RUN_TEST(test_set_without_rw_community_denied);
    RUN_TEST(test_set_with_rw_community_invokes_setter);
    RUN_TEST(test_set_readonly_not_writable);
    RUN_TEST(test_getbulk_returns_multiple);
    RUN_TEST(test_dynamic_counter_value);
    RUN_TEST(test_uptime_is_timeticks);
    RUN_TEST(test_unknown_community_no_response);
    RUN_TEST(test_v3_message_dropped);
    return UNITY_END();
}
