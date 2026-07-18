// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DWSIp address core (network_drivers/network/dws_ip): RFC 4291 text
// parsing, RFC 5952 canonical formatting, scope classification, v4-mapped handling, and
// malformed-input rejection. Pure host tests.

#include "network_drivers/network/ip.h"
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
    DWSIp ip;
    TEST_ASSERT_TRUE_MESSAGE(dws_ip_parse(s, &ip), s);
    char out[DWS_IP_STR_MAX];
    size_t n = dws_ip_format(&ip, out, sizeof(out));
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
    DWSIp ip;
    TEST_ASSERT_TRUE(dws_ip_parse("::ffff:10.0.0.1", &ip));
    TEST_ASSERT_TRUE(dws_ip_is_v4_mapped(&ip));
    TEST_ASSERT_EQUAL_HEX32(0x0A000001u, dws_ip_to_v4_be(&ip));
}

void test_classify_v4()
{
    DWSIp ip;
    dws_ip_parse("0.0.0.0", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_UNSPECIFIED, dws_ip_classify(&ip));
    dws_ip_parse("127.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_LOOPBACK, dws_ip_classify(&ip));
    dws_ip_parse("169.254.3.4", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_LINK_LOCAL, dws_ip_classify(&ip));
    dws_ip_parse("10.1.2.3", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_PRIVATE, dws_ip_classify(&ip));
    dws_ip_parse("172.16.0.1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_PRIVATE, dws_ip_classify(&ip));
    dws_ip_parse("172.32.0.1", &ip); // just outside 172.16/12
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_GLOBAL, dws_ip_classify(&ip));
    dws_ip_parse("192.168.5.5", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_PRIVATE, dws_ip_classify(&ip));
    dws_ip_parse("224.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_MULTICAST, dws_ip_classify(&ip));
    dws_ip_parse("8.8.8.8", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_GLOBAL, dws_ip_classify(&ip));
}

void test_classify_v6()
{
    DWSIp ip;
    dws_ip_parse("::", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_UNSPECIFIED, dws_ip_classify(&ip));
    dws_ip_parse("::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_LOOPBACK, dws_ip_classify(&ip));
    dws_ip_parse("fe80::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_LINK_LOCAL, dws_ip_classify(&ip));
    dws_ip_parse("fc00::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_PRIVATE, dws_ip_classify(&ip)); // ULA
    dws_ip_parse("fd12:3456::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_PRIVATE, dws_ip_classify(&ip));
    dws_ip_parse("ff02::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_MULTICAST, dws_ip_classify(&ip));
    dws_ip_parse("2001:db8::1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_GLOBAL, dws_ip_classify(&ip));
    // v4-mapped classifies as its v4 scope.
    dws_ip_parse("::ffff:127.0.0.1", &ip);
    TEST_ASSERT_EQUAL(DWSIpScope::DWS_IP_SCOPE_LOOPBACK, dws_ip_classify(&ip));
}

void test_reject_malformed()
{
    DWSIp ip;
    TEST_ASSERT_FALSE(dws_ip_parse("", &ip));
    TEST_ASSERT_FALSE(dws_ip_parse("256.0.0.1", &ip));         // octet > 255
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3", &ip));             // too few octets
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3.4.5", &ip));         // too many
    TEST_ASSERT_FALSE(dws_ip_parse("1::2::3", &ip));           // two "::"
    TEST_ASSERT_FALSE(dws_ip_parse("1:2:3:4:5:6:7:8:9", &ip)); // 9 groups
    TEST_ASSERT_FALSE(dws_ip_parse("1:2:3:4:5:6:7", &ip));     // 7 groups, no "::"
    TEST_ASSERT_FALSE(dws_ip_parse("12345::1", &ip));          // hextet too long
    TEST_ASSERT_FALSE(dws_ip_parse("2001:db8:::1", &ip));      // triple colon
    TEST_ASSERT_FALSE(dws_ip_parse("2001:db8::1:", &ip));      // trailing single colon
    TEST_ASSERT_FALSE(dws_ip_parse("2001:xyz::1", &ip));       // bad hex
    TEST_ASSERT_FALSE(dws_ip_parse("::ffff:1.2.3.4:5", &ip));  // v4 not last
    TEST_ASSERT_FALSE(dws_ip_parse("hello", &ip));             // no separators
}

void test_equal_and_from_v4()
{
    DWSIp a = dws_ip_from_v4_octets(192, 168, 1, 1);
    DWSIp b;
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.1.1", &b));
    TEST_ASSERT_TRUE(dws_ip_equal(&a, &b));
    TEST_ASSERT_EQUAL_HEX32(0xC0A80101u, dws_ip_to_v4_be(&a));

    DWSIp c;
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.1.2", &c));
    TEST_ASSERT_FALSE(dws_ip_equal(&a, &c));

    // Different families never compare equal.
    DWSIp v6;
    TEST_ASSERT_TRUE(dws_ip_parse("::c0a8:101", &v6));
    TEST_ASSERT_FALSE(dws_ip_equal(&a, &v6));
}

void test_from_v6_bytes()
{
    // 2001:db8::1 as raw network-order bytes -> DWSIp -> canonical text.
    const uint8_t raw[16] = {0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    DWSIp ip = dws_ip_from_v6_bytes(raw);
    TEST_ASSERT_EQUAL_UINT8(DWSIpFamily::DWS_IP_V6, ip.family);
    char s[DWS_IP_STR_MAX];
    dws_ip_format(&ip, s, sizeof(s));
    TEST_ASSERT_EQUAL_STRING("2001:db8::1", s);

    DWSIp parsed;
    TEST_ASSERT_TRUE(dws_ip_parse("2001:db8::1", &parsed));
    TEST_ASSERT_TRUE(dws_ip_equal(&ip, &parsed));
}

void test_is_unspecified()
{
    DWSIp none;
    none.family = DWSIpFamily::DWS_IP_NONE;
    TEST_ASSERT_TRUE(dws_ip_is_unspecified(&none));
    TEST_ASSERT_TRUE(dws_ip_is_unspecified(nullptr));

    DWSIp z4, z6, host;
    TEST_ASSERT_TRUE(dws_ip_parse("0.0.0.0", &z4));
    TEST_ASSERT_TRUE(dws_ip_parse("::", &z6));
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.1.1", &host));
    TEST_ASSERT_TRUE(dws_ip_is_unspecified(&z4)); // 0.0.0.0 is untrackable
    TEST_ASSERT_TRUE(dws_ip_is_unspecified(&z6)); // :: is untrackable
    TEST_ASSERT_FALSE(dws_ip_is_unspecified(&host));
}

void test_prefix_match()
{
    // IPv4 CIDR containment (the allowlist primitive - full address, no hashing).
    DWSIp net24, in24, out24;
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.1.0", &net24));
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.1.200", &in24));
    TEST_ASSERT_TRUE(dws_ip_parse("192.168.2.1", &out24));
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&in24, &net24, 24));
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&out24, &net24, 24));

    // Host bits outside the prefix are masked at compare time (10.1.2.3/8 == 10.0.0.0/8).
    DWSIp net8, in8, out8;
    TEST_ASSERT_TRUE(dws_ip_parse("10.1.2.3", &net8));
    TEST_ASSERT_TRUE(dws_ip_parse("10.255.255.255", &in8));
    TEST_ASSERT_TRUE(dws_ip_parse("11.0.0.1", &out8));
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&in8, &net8, 8));
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&out8, &net8, 8));

    // /32 is a single host; /0 matches everything of the family.
    DWSIp any;
    TEST_ASSERT_TRUE(dws_ip_parse("203.0.113.7", &any));
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&any, &any, 32));
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&out24, &any, 32));
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&any, &net8, 0));

    // IPv6 CIDR containment on the full 128-bit address.
    DWSIp v6net, v6in, v6out;
    TEST_ASSERT_TRUE(dws_ip_parse("2001:db8::", &v6net));
    TEST_ASSERT_TRUE(dws_ip_parse("2001:db8:0:0:1234::abcd", &v6in));
    TEST_ASSERT_TRUE(dws_ip_parse("2001:db9::1", &v6out));
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&v6in, &v6net, 32));
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&v6out, &v6net, 32));

    // Cross-family never matches, and an over-long prefix is rejected.
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&v6in, &net24, 24));  // v6 addr vs v4 net
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&in24, &v6net, 24));  // v4 addr vs v6 net
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&in24, &net24, 33));  // prefix > 32 for v4
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&v6in, &v6net, 129)); // prefix > 128 for v6
}

void test_ip_classify_equal_cidr_and_parse_edges()
{
    // classify: null and a DWSIpFamily::DWS_IP_NONE address are UNSPECIFIED.
    TEST_ASSERT_EQUAL_INT(DWSIpScope::DWS_IP_SCOPE_UNSPECIFIED, dws_ip_classify(nullptr));
    DWSIp none = {};
    DWSIp none2 = {};
    TEST_ASSERT_EQUAL_INT(DWSIpScope::DWS_IP_SCOPE_UNSPECIFIED, dws_ip_classify(&none));
    // equal: two DWSIpFamily::DWS_IP_NONE addresses compare equal (same non-address family).
    TEST_ASSERT_TRUE(dws_ip_equal(&none, &none2));
    // CIDR with a non-byte-aligned prefix: the partial high bits of the boundary byte must match.
    DWSIp net = dws_ip_from_v4_octets(192, 168, 0, 0);
    DWSIp in = dws_ip_from_v4_octets(192, 168, 15, 255); // 3rd byte 0x0F, high nibble 0 -> in /20
    DWSIp out = dws_ip_from_v4_octets(192, 168, 16, 0);  // 3rd byte 0x10, high nibble 1 -> out of /20
    TEST_ASSERT_TRUE(dws_ip_prefix_match(&in, &net, 20));
    TEST_ASSERT_FALSE(dws_ip_prefix_match(&out, &net, 20));
    // Parse edges: a lone leading ':' and an over-long string are rejected.
    DWSIp p;
    TEST_ASSERT_FALSE(dws_ip_parse(":1", &p));
    char toolong[80];
    for (int i = 0; i < 79; i++)
        toolong[i] = '1';
    toolong[79] = '\0';
    TEST_ASSERT_FALSE(dws_ip_parse(toolong, &p));
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3", &p));             // too few octets
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3.4.5", &p));         // too many octets
    TEST_ASSERT_FALSE(dws_ip_parse("256.0.0.1", &p));         // octet out of range
    TEST_ASSERT_FALSE(dws_ip_parse("gg::1", &p));             // invalid hex
    TEST_ASSERT_FALSE(dws_ip_parse("1:2:3:4:5:6:7:8:9", &p)); // too many v6 groups
    TEST_ASSERT_FALSE(dws_ip_parse(nullptr, &p));             // null string
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3.4", nullptr));      // null out
    TEST_ASSERT_FALSE(dws_ip_parse("1234.5.6.7", &p));        // octet has 4 digits
    TEST_ASSERT_FALSE(dws_ip_parse("1.2.3.z", &p));           // invalid char in v4
    // format guards: null ptr, non-v6/v4 family, and buffers too small for v4/v6/mapped.
    char buf[64];
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_format(nullptr, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_format(&none, buf, sizeof(buf))); // DWSIpFamily::DWS_IP_NONE family
    DWSIp v4 = dws_ip_from_v4_octets(255, 255, 255, 255);
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_format(&v4, buf, 4)); // "255.255.255.255" needs 16
    DWSIp v6;
    TEST_ASSERT_TRUE(dws_ip_parse("2001:db8::1", &v6));
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_format(&v6, buf, 4)); // v6 text needs more than 4
    DWSIp mapped;
    TEST_ASSERT_TRUE(dws_ip_parse("::ffff:1.2.3.4", &mapped));
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_format(&mapped, buf, 5)); // "::ffff:1.2.3.4" tail overflows
    // to_v4_be guards: null and a pure (non-mapped) v6 both yield 0.
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_to_v4_be(nullptr));
    TEST_ASSERT_EQUAL_UINT(0, dws_ip_to_v4_be(&v6));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_v4_round_trip);
    RUN_TEST(test_from_v6_bytes);
    RUN_TEST(test_is_unspecified);
    RUN_TEST(test_prefix_match);
    RUN_TEST(test_v6_canonical_5952);
    RUN_TEST(test_v4_mapped);
    RUN_TEST(test_classify_v4);
    RUN_TEST(test_classify_v6);
    RUN_TEST(test_reject_malformed);
    RUN_TEST(test_equal_and_from_v4);
    RUN_TEST(test_ip_classify_equal_cidr_and_parse_edges);
    return UNITY_END();
}
