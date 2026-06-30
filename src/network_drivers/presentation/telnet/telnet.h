// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telnet.h
 * @brief Layer 6/7 - minimal RFC 854 Telnet server (DETWS_ENABLE_TELNET).
 *
 * A zero-heap line-oriented Telnet console dispatched from the session layer's
 * PROTO_TELNET arms (the same way SSH is dispatched to ssh_conn). On connect it
 * negotiates server-side echo + suppress-go-ahead (so the client runs in
 * character mode and the server draws the line), accumulates a line, echoes
 * keystrokes (with backspace handling), and hands each completed line to a
 * command callback. Output can be pushed to all connected clients.
 *
 * Telnet is plaintext - no authentication or encryption. Use it only on a
 * trusted network; prefer SSH or the WebSocket terminal otherwise.
 *
 * Usage:
 * @code
 *   server.listen(23, PROTO_TELNET);     // open the Telnet port
 *   telnet_on_command(my_cmd_handler);   // void(const char *line, uint8_t id)
 * @endcode
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TELNET_H
#define DETERMINISTICESPASYNCWEBSERVER_TELNET_H

#include "shared_primitives/shim.h"

#if DETWS_ENABLE_TELNET

/** @brief Called with each completed input line (NUL-terminated, no CR/LF) and its client id. */
typedef void (*TelnetCommandCb)(const char *line, uint8_t conn_id);

// ---- Application API ------------------------------------------------------

/** @brief Register the per-line command handler. */
void telnet_on_command(TelnetCommandCb cb);

/** @brief Send text to every connected Telnet client (no trailing newline added). */
void telnet_print(const char *s);

/** @brief Send text + CRLF to every connected Telnet client. */
void telnet_println(const char *s);

/** @brief printf-style broadcast to every connected Telnet client (bounded by TELNET_BUF_SIZE). */
void telnet_printf(const char *fmt, ...);

/** @brief Number of connected Telnet clients. */
uint8_t telnet_client_count();

// ---- Connection layer (called by the session layer for PROTO_TELNET slots) -

/** @brief A Telnet connection was accepted on TCP slot @p slot. */
void telnet_accept(uint8_t slot);

/** @brief Drain and process received bytes for the Telnet connection on @p slot. */
void telnet_rx(uint8_t slot);

/** @brief The Telnet connection on @p slot closed; release its state. */
void telnet_close(uint8_t slot);

#endif // DETWS_ENABLE_TELNET

#endif // DETERMINISTICESPASYNCWEBSERVER_TELNET_H
