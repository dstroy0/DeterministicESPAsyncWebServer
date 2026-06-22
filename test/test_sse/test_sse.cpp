// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit and stress tests for the Server-Sent Events connection pool (sse.h/cpp).
//
// Sections:
//   POOL        -- sse_init / sse_alloc / sse_find / sse_free invariants
//   WRITE       -- sse_write() guard conditions and return values
//   STRESS      -- sustained alloc/free cycles and multi-slot isolation

#include "network_drivers/presentation/sse.h"
#include <string.h>
#include <unity.h>

void setUp()
{
    sse_init();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].pcb = &_mock_pcb;
    }
}

void tearDown()
{
}

// ====================================================================
// POOL TESTS - sse_init()
// ====================================================================

void test_sse_pool_size()
{
    TEST_ASSERT_EQUAL(2, MAX_SSE_CONNS); // default
}

void test_sse_ids_match_indices_after_init()
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
        TEST_ASSERT_EQUAL(i, (int)sse_pool[i].sse_id);
}

void test_sse_all_inactive_after_init()
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
        TEST_ASSERT_FALSE(sse_pool[i].active);
}

void test_sse_path_empty_after_init()
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
        TEST_ASSERT_EQUAL('\0', sse_pool[i].path[0]);
}

// ====================================================================
// POOL TESTS - sse_alloc()
// ====================================================================

void test_sse_alloc_returns_non_null()
{
    TEST_ASSERT_NOT_NULL(sse_alloc(0, "/events"));
}

void test_sse_alloc_sets_active()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_TRUE(sse->active);
}

void test_sse_alloc_sets_slot_id()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_EQUAL(0, (int)sse->slot_id);
}

void test_sse_alloc_stores_path()
{
    SseConn *sse = sse_alloc(0, "/sensors");
    TEST_ASSERT_EQUAL_STRING("/sensors", sse->path);
}

void test_sse_alloc_stores_different_paths_per_slot()
{
    SseConn *s0 = sse_alloc(0, "/events");
    SseConn *s1 = sse_alloc(1, "/metrics");
    TEST_ASSERT_EQUAL_STRING("/events", s0->path);
    TEST_ASSERT_EQUAL_STRING("/metrics", s1->path);
}

void test_sse_alloc_path_truncated_to_max()
{
    // Build a path longer than MAX_PATH_LEN
    char long_path[MAX_PATH_LEN + 16];
    long_path[0] = '/';
    for (int i = 1; i < MAX_PATH_LEN + 15; i++)
        long_path[i] = 'x';
    long_path[MAX_PATH_LEN + 15] = '\0';

    SseConn *sse = sse_alloc(0, long_path);
    TEST_ASSERT_NOT_NULL(sse);
    TEST_ASSERT_EQUAL(MAX_PATH_LEN - 1, (int)strlen(sse->path));
    TEST_ASSERT_EQUAL('\0', sse->path[MAX_PATH_LEN - 1]);
}

void test_sse_alloc_pool_full_returns_null()
{
    TEST_ASSERT_NOT_NULL(sse_alloc(0, "/a"));
    TEST_ASSERT_NOT_NULL(sse_alloc(1, "/b"));
    TEST_ASSERT_NULL(sse_alloc(2, "/c")); // MAX_SSE_CONNS = 2
}

void test_sse_alloc_sse_id_is_pool_index()
{
    // First free slot is 0 → sse_id should be 0
    SseConn *s0 = sse_alloc(0, "/a");
    TEST_ASSERT_EQUAL(0, (int)s0->sse_id);
    // Second free slot is 1 → sse_id should be 1
    SseConn *s1 = sse_alloc(1, "/b");
    TEST_ASSERT_EQUAL(1, (int)s1->sse_id);
}

// ====================================================================
// POOL TESTS - sse_find()
// ====================================================================

void test_sse_find_returns_correct_conn()
{
    SseConn *allocated = sse_alloc(0, "/events");
    SseConn *found = sse_find(0);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_PTR(allocated, found);
}

void test_sse_find_returns_null_when_empty()
{
    TEST_ASSERT_NULL(sse_find(0));
}

void test_sse_find_returns_null_for_different_slot()
{
    sse_alloc(0, "/events");
    TEST_ASSERT_NULL(sse_find(1));
}

void test_sse_find_after_both_slots_allocated()
{
    sse_alloc(0, "/a");
    sse_alloc(1, "/b");
    TEST_ASSERT_NOT_NULL(sse_find(0));
    TEST_ASSERT_NOT_NULL(sse_find(1));
}

void test_sse_find_checks_slot_id_not_sse_id()
{
    // sse_pool[0] → slot 3; sse_find(3) must return it, not sse_find(0)
    SseConn *sse = sse_alloc(3, "/x");
    TEST_ASSERT_NULL(sse_find(0));
    TEST_ASSERT_NOT_NULL(sse_find(3));
    TEST_ASSERT_EQUAL_PTR(sse, sse_find(3));
}

// ====================================================================
// POOL TESTS - sse_free()
// ====================================================================

void test_sse_free_deactivates_slot()
{
    sse_alloc(0, "/events");
    sse_free(0);
    TEST_ASSERT_FALSE(sse_pool[0].active);
}

void test_sse_free_restores_sse_id()
{
    sse_alloc(0, "/events");
    sse_free(0);
    TEST_ASSERT_EQUAL(0, (int)sse_pool[0].sse_id);
}

void test_sse_free_makes_slot_findable_as_null()
{
    sse_alloc(0, "/events");
    sse_free(0);
    TEST_ASSERT_NULL(sse_find(0));
}

void test_sse_free_clears_path()
{
    sse_alloc(0, "/events");
    sse_free(0);
    TEST_ASSERT_EQUAL('\0', sse_pool[0].path[0]);
}

void test_sse_free_nop_on_unallocated()
{
    sse_free(2); // slot 2 was never allocated
    // No crash; pool state unchanged
    TEST_ASSERT_FALSE(sse_pool[0].active);
    TEST_ASSERT_FALSE(sse_pool[1].active);
    TEST_PASS();
}

void test_sse_alloc_after_free_succeeds()
{
    sse_alloc(0, "/events");
    sse_free(0);
    SseConn *sse = sse_alloc(0, "/new");
    TEST_ASSERT_NOT_NULL(sse);
    TEST_ASSERT_TRUE(sse->active);
    TEST_ASSERT_EQUAL_STRING("/new", sse->path);
}

void test_sse_free_only_frees_matching_slot()
{
    sse_alloc(0, "/a");
    sse_alloc(1, "/b");
    sse_free(0);
    TEST_ASSERT_FALSE(sse_pool[0].active);
    TEST_ASSERT_TRUE(sse_pool[1].active);
    TEST_ASSERT_EQUAL_STRING("/b", sse_pool[1].path);
}

// ====================================================================
// WRITE TESTS - sse_write()
// ====================================================================

void test_sse_write_null_data_returns_false()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_FALSE(sse_write(sse, nullptr, nullptr, nullptr));
}

void test_sse_write_returns_false_when_conn_not_active()
{
    SseConn *sse = sse_alloc(0, "/events");
    conn_pool[0].state = CONN_FREE; // slot not active
    TEST_ASSERT_FALSE(sse_write(sse, "hello", nullptr, nullptr));
}

void test_sse_write_returns_false_when_pcb_null()
{
    SseConn *sse = sse_alloc(0, "/events");
    conn_pool[0].pcb = nullptr;
    TEST_ASSERT_FALSE(sse_write(sse, "data", nullptr, nullptr));
}

void test_sse_write_data_only_returns_true()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_TRUE(sse_write(sse, "hello", nullptr, nullptr));
}

void test_sse_write_with_event_returns_true()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_TRUE(sse_write(sse, "payload", "update", nullptr));
}

void test_sse_write_with_id_returns_true()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_TRUE(sse_write(sse, "payload", nullptr, "42"));
}

void test_sse_write_with_all_fields_returns_true()
{
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_TRUE(sse_write(sse, "body", "status", "1"));
}

void test_sse_write_does_not_affect_other_slots()
{
    SseConn *s0 = sse_alloc(0, "/a");
    SseConn *s1 = sse_alloc(1, "/b");
    // Write to slot 0 -- slot 1 state must be unchanged
    sse_write(s0, "msg", nullptr, nullptr);
    TEST_ASSERT_TRUE(s1->active);
    TEST_ASSERT_EQUAL_STRING("/b", s1->path);
    TEST_ASSERT_EQUAL(1, (int)s1->slot_id);
}

// ====================================================================
// STRESS TESTS
// ====================================================================

// 100 alloc/free cycles on one slot -- no state accumulation
void stress_sse_alloc_free_100_cycles()
{
    for (int i = 0; i < 100; i++)
    {
        SseConn *sse = sse_alloc(0, "/events");
        TEST_ASSERT_NOT_NULL_MESSAGE(sse, "alloc failed");
        TEST_ASSERT_TRUE_MESSAGE(sse->active, "not active");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("/events", sse->path, "path wrong");
        sse_free(0);
        TEST_ASSERT_FALSE_MESSAGE(sse_pool[0].active, "still active after free");
    }
}

// Alloc/free across both slots in alternating order
void stress_sse_alloc_free_both_slots_alternating()
{
    for (int cycle = 0; cycle < 50; cycle++)
    {
        SseConn *s0 = sse_alloc(0, "/a");
        SseConn *s1 = sse_alloc(1, "/b");
        TEST_ASSERT_NOT_NULL(s0);
        TEST_ASSERT_NOT_NULL(s1);
        TEST_ASSERT_NULL(sse_alloc(2, "/c")); // pool full

        sse_free(1);
        SseConn *s1b = sse_alloc(1, "/new");
        TEST_ASSERT_NOT_NULL(s1b);
        TEST_ASSERT_EQUAL_STRING("/new", s1b->path);

        sse_free(0);
        sse_free(1);
    }
}

// 100 sse_write calls on one slot -- no crash, no state corruption
void stress_sse_write_100_calls()
{
    SseConn *sse = sse_alloc(0, "/events");
    for (int i = 0; i < 100; i++)
    {
        bool ok = sse_write(sse, "data", "update", "1");
        TEST_ASSERT_TRUE_MESSAGE(ok, "write failed");
    }
    // Slot still intact after 100 writes
    TEST_ASSERT_TRUE(sse->active);
    TEST_ASSERT_EQUAL(0, (int)sse->slot_id);
}

// find() across full pool -- returns correct entry regardless of pool order
void stress_sse_find_with_full_pool()
{
    SseConn *s0 = sse_alloc(0, "/x");
    SseConn *s1 = sse_alloc(1, "/y");
    for (int i = 0; i < 50; i++)
    {
        TEST_ASSERT_EQUAL_PTR(s0, sse_find(0));
        TEST_ASSERT_EQUAL_PTR(s1, sse_find(1));
        TEST_ASSERT_NULL(sse_find(2));
        TEST_ASSERT_NULL(sse_find(3));
    }
}

// Slot isolation: write to slot 0 must not corrupt slot 1 path or state
void stress_sse_write_slot_isolation()
{
    SseConn *s0 = sse_alloc(0, "/events");
    SseConn *s1 = sse_alloc(1, "/metrics");

    for (int i = 0; i < 50; i++)
        sse_write(s0, "event_data", "update", "123");

    TEST_ASSERT_EQUAL_STRING("/metrics", s1->path);
    TEST_ASSERT_TRUE(s1->active);
    TEST_ASSERT_EQUAL(1, (int)s1->slot_id);
    TEST_ASSERT_EQUAL(1, (int)s1->sse_id);
}

int main()
{
    UNITY_BEGIN();

    // Pool: init
    RUN_TEST(test_sse_pool_size);
    RUN_TEST(test_sse_ids_match_indices_after_init);
    RUN_TEST(test_sse_all_inactive_after_init);
    RUN_TEST(test_sse_path_empty_after_init);

    // Pool: alloc
    RUN_TEST(test_sse_alloc_returns_non_null);
    RUN_TEST(test_sse_alloc_sets_active);
    RUN_TEST(test_sse_alloc_sets_slot_id);
    RUN_TEST(test_sse_alloc_stores_path);
    RUN_TEST(test_sse_alloc_stores_different_paths_per_slot);
    RUN_TEST(test_sse_alloc_path_truncated_to_max);
    RUN_TEST(test_sse_alloc_pool_full_returns_null);
    RUN_TEST(test_sse_alloc_sse_id_is_pool_index);

    // Pool: find
    RUN_TEST(test_sse_find_returns_correct_conn);
    RUN_TEST(test_sse_find_returns_null_when_empty);
    RUN_TEST(test_sse_find_returns_null_for_different_slot);
    RUN_TEST(test_sse_find_after_both_slots_allocated);
    RUN_TEST(test_sse_find_checks_slot_id_not_sse_id);

    // Pool: free
    RUN_TEST(test_sse_free_deactivates_slot);
    RUN_TEST(test_sse_free_restores_sse_id);
    RUN_TEST(test_sse_free_makes_slot_findable_as_null);
    RUN_TEST(test_sse_free_clears_path);
    RUN_TEST(test_sse_free_nop_on_unallocated);
    RUN_TEST(test_sse_alloc_after_free_succeeds);
    RUN_TEST(test_sse_free_only_frees_matching_slot);

    // Write
    RUN_TEST(test_sse_write_null_data_returns_false);
    RUN_TEST(test_sse_write_returns_false_when_conn_not_active);
    RUN_TEST(test_sse_write_returns_false_when_pcb_null);
    RUN_TEST(test_sse_write_data_only_returns_true);
    RUN_TEST(test_sse_write_with_event_returns_true);
    RUN_TEST(test_sse_write_with_id_returns_true);
    RUN_TEST(test_sse_write_with_all_fields_returns_true);
    RUN_TEST(test_sse_write_does_not_affect_other_slots);

    // Stress
    RUN_TEST(stress_sse_alloc_free_100_cycles);
    RUN_TEST(stress_sse_alloc_free_both_slots_alternating);
    RUN_TEST(stress_sse_write_100_calls);
    RUN_TEST(stress_sse_find_with_full_pool);
    RUN_TEST(stress_sse_write_slot_isolation);

    return UNITY_END();
}
