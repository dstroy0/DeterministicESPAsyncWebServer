// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the multi-source time fallback matrix (services/time_source):
// priority ordering, fallback when a source is unavailable, first-valid
// short-circuit (lower-priority callbacks not invoked), capacity, and reset.

#include "services/time_source/time_source.h"
#include <unity.h>

// Mock sources: each returns its g_* value (0 = unavailable) and counts calls.
static uint32_t g_gps, g_rtc, g_ntp;
static int g_gps_calls, g_rtc_calls, g_ntp_calls;
static uint32_t gps_fn()
{
    g_gps_calls++;
    return g_gps;
}
static uint32_t dws_rtc_fn()
{
    g_rtc_calls++;
    return g_rtc;
}
static uint32_t dws_ntp_fn()
{
    g_ntp_calls++;
    return g_ntp;
}

void setUp()
{
    dws_time_source_reset();
    g_gps = g_rtc = g_ntp = 0;
    g_gps_calls = g_rtc_calls = g_ntp_calls = 0;
}
void tearDown()
{
}

void test_single_source()
{
    TEST_ASSERT_TRUE(dws_time_source_add("ntp", 1, dws_ntp_fn));
    g_ntp = 1000;
    TEST_ASSERT_EQUAL_UINT32(1000, dws_time_now());
    TEST_ASSERT_EQUAL_STRING("ntp", dws_time_source_active());
}

void test_priority_order_lowest_value_wins()
{
    dws_time_source_add("ntp", 2, dws_ntp_fn);
    dws_time_source_add("gps", 0, gps_fn); // highest priority
    dws_time_source_add("rtc", 1, dws_rtc_fn);
    g_ntp = 100;
    g_gps = 200;
    g_rtc = 300;
    TEST_ASSERT_EQUAL_UINT32(200, dws_time_now()); // gps (priority 0)
    TEST_ASSERT_EQUAL_STRING("gps", dws_time_source_active());
}

void test_falls_back_when_primary_unavailable()
{
    dws_time_source_add("gps", 0, gps_fn);
    dws_time_source_add("rtc", 1, dws_rtc_fn);
    g_gps = 0; // gps has no fix
    g_rtc = 555;
    TEST_ASSERT_EQUAL_UINT32(555, dws_time_now());
    TEST_ASSERT_EQUAL_STRING("rtc", dws_time_source_active());
}

void test_all_unavailable_returns_zero()
{
    dws_time_source_add("gps", 0, gps_fn);
    dws_time_source_add("rtc", 1, dws_rtc_fn);
    TEST_ASSERT_EQUAL_UINT32(0, dws_time_now());
    TEST_ASSERT_NULL(dws_time_source_active());
}

void test_first_valid_short_circuits()
{
    dws_time_source_add("gps", 0, gps_fn);
    dws_time_source_add("rtc", 1, dws_rtc_fn);
    g_gps = 42;
    g_rtc = 99;
    TEST_ASSERT_EQUAL_UINT32(42, dws_time_now());
    TEST_ASSERT_EQUAL_INT(1, g_gps_calls); // gps queried
    TEST_ASSERT_EQUAL_INT(0, g_rtc_calls); // rtc never queried (gps already valid)
}

void test_fallback_queries_in_priority_order()
{
    dws_time_source_add("gps", 0, gps_fn);
    dws_time_source_add("rtc", 1, dws_rtc_fn);
    dws_time_source_add("ntp", 2, dws_ntp_fn);
    g_gps = 0;
    g_rtc = 7;
    g_ntp = 8;
    TEST_ASSERT_EQUAL_UINT32(7, dws_time_now());
    TEST_ASSERT_EQUAL_INT(1, g_gps_calls); // tried, invalid
    TEST_ASSERT_EQUAL_INT(1, g_rtc_calls); // tried, valid -> stop
    TEST_ASSERT_EQUAL_INT(0, g_ntp_calls); // never reached
    TEST_ASSERT_EQUAL_STRING("rtc", dws_time_source_active());
}

void test_table_full_rejects()
{
    for (int i = 0; i < DWS_TIME_SOURCE_MAX; i++)
        TEST_ASSERT_TRUE(dws_time_source_add("s", (uint8_t)i, dws_rtc_fn));
    TEST_ASSERT_FALSE(dws_time_source_add("overflow", 9, dws_rtc_fn));
}

void test_null_fn_rejected()
{
    TEST_ASSERT_FALSE(dws_time_source_add("bad", 0, nullptr));
}

void test_reset_clears_sources()
{
    dws_time_source_add("ntp", 0, dws_ntp_fn);
    g_ntp = 5;
    TEST_ASSERT_EQUAL_UINT32(5, dws_time_now());
    dws_time_source_reset();
    TEST_ASSERT_EQUAL_UINT32(0, dws_time_now());
    TEST_ASSERT_NULL(dws_time_source_active());
}

void test_http_date_from_active_source()
{
    // The HTTP Date header draws from the registry: no valid source -> nothing; a source with a
    // valid epoch -> the RFC 7231 IMF-fixdate for it. Null out / zero cap are rejected.
    char buf[40];
    TEST_ASSERT_EQUAL_UINT(0, dws_time_http_date(buf, sizeof(buf)));
    dws_time_source_add("rtc", 0, dws_rtc_fn);
    g_rtc = 784111777; // Sun, 06 Nov 1994 08:49:37 GMT
    TEST_ASSERT_TRUE(dws_time_http_date(buf, sizeof(buf)) > 0);
    TEST_ASSERT_EQUAL_STRING("Sun, 06 Nov 1994 08:49:37 GMT", buf);
    TEST_ASSERT_EQUAL_UINT(0, dws_time_http_date(nullptr, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT(0, dws_time_http_date(buf, 0));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_single_source);
    RUN_TEST(test_priority_order_lowest_value_wins);
    RUN_TEST(test_falls_back_when_primary_unavailable);
    RUN_TEST(test_all_unavailable_returns_zero);
    RUN_TEST(test_first_valid_short_circuits);
    RUN_TEST(test_fallback_queries_in_priority_order);
    RUN_TEST(test_table_full_rejects);
    RUN_TEST(test_null_fn_rejected);
    RUN_TEST(test_reset_clears_sources);
    RUN_TEST(test_http_date_from_active_source);
    return UNITY_END();
}
