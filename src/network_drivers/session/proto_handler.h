// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file proto_handler.h
 * @brief Layer 5 (Session) - per-protocol connection handler dispatch table.
 *
 * Every application protocol (HTTP, Telnet, SSH, and optional services such as
 * MQTT or Modbus) registers one ProtoHandler. The session layer (server_tick)
 * routes each connection event - and the main loop (DetWebServer::handle())
 * polls each active slot - through this table by ConnProto, so a new protocol
 * plugs in by registering a handler instead of editing the dispatchers.
 *
 * All callbacks are nullable, run on the main-loop task, and take the affected
 * connection slot index. The built-in HTTP/Telnet/SSH handlers are registered
 * lazily on first lookup, so dispatch works even before begin() (the native
 * test harness drives server_tick() directly).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PROTO_HANDLER_H
#define DETERMINISTICESPASYNCWEBSERVER_PROTO_HANDLER_H

#include "DetWebServerConfig.h"
#include "shared_primitives/shim.h"

/**
 * @brief Per-protocol connection event/poll callbacks (Layer 5 dispatch vtable).
 */
struct ProtoHandler
{
    void (*on_accept)(uint8_t slot); ///< EVT_CONNECT: a new connection was accepted.
    void (*on_data)(uint8_t slot);   ///< EVT_DATA: bytes are available in the slot's rx ring.
    void (*on_close)(uint8_t slot);  ///< EVT_DISCONNECT / EVT_ERROR: tear down slot state.
    void (*on_poll)(uint8_t slot);   ///< Called for an active slot each handle() loop (nullable).
};

/** @brief Largest ConnProto id the table holds (leaves room for optional protocols). */
#define DETWS_PROTO_MAX 8

/** @brief Register @p h for protocol @p proto (replaces any previous handler). */
void proto_register(ConnProto proto, const ProtoHandler *h);

/**
 * @brief Look up the handler for @p proto.
 * @return the registered handler, or nullptr if @p proto is PROTO_NONE or has
 *         no registered handler (no implicit fallback; the event is dropped).
 */
const ProtoHandler *proto_get(ConnProto proto);

#endif // DETERMINISTICESPASYNCWEBSERVER_PROTO_HANDLER_H
