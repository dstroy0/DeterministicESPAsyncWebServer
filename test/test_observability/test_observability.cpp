// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Transport observability (DWS_ENABLE_OBSERVABILITY): the dws_conn_on_event
// hook, the by-reason counters, the live ConnState::CONN_CLOSING gauge, and that the real
// lwIP callbacks (recv FIN / error / timeout / local close / backpressure) drive
// the right counter and fire the hook.

#include "network_drivers/transport/listener.h"
#include "network_drivers/transport/tcp.h"
#include <string.h>
#include <unity.h>

// Last event the hook saw.
static int g_calls;
static uint8_t g_slot;
static ConnState g_old, g_new;
static DWSConnReason g_reason;

static void on_event(uint8_t slot, ConnState olds, ConnState news, DWSConnReason reason)
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
    listener_add(0, 80, ConnProto::PROTO_HTTP);
    dws_conn_on_event(on_event);
    dws_conn_counters_reset();
    g_calls = 0;
}

void tearDown()
{
    dws_conn_on_event(nullptr);
}

// ---- the notify machinery (drives every reason directly) -------------------

void test_transition_fires_hook_with_args()
{
    dws_obs_transition(2, ConnState::CONN_FREE, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_ACCEPT);
    TEST_ASSERT_EQUAL(1, g_calls);
    TEST_ASSERT_EQUAL(2, g_slot);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, g_old);
    TEST_ASSERT_EQUAL(ConnState::CONN_ACTIVE, g_new);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_ACCEPT, g_reason);
}

void test_each_reason_bumps_its_counter()
{
    dws_obs_transition(0, ConnState::CONN_FREE, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_ACCEPT);
    dws_obs_transition(0, ConnState::CONN_ACTIVE, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_CLOSE_REMOTE);
    dws_obs_transition(0, ConnState::CONN_ACTIVE, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_CLOSE_LOCAL);
    dws_obs_transition(0, ConnState::CONN_ACTIVE, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_ERROR);
    dws_obs_transition(0, ConnState::CONN_ACTIVE, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_TIMEOUT);
    dws_obs_transition(0, ConnState::CONN_ACTIVE, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_ABORT);
    dws_obs_notice(0, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_BACKPRESSURE);
    dws_obs_notice(0, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_DEFER_DROP);

    DWSConnCounters c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(1, c.accepts);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_remote);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_error);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_timeout);
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_abort);
    TEST_ASSERT_EQUAL_UINT32(1, c.backpressure);
    TEST_ASSERT_EQUAL_UINT32(1, c.defer_drops);
}

void test_closing_gauge_is_derived_from_pool()
{
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closing_gauge);

    conn_pool[1].state = ConnState::CONN_CLOSING; // a slot actually dwelling
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closing_gauge);
    conn_pool[2].state = ConnState::CONN_CLOSING;
    TEST_ASSERT_EQUAL_UINT32(2, dws_conn_counters().closing_gauge);

    conn_pool[1].state = ConnState::CONN_FREE;
    conn_pool[2].state = ConnState::CONN_FREE;
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closing_gauge);

    // DRAINED is gauge-only: it must not inflate any cumulative close counter.
    dws_obs_transition(1, ConnState::CONN_CLOSING, ConnState::CONN_FREE, DWSConnReason::DWS_CONN_R_DRAINED);
    DWSConnCounters c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(0, c.closes_remote);
}

void test_reset_clears_cumulative_not_derived_gauge()
{
    dws_obs_transition(0, ConnState::CONN_FREE, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_ACCEPT);
    conn_pool[0].state = ConnState::CONN_CLOSING; // a slot is genuinely closing
    dws_conn_counters_reset();
    DWSConnCounters c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.accepts);       // cumulative cleared
    TEST_ASSERT_EQUAL_UINT32(1, c.closing_gauge); // derived from the pool, not by reset
}

void test_no_hook_after_unregister()
{
    dws_conn_on_event(nullptr);
    dws_obs_transition(0, ConnState::CONN_FREE, ConnState::CONN_ACTIVE, DWSConnReason::DWS_CONN_R_ACCEPT);
    TEST_ASSERT_EQUAL(0, g_calls);                            // hook silent
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().accepts); // counters still move
}

// ---- integration: the real transport callbacks ----------------------------

void test_recv_fin_counts_remote_close()
{
    struct tcp_pcb pcb;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    lowlevel_recv_cb(&conn_pool[0], &pcb, nullptr, ERR_OK); // null pbuf = FIN
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closes_remote);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_CLOSE_REMOTE, g_reason);
}

void test_err_cb_counts_error_close()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    lowlevel_err_cb(&conn_pool[0], ERR_ABRT);
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closes_error);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_ERROR, g_reason);
}

void test_timeout_sweep_counts_timeout()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].owner = 0;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS + 1);
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closes_timeout);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_TIMEOUT, g_reason);
}

void test_local_close_counts_local()
{
    // dws_conn_close(slot) reads the slot's pcb, frees the slot, and counts a
    // local close. The transport owns the teardown: the slot ends FREE/null.
    struct tcp_pcb pcb;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    dws_conn_close(0);
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closes_local);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_CLOSE_LOCAL, g_reason);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_NULL(conn_pool[0].pcb);
}

// dws_conn_abort_slot(slot) owns the hard-RST teardown: it frees the slot and
// counts an abort. A no-op (no count, no hook) when the slot has no live pcb.
void test_abort_slot_counts_abort_and_frees()
{
    struct tcp_pcb pcb;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    dws_conn_abort_slot(0);
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().closes_abort);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_ABORT, g_reason);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_NULL(conn_pool[0].pcb);
}

void test_abort_slot_noop_on_free_slot()
{
    conn_pool[0].state = ConnState::CONN_FREE;
    conn_pool[0].pcb = nullptr;
    dws_conn_abort_slot(0);
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closes_abort);
    TEST_ASSERT_EQUAL(0, g_calls);
}

void test_backpressure_counts_when_ring_full()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].rx_head = 0;
    conn_pool[0].rx_tail = 0;
    struct pbuf p;
    memset(&p, 0, sizeof(p));
    p.tot_len = RX_BUF_SIZE * 2; // larger than the whole ring -> refused
    err_t rc = lowlevel_recv_cb(&conn_pool[0], nullptr, &p, ERR_OK);
    TEST_ASSERT_EQUAL(ERR_MEM, rc);
    TEST_ASSERT_EQUAL_UINT32(1, dws_conn_counters().backpressure);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_BACKPRESSURE, g_reason);
}

// ---- ConnState::CONN_CLOSING real dwell (part 2) -------------------------------------

void test_begin_close_dwells_then_drains_on_ack()
{
    struct tcp_pcb pcb;
    pcb.snd_queuelen = 1; // response still in flight -> must dwell
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;

    dws_conn_begin_close(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_CLOSING, (ConnState)conn_pool[0].state); // dwelling
    DWSConnCounters c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(1, c.closing_gauge);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_CLOSE_LOCAL, g_reason);

    // Peer ACKs the whole response -> the sent callback finalizes the close.
    pcb.snd_queuelen = 0;
    lowlevel_sent_cb(&conn_pool[0], &pcb, 100);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(0, c.closing_gauge);
    TEST_ASSERT_EQUAL(DWSConnReason::DWS_CONN_R_DRAINED, g_reason);
}

void test_begin_close_finalizes_immediately_when_already_drained()
{
    struct tcp_pcb pcb;
    pcb.snd_queuelen = 0; // nothing pending -> close now, no dwell
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;

    dws_conn_begin_close(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    DWSConnCounters c = dws_conn_counters();
    TEST_ASSERT_EQUAL_UINT32(1, c.closes_local);
    TEST_ASSERT_EQUAL_UINT32(0, c.closing_gauge);
}

void test_begin_close_noop_if_not_active()
{
    conn_pool[0].state = ConnState::CONN_FREE;
    dws_conn_begin_close(0);
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closes_local);
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closing_gauge);
}

void test_closing_timeout_reaps_stuck_slot()
{
    struct tcp_pcb pcb;
    pcb.snd_queuelen = 1; // peer never ACKs -> would dwell forever
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    conn_pool[0].owner = 0;
    set_millis(1000);

    dws_conn_begin_close(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_CLOSING, (ConnState)conn_pool[0].state);

    // Before the bound: not reaped.
    set_millis(1000 + DWS_CLOSING_TIMEOUT_MS - 1);
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_CLOSING, (ConnState)conn_pool[0].state);

    // Past the bound: the sweep force-frees it (no pool leak).
    set_millis(1000 + DWS_CLOSING_TIMEOUT_MS + 1);
    DeterministicAsyncTCP::check_timeouts(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL_UINT32(0, dws_conn_counters().closing_gauge);
}

void test_recv_during_closing_is_drained_not_processed()
{
    struct tcp_pcb pcb;
    pcb.snd_queuelen = 1;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = &pcb;
    dws_conn_begin_close(0);
    TEST_ASSERT_EQUAL(ConnState::CONN_CLOSING, (ConnState)conn_pool[0].state);

    // Late inbound data while closing: acked + dropped, slot stays CLOSING.
    struct pbuf p;
    memset(&p, 0, sizeof(p));
    p.tot_len = 8;
    err_t rc = lowlevel_recv_cb(&conn_pool[0], &pcb, &p, ERR_OK);
    TEST_ASSERT_EQUAL(ERR_OK, rc);
    TEST_ASSERT_EQUAL(ConnState::CONN_CLOSING, (ConnState)conn_pool[0].state);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_transition_fires_hook_with_args);
    RUN_TEST(test_each_reason_bumps_its_counter);
    RUN_TEST(test_closing_gauge_is_derived_from_pool);
    RUN_TEST(test_reset_clears_cumulative_not_derived_gauge);
    RUN_TEST(test_no_hook_after_unregister);
    RUN_TEST(test_recv_fin_counts_remote_close);
    RUN_TEST(test_err_cb_counts_error_close);
    RUN_TEST(test_timeout_sweep_counts_timeout);
    RUN_TEST(test_local_close_counts_local);
    RUN_TEST(test_abort_slot_counts_abort_and_frees);
    RUN_TEST(test_abort_slot_noop_on_free_slot);
    RUN_TEST(test_backpressure_counts_when_ring_full);
    // ConnState::CONN_CLOSING real dwell
    RUN_TEST(test_begin_close_dwells_then_drains_on_ack);
    RUN_TEST(test_begin_close_finalizes_immediately_when_already_drained);
    RUN_TEST(test_begin_close_noop_if_not_active);
    RUN_TEST(test_closing_timeout_reaps_stuck_slot);
    RUN_TEST(test_recv_during_closing_is_drained_not_processed);
    return UNITY_END();
}
