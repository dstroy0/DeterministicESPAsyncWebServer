// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for OPC UA (services/opcua): the OPC UA Binary built-in type codec
// (incl. NodeId / DateTime), UACP header parsing, the Hello/Acknowledge handshake,
// and the SecureChannel (OpenSecureChannel/OPN, SecurityPolicy None). The TCP data
// handler (opcua_rx) is ESP32-only and HW-verified.

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

    uint8_t resp[256];
    int64_t now = opcua_filetime_from_unix(1700000000LL);
    size_t rn = opcua_build_create_session_response(&m, 0x1001, 0x2002, 1200000.0, 5, now, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    UaMsgHeader h;
    TEST_ASSERT_TRUE(opcua_parse_header(resp, rn, &h));
    TEST_ASSERT_EQUAL_MEMORY("MSG", h.type, 3);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)rn, h.size);

    UaReader r = {resp + 8, rn - 8, 0, false};
    char str[16];
    int32_t sl = 0;
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
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r));                 // ServerEndpoints[] empty
    TEST_ASSERT_EQUAL_INT32(0, ua_r_i32(&r));                 // ServerSoftwareCertificates[] empty
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerSignature.Algorithm null
    TEST_ASSERT_TRUE(ua_r_string(&r, str, sizeof(str), &sl)); // ServerSignature.Signature null
    TEST_ASSERT_EQUAL_UINT32(0, ua_r_u32(&r));                // MaxRequestMessageSize
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

int main()
{
    UNITY_BEGIN();
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
    return UNITY_END();
}
