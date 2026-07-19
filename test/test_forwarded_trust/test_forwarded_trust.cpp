// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the trusted-reverse-proxy forwarded-client resolver (services/forwarded_trust).
// A Forwarded / X-Forwarded-For address is client-spoofable, so it may only be believed when the real
// TCP peer is a configured trusted upstream. The resolver is pure (no sockets), so the host drives it
// directly. The security-critical property under test: a direct/untrusted peer's forwarded header is
// NEVER honored, and any malformed / obfuscated / unspecified token falls back to the TCP peer.

#include "network_drivers/network/ip.h"
#include "services/forwarded_trust/forwarded_trust.h"
#include <cstdio>
#include <unity.h>

static DWSIp v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return dws_ip_from_v4_octets(a, b, c, d);
}

static DWSIp v6(const char *s)
{
    DWSIp ip;
    ip.family = DWSIpFamily::DWS_IP_NONE;
    TEST_ASSERT_TRUE(dws_ip_parse(s, &ip));
    return ip;
}

void setUp()
{
    dws_forwarded_trust_reset();
}
void tearDown()
{
}

// An empty table trusts no header: contains() is always false and effective_ip keeps the TCP peer.
void test_empty_table_trusts_nothing()
{
    DWSIp peer = v4(203, 0, 113, 7);
    TEST_ASSERT_FALSE(dws_forwarded_trust_contains(&peer));
    DWSIp out;
    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&peer, "198.51.100.9", &out));
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &peer)); // fell back to the real peer
}

// A v4 CIDR matches inside its range and rejects outside / a v6 peer.
void test_v4_cidr_membership()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("10.0.0.0/8"));
    DWSIp in = v4(10, 4, 4, 1);
    DWSIp out_of = v4(11, 0, 0, 1);
    DWSIp six = v6("2001:db8::1");
    TEST_ASSERT_TRUE(dws_forwarded_trust_contains(&in));
    TEST_ASSERT_FALSE(dws_forwarded_trust_contains(&out_of));
    TEST_ASSERT_FALSE(dws_forwarded_trust_contains(&six)); // family mismatch never matches a v4 rule
}

// A v6 CIDR matches; a bare address is a host route (exact match only).
void test_v6_cidr_and_host_route()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("2001:db8::/32"));
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("192.0.2.5")); // bare = /32 host route
    DWSIp v6in = v6("2001:db8:abcd::1");
    DWSIp host = v4(192, 0, 2, 5);
    DWSIp host_nbr = v4(192, 0, 2, 6);
    TEST_ASSERT_TRUE(dws_forwarded_trust_contains(&v6in));
    TEST_ASSERT_TRUE(dws_forwarded_trust_contains(&host));
    TEST_ASSERT_FALSE(dws_forwarded_trust_contains(&host_nbr)); // the neighbor is not the host route
}

// Malformed CIDR strings are rejected and add nothing.
void test_add_cidr_rejects_malformed()
{
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr(nullptr));
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("not-an-ip"));
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("10.0.0.0/"));      // empty prefix
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("10.0.0.0/33"));    // over-long v4 prefix
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("2001:db8::/129")); // over-long v6 prefix
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("10.0.0.0/x"));     // non-digit prefix
    DWSIp any = v4(10, 0, 0, 1);
    TEST_ASSERT_FALSE(dws_forwarded_trust_contains(&any)); // nothing was added
}

// The table is bounded: adding past DWS_TRUSTED_PROXY_MAX fails.
void test_table_full()
{
    for (int i = 0; i < DWS_TRUSTED_PROXY_MAX; i++)
    {
        char cidr[24];
        snprintf(cidr, sizeof(cidr), "10.%d.0.0/16", i);
        TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr(cidr));
    }
    TEST_ASSERT_FALSE(dws_forwarded_trust_add_cidr("172.16.0.0/12")); // one past capacity
}

// A trusted proxy's valid forwarded client is honored (the lockout keys on the real client).
void test_trusted_peer_honors_forwarded()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("10.0.0.0/8"));
    DWSIp proxy = v4(10, 1, 2, 3);
    DWSIp out;
    TEST_ASSERT_TRUE(dws_forwarded_effective_ip(&proxy, "198.51.100.42", &out));
    DWSIp client = v4(198, 51, 100, 42);
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &client));
}

// A trusted proxy may forward a v6 client even over a v4 hop.
void test_trusted_peer_honors_v6_forwarded()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("10.0.0.0/8"));
    DWSIp proxy = v4(10, 9, 9, 9);
    DWSIp out;
    TEST_ASSERT_TRUE(dws_forwarded_effective_ip(&proxy, "2001:db8::abcd", &out));
    DWSIp client = v6("2001:db8::abcd");
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &client));
}

// THE security property: an untrusted (direct) peer's forwarded header is IGNORED - no spoofing.
void test_untrusted_peer_ignores_forwarded()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("10.0.0.0/8"));
    DWSIp attacker = v4(203, 0, 113, 66); // not in the trusted range
    DWSIp out;
    // The attacker sets X-Forwarded-For to a victim's address to try to lock the victim out.
    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&attacker, "198.51.100.1", &out));
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &attacker)); // keyed on the attacker's own address
}

// A trusted proxy with a malformed / obfuscated / unspecified / absent token keeps the TCP peer.
void test_trusted_peer_bad_token_falls_back()
{
    TEST_ASSERT_TRUE(dws_forwarded_trust_add_cidr("10.0.0.0/8"));
    DWSIp proxy = v4(10, 0, 0, 5);
    DWSIp out;

    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&proxy, "unknown", &out)); // RFC 7239 obfuscated
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &proxy));

    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&proxy, nullptr, &out)); // no header
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &proxy));

    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&proxy, "", &out)); // empty
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &proxy));

    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&proxy, "0.0.0.0", &out)); // unspecified
    TEST_ASSERT_TRUE(dws_ip_equal(&out, &proxy));
}

// Null-argument guards: null out fails; null peer leaves an unspecified out (never uninitialized).
void test_null_guards()
{
    DWSIp peer = v4(10, 0, 0, 1);
    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(&peer, "1.2.3.4", nullptr));
    DWSIp out;
    TEST_ASSERT_FALSE(dws_forwarded_effective_ip(nullptr, "1.2.3.4", &out));
    TEST_ASSERT_TRUE(dws_ip_is_unspecified(&out)); // written, not left uninitialized
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_table_trusts_nothing);
    RUN_TEST(test_v4_cidr_membership);
    RUN_TEST(test_v6_cidr_and_host_route);
    RUN_TEST(test_add_cidr_rejects_malformed);
    RUN_TEST(test_table_full);
    RUN_TEST(test_trusted_peer_honors_forwarded);
    RUN_TEST(test_trusted_peer_honors_v6_forwarded);
    RUN_TEST(test_untrusted_peer_ignores_forwarded);
    RUN_TEST(test_trusted_peer_bad_token_falls_back);
    RUN_TEST(test_null_guards);
    return UNITY_END();
}
