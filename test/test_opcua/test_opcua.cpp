// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for OPC UA increment 1 (services/opcua): the OPC UA Binary built-in
// type codec, UACP header parsing, and the Hello/Acknowledge handshake. The TCP
// data handler (opcua_rx) is ESP32-only and HW-verified.

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
    return UNITY_END();
}
