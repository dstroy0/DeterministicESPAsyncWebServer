// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file proto_builtins.cpp
 * @brief Layer 5 (Session) - the built-in protocol registry (policy, not mechanism).
 *
 * The session dispatcher (session.cpp) is pure mechanism: it routes events to whatever
 * ProtoHandler is registered for a connection's ConnProto and names no protocol. This
 * file is the one place that knows which protocols the build includes. Each built-in's
 * handler + callbacks live in its own module and are exposed by a pure `*_proto_handler()`
 * accessor (no session dependency); here we install each behind the matching feature flag.
 * Adding a protocol means writing its module and adding one guarded line here - never editing
 * the dispatcher.
 *
 * (The SSH remote-forward listener, PROTO_SSH_RFWD, is intentionally NOT here: it is a
 * runtime opt-in that self-registers from ssh_forward_begin().)
 */

#include "proto_handler.h"

#include "network_drivers/presentation/presentation.h" // http_proto_handler()
#if DETWS_ENABLE_TELNET
#include "network_drivers/presentation/telnet/telnet.h"
#endif
#if DETWS_ENABLE_SSH
#include "network_drivers/presentation/ssh/connection/ssh_conn.h"
#endif
#if DETWS_ENABLE_MODBUS
#include "services/modbus/modbus.h"
#endif
#if DETWS_ENABLE_OPCUA
#include "services/opcua/opcua.h"
#endif

// Register @p h for @p proto only if the module actually supplied one (modbus / opcua return nullptr
// on host builds, where there is no TCP transport handler).
static inline void register_if(ConnProto proto, const ProtoHandler *h)
{
    if (h)
        proto_register(proto, h);
}

void proto_register_builtins(void)
{
    register_if(PROTO_HTTP, http_proto_handler()); // always present (the core request/response protocol)
#if DETWS_ENABLE_TELNET
    register_if(PROTO_TELNET, telnet_proto_handler());
#endif
#if DETWS_ENABLE_SSH
    register_if(PROTO_SSH, ssh_proto_handler());
#endif
#if DETWS_ENABLE_MODBUS
    register_if(PROTO_MODBUS, modbus_proto_handler());
#endif
#if DETWS_ENABLE_OPCUA
    register_if(PROTO_OPCUA, opcua_proto_handler());
#endif
}
