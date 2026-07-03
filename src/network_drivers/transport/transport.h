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
 * `state`, `rx_head`, and `rx_tail` are `DetAtomic` (acquire/release): the
 * single-producer / single-consumer ring buffer is correct without a mutex
 * because the release store of an index publishes the preceding buffer writes
 * and the acquire load observes them, on either core.
 *
 * **Backpressure (lossless)**
 * When a whole inbound segment will not fit the free ring space, the recv
 * callback refuses it (returns ERR_MEM without freeing the pbuf); lwIP retains
 * it as refused_data and redelivers once the main loop has drained the ring, so
 * no received byte is dropped. Requires RX_BUF_SIZE > one TCP segment (TCP_MSS).
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
#include "network_drivers/network/det_ip.h" // DetIp (family-tagged peer address)
#include "shared_primitives/det_ring.h"     // DetAtomic + the shared SPSC ring drain primitive
#include <Arduino.h>
#include <atomic>

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
    uint8_t id;                 ///< Fixed slot index (0 … MAX_CONNS-1).
    DetAtomic<ConnState> state; ///< Lifecycle state; acquire/release for inter-task visibility.
    struct tcp_pcb *pcb;        ///< lwIP PCB; null when slot is free.
    uint32_t last_activity_ms;  ///< `millis()` timestamp of last TX/RX event.

    uint8_t rx_buffer[RX_BUF_SIZE]; ///< Ring buffer storage.
    DetAtomic<size_t> rx_head;      ///< Producer write index (lwIP/tcpip context).
    DetAtomic<size_t> rx_tail;      ///< Consumer read index (worker context).
    size_t rx_acked;                ///< rx_tail position last ACKed to lwIP (tcp_recved). Worker-only:
                                    ///< the window is reopened by exactly the bytes drained since, so it
                                    ///< tracks ring occupancy (ack-on-consume) rather than copy.

    uint8_t listener_id; ///< Index into listener_pool[]; set at accept time.
    uint8_t owner;       ///< Worker that owns this slot (round-robin at accept). Always 0 at N=1.
    ConnProto proto;     ///< Application protocol for this connection.
    uint8_t
        proto_slot; ///< Per-protocol session/pool index (0xFF = none): the SSH session, an MQTT/Modbus session, etc.
    uint8_t iface;  ///< DetIface this connection arrived on; set at accept time.
    uint8_t tls;    ///< Non-zero when this connection is TLS (set at accept time).
};

/** @brief Sentinel for TcpConn::proto_slot meaning "no per-protocol session bound". */
#define DETWS_PROTO_SLOT_NONE 0xFFu

/**
 * @brief softAP IPv4 address (network byte order) for STA/AP interface tagging.
 *
 * Zero when no softAP is configured. Set via DetWebServer::set_ap_ip(); the
 * accept callback tags each connection DETIFACE_AP when its local IP equals
 * this, else DETIFACE_STA. Used by per-route interface filters.
 */
extern uint32_t detws_ap_ip;

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
     *
     * Only sweeps slots owned by @p worker_id, so each worker reaps just its own
     * connections (no cross-worker writes). At DETWS_WORKER_COUNT=1 every slot is
     * owned by worker 0, so it sweeps the whole pool as before.
     */
    static void check_timeouts(int worker_id = 0);

    /**
     * @brief Runtime connection-idle timeout in milliseconds.
     *
     * Loaded from WebServerConfig::conn_timeout_ms at pool_init() time.
     * Defaults to CONN_TIMEOUT_MS if no config is supplied.
     */
    static uint32_t conn_timeout_ms;
};

// ---------------------------------------------------------------------------
// Connection output API (defined in transport.cpp)
// ---------------------------------------------------------------------------
// The one send/flush/close path for all higher layers. Presentation (WebSocket,
// SSE, SSH) and the HTTP application call these instead of touching lwIP, so the
// transport layer stays the sole owner of TCP I/O. det_conn_send/flush are
// TLS-aware (route through the TLS record layer when the slot is a TLS conn);
// with DETWS_ENABLE_TLS off they are byte-identical to tcp_write/tcp_output.

/**
 * @brief Send @p len bytes on connection @p slot (copies @p data; TLS-aware).
 * @return true if the bytes were queued; false if the send buffer was full and
 *         the write was refused. A streaming producer should pace with
 *         det_conn_sndbuf() and resume on a later loop; existing fixed-size
 *         senders may ignore the result.
 */
bool det_conn_send(uint8_t slot, const void *data, u16_t len);

/**
 * @brief Bytes that can currently be queued for sending on @p slot.
 *
 * Advisory free space in the TCP send buffer: a producer can send at most this
 * many bytes per handle() loop and resume on the next loop as the window drains
 * (the on_poll hook is the natural resume point). For a TLS slot the usable
 * plaintext is somewhat less (TLS record + cipher overhead). Returns 0 when
 * the slot has no pcb.
 */
u16_t det_conn_sndbuf(uint8_t slot);

/** @brief Flush queued bytes / finish the send on @p slot (TLS-aware). */
void det_conn_flush(uint8_t slot);

/**
 * @brief Reopen the TCP receive window by however much @p slot has drained.
 *
 * Ack-on-consume, owned entirely by the transport layer: recv_cb no longer
 * ACKs on copy. Instead the window tracks how much the application has actually
 * drained from the ring (rx_tail) since the last ACK, so it never advertises more
 * than the ring can hold and the peer is paced to the consumer; a slow sink (e.g.
 * flash writes during a streamed upload) can never overflow the ring and deadlock.
 *
 * Other layers do not touch the ring indices to manage flow control - the worker
 * just calls this once per owned slot per loop and transport does the rest
 * (computes the delta vs rx_acked, marshals tcp_recved to tcpip_thread, advances
 * rx_acked). A no-op when nothing was drained or @p slot is not active.
 */
void det_conn_ack_consumed(uint8_t slot);

// ---------------------------------------------------------------------------
// RX ring read API - the single way any layer drains received bytes.
//
// Transport owns the ring; consumers (HTTP/WS/Telnet/SSH/TLS and the framed
// services) must never index rx_buffer or advance rx_tail themselves - they call
// these. Consuming functions advance rx_tail only; the window is reopened by the
// worker's det_conn_ack_consumed() once per loop (one owner, no per-byte ACK).
// Single-consumer per slot (the owning worker), so no locking here. These are
// inline because the byte path is hot and the ring internals live in this header.
// ---------------------------------------------------------------------------

// All five delegate to the shared SPSC ring primitive (det_ring.h) over the slot's
// rx_buffer - the server transport never reimplements the ring math.

/** @brief Bytes currently available to read from @p slot's ring. */
static inline size_t det_conn_available(uint8_t slot)
{
    const TcpConn *c = &conn_pool[slot];
    return det_ring_available(c->rx_head, c->rx_tail, RX_BUF_SIZE);
}

/** @brief Pop one byte into @p out; false if the ring is empty. */
static inline bool det_conn_read_byte(uint8_t slot, uint8_t *out)
{
    TcpConn *c = &conn_pool[slot];
    return det_ring_read_byte(c->rx_buffer, RX_BUF_SIZE, c->rx_head, c->rx_tail, out);
}

/** @brief Copy @p n bytes at @p off from the tail into @p dst WITHOUT consuming (lookahead). */
static inline void det_conn_peek(uint8_t slot, size_t off, uint8_t *dst, size_t n)
{
    const TcpConn *c = &conn_pool[slot];
    det_ring_peek(c->rx_buffer, RX_BUF_SIZE, c->rx_tail, off, dst, n);
}

/** @brief Drop @p n bytes from the tail (advance past already-peeked data). */
static inline void det_conn_consume(uint8_t slot, size_t n)
{
    det_ring_consume(conn_pool[slot].rx_tail, RX_BUF_SIZE, n);
}

/** @brief Pop up to @p cap bytes into @p buf; returns the count read. */
static inline size_t det_conn_read(uint8_t slot, uint8_t *buf, size_t cap)
{
    TcpConn *c = &conn_pool[slot];
    return det_ring_read(c->rx_buffer, RX_BUF_SIZE, c->rx_head, c->rx_tail, buf, cap);
}

/**
 * @brief Write raw bytes straight to @p pcb (no TLS), context-safe.
 *
 * This is the one safe path for the TLS engine's BIO to emit ciphertext: it
 * writes directly when already running inside the lwIP thread (the marshaled
 * app-data send path) and marshals a raw write when called from the main-loop
 * task (the handshake / read pump), so a TLS handshake never does an
 * unsynchronized tcp_write from the main loop. Calls tcp_output() on success.
 * @return true if the bytes were queued; false on a full send buffer (ERR_MEM).
 */
bool det_conn_raw_send(struct tcp_pcb *pcb, const void *data, u16_t len);

/**
 * @brief Close connection @p slot gracefully (tcp_close), aborting if the FIN
 *        cannot be queued. The transport owns the whole teardown: it detaches the
 *        pcb from its lwIP callbacks, frees the slot, and (TLS slot) emits
 *        close_notify + frees the per-connection TLS context - so callers pass
 *        only the slot and never touch the raw tcp_pcb. A no-op if the slot has
 *        no live pcb.
 */
void det_conn_close(uint8_t slot);

/**
 * @brief Begin a graceful close that dwells in CONN_CLOSING until the peer ACKs.
 *
 * Unlike det_conn_close() (immediate teardown), this leaves the slot's PCB and
 * callbacks live and moves it ACTIVE -> CONN_CLOSING. The slot finalizes (PCB
 * closed, slot freed) from the sent callback once the response has fully drained,
 * or from the idle sweep after DETWS_CLOSING_TIMEOUT_MS if the peer never ACKs.
 * The caller must already have queued (det_conn_send) + flushed the response.
 * A no-op if the slot is not CONN_ACTIVE (e.g. an error freed it mid-write).
 */
void det_conn_begin_close(uint8_t slot_id);

/** @brief Detach @p pcb from its slot's lwIP callbacks before the slot is freed. */
void det_conn_detach(struct tcp_pcb *pcb);

/** @brief Hard-abort @p pcb (RST) for a fatal condition; no graceful FIN. */
void det_conn_abort(struct tcp_pcb *pcb);

/**
 * @brief Hard-abort connection @p slot (RST) for a fatal condition. The transport
 *        owns the teardown order: free the per-connection TLS context (abrupt, no
 *        close_notify), detach the pcb from its callbacks, reset the slot, then
 *        abort - so callers pass only the slot and never touch the raw tcp_pcb. A
 *        no-op if the slot has no live pcb.
 */
void det_conn_abort_slot(uint8_t slot);

/**
 * @brief Raw source IPv4 of the connection in @p slot, or 0 if the slot has no
 *        active pcb (or on host builds). Byte order is irrelevant: this is an
 *        identity key (e.g. for the auth lockout), not for display. Keeps the
 *        lwIP pcb access inside L4 so callers never reach into the pcb directly.
 */
uint32_t det_conn_remote_ip(uint8_t slot);

/**
 * @brief The connected peer's address as a family-tagged ::DetIp (IPv4 or IPv6).
 *
 * Unlike det_conn_remote_ip() (which flattens to a v4 uint32 and cannot represent a v6 peer),
 * this reports the real address for a dual-stack build (DETWS_ENABLE_IPV6). Format it with
 * det_ip_format() or classify it with det_ip_classify().
 * @return true if @p slot has an active connection whose address was written to @p out.
 */
bool det_conn_remote_addr(uint8_t slot, DetIp *out);

/**
 * @brief A stable per-peer 32-bit identity key for @p slot (the v4 address, or an FNV-1a hash of a
 *        v6 address). For rate-limit / auth-lockout buckets, where a v6 peer must not silently
 *        share the all-zero v4 bucket. Returns 0 if the slot has no active connection.
 */
uint32_t det_conn_remote_key(uint8_t slot);

// ---------------------------------------------------------------------------
// Observability (DETWS_ENABLE_OBSERVABILITY) - connection event hook + counters
// ---------------------------------------------------------------------------
#if DETWS_ENABLE_OBSERVABILITY

/** @brief Why a connection event fired (the reason for a transition or notice). */
enum DetConnReason
{
    DET_CONN_R_ACCEPT,       ///< New connection accepted (CONN_FREE -> CONN_ACTIVE).
    DET_CONN_R_CLOSE_REMOTE, ///< Peer closed gracefully (FIN received).
    DET_CONN_R_CLOSE_LOCAL,  ///< Application initiated the close.
    DET_CONN_R_ERROR,        ///< lwIP reported a fatal error on the connection.
    DET_CONN_R_TIMEOUT,      ///< Idle-timeout sweep reaped the slot.
    DET_CONN_R_ABORT,        ///< Forced abort (server stop / pool reset).
    DET_CONN_R_DRAINED,      ///< CONN_CLOSING slot finished draining -> closed.
    DET_CONN_R_BACKPRESSURE, ///< RX segment refused (ring full); no state change.
    DET_CONN_R_DEFER_DROP    ///< Event queue full; an event was dropped (no state change).
};

/** @brief Snapshot of the transport's lifetime counters (plus a live gauge). */
struct DetConnCounters
{
    uint32_t accepts;        ///< Connections accepted.
    uint32_t closes_remote;  ///< Closed by peer FIN.
    uint32_t closes_local;   ///< Closed by the application.
    uint32_t closes_error;   ///< Closed by an lwIP error.
    uint32_t closes_timeout; ///< Reaped by the idle-timeout sweep.
    uint32_t closes_abort;   ///< Force-aborted (stop / reset).
    uint32_t backpressure;   ///< RX segments refused for lack of ring space.
    uint32_t defer_drops;    ///< Deferred events dropped because the queue was full.
    uint32_t closing_gauge;  ///< Slots currently in CONN_CLOSING (live, not cumulative).
};

/**
 * @brief Callback fired on every connection state transition.
 *
 * Runs in whichever task drove the transition (tcpip_thread for accept / recv /
 * error, a worker for close / timeout), so keep it short and non-blocking and do
 * not call back into the server from it. @p old_state == @p new_state for the
 * non-transition notices (backpressure, defer-drop).
 */
typedef void (*DetConnEventCb)(uint8_t slot, ConnState old_state, ConnState new_state, DetConnReason reason);

/** @brief Register (or clear, with nullptr) the connection event callback. */
void det_conn_on_event(DetConnEventCb cb);

/** @brief Read a consistent snapshot of the transport counters. */
DetConnCounters det_conn_counters();

/** @brief Zero the cumulative counters (the live CONN_CLOSING gauge is untouched). */
void det_conn_counters_reset();

// Internal notify points (transport.cpp), reached via the macros below so both
// transport.cpp and listener.cpp (accept) record through one path.
void detws_obs_transition(uint8_t slot, ConnState olds, ConnState news, DetConnReason reason);
void detws_obs_notice(uint8_t slot, ConnState st, DetConnReason reason);
#define DETWS_OBS_TRANSITION(slot, olds, news, reason) detws_obs_transition((slot), (olds), (news), (reason))
#define DETWS_OBS_NOTICE(slot, st, reason) detws_obs_notice((slot), (st), (reason))

#else // !DETWS_ENABLE_OBSERVABILITY

// Compile to nothing; the arguments (incl. DetConnReason names, only declared
// when the feature is on) are dropped unparsed by the preprocessor.
#define DETWS_OBS_TRANSITION(slot, olds, news, reason) ((void)0)
#define DETWS_OBS_NOTICE(slot, st, reason) ((void)0)

#endif // DETWS_ENABLE_OBSERVABILITY

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

/*
 * Forward declaration of listener_enqueue() to break the circular include.
 * See listener.h for the full documentation of this function.
 * Returns true if the event was queued, false if it was dropped (queue full or
 * inactive listener) - the transport observes drops as DET_CONN_R_DEFER_DROP.
 */
bool listener_enqueue(uint8_t listener_id, const TcpEvt *evt);

#endif
