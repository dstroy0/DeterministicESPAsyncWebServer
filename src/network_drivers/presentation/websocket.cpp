// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file websocket.cpp
 * @brief WebSocket frame parser and connection pool implementation.
 *
 * Handles RFC 6455 framing.  Control frames (ping/pong/close) are handled
 * automatically here; data frames (text/binary) are surfaced to the
 * application layer via WS_FRAME_READY.
 *
 * **Automatic control frame handling**
 * - Ping  -> sends Pong with the same payload immediately.
 * - Close -> sends echoed Close frame, marks slot WS_CLOSED.
 * - Pong  -> silently discarded (keepalive response, no action needed).
 */

#include "websocket.h"
#include "lwip/tcp.h"
#include <string.h>

WsConn ws_pool[MAX_WS_CONNS];

void ws_init()
{
    for (int i = 0; i < MAX_WS_CONNS; i++)
    {
        ws_pool[i]       = {};
        ws_pool[i].ws_id = (uint8_t)i;
    }
}

WsConn *ws_alloc(uint8_t slot_id)
{
    for (int i = 0; i < MAX_WS_CONNS; i++)
    {
        if (!ws_pool[i].active)
        {
            ws_pool[i]            = {};
            ws_pool[i].ws_id      = (uint8_t)i;
            ws_pool[i].slot_id    = slot_id;
            ws_pool[i].active     = true;
            ws_pool[i].parse_state = WS_HEADER1;
            return &ws_pool[i];
        }
    }
    return nullptr;
}

WsConn *ws_find(uint8_t slot_id)
{
    for (int i = 0; i < MAX_WS_CONNS; i++)
    {
        if (ws_pool[i].active && ws_pool[i].slot_id == slot_id)
            return &ws_pool[i];
    }
    return nullptr;
}

void ws_free(uint8_t slot_id)
{
    for (int i = 0; i < MAX_WS_CONNS; i++)
    {
        if (ws_pool[i].active && ws_pool[i].slot_id == slot_id)
        {
            ws_pool[i] = {};
            ws_pool[i].ws_id = (uint8_t)i;
            return;
        }
    }
}

void ws_reset_frame(WsConn *ws)
{
    ws->parse_state  = WS_HEADER1;
    ws->opcode       = WS_OP_TEXT;
    ws->fin          = false;
    ws->masked       = false;
    ws->payload_len  = 0;
    ws->payload_idx  = 0;
    ws->len64_count  = 0;
    ws->mask_key[0]  = ws->mask_key[1] = ws->mask_key[2] = ws->mask_key[3] = 0;
    ws->buf[0]       = '\0';
}

// ---------------------------------------------------------------------------
// Frame send helpers
// ---------------------------------------------------------------------------

bool ws_send_frame(WsConn *ws, WsOpcode opcode,
                   const uint8_t *payload, uint16_t len)
{
    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
        return false;

    // Server-to-client frames are never masked (RFC 6455 §5.1)
    uint8_t header[4];
    uint8_t hlen;

    header[0] = 0x80 | (uint8_t)opcode; // FIN=1

    if (len <= 125)
    {
        header[1] = (uint8_t)len;
        hlen = 2;
    }
    else
    {
        header[1] = 126;
        header[2] = (uint8_t)(len >> 8);
        header[3] = (uint8_t)(len);
        hlen = 4;
    }

    tcp_write(conn->pcb, header, hlen, TCP_WRITE_FLAG_COPY);
    if (len > 0 && payload)
        tcp_write(conn->pcb, payload, len, TCP_WRITE_FLAG_COPY);

    return true;
}

void ws_close(WsConn *ws, WsCloseCode code)
{
    // Send Close frame with 2-byte status code payload
    uint8_t payload[2] = { (uint8_t)((uint16_t)code >> 8), (uint8_t)code };
    ws_send_frame(ws, WS_OP_CLOSE, payload, 2);

    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->pcb)
        tcp_output(conn->pcb);

    ws->parse_state = WS_CLOSED;
}

// ---------------------------------------------------------------------------
// Frame parser
// ---------------------------------------------------------------------------

void ws_parse(WsConn *ws)
{
    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->state != CONN_ACTIVE)
        return;

    while (conn->rx_tail != conn->rx_head)
    {
        // Stop if we hit a terminal state
        if (ws->parse_state == WS_FRAME_READY ||
            ws->parse_state == WS_CLOSED      ||
            ws->parse_state == WS_ERROR)
            return;

        uint8_t byte = conn->rx_buffer[conn->rx_tail];
        conn->rx_tail = (conn->rx_tail + 1) % RX_BUF_SIZE;

        switch (ws->parse_state)
        {
        case WS_HEADER1:
            ws->fin    = (byte & 0x80) != 0;
            // RSV1-3 must be zero (no extensions)
            if (byte & 0x70)
            {
                ws_close(ws, WS_CLOSE_PROTOCOL);
                ws->parse_state = WS_ERROR;
                return;
            }
            ws->opcode = (WsOpcode)(byte & 0x0F);
            ws->parse_state = WS_HEADER2;
            break;

        case WS_HEADER2:
            ws->masked = (byte & 0x80) != 0;
            {
                uint8_t len7 = byte & 0x7F;
                if (len7 <= 125)
                {
                    ws->payload_len = len7;
                    ws->parse_state = ws->masked ? WS_MASK0 : WS_PAYLOAD;
                    if (ws->payload_len == 0)
                        ws->parse_state = WS_FRAME_READY;
                }
                else if (len7 == 126)
                {
                    ws->payload_len = 0;
                    ws->parse_state = WS_LEN16_HI;
                }
                else
                {
                    // 64-bit length -- always too large
                    ws->len64_count = 0;
                    ws->parse_state = WS_LEN64;
                }
            }
            break;

        case WS_LEN16_HI:
            ws->payload_len = (uint32_t)byte << 8;
            ws->parse_state = WS_LEN16_LO;
            break;

        case WS_LEN16_LO:
            ws->payload_len |= byte;
            if (ws->payload_len > WS_FRAME_SIZE)
            {
                ws_close(ws, WS_CLOSE_TOO_BIG);
                ws->parse_state = WS_ERROR;
                return;
            }
            ws->parse_state = ws->masked ? WS_MASK0 : WS_PAYLOAD;
            if (ws->payload_len == 0)
                ws->parse_state = WS_FRAME_READY;
            break;

        case WS_LEN64:
            // Consume all 8 bytes then reject
            if (++ws->len64_count == 8)
            {
                ws_close(ws, WS_CLOSE_TOO_BIG);
                ws->parse_state = WS_ERROR;
                return;
            }
            break;

        case WS_MASK0: ws->mask_key[0] = byte; ws->parse_state = WS_MASK1; break;
        case WS_MASK1: ws->mask_key[1] = byte; ws->parse_state = WS_MASK2; break;
        case WS_MASK2: ws->mask_key[2] = byte; ws->parse_state = WS_MASK3; break;
        case WS_MASK3:
            ws->mask_key[3] = byte;
            ws->parse_state = (ws->payload_len > 0) ? WS_PAYLOAD : WS_FRAME_READY;
            break;

        case WS_PAYLOAD:
        {
            uint8_t unmasked = byte ^ ws->mask_key[ws->payload_idx % 4];
            if (ws->payload_idx < WS_FRAME_SIZE)
                ws->buf[ws->payload_idx] = unmasked;
            ws->payload_idx++;

            if (ws->payload_idx >= ws->payload_len)
            {
                ws->buf[ws->payload_idx < WS_FRAME_SIZE
                        ? ws->payload_idx : WS_FRAME_SIZE] = '\0';

                // Handle control frames automatically
                if (ws->opcode == WS_OP_PING)
                {
                    ws_send_frame(ws, WS_OP_PONG,
                                  ws->buf, (uint16_t)ws->payload_idx);
                    if (conn->pcb) tcp_output(conn->pcb);
                    ws_reset_frame(ws);
                }
                else if (ws->opcode == WS_OP_CLOSE)
                {
                    ws_close(ws, WS_CLOSE_NORMAL);
                }
                else if (ws->opcode == WS_OP_PONG)
                {
                    ws_reset_frame(ws);
                }
                else if (ws->opcode == WS_OP_CONTINUATION || !ws->fin)
                {
                    // Fragmentation not supported
                    ws_close(ws, WS_CLOSE_UNSUPPORTED);
                    ws->parse_state = WS_ERROR;
                }
                else
                {
                    ws->parse_state = WS_FRAME_READY;
                }
            }
            break;
        }

        default:
            break;
        }
    }
}
