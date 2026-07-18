// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file relay.h
 * @brief TCP relay / DNAT port forwarding (DWS_ENABLE_RELAY) - a bidirectional byte pump.
 *
 * Publishes an internal `host:port` through the server: an inbound (accepted) connection is relayed
 * to an origin (an outbound connection to the internal service), moving bytes in both directions.
 * The engine is pure - it touches the two sockets only through send/recv seams - so it is
 * host-testable and rides `dws_client` on the device. The app drives it: each poll tick (or whenever
 * a socket is readable/writable) it calls dws_relay_step() until the relay reports DONE, then closes
 * both sockets.
 *
 * Correctness details:
 *  - **Backpressure**: a `send` seam may accept fewer bytes than offered; the un-accepted bytes are
 *    carried in a per-direction buffer and retried on the next step before more are read.
 *  - **Independent half-close**: each direction finishes when its source signals EOF and its buffer
 *    drains. When a direction finishes, the opposite peer's optional `shutdown` seam is called once
 *    (propagating the half-close so the origin sees the client's FIN); the relay is DONE only when
 *    both directions have finished.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_RELAY_H
#define DETERMINISTICESPASYNCWEBSERVER_RELAY_H

#include "ServerConfig.h"

#if DWS_ENABLE_RELAY

#include <stddef.h>
#include <stdint.h>

/** @brief dws_relay_step() outcome. */
enum class DWSRelayStatus : int32_t
{
    DWS_RELAY_ERROR = -1,  ///< a send/recv seam reported an error; the caller should close both sides
    DWS_RELAY_RUNNING = 0, ///< still relaying (keep stepping)
    DWS_RELAY_DONE = 1,    ///< both directions finished (EOF + drained); the caller closes both sides
};

/**
 * @brief Read up to @p cap bytes from the peer into @p buf.
 * @return bytes read (> 0), 0 if none are available now, or < 0 once the peer has closed its send
 *         side (EOF) or errored.
 */
using DWSRelayRecvFn = int (*)(void *ctx, uint8_t *buf, size_t cap);

/**
 * @brief Write up to @p len bytes to the peer.
 * @return bytes accepted (> 0, may be < @p len under backpressure), 0 if none can be accepted right
 *         now, or < 0 on error.
 */
using DWSRelaySendFn = int (*)(void *ctx, const uint8_t *buf, size_t len);

/** @brief Optional: signal the peer that no more data will be sent to it (a write-side half-close). */
using DWSRelayShutdownFn = void (*)(void *ctx);

/** @brief One end of a relay (a socket, behind seams). @p shutdown may be null. */
struct DWSRelayEnd
{
    DWSRelayRecvFn recv;
    DWSRelaySendFn send;
    DWSRelayShutdownFn shutdown;
    void *ctx;
};

/** @brief A relay between two ends. Owns the per-direction carry buffers; zero heap. */
struct DWSRelay
{
    DWSRelayEnd a;
    DWSRelayEnd b;
    uint8_t buf_a2b[DWS_RELAY_BUF];
    uint8_t buf_b2a[DWS_RELAY_BUF];
    uint16_t a2b_len; ///< bytes read from a pending send to b
    uint16_t a2b_off; ///< how many of those already sent
    uint16_t b2a_len;
    uint16_t b2a_off;
    bool a_eof;         ///< the recv side of a has hit EOF
    bool b_eof;         ///< the recv side of b has hit EOF
    bool a2b_done;      ///< the a->b direction has finished (EOF + drained)
    bool b2a_done;      ///< the b->a direction has finished (EOF + drained)
    bool a_shut_sent;   ///< the shutdown seam of a has been called
    bool b_shut_sent;   ///< the shutdown seam of b has been called
    uint32_t bytes_a2b; ///< bytes relayed a->b (observability)
    uint32_t bytes_b2a; ///< bytes relayed b->a (observability)
};

/**
 * @brief Initialize a relay between @p client (the inbound connection) and @p origin (the outbound
 *        connection to the internal service). Both ends are copied.
 */
void dws_relay_init(DWSRelay *r, const DWSRelayEnd *client, const DWSRelayEnd *origin);

/**
 * @brief Do one non-blocking pass: flush any pending bytes and read more, in both directions.
 * @return a ::DWSRelayStatus. Call repeatedly (per poll tick) until DONE or ERROR, then close both.
 */
DWSRelayStatus dws_relay_step(DWSRelay *r);

/**
 * @brief Signal that a peer's send side has closed, when the transport reports EOF out of band (a
 *        close callback) rather than through @c recv returning < 0.
 *
 * Some transports (e.g. the server's `dws_conn`, which delivers a close as an `on_close` event, not
 * as a short read) cannot report EOF via the recv seam. Call this from that event so the direction
 * that peer sources finishes cleanly: the bytes already buffered are still flushed, then the opposite
 * peer's `shutdown` fires and the relay reaches DONE once both directions have finished.
 *
 * @param origin false for the client (inbound) side, true for the origin (outbound) side.
 */
void dws_relay_note_eof(DWSRelay *r, bool origin);

#endif // DWS_ENABLE_RELAY

#endif // DETERMINISTICESPASYNCWEBSERVER_RELAY_H
