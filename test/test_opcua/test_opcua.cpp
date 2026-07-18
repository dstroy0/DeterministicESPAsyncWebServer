// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for OPC UA (services/opcua): the Binary built-in type codec (incl.
// NodeId / DateTime / Variant / DataValue / ReferenceDescription), UACP framing,
// the Hello/Acknowledge handshake, the SecureChannel (OPN), the Session
// (CreateSession/ActivateSession), GetEndpoints, the Read / Write / Browse / CloseSession
// services and the ServiceFault fallback (SecurityPolicy None). The TCP data handler
// (dws_opcua_rx) is ESP32-only and HW-verified (incl. python asyncua interop).

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
    dws_ua_w_bool(&w, true);
    dws_ua_w_u8(&w, 0x7F);
    dws_ua_w_u16(&w, 0xBEEF);
    dws_ua_w_u32(&w, 0xDEADBEEF);
    dws_ua_w_u64(&w, 0x0123456789ABCDEFull);
    dws_ua_w_i32(&w, -12345);
    dws_ua_w_f32(&w, 3.5f);
    dws_ua_w_f64(&w, 2.5);
    dws_ua_w_string(&w, "hello", 5);
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_bool(&r));
    TEST_ASSERT_EQUAL_HEX8(0x7F, dws_ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, dws_ua_r_u16(&r));
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, dws_ua_r_u32(&r));
    TEST_ASSERT_TRUE(dws_ua_r_u64(&r) == 0x0123456789ABCDEFull);
    TEST_ASSERT_EQUAL_INT32(-12345, dws_ua_r_i32(&r));
    TEST_ASSERT_EQUAL_FLOAT(3.5f, dws_ua_r_f32(&r));
    double d = dws_ua_r_f64(&r);
    TEST_ASSERT_TRUE(d == 2.5); // exact in IEEE-754 (avoids Unity's optional double assert)
    char s[16];
    int32_t slen = 0;
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &slen));
    TEST_ASSERT_EQUAL_INT32(5, slen);
    TEST_ASSERT_EQUAL_STRING("hello", s);
    TEST_ASSERT_FALSE(r.err);
}

void test_string_null_roundtrip()
{
    uint8_t buf[8];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_string(&w, nullptr, -1);
    UaReader r = {buf, w.n, 0, false};
    char s[4];
    int32_t slen = 99;
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &slen));
    TEST_ASSERT_EQUAL_INT32(-1, slen);
}

void test_reader_underrun_latches()
{
    uint8_t buf[2] = {1, 2};
    UaReader r = {buf, sizeof(buf), 0, false};
    dws_ua_r_u32(&r); // only 2 bytes available
    TEST_ASSERT_TRUE(r.err);
}

void test_writer_overflow_fails_closed()
{
    uint8_t buf[3];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_u32(&w, 1); // 4 bytes into a 3-byte buffer
    TEST_ASSERT_FALSE(w.ok);
}

// Build a Hello message and assert it parses; then build its Acknowledge.
static size_t build_hello(uint8_t *out, size_t cap, uint32_t recv, uint32_t send, uint32_t maxmsg)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'H');
    dws_ua_w_u8(&w, 'E');
    dws_ua_w_u8(&w, 'L');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0); // size placeholder
    dws_ua_w_u32(&w, 0); // ProtocolVersion
    dws_ua_w_u32(&w, recv);
    dws_ua_w_u32(&w, send);
    dws_ua_w_u32(&w, maxmsg);
    dws_ua_w_u32(&w, 0); // MaxChunkCount
    dws_ua_w_string(&w, "opc.tcp://host:4840", 19);
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
    TEST_ASSERT_TRUE(dws_opcua_parse_header(hel, n, &h));
    TEST_ASSERT_EQUAL_MEMORY("HEL", h.type, 3);
    TEST_ASSERT_EQUAL_CHAR('F', h.chunk);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)n, h.size);
}

void test_parse_hello()
{
    uint8_t hel[128];
    size_t n = build_hello(hel, sizeof(hel), 65535, 32768, 0);
    OpcUaHello hello;
    TEST_ASSERT_TRUE(dws_opcua_parse_hello(hel, n, &hello));
    TEST_ASSERT_EQUAL_UINT32(0, hello.protocol_version);
    TEST_ASSERT_EQUAL_UINT32(65535, hello.recv_buf_size);
    TEST_ASSERT_EQUAL_UINT32(32768, hello.send_buf_size);
}

void test_parse_hello_rejects_short()
{
    uint8_t bad[12] = {'H', 'E', 'L', 'F', 12, 0, 0, 0, 0, 0, 0, 0};
    OpcUaHello hello;
    TEST_ASSERT_FALSE(dws_opcua_parse_hello(bad, sizeof(bad), &hello)); // no room for the 5 sizes
}

void test_build_ack_negotiates()
{
    uint8_t hel[128];
    size_t n = build_hello(hel, sizeof(hel), 65535, 65535, 0);
    OpcUaHello hello;
    TEST_ASSERT_TRUE(dws_opcua_parse_hello(hel, n, &hello));

    uint8_t ack[64];
    size_t an = dws_opcua_build_ack(&hello, ack, sizeof(ack));
    TEST_ASSERT_EQUAL_size_t(28, an); // 8 header + 5 x UInt32

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(ack, an, &h));
    TEST_ASSERT_EQUAL_MEMORY("ACK", h.type, 3);
    TEST_ASSERT_EQUAL_UINT32(28, h.size);

    UaReader r = {ack + 8, an - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));    // ProtocolVersion
    TEST_ASSERT_EQUAL_UINT32(8192, dws_ua_r_u32(&r)); // recv = min(client send 65535, server 8192)
    TEST_ASSERT_EQUAL_UINT32(8192, dws_ua_r_u32(&r)); // send = min(client recv 65535, server 8192)
    TEST_ASSERT_EQUAL_UINT32(8192, dws_ua_r_u32(&r)); // max msg (client 0 -> server)
    TEST_ASSERT_EQUAL_UINT32(1, dws_ua_r_u32(&r));    // max chunk
}

// ---------------------------------------------------------------------------
// Increment 2 - NodeId / DateTime / SecureChannel (OPN)
// ---------------------------------------------------------------------------

void test_nodeid_roundtrip()
{
    uint8_t buf[32];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_nodeid_numeric(&w, 0, 5);     // TwoByte
    dws_ua_w_nodeid_numeric(&w, 0, 446);   // FourByte (id > 255)
    dws_ua_w_nodeid_numeric(&w, 3, 70000); // Numeric (ns + 32-bit id)
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    UaNodeId id;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(0, id.ns);
    TEST_ASSERT_EQUAL_UINT32(5, id.id);
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(0, id.ns);
    TEST_ASSERT_EQUAL_UINT32(446, id.id);
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &id));
    TEST_ASSERT_EQUAL_UINT16(3, id.ns);
    TEST_ASSERT_EQUAL_UINT32(70000, id.id);
    TEST_ASSERT_FALSE(r.err);
}

void test_filetime_from_unix()
{
    TEST_ASSERT_TRUE(dws_opcua_filetime_from_unix(0) == 0);
    TEST_ASSERT_TRUE(dws_opcua_filetime_from_unix(-5) == 0);
    TEST_ASSERT_TRUE(dws_opcua_filetime_from_unix(1) == (11644473600LL + 1) * 10000000LL);
}

// Build a minimal OpenSecureChannelRequest (OPN, SecurityPolicy None).
static size_t build_open(uint8_t *out, size_t cap, uint32_t channel, uint32_t seq, uint32_t req_id, uint32_t handle,
                         uint32_t mode, uint32_t lifetime)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'O');
    dws_ua_w_u8(&w, 'P');
    dws_ua_w_u8(&w, 'N');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0); // size placeholder
    // Asymmetric security header.
    dws_ua_w_u32(&w, channel);
    const char *pol = OPCUA_POLICY_NONE_URI;
    dws_ua_w_string(&w, pol, (int32_t)strlen(pol));
    dws_ua_w_string(&w, nullptr, -1); // SenderCertificate
    dws_ua_w_string(&w, nullptr, -1); // ReceiverCertificateThumbprint
    // Sequence header.
    dws_ua_w_u32(&w, seq);
    dws_ua_w_u32(&w, req_id);
    // Body TypeId.
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_REQ);
    // RequestHeader.
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken (null)
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, handle);          // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, nullptr, -1);  // AuditEntryId
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader: null NodeId ...
    dws_ua_w_u8(&w, 0x00);             // ... + no body
    // OpenSecureChannelRequest body.
    dws_ua_w_u32(&w, 0);              // ClientProtocolVersion
    dws_ua_w_u32(&w, 0);              // RequestType = Issue
    dws_ua_w_u32(&w, mode);           // MessageSecurityMode
    dws_ua_w_string(&w, nullptr, -1); // ClientNonce
    dws_ua_w_u32(&w, lifetime);
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
    TEST_ASSERT_TRUE(dws_opcua_parse_open(buf, n, &oc));
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
    TEST_ASSERT_FALSE(dws_opcua_parse_open(buf, n, &oc));
}

void test_build_open_response()
{
    uint8_t buf[256];
    size_t n = build_open(buf, sizeof(buf), 0, 1, 7, 42, 1, 600000);
    OpcUaOpenChannel oc;
    TEST_ASSERT_TRUE(dws_opcua_parse_open(buf, n, &oc));

    uint8_t resp[256];
    int64_t now = dws_opcua_filetime_from_unix(1700000000LL);
    size_t rn = dws_opcua_build_open_response(&oc, 55, 99, 1, now, 600000, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("OPN", h.type, 3);
    TEST_ASSERT_EQUAL_CHAR('F', h.chunk);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)rn, h.size);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[64];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(55, dws_ua_r_u32(&r)); // SecureChannelId
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl));
    TEST_ASSERT_EQUAL_STRING(OPCUA_POLICY_NONE_URI, str);
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // SenderCertificate
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // ReceiverCertificateThumbprint
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_EQUAL_UINT32(1, dws_ua_r_u32(&r)); // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r)); // RequestId echoed

    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_OPEN_RESP, tid.id); // OpenSecureChannelResponse

    // ResponseHeader.
    TEST_ASSERT_TRUE(dws_ua_r_u64(&r) == (uint64_t)now); // Timestamp
    TEST_ASSERT_EQUAL_UINT32(42, dws_ua_r_u32(&r));      // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));       // ServiceResult = Good
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));       // ServiceDiagnostics
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r));       // StringTable (null)
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));         // AdditionalHeader NodeId (null)
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));       // AdditionalHeader body: none

    // OpenSecureChannelResponse body.
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));                // ServerProtocolVersion
    TEST_ASSERT_EQUAL_UINT32(55, dws_ua_r_u32(&r));               // ChannelId
    TEST_ASSERT_EQUAL_UINT32(99, dws_ua_r_u32(&r));               // TokenId
    TEST_ASSERT_TRUE(dws_ua_r_u64(&r) == (uint64_t)now);          // CreatedAt
    TEST_ASSERT_EQUAL_UINT32(600000, dws_ua_r_u32(&r));           // RevisedLifetime
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce (null)
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
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);     // size placeholder
    dws_ua_w_u32(&w, 0);     // SecureChannelId
    dws_ua_w_u32(&w, token); // SymmetricSecurityHeader.TokenId
    dws_ua_w_u32(&w, seq);   // SequenceHeader.SequenceNumber
    dws_ua_w_u32(&w, req_id);
    dws_ua_w_nodeid_numeric(&w, 0, type_id); // body TypeId
    // RequestHeader
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken (null for CreateSession)
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, handle);          // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, nullptr, -1);  // AuditEntryId
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader: null NodeId ...
    dws_ua_w_u8(&w, 0x00);             // ... + no body
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
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));
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
    TEST_ASSERT_FALSE(dws_opcua_parse_msg(buf, n, &m));
}

void test_build_create_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CREATE_SESSION_REQ, 7, 3, 100, 42);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));

    uint8_t resp[512];
    int64_t now = dws_opcua_filetime_from_unix(1700000000LL);
    OpcUaServerInfo si = {"opc.tcp://test:4840", "urn:test", "TestServer"};
    size_t rn = dws_opcua_build_create_session_response(&m, 0x1001, 0x2002, 1200000.0, &si, 5, now, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)rn, h.size);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[16];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(5, dws_ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(100, dws_ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CREATE_SESSION_RESP, tid.id);
    // ResponseHeader
    TEST_ASSERT_TRUE(dws_ua_r_u64(&r) == (uint64_t)now); // Timestamp
    TEST_ASSERT_EQUAL_UINT32(42, dws_ua_r_u32(&r));      // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));       // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));       // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r));       // StringTable null
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));         // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));       // AdditionalHeader body none
    // CreateSessionResponse body
    UaNodeId sid, atok;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &sid)); // SessionId
    TEST_ASSERT_EQUAL_UINT32(0x1001, sid.id);
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &atok)); // AuthenticationToken
    TEST_ASSERT_EQUAL_UINT32(0x2002, atok.id);
    TEST_ASSERT_TRUE(dws_ua_r_f64(&r) == 1200000.0);              // RevisedSessionTimeout
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce null
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // ServerCertificate null
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));                 // ServerEndpoints[] -> one endpoint
    // The endpoint encoding itself is validated by test_build_get_endpoints; here we
    // just confirm the session fields and a clean parse of the rest of the message.
    TEST_ASSERT_FALSE(r.err);
}

void test_build_activate_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_ACTIVATE_SESSION_REQ, 7, 4, 101, 43);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));

    uint8_t resp[128];
    size_t rn = dws_opcua_build_activate_session_response(&m, 6, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[8];
    int32_t sl = 0;
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(6, dws_ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(101, dws_ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_ACTIVATE_SESSION_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                                       // Timestamp
    TEST_ASSERT_EQUAL_UINT32(43, dws_ua_r_u32(&r));               // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));                // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));                // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r));                // StringTable null
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));                  // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));                // AdditionalHeader body none
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, str, sizeof(str), &sl)); // ServerNonce null
    TEST_ASSERT_EQUAL_INT32(-1, sl);
    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r)); // Results[] empty
    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r)); // DiagnosticInfos[] empty
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
    v.type = OpcUaVariantType::OPCUA_VAR_INT32;
    v.i32 = -7;
    dws_ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD);
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_EQUAL_HEX8(0x01, dws_ua_r_u8(&r));                              // mask: value present
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_INT32, dws_ua_r_u8(&r)); // Variant encoding byte
    TEST_ASSERT_EQUAL_INT32(-7, dws_ua_r_i32(&r));
    TEST_ASSERT_FALSE(r.err);
}

void test_datavalue_bad_status()
{
    uint8_t buf[16];
    UaWriter w = {buf, sizeof(buf), 0, true};
    OpcUaVariant v;
    memset(&v, 0, sizeof(v)); // null Variant
    dws_ua_w_datavalue(&w, &v, OPCUA_STATUS_BAD_NODE_ID_UNKNOWN);

    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_EQUAL_HEX8(0x02, dws_ua_r_u8(&r)); // mask: status only
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, dws_ua_r_u32(&r));
    TEST_ASSERT_FALSE(r.err);
}

// Build a ReadRequest MSG reading ns=1 numeric ids (AttributeId = Value).
static size_t build_read(uint8_t *out, size_t cap, uint32_t token, uint32_t seq, uint32_t req_id, uint32_t handle,
                         const uint32_t *ids, uint32_t n)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 0); // SecureChannelId
    dws_ua_w_u32(&w, token);
    dws_ua_w_u32(&w, seq);
    dws_ua_w_u32(&w, req_id);
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_READ_REQ);
    // RequestHeader
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u64(&w, 0);
    dws_ua_w_u32(&w, handle);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_string(&w, nullptr, -1);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u8(&w, 0x00);
    // ReadRequest body
    dws_ua_w_f64(&w, 0.0);        // MaxAge
    dws_ua_w_u32(&w, 0);          // TimestampsToReturn
    dws_ua_w_i32(&w, (int32_t)n); // NodesToRead count
    for (uint32_t i = 0; i < n; i++)
    {
        dws_ua_w_nodeid_numeric(&w, 1, ids[i]); // NodeId (ns=1)
        dws_ua_w_u32(&w, OPCUA_ATTR_VALUE);     // AttributeId
        dws_ua_w_string(&w, nullptr, -1);       // IndexRange
        dws_ua_w_u16(&w, 0);                    // DataEncoding QualifiedName.ns
        dws_ua_w_string(&w, nullptr, -1);       // QualifiedName.name
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
    TEST_ASSERT_TRUE(dws_opcua_parse_read(buf, n, &rr));
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
    TEST_ASSERT_TRUE(dws_opcua_parse_read(buf, n, &rr));

    OpcUaVariant vals[2];
    uint32_t sts[2];
    memset(vals, 0, sizeof(vals));
    vals[0].type = OpcUaVariantType::OPCUA_VAR_INT32;
    vals[0].i32 = 4242;
    sts[0] = OPCUA_STATUS_GOOD;
    vals[1].type = OpcUaVariantType::OPCUA_VAR_NULL;
    sts[1] = OPCUA_STATUS_BAD_NODE_ID_UNKNOWN;

    uint8_t resp[256];
    size_t rn = dws_opcua_build_read_response(&rr, vals, sts, 9, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r));   // TokenId echoed
    TEST_ASSERT_EQUAL_UINT32(9, dws_ua_r_u32(&r));   // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(200, dws_ua_r_u32(&r)); // RequestId echoed
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_READ_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(60, dws_ua_r_u32(&r)); // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));  // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));  // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r));  // StringTable null
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));    // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));  // AdditionalHeader body none

    // Results[]
    TEST_ASSERT_EQUAL_INT32(2, dws_ua_r_i32(&r)); // count
    // result 0: Good Int32 4242
    TEST_ASSERT_EQUAL_HEX8(0x01, dws_ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_INT32, dws_ua_r_u8(&r));
    TEST_ASSERT_EQUAL_INT32(4242, dws_ua_r_i32(&r));
    // result 1: Bad status, no value
    TEST_ASSERT_EQUAL_HEX8(0x02, dws_ua_r_u8(&r));
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, dws_ua_r_u32(&r));
    // DiagnosticInfos[]
    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r));
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
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 0); // SecureChannelId
    dws_ua_w_u32(&w, token);
    dws_ua_w_u32(&w, seq);
    dws_ua_w_u32(&w, req_id);
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_BROWSE_REQ);
    // RequestHeader
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u64(&w, 0);
    dws_ua_w_u32(&w, handle);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_string(&w, nullptr, -1);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u8(&w, 0x00);
    // BrowseRequest body
    dws_ua_w_nodeid_numeric(&w, 0, 0);   // View.ViewId
    dws_ua_w_u64(&w, 0);                 // View.Timestamp
    dws_ua_w_u32(&w, 0);                 // View.ViewVersion
    dws_ua_w_u32(&w, 0);                 // RequestedMaxReferencesPerNode
    dws_ua_w_i32(&w, 1);                 // NodesToBrowse count
    dws_ua_w_nodeid_numeric(&w, ns, id); // BrowseDescription.NodeId
    dws_ua_w_u32(&w, 0);                 // BrowseDirection (Forward)
    dws_ua_w_nodeid_numeric(&w, 0, 0);   // ReferenceTypeId (null = all)
    dws_ua_w_bool(&w, true);             // IncludeSubtypes
    dws_ua_w_u32(&w, 0);                 // NodeClassMask
    dws_ua_w_u32(&w, 0x3F);              // ResultMask (all)
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
    TEST_ASSERT_TRUE(dws_opcua_parse_browse(buf, n, &br));
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
    TEST_ASSERT_TRUE(dws_opcua_parse_browse(buf, n, &br));

    uint8_t resp[512];
    size_t rn = dws_opcua_build_browse_response(&br, test_browse_handler, 11, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r));   // TokenId
    TEST_ASSERT_EQUAL_UINT32(11, dws_ua_r_u32(&r));  // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(300, dws_ua_r_u32(&r)); // RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_BROWSE_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(70, dws_ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));  // ServiceResult Good
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));  // DiagnosticInfo
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r));  // StringTable
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));    // AdditionalHeader NodeId
    TEST_ASSERT_EQUAL_HEX8(0x00, dws_ua_r_u8(&r));  // AdditionalHeader body

    // Results[] -> one BrowseResult
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_GOOD, dws_ua_r_u32(&r)); // BrowseResult.StatusCode
    int32_t cp = 0;
    char tmp[8];
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, tmp, sizeof(tmp), &cp)); // ContinuationPoint (null)
    TEST_ASSERT_EQUAL_INT32(-1, cp);
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r)); // References[] count

    // ReferenceDescription
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid)); // ReferenceTypeId
    TEST_ASSERT_EQUAL_UINT32(OPCUA_REFTYPE_ORGANIZES, tid.id);
    TEST_ASSERT_TRUE(dws_ua_r_bool(&r));         // IsForward
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid)); // target NodeId (ExpandedNodeId)
    TEST_ASSERT_EQUAL_UINT16(1, tid.ns);
    TEST_ASSERT_EQUAL_UINT32(1, tid.id);
    // BrowseName (QualifiedName)
    char name[16];
    int32_t nl = 0;
    TEST_ASSERT_EQUAL_UINT16(1, dws_ua_r_u16(&r)); // BrowseName.ns
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, name, sizeof(name), &nl));
    TEST_ASSERT_EQUAL_STRING("Uptime", name);
    // DisplayName (LocalizedText): mask 0x02 (text only)
    TEST_ASSERT_EQUAL_HEX8(0x02, dws_ua_r_u8(&r));
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, name, sizeof(name), &nl));
    TEST_ASSERT_EQUAL_STRING("Uptime", name);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_NODECLASS_VARIABLE, dws_ua_r_u32(&r)); // NodeClass
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));                          // TypeDefinition
    TEST_ASSERT_EQUAL_UINT32(OPCUA_TYPEDEF_BASE_DATA_VARIABLE, tid.id);

    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r)); // DiagnosticInfos[]
    TEST_ASSERT_FALSE(r.err);
}

void test_build_browse_response_unknown()
{
    uint8_t buf[256];
    size_t n = build_browse(buf, sizeof(buf), 7, 6, 300, 70, 0, 999);
    OpcUaBrowseRequest br;
    TEST_ASSERT_TRUE(dws_opcua_parse_browse(buf, n, &br));

    uint8_t resp[256];
    size_t rn = dws_opcua_build_browse_response(&br, test_browse_handler, 11, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/Seq/RequestId
    UaNodeId tid;
    dws_ua_r_nodeid(&r, &tid);                                                   // TypeId
    (void)dws_ua_r_u64(&r);                                                      // Timestamp
    (void)dws_ua_r_u32(&r);                                                      // RequestHandle
    (void)dws_ua_r_u32(&r);                                                      // ServiceResult
    (void)dws_ua_r_u8(&r);                                                       // DiagnosticInfo
    (void)dws_ua_r_i32(&r);                                                      // StringTable
    dws_ua_r_nodeid(&r, &tid);                                                   // AdditionalHeader NodeId
    (void)dws_ua_r_u8(&r);                                                       // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));                                // Results count
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_NODE_ID_UNKNOWN, dws_ua_r_u32(&r)); // BrowseResult.StatusCode
    char tmp[4];
    int32_t cp = 0;
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, tmp, sizeof(tmp), &cp)); // ContinuationPoint null
    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r));                 // References[] empty
    TEST_ASSERT_FALSE(r.err);
}

void test_build_close_session_response()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), OPCUA_ID_CLOSE_SESSION_REQ, 7, 7, 400, 80);
    OpcUaMsg m;
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));

    uint8_t resp[64];
    size_t rn = dws_opcua_build_close_session_response(&m, 12, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);

    UaReader r = {resp + 8, rn - 8, 0, false};
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));   // SecureChannelId echoed
    TEST_ASSERT_EQUAL_UINT32(7, dws_ua_r_u32(&r));   // TokenId
    TEST_ASSERT_EQUAL_UINT32(12, dws_ua_r_u32(&r));  // SequenceNumber
    TEST_ASSERT_EQUAL_UINT32(400, dws_ua_r_u32(&r)); // RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_CLOSE_SESSION_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(80, dws_ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));  // ServiceResult Good
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
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_GET_ENDPOINTS_REQ, m.type_id);

    OpcUaServerInfo si = {"opc.tcp://test:4840", "urn:test", "TestServer"};
    uint8_t resp[512];
    size_t rn = dws_opcua_build_get_endpoints_response(&m, &si, 9, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/SequenceNumber/RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_GET_ENDPOINTS_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(90, dws_ua_r_u32(&r)); // RequestHandle echoed
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));  // ServiceResult Good
    (void)dws_ua_r_u8(&r);                          // DiagnosticInfo
    (void)dws_ua_r_i32(&r);                         // StringTable
    dws_ua_r_nodeid(&r, &tid);                      // AdditionalHeader NodeId
    (void)dws_ua_r_u8(&r);                          // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));   // Endpoints[] count

    // EndpointDescription
    char s[96];
    int32_t sl = 0;
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &sl)); // EndpointUrl
    TEST_ASSERT_EQUAL_STRING("opc.tcp://test:4840", s);
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &sl)); // ApplicationUri
    TEST_ASSERT_EQUAL_STRING("urn:test", s);
    dws_ua_r_string(&r, s, sizeof(s), &sl); // ProductUri
    uint8_t mask = dws_ua_r_u8(&r);         // ApplicationName LocalizedText mask
    if (mask & 0x01)
        dws_ua_r_string(&r, s, sizeof(s), &sl);
    if (mask & 0x02)
        dws_ua_r_string(&r, s, sizeof(s), &sl);
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r)); // ApplicationType = Server
    dws_ua_r_string(&r, s, sizeof(s), &sl);        // GatewayServerUri
    dws_ua_r_string(&r, s, sizeof(s), &sl);        // DiscoveryProfileUri
    TEST_ASSERT_EQUAL_INT32(-1, dws_ua_r_i32(&r)); // DiscoveryUrls[] null
    dws_ua_r_string(&r, s, sizeof(s), &sl);        // ServerCertificate
    TEST_ASSERT_EQUAL_UINT32(1, dws_ua_r_u32(&r)); // MessageSecurityMode = None
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &sl));
    TEST_ASSERT_EQUAL_STRING(OPCUA_POLICY_NONE_URI, s); // SecurityPolicyUri
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));       // UserIdentityTokens[] count
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &sl));
    TEST_ASSERT_EQUAL_STRING("anonymous", s);                 // PolicyId
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));            // TokenType = Anonymous
    dws_ua_r_string(&r, s, sizeof(s), &sl);                   // IssuedTokenType
    dws_ua_r_string(&r, s, sizeof(s), &sl);                   // IssuerEndpointUrl
    dws_ua_r_string(&r, s, sizeof(s), &sl);                   // SecurityPolicyUri
    TEST_ASSERT_TRUE(dws_ua_r_string(&r, s, sizeof(s), &sl)); // TransportProfileUri
    (void)dws_ua_r_u8(&r);                                    // SecurityLevel
    TEST_ASSERT_FALSE(r.err);
}

void test_build_service_fault()
{
    uint8_t buf[128];
    size_t n = build_msg(buf, sizeof(buf), 9999, 7, 9, 600, 91); // 9999 = unsupported service
    OpcUaMsg m;
    TEST_ASSERT_TRUE(dws_opcua_parse_msg(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT32(9999, m.type_id);

    uint8_t resp[64];
    size_t rn = dws_opcua_build_service_fault(&m, OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, 3, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16;
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_SERVICE_FAULT, tid.id);
    (void)dws_ua_r_u64(&r);                                                          // Timestamp
    TEST_ASSERT_EQUAL_UINT32(91, dws_ua_r_u32(&r));                                  // RequestHandle echoed
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED, dws_ua_r_u32(&r)); // ServiceResult
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
    v.type = OpcUaVariantType::OPCUA_VAR_DOUBLE;
    v.f64 = 3.25;
    dws_ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD);

    UaReader r = {buf, w.n, 0, false};
    OpcUaVariant got;
    uint32_t st = 0xFFFFFFFFu;
    TEST_ASSERT_TRUE(dws_ua_r_datavalue(&r, &got, &st));
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_DOUBLE, got.type);
    TEST_ASSERT_TRUE(got.f64 == 3.25);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_STATUS_GOOD, st);
    TEST_ASSERT_FALSE(r.err);
}

void test_parse_and_build_write()
{
    // Build a WriteRequest writing one Int32 to ns=1;i=10 (value-only DataValue).
    uint8_t buf[128];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);   // size placeholder
    dws_ua_w_u32(&w, 0);   // SecureChannelId
    dws_ua_w_u32(&w, 7);   // TokenId
    dws_ua_w_u32(&w, 5);   // SequenceNumber
    dws_ua_w_u32(&w, 700); // RequestId
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_WRITE_REQ);
    dws_ua_w_nodeid_numeric(&w, 0, 0); // RequestHeader: AuthenticationToken
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, 95);              // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, nullptr, -1);  // AuditEntryId
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u8(&w, 0x00);
    dws_ua_w_i32(&w, 1);                // NodesToWrite count
    dws_ua_w_nodeid_numeric(&w, 1, 10); // NodeId ns=1;i=10
    dws_ua_w_u32(&w, OPCUA_ATTR_VALUE); // AttributeId
    dws_ua_w_string(&w, nullptr, -1);   // IndexRange
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OpcUaVariantType::OPCUA_VAR_INT32;
    v.i32 = -1234;
    dws_ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD); // Value
    buf[4] = (uint8_t)w.n;
    buf[5] = (uint8_t)(w.n >> 8);
    buf[6] = buf[7] = 0;

    OpcUaWriteRequest wr;
    TEST_ASSERT_TRUE(dws_opcua_parse_write(buf, w.n, &wr));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_WRITE_REQ, wr.msg.type_id);
    TEST_ASSERT_EQUAL_UINT32(95, wr.msg.request_handle);
    TEST_ASSERT_EQUAL_UINT32(1, wr.count);
    TEST_ASSERT_EQUAL_UINT16(1, wr.items[0].ns);
    TEST_ASSERT_EQUAL_UINT32(10, wr.items[0].id);
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ATTR_VALUE, wr.items[0].attribute);
    TEST_ASSERT_EQUAL_HEX8(OpcUaVariantType::OPCUA_VAR_INT32, wr.items[0].value.type);
    TEST_ASSERT_EQUAL_INT32(-1234, wr.items[0].value.i32);

    // Build + parse the WriteResponse.
    uint32_t res[1] = {OPCUA_STATUS_GOOD};
    uint8_t resp[64];
    size_t rn = dws_opcua_build_write_response(&wr, res, 6, 0, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);
    UaMsgHeader h;
    TEST_ASSERT_TRUE(dws_opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
    UaReader r = {resp + 8, rn - 8, 0, false};
    r.off = 16; // skip SecureChannelId/TokenId/Seq/RequestId
    UaNodeId tid;
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&r, &tid));
    TEST_ASSERT_EQUAL_UINT32(OPCUA_ID_WRITE_RESP, tid.id);
    (void)dws_ua_r_u64(&r);                         // Timestamp
    TEST_ASSERT_EQUAL_UINT32(95, dws_ua_r_u32(&r)); // RequestHandle
    TEST_ASSERT_EQUAL_UINT32(0, dws_ua_r_u32(&r));  // ServiceResult Good
    (void)dws_ua_r_u8(&r);                          // DiagnosticInfo
    (void)dws_ua_r_i32(&r);                         // StringTable
    dws_ua_r_nodeid(&r, &tid);                      // AdditionalHeader NodeId
    (void)dws_ua_r_u8(&r);                          // AdditionalHeader body
    TEST_ASSERT_EQUAL_INT32(1, dws_ua_r_i32(&r));   // Results count
    TEST_ASSERT_EQUAL_HEX32(OPCUA_STATUS_GOOD, dws_ua_r_u32(&r));
    TEST_ASSERT_EQUAL_INT32(0, dws_ua_r_i32(&r)); // DiagnosticInfos
    TEST_ASSERT_FALSE(r.err);
}

// ---------------------------------------------------------------------------
// Codec coverage: every Variant / DataValue / NodeId branch + reader underruns
// ---------------------------------------------------------------------------

static void variant_rt(const OpcUaVariant *in, OpcUaVariant *out)
{
    uint8_t buf[64];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_variant(&w, in);
    TEST_ASSERT_TRUE(w.ok);
    UaReader r = {buf, w.n, 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_variant(&r, out));
    TEST_ASSERT_FALSE(r.err);
}

// Every supported scalar Variant type round-trips (and a null pointer encodes Null).
void test_variant_scalar_types()
{
    OpcUaVariant in, out;
    memset(&in, 0, sizeof(in));

    in.type = OpcUaVariantType::OPCUA_VAR_NULL;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_UINT8(OpcUaVariantType::OPCUA_VAR_NULL, out.type);

    uint8_t nb[8];
    UaWriter wn = {nb, sizeof(nb), 0, true};
    dws_ua_w_variant(&wn, nullptr); // null pointer -> Null Variant (encoding byte 0)
    TEST_ASSERT_EQUAL_UINT(1, wn.n);
    TEST_ASSERT_EQUAL_UINT8(OpcUaVariantType::OPCUA_VAR_NULL, nb[0]);

    in.type = OpcUaVariantType::OPCUA_VAR_BOOL;
    in.b = true;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_UINT8(OpcUaVariantType::OPCUA_VAR_BOOL, out.type);
    TEST_ASSERT_TRUE(out.b);

    in.type = OpcUaVariantType::OPCUA_VAR_INT32;
    in.i32 = -77;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_INT32(-77, out.i32);

    in.type = OpcUaVariantType::OPCUA_VAR_UINT32;
    in.u32 = 0xCAFEBABEu;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_HEX32(0xCAFEBABEu, out.u32);

    in.type = OpcUaVariantType::OPCUA_VAR_FLOAT;
    in.f32 = 1.25f;
    variant_rt(&in, &out);
    TEST_ASSERT_EQUAL_FLOAT(1.25f, out.f32);

    in.type = OpcUaVariantType::OPCUA_VAR_DOUBLE;
    in.f64 = 6.5;
    variant_rt(&in, &out);
    TEST_ASSERT_TRUE(out.f64 == 6.5);

    // STRING decodes as a pointer into the source buffer, so keep the buffer in scope
    // for the assertion (the variant_rt helper's buffer would already be out of scope).
    uint8_t sbuf[32];
    UaWriter ws = {sbuf, sizeof(sbuf), 0, true};
    OpcUaVariant sv;
    memset(&sv, 0, sizeof(sv));
    sv.type = OpcUaVariantType::OPCUA_VAR_STRING;
    sv.str = "ab";
    sv.str_len = 2;
    dws_ua_w_variant(&ws, &sv);
    TEST_ASSERT_TRUE(ws.ok);
    UaReader rs = {sbuf, ws.n, 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_variant(&rs, &out));
    TEST_ASSERT_EQUAL_INT32(2, out.str_len);
    TEST_ASSERT_EQUAL_MEMORY("ab", out.str, 2);
}

// A Variant with an unsupported type fails to encode; malformed Variants fail to decode.
void test_variant_errors()
{
    uint8_t buf[16];
    OpcUaVariant bad;
    memset(&bad, 0, sizeof(bad));
    bad.type = (OpcUaVariantType)200; // not a supported built-in type
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_variant(&w, &bad);
    TEST_ASSERT_FALSE(w.ok); // fail closed

    OpcUaVariant o;
    uint8_t arr[4] = {0x80, 0, 0, 0}; // array bit set: unsupported by the scalar decoder
    UaReader r1 = {arr, sizeof(arr), 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_variant(&r1, &o));
    TEST_ASSERT_TRUE(r1.err);

    uint8_t ut[1] = {50}; // unsupported built-in type id
    UaReader r2 = {ut, sizeof(ut), 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_variant(&r2, &o));

    uint8_t st[5] = {(uint8_t)OpcUaVariantType::OPCUA_VAR_STRING, 0x10, 0, 0, 0}; // len=16 but no bytes follow
    UaReader r3 = {st, sizeof(st), 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_variant(&r3, &o));
    TEST_ASSERT_TRUE(r3.err);
}

// A DataValue carrying every optional field round-trips; a malformed inner Variant fails.
void test_datavalue_all_masks()
{
    uint8_t buf[64];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_u8(&w, 0x3F); // value|status|sourceTS|serverTS|sourcePS|serverPS
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OpcUaVariantType::OPCUA_VAR_INT32;
    v.i32 = 9;
    dws_ua_w_variant(&w, &v);
    dws_ua_w_u32(&w, 0x80000000u); // StatusCode (Bad)
    dws_ua_w_u64(&w, 111);         // SourceTimestamp
    dws_ua_w_u16(&w, 5);           // SourcePicoseconds
    dws_ua_w_u64(&w, 222);         // ServerTimestamp
    dws_ua_w_u16(&w, 6);           // ServerPicoseconds
    TEST_ASSERT_TRUE(w.ok);

    UaReader r = {buf, w.n, 0, false};
    OpcUaVariant out;
    uint32_t status = 0;
    TEST_ASSERT_TRUE(dws_ua_r_datavalue(&r, &out, &status));
    TEST_ASSERT_EQUAL_INT32(9, out.i32);
    TEST_ASSERT_EQUAL_HEX32(0x80000000u, status);
    TEST_ASSERT_FALSE(r.err);

    uint8_t badv[2] = {0x01, 0x80}; // Value present, but Variant has the array bit set
    UaReader rb = {badv, sizeof(badv), 0, false};
    OpcUaVariant ov;
    uint32_t os = 0;
    TEST_ASSERT_FALSE(dws_ua_r_datavalue(&rb, &ov, &os));
}

// Every NodeId encoding kind: String, ByteString, Guid, the NamespaceUri/ServerIndex
// flags, and a rejected unknown kind.
void test_nodeid_encodings()
{
    UaNodeId id;

    uint8_t sid[10] = {0x03, 0x02, 0x00, 0x03, 0, 0, 0, 'a', 'b', 'c'}; // String, ns=2, len=3
    UaReader rs = {sid, sizeof(sid), 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&rs, &id));
    TEST_ASSERT_FALSE(id.numeric);
    TEST_ASSERT_EQUAL_UINT16(2, id.ns);

    uint8_t bid[7] = {0x05, 0x00, 0x00, 0x00, 0, 0, 0}; // ByteString, len=0
    UaReader rby = {bid, sizeof(bid), 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&rby, &id));
    TEST_ASSERT_FALSE(id.numeric);

    uint8_t gid[19];
    memset(gid, 0, sizeof(gid));
    gid[0] = 0x04; // Guid, ns + 16 bytes
    gid[1] = 0x01;
    UaReader rg = {gid, sizeof(gid), 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&rg, &id));
    TEST_ASSERT_FALSE(id.numeric);

    uint8_t inv[4] = {0x06, 0, 0, 0}; // unknown kind
    UaReader ri = {inv, sizeof(inv), 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_nodeid(&ri, &id));
    TEST_ASSERT_TRUE(ri.err);

    // TwoByte id with NamespaceUri (0x80) + ServerIndex (0x40): id, nsUri string, index.
    uint8_t fl[12] = {0xC0, 0x05, 0x02, 0, 0, 0, 'n', 's', 0x09, 0, 0, 0};
    UaReader rf = {fl, sizeof(fl), 0, false};
    TEST_ASSERT_TRUE(dws_ua_r_nodeid(&rf, &id));
    TEST_ASSERT_EQUAL_UINT32(5, id.id);
    TEST_ASSERT_FALSE(rf.err);
}

// The bounds-checked primitive readers latch err on underrun / overrun.
void test_reader_underruns()
{
    uint8_t b3[3] = {1, 2, 3};
    UaReader r = {b3, 3, 0, false};
    TEST_ASSERT_EQUAL_UINT64(0, dws_ua_r_u64(&r)); // 8-byte read on 3 bytes
    TEST_ASSERT_TRUE(r.err);

    char s[8];
    int32_t sl = 0;
    UaReader re = {b3, 0, 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_string(&re, s, sizeof(s), &sl)); // length read underruns

    uint8_t big[8] = {0x05, 0, 0, 0, 'a', 'b', 'c', 'd'}; // len=5 into a 4-byte buffer
    char sc[4];
    int32_t scl = 0;
    UaReader rc = {big, sizeof(big), 0, false};
    TEST_ASSERT_FALSE(dws_ua_r_string(&rc, sc, sizeof(sc), &scl)); // exceeds cap

    uint8_t nid[7] = {0x03, 0, 0, 0x10, 0, 0, 0}; // String NodeId, len=16 but no bytes
    UaReader rk = {nid, sizeof(nid), 0, false};
    UaNodeId id;
    TEST_ASSERT_FALSE(dws_ua_r_nodeid(&rk, &id)); // r_skip overruns
    TEST_ASSERT_TRUE(rk.err);
}

// A ReadRequest exercising the optional request-header and per-node fields: a
// non-empty AuditEntryId, an AdditionalHeader ExtensionObject *with* a body, and a
// node carrying an IndexRange and a DataEncoding QualifiedName.
static size_t build_read_full(uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0); // size placeholder
    dws_ua_w_u32(&w, 0); // SecureChannelId
    dws_ua_w_u32(&w, 7); // TokenId
    dws_ua_w_u32(&w, 1); // SequenceNumber
    dws_ua_w_u32(&w, 2); // RequestId
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_READ_REQ);
    // RequestHeader with a non-empty AuditEntryId and an AdditionalHeader body.
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, 42);              // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, "audit", 5);   // AuditEntryId (non-empty -> skip path)
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader NodeId (null)
    dws_ua_w_u8(&w, 0x01);             // ExtensionObject body encoding = ByteString
    dws_ua_w_string(&w, "xx", 2);      // ... body bytes (skipped)
    // ReadRequest body.
    dws_ua_w_f64(&w, 0.0);              // MaxAge
    dws_ua_w_u32(&w, 0);                // TimestampsToReturn
    dws_ua_w_i32(&w, 1);                // NodesToRead count
    dws_ua_w_nodeid_numeric(&w, 1, 5);  // NodeId
    dws_ua_w_u32(&w, OPCUA_ATTR_VALUE); // AttributeId
    dws_ua_w_string(&w, "0:1", 3);      // IndexRange (non-empty -> skip path)
    dws_ua_w_u16(&w, 0);                // DataEncoding QualifiedName.ns
    dws_ua_w_string(&w, "Default", 7);  // QualifiedName.name (non-empty -> skip path)
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.ok ? w.n : 0;
}

// The optional RequestHeader (AuditEntryId, AdditionalHeader body) and per-node
// (IndexRange, DataEncoding) fields are consumed correctly.
void test_parse_read_optional_fields()
{
    uint8_t buf[160];
    size_t n = build_read_full(buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    OpcUaReadRequest req;
    TEST_ASSERT_TRUE(dws_opcua_parse_read(buf, n, &req));
    TEST_ASSERT_EQUAL_UINT32(1, req.count);
    TEST_ASSERT_EQUAL_UINT32(5, req.items[0].id);
    TEST_ASSERT_EQUAL_UINT32(42, req.msg.request_handle);
}

// Every parse entry point rejects a header underrun, the wrong message type, a
// size-field mismatch and a corrupt body TypeId NodeId.
void test_parse_rejections()
{
    UaMsgHeader h;
    uint8_t tiny[8] = {0};
    TEST_ASSERT_FALSE(dws_opcua_parse_header(nullptr, 8, &h));
    TEST_ASSERT_FALSE(dws_opcua_parse_header(tiny, 4, &h)); // len < 8
    TEST_ASSERT_FALSE(dws_opcua_parse_header(tiny, 8, nullptr));

    uint8_t buf[128];
    OpcUaHello hello;
    size_t hn = build_hello(buf, sizeof(buf), 1, 2, 3);
    buf[0] = 'X';
    TEST_ASSERT_FALSE(dws_opcua_parse_hello(buf, hn, &hello)); // wrong type

    OpcUaOpenChannel oc;
    size_t on = build_open(buf, sizeof(buf), 1, 1, 1, 1, 1, 3600000);
    TEST_ASSERT_FALSE(dws_opcua_parse_open(buf, on - 1, &oc)); // size mismatch

    OpcUaMsg m;
    size_t mn = build_msg(buf, sizeof(buf), OPCUA_ID_CREATE_SESSION_REQ, 7, 3, 100, 42);
    TEST_ASSERT_FALSE(dws_opcua_parse_msg(buf, mn - 1, &m)); // size mismatch
    buf[24] = 0x06;                                          // corrupt body TypeId NodeId kind
    TEST_ASSERT_FALSE(dws_opcua_parse_msg(buf, mn, &m));

    OpcUaReadRequest rr;
    size_t rn = build_msg(buf, sizeof(buf), OPCUA_ID_READ_REQ, 7, 3, 100, 42);
    buf[0] = 'X';
    TEST_ASSERT_FALSE(dws_opcua_parse_read(buf, rn, &rr)); // wrong type
    buf[0] = 'M';
    TEST_ASSERT_FALSE(dws_opcua_parse_read(buf, rn - 1, &rr)); // size mismatch
    buf[24] = 0x06;
    TEST_ASSERT_FALSE(dws_opcua_parse_read(buf, rn, &rr)); // bad TypeId

    OpcUaWriteRequest wr;
    size_t wn = build_msg(buf, sizeof(buf), OPCUA_ID_WRITE_REQ, 7, 3, 100, 42);
    buf[0] = 'X';
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, wn, &wr));
    buf[0] = 'M';
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, wn - 1, &wr));
    buf[24] = 0x06;
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, wn, &wr));

    OpcUaBrowseRequest br;
    size_t bn = build_msg(buf, sizeof(buf), OPCUA_ID_BROWSE_REQ, 7, 3, 100, 42);
    buf[0] = 'X';
    TEST_ASSERT_FALSE(dws_opcua_parse_browse(buf, bn, &br));
    buf[0] = 'M';
    TEST_ASSERT_FALSE(dws_opcua_parse_browse(buf, bn - 1, &br));
    buf[24] = 0x06;
    TEST_ASSERT_FALSE(dws_opcua_parse_browse(buf, bn, &br));
}

// Every response builder rejects null args and fails closed when the output buffer
// overflows.
void test_build_guards_and_overflow()
{
    uint8_t tiny[4], big[256];
    OpcUaHello hello;
    OpcUaOpenChannel oc;
    OpcUaMsg msg;
    OpcUaReadRequest rr;
    OpcUaWriteRequest wr;
    OpcUaBrowseRequest br;
    OpcUaServerInfo info;
    memset(&hello, 0, sizeof(hello));
    memset(&oc, 0, sizeof(oc));
    memset(&msg, 0, sizeof(msg));
    memset(&rr, 0, sizeof(rr));
    memset(&wr, 0, sizeof(wr));
    memset(&br, 0, sizeof(br));
    memset(&info, 0, sizeof(info));

    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_ack(nullptr, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_ack(&hello, nullptr, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_open_response(nullptr, 1, 1, 1, 0, 1, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0,
                           dws_opcua_build_create_session_response(nullptr, 1, 1, 0.0, &info, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_get_endpoints_response(nullptr, &info, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_service_fault(nullptr, 0, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_activate_session_response(nullptr, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_read_response(nullptr, nullptr, nullptr, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_write_response(nullptr, nullptr, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_browse_response(nullptr, nullptr, 1, 0, big, sizeof(big)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_close_session_response(nullptr, 1, 0, big, sizeof(big)));

    // Tiny output buffer -> writer overflow -> 0 (open-response w.ok guard + patch_size).
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_ack(&hello, tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_open_response(&oc, 1, 1, 1, 0, 1, tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_UINT(0,
                           dws_opcua_build_create_session_response(&msg, 1, 1, 0.0, &info, 1, 0, tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_UINT(0, dws_opcua_build_read_response(&rr, nullptr, nullptr, 1, 0, tiny, sizeof(tiny)));
}

// The application-handler setters and the endpoint-URL setter run; the endpoint
// description serializes with a custom ServerInfo.
void test_setters_and_endpoint_url()
{
    dws_opcua_set_read_handler(nullptr);
    dws_opcua_set_write_handler(nullptr);
    dws_opcua_set_browse_handler(nullptr);
    dws_opcua_set_endpoint_url("opc.tcp://custom:4840");

    OpcUaServerInfo info = {"opc.tcp://custom:4840", "urn:test", "TestServer"};
    OpcUaMsg m;
    memset(&m, 0, sizeof(m));
    uint8_t buf[512]; // the endpoint description (transport/policy URIs) is ~250 bytes
    TEST_ASSERT_TRUE(dws_opcua_build_get_endpoints_response(&m, &info, 1, 0, buf, sizeof(buf)) > 0);
    dws_opcua_set_endpoint_url(nullptr); // restore the default
}

void test_rx_and_proto_handler_host_stubs()
{
    dws_opcua_rx(0);                             // host build: rx is a no-op
    dws_opcua_rx(255);                           // out-of-range slot is still a safe no-op
    TEST_ASSERT_NULL(dws_opcua_proto_handler()); // host has no instance-bound handler
}

void test_parse_open_with_cert_and_nonce()
{
    // An OPEN carrying non-empty SenderCertificate + ReceiverCertificateThumbprint + ClientNonce
    // exercises the ByteString-skip paths in dws_opcua_parse_open that a null-field OPEN never reaches.
    uint8_t buf[256];
    UaWriter w = {buf, sizeof(buf), 0, true};
    dws_ua_w_u8(&w, 'O');
    dws_ua_w_u8(&w, 'P');
    dws_ua_w_u8(&w, 'N');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0); // size placeholder
    dws_ua_w_u32(&w, 7); // channel
    const char *pol = OPCUA_POLICY_NONE_URI;
    dws_ua_w_string(&w, pol, (int32_t)strlen(pol));
    uint8_t cert[3] = {1, 2, 3};
    dws_ua_w_string(&w, (const char *)cert, 3); // SenderCertificate (non-empty)
    uint8_t thumb[2] = {9, 8};
    dws_ua_w_string(&w, (const char *)thumb, 2); // ReceiverCertificateThumbprint (non-empty)
    dws_ua_w_u32(&w, 5);                         // seq
    dws_ua_w_u32(&w, 55);                        // req_id
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_REQ);
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, 42);              // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, nullptr, -1);  // AuditEntryId
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AdditionalHeader NodeId
    dws_ua_w_u8(&w, 0x00);             // ... no body
    dws_ua_w_u32(&w, 0);               // ClientProtocolVersion
    dws_ua_w_u32(&w, 0);               // RequestType
    dws_ua_w_u32(&w, 1);               // MessageSecurityMode
    uint8_t nonce[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    dws_ua_w_string(&w, (const char *)nonce, 4); // ClientNonce (non-empty)
    dws_ua_w_u32(&w, 3600000);
    buf[4] = (uint8_t)w.n;
    buf[5] = (uint8_t)(w.n >> 8);
    buf[6] = buf[7] = 0;
    OpcUaOpenChannel oc;
    TEST_ASSERT_TRUE(dws_opcua_parse_open(buf, w.n, &oc));
    TEST_ASSERT_EQUAL_UINT32(7, oc.secure_channel_id);
    TEST_ASSERT_EQUAL_UINT32(42, oc.request_handle);
}

void test_parse_read_truncated_item_rejected()
{
    // A NodesToRead count larger than the items actually present makes the per-item NodeId read
    // underrun; the server must reject it (a malformed / hostile client request).
    uint32_t ids[1] = {0x1234};
    uint8_t buf[256];
    size_t n = build_read(buf, sizeof(buf), 88, 1, 100, 42, ids, 1);
    int nid = -1; // item NodeId is FourByte {0x01, ns=0x01, id 0x34 0x12}; the count i32 sits just before it
    for (size_t i = 0; i + 3 < n; i++)
        if (buf[i] == 0x01 && buf[i + 1] == 0x01 && buf[i + 2] == 0x34 && buf[i + 3] == 0x12)
        {
            nid = (int)i;
            break;
        }
    TEST_ASSERT_TRUE(nid >= 4);
    buf[nid - 4] = 0x0A; // NodesToRead count = 10, but only one item follows
    buf[nid - 3] = buf[nid - 2] = buf[nid - 1] = 0;
    OpcUaReadRequest rr;
    TEST_ASSERT_FALSE(dws_opcua_parse_read(buf, n, &rr));
}

void test_parse_browse_truncated_item_rejected()
{
    uint8_t buf[256];
    size_t n = build_browse(buf, sizeof(buf), 7, 6, 300, 70, 0, 0x07ED); // distinctive FourByte id
    int nid = -1; // BrowseDescription NodeId {0x01, ns=0x00, id 0xED 0x07}; the count i32 sits just before it
    for (size_t i = 0; i + 3 < n; i++)
        if (buf[i] == 0x01 && buf[i + 1] == 0x00 && buf[i + 2] == 0xED && buf[i + 3] == 0x07)
        {
            nid = (int)i;
            break;
        }
    TEST_ASSERT_TRUE(nid >= 4);
    buf[nid - 4] = 0x0A; // NodesToBrowse count = 10, but only one item follows
    buf[nid - 3] = buf[nid - 2] = buf[nid - 1] = 0;
    OpcUaBrowseRequest br;
    TEST_ASSERT_FALSE(dws_opcua_parse_browse(buf, n, &br));
}

// Build a WriteRequest with a controllable NodesToWrite count and IndexRange length (one real item).
static size_t build_write(uint8_t *out, size_t cap, int32_t count_field, int32_t ir_len)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 7);
    dws_ua_w_u32(&w, 5);
    dws_ua_w_u32(&w, 700);
    dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_WRITE_REQ);
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u64(&w, 0);
    dws_ua_w_u32(&w, 95);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_string(&w, nullptr, -1);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_nodeid_numeric(&w, 0, 0);
    dws_ua_w_u8(&w, 0x00);
    dws_ua_w_i32(&w, count_field);      // NodesToWrite count (may exceed the items present)
    dws_ua_w_nodeid_numeric(&w, 1, 10); // WriteValue.NodeId
    dws_ua_w_u32(&w, OPCUA_ATTR_VALUE); // AttributeId
    if (ir_len > 0)
    {
        dws_ua_w_i32(&w, ir_len);
        for (int32_t i = 0; i < ir_len; i++)
            dws_ua_w_u8(&w, '0'); // non-empty IndexRange
    }
    else
    {
        dws_ua_w_string(&w, nullptr, -1);
    }
    OpcUaVariant v;
    memset(&v, 0, sizeof(v));
    v.type = OpcUaVariantType::OPCUA_VAR_INT32;
    v.i32 = 0x11223344; // distinctive marker for the malformed-DataValue test
    dws_ua_w_datavalue(&w, &v, OPCUA_STATUS_GOOD);
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_write_truncated_item_and_indexrange()
{
    OpcUaWriteRequest wr;
    // Count claims two items but only one is present -> the second NodeId read underruns -> reject.
    uint8_t buf[160];
    size_t n = build_write(buf, sizeof(buf), 2, 0);
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, n, &wr));
    // A non-empty IndexRange is skipped; the request stays valid.
    n = build_write(buf, sizeof(buf), 1, 3);
    TEST_ASSERT_TRUE(dws_opcua_parse_write(buf, n, &wr));
    TEST_ASSERT_EQUAL_UINT32(1, wr.count);
}

void test_parse_open_wrong_body_typeid()
{
    uint8_t buf[256];
    size_t n = build_open(buf, sizeof(buf), 0, 1, 100, 42, 1, 3600000);
    // Body TypeId is OPEN_REQ (446 -> FourByte bytes 01 00 BE 01); corrupt the id so it no longer matches.
    for (size_t i = 0; i + 3 < n; i++)
        if (buf[i] == 0x01 && buf[i + 1] == 0x00 && buf[i + 2] == 0xBE && buf[i + 3] == 0x01)
        {
            buf[i + 2] = 0xFF;
            break;
        }
    OpcUaOpenChannel oc;
    TEST_ASSERT_FALSE(dws_opcua_parse_open(buf, n, &oc));
}

void test_parse_write_malformed_datavalue_rejected()
{
    uint8_t buf[160];
    size_t n = build_write(buf, sizeof(buf), 1, 0);
    // The item's DataValue is INT32 0x11223344; corrupt its Variant type byte to an unsupported value.
    int vpos = -1;
    for (size_t i = 1; i + 3 < n; i++)
        if (buf[i] == 0x44 && buf[i + 1] == 0x33 && buf[i + 2] == 0x22 && buf[i + 3] == 0x11)
        {
            vpos = (int)i;
            break;
        }
    TEST_ASSERT_TRUE(vpos > 0);
    buf[vpos - 1] = 200; // unsupported Variant type byte
    OpcUaWriteRequest wr;
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, n, &wr));
}

// A request built valid up to TimeoutHint but truncated before the AdditionalHeader ExtensionObject.
static size_t build_req_trunc_at_addhdr(uint8_t *out, size_t cap, uint32_t type_id)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'M');
    dws_ua_w_u8(&w, 'S');
    dws_ua_w_u8(&w, 'G');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 7);
    dws_ua_w_u32(&w, 5);
    dws_ua_w_u32(&w, 700);
    dws_ua_w_nodeid_numeric(&w, 0, type_id);
    dws_ua_w_nodeid_numeric(&w, 0, 0); // AuthenticationToken
    dws_ua_w_u64(&w, 0);               // Timestamp
    dws_ua_w_u32(&w, 0);               // RequestHandle
    dws_ua_w_u32(&w, 0);               // ReturnDiagnostics
    dws_ua_w_string(&w, nullptr, -1);  // AuditEntryId
    dws_ua_w_u32(&w, 0);               // TimeoutHint
    // AdditionalHeader omitted -> r_ext_object_skip's NodeId read underruns
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_request_header_truncated_addhdr()
{
    uint8_t buf[64];
    size_t n = build_req_trunc_at_addhdr(buf, sizeof(buf), OPCUA_ID_WRITE_REQ);
    OpcUaWriteRequest wr;
    TEST_ASSERT_FALSE(dws_opcua_parse_write(buf, n, &wr)); // AdditionalHeader ExtensionObject underruns
}

// Build an OPN frame truncated at a chosen stage: 0 = no body TypeId, 2 = valid TypeId + partial header.
static size_t build_open_frame(uint8_t *out, size_t cap, int stage)
{
    UaWriter w = {out, cap, 0, true};
    dws_ua_w_u8(&w, 'O');
    dws_ua_w_u8(&w, 'P');
    dws_ua_w_u8(&w, 'N');
    dws_ua_w_u8(&w, 'F');
    dws_ua_w_u32(&w, 0);
    dws_ua_w_u32(&w, 0);              // SecureChannelId
    dws_ua_w_string(&w, nullptr, -1); // SecurityPolicyUri
    dws_ua_w_string(&w, nullptr, -1); // SenderCertificate
    dws_ua_w_string(&w, nullptr, -1); // ReceiverCertificateThumbprint
    dws_ua_w_u32(&w, 1);              // SequenceNumber
    dws_ua_w_u32(&w, 100);            // RequestId
    if (stage >= 1)
        dws_ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_REQ); // body TypeId
    if (stage >= 2)
    {
        dws_ua_w_nodeid_numeric(&w, 0, 0); // RequestHeader: AuthenticationToken
        dws_ua_w_u64(&w, 0);               // Timestamp, then truncated
    }
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = out[7] = 0;
    return w.n;
}

void test_parse_open_truncated_frames()
{
    OpcUaOpenChannel oc;
    uint8_t buf[64];
    size_t n = build_open_frame(buf, sizeof(buf), 0); // no body TypeId -> TypeId NodeId read underruns
    TEST_ASSERT_FALSE(dws_opcua_parse_open(buf, n, &oc));
    n = build_open_frame(buf, sizeof(buf), 2); // valid TypeId, truncated RequestHeader -> r_request_header fails
    TEST_ASSERT_FALSE(dws_opcua_parse_open(buf, n, &oc));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_read_optional_fields);
    RUN_TEST(test_parse_rejections);
    RUN_TEST(test_build_guards_and_overflow);
    RUN_TEST(test_setters_and_endpoint_url);
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
    RUN_TEST(test_rx_and_proto_handler_host_stubs);
    RUN_TEST(test_parse_open_with_cert_and_nonce);
    RUN_TEST(test_parse_read_truncated_item_rejected);
    RUN_TEST(test_parse_browse_truncated_item_rejected);
    RUN_TEST(test_parse_write_truncated_item_and_indexrange);
    RUN_TEST(test_parse_open_wrong_body_typeid);
    RUN_TEST(test_parse_write_malformed_datavalue_rejected);
    RUN_TEST(test_parse_request_header_truncated_addhdr);
    RUN_TEST(test_parse_open_truncated_frames);
    return UNITY_END();
}
