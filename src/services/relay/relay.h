// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file relay.h
 * @brief TCP relay / DNAT port forwarding (DETWS_ENABLE_RELAY) - a bidirectional byte pump.
 *
 * Publishes an internal `host:port` through the server: an inbound (accepted) connection is relayed
 * to an origin (an outbound connection to the internal service), moving bytes in both directions.
 * The engine is pure - it touches the two sockets only through send/recv seams - so it is
 * host-testable and rides `det_client` on the device. The app drives it: each poll tick (or whenever
 * a socket is readable/writable) it calls det_relay_step() until the relay reports DONE, then closes
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

#if DETWS_ENABLE_RELAY

#include <stddef.h>
#include <stdint.h>

/** @brief det_relay_step() outcome. */
enum DetRelayStatus
{
    DET_RELAY_ERROR = -1,  ///< a send/recv seam reported an error; the caller should close both sides
    DET_RELAY_RUNNING = 0, ///< still relaying (keep stepping)
    DET_RELAY_DONE = 1,    ///< both directions finished (EOF + drained); the caller closes both sides
};

/**
 * @brief Read up to @p cap bytes from the peer into @p buf.
 * @return bytes read (> 0), 0 if none are available now, or < 0 once the peer has closed its send
 *         side (EOF) or errored.
 */
typedef int (*DetRelayRecvFn)(void *ctx, uint8_t *buf, size_t cap);

/**
 * @brief Write up to @p len bytes to the peer.
 * @return bytes accepted (> 0, may be < @p len under backpressure), 0 if none can be accepted right
 *         now, or < 0 on error.
 */
typedef int (*DetRelaySendFn)(void *ctx, const uint8_t *buf, size_t len);

/** @brief Optional: signal the peer that no more data will be sent to it (a write-side half-close). */
typedef void (*DetRelayShutdownFn)(void *ctx);

/** @brief One end of a relay (a socket, behind seams). @p shutdown may be null. */
struct DetRelayEnd
{
    DetRelayRecvFn recv;
    DetRelaySendFn send;
    DetRelayShutdownFn shutdown;
    void *ctx;
};

/** @brief A relay between two ends. Owns the per-direction carry buffers; zero heap. */
struct DetRelay
{
    DetRelayEnd a, b;
    uint8_t buf_a2b[DETWS_RELAY_BUF];
    uint8_t buf_b2a[DETWS_RELAY_BUF];
    uint16_t a2b_len, a2b_off; ///< bytes read from a pending send to b, and how many already sent
    uint16_t b2a_len, b2a_off;
    bool a_eof, b_eof;             ///< the recv side of a / b has hit EOF
    bool a2b_done, b2a_done;       ///< the a->b / b->a direction has finished (EOF + drained)
    bool a_shut_sent, b_shut_sent; ///< the shutdown seam of a / b has been called
    uint32_t bytes_a2b, bytes_b2a; ///< bytes relayed each way (observability)
};

/**
 * @brief Initialize a relay between @p client (the inbound connection) and @p origin (the outbound
 *        connection to the internal service). Both ends are copied.
 */
void det_relay_init(DetRelay *r, const DetRelayEnd *client, const DetRelayEnd *origin);

/**
 * @brief Do one non-blocking pass: flush any pending bytes and read more, in both directions.
 * @return a ::DetRelayStatus. Call repeatedly (per poll tick) until DONE or ERROR, then close both.
 */
int det_relay_step(DetRelay *r);

#endif // DETWS_ENABLE_RELAY

#endif // DETERMINISTICESPASYNCWEBSERVER_RELAY_H
