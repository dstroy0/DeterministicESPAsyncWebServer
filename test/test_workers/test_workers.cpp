// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Phase 2 core-partitioning invariant (built with DETWS_WORKER_COUNT=2): a worker
// only ever touches the connection slots it owns. The idle-timeout sweep is the
// one place a worker writes slot state from its own task, so it must reap ONLY its
// owned slots - otherwise two workers could write the same slot. The per-worker
// event-queue ROUTING is verified on hardware (the host FreeRTOS queue is a stub
// that ignores the queue handle).

#include "network_drivers/session/worker.h"
#include "network_drivers/transport/tcp.h"
#include <Arduino.h> // set_millis
#include <unity.h>

void setUp()
{
    DeterministicAsyncTCP::pool_init();
}
void tearDown()
{
}

void test_worker_count_is_two()
{
    TEST_ASSERT_EQUAL_INT(2, detws_worker_count());
}

void test_check_timeouts_reaps_only_owned_slots()
{
    set_millis(100000); // far past CONN_TIMEOUT_MS so both slots are stale

    conn_pool[0].owner = 0;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr; // no pcb -> sweep frees the slot without a tcp_abort
    conn_pool[0].last_activity_ms = 0;

    conn_pool[1].owner = 1;
    conn_pool[1].state = ConnState::CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = 0;

    // Worker 0 sweeps: only its own slot is reaped; worker 1's slot is untouched.
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL_INT(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL_INT(ConnState::CONN_ACTIVE, (ConnState)conn_pool[1].state);

    // Worker 1 sweeps: now its own slot is reaped.
    DeterministicAsyncTCP::check_timeouts(1);
    TEST_ASSERT_EQUAL_INT(ConnState::CONN_FREE, (ConnState)conn_pool[1].state);
}

void test_pool_init_defaults_owner_zero()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL_UINT8(0, conn_pool[i].owner);
}

void test_worker_self_id_roundtrip()
{
    // detws_worker_set_self binds the calling context's worker id; detws_worker_self reads it back.
    detws_worker_set_self(1);
    TEST_ASSERT_EQUAL_INT(1, detws_worker_self());
    detws_worker_set_self(0);
    TEST_ASSERT_EQUAL_INT(0, detws_worker_self());
}

void test_host_worker_lifecycle_is_noops()
{
    // On host there is no worker task: start/stop/wake are no-ops and running() stays false.
    detws_workers_start(nullptr);
    TEST_ASSERT_FALSE(detws_workers_running());
    detws_worker_wake(0);
    detws_workers_stop();
    TEST_ASSERT_FALSE(detws_workers_running());
}

static void set_flag_to_42(void *arg)
{
    *(int *)arg = 42;
}
void test_host_defer_runs_inline_and_rejects_null()
{
    // On host the caller and pipeline are the same thread, so detws_defer runs the callback inline
    // immediately; a null callback is rejected.
    TEST_ASSERT_FALSE(detws_defer(0, nullptr, nullptr));
    int flag = 0;
    TEST_ASSERT_TRUE(detws_defer(0, set_flag_to_42, &flag));
    TEST_ASSERT_EQUAL_INT(42, flag);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_worker_count_is_two);
    RUN_TEST(test_check_timeouts_reaps_only_owned_slots);
    RUN_TEST(test_pool_init_defaults_owner_zero);
    RUN_TEST(test_worker_self_id_roundtrip);
    RUN_TEST(test_host_worker_lifecycle_is_noops);
    RUN_TEST(test_host_defer_runs_inline_and_rejects_null);
    return UNITY_END();
}
