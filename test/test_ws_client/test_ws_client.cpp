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
    size_t n = ws_client_build_frame(buf, sizeof(buf), WSC_OP_TEXT, (const uint8_t *)"Hi", 2, mask);
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
    size_t n = ws_client_build_frame(buf, sizeof(buf), WSC_OP_BINARY, payload, sizeof(payload), mask);
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
    TEST_ASSERT_EQUAL_UINT8(WSC_OP_TEXT, op);
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

int main(int, char **)
{
    UNITY_BEGIN();
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
