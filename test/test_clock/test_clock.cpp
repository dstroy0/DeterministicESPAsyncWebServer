// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the pluggable monotonic clock (services/det_clock): the platform
// default, a custom clock divided down to the internal 1000 Hz, and revert.

#include "services/det_clock.h"
#include <Arduino.h> // set_millis (host mock)
#include <unity.h>

static uint32_t g_fake = 0;
static uint32_t fake_clock()
{
    return g_fake;
}

void setUp()
{
    detws_set_clock(nullptr, 0); // start each test on the platform default
}
void tearDown()
{
    detws_set_clock(nullptr, 0);
}

void test_default_is_platform_millis()
{
    set_millis(5000);
    TEST_ASSERT_EQUAL_UINT32(5000, detws_millis());
    set_millis(12345);
    TEST_ASSERT_EQUAL_UINT32(12345, detws_millis());
}

void test_custom_clock_divides_to_1000hz()
{
    detws_set_clock(fake_clock, 8000); // 8 kHz source -> divide by 8
    g_fake = 8000;
    TEST_ASSERT_EQUAL_UINT32(1000, detws_millis());
    g_fake = 16000;
    TEST_ASSERT_EQUAL_UINT32(2000, detws_millis());

    g_fake = 1000000;
    detws_set_clock(fake_clock, 1000000); // 1 MHz source -> divide by 1000
    TEST_ASSERT_EQUAL_UINT32(1000, detws_millis());
}

void test_sub_khz_source_not_divided()
{
    detws_set_clock(fake_clock, 500); // < 1000: cannot divide up, used as-is
    g_fake = 1234;
    TEST_ASSERT_EQUAL_UINT32(1234, detws_millis());
}

void test_revert_to_default()
{
    detws_set_clock(fake_clock, 1000);
    g_fake = 42;
    TEST_ASSERT_EQUAL_UINT32(42, detws_millis());
    detws_set_clock(nullptr, 0);
    set_millis(777);
    TEST_ASSERT_EQUAL_UINT32(777, detws_millis());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_default_is_platform_millis);
    RUN_TEST(test_custom_clock_divides_to_1000hz);
    RUN_TEST(test_sub_khz_source_not_divided);
    RUN_TEST(test_revert_to_default);
    return UNITY_END();
}
