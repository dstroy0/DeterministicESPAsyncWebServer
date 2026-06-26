// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the dashboard widget-table JSON serializers (services/dashboard
// core). Pure logic - no server - so it runs on the host.

#include "services/dashboard/dashboard.h"
#include <unity.h>

static const DetwsWidget W[] = {
    {DETWS_WIDGET_GAUGE, "Temp", "temp", 0, 100, "C"},
    {DETWS_WIDGET_VALUE, "Count", "count", 0, 0, ""},
};

void setUp()
{
    detws_dashboard_configure(W, 2);
}
void tearDown()
{
}

// The widget table serializes to the layout JSON the page renders from.
void test_layout_json()
{
    char buf[512];
    int n = detws_dashboard_layout_json(buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING(
        "[{\"type\":\"gauge\",\"label\":\"Temp\",\"key\":\"temp\",\"min\":0,\"max\":100,\"unit\":\"C\"},"
        "{\"type\":\"value\",\"label\":\"Count\",\"key\":\"count\",\"min\":0,\"max\":0,\"unit\":\"\"}]",
        buf);
}

// Fresh configure() zeroes every value.
void test_values_json_initial_zero()
{
    char buf[256];
    detws_dashboard_values_json(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"temp\":0,\"count\":0}", buf);
}

// set() updates by key and the values JSON reflects it.
void test_set_and_values()
{
    TEST_ASSERT_TRUE(detws_dashboard_set("temp", 23.5f));
    TEST_ASSERT_TRUE(detws_dashboard_set("count", 7.0f));
    char buf[256];
    detws_dashboard_values_json(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"temp\":23.5,\"count\":7}", buf);
}

// An unknown key is rejected.
void test_set_unknown_key()
{
    TEST_ASSERT_FALSE(detws_dashboard_set("nope", 1.0f));
}

// Reconfiguring resets the values.
void test_configure_resets_values()
{
    detws_dashboard_set("temp", 99.0f);
    detws_dashboard_configure(W, 2);
    char buf[256];
    detws_dashboard_values_json(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"temp\":0,\"count\":0}", buf);
}

// A buffer too small to hold the JSON fails closed (returns 0, no overflow).
void test_small_buffer_fails_closed()
{
    char buf[8];
    TEST_ASSERT_EQUAL_INT(0, detws_dashboard_layout_json(buf, sizeof(buf)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_layout_json);
    RUN_TEST(test_values_json_initial_zero);
    RUN_TEST(test_set_and_values);
    RUN_TEST(test_set_unknown_key);
    RUN_TEST(test_configure_resets_values);
    RUN_TEST(test_small_buffer_fails_closed);
    return UNITY_END();
}
