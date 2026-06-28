// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Transport observability (DETWS_ENABLE_OBSERVABILITY): the det_conn_on_event
// hook, the by-reason counters, the live CONN_CLOSING gauge, and that the real
// lwIP callbacks (recv FIN / error / timeout / local close / backpressure) drive
// the right counter and fire the hook.

#include "network_drivers/transport/listener.h"
#include "network_drivers/transport/transport.h"
#include <string.h>
#include <unity.h>

// Last event the hook saw.
static int g_calls;
static uint8_t g_slot;
static ConnState g_old, g_new;
static DetConnReason g_reason;

static void on_event(uint8_t slot, ConnState olds, ConnState news, DetConnReason reason)
{
    g_calls++;
    g_slot = slot;
    g_old = olds;
    g_new = news;
    g_reason = reason;
}

void setUp()
{
    set_millis(0);
    DeterministicAsyncTCP::pool_init();
    listener_add(0, 80, PROTO_HTTP);
    det_conn_on_event(on_event);
    det_conn_counters_reset();
    g_calls = 0;
}

void tearDown()
{
    det_conn_on_event(nullptr);
}

// ---- the notify machinery (drives every reason directly) -------------------

void test_transition_fires_hook_with_args()
{
    detws_obs_transition(2, CONN_FREE, CONN_ACTIVE, DET_CONN_R_ACCEPT);
    TEST_ASSERT_EQUAL(1, g_calls);
    TEST_ASSERT_EQUAL(2, g_slot);
    TEST_ASSERT_EQUAL(CONN_FREE, g_old);
    TEST_ASSERT_EQUAL(CONN_ACTIVE, g_new);
    TEST_ASSERT_EQUAL(DET_CONN_R_ACCEPT, g_reason);
}

void test_each_reason_bumps_its_counter()
{
    detws_obs_transition(0, CONN_FREE, CONN_ACTIVE, DET_CONN_R_ACCEPT);
    detws_obs_transition(0, CONN_ACTIVE, CONN_FREE, DET_CONN_R_CLOSE_REMOTE);
    detws_obs_transition(0, CONN_ACTIVE, CONN_FREE, DET_CONN_R_CLOSE_LOCAL);
    detws_obs_transition(0, CONN_ACTIVE, CONN_FREE, DET_CONN_R_ERROR);
    detws_obs_transition(0, CONN_ACTIVE, CONN_FREE, DET_CONN_R_TIMEOUT);
    detws_obs_transition(0, CONN_ACTIVE, CONN_FREE, DET_CONN_R_ABORT);
    detws_obs_notice(0, CONN_ACTIVE, DET_CONN_R_BACKPRESSURE);
    detws_obs_notice(0, CONN_ACTIVE, DET_CONN_R_DEFER_DROP);

    DetConnCounters c = det_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(1, c.accepts);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_remote);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_error);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_timeout);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_abort);
    TEST_ASSERT_EQUAL_UINT32(1, c.backpressure);
    TEST_ASSERT_EQUAL_UINT32(1, c.defer_drops);
}

void test_closing_gauge_tracks_dwell()
{
    DetConnCounters c = det_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.closing_gauge);

    detws_obs_transition(1, CONN_ACTIVE, CONN_CLOSING, DET_CONN_R_CLOSE_LOCAL); // enter
    c = det_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(1, c.closing_gauge);

    detws_obs_transition(1, CONN_CLOSING, CONN_FREE, DET_CONN_R_DRAINED); // leave
    c = det_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.closing_gauge);
    // DRAINED is gauge-only: it must not inflate any cumulative close counter.
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(0, c.closes_remote);
}

void test_reset_clears_cumulative_not_gauge()
{
    detws_obs_transition(0, CONN_FREE, CONN_ACTIVE, DET_CONN_R_ACCEPT);
    detws_obs_transition(0, CONN_ACTIVE, CONN_CLOSING, DET_CONN_R_CLOSE_LOCAL);
    det_conn_counters_reset();
    DetConnCounters c = det_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.accepts);
    TEST_ASSERT_EQUAL_UINT32(0, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(1, c.closing_gauge); // live gauge survives a reset
}

void test_no_hook_after_unregister()
{
    det_conn_on_event(nullptr);
    detws_obs_transition(0, CONN_FREE, CONN_ACTIVE, DET_CONN_R_ACCEPT);
    TEST_ASSERT_EQUAL(0, g_calls);                            // hook silent
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().accepts); // counters still move
}

// ---- integration: the real transport callbacks ----------------------------

void test_recv_fin_counts_remote_close()
{
    struct tcp_pcb pcb;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    lowlevel_recv_cb(&conn_pool[0], &pcb, nullptr, ERR_OK); // null pbuf = FIN
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().closes_remote);
    TEST_ASSERT_EQUAL(DET_CONN_R_CLOSE_REMOTE, g_reason);
}

void test_err_cb_counts_error_close()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    lowlevel_err_cb(&conn_pool[0], ERR_ABRT);
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().closes_error);
    TEST_ASSERT_EQUAL(DET_CONN_R_ERROR, g_reason);
}

void test_timeout_sweep_counts_timeout()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].owner = 0;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS + 1);
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state);
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().closes_timeout);
    TEST_ASSERT_EQUAL(DET_CONN_R_TIMEOUT, g_reason);
}

void test_local_close_counts_local()
{
    struct tcp_pcb pcb;
    det_conn_close(0, &pcb);
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().closes_local);
    TEST_ASSERT_EQUAL(DET_CONN_R_CLOSE_LOCAL, g_reason);
}

void test_backpressure_counts_when_ring_full()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].rx_head = 0;
    conn_pool[0].rx_tail = 0;
    struct pbuf p;
    memset(&p, 0, sizeof(p));
    p.tot_len = RX_BUF_SIZE * 2; // larger than the whole ring -> refused
    err_t rc = lowlevel_recv_cb(&conn_pool[0], nullptr, &p, ERR_OK);
    TEST_ASSERT_EQUAL(ERR_MEM, rc);
    TEST_ASSERT_EQUAL_UINT32(1, det_conn_counters().backpressure);
    TEST_ASSERT_EQUAL(DET_CONN_R_BACKPRESSURE, g_reason);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_transition_fires_hook_with_args);
    RUN_TEST(test_each_reason_bumps_its_counter);
    RUN_TEST(test_closing_gauge_tracks_dwell);
    RUN_TEST(test_reset_clears_cumulative_not_gauge);
    RUN_TEST(test_no_hook_after_unregister);
    RUN_TEST(test_recv_fin_counts_remote_close);
    RUN_TEST(test_err_cb_counts_error_close);
    RUN_TEST(test_timeout_sweep_counts_timeout);
    RUN_TEST(test_local_close_counts_local);
    RUN_TEST(test_backpressure_counts_when_ring_full);
    return UNITY_END();
}
