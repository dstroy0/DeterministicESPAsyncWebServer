// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/sockpool: the LRU connection-slot recycling pool.

#include "services/sockpool/sockpool.h"
#include <unity.h>

static SockSlot g_slots[3];
static SockPool g_pool;

void setUp(void)
{
    detws_sockpool_init(&g_pool, g_slots, 3);
}
void tearDown(void)
{
}

void test_acquire_free(void)
{
    size_t idx = 99;
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_FREE, detws_sockpool_acquire(&g_pool, 100, 10, &idx, nullptr));
    TEST_ASSERT_EQUAL_size_t(0, idx);
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_FREE, detws_sockpool_acquire(&g_pool, 101, 11, &idx, nullptr));
    TEST_ASSERT_EQUAL_size_t(1, idx);
    TEST_ASSERT_EQUAL_size_t(2, detws_sockpool_in_use(&g_pool));
    // Find.
    size_t found = 99;
    TEST_ASSERT_TRUE(detws_sockpool_find(&g_pool, 101, &found));
    TEST_ASSERT_EQUAL_size_t(1, found);
    TEST_ASSERT_FALSE(detws_sockpool_find(&g_pool, 999, &found));
}

void test_lru_recycle(void)
{
    // Fill: id 100@t10, 101@t20, 102@t30.
    detws_sockpool_acquire(&g_pool, 100, 10, nullptr, nullptr);
    detws_sockpool_acquire(&g_pool, 101, 20, nullptr, nullptr);
    detws_sockpool_acquire(&g_pool, 102, 30, nullptr, nullptr);
    TEST_ASSERT_EQUAL_size_t(3, detws_sockpool_in_use(&g_pool));
    // Acquire a 4th: the LRU (id 100, oldest tick) is recycled.
    size_t idx = 99;
    uint32_t evicted = 0;
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_RECYCLED, detws_sockpool_acquire(&g_pool, 103, 40, &idx, &evicted));
    TEST_ASSERT_EQUAL_UINT32(100, evicted);
    TEST_ASSERT_EQUAL_size_t(0, idx);                              // slot 0 held id 100
    TEST_ASSERT_FALSE(detws_sockpool_find(&g_pool, 100, nullptr)); // gone
    TEST_ASSERT_TRUE(detws_sockpool_find(&g_pool, 103, nullptr));  // now present
    TEST_ASSERT_EQUAL_size_t(3, detws_sockpool_in_use(&g_pool));   // still full
}

void test_touch_changes_lru(void)
{
    detws_sockpool_acquire(&g_pool, 100, 10, nullptr, nullptr);
    detws_sockpool_acquire(&g_pool, 101, 20, nullptr, nullptr);
    detws_sockpool_acquire(&g_pool, 102, 30, nullptr, nullptr);
    // Touch id 100 so it is no longer the LRU; now id 101 is oldest.
    size_t i100 = 99;
    detws_sockpool_find(&g_pool, 100, &i100);
    detws_sockpool_touch(&g_pool, i100, 50);
    uint32_t evicted = 0;
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_RECYCLED, detws_sockpool_acquire(&g_pool, 103, 60, nullptr, &evicted));
    TEST_ASSERT_EQUAL_UINT32(101, evicted);
}

void test_release_reopens_free(void)
{
    detws_sockpool_acquire(&g_pool, 100, 10, nullptr, nullptr);
    size_t idx = 99;
    detws_sockpool_find(&g_pool, 100, &idx);
    TEST_ASSERT_TRUE(detws_sockpool_release(&g_pool, idx));
    TEST_ASSERT_EQUAL_size_t(0, detws_sockpool_in_use(&g_pool));
    // Double release is a no-op false.
    TEST_ASSERT_FALSE(detws_sockpool_release(&g_pool, idx));
    // A subsequent acquire reuses a free slot (not a recycle).
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_FREE, detws_sockpool_acquire(&g_pool, 200, 20, nullptr, nullptr));
}

void test_empty_pool_fails(void)
{
    SockPool empty;
    detws_sockpool_init(&empty, nullptr, 0);
    TEST_ASSERT_EQUAL_INT(SockAcq::SOCK_ACQ_FAIL, detws_sockpool_acquire(&empty, 1, 1, nullptr, nullptr));
}

void test_null_guard_subconditions()
{
    detws_sockpool_init(nullptr, nullptr, 0); // null pool -> no-op
    detws_sockpool_touch(nullptr, 0, 0);      // null pool -> no-op
    size_t idx = 0;
    TEST_ASSERT_FALSE(detws_sockpool_find(nullptr, 1, &idx));    // null pool -> false
    TEST_ASSERT_EQUAL_size_t(0, detws_sockpool_in_use(nullptr)); // null pool -> 0
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_acquire_free);
    RUN_TEST(test_lru_recycle);
    RUN_TEST(test_touch_changes_lru);
    RUN_TEST(test_release_reopens_free);
    RUN_TEST(test_empty_pool_fails);
    RUN_TEST(test_null_guard_subconditions);
    return UNITY_END();
}
