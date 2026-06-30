// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.cpp
 * @brief Layer 6 (Presentation) - wires the transport ring buffer to the HTTP parser.
 *
 * This file is now a thin adapter.  The HTTP parsing logic lives in
 * http_parser.cpp.  This layer only knows about:
 *   - The transport RX read API (det_conn_available / det_conn_read_byte) - it
 *     never indexes the ring itself; transport owns rx_buffer / rx_head / rx_tail
 *   - Slot-indexed helpers that the session and application layers expect
 *
 * The slot-indexed `http_reset()` pre-stamps slot_id then delegates to
 * `http_parser_reset()`.  The slot-indexed `http_parse()` drains all
 * available bytes from the ring buffer through `http_parser_feed()` and
 * stops as soon as the parser reaches any terminal state.
 */

#include "network_drivers/presentation/presentation.h"

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

    HttpReq *req = &http_pool[slot_id];

    // Drain via the transport read API - the parser never touches the ring itself.
    // Check the terminal state BEFORE consuming so a pipelined next request is left
    // in the ring; the window is reopened by the worker's det_conn_ack_consumed().
    while (det_conn_available(slot_id) > 0)
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

        uint8_t byte = 0;
        if (!det_conn_read_byte(slot_id, &byte)) // ring drained between available() and here
            break;
        http_parser_feed(req, byte);
    }
}
