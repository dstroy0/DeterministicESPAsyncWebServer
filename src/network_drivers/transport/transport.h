// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file transport.h
 * @brief Layer 4 (Transport) - TCP connection pool, ring buffers, and lwIP integration.
 *
 * Defines the static connection pool and the per-connection event plumbing.
 * Each listener port owns its own FreeRTOS queue (see listener.h); the
 * session layer drains all active queues each tick via server_tick().
 *
 * **Concurrency model**
 * | Context          | Reads                  | Writes                  |
 * |------------------|------------------------|-------------------------|
 * | lwIP callbacks   | rx_head (to check full)| rx_buffer[], rx_head    |
 * | Arduino loop     | rx_buffer[], rx_tail   | rx_tail                 |
 *
 * `rx_head` and `rx_tail` are `volatile` to prevent the compiler from
 * caching them across the inter-task boundary.  The single-producer /
 * single-consumer ring buffer design guarantees correctness without a mutex
 * as long as writes to each index are atomic (true for 32-bit Xtensa).
 *
 * **Backpressure**
 * When the ring buffer is full, `tcp_recved()` is called with only the
 * bytes actually copied - not `p->tot_len`.  This shrinks the TCP receive
 * window and applies backpressure to the sender rather than silently
 * dropping data.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TRANSPORT_H
#define DETERMINISTICESPASYNCWEBSERVER_TRANSPORT_H

#include "DetWebServerConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Connection state
// ---------------------------------------------------------------------------

/**
 * @brief Lifecycle state of a connection pool slot.
 *
 * Transitions:
 * - `CONN_FREE → CONN_ACTIVE` : accept callback fires.
 * - `CONN_ACTIVE → CONN_FREE` : graceful close, error, or timeout.
 * - `CONN_ACTIVE → CONN_CLOSING` : (reserved for future half-close support).
 */
enum ConnState
{
    CONN_FREE,   ///< Slot is available; no PCB is attached.
    CONN_ACTIVE, ///< Live connection; PCB is valid.
    CONN_CLOSING ///< FIN sent; waiting for final ACK (reserved).
};

/**
 * @brief A single TCP connection context.
 *
 * Sized so that `MAX_CONNS` instances fit in a static array without
 * fragmentation.  All fields except the volatile ring-buffer indices may
 * only be accessed from the Arduino main-loop task.
 */
struct TcpConn
{
    uint8_t id;                ///< Fixed slot index (0 … MAX_CONNS-1).
    volatile ConnState state;  ///< Lifecycle state; volatile for inter-task visibility.
    struct tcp_pcb *pcb;       ///< lwIP PCB; null when slot is free.
    uint32_t last_activity_ms; ///< `millis()` timestamp of last TX/RX event.

    uint8_t rx_buffer[RX_BUF_SIZE]; ///< Ring buffer storage.
    volatile size_t rx_head;        ///< Producer write index (lwIP context).
    volatile size_t rx_tail;        ///< Consumer read index (main-loop context).

    uint8_t listener_id; ///< Index into listener_pool[]; set at accept time.
    ConnProto proto;     ///< Application protocol for this connection.
    uint8_t ssh_id;      ///< SSH session slot (PROTO_SSH only); 0xFF when none.
};

/** @brief Sentinel for TcpConn::ssh_id meaning "no SSH session bound". */
#define SSH_ID_NONE 0xFFu

/** @brief Static pool of connection contexts.  Defined in transport.cpp. */
extern TcpConn conn_pool[MAX_CONNS];

// ---------------------------------------------------------------------------
// Event queue
// ---------------------------------------------------------------------------

/**
 * @brief Type of connection event posted to a listener's FreeRTOS queue.
 */
enum EvtType
{
    EVT_CONNECT,    ///< New connection accepted.
    EVT_DATA,       ///< Data received; bytes are already in the ring buffer.
    EVT_DISCONNECT, ///< Remote peer closed the connection gracefully.
    EVT_ERROR       ///< lwIP reported an error (PCB may already be freed).
};

/**
 * @brief Event record posted from lwIP callbacks to the session layer.
 *
 * Small enough (≤12 bytes on 32-bit) that the FreeRTOS queue copies it by
 * value - no pointer lifetime issues.
 */
struct TcpEvt
{
    EvtType type;    ///< What happened.
    uint8_t slot_id; ///< Which connection slot is affected.
    size_t data_len; ///< Bytes copied (EVT_DATA only); 0 for other types.
};

// ---------------------------------------------------------------------------
// DeterministicAsyncTCP
// ---------------------------------------------------------------------------

/**
 * @class DeterministicAsyncTCP
 * @brief Static-only facade managing the shared TCP connection pool.
 *
 * All state is class-level (no instances).  pool_init() initializes the
 * connection pool and the runtime timeout config once per boot (or per
 * restart cycle).  Listening sockets and per-listener queues are owned by
 * the listener layer (see listener.h); this class only manages the shared
 * conn_pool[] and the idle-timeout sweep.
 */
class DeterministicAsyncTCP
{
  public:
    /**
     * @brief Initialize the connection pool and store the runtime config.
     *
     * Zeroes all connection slots and sets conn_timeout_ms from @p cfg.
     * Call this before calling listener_add() for each port.
     *
     * @param cfg  Optional runtime config.  Pass nullptr to use compile-time
     *             defaults (CONN_TIMEOUT_MS).
     */
    static void pool_init(const WebServerConfig *cfg = nullptr);

    /**
     * @brief Abort all active connections and reset the pool to CONN_FREE.
     *
     * Does not touch listener PCBs or listener queues - call listener_stop_all()
     * before this if you also want to close the listening sockets.
     * Safe to call from the main-loop task.
     */
    static void stop();

    /**
     * @brief Scan the pool and force-close connections idle for > conn_timeout_ms.
     *
     * Called at the start of every server_tick() call, before any event queue
     * is drained.  Uses `tcp_abort()` (not `tcp_close()`) because the
     * connection has already timed out and a graceful FIN is not warranted.
     *
     * A timed-out slot has its state set to `CONN_FREE` and `pcb` cleared
     * *before* `tcp_abort()` is called, so any in-flight lwIP callback for
     * that PCB will see `slot->state != CONN_ACTIVE` and exit without
     * touching the slot.
     */
    static void check_timeouts();

    /**
     * @brief Runtime connection-idle timeout in milliseconds.
     *
     * Loaded from WebServerConfig::conn_timeout_ms at pool_init() time.
     * Defaults to CONN_TIMEOUT_MS if no config is supplied.
     */
    static uint32_t conn_timeout_ms;

    /**
     * @brief Always returns 0 - the library makes no heap allocations.
     *
     * Each listener's event queue is backed by statically-allocated BSS
     * storage inside the Listener struct.  Retained for API compatibility.
     */
    static size_t heap_needed();

    /**
     * @brief Always returns true - no heap allocation means no pre-flight needed.
     *
     * Retained for API compatibility.  Safe to call or omit.
     */
    static bool heap_available();
};

// ---------------------------------------------------------------------------
// Per-connection lwIP callbacks (defined in transport.cpp, used in listener.cpp)
// ---------------------------------------------------------------------------

/**
 * @brief lwIP receive callback - wired to each new connection by listener_accept_cb.
 * @see transport.cpp
 */
err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/**
 * @brief lwIP sent callback - refreshes the idle-timeout timestamp.
 * @see transport.cpp
 */
err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);

/**
 * @brief lwIP error callback - fires when the stack detects a fatal error.
 * @see transport.cpp
 */
void lowlevel_err_cb(void *arg, err_t err);

// ---------------------------------------------------------------------------
// Event enqueue (defined in listener.cpp, called from transport.cpp)
// ---------------------------------------------------------------------------

/**
 * @brief Post @p evt to the queue owned by listener @p listener_id.
 *
 * Forward declaration here breaks the circular-include chain:
 * transport.cpp calls this function but cannot include listener.h (because
 * listener.h already includes transport.h).  The linker resolves it to
 * listener.cpp at link time.
 *
 * @param listener_id  Index into listener_pool[].
 * @param evt          Event to enqueue.
 */
void listener_enqueue(uint8_t listener_id, const TcpEvt *evt);

#endif
