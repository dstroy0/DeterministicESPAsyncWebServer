// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host unit tests for the outbound WebSocket client codec (env:native_ws_client).

#include "services/ws_client/ws_client.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// RFC 6455 4.2.2 worked example: key "dGhlIHNhbXBsZSBub25jZQ==" yields
// accept "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=".
void test_accept_rfc_example()
{
    char acc[32];
    ws_client_accept_for_key("dGhlIHNhbXBsZSBub25jZQ==", acc, sizeof(acc));
    TEST_ASSERT_EQUAL_STRING("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", acc);
}

void test_build_handshake()
{
    uint8_t buf[256];
    size_t n = ws_client_build_handshake(buf, sizeof(buf), "example.com", "/chat", "dGhlIHNhbXBsZSBub25jZQ==");
    TEST_ASSERT_GREATER_THAN(0, n);
    buf[n] = '\0';
    TEST_ASSERT_NOT_NULL(strstr((char *)buf, "GET /chat HTTP/1.1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr((char *)buf, "Host: example.com\r\n"));
    TEST_ASSERT_NOT_NULL(strstr((char *)buf, "Upgrade: websocket\r\n"));
    TEST_ASSERT_NOT_NULL(strstr((char *)buf, "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"));
    TEST_ASSERT_NOT_NULL(strstr((char *)buf, "Sec-WebSocket-Version: 13\r\n\r\n"));
}

void test_check_response_ok()
{
    const char *resp = "HTTP/1.1 101 Switching Protocols\r\n"
                       "Upgrade: websocket\r\n"
                       "Connection: Upgrade\r\n"
                       "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
    TEST_ASSERT_TRUE(ws_client_check_response((const uint8_t *)resp, strlen(resp), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));
}

void test_check_response_bad_accept()
{
    const char *resp = "HTTP/1.1 101 Switching Protocols\r\n"
                       "Sec-WebSocket-Accept: wrongwrongwrongwrongwrongwro=\r\n\r\n";
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)resp, strlen(resp), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="));
}

void test_check_response_not_101()
{
    const char *resp = "HTTP/1.1 400 Bad Request\r\n\r\n";
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)resp, strlen(resp), "x"));
}

void test_build_frame_masked()
{
    uint8_t buf[32];
    uint8_t mask[4] = {0x01, 0x02, 0x03, 0x04};
    size_t n = ws_client_build_frame(buf, sizeof(buf), WsClientOpcode::WSC_OP_TEXT, (const uint8_t *)"Hi", 2, mask);
    TEST_ASSERT_EQUAL_size_t(8, n);             // 2 hdr + 4 mask + 2 payload
    TEST_ASSERT_EQUAL_HEX8(0x81, buf[0]);       // FIN + text
    TEST_ASSERT_EQUAL_HEX8(0x82, buf[1]);       // mask bit + len 2
    TEST_ASSERT_EQUAL_MEMORY(mask, buf + 2, 4); // mask key
    TEST_ASSERT_EQUAL_HEX8('H' ^ 0x01, buf[6]);
    TEST_ASSERT_EQUAL_HEX8('i' ^ 0x02, buf[7]);
}

void test_build_frame_extended_len()
{
    uint8_t buf[300];
    uint8_t payload[200];
    memset(payload, 'a', sizeof(payload));
    uint8_t mask[4] = {0, 0, 0, 0}; // zero mask -> payload copied verbatim
    size_t n = ws_client_build_frame(buf, sizeof(buf), WsClientOpcode::WSC_OP_BINARY, payload, sizeof(payload), mask);
    TEST_ASSERT_EQUAL_size_t(2 + 2 + 4 + 200, n);
    TEST_ASSERT_EQUAL_HEX8(0x82, buf[0]);       // FIN + binary
    TEST_ASSERT_EQUAL_HEX8(0x80 | 126, buf[1]); // mask + 16-bit length follows
    TEST_ASSERT_EQUAL_UINT16(200, (buf[2] << 8) | buf[3]);
}

void test_parse_frame_server_text()
{
    // Server (unmasked) text frame "hello".
    uint8_t f[] = {0x81, 0x05, 'h', 'e', 'l', 'l', 'o'};
    uint8_t op;
    bool fin;
    size_t off, plen, consumed;
    TEST_ASSERT_TRUE(ws_client_parse_frame(f, sizeof(f), &op, &fin, &off, &plen, &consumed));
    TEST_ASSERT_EQUAL_UINT8(WsClientOpcode::WSC_OP_TEXT, op);
    TEST_ASSERT_TRUE(fin);
    TEST_ASSERT_EQUAL_size_t(2, off);
    TEST_ASSERT_EQUAL_size_t(5, plen);
    TEST_ASSERT_EQUAL_size_t(7, consumed);
    TEST_ASSERT_EQUAL_MEMORY("hello", f + off, 5);
}

void test_parse_frame_incomplete()
{
    uint8_t f[] = {0x81, 0x05, 'h', 'e'}; // says 5 bytes, only 2 present
    uint8_t op;
    bool fin;
    size_t off, plen, consumed;
    TEST_ASSERT_FALSE(ws_client_parse_frame(f, sizeof(f), &op, &fin, &off, &plen, &consumed));
}

void test_parse_frame_extended_len()
{
    uint8_t f[4 + 300];
    f[0] = 0x82;
    f[1] = 126;
    f[2] = 0x01;
    f[3] = 0x2C; // 300
    memset(f + 4, 'z', 300);
    uint8_t op;
    bool fin;
    size_t off, plen, consumed;
    TEST_ASSERT_TRUE(ws_client_parse_frame(f, sizeof(f), &op, &fin, &off, &plen, &consumed));
    TEST_ASSERT_EQUAL_size_t(4, off);
    TEST_ASSERT_EQUAL_size_t(300, plen);
    TEST_ASSERT_EQUAL_size_t(304, consumed);
}

// accept_for_key fails closed on null/zero-cap output, a null key, an over-long key,
// and an output buffer too small for the 28-char accept value.
void test_accept_for_key_guards()
{
    char out[64];
    ws_client_accept_for_key("key", out, 0);      // out_cap == 0
    ws_client_accept_for_key("key", nullptr, 10); // out == nullptr

    out[0] = 'x';
    ws_client_accept_for_key(nullptr, out, sizeof(out)); // key == nullptr
    TEST_ASSERT_EQUAL_STRING("", out);

    char long_key[80];
    memset(long_key, 'a', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';
    out[0] = 'x';
    ws_client_accept_for_key(long_key, out, sizeof(out)); // key + magic overflows concat
    TEST_ASSERT_EQUAL_STRING("", out);

    char small[10];
    small[0] = 'x';
    ws_client_accept_for_key("dGhlIHNhbXBsZSBub25jZQ==", small, sizeof(small)); // out_cap < 29
    TEST_ASSERT_EQUAL_STRING("", small);
}

// build_handshake rejects null args and a buffer too small for the request.
void test_build_handshake_guards()
{
    uint8_t out[256];
    TEST_ASSERT_EQUAL_UINT(0, ws_client_build_handshake(nullptr, sizeof(out), "h", "/", "k"));
    TEST_ASSERT_EQUAL_UINT(0, ws_client_build_handshake(out, sizeof(out), nullptr, "/", "k"));
    TEST_ASSERT_EQUAL_UINT(0, ws_client_build_handshake(out, sizeof(out), "h", nullptr, "k"));
    TEST_ASSERT_EQUAL_UINT(0, ws_client_build_handshake(out, sizeof(out), "h", "/", nullptr));
    TEST_ASSERT_EQUAL_UINT(0, ws_client_build_handshake(out, 10, "host", "/path", "key")); // overflow
}

// check_response rejects null/short args, a buffer with no line ending, a non-101
// status, and a 101 response without the Accept header.
void test_check_response_guards()
{
    TEST_ASSERT_FALSE(ws_client_check_response(nullptr, 100, "acc"));
    uint8_t s[4] = {'a', 'b', 'c', 'd'};
    TEST_ASSERT_FALSE(ws_client_check_response(s, 4, "acc")); // len < 12
    const char *ok = "HTTP/1.1 101 x";
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)ok, strlen(ok), nullptr)); // null accept

    const char *no_eol = "HTTP/1.1 101 Switching Pr"; // >= 12, no '\n'
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)no_eol, strlen(no_eol), "acc"));

    const char *r200 = "HTTP/1.1 200 OK\r\n\r\n";
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)r200, strlen(r200), "acc")); // not 101

    const char *no_acc = "HTTP/1.1 101 OK\r\nUpgrade: websocket\r\n\r\n";
    TEST_ASSERT_FALSE(ws_client_check_response((const uint8_t *)no_acc, strlen(no_acc), "acc")); // no Accept header
}

// build_frame rejects null out/mask and a too-small buffer, and encodes a 64-bit
// length for a >= 64 KiB payload.
static uint8_t s_big_pl[65536];
static uint8_t s_big_out[65600];
void test_build_frame_guards_and_64bit()
{
    uint8_t mask[4] = {1, 2, 3, 4};
    uint8_t out[16];
    TEST_ASSERT_EQUAL_UINT(
        0, ws_client_build_frame(nullptr, sizeof(out), WsClientOpcode::WSC_OP_TEXT, (const uint8_t *)"x", 1, mask));
    TEST_ASSERT_EQUAL_UINT(
        0, ws_client_build_frame(out, sizeof(out), WsClientOpcode::WSC_OP_TEXT, (const uint8_t *)"x", 1, nullptr));
    TEST_ASSERT_EQUAL_UINT(
        0, ws_client_build_frame(out, 4, WsClientOpcode::WSC_OP_TEXT, (const uint8_t *)"hello", 5, mask)); // too small

    memset(s_big_pl, 'z', sizeof(s_big_pl));
    size_t n = ws_client_build_frame(s_big_out, sizeof(s_big_out), WsClientOpcode::WSC_OP_BINARY, s_big_pl,
                                     sizeof(s_big_pl), mask);
    TEST_ASSERT_TRUE(n > sizeof(s_big_pl));
    TEST_ASSERT_EQUAL_HEX8(0x80 | 127, s_big_out[1]); // 64-bit length marker
}

// parse_frame rejects truncated headers, an absurd 64-bit length, and stays aligned
// past a (nonstandard) server-side mask.
void test_parse_frame_edges()
{
    uint8_t op;
    bool fin;
    size_t po, pl, cons;
    TEST_ASSERT_FALSE(ws_client_parse_frame(nullptr, 2, &op, &fin, &po, &pl, &cons));
    uint8_t one[1] = {0x81};
    TEST_ASSERT_FALSE(ws_client_parse_frame(one, 1, &op, &fin, &po, &pl, &cons)); // avail < 2

    uint8_t f126[2] = {0x82, 126}; // 16-bit length announced, none present
    TEST_ASSERT_FALSE(ws_client_parse_frame(f126, 2, &op, &fin, &po, &pl, &cons));
    uint8_t f127[2] = {0x82, 127}; // 64-bit length announced, none present
    TEST_ASSERT_FALSE(ws_client_parse_frame(f127, 2, &op, &fin, &po, &pl, &cons));

    uint8_t huge[10] = {0x82, 127, 0x00, 0x00, 0x00, 0x01, 0, 0, 0, 0}; // 0x1_0000_0000 > 4 GiB
    TEST_ASSERT_FALSE(ws_client_parse_frame(huge, 10, &op, &fin, &po, &pl, &cons));

    uint8_t ok64[10] = {0x82, 127, 0, 0, 0, 0, 0, 0, 0, 100}; // valid 64-bit length (100), payload absent
    TEST_ASSERT_FALSE(ws_client_parse_frame(ok64, 10, &op, &fin, &po, &pl, &cons)); // length read, then payload short

    uint8_t masked[8] = {0x81, 0x82, 0, 0, 0, 0, 'a', 'b'}; // masked len-2 frame
    TEST_ASSERT_TRUE(ws_client_parse_frame(masked, 8, &op, &fin, &po, &pl, &cons));
    TEST_ASSERT_EQUAL_size_t(6, po); // 2-byte header + 4-byte mask
    TEST_ASSERT_EQUAL_size_t(2, pl);
}

// On a host build the transport entry points are inert stubs.
void test_host_transport_stubs()
{
    ws_client_on_message(nullptr);
    TEST_ASSERT_FALSE(ws_client_connect("h", 80, false, "/"));
    TEST_ASSERT_FALSE(ws_client_send_text("hi"));
    TEST_ASSERT_FALSE(ws_client_send_binary((const uint8_t *)"x", 1));
    TEST_ASSERT_FALSE(ws_client_loop());
    TEST_ASSERT_FALSE(ws_client_connected());
    ws_client_close();
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_accept_for_key_guards);
    RUN_TEST(test_build_handshake_guards);
    RUN_TEST(test_check_response_guards);
    RUN_TEST(test_build_frame_guards_and_64bit);
    RUN_TEST(test_parse_frame_edges);
    RUN_TEST(test_host_transport_stubs);
    RUN_TEST(test_accept_rfc_example);
    RUN_TEST(test_build_handshake);
    RUN_TEST(test_check_response_ok);
    RUN_TEST(test_check_response_bad_accept);
    RUN_TEST(test_check_response_not_101);
    RUN_TEST(test_build_frame_masked);
    RUN_TEST(test_build_frame_extended_len);
    RUN_TEST(test_parse_frame_server_text);
    RUN_TEST(test_parse_frame_incomplete);
    RUN_TEST(test_parse_frame_extended_len);
    return UNITY_END();
}
