// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit and stress tests for Layer 4 (Transport) — constants, pool invariants,
// ring-buffer arithmetic, timeout logic, event-queue behaviour, and
// sustained-load correctness.

#include "network_drivers/transport.h"
#include <unity.h>

// transport.cpp is compiled into the native env — no stubs needed.

void setUp()
{
    set_millis(0);
    DeterministicAsyncTCP::init(80);
}

void tearDown()
{
}

// ====================================================================
// UNIT TESTS
// ====================================================================

// ---- Compile-time constants ----------------------------------------

void test_pool_capacity_is_four()
{
    TEST_ASSERT_EQUAL(4, MAX_CONNS);
}
void test_rx_buffer_size_is_one_kb()
{
    TEST_ASSERT_EQUAL(1024, RX_BUF_SIZE);
}
void test_timeout_constant_is_5000ms()
{
    TEST_ASSERT_EQUAL(5000, CONN_TIMEOUT_MS);
}

// ---- Pool defaults after init() ------------------------------------

void test_all_slots_free_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[i].state);
}

void test_all_pcbs_null_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_NULL(conn_pool[i].pcb);
}

void test_all_ring_buffers_empty_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(conn_pool[i].rx_head, conn_pool[i].rx_tail);
}

void test_slot_ids_match_indices()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(i, conn_pool[i].id);
}

// ---- Ring buffer arithmetic ----------------------------------------

void test_ring_empty_when_head_equals_tail()
{
    TcpConn s = {};
    TEST_ASSERT_EQUAL(s.rx_head, s.rx_tail);
}

void test_ring_wrap_at_boundary()
{
    size_t next = (size_t)(RX_BUF_SIZE - 1 + 1) % RX_BUF_SIZE;
    TEST_ASSERT_EQUAL(0, (int)next);
}

void test_ring_full_sentinel_one_slot_reserved()
{
    size_t tail = 0;
    size_t head = RX_BUF_SIZE - 1;
    TEST_ASSERT_EQUAL(tail, (head + 1) % RX_BUF_SIZE);
}

void test_ring_can_store_size_minus_one_bytes()
{
    TcpConn s = {};
    s.rx_head = 0;
    s.rx_tail = 0;
    size_t count = 0;
    while (true)
    {
        size_t next = (s.rx_head + 1) % RX_BUF_SIZE;
        if (next == s.rx_tail)
            break;
        s.rx_buffer[s.rx_head] = (uint8_t)count;
        s.rx_head = next;
        count++;
    }
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, (int)count);
}

// ---- Event types ---------------------------------------------------

void test_event_types_are_distinct()
{
    TEST_ASSERT_NOT_EQUAL((int)EVT_CONNECT, (int)EVT_DATA);
    TEST_ASSERT_NOT_EQUAL((int)EVT_DATA, (int)EVT_DISCONNECT);
    TEST_ASSERT_NOT_EQUAL((int)EVT_DISCONNECT, (int)EVT_ERROR);
    TEST_ASSERT_NOT_EQUAL((int)EVT_CONNECT, (int)EVT_ERROR);
}

// ---- Timeout logic -------------------------------------------------

void test_timeout_does_not_fire_on_free_slot()
{
    conn_pool[0].state = CONN_FREE;
    set_millis(CONN_TIMEOUT_MS + 1);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state);
}

void test_timeout_does_not_fire_before_deadline()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS - 1);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(CONN_ACTIVE, conn_pool[0].state);
}

void test_timeout_fires_at_deadline()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state);
    TEST_ASSERT_NULL(conn_pool[0].pcb);
}

void test_timeout_fires_only_on_stale_slots()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;

    conn_pool[1].state = CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = CONN_TIMEOUT_MS; // fresh

    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();

    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state);
    TEST_ASSERT_EQUAL(CONN_ACTIVE, conn_pool[1].state);
}

void test_init_succeeds_on_native()
{
    bool ok = DeterministicAsyncTCP::init(80);
    TEST_ASSERT_TRUE(ok);
}

// Regression: init() must zero last_activity_ms (was left uninitialised
// before the conn_pool[i]={} fix, causing cross-test timeout spurious fires).
void test_all_last_activity_ms_zero_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(0, (int)conn_pool[i].last_activity_ms);
}

void test_queue_not_null_after_init()
{
    TEST_ASSERT_NOT_NULL(DeterministicAsyncTCP::queue);
}

// ====================================================================
// STRESS TESTS
// ====================================================================

// Fill the ring buffer to max capacity with a known pattern,
// then drain and verify every byte — no corruption under full load.
void stress_ring_buffer_fill_drain_integrity()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;
    const int FILL = RX_BUF_SIZE - 1; // sentinel leaves one slot empty

    // Write known pattern
    for (int i = 0; i < FILL; i++)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        s->rx_buffer[s->rx_head] = (uint8_t)(i & 0xFF);
        s->rx_head = next;
    }

    // Buffer must report full (next_head == tail)
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, (int)((s->rx_head - s->rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE));

    // Drain and verify every byte
    for (int i = 0; i < FILL; i++)
    {
        uint8_t expected = (uint8_t)(i & 0xFF);
        uint8_t actual = s->rx_buffer[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        TEST_ASSERT_EQUAL_MESSAGE(expected, actual, "ring buffer byte mismatch");
    }

    // Must be empty
    TEST_ASSERT_EQUAL(s->rx_head, s->rx_tail);
}

// Half-fill, half-drain, repeat — forces pointer wrap-around multiple
// times and proves the circular invariant holds under sustained partial load.
void stress_ring_buffer_multi_cycle_no_corruption()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;

    uint8_t write_val = 0;
    uint8_t read_val = 0;

    for (int cycle = 0; cycle < 8; cycle++)
    {
        const int BATCH = RX_BUF_SIZE / 2; // 512 bytes per batch

        for (int i = 0; i < BATCH; i++)
        {
            size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
            TEST_ASSERT_NOT_EQUAL_MESSAGE(next, s->rx_tail, "ring full during stress write");
            s->rx_buffer[s->rx_head] = write_val++;
            s->rx_head = next;
        }

        while (s->rx_tail != s->rx_head)
        {
            TEST_ASSERT_EQUAL_MESSAGE(read_val, s->rx_buffer[s->rx_tail], "ring corrupt on drain");
            read_val++;
            s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        }
    }

    TEST_ASSERT_EQUAL(s->rx_head, s->rx_tail); // empty after all cycles
}

// All four connection slots timeout simultaneously — every slot must be
// freed and none must corrupt a neighbour.
void stress_all_slots_timeout_simultaneously()
{
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].pcb = nullptr;
        conn_pool[i].last_activity_ms = 0;
    }

    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();

    for (int i = 0; i < MAX_CONNS; i++)
    {
        TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[i].state);
        TEST_ASSERT_NULL(conn_pool[i].pcb);
        TEST_ASSERT_EQUAL(i, conn_pool[i].id); // id must not be smashed
    }
}

// Arms all slots, times them out, re-arms, times out again — 5 cycles.
// Verifies the pool recovers cleanly and check_timeouts is idempotent.
void stress_timeout_arm_recover_cycle()
{
    for (int cycle = 0; cycle < 5; cycle++)
    {
        for (int i = 0; i < MAX_CONNS; i++)
        {
            conn_pool[i].state = CONN_ACTIVE;
            conn_pool[i].pcb = nullptr;
            conn_pool[i].last_activity_ms = 0;
        }

        set_millis((uint32_t)(CONN_TIMEOUT_MS * (cycle + 1)));
        DeterministicAsyncTCP::check_timeouts();

        for (int i = 0; i < MAX_CONNS; i++)
            TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[i].state);
    }
}

// Runs check_timeouts() 2000 times against a mix of free, active-fresh,
// and active-stale slots — verifies no crash and final state is correct.
void stress_check_timeouts_high_call_rate()
{
    conn_pool[0].state = CONN_FREE;
    conn_pool[1].state = CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = 0;
    conn_pool[2].state = CONN_ACTIVE;
    conn_pool[2].pcb = nullptr;
    conn_pool[2].last_activity_ms = CONN_TIMEOUT_MS; // diff = (now - TIMEOUT_MS) = 0 < TIMEOUT_MS
    conn_pool[3].state = CONN_FREE;

    set_millis(CONN_TIMEOUT_MS); // slot 1 will expire, slot 2 won't

    for (int i = 0; i < 2000; i++)
        DeterministicAsyncTCP::check_timeouts();

    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state);
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[1].state);   // expired
    TEST_ASSERT_EQUAL(CONN_ACTIVE, conn_pool[2].state); // still fresh
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[3].state);
}

// Fills ring buffer with ascending bytes one byte at a time, checking
// capacity after each write, then drains one byte at a time and verifies.
void stress_ring_buffer_byte_by_byte_fill_and_drain()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;

    int written = 0;
    while (true)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        if (next == s->rx_tail)
            break; // full
        s->rx_buffer[s->rx_head] = (uint8_t)(written & 0xFF);
        s->rx_head = next;
        written++;
    }
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, written);

    int read = 0;
    while (s->rx_tail != s->rx_head)
    {
        TEST_ASSERT_EQUAL((uint8_t)(read & 0xFF), s->rx_buffer[s->rx_tail]);
        s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        read++;
    }
    TEST_ASSERT_EQUAL(written, read);
}

int main()
{
    UNITY_BEGIN();

    // Unit tests
    RUN_TEST(test_pool_capacity_is_four);
    RUN_TEST(test_rx_buffer_size_is_one_kb);
    RUN_TEST(test_timeout_constant_is_5000ms);
    RUN_TEST(test_all_slots_free_after_init);
    RUN_TEST(test_all_pcbs_null_after_init);
    RUN_TEST(test_all_ring_buffers_empty_after_init);
    RUN_TEST(test_slot_ids_match_indices);
    RUN_TEST(test_ring_empty_when_head_equals_tail);
    RUN_TEST(test_ring_wrap_at_boundary);
    RUN_TEST(test_ring_full_sentinel_one_slot_reserved);
    RUN_TEST(test_ring_can_store_size_minus_one_bytes);
    RUN_TEST(test_event_types_are_distinct);
    RUN_TEST(test_timeout_does_not_fire_on_free_slot);
    RUN_TEST(test_timeout_does_not_fire_before_deadline);
    RUN_TEST(test_timeout_fires_at_deadline);
    RUN_TEST(test_timeout_fires_only_on_stale_slots);
    RUN_TEST(test_init_succeeds_on_native);
    RUN_TEST(test_all_last_activity_ms_zero_after_init);
    RUN_TEST(test_queue_not_null_after_init);

    // Stress tests
    RUN_TEST(stress_ring_buffer_fill_drain_integrity);
    RUN_TEST(stress_ring_buffer_multi_cycle_no_corruption);
    RUN_TEST(stress_all_slots_timeout_simultaneously);
    RUN_TEST(stress_timeout_arm_recover_cycle);
    RUN_TEST(stress_check_timeouts_high_call_rate);
    RUN_TEST(stress_ring_buffer_byte_by_byte_fill_and_drain);

    return UNITY_END();
}
