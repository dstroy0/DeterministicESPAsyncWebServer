// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for OPC UA (services/opcua): the Binary built-in type codec (incl.
// NodeId / DateTime / Variant / DataValue / ReferenceDescription), UACP framing,
// the Hello/Acknowledge handshake, the SecureChannel (OPN), the Session
// (CreateSession/ActivateSession), GetEndpoints, the Read / Write / Browse / CloseSession
// services and the ServiceFault fallback (SecurityPolicy None). The TCP data handler
// (opcua_rx) is ESP32-only and HW-verified (incl. python asyncua interop).

#include "services/opcua/opcua.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_codec_roundtrip()
{
    uint8_t buf[128];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_bool(&w, true);
    ua_w_u8(&w, 0x7F);
    ua_w_u16(&w, 0xBEEF);
    ua_w_u32(&w, 0xDEADBEEF);
    ua_w_u64(&w, 0x0123456789ABCDEFull);
    ua_w_i32(&w, -12345);
    ua_w_f32(&w, 3.5f);
    ua_w_f64(&w, 2.5);
    ua_w_string(&w, "hello", 5);
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_TRUE(ua_r_bool(&r));
    TEST_ASSERT_EQUAL_HEX8(0x7F, ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, ua_r_u16(&r));
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, ua_r_u32(&r));
    TEST_ASSERT_TRUE(ua_r_u64(&r) == 0x0123456789ABCDEFull);
    TEST_ASSERT_EQUAL_INT32(-12345, ua_r_i32(&r));
    TEST_ASSERT_EQUAL_FLOAT(3.5f, ua_r_f32(&r));
    double d = ua_r_f64(&r);
    TEST_ASSERT_TRUE(d == 2.5); // exact in IEEE-754 (avoids Unity's optional double assert)
    char s[16];
    int32_t slen = 0;
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &slen));
    TEST_ASSERT_EQUAL_INT32(5, slen);
    TEST_ASSERT_EQUAL_STRING("hello", s);
    TEST_ASSERT_FALSE(r.err);
}

void test_string_null_roundtrip()
{
    uint8_t buf[8];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_string(&w, nullptr, -1);
    UaReader r = {buf, w.n, 0, false};
    char s[4];
    int32_t slen = 99;
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &slen));
    TEST_ASSERT_EQUAL_INT32(-1, slen);
}

void test_reader_underrun_latches()
{
    uint8_t buf[2] = {1, 2};
    UaReader r = {buf, sizeof(buf), 0, false};
    ua_r_u32(&r); // only 2 bytes available
    TEST_ASSERT_TRUE(r.err);
}

void test_writer_overflow_fails_closed()
{
    uint8_t buf[3];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_u32(&w, 1); // 4 bytes into a 3-byte buffer
    TEST_ASSERT_FALSE(w.ok);
}

// Build a Hello message and assert it parses; then build its Acknowledge.
static size_t build_hello(uint8_t *out, size_t cap, uint32_t recv, uint32_t send, uint32_t maxmsg)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'H');
    ua_w_u8(&w, 'E');
    ua_w_u8(&w, 'L');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    ua_w_u32(&w, 0); // ProtocolVersion
    ua_w_u32(&w, recv);
    ua_w_u32(&w, send);
    ua_w_u32(&w, maxmsg);
    ua_w_u32(&w, 0); // MaxChunkCount
    ua_w_string(&w, "opc.tcp://host:4840", 19);
    // patch size
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_header()
{
    uint8_t hel[128];
    size_t n = build_hello(hel, sizeof(hel), 65535, 65535, 0);
    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(hel, n, &h));
    TEST_ASSERT_EQUAL_MEMORY("HEL", h.type, 3);
    TEST_ASSERT_EQUAL_CHAR('F', h.chunk);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)n, h.size);
}

void test_parse_hello()
{
    uint8_t hel[128];
    size_t n = build_hello(hel, sizeof(hel), 65535, 32768, 0);
    OpcUaHello hello;
    TEST_ASSERT_TRUE(opcua_parse_hello(hel, n, &hello));
    TEST_ASSERT_EQUAL_UINT32(0, hello.protocol_version);
    TEST_ASSERT_EQUAL_UINT32(65535, hello.recv_buf_size);
    TEST_ASSERT_EQUAL_UINT32(32768, hello.send_buf_size);
}

void test_parse_hello_rejects_short()
{
    uint8_t bad[12] = {'H', 'E', 'L', 'F', 12, 0, 0, 0, 0, 0, 0, 0};
    OpcUaHello hello;
    TEST_ASSERT_FALSE(opcua_parse_hello(bad, sizeof(bad), &hello)); // no room for the 5 sizes
}

void test_build_ack_negotiates()
{
    uint8_t hel[128];
    size_t n = build_hello(hel, sizeof(hel), 65535, 65535, 0);
    OpcUaHello hello;
    TEST_ASSERT_TRUE(opcua_parse_hello(hel, n, &hello));

    uint8_t ack[64];
    size_t an = opcua_build_ack(&hello, ack, sizeof(ack));
    TEST_ASSERT_EQUAL_size_t(28, an); // 8 header + 5 x UInt32

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(ack, an, &h));
    TEST_ASSERT_EQUAL_MEMORY("ACK", h.type, 3);
    TEST_ASSERT_EQUAL_UINT32(28, h.size);

    UaReader r = {ack + 8, an - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));    // ProtocolVersion
    TEST_ASSERT_EQUAL_UINT32(8192, ua_r_u32(&r)); // recv = min(client send 65535, server 8192)
    TEST_ASSERT_EQUAL_UINT32(8192, ua_r_u32(&r)); // send = min(client recv 65535, server 8192)
    TEST_ASSERT_EQUAL_UINT32(8192, ua_r_u32(&r)); // max msg (client 0 -> server)
    TEST_ASSERT_EQUAL_UINT32(1, ua_r_u32(&r));    // max chunk
}

// ---------------------------------------------------------------------------
// Increment 2 - NodeId / DateTime / SecureChannel (OPN)
// ---------------------------------------------------------------------------

void test_nodeid_roundtrip()
{
    uint8_t buf[32];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_nodeid_numeric(&w, 0, 5);     // TwoByte
    ua_w_nodeid_numeric(&w, 0, 446);   // FourByte (id > 255)
    ua_w_nodeid_numeric(&w, 3, 70000); // Numeric (ns + 32-bit id)
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    UaNodeId id;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(0, id.ns);
    TEST_ASSERT_EQUAL_UINT32(5, id.id);
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(0, id.ns);
    TEST_ASSERT_EQUAL_UINT32(446, id.id);
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(3, id.ns);
    TEST_ASSERT_EQUAL_UINT32(70000, id.id);
    TEST_ASSERT_FALSE(r.err);
}

void test_filetime_from_unix()
{
    TEST_ASSERT_TRUE(opcua_filetime_from_unix(0) == 0);
    TEST_ASSERT_TRUE(opcua_filetime_from_unix(-5) == 0);
    TEST_ASSERT_TRUE(opcua_filetime_from_unix(1) == (11644473600LL + 1) * 10000000LL);
}

// Build a minimal OpenSecureChannelRequest (OPN, SecurityPolicy None).
static size_t build_open(uint8_t *out, size_t cap, uint32_t channel, uint32_t seq, uint32_t req_id, uint32_t handle,
                         uint32_t mode, uint32_t lifetime)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'P');
    ua_w_u8(&w, 'N');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    // Asymmetric security header.
    ua_w_u32(&w, channel);
    const char *pol = OPCUA_POLICY_NONE_URI;
    ua_w_string(&w, pol, (int32_t)strlen(pol));
    ua_w_string(&w, nullptr, -1); // SenderCertificate
    ua_w_string(&w, nullptr, -1); // ReceiverCertificateThumbprint
    // Sequence header.
    ua_w_u32(&w, seq);
    ua_w_u32(&w, req_id);
    // Body TypeId.
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_REQ);
    // RequestHeader.
    ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken (null)
    ua_w_u64(&w, 0);               // Timestamp
    ua_w_u32(&w, handle);          // RequestHandle
    ua_w_u32(&w, 0);               // ReturnDiagnostics
    ua_w_string(&w, nullptr, -1);  // AuditEntryId
    ua_w_u32(&w, 0);               // TimeoutHint
    ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader: null NodeId ...
    ua_w_u8(&w, 0x00);             // ... + no body
    // OpenSecureChannelRequest body.
    ua_w_u32(&w, 0);              // ClientProtocolVersion
    ua_w_u32(&w, 0);              // RequestType = Issue
    ua_w_u32(&w, mode);           // MessageSecurityMode
    ua_w_string(&w, nullptr, -1); // ClientNonce
    ua_w_u32(&w, lifetime);
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_open()
{
    uint8_t buf[256];
    size_t n = build_open(buf, sizeof(buf), 0, 1, 100, 42, 1, 3600000);
    OpcUaOpenChannel oc;
    TEST_ASSERT_TRUE(opcua_parse_open(buf, n, &oc));
    TEST_ASSERT_EQUAL_UINT32(0, oc.secure_channel_id);
    TEST_ASSERT_EQUAL_UINT32(1, oc.sequence_number);
    TEST_ASSERT_EQUAL_UINT32(100, oc.request_id);
    TEST_ASSERT_EQUAL_UINT32(42, oc.request_handle);
    TEST_ASSERT_EQUAL_UINT32(0, oc.security_token_request_type);
    TEST_ASSERT_EQUAL_UINT32(1, oc.message_security_mode);
    TEST_ASSERT_EQUAL_UINT32(3600000, oc.requested_lifetime);
}

void test_parse_open_rejects_wrong_type()
{
    uint8_t buf[256];
    size_t n = build_open(buf, sizeof(buf), 0, 1, 100, 42, 1, 3600000);
    // Corrupt the message type so it is no longer "OPN".
    buf[0] = 'M';
    OpcUaOpenChannel oc;
    TEST_ASSERT_FALSE(opcua_parse_open(buf, n, &oc));
}

void test_build_open_response()
{
    uint8_t buf[256];
    size_t n = build_open(buf, sizeof(buf), 0, 1, 7, 42, 1, 600000);
    OpcUaOpenChannel oc;
    TEST_ASSERT_TRUE(opcua_parse_open(buf, n, &oc));

    uint8_t resp[256];
    int64_t now = opcua_filetime_from_unix(1700000000LL);
    size_t rn = opcua_build_open_response(&oc, 55, 99, 1, now, 600000, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("OPN", h.type, 3);
    TEST_ASSERT_EQUAL_CHAR('F', h.chunk);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)rn, h.size);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[64];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(55, ua_r_u32(&r)); // SecureChannelId
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl));
    TEST_ASSERT_EQUAL_STRING(OPCUA_POLICY_NONE_URI, str);
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // SenderCertificate
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ReceiverCertificateThumbprint
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_EQUAL_UINT32(1, ua_r_u32(&r)); // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r)); // RequestId echoed

    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_OPEN_RESP, tid.id); // OpenSecureChannelResponse

    // ResponseHeader.
    TEST_ASSERT_TRUE(ua_r_u64(&r) == (uint64_t)now); // Timestamp
    TEST_ASSERT_EQUAL_UINT32(42, ua_r_u32(&r));      // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));       // ServiceResult = Good
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));       // ServiceDiagnostics
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r));       // StringTable (null)
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));         // AdditionalHeader NodeId (null)
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));       // AdditionalHeader body: none

    // OpenSecureChannelResponse body.
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));                // ServerProtocolVersion
    TEST_ASSERT_EQUAL_UINT32(55, ua_r_u32(&r));               // ChannelId
    TEST_ASSERT_EQUAL_UINT32(99, ua_r_u32(&r));               // TokenId
    TEST_ASSERT_TRUE(ua_r_u64(&r) == (uint64_t)now);          // CreatedAt
    TEST_ASSERT_EQUAL_UINT32(600000, ua_r_u32(&r));           // RevisedLifetime
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce (null)
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Increment 3 - Session (CreateSession / ActivateSession over MSG)
// ---------------------------------------------------------------------------

// Build a minimal MSG service request: envelope + body TypeId + RequestHeader.
// (The service-specific body after the RequestHeader is not needed by the server.)
static size_t build_msg(uint8_t *out, size_t cap, uint32_t type_id, uint32_t token, uint32_t seq, uint32_t req_id,
                        uint32_t handle)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);     // size placeholder
    ua_w_u32(&w, 0);     // SecureChannelId
    ua_w_u32(&w, token); // SymmetricSecurityHeader.TokenId
    ua_w_u32(&w, seq);   // SequenceHeader.SequenceNumber
    ua_w_u32(&w, req_id);
    ua_w_nodeid_numeric(&w, 0, type_id); // body TypeId
    // RequestHeader
    ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken (null for CreateSession)
    ua_w_u64(&w, 0);               // Timestamp
    ua_w_u32(&w, handle);          // RequestHandle
    ua_w_u32(&w, 0);               // ReturnDiagnostics
    ua_w_string(&w, nullptr, -1);  // AuditEntryId
    ua_w_u32(&w, 0);               // TimeoutHint
    ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader: null NodeId ...
    ua_w_u8(&w, 0x00);             // ... + no body
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_msg()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CREATE_SESSION_REQ, 7, 3, 100, 42);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT32(7, m.token_id);
    TEST_ASSERT_EQUAL_UINT32(3, m.sequence_number);
    TEST_ASSERT_EQUAL_UINT32(100, m.request_id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CREATE_SESSION_REQ, m.type_id);
    TEST_ASSERT_EQUAL_UINT32(42, m.request_handle);
}

void test_parse_msg_rejects_non_msg()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CREATE_SESSION_REQ, 7, 3, 100, 42);
    buf[0] = 'O'; // make it "OSG"
    OpcUaMsg m;
    TEST_ASSERT_FALSE(opcua_parse_msg(buf, n, &m));
}

void test_build_create_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CREATE_SESSION_REQ, 7, 3, 100, 42);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));

    uint8_t resp[512];
    int64_t now = opcua_filetime_from_unix(1700000000LL);
    OpcUaServerInfo si = {"opc.tcp://test:4840", "urn:test", "TestServer"};
    size_t rn = opcua_build_create_session_response(&m, 0x1001, 0x2002, 1200000.0, &si, 5, now, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)rn, h.size);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[16];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(5, ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(100, ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CREATE_SESSION_RESP, tid.id);
    // ResponseHeader
    TEST_ASSERT_TRUE(ua_r_u64(&r) == (uint64_t)now); // Timestamp
    TEST_ASSERT_EQUAL_UINT32(42, ua_r_u32(&r));      // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));       // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));       // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r));       // StringTable null
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));         // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));       // AdditionalHeader body none
    // CreateSessionResponse body
    UaNodeId sid, atok;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &sid)); // SessionId
    TEST_ASSERT_EQUAL_UINT32(0x1001, sid.id);
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &atok)); // AuthenticationToken
    TEST_ASSERT_EQUAL_UINT32(0x2002, atok.id);
    TEST_ASSERT_TRUE(ua_r_f64(&r) == 1200000.0);              // RevisedSessionTimeout
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce null
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerCertificate null
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));                 // ServerEndpoints[] -> one endpoint
    // The endpoint encoding itself is validated by test_build_get_endpoints; here we
    // just confirm the session fields and a clean parse of the rest of the message.
    TEST_ASSERT_FALSE(r.err);
}

void test_build_activate_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_ACTIVATE_SESSION_REQ, 7, 4, 101, 43);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));

    uint8_t resp[128];
    size_t rn = opcua_build_activate_session_response(&m, 6, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[8];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(6, ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(101, ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_ACTIVATE_SESSION_RESP, tid.id);
    (void)ua_r_u64(&r);                                       // Timestamp
    TEST_ASSERT_EQUAL_UINT32(43, ua_r_u32(&r));               // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));                // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));                // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r));                // StringTable null
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));                  // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));                // AdditionalHeader body none
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce null
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r)); // Results[] empty
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r)); // DiagnosticInfos[] empty
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Increment 4 - Read service (Variant / DataValue + ReadRequest/Response)
// ---------------------------------------------------------------------------

void test_datavalue_good_int32()
{
    uint8_t buf[32];
    UaWriter w = {buf, sizeof(buf), 0, true};
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OPCUA_VAR_INT32;
    v.i32 = -7;
    ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD);
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_EQUAL_HEX8(0x01, ua_r_u8(&r));            // mask: value present
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_INT32, ua_r_u8(&r)); // Variant encoding byte
    TEST_ASSERT_EQUAL_INT32(-7, ua_r_i32(&r));
    TEST_ASSERT_FALSE(r.err);
}

void test_datavalue_bad_status()
{
    uint8_t buf[16];
    UaWriter w = {buf, sizeof(buf), 0, true};
    OpcUaVariant v;
    memset(&v, 0, sizeof(v)); // null Variant
    ua_w_datavalue(&w, &v, OPCUA_STATUS_BAD_NODE_ID_UNKNOWN);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_EQUAL_HEX8(0x02, ua_r_u8(&r)); // mask: status only
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, ua_r_u32(&r));
    TEST_ASSERT_FALSE(r.err);
}

// Build a ReadRequest MSG reading ns=1 numeric ids (AttributeId = Value).
static size_t build_read(uint8_t *out, size_t cap, uint32_t token, uint32_t seq, uint32_t req_id, uint32_t handle,
                         const uint32_t *ids, uint32_t n)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);
    ua_w_u32(&w, 0); // SecureChannelId
    ua_w_u32(&w, token);
    ua_w_u32(&w, seq);
    ua_w_u32(&w, req_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_READ_REQ);
    // RequestHeader
    ua_w_nodeid_numeric(&w, 0, 0);
    ua_w_u64(&w, 0);
    ua_w_u32(&w, handle);
    ua_w_u32(&w, 0);
    ua_w_string(&w, nullptr, -1);
    ua_w_u32(&w, 0);
    ua_w_nodeid_numeric(&w, 0, 0);
    ua_w_u8(&w, 0x00);
    // ReadRequest body
    ua_w_f64(&w, 0.0);        // MaxAge
    ua_w_u32(&w, 0);          // TimestampsToReturn
    ua_w_i32(&w, (int32_t)n); // NodesToRead count
    for (uint32_t i = 0; i < n; i++)
    {
        ua_w_nodeid_numeric(&w, 1, ids[i]); // NodeId (ns=1)
        ua_w_u32(&w, OPCUA_ATTR_VALUE);     // AttributeId
        ua_w_string(&w, nullptr, -1);       // IndexRange
        ua_w_u16(&w, 0);                    // DataEncoding QualifiedName.ns
        ua_w_string(&w, nullptr, -1);       // QualifiedName.name
    }
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_read()
{
    uint8_t buf[256];
    uint32_t ids[2] = {1001, 1002};
    size_t n = build_read(buf, sizeof(buf), 7, 5, 200, 60, ids, 2);
    OpcUaReadRequest rr;
    TEST_ASSERT_TRUE(opcua_parse_read(buf, n, &rr));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_READ_REQ, rr.msg.type_id);
    TEST_ASSERT_EQUAL_UINT32(60, rr.msg.request_handle);
    TEST_ASSERT_EQUAL_UINT32(2, rr.total);
    TEST_ASSERT_EQUAL_UINT32(2, rr.count);
    TEST_ASSERT_EQUAL_UINT16(1, rr.items[0].ns);
    TEST_ASSERT_EQUAL_UINT32(1001, rr.items[0].id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ATTR_VALUE, rr.items[0].attribute);
    TEST_ASSERT_EQUAL_UINT32(1002, rr.items[1].id);
}

void test_build_read_response()
{
    uint8_t buf[256];
    uint32_t ids[2] = {1001, 1002};
    size_t n = build_read(buf, sizeof(buf), 7, 5, 200, 60, ids, 2);
    OpcUaReadRequest rr;
    TEST_ASSERT_TRUE(opcua_parse_read(buf, n, &rr));

    OpcUaVariant vals[2];
    uint32_t sts[2];
    memset(vals, 0, sizeof(vals));
    vals[0].type = OPCUA_VAR_INT32;
    vals[0].i32 = 4242;
    sts[0] = OPCUA_STATUS_GOOD;
    vals[1].type = OPCUA_VAR_NULL;
    sts[1] = OPCUA_STATUS_BAD_NODE_ID_UNKNOWN;

    uint8_t resp[256];
    size_t rn = opcua_build_read_response(&rr, vals, sts, 9, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(9, ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(200, ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_READ_RESP, tid.id);
    (void)ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(60, ua_r_u32(&r)); // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));  // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));  // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r));  // StringTable null
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));    // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));  // AdditionalHeader body none

    // Results[]
    TEST_ASSERT_EQUAL_INT32(2, ua_r_i32(&r)); // count
    // result 0: Good Int32 4242
    TEST_ASSERT_EQUAL_HEX8(0x01, ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_INT32, ua_r_u8(&r));
    TEST_ASSERT_EQUAL_INT32(4242, ua_r_i32(&r));
    // result 1: Bad status, no value
    TEST_ASSERT_EQUAL_HEX8(0x02, ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, ua_r_u32(&r));
    // DiagnosticInfos[]
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r));
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Increment 5 - Browse service + CloseSession
// ---------------------------------------------------------------------------

// Test Browse resolver: node ns=0;i=85 (Objects) has one child; everything else unknown.
static int32_t test_browse_handler(uint16_t ns, uint32_t id, OpcUaReference *out, uint32_t max)
{
    if (ns == 0 && id == 85 && max >= 1)
    {
        out[0].ref_type_id = OPCUA_REFTYPE_ORGANIZES;
        out[0].is_forward = true;
        out[0].target_ns = 1;
        out[0].target_id = 1;
        out[0].browse_name_ns = 1;
        out[0].browse_name = "Uptime";
        out[0].display_name = "Uptime";
        out[0].node_class = OPCUA_NODECLASS_VARIABLE;
        out[0].type_def_id = OPCUA_TYPEDEF_BASE_DATA_VARIABLE;
        return 1;
    }
    return -1; // unknown node
}

// Build a BrowseRequest MSG that browses one node (ns, id).
static size_t build_browse(uint8_t *out, size_t cap, uint32_t token, uint32_t seq, uint32_t req_id, uint32_t handle,
                           uint16_t ns, uint32_t id)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);
    ua_w_u32(&w, 0); // SecureChannelId
    ua_w_u32(&w, token);
    ua_w_u32(&w, seq);
    ua_w_u32(&w, req_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_BROWSE_REQ);
    // RequestHeader
    ua_w_nodeid_numeric(&w, 0, 0);
    ua_w_u64(&w, 0);
    ua_w_u32(&w, handle);
    ua_w_u32(&w, 0);
    ua_w_string(&w, nullptr, -1);
    ua_w_u32(&w, 0);
    ua_w_nodeid_numeric(&w, 0, 0);
    ua_w_u8(&w, 0x00);
    // BrowseRequest body
    ua_w_nodeid_numeric(&w, 0, 0);   // View.ViewId
    ua_w_u64(&w, 0);                 // View.Timestamp
    ua_w_u32(&w, 0);                 // View.ViewVersion
    ua_w_u32(&w, 0);                 // RequestedMaxReferencesPerNode
    ua_w_i32(&w, 1);                 // NodesToBrowse count
    ua_w_nodeid_numeric(&w, ns, id); // BrowseDescription.NodeId
    ua_w_u32(&w, 0);                 // BrowseDirection (Forward)
    ua_w_nodeid_numeric(&w, 0, 0);   // ReferenceTypeId (null = all)
    ua_w_bool(&w, true);             // IncludeSubtypes
    ua_w_u32(&w, 0);                 // NodeClassMask
    ua_w_u32(&w, 0x3F);              // ResultMask (all)
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_browse()
{
    uint8_t buf[256];
    size_t n = build_browse(buf, sizeof(buf), 7, 6, 300, 70, 0, 85);
    OpcUaBrowseRequest br;
    TEST_ASSERT_TRUE(opcua_parse_browse(buf, n, &br));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_BROWSE_REQ, br.msg.type_id);
    TEST_ASSERT_EQUAL_UINT32(70, br.msg.request_handle);
    TEST_ASSERT_EQUAL_UINT32(1, br.count);
    TEST_ASSERT_EQUAL_UINT16(0, br.items[0].ns);
    TEST_ASSERT_EQUAL_UINT32(85, br.items[0].id);
}

void test_build_browse_response()
{
    uint8_t buf[256];
    size_t n = build_browse(buf, sizeof(buf), 7, 6, 300, 70, 0, 85);
    OpcUaBrowseRequest br;
    TEST_ASSERT_TRUE(opcua_parse_browse(buf, n, &br));

    uint8_t resp[512];
    size_t rn = opcua_build_browse_response(&br, test_browse_handler, 11, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r));   // TokenId
    TEST_ASSERT_EQUAL_UINT32(11, ua_r_u32(&r));  // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(300, ua_r_u32(&r)); // RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_BROWSE_RESP, tid.id);
    (void)ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(70, ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));  // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));  // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r));  // StringTable
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));    // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, ua_r_u8(&r));  // AdditionalHeader body

    // Results[] -> one BrowseResult
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_GOOD, ua_r_u32(&r)); // BrowseResult.StatusCode
    int32_t cp = 0;
    char tmp[8];
    TEST_ASSERT_TRUE(ua_r_string(&r, tmp, sizeof(tmp), &cp)); // ContinuationPoint (null)
    TEST_ASSERT_EQUAL_INT32(-1, cp);
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r)); // References[] count

    // ReferenceDescription
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid)); // ReferenceTypeId
    TEST_ASSERT_EQUAL_UINT32(OPCUA_REFTYPE_ORGANIZES, tid.id);
    TEST_ASSERT_TRUE(ua_r_bool(&r));         // IsForward
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid)); // target NodeId (ExpandedNodeId)
    TEST_ASSERT_EQUAL_UINT16(1, tid.ns);
    TEST_ASSERT_EQUAL_UINT32(1, tid.id);
    // BrowseName (QualifiedName)
    char name[16];
    int32_t nl = 0;
    TEST_ASSERT_EQUAL_UINT16(1, ua_r_u16(&r)); // BrowseName.ns
    TEST_ASSERT_TRUE(ua_r_string(&r, name, sizeof(name), &nl));
    TEST_ASSERT_EQUAL_STRING("Uptime", name);
    // DisplayName (LocalizedText): mask 0x02 (text only)
    TEST_ASSERT_EQUAL_HEX8(0x02, ua_r_u8(&r));
    TEST_ASSERT_TRUE(ua_r_string(&r, name, sizeof(name), &nl));
    TEST_ASSERT_EQUAL_STRING("Uptime", name);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_VARIABLE, ua_r_u32(&r)); // NodeClass
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));                          // TypeDefinition
    TEST_ASSERT_EQUAL_UINT32(OPCUA_TYPEDEF_BASE_DATA_VARIABLE, tid.id);

    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r)); // DiagnosticInfos[]
    TEST_ASSERT_FALSE(r.err);
}

void test_build_browse_response_unknown()
{
    uint8_t buf[256];
    size_t n = build_browse(buf, sizeof(buf), 7, 6, 300, 70, 0, 999);
    OpcUaBrowseRequest br;
    TEST_ASSERT_TRUE(opcua_parse_browse(buf, n, &br));

    uint8_t resp[256];
    size_t rn = opcua_build_browse_response(&br, test_browse_handler, 11, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/Seq/RequestId
    UaNodeId tid;
    ua_r_nodeid(&r, &tid);                                                   // TypeId
    (void)ua_r_u64(&r);                                                      // Timestamp
    (void)ua_r_u32(&r);                                                      // RequestHandle
    (void)ua_r_u32(&r);                                                      // ServiceResult
    (void)ua_r_u8(&r);                                                       // DiagnosticInfo
    (void)ua_r_i32(&r);                                                      // StringTable
    ua_r_nodeid(&r, &tid);                                                   // AdditionalHeader NodeId
    (void)ua_r_u8(&r);                                                       // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));                                // Results count
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, ua_r_u32(&r)); // BrowseResult.StatusCode
    char tmp[4];
    int32_t cp = 0;
    TEST_ASSERT_TRUE(ua_r_string(&r, tmp, sizeof(tmp), &cp)); // ContinuationPoint null
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r));                 // References[] empty
    TEST_ASSERT_FALSE(r.err);
}

void test_build_close_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CLOSE_SESSION_REQ, 7, 7, 400, 80);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));

    uint8_t resp[64];
    size_t rn = opcua_build_close_session_response(&m, 12, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, ua_r_u32(&r));   // TokenId
    TEST_ASSERT_EQUAL_UINT32(12, ua_r_u32(&r));  // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(400, ua_r_u32(&r)); // RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CLOSE_SESSION_RESP, tid.id);
    (void)ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(80, ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));  // ServiceResult Good
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// GetEndpoints + ServiceFault (interop)
// ---------------------------------------------------------------------------

void test_build_get_endpoints()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_GET_ENDPOINTS_REQ, 7, 8, 500, 90);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_GET_ENDPOINTS_REQ, m.type_id);

    OpcUaServerInfo si = {"opc.tcp://test:4840", "urn:test", "TestServer"};
    uint8_t resp[512];
    size_t rn = opcua_build_get_endpoints_response(&m, &si, 9, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/SequenceNumber/RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_GET_ENDPOINTS_RESP, tid.id);
    (void)ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(90, ua_r_u32(&r)); // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));  // ServiceResult Good
    (void)ua_r_u8(&r);                          // DiagnosticInfo
    (void)ua_r_i32(&r);                         // StringTable
    ua_r_nodeid(&r, &tid);                      // AdditionalHeader NodeId
    (void)ua_r_u8(&r);                          // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));   // Endpoints[] count

    // EndpointDescription
    char s[96];
    int32_t sl = 0;
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &sl)); // EndpointUrl
    TEST_ASSERT_EQUAL_STRING("opc.tcp://test:4840", s);
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &sl)); // ApplicationUri
    TEST_ASSERT_EQUAL_STRING("urn:test", s);
    ua_r_string(&r, s, sizeof(s), &sl); // ProductUri
    uint8_t mask = ua_r_u8(&r);         // ApplicationName LocalizedText mask
    if (mask & 0x01)
        ua_r_string(&r, s, sizeof(s), &sl);
    if (mask & 0x02)
        ua_r_string(&r, s, sizeof(s), &sl);
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r)); // ApplicationType = Server
    ua_r_string(&r, s, sizeof(s), &sl);        // GatewayServerUri
    ua_r_string(&r, s, sizeof(s), &sl);        // DiscoveryProfileUri
    TEST_ASSERT_EQUAL_INT32(-1, ua_r_i32(&r)); // DiscoveryUrls[] null
    ua_r_string(&r, s, sizeof(s), &sl);        // ServerCertificate
    TEST_ASSERT_EQUAL_UINT32(1, ua_r_u32(&r)); // MessageSecurityMode = None
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &sl));
    TEST_ASSERT_EQUAL_STRING(OPCUA_POLICY_NONE_URI, s); // SecurityPolicyUri
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));           // UserIdentityTokens[] count
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &sl));
    TEST_ASSERT_EQUAL_STRING("anonymous", s);             // PolicyId
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));            // TokenType = Anonymous
    ua_r_string(&r, s, sizeof(s), &sl);                   // IssuedTokenType
    ua_r_string(&r, s, sizeof(s), &sl);                   // IssuerEndpointUrl
    ua_r_string(&r, s, sizeof(s), &sl);                   // SecurityPolicyUri
    TEST_ASSERT_TRUE(ua_r_string(&r, s, sizeof(s), &sl)); // TransportProfileUri
    (void)ua_r_u8(&r);                                    // SecurityLevel
    TEST_ASSERT_FALSE(r.err);
}

void test_build_service_fault()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), 9999, 7, 9, 600, 91); // 9999 = unsupported service
    OpcUaMsg m;
    TEST_ASSERT_TRUE(opcua_parse_msg(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT32(9999, m.type_id);

    uint8_t resp[64];
    size_t rn = opcua_build_service_fault(&m, OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, 3, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16;
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_SERVICE_FAULT, tid.id);
    (void)ua_r_u64(&r);                                                          // Timestamp
    TEST_ASSERT_EQUAL_UINT32(91, ua_r_u32(&r));                                  // RequestHandle echoed
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, ua_r_u32(&r)); // ServiceResult
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Write service (DataValue decode + WriteRequest/Response)
// ---------------------------------------------------------------------------

void test_datavalue_roundtrip()
{
    uint8_t buf[32];
    UaWriter w = {buf, sizeof(buf), 0, true};
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OPCUA_VAR_DOUBLE;
    v.f64 = 3.25;
    ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD);

    UaReader r = {buf, w.n, 0, false};
    OpcUaVariant got;
    uint32_t st = 0xFFFFFFFFu;
    TEST_ASSERT_TRUE(ua_r_datavalue(&r, &got, &st));
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_DOUBLE, got.type);
    TEST_ASSERT_TRUE(got.f64 == 3.25);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_STATUS_GOOD, st);
    TEST_ASSERT_FALSE(r.err);
}

void test_parse_and_build_write()
{
    // Build a WriteRequest writing one Int32 to ns=1;i=10 (value-only DataValue).
    uint8_t buf[128];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_u8(&w, 'M');
    ua_w_u8(&w, 'S');
    ua_w_u8(&w, 'G');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);   // size placeholder
    ua_w_u32(&w, 0);   // SecureChannelId
    ua_w_u32(&w, 7);   // TokenId
    ua_w_u32(&w, 5);   // SequenceNumber
    ua_w_u32(&w, 700); // RequestId
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_WRITE_REQ);
    ua_w_nodeid_numeric(&w, 0, 0); // RequestHeader: AuthenticationToken
    ua_w_u64(&w, 0);               // Timestamp
    ua_w_u32(&w, 95);              // RequestHandle
    ua_w_u32(&w, 0);               // ReturnDiagnostics
    ua_w_string(&w, nullptr, -1);  // AuditEntryId
    ua_w_u32(&w, 0);               // TimeoutHint
    ua_w_nodeid_numeric(&w, 0, 0);
    ua_w_u8(&w, 0x00);
    ua_w_i32(&w, 1);                // NodesToWrite count
    ua_w_nodeid_numeric(&w, 1, 10); // NodeId ns=1;i=10
    ua_w_u32(&w, OPCUA_ATTR_VALUE); // AttributeId
    ua_w_string(&w, nullptr, -1);   // IndexRange
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OPCUA_VAR_INT32;
    v.i32 = -1234;
    ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD); // Value
    buf[4] = (uint8_t)w.n;
    buf[5] = (uint8_t)(w.n >> 8);
    buf[6] = buf[7] = 0;

    OpcUaWriteRequest wr;
    TEST_ASSERT_TRUE(opcua_parse_write(buf, w.n, &wr));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_WRITE_REQ, wr.msg.type_id);
    TEST_ASSERT_EQUAL_UINT32(95, wr.msg.request_handle);
    TEST_ASSERT_EQUAL_UINT32(1, wr.count);
    TEST_ASSERT_EQUAL_UINT16(1, wr.items[0].ns);
    TEST_ASSERT_EQUAL_UINT32(10, wr.items[0].id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ATTR_VALUE, wr.items[0].attribute);
    TEST_ASSERT_EQUAL_HEX8(OPCUA_VAR_INT32, wr.items[0].value.type);
    TEST_ASSERT_EQUAL_INT32(-1234, wr.items[0].value.i32);

    // Build + parse the WriteResponse.
    uint32_t res[1] = {OPCUA_STATUS_GOOD};
    uint8_t resp[64];
    size_t rn = opcua_build_write_response(&wr, res, 6, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);
    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/Seq/RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_WRITE_RESP, tid.id);
    (void)ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(95, ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));  // ServiceResult Good
    (void)ua_r_u8(&r);                          // DiagnosticInfo
    (void)ua_r_i32(&r);                         // StringTable
    ua_r_nodeid(&r, &tid);                      // AdditionalHeader NodeId
    (void)ua_r_u8(&r);                          // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, ua_r_i32(&r));   // Results count
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_GOOD, ua_r_u32(&r));
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r)); // DiagnosticInfos
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Codec coverage: every Variant / DataValue / NodeId branch + reader underruns
// ---------------------------------------------------------------------------

static void variant_rt(const OpcUaVariant *in, OpcUaVariant *out)
{
    uint8_t buf[64];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_variant(&w, in);
    TEST_ASSERT_TRUE(w.ok);
    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_TRUE(ua_r_variant(&r, out));
    TEST_ASSERT_FALSE(r.err);
}

// Every supported scalar Variant type round-trips (and a null pointer encodes Null).
void test_variant_scalar_types()
{
    OpcUaVariant in, out;
    memset(&in, 0, sizeof(in));

    in.type = OPCUA_VAR_NULL;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_UINT8(OPCUA_VAR_NULL, out.type);

    uint8_t nb[8];
    UaWriter wn = {nb, sizeof(nb), 0, true};
    ua_w_variant(&wn, nullptr); // null pointer -> Null Variant (encoding byte 0)
    TEST_ASSERT_EQUAL_UINT(1, wn.n);
    TEST_ASSERT_EQUAL_UINT8(OPCUA_VAR_NULL, nb[0]);

    in.type = OPCUA_VAR_BOOL;
    in.b = true;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_UINT8(OPCUA_VAR_BOOL, out.type);
    TEST_ASSERT_TRUE(out.b);

    in.type = OPCUA_VAR_INT32;
    in.i32 = -77;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_INT32(-77, out.i32);

    in.type = OPCUA_VAR_UINT32;
    in.u32 = 0xCAFEBABEu;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_HEX32(0xCAFEBABEu, out.u32);

    in.type = OPCUA_VAR_FLOAT;
    in.f32 = 1.25f;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_FLOAT(1.25f, out.f32);

    in.type = OPCUA_VAR_DOUBLE;
    in.f64 = 6.5;
    variant_rt(&in, &out);
    TEST_ASSERT_TRUE(out.f64 == 6.5);

    // STRING decodes as a pointer into the source buffer, so keep the buffer in scope
    // for the assertion (the variant_rt helper's buffer would already be out of scope).
    uint8_t sbuf[32];
    UaWriter ws = {sbuf, sizeof(sbuf), 0, true};
    OpcUaVariant sv;
    memset(&sv, 0, sizeof(sv));
    sv.type = OPCUA_VAR_STRING;
    sv.str = "ab";
    sv.str_len = 2;
    ua_w_variant(&ws, &sv);
    TEST_ASSERT_TRUE(ws.ok);
    UaReader rs = {sbuf, ws.n, 0, false};
    TEST_ASSERT_TRUE(ua_r_variant(&rs, &out));
    TEST_ASSERT_EQUAL_INT32(2, out.str_len);
    TEST_ASSERT_EQUAL_MEMORY("ab", out.str, 2);
}

// A Variant with an unsupported type fails to encode; malformed Variants fail to decode.
void test_variant_errors()
{
    uint8_t buf[16];
    OpcUaVariant bad;
    memset(&bad, 0, sizeof(bad));
    bad.type = 200; // not a supported built-in type
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_variant(&w, &bad);
    TEST_ASSERT_FALSE(w.ok); // fail closed

    OpcUaVariant o;
    uint8_t arr[4] = {0x80, 0, 0, 0}; // array bit set: unsupported by the scalar decoder
    UaReader r1 = {arr, sizeof(arr), 0, false};
    TEST_ASSERT_FALSE(ua_r_variant(&r1, &o));
    TEST_ASSERT_TRUE(r1.err);

    uint8_t ut[1] = {50}; // unsupported built-in type id
    UaReader r2 = {ut, sizeof(ut), 0, false};
    TEST_ASSERT_FALSE(ua_r_variant(&r2, &o));

    uint8_t st[5] = {OPCUA_VAR_STRING, 0x10, 0, 0, 0}; // len=16 but no bytes follow
    UaReader r3 = {st, sizeof(st), 0, false};
    TEST_ASSERT_FALSE(ua_r_variant(&r3, &o));
    TEST_ASSERT_TRUE(r3.err);
}

// A DataValue carrying every optional field round-trips; a malformed inner Variant fails.
void test_datavalue_all_masks()
{
    uint8_t buf[64];
    UaWriter w = {buf, sizeof(buf), 0, true};
    ua_w_u8(&w, 0x3F); // value|status|sourceTS|serverTS|sourcePS|serverPS
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OPCUA_VAR_INT32;
    v.i32 = 9;
    ua_w_variant(&w, &v);
    ua_w_u32(&w, 0x80000000u); // StatusCode (Bad)
    ua_w_u64(&w, 111);         // SourceTimestamp
    ua_w_u16(&w, 5);           // SourcePicoseconds
    ua_w_u64(&w, 222);         // ServerTimestamp
    ua_w_u16(&w, 6);           // ServerPicoseconds
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    OpcUaVariant out;
    uint32_t status = 0;
    TEST_ASSERT_TRUE(ua_r_datavalue(&r, &out, &status));
    TEST_ASSERT_EQUAL_INT32(9, out.i32);
    TEST_ASSERT_EQUAL_HEX32(0x80000000u, status);
    TEST_ASSERT_FALSE(r.err);

    uint8_t badv[2] = {0x01, 0x80}; // Value present, but Variant has the array bit set
    UaReader rb = {badv, sizeof(badv), 0, false};
    OpcUaVariant ov;
    uint32_t os = 0;
    TEST_ASSERT_FALSE(ua_r_datavalue(&rb, &ov, &os));
}

// Every NodeId encoding kind: String, ByteString, Guid, the NamespaceUri/ServerIndex
// flags, and a rejected unknown kind.
void test_nodeid_encodings()
{
    UaNodeId id;

    uint8_t sid[10] = {0x03, 0x02, 0x00, 0x03, 0, 0, 0, 'a', 'b', 'c'}; // String, ns=2, len=3
    UaReader rs = {sid, sizeof(sid), 0, false};
    TEST_ASSERT_TRUE(ua_r_nodeid(&rs, &id));
    TEST_ASSERT_FALSE(id.numeric);
    TEST_ASSERT_EQUAL_UINT16(2, id.ns);

    uint8_t bid[7] = {0x05, 0x00, 0x00, 0x00, 0, 0, 0}; // ByteString, len=0
    UaReader rby = {bid, sizeof(bid), 0, false};
    TEST_ASSERT_TRUE(ua_r_nodeid(&rby, &id));
    TEST_ASSERT_FALSE(id.numeric);

    uint8_t gid[19];
    memset(gid, 0, sizeof(gid));
    gid[0] = 0x04; // Guid, ns + 16 bytes
    gid[1] = 0x01;
    UaReader rg = {gid, sizeof(gid), 0, false};
    TEST_ASSERT_TRUE(ua_r_nodeid(&rg, &id));
    TEST_ASSERT_FALSE(id.numeric);

    uint8_t inv[4] = {0x06, 0, 0, 0}; // unknown kind
    UaReader ri = {inv, sizeof(inv), 0, false};
    TEST_ASSERT_FALSE(ua_r_nodeid(&ri, &id));
    TEST_ASSERT_TRUE(ri.err);

    // TwoByte id with NamespaceUri (0x80) + ServerIndex (0x40): id, nsUri string, index.
    uint8_t fl[12] = {0xC0, 0x05, 0x02, 0, 0, 0, 'n', 's', 0x09, 0, 0, 0};
    UaReader rf = {fl, sizeof(fl), 0, false};
    TEST_ASSERT_TRUE(ua_r_nodeid(&rf, &id));
    TEST_ASSERT_EQUAL_UINT32(5, id.id);
    TEST_ASSERT_FALSE(rf.err);
}

// The bounds-checked primitive readers latch err on underrun / overrun.
void test_reader_underruns()
{
    uint8_t b3[3] = {1, 2, 3};
    UaReader r = {b3, 3, 0, false};
    TEST_ASSERT_EQUAL_UINT64(0, ua_r_u64(&r)); // 8-byte read on 3 bytes
    TEST_ASSERT_TRUE(r.err);

    char s[8];
    int32_t sl = 0;
    UaReader re = {b3, 0, 0, false};
    TEST_ASSERT_FALSE(ua_r_string(&re, s, sizeof(s), &sl)); // length read underruns

    uint8_t big[8] = {0x05, 0, 0, 0, 'a', 'b', 'c', 'd'}; // len=5 into a 4-byte buffer
    char sc[4];
    int32_t scl = 0;
    UaReader rc = {big, sizeof(big), 0, false};
    TEST_ASSERT_FALSE(ua_r_string(&rc, sc, sizeof(sc), &scl)); // exceeds cap

    uint8_t nid[7] = {0x03, 0, 0, 0x10, 0, 0, 0}; // String NodeId, len=16 but no bytes
    UaReader rk = {nid, sizeof(nid), 0, false};
    UaNodeId id;
    TEST_ASSERT_FALSE(ua_r_nodeid(&rk, &id)); // r_skip overruns
    TEST_ASSERT_TRUE(rk.err);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_variant_scalar_types);
    RUN_TEST(test_variant_errors);
    RUN_TEST(test_datavalue_all_masks);
    RUN_TEST(test_nodeid_encodings);
    RUN_TEST(test_reader_underruns);
    RUN_TEST(test_codec_roundtrip);
    RUN_TEST(test_string_null_roundtrip);
    RUN_TEST(test_reader_underrun_latches);
    RUN_TEST(test_writer_overflow_fails_closed);
    RUN_TEST(test_parse_header);
    RUN_TEST(test_parse_hello);
    RUN_TEST(test_parse_hello_rejects_short);
    RUN_TEST(test_build_ack_negotiates);
    RUN_TEST(test_nodeid_roundtrip);
    RUN_TEST(test_filetime_from_unix);
    RUN_TEST(test_parse_open);
    RUN_TEST(test_parse_open_rejects_wrong_type);
    RUN_TEST(test_build_open_response);
    RUN_TEST(test_parse_msg);
    RUN_TEST(test_parse_msg_rejects_non_msg);
    RUN_TEST(test_build_create_session_response);
    RUN_TEST(test_build_activate_session_response);
    RUN_TEST(test_datavalue_good_int32);
    RUN_TEST(test_datavalue_bad_status);
    RUN_TEST(test_parse_read);
    RUN_TEST(test_build_read_response);
    RUN_TEST(test_parse_browse);
    RUN_TEST(test_build_browse_response);
    RUN_TEST(test_build_browse_response_unknown);
    RUN_TEST(test_build_close_session_response);
    RUN_TEST(test_build_get_endpoints);
    RUN_TEST(test_build_service_fault);
    RUN_TEST(test_datavalue_roundtrip);
    RUN_TEST(test_parse_and_build_write);
    return UNITY_END();
}
