// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.cpp
 * @brief Layer 6 (Presentation) — wires the transport ring buffer to the HTTP parser.
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

void http_reset(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;
    http_pool[slot_id].slot_id = slot_id; // ensure slot_id is correct before reset reads it
    http_parser_reset(&http_pool[slot_id]);
}

void http_parse(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;

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
            return; // terminal state — drain nothing further
        default:
            break;
        }

        uint8_t byte = tcp->rx_buffer[tcp->rx_tail];
        tcp->rx_tail = (tcp->rx_tail + 1) % RX_BUF_SIZE;
        http_parser_feed(req, byte);
    }
}
