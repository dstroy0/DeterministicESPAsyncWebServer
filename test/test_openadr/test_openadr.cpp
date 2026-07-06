// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/openadr: the OpenADR 3.0 event / report JSON builders.

#include "services/openadr/openadr.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static bool has(const char *hay, const char *needle)
{
    return strstr(hay, needle) != nullptr;
}

void test_event(void)
{
    OpenAdrInterval iv[2];
    iv[0] = {1720000000u, 3600, "SIMPLE", 1.0};
    iv[1] = {1720003600u, 3600, "PRICE", 0.125};
    char buf[1024];
    size_t n = detws_openadr_event("program-1", "peak", iv, 2, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_TRUE(has(buf, "\"objectType\":\"EVENT\""));
    TEST_ASSERT_TRUE(has(buf, "\"programID\":\"program-1\""));
    TEST_ASSERT_TRUE(has(buf, "\"eventName\":\"peak\""));
    TEST_ASSERT_TRUE(has(buf, "\"start\":1720000000"));
    TEST_ASSERT_TRUE(has(buf, "\"duration\":3600"));
    TEST_ASSERT_TRUE(has(buf, "\"type\":\"SIMPLE\",\"values\":[1.000]"));
    TEST_ASSERT_TRUE(has(buf, "\"type\":\"PRICE\",\"values\":[0.125]"));
}

void test_report_negative_value(void)
{
    char buf[512];
    detws_openadr_report("program-1", "event-9", "meter-A", -2.5, 1720000000u, buf, sizeof(buf));
    TEST_ASSERT_TRUE(has(buf, "\"objectType\":\"REPORT\""));
    TEST_ASSERT_TRUE(has(buf, "\"eventID\":\"event-9\""));
    TEST_ASSERT_TRUE(has(buf, "\"resourceName\":\"meter-A\""));
    TEST_ASSERT_TRUE(has(buf, "\"values\":[-2.500]"));
    TEST_ASSERT_TRUE(has(buf, "\"start\":1720000000"));
}

void test_json_escape(void)
{
    OpenAdrInterval iv = {0, 60, "SIMPLE", 0.0};
    char buf[512];
    detws_openadr_event("prog\"x", "a\\b", &iv, 1, buf, sizeof(buf));
    TEST_ASSERT_TRUE(has(buf, "\"programID\":\"prog\\\"x\""));
    TEST_ASSERT_TRUE(has(buf, "\"eventName\":\"a\\\\b\""));
}

void test_overflow(void)
{
    OpenAdrInterval iv = {0, 60, "SIMPLE", 1.0};
    char buf[16];
    TEST_ASSERT_EQUAL_size_t(0, detws_openadr_event("p", "e", &iv, 1, buf, sizeof(buf)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_event);
    RUN_TEST(test_report_negative_value);
    RUN_TEST(test_json_escape);
    RUN_TEST(test_overflow);
    return UNITY_END();
}
