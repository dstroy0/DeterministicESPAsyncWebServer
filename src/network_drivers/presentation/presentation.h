// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.h
 * @brief Layer 6 (Presentation) - wires the transport ring buffer to the HTTP parser.
 *
 * This layer owns two responsibilities:
 *   1. Drain bytes from `conn_pool[slot_id].rx_buffer` (transport ring buffer)
 *      and feed them into the HTTP parser one at a time.
 *   2. Expose slot-indexed `http_reset()` and `http_parse()` helpers that the
 *      session layer (server_tick) and application layer (handle) call by slot ID.
 *
 * The actual HTTP parsing logic lives in `http_parser.h / http_parser.cpp`.
 * This file merely includes it so callers only need one include.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PRESENTATION_H
#define DETERMINISTICESPASYNCWEBSERVER_PRESENTATION_H

#include "../transport/transport.h"
#include "http_parser.h"

// ---------------------------------------------------------------------------
// Slot-indexed wrappers called by the session and application layers
// ---------------------------------------------------------------------------

/**
 * @brief Reset the HTTP parser for a connection slot.
 *
 * Delegates to http_parser_reset(&http_pool[slot_id]).  Silently ignores
 * out-of-range slot IDs.
 *
 * @param slot_id Index into conn_pool / http_pool (0 … MAX_CONNS-1).
 */
void http_reset(uint8_t slot_id);

/**
 * @brief Drain the transport ring buffer and advance the HTTP parser.
 *
 * Reads all available bytes from `conn_pool[slot_id].rx_buffer` and feeds
 * each byte to `http_parser_feed()`.  Stops early if the parser reaches a
 * terminal state (PARSE_COMPLETE, PARSE_ERROR, PARSE_ENTITY_TOO_LARGE,
 * PARSE_URI_TOO_LONG).
 *
 * Silently ignores out-of-range slot IDs.
 *
 * @param slot_id Connection slot to parse.
 */
void http_parse(uint8_t slot_id);

#endif
