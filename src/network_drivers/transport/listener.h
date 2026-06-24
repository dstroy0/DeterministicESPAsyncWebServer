// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file listener.h
 * @brief Layer 4 (Listener) - per-port TCP listener abstraction.
 *
 * Each active listener owns one lwIP listening PCB and one statically-
 * allocated FreeRTOS queue.  When a new client connects, `listener_accept_cb`
 * claims a slot from the shared `conn_pool`, wires the standard per-connection
 * callbacks, and posts `EVT_CONNECT` to the owning listener's queue.
 *
 * The session layer drains all active listener queues each `server_tick()`,
 * routing events to the correct protocol handler via `TcpConn::proto`.
 *
 * **Single accept callback**
 * `tcp_arg(listen_pcb, (void*)(uintptr_t)idx)` embeds the listener index in
 * the PCB user-data so a single static `listener_accept_cb` handles all ports.
 *
 * **Circular-dependency resolution**
 * transport.cpp needs to post events to listener queues but cannot include
 * this header (listener.h already includes transport.h).  The symbol
 * `listener_enqueue()` is exported from listener.cpp; transport.cpp calls it
 * via a forward declaration added to transport.h so no circular include
 * is introduced.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_LISTENER_H
#define DETERMINISTICESPASYNCWEBSERVER_LISTENER_H

#include "DetWebServerConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"
#include "transport.h"

// ---------------------------------------------------------------------------
// Listener pool entry
// ---------------------------------------------------------------------------

/**
 * @brief State for one TCP listening port.
 *
 * All queue storage is embedded in this struct so the entire listener pool
 * lives in BSS - no heap allocation anywhere in the listener layer.
 *
 * A single `Listener` instance consumes:
 *   sizeof(tcp_pcb*) + sizeof(StaticQueue_t) + EVT_QUEUE_DEPTH*sizeof(TcpEvt)
 *   + sizeof(QueueHandle_t) + 3 bytes overhead (port, proto, active).
 */
struct Listener
{
    uint16_t port;               ///< TCP port this listener binds.
    ConnProto proto;             ///< Application protocol for all connections accepted here.
    struct tcp_pcb *listen_pcb;  ///< lwIP listen PCB; nullptr when inactive.
    StaticQueue_t _queue_struct; ///< FreeRTOS static queue descriptor.
    uint8_t _queue_storage[EVT_QUEUE_DEPTH * sizeof(TcpEvt)]; ///< Queue backing store.
    QueueHandle_t queue;                                      ///< Handle returned by xQueueCreateStatic().
    bool active; ///< True after listener_add(), false after listener_stop().
    bool tls;    ///< True when connections accepted here begin a TLS handshake.
};

/** @brief Static pool of listener contexts.  Defined in listener.cpp. */
extern Listener listener_pool[MAX_LISTENERS];

// ---------------------------------------------------------------------------
// Listener management API
// ---------------------------------------------------------------------------

/**
 * @brief Create a listening socket on @p port and register it at @p idx.
 *
 * If the slot at @p idx is already active it is stopped first.
 * Creates a per-listener FreeRTOS static queue and an lwIP listening PCB.
 * Wires `listener_accept_cb` as the accept handler with the listener index
 * embedded as the PCB user-data argument.
 *
 * @param idx   Slot in listener_pool[] (0 … MAX_LISTENERS-1).
 * @param port  TCP port to bind and listen on.
 * @param proto Application protocol spoken on connections from this port.
 * @param tls   When true, connections accepted here start a TLS handshake.
 * @return Positive value on success; -1 on failure (pool full or lwIP error).
 */
int32_t listener_add(uint8_t idx, uint16_t port, ConnProto proto, bool tls = false);

/**
 * @brief Stop listening on the port at @p idx and release its resources.
 *
 * Idempotent - safe to call on an already-stopped slot.
 * Closes the lwIP listening PCB and deletes the FreeRTOS queue.
 * Does not close any connections already accepted on this port.
 *
 * @param idx  Slot in listener_pool[].
 */
void listener_stop(uint8_t idx);

/**
 * @brief Stop all active listeners.
 *
 * Convenience wrapper that calls listener_stop() for every slot in
 * listener_pool[].  Called by DetWebServer::stop().
 */
void listener_stop_all();

/**
 * @brief Post @p evt to the queue owned by listener @p listener_id.
 *
 * Called from transport.cpp callbacks (running in tcpip_thread context) to
 * deliver connection events to the session layer.  Uses xQueueSend with
 * timeout=0 - a full queue means the application is not calling server_tick()
 * fast enough; the dropped event is recoverable via connection timeout.
 *
 * @param listener_id  Index into listener_pool[]; must be < MAX_LISTENERS.
 * @param evt          Event to copy into the queue.
 */
void listener_enqueue(uint8_t listener_id, const TcpEvt *evt);

/**
 * @brief Fixed-window global accept-rate gate (connection-flood defense).
 *
 * Returns true if a new connection accepted at @p now_ms is within the
 * DETWS_ACCEPT_THROTTLE_MAX-per-DETWS_ACCEPT_THROTTLE_WINDOW_MS budget (and
 * counts it), false if the budget for the current window is exhausted. State is
 * two static counters shared across all listeners. The accept callback consults
 * this only when DETWS_ENABLE_ACCEPT_THROTTLE is set; the function is always
 * compiled so it can be unit-tested. Call listener_accept_throttle_reset() to
 * clear the window (e.g. between tests).
 */
bool listener_accept_allowed(uint32_t now_ms);

/** @brief Reset the accept-throttle window counters. */
void listener_accept_throttle_reset(void);

#endif
