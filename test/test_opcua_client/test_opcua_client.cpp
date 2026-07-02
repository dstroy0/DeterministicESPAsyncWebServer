// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Round-trip tests for the OPC UA client (services/opcua_client): the client builds
// each request, the server (services/opcua) parses it and builds its response, and
// the client parses that response. This proves full client<->server wire interop
// in-process - the same exchange the board runs over TCP.

#include "services/opcua/opcua.h"
#include "services/opcua_client/opcua_client.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Browse resolver used by the server side of the round trip.
static int32_t srv_browse(uint16_t ns, uint32_t id, OpcUaReference *out, uint32_t max)
{
    if (ns == 0 && id == 85 && max >= 2)
    {
        const char *names[2] = {"Uptime", "Temperature"};
        for (int i = 0; i < 2; i++)
        {
            out[i].ref_type_id = OPCUA_REFTYPE_ORGANIZES;
            out[i].is_forward = true;
            out[i].target_ns = 1;
            out[i].target_id = i + 1;
            out[i].browse_name_ns = 1;
            out[i].browse_name = names[i];
            out[i].display_name = names[i];
            out[i].node_class = OPCUA_NODECLASS_VARIABLE;
            out[i].type_def_id = OPCUA_TYPEDEF_BASE_DATA_VARIABLE;
        }
        return 2;
    }
    return -1;
}

void test_hello_ack_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);

    uint8_t req[128];
    size_t rn = opcua_client_hello("opc.tcp://host:4840", req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaHello hello;
    TEST_ASSERT_TRUE(opcua_parse_hello(req, rn, &hello));
    TEST_ASSERT_EQUAL_UINT32(DETWS_OPCUA_BUF, hello.recv_buf_size);

    uint8_t ack[64];
    size_t an = opcua_build_ack(&hello, ack, sizeof(ack));
    TEST_ASSERT_TRUE(an > 0);

    OpcUaAckInfo info;
    TEST_ASSERT_TRUE(opcua_client_on_ack(ack, an, &info));
    TEST_ASSERT_EQUAL_UINT32(DETWS_OPCUA_BUF, info.recv_buf_size);
    TEST_ASSERT_EQUAL_UINT32(1, info.max_chunk_count);
}

void test_open_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);

    uint8_t req[256];
    size_t rn = opcua_client_open(&c, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaOpenChannel oc;
    TEST_ASSERT_TRUE(opcua_parse_open(req, rn, &oc));
    TEST_ASSERT_EQUAL_UINT32(1, oc.message_security_mode); // None

    uint8_t resp[256];
    size_t sn = opcua_build_open_response(&oc, 77, 88, 1, 0, 3600000, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    TEST_ASSERT_TRUE(opcua_client_on_open(&c, resp, sn));
    TEST_ASSERT_EQUAL_UINT32(77, c.channel_id);
    TEST_ASSERT_EQUAL_UINT32(88, c.token_id);
}

void test_session_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88; // pretend the channel is open

    // CreateSession
    uint8_t req[256];
    size_t rn = opcua_client_create_session(&c, "sess", "opc.tcp://host:4840", req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(req, rn, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CREATE_SESSION_REQ, m.type_id);

    uint8_t resp[512];
    OpcUaServerInfo si = {"opc.tcp://host:4840", "urn:test", "TestServer"};
    size_t sn = opcua_build_create_session_response(&m, 0x500, 0x600, 1200000.0, &si, 1, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);
    TEST_ASSERT_TRUE(opcua_client_on_create_session(&c, resp, sn));
    TEST_ASSERT_EQUAL_UINT32(0x600, c.session_auth_id); // AuthenticationToken captured
    TEST_ASSERT_EQUAL_UINT16(1, c.session_auth_ns);

    // ActivateSession (must now carry the session AuthenticationToken)
    rn = opcua_client_activate_session(&c, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);
    TEST_ASSERT_TRUE(opcua_parse_msg(req, rn, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_ACTIVATE_SESSION_REQ, m.type_id);
    sn = opcua_build_activate_session_response(&m, 2, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);
    TEST_ASSERT_TRUE(opcua_client_on_activate_session(resp, sn));
}

void test_read_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88;

    OpcUaReadItem items[2] = {{1, 1, true, OPCUA_ATTR_VALUE}, {1, 2, true, OPCUA_ATTR_VALUE}};
    uint8_t req[256];
    size_t rn = opcua_client_read(&c, items, 2, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaReadRequest rr;
    TEST_ASSERT_TRUE(opcua_parse_read(req, rn, &rr));
    TEST_ASSERT_EQUAL_UINT32(2, rr.count);
    TEST_ASSERT_EQUAL_UINT32(1, rr.items[0].id);
    TEST_ASSERT_EQUAL_UINT32(2, rr.items[1].id);

    OpcUaVariant svals[2];
    uint32_t ssts[2];
    memset(svals, 0, sizeof(svals));
    svals[0].type = OPCUA_VAR_UINT32;
    svals[0].u32 = 4242;
    ssts[0] = OPCUA_STATUS_GOOD;
    svals[1].type = OPCUA_VAR_DOUBLE;
    svals[1].f64 = 2.5;
    ssts[1] = OPCUA_STATUS_GOOD;

    uint8_t resp[256];
    size_t sn = opcua_build_read_response(&rr, svals, ssts, 3, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaVariant cvals[2];
    uint32_t csts[2];
    int32_t n = opcua_client_on_read(resp, sn, cvals, csts, 2);
    TEST_ASSERT_EQUAL_INT32(2, n);
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_UINT32, cvals[0].type);
    TEST_ASSERT_EQUAL_UINT32(4242, cvals[0].u32);
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_DOUBLE, cvals[1].type);
    TEST_ASSERT_TRUE(cvals[1].f64 == 2.5);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_STATUS_GOOD, csts[0]);
}

void test_browse_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88;

    uint8_t req[256];
    size_t rn = opcua_client_browse(&c, 0, 85, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaBrowseRequest br;
    TEST_ASSERT_TRUE(opcua_parse_browse(req, rn, &br));
    TEST_ASSERT_EQUAL_UINT32(1, br.count);
    TEST_ASSERT_EQUAL_UINT32(85, br.items[0].id);

    uint8_t resp[512];
    size_t sn = opcua_build_browse_response(&br, srv_browse, 4, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaClientRef refs[4];
    int32_t n = opcua_client_on_browse(resp, sn, refs, 4);
    TEST_ASSERT_EQUAL_INT32(2, n);
    TEST_ASSERT_EQUAL_STRING("Uptime", refs[0].browse_name);
    TEST_ASSERT_EQUAL_UINT32(1, refs[0].target_id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_VARIABLE, refs[0].node_class);
    TEST_ASSERT_EQUAL_STRING("Temperature", refs[1].browse_name);
    TEST_ASSERT_EQUAL_UINT32(2, refs[1].target_id);
}

void test_get_endpoints_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88;

    uint8_t req[256];
    size_t rn = opcua_client_get_endpoints(&c, "opc.tcp://host:4840", req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(req, rn, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_GET_ENDPOINTS_REQ, m.type_id);

    OpcUaServerInfo si = {"opc.tcp://host:4840", "urn:test", "TestServer"};
    uint8_t resp[512];
    size_t sn = opcua_build_get_endpoints_response(&m, &si, 7, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    int32_t ep = opcua_client_on_get_endpoints(resp, sn);
    TEST_ASSERT_EQUAL_INT32(1, ep);
}

void test_service_fault_rejected_by_parsers()
{
    // An unknown service draws a ServiceFault; a typed parser must reject it (wrong TypeId).
    OpcUaMsg m;
    memset(&m, 0, sizeof(m));
    m.token_id = 88;
    m.request_id = 9;
    m.request_handle = 3;
    uint8_t resp[64];
    size_t sn = opcua_build_service_fault(&m, OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, 1, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaVariant vals[1];
    uint32_t sts[1];
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_read(resp, sn, vals, sts, 1));
    TEST_ASSERT_FALSE(opcua_client_on_activate_session(resp, sn));
}

void test_write_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88;

    OpcUaWriteItem items[1];
    memset(items, 0, sizeof(items));
    items[0].ns = 1;
    items[0].id = 10;
    items[0].numeric = true;
    items[0].attribute = OPCUA_ATTR_VALUE;
    items[0].value.type = OPCUA_VAR_UINT32;
    items[0].value.u32 = 4242;

    uint8_t req[128];
    size_t rn = opcua_client_write(&c, items, 1, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaWriteRequest wr;
    TEST_ASSERT_TRUE(opcua_parse_write(req, rn, &wr));
    TEST_ASSERT_EQUAL_UINT32(1, wr.count);
    TEST_ASSERT_EQUAL_UINT32(10, wr.items[0].id);
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_UINT32, wr.items[0].value.type);
    TEST_ASSERT_EQUAL_UINT32(4242, wr.items[0].value.u32);

    uint32_t res[1] = {OPCUA_STATUS_GOOD};
    uint8_t resp[64];
    size_t sn = opcua_build_write_response(&wr, res, 9, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    uint32_t got[1] = {0xFFFFFFFFu};
    int32_t nres = opcua_client_on_write(resp, sn, got, 1);
    TEST_ASSERT_EQUAL_INT32(1, nres);
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_GOOD, got[0]);
}

void test_close_session_roundtrip()
{
    OpcUaClient c;
    opcua_client_init(&c);
    c.token_id = 88;

    uint8_t req[128];
    size_t rn = opcua_client_close_session(&c, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(req, rn, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CLOSE_SESSION_REQ, m.type_id);

    uint8_t resp[64];
    size_t sn = opcua_build_close_session_response(&m, 5, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);
    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, sn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
}

void test_close_channel_is_clo()
{
    uint8_t req[16];
    size_t rn = opcua_client_close_channel(req, sizeof(req));
    TEST_ASSERT_EQUAL_size_t(8, rn);
    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(req, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("CLO", h.type, 3);
}

void test_seq_and_request_id_increment()
{
    OpcUaClient c;
    opcua_client_init(&c);
    uint8_t req[256];
    opcua_client_open(&c, req, sizeof(req));                     // seq 1, request 1
    opcua_client_create_session(&c, "s", "u", req, sizeof(req)); // seq 2, request 2
    TEST_ASSERT_EQUAL_UINT32(2, c.seq);
    TEST_ASSERT_EQUAL_UINT32(2, c.request_id);
    TEST_ASSERT_TRUE(c.request_handle >= 2);
}

// on_read decodes every scalar Variant type and a per-value StatusCode.
void test_on_read_all_variant_types()
{
    OpcUaReadRequest rr;
    memset(&rr, 0, sizeof(rr));
    rr.count = 5;

    OpcUaVariant sv[5];
    uint32_t ss[5];
    memset(sv, 0, sizeof(sv));
    sv[0].type = OPCUA_VAR_BOOL;
    sv[0].b = true;
    sv[1].type = OPCUA_VAR_INT32;
    sv[1].i32 = -5;
    sv[2].type = OPCUA_VAR_FLOAT;
    sv[2].f32 = 1.5f;
    sv[3].type = OPCUA_VAR_STRING;
    sv[3].str = "hi";
    sv[3].str_len = 2;
    sv[4].type = OPCUA_VAR_INT32;
    sv[4].i32 = 7;
    ss[0] = ss[1] = ss[2] = ss[3] = OPCUA_STATUS_GOOD;
    ss[4] = OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED; // forces the StatusCode mask bit

    uint8_t resp[512];
    size_t sn = opcua_build_read_response(&rr, sv, ss, 5, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaVariant cv[5];
    uint32_t cs[5];
    int32_t n = opcua_client_on_read(resp, sn, cv, cs, 5);
    TEST_ASSERT_EQUAL_INT32(5, n);
    TEST_ASSERT_TRUE(cv[0].b);
    TEST_ASSERT_EQUAL_INT32(-5, cv[1].i32);
    TEST_ASSERT_EQUAL_FLOAT(1.5f, cv[2].f32);
    TEST_ASSERT_EQUAL_INT32(2, cv[3].str_len);
    TEST_ASSERT_EQUAL_MEMORY("hi", cv[3].str, 2);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, cs[4]);
}

// A ServiceFault (non-Good ServiceResult) is rejected by every service parser.
void test_client_parsers_reject_fault()
{
    OpcUaMsg m;
    memset(&m, 0, sizeof(m));
    uint8_t resp[128];
    size_t sn = opcua_build_service_fault(&m, OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, 1, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaClient c;
    opcua_client_init(&c);
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_get_endpoints(resp, sn));
    TEST_ASSERT_FALSE(opcua_client_on_create_session(&c, resp, sn));
    uint32_t results[1];
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_write(resp, sn, results, 1));
    OpcUaClientRef refs[1];
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_browse(resp, sn, refs, 1));
}

// Malformed responses are rejected: wrong UACP type, too-short ACK, corrupt body
// TypeId, and a truncated body.
void test_client_parsers_reject_malformed()
{
    OpcUaAckInfo ai;
    uint8_t ack[28] = {'A', 'C', 'K', 'F', 28, 0, 0, 0};
    ack[0] = 'X'; // wrong type
    TEST_ASSERT_FALSE(opcua_client_on_ack(ack, sizeof(ack), &ai));
    uint8_t shortack[12] = {'A', 'C', 'K', 'F', 12, 0, 0, 0};
    TEST_ASSERT_FALSE(opcua_client_on_ack(shortack, sizeof(shortack), &ai)); // < 8+20

    OpcUaClient c;
    opcua_client_init(&c);
    uint8_t opn[64];
    memset(opn, 0, sizeof(opn));
    opn[0] = 'M'; // OPN expected, MSG given
    opn[1] = 'S';
    opn[2] = 'G';
    opn[3] = 'F';
    opn[4] = 64;
    TEST_ASSERT_FALSE(opcua_client_on_open(&c, opn, sizeof(opn)));

    // Build a valid Read response, then corrupt the body TypeId NodeId kind.
    OpcUaReadRequest rr;
    memset(&rr, 0, sizeof(rr));
    rr.count = 0;
    uint8_t resp[128];
    size_t sn = opcua_build_read_response(&rr, nullptr, nullptr, 0, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);
    uint8_t save = resp[24];
    resp[24] = 0x06; // invalid NodeId encoding kind
    OpcUaVariant v[1];
    uint32_t s[1];
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_read(resp, sn, v, s, 1));
    resp[24] = save;
    // A body truncated mid-header underruns the reader.
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_read(resp, sn - 4, v, s, 1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_on_read_all_variant_types);
    RUN_TEST(test_client_parsers_reject_fault);
    RUN_TEST(test_client_parsers_reject_malformed);
    RUN_TEST(test_hello_ack_roundtrip);
    RUN_TEST(test_open_roundtrip);
    RUN_TEST(test_session_roundtrip);
    RUN_TEST(test_get_endpoints_roundtrip);
    RUN_TEST(test_service_fault_rejected_by_parsers);
    RUN_TEST(test_read_roundtrip);
    RUN_TEST(test_browse_roundtrip);
    RUN_TEST(test_write_roundtrip);
    RUN_TEST(test_close_session_roundtrip);
    RUN_TEST(test_close_channel_is_clo);
    RUN_TEST(test_seq_and_request_id_increment);
    return UNITY_END();
}
