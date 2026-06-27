// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the radio-power mode names (services/radio_power). Applying the
// settings to the radio is ESP32-only (a no-op on host).

#include "services/radio_power/radio_power.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_ps_names()
{
    TEST_ASSERT_EQUAL_STRING("none", detws_radio_ps_name(DETWS_PS_NONE));
    TEST_ASSERT_EQUAL_STRING("min_modem", detws_radio_ps_name(DETWS_PS_MIN_MODEM));
    TEST_ASSERT_EQUAL_STRING("max_modem", detws_radio_ps_name(DETWS_PS_MAX_MODEM));
    TEST_ASSERT_EQUAL_STRING("none", detws_radio_ps_name(99)); // unknown -> none
}

void test_apply_is_noop_on_host()
{
    detws_radio_power_apply(); // must not crash
    TEST_ASSERT_EQUAL_UINT8(DETWS_PS_NONE, detws_radio_ps_get());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ps_names);
    RUN_TEST(test_apply_is_noop_on_host);
    return UNITY_END();
}
