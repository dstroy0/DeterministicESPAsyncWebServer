// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file session.cpp
 * @brief Layer 5 (Session) - event queue processor implementation.
 *
 * server_tick() is the only function here.  Its bounded loop drains every
 * active listener's FreeRTOS queue in one call so that the application layer
 * always sees the most up-to-date state before checking http_pool[].
 *
 * Events are routed to the correct protocol handler via TcpConn::proto.
 * PROTO_NONE falls through to PROTO_HTTP for backward compatibility with
 * test code that sets up conn_pool slots manually without setting proto.
 */

#include "session.h"
#include "../transport/listener.h"
#if DETWS_ENABLE_SSH
#include "../presentation/ssh/ssh_conn.h"
#endif

void server_tick()
{
    /*
     * Check timeouts BEFORE draining events.  This ensures that a slot
     * freed by a timeout is already in CONN_FREE state if a coincident
     * EVT_DISCONNECT or EVT_ERROR is dequeued in the same tick - the
     * http_reset() call for that event is then a clean no-op.
     */
    DeterministicAsyncTCP::check_timeouts();

    for (uint8_t li = 0; li < MAX_LISTENERS; li++)
    {
        Listener *lst = &listener_pool[li];
        if (!lst->active || !lst->queue)
            continue;

        TcpEvt evt;
        while (xQueueReceive(lst->queue, &evt, 0) == pdTRUE)
        {
            ConnProto proto = conn_pool[evt.slot_id].proto;

            switch (evt.type)
            {
            case EVT_CONNECT:
                switch (proto)
                {
                case PROTO_NONE: /* fallthrough - treat unset slots as HTTP */
                case PROTO_HTTP:
                    http_reset(evt.slot_id);
                    break;
                case PROTO_TELNET:
                    break;
                case PROTO_SSH:
#if DETWS_ENABLE_SSH
                    ssh_conn_accept(evt.slot_id);
#endif
                    break;
                }
                break;

            case EVT_DISCONNECT:
            case EVT_ERROR:
                switch (proto)
                {
                case PROTO_NONE:
                case PROTO_HTTP:
                    http_reset(evt.slot_id);
                    break;
                case PROTO_TELNET:
                    break;
                case PROTO_SSH:
#if DETWS_ENABLE_SSH
                    ssh_conn_close(evt.slot_id);
#endif
                    break;
                }
                break;

            case EVT_DATA:
                switch (proto)
                {
                case PROTO_NONE:
                case PROTO_HTTP:
                    http_parse(evt.slot_id);
                    break;
                case PROTO_TELNET:
                    break;
                case PROTO_SSH:
#if DETWS_ENABLE_SSH
                    ssh_conn_rx(evt.slot_id);
#endif
                    break;
                }
                break;
            }
        }
    }
}
