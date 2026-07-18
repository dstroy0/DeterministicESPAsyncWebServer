// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file websocket_sse.cpp
 * @brief WebSocket (RFC 6455) and Server-Sent Events upgrade + public API for DWS.
 *
 * Split out of dwserver.cpp (single-purpose server files). The WS handshake (Sec-WebSocket-Accept
 * over SHA-1+base64, optional permessage-deflate), the SSE 200 upgrade, and the send/broadcast
 * public API for both. The frame codecs live in the presentation layer (websocket/, sse/); this
 * is the DWS glue. The upgrade entry points are called by the route dispatcher in
 * dwserver.cpp (declared in server/dwserver_internal.h). Behaviour is identical to the pre-split code.
 */

#include "dwserver.h"
#include "network_drivers/transport/tcp.h" // conn_pool, dws_conn_*, TcpConn/ConnState
#include "server/dwserver_internal.h"
#if DWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/base64/base64.h"       // base64_decode/encode
#include "network_drivers/presentation/sha1/sha1.h"           // sha1, SHA1_DIGEST_LEN
#include "network_drivers/presentation/websocket/websocket.h" // ws_pool, WsConn, ws_alloc/send_frame/close
#endif
#if DWS_ENABLE_SSE
#include "network_drivers/presentation/sse/sse.h" // sse_pool, SseConn, sse_alloc/write
#endif
#include <stdio.h>
#include <string.h>

#if DWS_ENABLE_WEBSOCKET
// Magic GUID concatenated to the client key for the WS accept hash (RFC 6455 4.2.2).
static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
#endif

// ---------------------------------------------------------------------------
// WebSocket handshake helpers
// ---------------------------------------------------------------------------

#if DWS_ENABLE_WEBSOCKET
/**
 * @brief Compute the Sec-WebSocket-Accept value for the HTTP 101 response.
 *
 * Concatenates the client key with the RFC 6455 magic GUID, SHA-1 hashes
 * the result, and base64-encodes the 20-byte digest into @p out.
 * @p out must be at least 29 bytes (28 base64 chars + null terminator).
 */
static const size_t WS_MAX_KEY_LEN = 64;

static bool ws_accept_key(const char *client_key, char *out)
{
    size_t key_len = strnlen(client_key, WS_MAX_KEY_LEN + 1);
    if (key_len > WS_MAX_KEY_LEN)
    {
        out[0] = '\0';
        return false;
    }
    // RFC 6455 4.2.1: the Sec-WebSocket-Key must base64-decode to exactly 16 bytes.
    uint8_t raw[24];
    if (base64_decode(client_key, raw, sizeof(raw)) != 16)
    {
        out[0] = '\0';
        return false;
    }
    size_t magic_len = sizeof(WS_MAGIC) - 1;
    char concat[WS_MAX_KEY_LEN + sizeof(WS_MAGIC)];
    memcpy(concat, client_key, key_len);
    memcpy(concat + key_len, WS_MAGIC, magic_len);

    uint8_t digest[SHA1_DIGEST_LEN];
    sha1((const uint8_t *)concat, key_len + magic_len, digest);
    base64_encode(digest, SHA1_DIGEST_LEN, out);
    return true;
}

/**
 * @brief Send the HTTP 101 Switching Protocols handshake and upgrade the slot.
 *
 * Does NOT close the TCP connection -- that is intentional.  The slot moves
 * from HTTP parse ownership to WS frame parse ownership.
 */
/**
 * @brief Send a 426 Upgrade Required for an unsupported Sec-WebSocket-Version.
 *
 * RFC 6455 §4.2.1: if the version is not 13 the server MUST respond with a
 * 426 and include a Sec-WebSocket-Version header listing the versions it
 * supports.  Closes the connection afterward.
 */
void ws_send_version_required(uint8_t slot_id)
{
    if (!dws_conn_active(slot_id))
    {
        http_reset(slot_id);
        return;
    }

    static const char resp[] = "HTTP/1.1 426 Upgrade Required\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Content-Length: 0\r\n"
                               "Connection: close\r\n\r\n";

    dws_conn_send(slot_id, resp, (u16_t)(sizeof(resp) - 1));
    dws_conn_flush(slot_id);
    dws_conn_begin_close(slot_id); // dwell in ConnState::CONN_CLOSING until the response drains

    http_reset(slot_id);
}

bool ws_do_upgrade(uint8_t slot_id, HttpReq *req, WsConnectHandler on_connect)
{
    const char *client_key = http_get_header(req, "Sec-WebSocket-Key");
    if (!client_key)
        return false;

    char accept[32];
    if (!ws_accept_key(client_key, accept))
        return false;

    if (!dws_conn_active(slot_id))
        return false;

    char hdr[WS_HDR_BUF_SIZE];
    int hlen;
#if DWS_ENABLE_WS_DEFLATE
    // Negotiate permessage-deflate (RFC 7692) if the client offered it. We force
    // no_context_takeover in both directions so each message decompresses
    // independently (the INFLATE window is the message buffer, not a kept window).
    const char *ws_ext = http_get_header(req, "Sec-WebSocket-Extensions");
    bool pmd = ws_ext && strstr(ws_ext, "permessage-deflate");
    hlen = snprintf(hdr, sizeof(hdr),
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: %s\r\n"
                    "%s"
                    "\r\n",
                    accept,
                    pmd ? "Sec-WebSocket-Extensions: permessage-deflate; client_no_context_takeover; "
                          "server_no_context_takeover\r\n"
                        : "");
#else
    hlen = snprintf(hdr, sizeof(hdr),
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: %s\r\n\r\n",
                    accept);
#endif

    dws_conn_send(slot_id, hdr, (u16_t)hlen);
    dws_conn_flush(slot_id);

    // Reset HTTP parser but keep the TCP slot -- WS owns it now
    http_reset(slot_id);

    WsConn *ws = ws_alloc(slot_id);
    if (!ws)
    {
        // No WS slot available -- abort the connection (transport owns the teardown)
        dws_conn_abort_slot(slot_id);
        return false;
    }

#if DWS_ENABLE_WS_DEFLATE
    ws->pmd = pmd;
#endif
    if (on_connect)
        on_connect(ws->ws_id);

    return true;
}
#endif // DWS_ENABLE_WEBSOCKET

// ---------------------------------------------------------------------------
// SSE upgrade helper
// ---------------------------------------------------------------------------

#if DWS_ENABLE_SSE
/**
 * @brief Send the HTTP 200 + SSE headers and promote the slot to SSE mode.
 */
bool sse_do_upgrade(uint8_t slot_id, HttpReq *req, SseConnectHandler on_connect)
{
    if (!dws_conn_active(slot_id))
        return false;

    static const char SSE_HDR[] = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/event-stream\r\n"
                                  "Cache-Control: no-cache\r\n"
                                  "Connection: keep-alive\r\n\r\n";

    dws_conn_send(slot_id, SSE_HDR, (u16_t)(sizeof(SSE_HDR) - 1));
    dws_conn_flush(slot_id);

    // Copy the path BEFORE resetting the parser: http_reset() zeroes the whole
    // HttpReq (including req->path), so a pointer into it would dangle. The saved
    // path is what sse_broadcast() matches against.
    char path[MAX_PATH_LEN];
    strncpy(path, req->path, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    http_reset(slot_id);

    SseConn *sse = sse_alloc(slot_id, path);
    if (!sse)
    {
        dws_conn_abort_slot(slot_id); // transport owns detach + reset + RST
        return false;
    }

    if (on_connect)
        on_connect(sse->sse_id);

    return true;
}
#endif // DWS_ENABLE_SSE

// ---------------------------------------------------------------------------
// WebSocket public API
// ---------------------------------------------------------------------------

#if DWS_ENABLE_WEBSOCKET
void DWS::ws_send_text(uint8_t ws_id, const char *text)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
        return;
    uint16_t len = (uint16_t)strnlen(text, 0xFFFF);
    if (ws_send_frame(ws, WsOpcode::WS_OP_TEXT, (const uint8_t *)text, len))
    {
        if (dws_conn_active(ws->slot_id))
            dws_conn_flush(ws->slot_id);
    }
}

void DWS::ws_send_binary(uint8_t ws_id, const uint8_t *data, uint16_t len)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
        return;
    if (ws_send_frame(ws, WsOpcode::WS_OP_BINARY, data, len))
    {
        if (dws_conn_active(ws->slot_id))
            dws_conn_flush(ws->slot_id);
    }
}

void DWS::ws_disconnect(uint8_t ws_id)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    ws_close(ws, WsCloseCode::WS_CLOSE_NORMAL);
    if (dws_conn_active(ws->slot_id))
        dws_conn_flush(ws->slot_id);
    // handle() detects WsParseState::WS_CLOSED next tick and fires ws_close callback
}
#endif // DWS_ENABLE_WEBSOCKET

// ---------------------------------------------------------------------------
// Server-Sent Events public API
// ---------------------------------------------------------------------------

#if DWS_ENABLE_SSE
void DWS::sse_send(uint8_t sse_id, const char *data, const char *event, const char *id)
{
    if (sse_id >= MAX_SSE_CONNS || !sse_pool[sse_id].active)
        return;
    SseConn *sse = &sse_pool[sse_id];
    if (sse_write(sse, data, event, id))
    {
        if (dws_conn_active(sse->slot_id))
            dws_conn_flush(sse->slot_id);
    }
}

void DWS::sse_broadcast(const char *path, const char *data, const char *event, const char *id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (!sse_pool[i].active)
            continue;
        if (strcmp(sse_pool[i].path, path) != 0)
            continue;
        SseConn *sse = &sse_pool[i];
        if (sse_write(sse, data, event, id))
        {
            if (dws_conn_active(sse->slot_id))
                dws_conn_flush(sse->slot_id);
        }
    }
}
#endif // DWS_ENABLE_SSE
