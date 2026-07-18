// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_CLIENT_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_CLIENT_H

/**
 * @file client.h
 * @brief Layer 4 outbound TCP client transport - the client-side peer of the
 *        (server) transport in tcp.cpp.
 *
 * A small fixed pool of outbound connections so the application's clients
 * (services/http_client, services/mqtt, services/ws_client) no longer each own a
 * private raw-lwIP TCP stack at L7. As with the server transport, every raw
 * `tcp_*()` call is marshaled into `tcpip_thread` via `tcpip_api_call()`, so the
 * main-loop/worker task never races the stack. All storage is static (no heap).
 *
 * The receive ring carries **wire bytes**: for a plaintext connection those are
 * the application bytes; for a TLS connection they are ciphertext and the caller
 * layers the shared client-TLS session (`det_tls_client_session_*`) on top, pointing its
 * BIO at det_client_send() / det_client_read().
 *
 * The core is non-blocking (read/available/send), so it suits both a polling
 * client loop (MQTT, WebSocket) and a blocking request (HTTP) that drives its own
 * deadline. Only det_client_open() blocks, on DNS + connect.
 */

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Resolve @p host (dotted-quad fast path, else DNS) and connect to
 *        @p host : @p port, blocking up to @p timeout_ms.
 * @return A client id in [0, DETWS_CLIENT_CONNS) on success, or < 0 on failure
 *         (pool full, DNS failure, connect timeout/refused).
 */
int det_client_open(const char *host, uint16_t port, uint32_t timeout_ms);

/** @brief True once the TCP handshake has completed for @p cid. */
bool det_client_connected(int cid);

/** @brief True once the peer closed (FIN) or the connection errored. */
bool det_client_is_closed(int cid);

/** @brief Queue @p len wire bytes for transmission (marshaled tcp_write + output). */
bool det_client_send(int cid, const void *data, size_t len);

/** @brief Wire bytes currently buffered and ready to read. */
size_t det_client_available(int cid);

/** @brief Drain up to @p cap buffered wire bytes into @p buf; returns the count. */
size_t det_client_read(int cid, uint8_t *buf, size_t cap);

/** @brief Tear down the connection (marshaled) and return the slot to the pool. */
void det_client_close(int cid);

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_CLIENT_H
