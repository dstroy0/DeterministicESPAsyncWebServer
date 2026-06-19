// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file session.cpp
 * @brief Layer 5 (Session) — event queue processor implementation.
 *
 * server_tick() is the only function here.  Its bounded loop drains the
 * entire FreeRTOS queue in one call so that the application layer always
 * sees the most up-to-date state before checking http_pool[].
 */

#include "session.h"

void server_tick()
{
    /*
     * Check timeouts BEFORE draining events.  This ensures that a slot
     * freed by a timeout is already in CONN_FREE state if a coincident
     * EVT_DISCONNECT or EVT_ERROR is dequeued in the same tick — the
     * http_reset() call for that event is then a clean no-op.
     */
    DeterministicAsyncTCP::check_timeouts();

    TcpEvt evt;
    while (xQueueReceive(DeterministicAsyncTCP::queue, &evt, 0) == pdTRUE)
    {
        switch (evt.type)
        {
        case EVT_CONNECT:
        case EVT_DISCONNECT:
        case EVT_ERROR:
            /* Reset the HTTP parser so the slot is ready for its next request. */
            http_reset(evt.slot_id);
            break;

        case EVT_DATA:
            /* Advance the parser; may transition slot to PARSE_COMPLETE. */
            http_parse(evt.slot_id);
            break;
        }
    }
}
