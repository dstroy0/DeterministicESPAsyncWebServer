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
#if DETWS_ENABLE_TELNET
#include "../presentation/telnet.h"
#endif

#if DETWS_ENABLE_TLS
#include "../presentation/http_parser.h"
#include "../tls/det_tls.h"

// Abort a TLS connection (fatal handshake/read error): free the TLS context and
// tear down the TCP slot (through the transport-layer connection API).
static void tls_abort(uint8_t slot)
{
    TcpConn *c = &conn_pool[slot];
    det_tls_conn_free(slot);
    if (c->pcb)
    {
        struct tcp_pcb *p = c->pcb;
        det_conn_detach(p);
        c->state = CONN_FREE;
        c->pcb = nullptr;
        det_conn_abort(p);
    }
    http_reset(slot);
}

// Pump a TLS connection: drive the handshake to completion, then decrypt any
// application data straight into the HTTP parser (same byte-by-byte feed the
// plaintext path uses; the rx ring now holds ciphertext, consumed by the BIO).
static void tls_data(uint8_t slot)
{
    if (!det_tls_established(slot))
    {
        int h = det_tls_handshake(slot);
        if (h < 0)
        {
            tls_abort(slot);
            return;
        }
        if (h == 0)
            return; // still handshaking; wait for more ciphertext
    }

    uint8_t buf[256];
    int n;
    while ((n = det_tls_read(slot, buf, sizeof(buf))) > 0)
    {
        HttpReq *req = &http_pool[slot];
        for (int i = 0; i < n; i++)
        {
            if (req->parse_state == PARSE_COMPLETE || req->parse_state == PARSE_ERROR ||
                req->parse_state == PARSE_ENTITY_TOO_LARGE || req->parse_state == PARSE_URI_TOO_LONG)
                break; // terminal state - let handle() dispatch before reading more
            http_parser_feed(req, buf[i]);
        }
    }
    if (n < 0)
        tls_abort(slot);
}
#endif // DETWS_ENABLE_TLS

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
#if DETWS_ENABLE_TELNET
                    telnet_accept(evt.slot_id);
#endif
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
#if DETWS_ENABLE_TLS
                    if (conn_pool[evt.slot_id].tls)
                        det_tls_conn_free(evt.slot_id); // also covers timeouts (EVT_ERROR)
#endif
                    http_reset(evt.slot_id);
                    break;
                case PROTO_TELNET:
#if DETWS_ENABLE_TELNET
                    telnet_close(evt.slot_id);
#endif
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
#if DETWS_ENABLE_TLS
                    if (conn_pool[evt.slot_id].tls)
                    {
                        tls_data(evt.slot_id);
                        break;
                    }
#endif
                    http_parse(evt.slot_id);
                    break;
                case PROTO_TELNET:
#if DETWS_ENABLE_TELNET
                    telnet_rx(evt.slot_id);
#endif
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
