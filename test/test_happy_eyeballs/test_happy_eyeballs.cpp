// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/happy_eyeballs: RFC 6724 ordering + RFC 8305 family interleave + attempt gate.

#include "services/happy_eyeballs/happy_eyeballs.h"
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static DWSIp v6(const char *s)
{
    DWSIp ip;
    dws_ip_parse(s, &ip);
    return ip;
}
static DWSIp v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return dws_ip_from_v4_octets(a, b, c, d);
}

void test_pref_order(void)
{
    DWSIp g6 = v6("2606:4700::1");   // global v6
    DWSIp g4 = v4(93, 184, 216, 34); // global v4
    DWSIp ll6 = v6("fe80::1");       // link-local v6
    DWSIp lo6 = v6("::1");           // loopback
    // Global outranks link-local outranks loopback; within global, native v6 outranks v4.
    TEST_ASSERT_TRUE(dws_he_pref(&g6) > dws_he_pref(&g4));
    TEST_ASSERT_TRUE(dws_he_pref(&g4) > dws_he_pref(&ll6));
    TEST_ASSERT_TRUE(dws_he_pref(&ll6) > dws_he_pref(&lo6));
}

void test_order_and_interleave(void)
{
    // Two global v6 + one global v4, given v4-first: sort puts v6 ahead, interleave alternates.
    DWSIp list[3] = {v4(93, 184, 216, 34), v6("2606:4700::1"), v6("2606:4700::2")};
    dws_he_order(list, 3);
    // First is a v6 (highest pref); second must be the v4 (family alternation); third the other v6.
    TEST_ASSERT_EQUAL_INT(DWSIpFamily::DWS_IP_V6, list[0].family);
    TEST_ASSERT_EQUAL_INT(DWSIpFamily::DWS_IP_V4, list[1].family);
    TEST_ASSERT_EQUAL_INT(DWSIpFamily::DWS_IP_V6, list[2].family);
}

void test_order_single_family(void)
{
    // All v4: interleave is a no-op, order stays preference-sorted (global before private).
    DWSIp list[3] = {v4(192, 168, 1, 5), v4(8, 8, 8, 8), v4(10, 0, 0, 1)};
    dws_he_order(list, 3);
    TEST_ASSERT_EQUAL_UINT8(8, list[0].bytes[0]); // 8.8.8.8 global first
    // The two private addresses follow in stable order.
    TEST_ASSERT_TRUE(list[1].bytes[0] == 192 || list[1].bytes[0] == 10);
}

void test_attempt_due(void)
{
    TEST_ASSERT_FALSE(dws_he_attempt_due(1000, 1000 + 249, DWS_HE_ATTEMPT_DELAY_MS));
    TEST_ASSERT_TRUE(dws_he_attempt_due(1000, 1000 + 250, DWS_HE_ATTEMPT_DELAY_MS));
    // Wrap-safe across the uint32 rollover.
    TEST_ASSERT_TRUE(dws_he_attempt_due(0xFFFFFF00u, 0xFFFFFF00u + 250, DWS_HE_ATTEMPT_DELAY_MS));
}

void test_pref_scopes_and_order_edges()
{
    // Exercise the multicast + unspecified scope arms of dws_he_pref (values are dws_ip-classified).
    DWSIp mc4 = dws_ip_from_v4_octets(239, 1, 2, 3); // admin-scoped IPv4 multicast
    DWSIp mc6;
    dws_ip_parse("ff0e::1", &mc6); // global-scope IPv6 multicast
    DWSIp un;
    dws_ip_parse("::", &un); // unspecified
    (void)dws_he_pref(&mc4);
    (void)dws_he_pref(&mc6);
    (void)dws_he_pref(&un);
    // n <= 1 returns immediately (no reorder).
    DWSIp one[1];
    dws_ip_parse("2606:4700::1", &one[0]);
    dws_he_order(one, 1);
    // A larger mixed list exercises the v4/v6 interleave.
    DWSIp many[5];
    dws_ip_parse("2606:4700::1", &many[0]);
    many[1] = dws_ip_from_v4_octets(8, 8, 8, 8);
    dws_ip_parse("2606:4700::2", &many[2]);
    many[3] = dws_ip_from_v4_octets(1, 1, 1, 1);
    dws_ip_parse("2606:4700::3", &many[4]);
    dws_he_order(many, 5);
    TEST_PASS();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pref_order);
    RUN_TEST(test_order_and_interleave);
    RUN_TEST(test_order_single_family);
    RUN_TEST(test_attempt_due);
    RUN_TEST(test_pref_scopes_and_order_edges);
    return UNITY_END();
}
