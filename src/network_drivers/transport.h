// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file transport.h
 * @brief Layer 4 (Transport) — TCP connection pool, ring buffers, and lwIP integration.
 *
 * Defines the static connection pool and the FreeRTOS event queue used to
 * pass connection events from lwIP callbacks (running in `tcpip_thread`) to
 * the Arduino main-loop task.
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
 * bytes actually copied — not `p->tot_len`.  This shrinks the TCP receive
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
};

/** @brief Static pool of connection contexts.  Defined in transport.cpp. */
extern TcpConn conn_pool[MAX_CONNS];

// ---------------------------------------------------------------------------
// Event queue
// ---------------------------------------------------------------------------

/**
 * @brief Type of connection event posted to the FreeRTOS event queue.
 */
enum EvtType
{
    EVT_CONNECT,    ///< New connection accepted.
    EVT_DATA,       ///< Data received; bytes are already in the ring buffer.
    EVT_DISCONNECT, ///< Remote peer closed the connection gracefully.
    EVT_ERROR       ///< lwIP reported an error (PCB may already be freed).
};

/**
 * @brief Event record posted from lwIP callbacks to the main-loop task.
 *
 * Small enough (8 bytes on 32-bit) that the FreeRTOS queue copies it by
 * value — no pointer lifetime issues.
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
 * @brief Static-only facade over the raw lwIP TCP API.
 *
 * All state is class-level (no instances).  Initialises the connection pool,
 * the FreeRTOS event queue, and the lwIP listening PCB once per boot via
 * init().  The lwIP callbacks (lowlevel_accept_cb, lowlevel_recv_cb, …) are
 * private implementation details in transport.cpp.
 */
class DeterministicAsyncTCP
{
  public:
    /**
     * @brief Initialise the TCP stack, create the event queue, and begin listening.
     *
     * Checks whether enough heap is available before attempting queue creation.
     * Zeroes the connection pool and stores the runtime config.
     *
     * @param port TCP port to bind and listen on.
     * @param cfg  Optional runtime config.  Pass nullptr to use defaults.
     * @return Positive value on success; negative value whose absolute value is
     *         the number of heap bytes needed when initialisation fails.
     */
    static int32_t init(uint16_t port, const WebServerConfig *cfg = nullptr);

    /**
     * @brief Stop the server: abort all connections, close the listener, free the queue.
     *
     * Safe to call from the main-loop task.  After stop() returns,
     * init() may be called again to restart.
     */
    static void stop();

    /**
     * @brief Scan the pool and force-close connections idle for > conn_timeout_ms.
     *
     * Called at the start of every server_tick() call, before the event queue
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
     * @brief FreeRTOS queue handle shared between lwIP callbacks and server_tick().
     *
     * Events are posted from `tcpip_thread` context via `xQueueSend()` (not
     * the ISR variant) because lwIP callbacks run in a task, not a hardware ISR.
     * Queue depth of 16 is sufficient for `MAX_CONNS * 4` event bursts.
     */
    static QueueHandle_t queue;

    /**
     * @brief Runtime connection-idle timeout in milliseconds.
     *
     * Loaded from WebServerConfig::conn_timeout_ms at init() time.
     * Defaults to CONN_TIMEOUT_MS if no config is supplied.
     */
    static uint32_t conn_timeout_ms;
};

#endif
