// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/failsafe: the software watchdog / deadlock detector. Uses the explicit
// *_at(now) API with a synthetic clock, so no time mock is needed.

#include "services/failsafe/failsafe.h"
#include <string.h>
#include <unity.h>

static int s_fired_id = -1;
static const char *s_fired_name = nullptr;
static int s_fire_count = 0;

static void on_breach(int id, const char *name, void *arg)
{
    (void)arg;
    s_fired_id = id;
    s_fired_name = name;
    s_fire_count++;
}

void setUp(void)
{
    detws_failsafe_reset();
    detws_failsafe_on_breach(on_breach, nullptr);
    s_fired_id = -1;
    s_fired_name = nullptr;
    s_fire_count = 0;
}
void tearDown(void)
{
}

// The wrap-safe overdue predicate.
void test_overdue_predicate(void)
{
    TEST_ASSERT_FALSE(detws_lifeline_overdue(1000, 900, 200)); // 100 <= 200
    TEST_ASSERT_FALSE(detws_lifeline_overdue(1000, 800, 200)); // 200, not > 200
    TEST_ASSERT_TRUE(detws_lifeline_overdue(1000, 700, 200));  // 300 > 200
    // Across a millis() rollover: last_feed just before wrap, now just after.
    TEST_ASSERT_FALSE(detws_lifeline_overdue(50, 0xFFFFFF00u, 1000));         // gap 306 <= 1000
    TEST_ASSERT_TRUE(detws_lifeline_overdue(0x00000400u, 0xFFFFFF00u, 1000)); // gap 1284 > 1000
}

void test_register_and_not_overdue_when_fresh(void)
{
    int id = detws_failsafe_register_at("worker", 500, 1000);
    TEST_ASSERT_EQUAL_INT(0, id);
    // Just under the deadline: not overdue, no fire.
    TEST_ASSERT_EQUAL_UINT32(0, detws_failsafe_check_at(1400));
    TEST_ASSERT_EQUAL_INT(0, s_fire_count);
}

void test_breach_fires_once_then_clears_on_feed(void)
{
    int a = detws_failsafe_register_at("a", 500, 1000);
    // b has a huge deadline so it never trips during this test - a stays the only overdue lifeline.
    int b = detws_failsafe_register_at("b", 1000000, 1000);
    TEST_ASSERT_EQUAL_INT(0, a);
    TEST_ASSERT_EQUAL_INT(1, b);

    // Only a is overdue at t=1600 (age 600 > 500).
    uint32_t mask = detws_failsafe_check_at(1600);
    TEST_ASSERT_EQUAL_UINT32(1u << 0, mask); // only a
    TEST_ASSERT_EQUAL_INT(1, s_fire_count);
    TEST_ASSERT_EQUAL_INT(a, s_fired_id);
    TEST_ASSERT_EQUAL_STRING("a", s_fired_name);

    // Still overdue at 1700: stays in the mask but does NOT re-fire.
    mask = detws_failsafe_check_at(1700);
    TEST_ASSERT_EQUAL_UINT32(1u << 0, mask);
    TEST_ASSERT_EQUAL_INT(1, s_fire_count); // no re-fire

    // Feeding a clears the breach; a later miss fires again (a fresh episode).
    detws_failsafe_feed_at(a, 1700);
    TEST_ASSERT_EQUAL_UINT32(0, detws_failsafe_check_at(1800));       // fed, healthy
    TEST_ASSERT_EQUAL_UINT32(1u << 0, detws_failsafe_check_at(2300)); // overdue again
    TEST_ASSERT_EQUAL_INT(2, s_fire_count);
}

void test_registry_full(void)
{
    for (int i = 0; i < DETWS_FAILSAFE_MAX_LIFELINES; i++)
        TEST_ASSERT_EQUAL_INT(i, detws_failsafe_register_at("x", 100, 0));
    TEST_ASSERT_EQUAL_INT(-1, detws_failsafe_register_at("overflow", 100, 0));
}

void test_feed_bad_id(void)
{
    TEST_ASSERT_FALSE(detws_failsafe_feed_at(-1, 0));
    TEST_ASSERT_FALSE(detws_failsafe_feed_at(3, 0)); // not armed
}

void test_json(void)
{
    int a = detws_failsafe_register_at("motor", 500, 1000);
    (void)a;
    char buf[128];
    int n = detws_failsafe_json_at(1200, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"name\":\"motor\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"overdue\":false"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"age_ms\":200"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"deadline_ms\":500"));
    // After the deadline, overdue flips true.
    detws_failsafe_json_at(2000, buf, sizeof(buf));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"overdue\":true"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_overdue_predicate);
    RUN_TEST(test_register_and_not_overdue_when_fresh);
    RUN_TEST(test_breach_fires_once_then_clears_on_feed);
    RUN_TEST(test_registry_full);
    RUN_TEST(test_feed_bad_id);
    RUN_TEST(test_json);
    return UNITY_END();
}
