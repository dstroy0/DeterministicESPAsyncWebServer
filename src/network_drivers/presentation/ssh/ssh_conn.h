// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_conn.h
 * @brief Glue between the TCP transport (conn_pool) and the SSH protocol stack.
 *
 * Binds a PROTO_SSH TcpConn slot to an SSH session slot, pumps ring-buffer
 * bytes through the banner exchange and binary-packet layer, and writes the
 * dispatcher's outbound packets back to the socket. This is the integration
 * layer the session loop calls for PROTO_SSH connections.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_CONN_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_CONN_H

#include <stdint.h>

/**
 * @brief One-time setup: install the dispatcher emit callback. Call from begin().
 */
void ssh_conn_setup();

/**
 * @brief Handle a new PROTO_SSH connection on @p conn_slot.
 *
 * Allocates an SSH session slot, initializes the transport/packet/channel
 * state, and sends the server identification banner. If no SSH slot is free
 * the connection is aborted.
 */
void ssh_conn_accept(uint8_t conn_slot);

/**
 * @brief Drain @p conn_slot's receive ring buffer through the SSH stack.
 *
 * Feeds the banner parser until the client identification string completes,
 * then the binary-packet layer; complete messages are dispatched and any
 * responses are written to the socket. Closes the connection if the protocol
 * signals a fatal condition.
 */
void ssh_conn_rx(uint8_t conn_slot);

/**
 * @brief Tear down SSH state for @p conn_slot (disconnect / error).
 */
void ssh_conn_close(uint8_t conn_slot);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_CONN_H
