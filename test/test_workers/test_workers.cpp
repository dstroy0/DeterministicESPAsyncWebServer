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
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr; // no pcb -> sweep frees the slot without a tcp_abort
    conn_pool[0].last_activity_ms = 0;

    conn_pool[1].owner = 1;
    conn_pool[1].state = CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = 0;

    // Worker 0 sweeps: only its own slot is reaped; worker 1's slot is untouched.
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL_INT(CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL_INT(CONN_ACTIVE, (ConnState)conn_pool[1].state);

    // Worker 1 sweeps: now its own slot is reaped.
    DeterministicAsyncTCP::check_timeouts(1);
    TEST_ASSERT_EQUAL_INT(CONN_FREE, (ConnState)conn_pool[1].state);
}

void test_pool_init_defaults_owner_zero()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL_UINT8(0, conn_pool[i].owner);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_worker_count_is_two);
    RUN_TEST(test_check_timeouts_reaps_only_owned_slots);
    RUN_TEST(test_pool_init_defaults_owner_zero);
    return UNITY_END();
}
