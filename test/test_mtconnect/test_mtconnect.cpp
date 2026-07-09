// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/mtconnect: the MTConnectStreams + MTConnectError document builders.

#include "services/mtconnect/mtconnect.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static bool contains(const char *hay, const char *needle)
{
    return strstr(hay, needle) != nullptr;
}

void test_streams_document(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1500, 42, "cnc1");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Availability", "avail", 40, "2026-07-06T00:00:00Z", "AVAILABLE");
    detws_mtc_streams_add(&s, DETWS_MTC_SAMPLE, "Position", "xpos", 41, "2026-07-06T00:00:01Z", "12.5");
    detws_mtc_streams_add(&s, DETWS_MTC_CONDITION, "SystemCondition", "sys", 42, "2026-07-06T00:00:02Z", "Fault");
    size_t n = detws_mtc_streams_end(&s);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);

    TEST_ASSERT_TRUE(contains(buf, "<MTConnectStreams"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\""));
    TEST_ASSERT_TRUE(contains(buf, "nextSequence=\"42\""));
    TEST_ASSERT_TRUE(contains(buf, "<DeviceStream name=\"cnc1\">"));
    TEST_ASSERT_TRUE(contains(buf, "<Events><Availability dataItemId=\"avail\" sequence=\"40\""));
    TEST_ASSERT_TRUE(contains(buf, ">AVAILABLE</Availability></Events>"));
    TEST_ASSERT_TRUE(contains(buf, "<Samples><Position dataItemId=\"xpos\" sequence=\"41\""));
    TEST_ASSERT_TRUE(contains(buf, ">12.5</Position></Samples>"));
    TEST_ASSERT_TRUE(contains(buf, "<Condition><Fault type=\"SystemCondition\" dataItemId=\"sys\""));
    // Properly closed.
    TEST_ASSERT_TRUE(contains(buf, "</ComponentStream></DeviceStream></Streams></MTConnectStreams>"));
}

void test_streams_escapes_value(void)
{
    char buf[512];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "d");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Message", "msg", 1, "T", "a<b&c");
    detws_mtc_streams_end(&s);
    TEST_ASSERT_TRUE(contains(buf, ">a&lt;b&amp;c</Message>"));
}

void test_error_document(void)
{
    char buf[512];
    size_t n = detws_mtc_error(1500, "UNSUPPORTED", "bad path", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(contains(buf, "<MTConnectError"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\""));
    TEST_ASSERT_TRUE(contains(buf, "<Error errorCode=\"UNSUPPORTED\">bad path</Error>"));
}

void test_overflow_returns_zero(void)
{
    char buf[40];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "device");
    detws_mtc_streams_add(&s, DETWS_MTC_SAMPLE, "Position", "x", 1, "T", "1.0");
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_streams_end(&s)); // did not fit
}

void test_escape_gt_quote_and_overflow()
{
    char buf[512];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "d");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Message", "msg", 1, "T", "a>b\"c");
    detws_mtc_streams_end(&s);
    TEST_ASSERT_NOT_NULL(strstr(buf, "&gt;"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "&quot;"));
    // Stream overflow closes at end(); error document overflow fails closed.
    char tiny[64];
    DetwsMtcStreams s2;
    detws_mtc_streams_begin(&s2, tiny, sizeof(tiny), 1, 1, "device-with-a-very-long-name");
    detws_mtc_streams_add(&s2, DETWS_MTC_SAMPLE, "Position", "xpos", 1, "2026-07-06T00:00:01Z", "12.5");
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_streams_end(&s2));
    char t2[8];
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_error(1, "X", "y", t2, sizeof(t2)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_streams_document);
    RUN_TEST(test_streams_escapes_value);
    RUN_TEST(test_error_document);
    RUN_TEST(test_overflow_returns_zero);
    RUN_TEST(test_escape_gt_quote_and_overflow);
    return UNITY_END();
}
