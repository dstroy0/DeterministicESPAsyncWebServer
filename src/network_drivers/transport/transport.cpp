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
#include "services/det_clock.h" // detws_millis() pluggable monotonic clock

#if DETWS_ENABLE_TLS
#include "network_drivers/tls/det_tls.h"
#endif

// ---------------------------------------------------------------------------
// Cross-thread TCP serialization
// ---------------------------------------------------------------------------
// The raw lwIP API is not thread-safe: its callbacks run in the tcpip_thread
// task, while this library issues writes/closes from the Arduino main-loop task.
// Calling tcp_*() from the main loop concurrently with tcpip_thread processing
// an inbound segment corrupts the PCB state - under a streaming upload (the peer
// is actively sending as the server responds/closes) it trips lwIP's
// "tcp_receive: wrong state" assert and panics.
//
// Arduino-esp32 ships lwIP with LWIP_TCPIP_CORE_LOCKING disabled, so
// LOCK_TCPIP_CORE() is a no-op. The portable fix is tcpip_api_call(): it runs a
// function *inside* tcpip_thread and blocks the caller until it completes, so
// every main-loop-originated tcp_*() executes in the one safe context. lwIP's
// own callbacks already run in that context and must NOT marshal again (they
// call tcp_*() directly).
#if defined(ARDUINO)
#include "lwip/priv/tcpip_priv.h"
#include <string.h>

enum DetTcpOp
{
    DET_OP_SEND,
    DET_OP_OUTPUT,
    DET_OP_CLOSE,
    DET_OP_ABORT,
    DET_OP_DETACH,
    DET_OP_RAWSEND // raw tcp_write of already-encrypted bytes (TLS BIO), no TLS re-entry
};

// True while det_tcp_do() is executing, i.e. while we are running inside the lwIP
// thread. Lets det_conn_raw_send() pick a direct vs. marshaled tcp_write so the
// TLS BIO is safe from either context without re-marshaling (which would deadlock).
static volatile bool s_in_tcpip_thread = false;

struct DetTcpCall
{
    struct tcpip_api_call_data base;
    DetTcpOp op;
    uint8_t slot;
    struct tcp_pcb *pcb;
    const void *data;
    u16_t len;
    err_t result; ///< outcome of the op (DET_OP_SEND: whether the write was queued)
};

// Runs in tcpip_thread (via tcpip_api_call). Performs the requested raw lwIP op
// in the one context where it is safe; TLS record I/O (which also reaches
// tcp_write through the BIO) is done here too.
static err_t det_tcp_do(struct tcpip_api_call_data *c)
{
    DetTcpCall *k = (DetTcpCall *)c;
    k->result = ERR_OK;
    s_in_tcpip_thread = true; // any tcp_write reached from here is already in-thread
    switch (k->op)
    {
    case DET_OP_RAWSEND:
        k->result = tcp_write(k->pcb, k->data, k->len, TCP_WRITE_FLAG_COPY);
        if (k->result == ERR_OK)
            tcp_output(k->pcb);
        break;
    case DET_OP_SEND:
#if DETWS_ENABLE_TLS
        if (conn_pool[k->slot].tls)
        {
            k->result = (det_tls_write(k->slot, k->data, k->len) >= 0) ? ERR_OK : ERR_MEM;
            break;
        }
#endif
        k->result = tcp_write(k->pcb, k->data, k->len, TCP_WRITE_FLAG_COPY);
        break;
    case DET_OP_OUTPUT:
        tcp_output(k->pcb);
        break;
    case DET_OP_CLOSE:
#if DETWS_ENABLE_TLS
        if (conn_pool[k->slot].tls)
            det_tls_conn_end(k->slot); // close_notify + free the TLS context
#endif
        if (tcp_close(k->pcb) != ERR_OK)
            tcp_abort(k->pcb);
        break;
    case DET_OP_ABORT:
        tcp_abort(k->pcb);
        break;
    case DET_OP_DETACH:
        tcp_arg(k->pcb, nullptr);
        break;
    }
    s_in_tcpip_thread = false;
    return ERR_OK;
}

static inline err_t det_tcp_marshal(DetTcpOp op, uint8_t slot, struct tcp_pcb *pcb, const void *data, u16_t len)
{
    DetTcpCall k;
    memset(&k, 0, sizeof(k));
    k.op = op;
    k.slot = slot;
    k.pcb = pcb;
    k.data = data;
    k.len = len;
    tcpip_api_call(det_tcp_do, &k.base);
    return k.result;
}
#endif // ARDUINO

TcpConn conn_pool[MAX_CONNS];

uint32_t detws_ap_ip = 0;

uint32_t DeterministicAsyncTCP::conn_timeout_ms = CONN_TIMEOUT_MS;

// ---------------------------------------------------------------------------
// Connection output API
// ---------------------------------------------------------------------------
// The single send/flush/close path for every higher layer (HTTP app, WebSocket,
// SSE, SSH). Keeping it here means presentation and application code never call
// lwIP directly - they hand bytes to the transport layer, which decides whether
// they go out as plaintext (tcp_write) or through the TLS record layer. With
// DETWS_ENABLE_TLS off this is byte-identical to a direct tcp_write/tcp_output.

bool det_conn_send(uint8_t slot, struct tcp_pcb *pcb, const void *data, u16_t len)
{
#if defined(ARDUINO)
    return det_tcp_marshal(DET_OP_SEND, slot, pcb, data, len) == ERR_OK; // write runs in tcpip_thread
#else
#if DETWS_ENABLE_TLS
    if (conn_pool[slot].tls)
        return det_tls_write(slot, data, len) >= 0;
#endif
    (void)slot;
    return tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY) == ERR_OK;
#endif
}

u16_t det_conn_sndbuf(uint8_t slot, struct tcp_pcb *pcb)
{
    (void)slot;
    if (!pcb)
        return 0;
    u16_t avail = tcp_sndbuf(pcb);
#if DETWS_ENABLE_TLS
    // A TLS record adds header + MAC/tag overhead; report a conservative plaintext
    // budget so a caller that fills it does not overrun the cipher's framing.
    if (conn_pool[slot].tls)
        avail = (avail > 64) ? (u16_t)(avail - 64) : 0;
#endif
    return avail;
}

void det_conn_flush(uint8_t slot, struct tcp_pcb *pcb)
{
#if DETWS_ENABLE_TLS
    if (conn_pool[slot].tls)
        return; // ciphertext was already pushed by the TLS BIO (tcp_output per record);
                // flush must NOT end the session - persistent TLS (wss / TLS SSE) reuses it
#endif
    (void)slot;
#if defined(ARDUINO)
    det_tcp_marshal(DET_OP_OUTPUT, slot, pcb, nullptr, 0);
#else
    tcp_output(pcb);
#endif
}

bool det_conn_raw_send(struct tcp_pcb *pcb, const void *data, u16_t len)
{
    if (!pcb)
        return false;
#if defined(ARDUINO)
    if (s_in_tcpip_thread)
    {
        // Already inside the lwIP thread (the marshaled app-data send path): write
        // directly - re-marshaling here would deadlock on the tcpip mailbox.
        err_t e = tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);
        if (e == ERR_OK)
            tcp_output(pcb);
        return e == ERR_OK;
    }
    // Main-loop task (TLS handshake / read pump): marshal a raw write so the
    // tcp_write runs in the lwIP thread, not racing it.
    return det_tcp_marshal(DET_OP_RAWSEND, 0, pcb, data, len) == ERR_OK;
#else
    err_t e = tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);
    if (e == ERR_OK)
        tcp_output(pcb);
    return e == ERR_OK;
#endif
}

void det_conn_close(uint8_t slot, struct tcp_pcb *pcb)
{
    (void)slot;
#if defined(ARDUINO)
    det_tcp_marshal(DET_OP_CLOSE, slot, pcb, nullptr, 0); // TLS teardown + FIN in tcpip_thread
#else
#if DETWS_ENABLE_TLS
    if (conn_pool[slot].tls)
        det_tls_conn_end(slot);
#endif
    if (tcp_close(pcb) != ERR_OK)
        tcp_abort(pcb);
#endif
}

void det_conn_detach(struct tcp_pcb *pcb)
{
    // Disassociate the slot from this pcb's lwIP callbacks before freeing the
    // slot, so any late callback for the pcb finds a null arg and does nothing.
#if defined(ARDUINO)
    det_tcp_marshal(DET_OP_DETACH, 0, pcb, nullptr, 0);
#else
    tcp_arg(pcb, nullptr);
#endif
}

void det_conn_abort(struct tcp_pcb *pcb)
{
    // Hard reset (RST) for a fatal condition - no graceful FIN.
#if defined(ARDUINO)
    det_tcp_marshal(DET_OP_ABORT, 0, pcb, nullptr, 0);
#else
    tcp_abort(pcb);
#endif
}

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
            det_conn_detach(pcb); // tcpip_thread-marshaled tcp_arg(null) + abort
            det_conn_abort(pcb);
        }
        conn_pool[i].state = CONN_FREE;
        conn_pool[i].pcb = nullptr;
    }
}

uint32_t det_conn_remote_ip(uint8_t slot)
{
#ifdef ARDUINO
    if (slot >= MAX_CONNS)
        return 0;
    TcpConn *conn = &conn_pool[slot];
    if (conn->state == CONN_ACTIVE && conn->pcb)
        return ip4_addr_get_u32(ip_2_ip4(&conn->pcb->remote_ip));
#else
    (void)slot;
#endif
    return 0;
}

void DeterministicAsyncTCP::check_timeouts(int worker_id)
{
    uint32_t now = detws_millis();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        TcpConn *slot = &conn_pool[i];
        if (slot->owner != worker_id) // each worker reaps only its own slots
            continue;
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
            det_conn_detach(pcb); // tcpip_thread-marshaled tcp_arg(null) + abort
            det_conn_abort(pcb);
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

    slot->last_activity_ms = detws_millis();

    /*
     * Backpressure without data loss: if the whole segment will not fit in the
     * free ring space, refuse it (return ERR_MEM without freeing) so lwIP retains
     * it as refused_data and redelivers once the application has drained the
     * ring; nudge the main loop to drain. The previous code copied what fit and
     * dropped the rest, silently corrupting bodies larger than the ring (e.g.
     * streamed uploads). NOTE: needs RX_BUF_SIZE > the largest incoming segment
     * (TCP_MSS) so a full segment can eventually fit; smaller rings only ever see
     * sub-MSS requests, which always fit.
     */
    size_t used = (slot->rx_head + RX_BUF_SIZE - slot->rx_tail) % RX_BUF_SIZE;
    size_t free_space = (RX_BUF_SIZE - 1) - used; // keep one slot to tell full from empty
    if (p->tot_len > free_space)
    {
        TcpEvt evt = {EVT_DATA, slot->id, 0}; // wake the loop so it drains the ring
        enqueue(slot, evt);
        return ERR_MEM; // do NOT pbuf_free(p): lwIP keeps it and redelivers
    }

    size_t bytes_copied = 0;
    struct pbuf *q = p;
    while (q != nullptr)
    {
        uint8_t *payload = (uint8_t *)q->payload;
        for (u16_t i = 0; i < q->len; i++)
        {
            slot->rx_buffer[slot->rx_head] = payload[i];
            slot->rx_head = (slot->rx_head + 1) % RX_BUF_SIZE;
            bytes_copied++;
        }
        q = q->next;
    }

    // The whole segment was stored, so acknowledge all of it.
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
        slot->last_activity_ms = detws_millis();
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
