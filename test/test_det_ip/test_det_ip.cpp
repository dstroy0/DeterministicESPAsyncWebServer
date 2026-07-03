// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DetIp address core (network_drivers/network/det_ip): RFC 4291 text
// parsing, RFC 5952 canonical formatting, scope classification, v4-mapped handling, and
// malformed-input rejection. Pure host tests.

#include "network_drivers/network/det_ip.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Parse s, format it back, and assert the canonical text equals expect.
static void canon(const char *s, const char *expect)
{
    DetIp ip;
    TEST_ASSERT_TRUE_MESSAGE(det_ip_parse(s, &ip), s);
    char out[DET_IP_STR_MAX];
    size_t n = det_ip_format(&ip, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT((unsigned)strlen(expect), (unsigned)n);
    TEST_ASSERT_EQUAL_STRING(expect, out);
}

void test_v4_round_trip()
{
    canon("192.168.1.1", "192.168.1.1");
    canon("0.0.0.0", "0.0.0.0");
    canon("255.255.255.255", "255.255.255.255");
    canon("8.8.8.8", "8.8.8.8");
}

void test_v6_canonical_5952()
{
    // RFC 5952: lower-case, no leading zeros, longest zero run -> "::".
    canon("2001:0db8:0000:0000:0000:0000:0000:0001", "2001:db8::1");
    canon("2001:DB8::1", "2001:db8::1");
    canon("0:0:0:0:0:0:0:0", "::");
    canon("0:0:0:0:0:0:0:1", "::1");
    canon("fe80:0:0:0:0:0:0:1", "fe80::1");
    canon("2001:0db8:0000:0000:0000:ff00:0042:8329", "2001:db8::ff00:42:8329");
    // Two equal-length zero runs -> compress the leftmost (RFC 5952 4.2.3).
    canon("2001:db8:0:0:1:0:0:1", "2001:db8::1:0:0:1");
    // A single zero group is NOT compressed.
    canon("2001:db8:0:1:1:1:1:1", "2001:db8:0:1:1:1:1:1");
    // Full 8 groups, no zeros.
    canon("2001:db8:1:2:3:4:5:6", "2001:db8:1:2:3:4:5:6");
}

void test_v4_mapped()
{
    canon("::ffff:192.168.1.1", "::ffff:192.168.1.1");
    canon("::ffff:0:0", "::ffff:0.0.0.0");
    DetIp ip;
    TEST_ASSERT_TRUE(det_ip_parse("::ffff:10.0.0.1", &ip));
    TEST_ASSERT_TRUE(det_ip_is_v4_mapped(&ip));
    TEST_ASSERT_EQUAL_HEX32(0x0A000001u, det_ip_to_v4_be(&ip));
}

void test_classify_v4()
{
    DetIp ip;
    det_ip_parse("0.0.0.0", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_UNSPECIFIED, det_ip_classify(&ip));
    det_ip_parse("127.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_LOOPBACK, det_ip_classify(&ip));
    det_ip_parse("169.254.3.4", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_LINK_LOCAL, det_ip_classify(&ip));
    det_ip_parse("10.1.2.3", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_PRIVATE, det_ip_classify(&ip));
    det_ip_parse("172.16.0.1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_PRIVATE, det_ip_classify(&ip));
    det_ip_parse("172.32.0.1", &ip); // just outside 172.16/12
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_GLOBAL, det_ip_classify(&ip));
    det_ip_parse("192.168.5.5", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_PRIVATE, det_ip_classify(&ip));
    det_ip_parse("224.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_MULTICAST, det_ip_classify(&ip));
    det_ip_parse("8.8.8.8", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_GLOBAL, det_ip_classify(&ip));
}

void test_classify_v6()
{
    DetIp ip;
    det_ip_parse("::", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_UNSPECIFIED, det_ip_classify(&ip));
    det_ip_parse("::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_LOOPBACK, det_ip_classify(&ip));
    det_ip_parse("fe80::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_LINK_LOCAL, det_ip_classify(&ip));
    det_ip_parse("fc00::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_PRIVATE, det_ip_classify(&ip)); // ULA
    det_ip_parse("fd12:3456::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_PRIVATE, det_ip_classify(&ip));
    det_ip_parse("ff02::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_MULTICAST, det_ip_classify(&ip));
    det_ip_parse("2001:db8::1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_GLOBAL, det_ip_classify(&ip));
    // v4-mapped classifies as its v4 scope.
    det_ip_parse("::ffff:127.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DET_IP_SCOPE_LOOPBACK, det_ip_classify(&ip));
}

void test_reject_malformed()
{
    DetIp ip;
    TEST_ASSERT_FALSE(det_ip_parse("", &ip));
    TEST_ASSERT_FALSE(det_ip_parse("256.0.0.1", &ip));         // octet > 255
    TEST_ASSERT_FALSE(det_ip_parse("1.2.3", &ip));             // too few octets
    TEST_ASSERT_FALSE(det_ip_parse("1.2.3.4.5", &ip));         // too many
    TEST_ASSERT_FALSE(det_ip_parse("1::2::3", &ip));           // two "::"
    TEST_ASSERT_FALSE(det_ip_parse("1:2:3:4:5:6:7:8:9", &ip)); // 9 groups
    TEST_ASSERT_FALSE(det_ip_parse("1:2:3:4:5:6:7", &ip));     // 7 groups, no "::"
    TEST_ASSERT_FALSE(det_ip_parse("12345::1", &ip));          // hextet too long
    TEST_ASSERT_FALSE(det_ip_parse("2001:db8:::1", &ip));      // triple colon
    TEST_ASSERT_FALSE(det_ip_parse("2001:db8::1:", &ip));      // trailing single colon
    TEST_ASSERT_FALSE(det_ip_parse("2001:xyz::1", &ip));       // bad hex
    TEST_ASSERT_FALSE(det_ip_parse("::ffff:1.2.3.4:5", &ip));  // v4 not last
    TEST_ASSERT_FALSE(det_ip_parse("hello", &ip));             // no separators
}

void test_equal_and_from_v4()
{
    DetIp a = det_ip_from_v4_octets(192, 168, 1, 1);
    DetIp b;
    TEST_ASSERT_TRUE(det_ip_parse("192.168.1.1", &b));
    TEST_ASSERT_TRUE(det_ip_equal(&a, &b));
    TEST_ASSERT_EQUAL_HEX32(0xC0A80101u, det_ip_to_v4_be(&a));

    DetIp c;
    TEST_ASSERT_TRUE(det_ip_parse("192.168.1.2", &c));
    TEST_ASSERT_FALSE(det_ip_equal(&a, &c));

    // Different families never compare equal.
    DetIp v6;
    TEST_ASSERT_TRUE(det_ip_parse("::c0a8:101", &v6));
    TEST_ASSERT_FALSE(det_ip_equal(&a, &v6));
}

void test_from_v6_bytes()
{
    // 2001:db8::1 as raw network-order bytes -> DetIp -> canonical text.
    const uint8_t raw[16] = {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    DetIp ip = det_ip_from_v6_bytes(raw);
    TEST_ASSERT_EQUAL_UINT8(DET_IP_V6, ip.family);
    char s[DET_IP_STR_MAX];
    det_ip_format(&ip, s, sizeof(s));
    TEST_ASSERT_EQUAL_STRING("2001:db8::1", s);

    DetIp parsed;
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8::1", &parsed));
    TEST_ASSERT_TRUE(det_ip_equal(&ip, &parsed));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_v4_round_trip);
    RUN_TEST(test_from_v6_bytes);
    RUN_TEST(test_v6_canonical_5952);
    RUN_TEST(test_v4_mapped);
    RUN_TEST(test_classify_v4);
    RUN_TEST(test_classify_v6);
    RUN_TEST(test_reject_malformed);
    RUN_TEST(test_equal_and_from_v4);
    return UNITY_END();
}
