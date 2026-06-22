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
        ws_pool[i] = {};
        ws_pool[i].ws_id = (uint8_t)i;
    }
}

WsConn *ws_alloc(uint8_t slot_id)
{
    for (int i = 0; i < MAX_WS_CONNS; i++)
    {
        if (!ws_pool[i].active)
        {
            ws_pool[i] = {};
            ws_pool[i].ws_id = (uint8_t)i;
            ws_pool[i].slot_id = slot_id;
            ws_pool[i].active = true;
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

// Reset only the per-frame parser fields, preserving any in-progress
// fragmented-message state (msg_len/msg_opcode/fragmenting/buf). Used to
// resume reading the next frame after handling an interleaved control frame.
static void ws_reset_perframe(WsConn *ws)
{
    ws->parse_state = WS_HEADER1;
    ws->opcode = WS_OP_TEXT;
    ws->fin = false;
    ws->masked = false;
    ws->payload_len = 0;
    ws->payload_idx = 0;
    ws->len64_count = 0;
    ws->mask_key[0] = ws->mask_key[1] = ws->mask_key[2] = ws->mask_key[3] = 0;
}

void ws_reset_frame(WsConn *ws)
{
    ws_reset_perframe(ws);
    // Also clear reassembly state - a full reset between messages.
    ws->fragmenting = false;
    ws->msg_opcode = WS_OP_TEXT;
    ws->msg_len = 0;
    ws->buf[0] = '\0';
    ws->ctl_buf[0] = '\0';
}

// ---------------------------------------------------------------------------
// Frame send helpers
// ---------------------------------------------------------------------------

bool ws_send_frame(WsConn *ws, WsOpcode opcode, const uint8_t *payload, uint16_t len)
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
    uint8_t payload[2] = {(uint8_t)((uint16_t)code >> 8), (uint8_t)code};
    ws_send_frame(ws, WS_OP_CLOSE, payload, 2);

    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->pcb)
        tcp_output(conn->pcb);

    ws->parse_state = WS_CLOSED;
}

// ---------------------------------------------------------------------------
// Frame parser
// ---------------------------------------------------------------------------

// RFC 6455 §5.5: opcodes 0x8 (close), 0x9 (ping), 0xA (pong) are control frames.
static inline bool ws_is_control(WsOpcode op)
{
    return ((uint8_t)op & 0x08) != 0;
}

// Called once a frame's full payload has been received (payload_idx ==
// payload_len, also true immediately for zero-length frames once the masking
// key is consumed).  Control frames are handled in place; data frames are
// reassembled and delivered as WS_FRAME_READY only when the FIN frame arrives.
static void ws_finish_frame(WsConn *ws, TcpConn *conn)
{
    // ---- Control frames (ping/pong/close): use the separate ctl_buf ----
    if (ws_is_control(ws->opcode))
    {
        size_t n = ws->payload_idx < sizeof(ws->ctl_buf) - 1 ? ws->payload_idx : sizeof(ws->ctl_buf) - 1;
        ws->ctl_buf[n] = '\0';

        if (ws->opcode == WS_OP_PING)
        {
            ws_send_frame(ws, WS_OP_PONG, ws->ctl_buf, (uint16_t)ws->payload_idx);
            if (conn->pcb)
                tcp_output(conn->pcb);
        }
        else if (ws->opcode == WS_OP_CLOSE)
        {
            ws_close(ws, WS_CLOSE_NORMAL);
            return;
        }
        // PONG: silently ignored.

        // Resume reading the next frame, keeping any partial data message.
        ws_reset_perframe(ws);
        return;
    }

    // ---- Data frames (text/binary/continuation): reassemble into buf ----
    ws->msg_len += ws->payload_idx;

    if (ws->fin)
    {
        // Whole message received - surface it to the application.
        size_t n = ws->msg_len < WS_FRAME_SIZE ? ws->msg_len : WS_FRAME_SIZE;
        ws->buf[n] = '\0';
        ws->opcode = ws->msg_opcode;   // report the original TEXT/BINARY opcode
        ws->payload_len = ws->msg_len; // app reads payload_len / payload_idx
        ws->payload_idx = ws->msg_len;
        ws->fragmenting = false;
        ws->parse_state = WS_FRAME_READY;
    }
    else
    {
        // More fragments to come; keep buf and msg_len, read the next frame.
        ws->fragmenting = true;
        ws_reset_perframe(ws);
    }
}

void ws_parse(WsConn *ws)
{
    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->state != CONN_ACTIVE)
        return;

    while (conn->rx_tail != conn->rx_head)
    {
        // Stop if we hit a terminal state
        if (ws->parse_state == WS_FRAME_READY || ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
            return;

        uint8_t byte = conn->rx_buffer[conn->rx_tail];
        conn->rx_tail = (conn->rx_tail + 1) % RX_BUF_SIZE;

        switch (ws->parse_state)
        {
        case WS_HEADER1:
            ws->fin = (byte & 0x80) != 0;
            // RSV1-3 must be zero (no extensions)
            if (byte & 0x70)
            {
                ws_close(ws, WS_CLOSE_PROTOCOL);
                ws->parse_state = WS_ERROR;
                return;
            }
            ws->opcode = (WsOpcode)(byte & 0x0F);
            // RFC 6455 §5.2: only opcodes 0x0/0x1/0x2 (data) and 0x8/0x9/0xA
            // (control) are defined; everything else MUST fail the connection.
            switch (ws->opcode)
            {
            case WS_OP_CONTINUATION:
            case WS_OP_TEXT:
            case WS_OP_BINARY:
            case WS_OP_CLOSE:
            case WS_OP_PING:
            case WS_OP_PONG:
                break;
            default:
                ws_close(ws, WS_CLOSE_PROTOCOL);
                ws->parse_state = WS_ERROR;
                return;
            }
            // RFC 6455 §5.5: control frames MUST NOT be fragmented (FIN set).
            if (ws_is_control(ws->opcode) && !ws->fin)
            {
                ws_close(ws, WS_CLOSE_PROTOCOL);
                ws->parse_state = WS_ERROR;
                return;
            }
            // RFC 6455 §5.4: fragmentation sequencing for data frames.
            if (!ws_is_control(ws->opcode))
            {
                if (ws->opcode == WS_OP_CONTINUATION)
                {
                    // A continuation with no message in progress is illegal.
                    if (!ws->fragmenting)
                    {
                        ws_close(ws, WS_CLOSE_PROTOCOL);
                        ws->parse_state = WS_ERROR;
                        return;
                    }
                }
                else
                {
                    // A new text/binary frame while a message is still open is
                    // illegal - the previous message must finish first.
                    if (ws->fragmenting)
                    {
                        ws_close(ws, WS_CLOSE_PROTOCOL);
                        ws->parse_state = WS_ERROR;
                        return;
                    }
                    // Start of a new data message.
                    ws->msg_opcode = ws->opcode;
                    ws->msg_len = 0;
                }
            }
            ws->parse_state = WS_HEADER2;
            break;

        case WS_HEADER2:
            ws->masked = (byte & 0x80) != 0;
            // RFC 6455 §5.1: every client-to-server frame MUST be masked.
            if (!ws->masked)
            {
                ws_close(ws, WS_CLOSE_PROTOCOL);
                ws->parse_state = WS_ERROR;
                return;
            }
            {
                uint8_t len7 = byte & 0x7F;
                // RFC 6455 §5.5: control frames MUST have payload length <= 125.
                if (ws_is_control(ws->opcode) && len7 > 125)
                {
                    ws_close(ws, WS_CLOSE_PROTOCOL);
                    ws->parse_state = WS_ERROR;
                    return;
                }
                if (len7 <= 125)
                {
                    // Masking is mandatory, so always consume the 4 mask bytes
                    // next - even for zero-length frames (WS_MASK3 finishes them).
                    ws->payload_len = len7;
                    // Reassembled data message must fit in WS_FRAME_SIZE.
                    if (!ws_is_control(ws->opcode) && ws->msg_len + ws->payload_len > WS_FRAME_SIZE)
                    {
                        ws_close(ws, WS_CLOSE_TOO_BIG);
                        ws->parse_state = WS_ERROR;
                        return;
                    }
                    ws->parse_state = WS_MASK0;
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
            // 16-bit length only occurs on data frames (control frames are
            // capped at 125); the reassembled message must fit WS_FRAME_SIZE.
            if (ws->msg_len + ws->payload_len > WS_FRAME_SIZE)
            {
                ws_close(ws, WS_CLOSE_TOO_BIG);
                ws->parse_state = WS_ERROR;
                return;
            }
            // Masking is mandatory; consume the 4 mask bytes next.
            ws->parse_state = WS_MASK0;
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

        case WS_MASK0:
            ws->mask_key[0] = byte;
            ws->parse_state = WS_MASK1;
            break;
        case WS_MASK1:
            ws->mask_key[1] = byte;
            ws->parse_state = WS_MASK2;
            break;
        case WS_MASK2:
            ws->mask_key[2] = byte;
            ws->parse_state = WS_MASK3;
            break;
        case WS_MASK3:
            ws->mask_key[3] = byte;
            if (ws->payload_len > 0)
                ws->parse_state = WS_PAYLOAD;
            else
                ws_finish_frame(ws, conn); // zero-length frame is complete now
            break;

        case WS_PAYLOAD: {
            // Mask is applied per frame, so the keystream index is the
            // within-frame position.
            uint8_t unmasked = byte ^ ws->mask_key[ws->payload_idx % 4];
            if (ws_is_control(ws->opcode))
            {
                // Control payload goes to its own buffer so it never disturbs
                // a partially-assembled data message.
                if (ws->payload_idx < sizeof(ws->ctl_buf) - 1)
                    ws->ctl_buf[ws->payload_idx] = unmasked;
            }
            else
            {
                // Data payload appends after any earlier fragments.
                uint32_t pos = ws->msg_len + ws->payload_idx;
                if (pos < WS_FRAME_SIZE)
                    ws->buf[pos] = unmasked;
            }
            ws->payload_idx++;

            if (ws->payload_idx >= ws->payload_len)
                ws_finish_frame(ws, conn);
            break;
        }

        default:
            break;
        }
    }
}
