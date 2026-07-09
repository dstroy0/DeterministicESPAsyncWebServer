// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/mdns_adaptive: RF-aware backoff, TTL refresher, auto-sleep beacon.

#include "services/mdns_adaptive/mdns_adaptive.h"
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_refresh_interval(void)
{
    TEST_ASSERT_EQUAL_UINT32(60000, detws_mdns_refresh_interval(120)); // 120s TTL -> refresh at 60s
    TEST_ASSERT_EQUAL_UINT32(0, detws_mdns_refresh_interval(0));
}

void test_backoff_and_recover(void)
{
    MdnsBeacon b;
    detws_mdns_beacon_init(&b, 60000, 480000, 5); // base 60s, ceil 480s, backoff at 5 contenders
    // Crowded: double each window, capped at ceiling.
    TEST_ASSERT_EQUAL_UINT32(120000, detws_mdns_beacon_adapt(&b, 8));
    TEST_ASSERT_EQUAL_UINT32(240000, detws_mdns_beacon_adapt(&b, 8));
    TEST_ASSERT_EQUAL_UINT32(480000, detws_mdns_beacon_adapt(&b, 8));
    TEST_ASSERT_EQUAL_UINT32(480000, detws_mdns_beacon_adapt(&b, 8)); // clamped at ceiling
    // Moderate contention (below threshold, nonzero): hold.
    TEST_ASSERT_EQUAL_UINT32(480000, detws_mdns_beacon_adapt(&b, 3));
    // Quiet air: halve back toward base.
    TEST_ASSERT_EQUAL_UINT32(240000, detws_mdns_beacon_adapt(&b, 0));
    TEST_ASSERT_EQUAL_UINT32(120000, detws_mdns_beacon_adapt(&b, 0));
    TEST_ASSERT_EQUAL_UINT32(60000, detws_mdns_beacon_adapt(&b, 0));
    TEST_ASSERT_EQUAL_UINT32(60000, detws_mdns_beacon_adapt(&b, 0)); // clamped at base
}

void test_due(void)
{
    MdnsBeacon b;
    detws_mdns_beacon_init(&b, 60000, 480000, 5);
    TEST_ASSERT_FALSE(detws_mdns_beacon_due(&b, 1000, 1000 + 59999));
    TEST_ASSERT_TRUE(detws_mdns_beacon_due(&b, 1000, 1000 + 60000));
    TEST_ASSERT_TRUE(detws_mdns_beacon_due(&b, 1000, 1000 + 120000));
    // Wrap-safe: last just below the uint32 rollover, now just after.
    TEST_ASSERT_TRUE(detws_mdns_beacon_due(&b, 0xFFFFFF00u, 0xFFFFFF00u + 60000));
}

void test_presleep(void)
{
    MdnsBeacon b;
    detws_mdns_beacon_init(&b, 60000, 480000, 5);
    // 10s since last announce; a 40s sleep would leave us at 50s < 60s -> not due yet.
    TEST_ASSERT_FALSE(detws_mdns_beacon_presleep_due(&b, 0, 10000, 40000));
    // 10s since last; a 60s sleep would reach 70s >= 60s -> announce before sleeping.
    TEST_ASSERT_TRUE(detws_mdns_beacon_presleep_due(&b, 0, 10000, 60000));
    // Huge sleep must not overflow the elapsed+sleep sum.
    TEST_ASSERT_TRUE(detws_mdns_beacon_presleep_due(&b, 0, 0xFFFFFFF0u, 0xFFFFFFF0u));
}

void test_refresh_interval_and_beacon()
{
    (void)detws_mdns_refresh_interval(0); // ttl 0 edge
    (void)detws_mdns_refresh_interval(3600);
    MdnsBeacon b;
    detws_mdns_beacon_init(&b, 1000, 60000, 3);
    detws_mdns_beacon_adapt(&b, 5);                           // high contention
    detws_mdns_beacon_adapt(&b, 0);                           // no contention
    TEST_ASSERT_FALSE(detws_mdns_beacon_due(&b, 1000, 1000)); // not yet due
    detws_mdns_beacon_init(nullptr, 0, 0, 0);                 // null guard
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_refresh_interval);
    RUN_TEST(test_backoff_and_recover);
    RUN_TEST(test_due);
    RUN_TEST(test_presleep);
    RUN_TEST(test_refresh_interval_and_beacon);
    return UNITY_END();
}
