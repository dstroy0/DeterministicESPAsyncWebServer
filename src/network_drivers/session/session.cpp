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
#include "proto_handler.h"
#include "scratch.h"
#if DETWS_ENABLE_SSH
#include "../presentation/ssh/ssh_conn.h"
#endif
#if DETWS_ENABLE_TELNET
#include "../presentation/telnet.h"
#endif
#if DETWS_ENABLE_MODBUS
#include "services/modbus/modbus.h"
#endif

#if DETWS_ENABLE_TLS
#include "../presentation/http_parser.h"
#include "../tls/det_tls.h"
#if DETWS_ENABLE_WEBSOCKET
#include "../presentation/websocket.h"
#endif

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

#if DETWS_ENABLE_WEBSOCKET
    // A TLS slot upgraded to WebSocket is pumped from handle() (it decrypts
    // records and feeds the WS frame parser, dispatching each frame); leave the
    // ciphertext in the rx ring for it rather than feeding the HTTP parser here.
    if (ws_find(slot))
        return;
#endif

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

// ---------------------------------------------------------------------------
// Protocol-handler dispatch table (see proto_handler.h)
// ---------------------------------------------------------------------------
static const ProtoHandler *s_proto_handlers[DETWS_PROTO_MAX];

void proto_register(ConnProto proto, const ProtoHandler *h)
{
    if ((unsigned)proto < DETWS_PROTO_MAX)
        s_proto_handlers[proto] = h;
}

// Built-in HTTP handler. The data/close paths branch on TLS (a TLS slot's rx
// ring holds ciphertext, decrypted into the parser); accept maps directly.
static void http_evt_accept(uint8_t slot)
{
    http_conn_open(slot); // resets the parser + (keep-alive) the per-conn request tally
}
static void http_evt_data(uint8_t slot)
{
#if DETWS_ENABLE_TLS
    if (conn_pool[slot].tls)
    {
        tls_data(slot);
        return;
    }
#endif
    http_parse(slot); // a no-op once the slot has upgraded to WebSocket (see http_parse)
}
static void http_evt_close(uint8_t slot)
{
#if DETWS_ENABLE_TLS
    if (conn_pool[slot].tls)
        det_tls_conn_free(slot); // also covers timeouts (EVT_ERROR)
#endif
    http_reset(slot);
}
static const ProtoHandler s_http_handler = {http_evt_accept, http_evt_data, http_evt_close, nullptr};

#if DETWS_ENABLE_TELNET
static const ProtoHandler s_telnet_handler = {telnet_accept, telnet_rx, telnet_close, nullptr};
#endif
#if DETWS_ENABLE_SSH
static const ProtoHandler s_ssh_handler = {ssh_conn_accept, ssh_conn_rx, ssh_conn_close, nullptr};
#endif
#if DETWS_ENABLE_MODBUS
// Modbus keeps no per-connection state (a partial frame waits in the rx ring),
// so only the data handler is needed.
static const ProtoHandler s_modbus_handler = {nullptr, modbus_rx, nullptr, nullptr};
#endif

const ProtoHandler *proto_get(ConnProto proto)
{
    // Lazily register the built-ins on first lookup so dispatch works before
    // begin() (the native test harness drives server_tick() directly). Keyed on
    // the always-present HTTP slot; new protocols add themselves via proto_register.
    if (!s_proto_handlers[PROTO_HTTP])
    {
        s_proto_handlers[PROTO_HTTP] = &s_http_handler;
#if DETWS_ENABLE_TELNET
        s_proto_handlers[PROTO_TELNET] = &s_telnet_handler;
#endif
#if DETWS_ENABLE_SSH
        s_proto_handlers[PROTO_SSH] = &s_ssh_handler;
#endif
#if DETWS_ENABLE_MODBUS
        s_proto_handlers[PROTO_MODBUS] = &s_modbus_handler;
#endif
    }
    const ProtoHandler *h = ((unsigned)proto < DETWS_PROTO_MAX) ? s_proto_handlers[proto] : nullptr;
    if (!h)
        h = s_proto_handlers[PROTO_HTTP]; // PROTO_NONE / unregistered -> HTTP
    return h;
}

// Dispatch one drained event to its slot's protocol handler. Shared by the
// single-queue (N=1) and per-worker-queue (N>1) drain paths below.
static inline void dispatch_event(const TcpEvt &evt)
{
    // Per-dispatch reset of the calling worker's scratch arena: every handler
    // runs with the whole arena available, and any scratch it borrows is
    // reclaimed before the next event - the backstop that stops a forgotten
    // release from accumulating across events.
    scratch_reset();

    // Route to the slot's protocol handler (PROTO_NONE and any unregistered
    // protocol fall back to HTTP, preserving legacy behavior).
    const ProtoHandler *h = proto_get(conn_pool[evt.slot_id].proto);
    if (!h)
        return;

    switch (evt.type)
    {
    case EVT_CONNECT:
        if (h->on_accept)
            h->on_accept(evt.slot_id);
        break;
    case EVT_DATA:
        if (h->on_data)
            h->on_data(evt.slot_id);
        break;
    case EVT_DISCONNECT:
    case EVT_ERROR:
        if (h->on_close)
            h->on_close(evt.slot_id);
        break;
    }
}

void server_tick(int worker_id)
{
    /*
     * Check timeouts BEFORE draining events.  This ensures that a slot
     * freed by a timeout is already in CONN_FREE state if a coincident
     * EVT_DISCONNECT or EVT_ERROR is dequeued in the same tick - the
     * http_reset() call for that event is then a clean no-op. Each worker
     * sweeps only the slots it owns.
     */
    DeterministicAsyncTCP::check_timeouts(worker_id);

#if DETWS_WORKER_COUNT > 1
    // Drain only this worker's queue: it is the sole consumer of its slots.
    QueueHandle_t q = listener_worker_queue(worker_id);
    if (!q)
        return;
    TcpEvt evt;
    while (xQueueReceive(q, &evt, 0) == pdTRUE)
        dispatch_event(evt);
#else
    (void)worker_id; // single worker owns all slots; drain every listener queue
    for (uint8_t li = 0; li < MAX_LISTENERS; li++)
    {
        Listener *lst = &listener_pool[li];
        if (!lst->active || !lst->queue)
            continue;

        TcpEvt evt;
        while (xQueueReceive(lst->queue, &evt, 0) == pdTRUE)
            dispatch_event(evt);
    }
#endif
}
