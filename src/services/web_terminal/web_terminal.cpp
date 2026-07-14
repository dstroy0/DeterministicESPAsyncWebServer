// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file web_terminal.cpp
 * @brief Browser web-serial terminal over WebSocket (DETWS_ENABLE_WEB_TERMINAL).
 */

#include "services/web_terminal/web_terminal.h"

#if DETWS_ENABLE_WEB_TERMINAL

// Dependency (WEB_TERMINAL requires WEBSOCKET) is enforced centrally in ServerConfig.h.

#include "network_drivers/application/web_assets.h" // DETWS_TERMINAL_PAGE
#include "shared_primitives/mime.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// State (all static / BSS - no heap)
// ---------------------------------------------------------------------------
// All web-terminal state, owned by one instance (internal linkage): the server handle, the
// command callback, the WebSocket path, and which ws slots are terminal browsers. Grouped so
// it is one named owner, unreachable cross-TU. (The route/ws handlers are fixed-signature
// callbacks, so they reach this single owner directly.)
struct WebTerminalCtx
{
    DetWebServer *srv = nullptr;
    TermCommandCb cb = nullptr;
    char ws_path[MAX_PATH_LEN] = {0};
    bool is_client[MAX_WS_CONNS] = {}; // which ws slots are terminal browsers
};
static WebTerminalCtx s_term;

// ---- internal route handlers ----------------------------------------------

static void term_page_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    if (s_term.srv)
        s_term.srv->send(slot_id, 200, DET_MIME_TEXT_HTML, DETWS_TERMINAL_PAGE);
}

static void term_ws_connect(uint8_t ws_id)
{
    if (ws_id < MAX_WS_CONNS)
        s_term.is_client[ws_id] = true;
    if (s_term.srv)
        s_term.srv->ws_send_text(ws_id, "DeterministicESPAsyncWebServer terminal ready\n");
}

static void term_ws_message(uint8_t ws_id)
{
    if (s_term.cb && ws_id < MAX_WS_CONNS)
        s_term.cb(ws_payload(ws_id), ws_id);
}

static void term_ws_close(uint8_t ws_id)
{
    if (ws_id < MAX_WS_CONNS)
        s_term.is_client[ws_id] = false;
}

// ---- public API -----------------------------------------------------------

void detws_web_terminal_begin(DetWebServer &server, const char *path)
{
    s_term.srv = &server;
    for (uint8_t i = 0; i < MAX_WS_CONNS; i++)
        s_term.is_client[i] = false;

    if (!path || !path[0])
        path = "/terminal";
    snprintf(s_term.ws_path, sizeof(s_term.ws_path), "%s/ws", path);

    server.on(path, HttpMethod::HTTP_GET, term_page_handler);
    server.on_ws(s_term.ws_path, term_ws_connect, term_ws_message, term_ws_close);
}

void detws_web_terminal_on_command(TermCommandCb cb)
{
    s_term.cb = cb;
}

void detws_web_terminal_print(const char *s)
{
    if (!s_term.srv || !s)
        return;
    for (uint8_t i = 0; i < MAX_WS_CONNS; i++)
    {
        if (s_term.is_client[i] && ws_active(i))
            s_term.srv->ws_send_text(i, s);
    }
}

void detws_web_terminal_println(const char *s)
{
    char buf[TERM_TX_BUF_SIZE];
    snprintf(buf, sizeof(buf), "%s\n", s ? s : "");
    detws_web_terminal_print(buf);
}

void detws_web_terminal_printf(const char *fmt, ...)
{
    char buf[TERM_TX_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    detws_web_terminal_print(buf);
}

uint8_t detws_web_terminal_client_count()
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_WS_CONNS; i++)
        if (s_term.is_client[i] && ws_active(i))
            n++;
    return n;
}

#endif // DETWS_ENABLE_WEB_TERMINAL
