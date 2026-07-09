// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HAProxy PROXY protocol codec (services/proxy_protocol): the v1 (text)
// and v2 (binary) builders + the unified parser. Per the HAProxy spec. Pure host tests.

#include "services/proxy_protocol/proxy_protocol.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// 203.0.113.50 / 203.0.113.10 in host-order uint32.
static const uint32_t SRC = 0xCB007132u;
static const uint32_t DST = 0xCB00710Au;

void test_v1_build()
{
    char buf[64];
    size_t n = proxy_v1_build(buf, sizeof(buf), SRC, DST, 12345, 80);
    TEST_ASSERT_EQUAL_STRING("PROXY TCP4 203.0.113.50 203.0.113.10 12345 80\r\n", buf);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
}

void test_v1_round_trip()
{
    uint8_t buf[64];
    size_t n = proxy_v1_build((char *)buf, sizeof(buf), SRC, DST, 12345, 80);
    ProxyInfo info;
    size_t consumed;
    TEST_ASSERT_TRUE(proxy_parse(buf, n, &info, &consumed));
    TEST_ASSERT_EQUAL_UINT8(1, info.version);
    TEST_ASSERT_TRUE(info.has_addr);
    TEST_ASSERT_EQUAL_HEX32(SRC, info.src_addr);
    TEST_ASSERT_EQUAL_HEX32(DST, info.dst_addr);
    TEST_ASSERT_EQUAL_UINT16(12345, info.src_port);
    TEST_ASSERT_EQUAL_UINT16(80, info.dst_port);
    TEST_ASSERT_EQUAL_size_t(n, consumed);
}

void test_v2_build_bytes()
{
    uint8_t buf[32];
    size_t n = proxy_v2_build(buf, sizeof(buf), SRC, DST, 12345, 80);
    const uint8_t expect[] = {
        0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A, // signature
        0x21, 0x11, 0x00, 0x0C,                                                 // ver_cmd, fam, len 12
        0xCB, 0x00, 0x71, 0x32,                                                 // src 203.0.113.50
        0xCB, 0x00, 0x71, 0x0A,                                                 // dst 203.0.113.10
        0x30, 0x39,                                                             // src port 12345
        0x00, 0x50                                                              // dst port 80
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_size_t(28, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_v2_round_trip()
{
    uint8_t buf[32];
    size_t n = proxy_v2_build(buf, sizeof(buf), SRC, DST, 443, 8080);
    ProxyInfo info;
    size_t consumed;
    TEST_ASSERT_TRUE(proxy_parse(buf, n, &info, &consumed));
    TEST_ASSERT_EQUAL_UINT8(2, info.version);
    TEST_ASSERT_TRUE(info.has_addr);
    TEST_ASSERT_EQUAL_HEX32(SRC, info.src_addr);
    TEST_ASSERT_EQUAL_UINT16(443, info.src_port);
    TEST_ASSERT_EQUAL_UINT16(8080, info.dst_port);
    TEST_ASSERT_EQUAL_size_t(n, consumed);
}

void test_v1_unknown()
{
    const char *raw = "PROXY UNKNOWN\r\n";
    ProxyInfo info;
    size_t consumed;
    TEST_ASSERT_TRUE(proxy_parse((const uint8_t *)raw, strlen(raw), &info, &consumed));
    TEST_ASSERT_EQUAL_UINT8(1, info.version);
    TEST_ASSERT_FALSE(info.has_addr);
    TEST_ASSERT_EQUAL_size_t(strlen(raw), consumed);
}

void test_not_a_proxy_header()
{
    const char *http = "GET / HTTP/1.1\r\n";
    ProxyInfo info;
    size_t consumed;
    TEST_ASSERT_FALSE(proxy_parse((const uint8_t *)http, strlen(http), &info, &consumed));
}

void test_incomplete()
{
    ProxyInfo info;
    size_t consumed;
    // v1 prefix but no CRLF yet.
    const char *v1 = "PROXY TCP4 203.0.113.50 203.0.113.10 12345 80";
    TEST_ASSERT_FALSE(proxy_parse((const uint8_t *)v1, strlen(v1), &info, &consumed));
    // v2 signature only (no address block).
    const uint8_t v2sig[] = {0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A, 0x21, 0x11};
    TEST_ASSERT_FALSE(proxy_parse(v2sig, sizeof(v2sig), &info, &consumed));
}

void test_build_overflow_fails_closed()
{
    char small[16];
    TEST_ASSERT_EQUAL_size_t(0, proxy_v1_build(small, sizeof(small), SRC, DST, 12345, 80));
    uint8_t v2small[20];
    TEST_ASSERT_EQUAL_size_t(0, proxy_v2_build(v2small, sizeof(v2small), SRC, DST, 12345, 80)); // needs 28
}

void test_v1_malformed_addresses_fail_closed()
{
    ProxyInfo info;
    size_t consumed = 0;
    // Each malformed v1 source address drives a distinct parse_ipv4 reject path.
    TEST_ASSERT_FALSE(
        proxy_parse((const uint8_t *)"PROXY TCP4 25x.0.0.1 10.0.0.1 1 2 ", 34, &info, &consumed)); // non-digit
    TEST_ASSERT_FALSE(
        proxy_parse((const uint8_t *)"PROXY TCP4 999.0.0.1 10.0.0.1 1 2 ", 34, &info, &consumed)); // octet > 255
    TEST_ASSERT_FALSE(
        proxy_parse((const uint8_t *)"PROXY TCP4 10-0-0-1 10.0.0.1 1 2 ", 33, &info, &consumed)); // missing dot
    TEST_ASSERT_FALSE(
        proxy_parse((const uint8_t *)"PROXY TCP4 10.0.0 10.0.0.1 1 2 ", 31, &info, &consumed)); // three octets
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_v1_build);
    RUN_TEST(test_v1_round_trip);
    RUN_TEST(test_v2_build_bytes);
    RUN_TEST(test_v2_round_trip);
    RUN_TEST(test_v1_unknown);
    RUN_TEST(test_not_a_proxy_header);
    RUN_TEST(test_incomplete);
    RUN_TEST(test_build_overflow_fails_closed);
    RUN_TEST(test_v1_malformed_addresses_fail_closed);
    return UNITY_END();
}
