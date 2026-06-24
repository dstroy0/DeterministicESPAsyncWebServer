// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file transport.cpp
 * @brief Layer 4 (Transport) - TCP connection management implementation.
 *
 * All lwIP raw-API callbacks run in the `tcpip_thread` FreeRTOS task context.
 * They are NOT hardware ISRs, which is why we use `xQueueSend()` (non-ISR
 * variant) with timeout=0 rather than `xQueueSendFromISR()`.
 *
 * **Ring buffer write ordering**
 * The producer (recv callback) writes payload bytes into `rx_buffer[]` and
 * then advances `rx_head`.  The consumer (main loop) reads from `rx_buffer[]`
 * at `rx_tail` and advances `rx_tail`.  Because Xtensa LX7 stores are
 * in-order and `rx_head`/`rx_tail` are `volatile`, no memory barriers are
 * needed beyond the `volatile` annotation.
 *
 * **Listener coupling**
 * Each TcpConn carries `listener_id` (set at accept time by listener_accept_cb
 * in listener.cpp).  The `enqueue()` helper forwards events to
 * `listener_enqueue()` - defined in listener.cpp - so transport.cpp needs no
 * direct knowledge of the Listener struct.  `listener_enqueue()` is forward-
 * declared in transport.h to avoid a circular include.
 */

#include "transport.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"

TcpConn conn_pool[MAX_CONNS];

uint32_t detws_ap_ip = 0;

uint32_t DeterministicAsyncTCP::conn_timeout_ms = CONN_TIMEOUT_MS;

/**
 * @brief Non-blocking event enqueue helper.
 *
 * Forwards the event to the queue owned by the connection's listener.
 * xQueueSend with timeout=0 returns immediately if the queue is full.
 * A full queue indicates the application is not calling server_tick() fast
 * enough; dropped events are recoverable via the idle-timeout sweep.
 */
static inline void enqueue(TcpConn *slot, const TcpEvt &evt)
{
    listener_enqueue(slot->listener_id, &evt);
}

size_t DeterministicAsyncTCP::heap_needed()
{
    return 0; // all queues are statically allocated in BSS (inside Listener structs)
}

bool DeterministicAsyncTCP::heap_available()
{
    return true; // no heap allocation; always safe to call begin()
}

void DeterministicAsyncTCP::pool_init(const WebServerConfig *cfg)
{
    conn_timeout_ms = cfg ? cfg->conn_timeout_ms : CONN_TIMEOUT_MS;
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {}; // zero all fields
        conn_pool[i].id = i;
        conn_pool[i].state = CONN_FREE;
    }
}

void DeterministicAsyncTCP::stop()
{
    // Abort all active connections - listener PCBs and queues are owned by
    // the listener layer and must be cleaned up via listener_stop_all() first.
    for (int i = 0; i < MAX_CONNS; i++)
    {
        if (conn_pool[i].state == CONN_ACTIVE && conn_pool[i].pcb)
        {
            struct tcp_pcb *pcb = conn_pool[i].pcb;
            conn_pool[i].state = CONN_FREE;
            conn_pool[i].pcb = nullptr;
            tcp_arg(pcb, nullptr);
            tcp_abort(pcb);
        }
        conn_pool[i].state = CONN_FREE;
        conn_pool[i].pcb = nullptr;
    }
}

void DeterministicAsyncTCP::check_timeouts()
{
    uint32_t now = millis();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        TcpConn *slot = &conn_pool[i];
        if (slot->state != CONN_ACTIVE)
            continue;
        if ((now - slot->last_activity_ms) < conn_timeout_ms)
            continue;

        struct tcp_pcb *pcb = slot->pcb;
        /*
         * Clear state BEFORE calling tcp_abort so that any lwIP callback
         * firing on the same PCB during or after abort sees state==CONN_FREE
         * and exits immediately without accessing freed memory.
         */
        slot->state = CONN_FREE;
        slot->pcb = nullptr;
        if (pcb)
        {
            tcp_arg(pcb, nullptr);
            tcp_abort(pcb);
        }
        TcpEvt evt = {EVT_ERROR, (uint8_t)i, 0};
        enqueue(slot, evt);
    }
}

// ---------------------------------------------------------------------------
// lwIP callbacks - execute in tcpip_thread task context
// These are non-static so listener.cpp can take their address.
// ---------------------------------------------------------------------------

/**
 * @brief lwIP receive callback - fires when data arrives on a connection.
 *
 * Copies pbuf chain bytes into the ring buffer and calls tcp_recved() with
 * only the bytes actually stored, applying TCP-level backpressure when the
 * buffer is full.  A null pbuf signals graceful remote close (FIN received).
 */
err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TcpConn *slot = (TcpConn *)arg;
    if (!slot || slot->state != CONN_ACTIVE)
        return ERR_VAL;

    if (p == nullptr)
    {
        /*
         * Null pbuf signals graceful remote close (FIN received).
         * Clear state and pcb before tcp_close so any stale callbacks
         * are harmless.
         */
        slot->state = CONN_FREE;
        slot->pcb = nullptr;
        tcp_arg(tpcb, nullptr);
        if (tcp_close(tpcb) != ERR_OK)
            tcp_abort(tpcb);
        TcpEvt evt = {EVT_DISCONNECT, slot->id, 0};
        enqueue(slot, evt);
        return ERR_OK;
    }

    slot->last_activity_ms = millis();

    size_t bytes_copied = 0;
    bool full = false;
    struct pbuf *q = p;
    while (q != nullptr && !full)
    {
        uint8_t *payload = (uint8_t *)q->payload;
        for (u16_t i = 0; i < q->len; i++)
        {
            size_t next_head = (slot->rx_head + 1) % RX_BUF_SIZE;
            if (next_head == slot->rx_tail)
            {
                full = true;
                break;
            }
            slot->rx_buffer[slot->rx_head] = payload[i];
            slot->rx_head = next_head;
            bytes_copied++;
        }
        q = q->next;
    }

    /*
     * Acknowledge only bytes actually placed in the ring buffer.
     * This shrinks the TCP receive window and applies backpressure if
     * the application falls behind - data is never silently dropped.
     */
    tcp_recved(tpcb, (u16_t)bytes_copied);
    pbuf_free(p);

    if (bytes_copied > 0)
    {
        TcpEvt evt = {EVT_DATA, slot->id, bytes_copied};
        enqueue(slot, evt);
    }

    return ERR_OK;
}

/**
 * @brief lwIP sent callback - fires after the stack acknowledges sent bytes.
 *
 * Used only to refresh the idle-timeout timestamp so an active sender doesn't
 * get timed out while its responses are in flight.
 */
err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    TcpConn *slot = (TcpConn *)arg;
    if (slot)
        slot->last_activity_ms = millis();
    (void)tpcb;
    (void)len;
    return ERR_OK;
}

/**
 * @brief lwIP error callback - fires when the stack detects a fatal error.
 *
 * By the time this fires the PCB is already gone internally, so we must NOT
 * call tcp_close() or tcp_abort().  Null out the slot's pcb pointer and post
 * EVT_ERROR so the session layer resets the protocol state.
 */
void lowlevel_err_cb(void *arg, err_t err)
{
    TcpConn *slot = (TcpConn *)arg;
    if (!slot)
        return;

    /*
     * When lwIP fires the error callback the PCB has already been freed
     * internally.  We must NOT call tcp_close/tcp_abort here - just null
     * out our pointer to prevent any future access.
     */
    slot->state = CONN_FREE;
    slot->pcb = nullptr;

    TcpEvt evt = {EVT_ERROR, slot->id, 0};
    enqueue(slot, evt);
    (void)err;
}
