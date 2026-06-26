// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the dashboard widget-table JSON serializers (services/dashboard
// core). Pure logic - no server - so it runs on the host.

#include "services/dashboard/dashboard.h"
#include <stdio.h>
#include <string.h>
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

// A well-formed control message parses into a key + value.
void test_parse_control_ok()
{
    char key[32];
    float v = -1;
    TEST_ASSERT_TRUE(detws_dashboard_parse_control("{\"k\":\"toggle1\",\"v\":1}", key, sizeof(key), &v));
    TEST_ASSERT_EQUAL_STRING("toggle1", key);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, v);
}

// Fractional control values parse too.
void test_parse_control_float()
{
    char key[32];
    float v = 0;
    TEST_ASSERT_TRUE(detws_dashboard_parse_control("{\"k\":\"speed\",\"v\":3.5}", key, sizeof(key), &v));
    TEST_ASSERT_EQUAL_STRING("speed", key);
    TEST_ASSERT_EQUAL_FLOAT(3.5f, v);
}

// Malformed messages are rejected.
void test_parse_control_rejects_malformed()
{
    char key[32];
    float v;
    TEST_ASSERT_FALSE(detws_dashboard_parse_control("{\"k\":\"x\"}", key, sizeof(key), &v));
    TEST_ASSERT_FALSE(detws_dashboard_parse_control("{\"v\":1}", key, sizeof(key), &v));
    TEST_ASSERT_FALSE(detws_dashboard_parse_control("not json", key, sizeof(key), &v));
    TEST_ASSERT_FALSE(detws_dashboard_parse_control("{\"k\":\"x\",\"v\":}", key, sizeof(key), &v));
}

static char g_ctrl_key[32];
static float g_ctrl_val;
static int g_ctrl_calls;
static void ctrl_cb(const char *key, float value)
{
    snprintf(g_ctrl_key, sizeof(g_ctrl_key), "%s", key);
    g_ctrl_val = value;
    g_ctrl_calls++;
}

// dispatch parses and invokes the registered callback; malformed -> no call.
void test_dispatch_control_invokes_cb()
{
    g_ctrl_calls = 0;
    g_ctrl_val = 0;
    g_ctrl_key[0] = '\0';
    detws_dashboard_on_control(ctrl_cb);
    TEST_ASSERT_TRUE(detws_dashboard_dispatch_control("{\"k\":\"led\",\"v\":1}"));
    TEST_ASSERT_EQUAL_INT(1, g_ctrl_calls);
    TEST_ASSERT_EQUAL_STRING("led", g_ctrl_key);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, g_ctrl_val);
    TEST_ASSERT_FALSE(detws_dashboard_dispatch_control("garbage"));
    TEST_ASSERT_EQUAL_INT(1, g_ctrl_calls);
}

// The new control / chart widget types serialize with their type names.
void test_layout_control_types()
{
    static const DetwsWidget CW[] = {
        {DETWS_WIDGET_CHART, "C", "c", 0, 1, ""},
        {DETWS_WIDGET_BUTTON, "B", "b", 0, 0, ""},
        {DETWS_WIDGET_TOGGLE, "T", "t", 0, 1, ""},
        {DETWS_WIDGET_SLIDER, "S", "s", 0, 100, "%"},
    };
    detws_dashboard_configure(CW, 4);
    char buf[512];
    detws_dashboard_layout_json(buf, sizeof(buf));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"type\":\"chart\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"type\":\"button\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"type\":\"toggle\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"type\":\"slider\""));
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
    RUN_TEST(test_parse_control_ok);
    RUN_TEST(test_parse_control_float);
    RUN_TEST(test_parse_control_rejects_malformed);
    RUN_TEST(test_dispatch_control_invokes_cb);
    RUN_TEST(test_layout_control_types);
    return UNITY_END();
}
