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

#include <stddef.h>
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

/**
 * @brief Send application data to the client over an SSH channel.
 *
 * Frames @p data as SSH_MSG_CHANNEL_DATA on channel @p channel, encrypts+MACs it,
 * and writes it to the socket. @p ssh_slot and @p channel are the values passed to
 * the data callback registered via ssh_channel_set_data_cb(). A single call sends
 * at most one channel-data message (bounded by the peer's flow-control window).
 *
 * @return Number of bytes sent, or -1 on error (bad slot, channel closed/unknown,
 *         peer window/packet limit, or no active connection).
 */
int ssh_conn_send(uint8_t ssh_slot, uint32_t channel, const uint8_t *data, size_t len);

/**
 * @brief Close an SSH channel from the server side: frame CHANNEL_EOF and
 *        CHANNEL_CLOSE as two binary packets and write them to the socket.
 *
 * Used by the port-forwarding owner when the forwarded TCP peer closes.
 * @return 0 on success, -1 on error (bad slot, channel closed/unknown, no
 *         active connection, or scratch exhausted).
 */
int ssh_conn_close_channel(uint8_t ssh_slot, uint32_t channel);

/**
 * @brief Open a server-initiated "forwarded-tcpip" channel to the client (ssh -R):
 *        build the CHANNEL_OPEN (RFC 4254 §7.2) via the channel codec, frame + send it
 *        on @p ssh_slot's socket, and return the new local channel id. The client's
 *        CHANNEL_OPEN_CONFIRMATION (or FAILURE) later drives the forward-confirm callback.
 *
 * @param conn_addr / conn_port  the forward's bound address/port (address connected).
 * @param orig_addr / orig_port  the peer that connected to the forwarded port (advisory).
 * @return the local channel id (>= 0), or -1 (no active connection, channel pool full,
 *         or scratch exhausted). Used by the remote-forward owner.
 */
int ssh_conn_open_forwarded(uint8_t ssh_slot, const char *conn_addr, uint16_t conn_port, const char *orig_addr,
                            uint16_t orig_port);

/**
 * @brief Per-loop poll hook for an SSH connection (registered as the SSH protocol
 *        handler's on_poll). Drives the port-forwarding pump; a no-op when
 *        forwarding is compiled out.
 */
void ssh_conn_poll(uint8_t conn_slot);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_CONN_H
