// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file transport.cpp
 * @brief Layer 4 (Transport) — TCP connection management implementation.
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
 */

#include "transport.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"

TcpConn conn_pool[MAX_CONNS];
static struct tcp_pcb *listen_pcb = nullptr;

QueueHandle_t DeterministicAsyncTCP::queue = nullptr;
uint32_t DeterministicAsyncTCP::conn_timeout_ms = CONN_TIMEOUT_MS;

static err_t lowlevel_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void lowlevel_err_cb(void *arg, err_t err);

/**
 * @brief Non-blocking event enqueue helper.
 *
 * xQueueSend with timeout=0 means the call returns immediately if the queue
 * is full.  In practice the queue (depth 16) is large enough for any burst
 * of MAX_CONNS events; a full queue indicates the application is not calling
 * server_tick() fast enough.  Dropped events are recoverable via timeout.
 */
static inline void enqueue(const TcpEvt &evt)
{
    xQueueSend(DeterministicAsyncTCP::queue, &evt, 0);
}

int32_t DeterministicAsyncTCP::init(uint16_t port, const WebServerConfig *cfg)
{
    // Load runtime config (or fall back to compile-time default)
    conn_timeout_ms = cfg ? cfg->conn_timeout_ms : CONN_TIMEOUT_MS;

    // Minimum heap required: 16-slot queue of TcpEvt records
    static const int32_t QUEUE_RAM_NEEDED = (int32_t)(16 * sizeof(TcpEvt));

    queue = xQueueCreate(16, sizeof(TcpEvt));
    if (queue == nullptr)
        return -QUEUE_RAM_NEEDED;

    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {}; // zero all fields, including last_activity_ms
        conn_pool[i].id = i;
        conn_pool[i].state = CONN_FREE;
    }

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb)
        return -1;

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (err != ERR_OK)
    {
        tcp_abort(pcb);
        return -1;
    }

    listen_pcb = tcp_listen_with_backlog(pcb, MAX_CONNS);
    if (!listen_pcb)
    {
        tcp_abort(pcb);
        return -1;
    }

    tcp_arg(listen_pcb, nullptr);
    tcp_accept(listen_pcb, lowlevel_accept_cb);

    return 1;
}

void DeterministicAsyncTCP::stop()
{
    // Close the listener first — no new connections accepted
    if (listen_pcb)
    {
        tcp_close(listen_pcb);
        listen_pcb = nullptr;
    }

    // Abort all active connections
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

    // Free the event queue
    if (queue)
    {
        vQueueDelete(queue);
        queue = nullptr;
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
            tcp_arg(pcb, nullptr); // detach slot pointer from PCB
            tcp_abort(pcb);
        }
        TcpEvt evt = {EVT_ERROR, (uint8_t)i, 0};
        enqueue(evt);
    }
}

// ---------------------------------------------------------------------------
// lwIP callbacks — execute in tcpip_thread task context
// ---------------------------------------------------------------------------

/**
 * @brief lwIP accept callback — fires when a new client connects.
 *
 * Finds a free slot, wires up the per-connection callbacks, and posts
 * EVT_CONNECT.  If the pool is full, rejects the connection with ERR_ABRT
 * (which tells lwIP the PCB is already gone from our side).
 */
static err_t lowlevel_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    if (err != ERR_OK || newpcb == nullptr)
        return ERR_VAL;

    int free_slot = -1;
    for (int i = 0; i < MAX_CONNS; i++)
    {
        if (conn_pool[i].state == CONN_FREE)
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        /*
         * Pool is full — reject connection immediately.  ERR_ABRT signals
         * lwIP that the PCB has already been aborted by this callback.
         */
        tcp_abort(newpcb);
        return ERR_ABRT;
    }

    TcpConn *slot = &conn_pool[free_slot];
    slot->state = CONN_ACTIVE;
    slot->pcb = newpcb;
    slot->last_activity_ms = millis();
    slot->rx_head = 0;
    slot->rx_tail = 0;

    tcp_arg(newpcb, slot);
    tcp_recv(newpcb, lowlevel_recv_cb);
    tcp_sent(newpcb, lowlevel_sent_cb);
    tcp_err(newpcb, lowlevel_err_cb);

    TcpEvt evt = {EVT_CONNECT, (uint8_t)free_slot, 0};
    enqueue(evt);

    return ERR_OK;
}

/**
 * @brief lwIP receive callback — fires when data arrives on a connection.
 *
 * Copies pbuf chain bytes into the ring buffer and calls tcp_recved() with
 * only the bytes actually stored, applying TCP-level backpressure when the
 * buffer is full.  A null pbuf signals graceful remote close (FIN received).
 */
static err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
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
        enqueue(evt);
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
     * the application falls behind — data is never silently dropped.
     */
    tcp_recved(tpcb, (u16_t)bytes_copied);
    pbuf_free(p);

    if (bytes_copied > 0)
    {
        TcpEvt evt = {EVT_DATA, slot->id, bytes_copied};
        enqueue(evt);
    }

    return ERR_OK;
}

/**
 * @brief lwIP sent callback — fires after the stack acknowledges sent bytes.
 *
 * Used only to refresh the idle-timeout timestamp so an active sender doesn't
 * get timed out while its responses are in flight.
 */
static err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    TcpConn *slot = (TcpConn *)arg;
    if (slot)
        slot->last_activity_ms = millis();
    return ERR_OK;
}

/**
 * @brief lwIP error callback — fires when the stack detects a fatal error.
 *
 * By the time this fires the PCB is already gone internally, so we must NOT
 * call tcp_close() or tcp_abort().  Null out the slot's pcb pointer and post
 * EVT_ERROR so the session layer resets the HTTP parser.
 */
static void lowlevel_err_cb(void *arg, err_t err)
{
    TcpConn *slot = (TcpConn *)arg;
    if (!slot)
        return;

    /*
     * When lwIP fires the error callback the PCB has already been freed
     * internally.  We must NOT call tcp_close/tcp_abort here — just null
     * out our pointer to prevent any future access.
     */
    slot->state = CONN_FREE;
    slot->pcb = nullptr;

    TcpEvt evt = {EVT_ERROR, slot->id, 0};
    enqueue(evt);
}
