// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the preempting work queue (services/preempt_queue) host core: the
// fixed ring's order (FIFO), urgent-to-front, fail-closed-when-full, high-water,
// and the drain/handler dispatch. The ARDUINO FreeRTOS task + ISR post + preempt
// latency are HW-verified separately (the host has no tasks/ISRs).

#include "services/preempt_queue/preempt_queue.h"
#include <string.h> // memcpy
#include <unity.h>
#include <vector>

static std::vector<uint32_t> g_seen;
static std::vector<uint32_t> g_seen_dma; // items drained on the internal DMA lane

static void on_item(const void *item, void *ctx)
{
    (void)ctx;
    uint32_t v;
    memcpy(&v, item, sizeof(v));
    g_seen.push_back(v);
}

static void on_item_dma(const void *item, void *ctx)
{
    (void)ctx;
    uint32_t v;
    memcpy(&v, item, sizeof(v));
    g_seen_dma.push_back(v);
}

static bool post_u32(uint32_t v)
{
    return detws_pq_post(&v, 0);
}

static void stop_all_lanes()
{
    for (int l = 0; l < DETWS_PQ_LANE_COUNT; l++)
        detws_pq_stop_lane((detws_pq_lane)l);
}

void setUp()
{
    g_seen.clear();
    g_seen_dma.clear();
    stop_all_lanes();
    DetwsPqConfig cfg = {};
    cfg.handler = on_item;
    cfg.ctx = nullptr;
    cfg.priority = 5;
    cfg.core = 1;
    cfg.name = "test_pq";
    detws_pq_start(&cfg); // starts the USER lane (no-arg API)
}
void tearDown()
{
    stop_all_lanes();
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

// --- Named-lane tests -----------------------------------------------------------------

void test_internal_lanes_outrank_user()
{
    // DMA highest, then forward, then device, all above the user lane.
    TEST_ASSERT_GREATER_THAN_UINT8(detws_pq_lane_priority(DETWS_PQ_LANE_FORWARD),
                                   detws_pq_lane_priority(DETWS_PQ_LANE_DMA));
    TEST_ASSERT_GREATER_THAN_UINT8(detws_pq_lane_priority(DETWS_PQ_LANE_DEVICE),
                                   detws_pq_lane_priority(DETWS_PQ_LANE_FORWARD));
    TEST_ASSERT_GREATER_THAN_UINT8(detws_pq_lane_priority(DETWS_PQ_LANE_USER),
                                   detws_pq_lane_priority(DETWS_PQ_LANE_DEVICE));
}

void test_lanes_are_isolated()
{
    // The USER lane is already started by setUp; start the internal DMA lane too.
    DetwsPqConfig dma = {};
    dma.handler = on_item_dma;
    dma.core = 1;
    TEST_ASSERT_TRUE(detws_pq_start_lane(DETWS_PQ_LANE_DMA, &dma));

    uint32_t u = 11, d = 22;
    TEST_ASSERT_TRUE(detws_pq_post(&u, 0));                         // -> USER
    TEST_ASSERT_TRUE(detws_pq_post_lane(DETWS_PQ_LANE_DMA, &d, 0)); // -> DMA

    // Draining one lane must not touch the other's queue or handler.
    detws_pq_drain_lane(DETWS_PQ_LANE_DMA);
    TEST_ASSERT_EQUAL_size_t(0, g_seen.size());
    TEST_ASSERT_EQUAL_size_t(1, g_seen_dma.size());
    TEST_ASSERT_EQUAL_UINT32(22, g_seen_dma[0]);

    detws_pq_drain(); // USER
    TEST_ASSERT_EQUAL_size_t(1, g_seen.size());
    TEST_ASSERT_EQUAL_UINT32(11, g_seen[0]);
}

void test_lane_start_stop_running_independent()
{
    TEST_ASSERT_TRUE(detws_pq_running_lane(DETWS_PQ_LANE_USER)); // setUp started it
    TEST_ASSERT_FALSE(detws_pq_running_lane(DETWS_PQ_LANE_DMA));

    DetwsPqConfig dma = {};
    dma.handler = on_item_dma;
    TEST_ASSERT_TRUE(detws_pq_start_lane(DETWS_PQ_LANE_DMA, &dma));
    TEST_ASSERT_TRUE(detws_pq_running_lane(DETWS_PQ_LANE_DMA));
    TEST_ASSERT_FALSE(detws_pq_start_lane(DETWS_PQ_LANE_DMA, &dma)); // double start is a no-op

    detws_pq_stop_lane(DETWS_PQ_LANE_DMA);
    TEST_ASSERT_FALSE(detws_pq_running_lane(DETWS_PQ_LANE_DMA));
    TEST_ASSERT_TRUE(detws_pq_running_lane(DETWS_PQ_LANE_USER)); // USER unaffected
}

void test_lane_high_water_is_per_lane()
{
    DetwsPqConfig dma = {};
    dma.handler = on_item_dma;
    TEST_ASSERT_TRUE(detws_pq_start_lane(DETWS_PQ_LANE_DMA, &dma));
    uint32_t v = 5;
    detws_pq_post_lane(DETWS_PQ_LANE_DMA, &v, 0);
    detws_pq_post_lane(DETWS_PQ_LANE_DMA, &v, 0);
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(2, detws_pq_high_water_lane(DETWS_PQ_LANE_DMA));
    TEST_ASSERT_EQUAL_size_t(0, detws_pq_high_water_lane(DETWS_PQ_LANE_DEVICE)); // untouched lane
}

void test_lane_api_urgent_and_drain()
{
    stop_all_lanes();
    DetwsPqConfig cfg = {};
    cfg.handler = on_item_dma;
    TEST_ASSERT_TRUE(detws_pq_start_lane(DETWS_PQ_LANE_DMA, &cfg));
    uint32_t a = 10, b = 20;
    TEST_ASSERT_TRUE(detws_pq_post_lane(DETWS_PQ_LANE_DMA, &a, 0));
    TEST_ASSERT_TRUE(detws_pq_post_lane_urgent(DETWS_PQ_LANE_DMA, &b, 0)); // urgent -> jumps the queue
    detws_pq_drain_lane(DETWS_PQ_LANE_DMA);
    TEST_ASSERT_EQUAL_UINT32(2u, (uint32_t)g_seen_dma.size());
    TEST_ASSERT_EQUAL_UINT32(20u, g_seen_dma[0]); // urgent item first
    TEST_ASSERT_EQUAL_UINT32(10u, g_seen_dma[1]);
    // Guards: urgent-post to a bad lane / with a null item fails closed; drain of a bad lane is a no-op.
    TEST_ASSERT_FALSE(detws_pq_post_lane_urgent((detws_pq_lane)DETWS_PQ_LANE_COUNT, &a, 0));
    TEST_ASSERT_FALSE(detws_pq_post_lane_urgent(DETWS_PQ_LANE_DMA, nullptr, 0));
    detws_pq_drain_lane((detws_pq_lane)DETWS_PQ_LANE_COUNT);
    detws_pq_stop_lane(DETWS_PQ_LANE_DMA);
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
    RUN_TEST(test_internal_lanes_outrank_user);
    RUN_TEST(test_lanes_are_isolated);
    RUN_TEST(test_lane_start_stop_running_independent);
    RUN_TEST(test_lane_high_water_is_per_lane);
    RUN_TEST(test_lane_api_urgent_and_drain);
    return UNITY_END();
}
