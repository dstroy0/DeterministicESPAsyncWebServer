// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.cpp
 * @brief Layer 6 (Presentation) - wires the transport ring buffer to the HTTP parser.
 *
 * This file is now a thin adapter.  The HTTP parsing logic lives in
 * http_parser.cpp.  This layer only knows about:
 *   - The transport ring buffer (conn_pool[slot].rx_buffer / rx_head / rx_tail)
 *   - Slot-indexed helpers that the session and application layers expect
 *
 * The slot-indexed `http_reset()` pre-stamps slot_id then delegates to
 * `http_parser_reset()`.  The slot-indexed `http_parse()` drains all
 * available bytes from the ring buffer through `http_parser_feed()` and
 * stops as soon as the parser reaches any terminal state.
 */

#include "presentation.h"
#if DETWS_ENABLE_WEBSOCKET
#include "websocket.h" // ws_find(): a WS-upgraded slot must never be HTTP-parsed
#endif

#if DETWS_ENABLE_KEEPALIVE
uint16_t http_req_count[MAX_CONNS];
#endif

void http_reset(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;
    http_pool[slot_id].slot_id = slot_id; // ensure slot_id is correct before reset reads it
    http_parser_reset(&http_pool[slot_id]);
}

void http_conn_open(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;
#if DETWS_ENABLE_KEEPALIVE
    http_req_count[slot_id] = 0; // fresh connection: clear the keep-alive request tally
#endif
    http_reset(slot_id);
}

void http_parse(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;

#if DETWS_ENABLE_WEBSOCKET
    // Once a slot upgrades to WebSocket its rx ring carries WS frames, not HTTP.
    // The WS frame parser is pumped separately (handle()/the worker loop); feeding
    // those bytes to the HTTP parser here would consume - and corrupt - the first
    // WS frame. This guard makes "never HTTP-parse a WS slot" hold for every caller
    // (the event-queue dispatch raced the WS pump and ate the first frame's header
    // byte, dropping the first connection after a reboot).
    if (ws_find(slot_id))
        return;
#endif

    TcpConn *tcp = &conn_pool[slot_id];
    HttpReq *req = &http_pool[slot_id];

    while (tcp->rx_tail != tcp->rx_head)
    {
        switch (req->parse_state)
        {
        case PARSE_COMPLETE:
        case PARSE_ERROR:
        case PARSE_ENTITY_TOO_LARGE:
        case PARSE_URI_TOO_LONG:
            return; // terminal state - drain nothing further
        default:
            break;
        }

        uint8_t byte = tcp->rx_buffer[tcp->rx_tail];
        tcp->rx_tail = (tcp->rx_tail + 1) % RX_BUF_SIZE;
        http_parser_feed(req, byte);
    }
}
