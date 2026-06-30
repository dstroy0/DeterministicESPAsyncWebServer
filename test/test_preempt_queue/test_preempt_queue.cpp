// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the preempting work queue (services/preempt_queue) host core: the
// fixed ring's order (FIFO), urgent-to-front, fail-closed-when-full, high-water,
// and the drain/handler dispatch. The ARDUINO FreeRTOS task + ISR post + preempt
// latency are HW-verified separately (the host has no tasks/ISRs).

#include "services/preempt_queue/preempt_queue.h"
#include <unity.h>
#include <vector>

static std::vector<uint32_t> g_seen;

static void on_item(const void *item, void *ctx)
{
    (void)ctx;
    uint32_t v;
    memcpy(&v, item, sizeof(v));
    g_seen.push_back(v);
}

static bool post_u32(uint32_t v)
{
    return detws_pq_post(&v, 0);
}

void setUp()
{
    g_seen.clear();
    detws_pq_stop();
    DetwsPqConfig cfg = {};
    cfg.handler = on_item;
    cfg.ctx = nullptr;
    cfg.priority = 5;
    cfg.core = 1;
    cfg.name = "test_pq";
    detws_pq_start(&cfg);
}
void tearDown()
{
    detws_pq_stop();
}

void test_start_validates_and_runs()
{
    detws_pq_stop();
    TEST_ASSERT_FALSE(detws_pq_start(nullptr)); // null config
    DetwsPqConfig bad = {};
    bad.handler = nullptr;
    TEST_ASSERT_FALSE(detws_pq_start(&bad)); // null handler
    DetwsPqConfig ok = {};
    ok.handler = on_item;
    TEST_ASSERT_TRUE(detws_pq_start(&ok));
    TEST_ASSERT_TRUE(detws_pq_running());
    TEST_ASSERT_FALSE(detws_pq_start(&ok)); // double start is a no-op
}

void test_fifo_order()
{
    TEST_ASSERT_TRUE(post_u32(10));
    TEST_ASSERT_TRUE(post_u32(20));
    TEST_ASSERT_TRUE(post_u32(30));
    detws_pq_drain();
    TEST_ASSERT_EQUAL_size_t(3, g_seen.size());
    TEST_ASSERT_EQUAL_UINT32(10, g_seen[0]);
    TEST_ASSERT_EQUAL_UINT32(20, g_seen[1]);
    TEST_ASSERT_EQUAL_UINT32(30, g_seen[2]);
}

void test_urgent_goes_to_front()
{
    post_u32(1);
    post_u32(2);
    uint32_t u = 99;
    TEST_ASSERT_TRUE(detws_pq_post_urgent(&u, 0));
    detws_pq_drain();
    TEST_ASSERT_EQUAL_size_t(3, g_seen.size());
    TEST_ASSERT_EQUAL_UINT32(99, g_seen[0]); // urgent first
    TEST_ASSERT_EQUAL_UINT32(1, g_seen[1]);
    TEST_ASSERT_EQUAL_UINT32(2, g_seen[2]);
}

void test_fail_closed_when_full()
{
    // The test env sizes DETWS_PQ_DEPTH = 4.
    for (uint32_t i = 0; i < DETWS_PQ_DEPTH; i++)
        TEST_ASSERT_TRUE(post_u32(i));
    TEST_ASSERT_FALSE(post_u32(999)); // full -> dropped, not blocked
    detws_pq_drain();
    TEST_ASSERT_EQUAL_size_t(DETWS_PQ_DEPTH, g_seen.size());
}

void test_high_water_tracks_peak()
{
    post_u32(1);
    post_u32(2);
    post_u32(3);
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(3, detws_pq_high_water());
    detws_pq_drain();
    // peak persists after draining
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(3, detws_pq_high_water());
}

void test_from_isr_enqueues()
{
    uint32_t v = 7;
    TEST_ASSERT_TRUE(detws_pq_post_from_isr(&v));
    detws_pq_drain();
    TEST_ASSERT_EQUAL_size_t(1, g_seen.size());
    TEST_ASSERT_EQUAL_UINT32(7, g_seen[0]);
}

void test_drain_empties_and_reuses()
{
    post_u32(1);
    detws_pq_drain();
    g_seen.clear();
    detws_pq_drain(); // empty: no-op
    TEST_ASSERT_EQUAL_size_t(0, g_seen.size());
    // ring wraps cleanly after a drain
    for (uint32_t i = 0; i < DETWS_PQ_DEPTH; i++)
        TEST_ASSERT_TRUE(post_u32(100 + i));
    detws_pq_drain();
    TEST_ASSERT_EQUAL_size_t(DETWS_PQ_DEPTH, g_seen.size());
    TEST_ASSERT_EQUAL_UINT32(100, g_seen[0]);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_start_validates_and_runs);
    RUN_TEST(test_fifo_order);
    RUN_TEST(test_urgent_goes_to_front);
    RUN_TEST(test_fail_closed_when_full);
    RUN_TEST(test_high_water_tracks_peak);
    RUN_TEST(test_from_isr_enqueues);
    RUN_TEST(test_drain_empties_and_reuses);
    return UNITY_END();
}
