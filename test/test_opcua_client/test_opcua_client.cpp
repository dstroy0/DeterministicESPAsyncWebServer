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
    TEST_ASSERT_EQUAL_UINT32(DWS_OPCUA_BUF, hello.recv_buf_size);

    uint8_t ack[64];
    size_t an = opcua_build_ack(&hello, ack, sizeof(ack));
    TEST_ASSERT_TRUE(an > 0);

    OpcUaAckInfo info;
    TEST_ASSERT_TRUE(opcua_client_on_ack(ack, an, &info));
    TEST_ASSERT_EQUAL_UINT32(DWS_OPCUA_BUF, info.recv_buf_size);
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
    svals[0].type = OpcUaVariantType::OPCUA_VAR_UINT32;
    svals[0].u32 = 4242;
    ssts[0] = OPCUA_STATUS_GOOD;
    svals[1].type = OpcUaVariantType::OPCUA_VAR_DOUBLE;
    svals[1].f64 = 2.5;
    ssts[1] = OPCUA_STATUS_GOOD;

    uint8_t resp[256];
    size_t sn = opcua_build_read_response(&rr, svals, ssts, 3, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);

    OpcUaVariant cvals[2];
    uint32_t csts[2];
    int32_t n = opcua_client_on_read(resp, sn, cvals, csts, 2);
    TEST_ASSERT_EQUAL_INT32(2, n);
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_UINT32, cvals[0].type);
    TEST_ASSERT_EQUAL_UINT32(4242, cvals[0].u32);
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_DOUBLE, cvals[1].type);
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
    items[0].value.type = OpcUaVariantType::OPCUA_VAR_UINT32;
    items[0].value.u32 = 4242;

    uint8_t req[128];
    size_t rn = opcua_client_write(&c, items, 1, req, sizeof(req));
    TEST_ASSERT_TRUE(rn > 0);

    OpcUaWriteRequest wr;
    TEST_ASSERT_TRUE(opcua_parse_write(req, rn, &wr));
    TEST_ASSERT_EQUAL_UINT32(1, wr.count);
    TEST_ASSERT_EQUAL_UINT32(10, wr.items[0].id);
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_UINT32, wr.items[0].value.type);
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
    sv[0].type = OpcUaVariantType::OPCUA_VAR_BOOL;
    sv[0].b = true;
    sv[1].type = OpcUaVariantType::OPCUA_VAR_INT32;
    sv[1].i32 = -5;
    sv[2].type = OpcUaVariantType::OPCUA_VAR_FLOAT;
    sv[2].f32 = 1.5f;
    sv[3].type = OpcUaVariantType::OPCUA_VAR_STRING;
    sv[3].str = "hi";
    sv[3].str_len = 2;
    sv[4].type = OpcUaVariantType::OPCUA_VAR_INT32;
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

void test_builder_overflow_guard()
{
    // A capacity too small for even the frame header overflows the writer; cw_patch returns 0.
    uint8_t tiny[6];
    TEST_ASSERT_EQUAL_size_t(0, opcua_client_hello("opc.tcp://host:4840", tiny, sizeof(tiny)));
    OpcUaClient c;
    opcua_client_init(&c);
    TEST_ASSERT_EQUAL_size_t(0, opcua_client_open(&c, tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_size_t(0, opcua_client_activate_session(&c, tiny, sizeof(tiny)));
}

void test_on_read_unknown_variant_rejected()
{
    // A server sending a DataValue whose Variant type byte is unsupported must be rejected, not
    // mis-decoded. Build a valid UINT32 response with a distinctive value, then patch its type byte.
    OpcUaClient c;
    opcua_client_init(&c);
    OpcUaReadItem items[1] = {{1, 1, true, OPCUA_ATTR_VALUE}};
    uint8_t req[128];
    size_t rn = opcua_client_read(&c, items, 1, req, sizeof(req));
    OpcUaReadRequest rr;
    TEST_ASSERT_TRUE(opcua_parse_read(req, rn, &rr));
    OpcUaVariant sv[1];
    uint32_t ss[1];
    memset(sv, 0, sizeof(sv));
    sv[0].type = OpcUaVariantType::OPCUA_VAR_UINT32;
    sv[0].u32 = 0xA1B2C3D4; // distinctive little-endian marker D4 C3 B2 A1
    ss[0] = OPCUA_STATUS_GOOD;
    uint8_t resp[128];
    size_t sn = opcua_build_read_response(&rr, sv, ss, 1, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(sn > 0);
    int type_off = -1;
    for (size_t i = 1; i + 4 < sn; i++)
        if (resp[i] == (uint8_t)OpcUaVariantType::OPCUA_VAR_UINT32 && resp[i + 1] == 0xD4 && resp[i + 2] == 0xC3 &&
            resp[i + 3] == 0xB2 && resp[i + 4] == 0xA1)
        {
            type_off = (int)i;
            break;
        }
    TEST_ASSERT_TRUE(type_off > 0);
    resp[type_off] = 200; // an unsupported Variant type
    OpcUaVariant cv[1];
    uint32_t cs[1];
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_read(resp, sn, cv, cs, 1)); // default arm -> err -> -1
}

// Build a minimal valid MSG response (header + security/sequence + TypeId + ResponseHeader) whose
// service body starts with a single i32 - used to feed the array-count guards an out-of-range value.
static size_t build_min_response(uint8_t *out, size_t cap, uint32_t type_id, int32_t count_field, int32_t str_table = 0)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    ua_w_u32(&w, 0); // SecureChannelId
    ua_w_u32(&w, 0); // TokenId
    ua_w_u32(&w, 0); // SequenceNumber
    ua_w_u32(&w, 0); // RequestId
    ua_w_nodeid_numeric(&w, 0, type_id);
    ua_w_u64(&w, 0);                 // ResponseHeader.Timestamp
    ua_w_u32(&w, 0);                 // RequestHandle
    ua_w_u32(&w, OPCUA_STATUS_GOOD); // ServiceResult
    ua_w_u8(&w, 0);                  // ServiceDiagnostics
    ua_w_i32(&w, str_table);         // StringTable count
    for (int32_t i = 0; i < str_table; i++)
        ua_w_i32(&w, -1);          // one (null) StringTable entry each
    ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader NodeId (null)
    ua_w_u8(&w, 0);                // AdditionalHeader ExtensionObject (no body)
    ua_w_i32(&w, count_field);     // service body: Results/Endpoints count
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = (uint8_t)(w.n >> 16);
    out[7] = (uint8_t)(w.n >> 24);
    return w.ok ? w.n : 0;
}

void test_response_parsers_reject_negative_count()
{
    uint8_t resp[128];
    OpcUaVariant v[1];
    uint32_t s[1];
    size_t n = build_min_response(resp, sizeof(resp), OPCUA_ID_READ_RESP, -1);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_read(resp, n, v, s, 1));
    uint32_t wr[1];
    n = build_min_response(resp, sizeof(resp), OPCUA_ID_WRITE_RESP, -1);
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_write(resp, n, wr, 1));
    OpcUaClientRef refs[1];
    n = build_min_response(resp, sizeof(resp), OPCUA_ID_BROWSE_RESP, -1);
    TEST_ASSERT_EQUAL_INT32(-1, opcua_client_on_browse(resp, n, refs, 1));
}

// Build an OPN response frame with a controllable SecurityPolicyUri length, body TypeId, and whether
// a ResponseHeader follows - to drive on_open's skip/guard paths.
static size_t build_opn(uint8_t *out, size_t cap, uint32_t type_id, int32_t policy_len, bool with_rh)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'P');
    ua_w_u8(&w, 'N');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    ua_w_u32(&w, 0); // SecureChannelId
    ua_w_i32(&w, policy_len);
    for (int32_t i = 0; i < policy_len; i++)
        ua_w_u8(&w, 'x'); // SecurityPolicyUri bytes
    ua_w_i32(&w, -1);     // SenderCertificate (null)
    ua_w_i32(&w, -1);     // ReceiverCertificateThumbprint (null)
    ua_w_u32(&w, 0);      // SequenceNumber
    ua_w_u32(&w, 0);      // RequestId
    ua_w_nodeid_numeric(&w, 0, type_id);
    if (with_rh)
    {
        ua_w_u64(&w, 0);
        ua_w_u32(&w, 0);
        ua_w_u32(&w, OPCUA_STATUS_GOOD);
        ua_w_u8(&w, 0);
        ua_w_i32(&w, 0);
        ua_w_nodeid_numeric(&w, 0, 0);
        ua_w_u8(&w, 0);
    }
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = (uint8_t)(w.n >> 16);
    out[7] = (uint8_t)(w.n >> 24);
    return w.ok ? w.n : 0;
}

void test_on_open_guards()
{
    OpcUaClient c;
    opcua_client_init(&c);
    uint8_t buf[128];
    // Non-empty SecurityPolicyUri exercises the cr_skip_string skip; then the missing ResponseHeader
    // underruns the reader -> parse fails.
    size_t n = build_opn(buf, sizeof(buf), OPCUA_ID_OPEN_RESP, 10, false);
    TEST_ASSERT_FALSE(opcua_client_on_open(&c, buf, n));
    // A wrong body TypeId is rejected.
    n = build_opn(buf, sizeof(buf), 999, 0, true);
    TEST_ASSERT_FALSE(opcua_client_on_open(&c, buf, n));
    // A SecurityPolicyUri length larger than the frame overflows cr_skip.
    uint8_t ovf[16];
    UaWriter w = {ovf, sizeof(ovf), 0, true};
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'P');
    ua_w_u8(&w, 'N');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);
    ua_w_u32(&w, 0);       // SecureChannelId
    ua_w_i32(&w, 1000000); // SecurityPolicyUri length far past the frame
    ovf[4] = (uint8_t)w.n;
    ovf[5] = ovf[6] = ovf[7] = 0;
    TEST_ASSERT_FALSE(opcua_client_on_open(&c, ovf, w.n));
}

void test_response_header_string_table_skip()
{
    // A ResponseHeader carrying a non-empty StringTable makes cr_skip_string_array iterate; the
    // response is otherwise valid (0 results) so on_read succeeds.
    uint8_t resp[128];
    size_t n = build_min_response(resp, sizeof(resp), OPCUA_ID_READ_RESP, 0, 1);
    OpcUaVariant v[1];
    uint32_t s[1];
    TEST_ASSERT_EQUAL_INT32(0, opcua_client_on_read(resp, n, v, s, 1));
}

// Forge a BrowseResponse carrying one ReferenceDescription whose DisplayName LocalizedText sets the
// Locale mask bit (0x01). Our own server never emits a Locale (ua_w_localizedtext is always called
// with a null locale), so only a forged / third-party-server response exercises the client's
// Locale-skip parse path - a real path for a spec-compliant peer that does send a Locale.
static size_t build_browse_with_locale(uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    ua_w_u32(&w, 0); // SecureChannelId
    ua_w_u32(&w, 0); // TokenId
    ua_w_u32(&w, 0); // SequenceNumber
    ua_w_u32(&w, 0); // RequestId
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_BROWSE_RESP);
    ua_w_u64(&w, 0);                 // ResponseHeader.Timestamp
    ua_w_u32(&w, 0);                 // RequestHandle
    ua_w_u32(&w, OPCUA_STATUS_GOOD); // ServiceResult
    ua_w_u8(&w, 0);                  // ServiceDiagnostics
    ua_w_i32(&w, 0);                 // StringTable count (none)
    ua_w_nodeid_numeric(&w, 0, 0);   // AdditionalHeader NodeId (null)
    ua_w_u8(&w, 0);                  // AdditionalHeader ExtensionObject (no body)
    // BrowseResponse body: Results[1] -> one BrowseResult -> one ReferenceDescription.
    ua_w_i32(&w, 1);                                              // Results count
    ua_w_u32(&w, OPCUA_STATUS_GOOD);                              // BrowseResult.StatusCode
    ua_w_i32(&w, -1);                                             // ContinuationPoint (null ByteString)
    ua_w_i32(&w, 1);                                              // References count
    ua_w_nodeid_numeric(&w, 0, OPCUA_REFTYPE_ORGANIZES);          // ReferenceTypeId
    ua_w_bool(&w, true);                                          // IsForward
    ua_w_nodeid_numeric(&w, 1, 7);                                // TargetId (NodeId, numeric)
    ua_w_u16(&w, 1);                                              // BrowseName.NamespaceIndex
    ua_w_string(&w, "Node7", 5);                                  // BrowseName.Name
    ua_w_u8(&w, 0x03);                                            // DisplayName mask: Locale (0x01) + Text (0x02)
    ua_w_string(&w, "en-US", 5);                                  // Locale -> client Locale-skip path
    ua_w_string(&w, "Node 7", 6);                                 // Text
    ua_w_u32(&w, OPCUA_NODECLASS_VARIABLE);                       // NodeClass
    ua_w_nodeid_numeric(&w, 0, OPCUA_TYPEDEF_BASE_DATA_VARIABLE); // TypeDefinition
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = (uint8_t)(w.n >> 16);
    out[7] = (uint8_t)(w.n >> 24);
    return w.ok ? w.n : 0;
}

// A DisplayName carrying a Locale (mask 0x01) is skipped, and the reference still parses correctly.
void test_browse_display_name_locale()
{
    uint8_t resp[256];
    size_t n = build_browse_with_locale(resp, sizeof(resp));
    TEST_ASSERT_TRUE(n > 0);

    OpcUaClientRef refs[2];
    int32_t got = opcua_client_on_browse(resp, n, refs, 2);
    TEST_ASSERT_EQUAL_INT32(1, got);
    TEST_ASSERT_EQUAL_STRING("Node7", refs[0].browse_name);
    TEST_ASSERT_EQUAL_UINT32(7, refs[0].target_id);
    TEST_ASSERT_EQUAL_UINT16(1, refs[0].target_ns);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_browse_display_name_locale);
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
    RUN_TEST(test_builder_overflow_guard);
    RUN_TEST(test_on_read_unknown_variant_rejected);
    RUN_TEST(test_response_parsers_reject_negative_count);
    RUN_TEST(test_on_open_guards);
    RUN_TEST(test_response_header_string_table_skip);
    return UNITY_END();
}
