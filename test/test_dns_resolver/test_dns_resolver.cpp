// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DNS answer classifier / verifier (services/dns_resolver).
// The actual resolve is ESP32-only (returns false on host).

#include "services/dns_resolver/dns_resolver.h"
#include <unity.h>

#define IPV4(a, b, c, d) (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))

void setUp()
{
}
void tearDown()
{
}

void test_classify()
{
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_UNSPECIFIED, dws_dns_resolver_classify(0u));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_BROADCAST, dws_dns_resolver_classify(0xFFFFFFFFu));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_LOOPBACK, dws_dns_resolver_classify(IPV4(127, 0, 0, 1)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_PRIVATE, dws_dns_resolver_classify(IPV4(10, 0, 0, 5)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_PRIVATE, dws_dns_resolver_classify(IPV4(172, 16, 0, 1)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_PRIVATE, dws_dns_resolver_classify(IPV4(192, 168, 1, 1)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_LINKLOCAL, dws_dns_resolver_classify(IPV4(169, 254, 1, 1)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_MULTICAST, dws_dns_resolver_classify(IPV4(224, 0, 0, 1)));
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_PUBLIC, dws_dns_resolver_classify(IPV4(8, 8, 8, 8)));
    // 172.32.x is OUTSIDE the 172.16/12 private block -> public.
    TEST_ASSERT_EQUAL_INT(DWSIpClass::DWS_IP_PUBLIC, dws_dns_resolver_classify(IPV4(172, 32, 0, 1)));
}

void test_verify_rejects_suspicious()
{
    TEST_ASSERT_FALSE(dws_dns_resolver_verify(IPV4(0, 0, 0, 0)));         // blocked / no answer
    TEST_ASSERT_FALSE(dws_dns_resolver_verify(IPV4(127, 0, 0, 1)));       // rebinding to localhost
    TEST_ASSERT_FALSE(dws_dns_resolver_verify(IPV4(255, 255, 255, 255))); // broadcast
    TEST_ASSERT_FALSE(dws_dns_resolver_verify(IPV4(224, 0, 0, 1)));       // multicast
}

void test_verify_accepts_plausible()
{
    TEST_ASSERT_TRUE(dws_dns_resolver_verify(IPV4(8, 8, 8, 8)));      // public
    TEST_ASSERT_TRUE(dws_dns_resolver_verify(IPV4(192, 168, 1, 50))); // private (LAN host)
    TEST_ASSERT_TRUE(dws_dns_resolver_verify(IPV4(169, 254, 0, 2)));  // link-local
}

void test_resolve_is_noop_on_host()
{
    uint32_t ip = 0xDEADBEEF;
    TEST_ASSERT_FALSE(dws_dns_resolver_resolve("example.com", &ip));
    TEST_ASSERT_FALSE(dws_dns_resolver_resolve_verified("example.com", &ip));
}

void test_resolve_verified_paths()
{
    uint32_t ip = 0;
    // resolve fails -> false.
    dws_dns_resolver_test_set_resolve(false, 0);
    TEST_ASSERT_FALSE(dws_dns_resolver_resolve_verified("example.com", &ip));
    // resolve succeeds but the answer is a loopback (DNS-rebinding) -> verify rejects it.
    dws_dns_resolver_test_set_resolve(true, IPV4(127, 0, 0, 1));
    TEST_ASSERT_FALSE(dws_dns_resolver_resolve_verified("example.com", &ip));
    // resolve succeeds and the answer is a plausible public address -> true, out_ip set.
    dws_dns_resolver_test_set_resolve(true, IPV4(8, 8, 8, 8));
    TEST_ASSERT_TRUE(dws_dns_resolver_resolve_verified("example.com", &ip));
    TEST_ASSERT_EQUAL_UINT32(IPV4(8, 8, 8, 8), ip);
    TEST_ASSERT_TRUE(dws_dns_resolver_resolve_verified("example.com", nullptr)); // null out_ip ok
    dws_dns_resolver_test_set_resolve(false, 0);                                 // reset the hook
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_classify);
    RUN_TEST(test_verify_rejects_suspicious);
    RUN_TEST(test_verify_accepts_plausible);
    RUN_TEST(test_resolve_is_noop_on_host);
    RUN_TEST(test_resolve_verified_paths);
    return UNITY_END();
}
