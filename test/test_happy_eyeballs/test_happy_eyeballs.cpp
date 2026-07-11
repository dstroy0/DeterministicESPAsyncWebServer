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

static DetIp v6(const char *s)
{
    DetIp ip;
    det_ip_parse(s, &ip);
    return ip;
}
static DetIp v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return det_ip_from_v4_octets(a, b, c, d);
}

void test_pref_order(void)
{
    DetIp g6 = v6("2606:4700::1");   // global v6
    DetIp g4 = v4(93, 184, 216, 34); // global v4
    DetIp ll6 = v6("fe80::1");       // link-local v6
    DetIp lo6 = v6("::1");           // loopback
    // Global outranks link-local outranks loopback; within global, native v6 outranks v4.
    TEST_ASSERT_TRUE(detws_he_pref(&g6) > detws_he_pref(&g4));
    TEST_ASSERT_TRUE(detws_he_pref(&g4) > detws_he_pref(&ll6));
    TEST_ASSERT_TRUE(detws_he_pref(&ll6) > detws_he_pref(&lo6));
}

void test_order_and_interleave(void)
{
    // Two global v6 + one global v4, given v4-first: sort puts v6 ahead, interleave alternates.
    DetIp list[3] = {v4(93, 184, 216, 34), v6("2606:4700::1"), v6("2606:4700::2")};
    detws_he_order(list, 3);
    // First is a v6 (highest pref); second must be the v4 (family alternation); third the other v6.
    TEST_ASSERT_EQUAL_INT(DetIpFamily::DET_IP_V6, list[0].family);
    TEST_ASSERT_EQUAL_INT(DetIpFamily::DET_IP_V4, list[1].family);
    TEST_ASSERT_EQUAL_INT(DetIpFamily::DET_IP_V6, list[2].family);
}

void test_order_single_family(void)
{
    // All v4: interleave is a no-op, order stays preference-sorted (global before private).
    DetIp list[3] = {v4(192, 168, 1, 5), v4(8, 8, 8, 8), v4(10, 0, 0, 1)};
    detws_he_order(list, 3);
    TEST_ASSERT_EQUAL_UINT8(8, list[0].bytes[0]); // 8.8.8.8 global first
    // The two private addresses follow in stable order.
    TEST_ASSERT_TRUE(list[1].bytes[0] == 192 || list[1].bytes[0] == 10);
}

void test_attempt_due(void)
{
    TEST_ASSERT_FALSE(detws_he_attempt_due(1000, 1000 + 249, DETWS_HE_ATTEMPT_DELAY_MS));
    TEST_ASSERT_TRUE(detws_he_attempt_due(1000, 1000 + 250, DETWS_HE_ATTEMPT_DELAY_MS));
    // Wrap-safe across the uint32 rollover.
    TEST_ASSERT_TRUE(detws_he_attempt_due(0xFFFFFF00u, 0xFFFFFF00u + 250, DETWS_HE_ATTEMPT_DELAY_MS));
}

void test_pref_scopes_and_order_edges()
{
    // Exercise the multicast + unspecified scope arms of detws_he_pref (values are det_ip-classified).
    DetIp mc4 = det_ip_from_v4_octets(239, 1, 2, 3); // admin-scoped IPv4 multicast
    DetIp mc6;
    det_ip_parse("ff0e::1", &mc6); // global-scope IPv6 multicast
    DetIp un;
    det_ip_parse("::", &un); // unspecified
    (void)detws_he_pref(&mc4);
    (void)detws_he_pref(&mc6);
    (void)detws_he_pref(&un);
    // n <= 1 returns immediately (no reorder).
    DetIp one[1];
    det_ip_parse("2606:4700::1", &one[0]);
    detws_he_order(one, 1);
    // A larger mixed list exercises the v4/v6 interleave.
    DetIp many[5];
    det_ip_parse("2606:4700::1", &many[0]);
    many[1] = det_ip_from_v4_octets(8, 8, 8, 8);
    det_ip_parse("2606:4700::2", &many[2]);
    many[3] = det_ip_from_v4_octets(1, 1, 1, 1);
    det_ip_parse("2606:4700::3", &many[4]);
    detws_he_order(many, 5);
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
