// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file web_terminal.cpp
 * @brief Browser web-serial terminal over WebSocket (DETWS_ENABLE_WEB_TERMINAL).
 */

#include "services/web_terminal.h"

#if DETWS_ENABLE_WEB_TERMINAL

#if !DETWS_ENABLE_WEBSOCKET
#error "DETWS_ENABLE_WEB_TERMINAL requires DETWS_ENABLE_WEBSOCKET"
#endif

#include "network_drivers/application/html.h" // DETWS_TERMINAL_PAGE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// State (all static / BSS - no heap)
// ---------------------------------------------------------------------------
static DetWebServer *s_srv = nullptr;
static TermCommandCb s_cb = nullptr;
static char s_ws_path[MAX_PATH_LEN];
static bool s_is_client[MAX_WS_CONNS]; // which ws slots are terminal browsers

// ---- internal route handlers ----------------------------------------------

static void term_page_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    if (s_srv)
        s_srv->send(slot_id, 200, "text/html", DETWS_TERMINAL_PAGE);
}

static void term_ws_connect(uint8_t ws_id)
{
    if (ws_id < MAX_WS_CONNS)
        s_is_client[ws_id] = true;
    if (s_srv)
        s_srv->ws_send_text(ws_id, "DeterministicESPAsyncWebServer terminal ready\n");
}

static void term_ws_message(uint8_t ws_id)
{
    if (s_cb && ws_id < MAX_WS_CONNS)
        s_cb((const char *)ws_pool[ws_id].buf, ws_id);
}

static void term_ws_close(uint8_t ws_id)
{
    if (ws_id < MAX_WS_CONNS)
        s_is_client[ws_id] = false;
}

// ---- public API -----------------------------------------------------------

void detws_web_terminal_begin(DetWebServer &server, const char *path)
{
    s_srv = &server;
    for (uint8_t i = 0; i < MAX_WS_CONNS; i++)
        s_is_client[i] = false;

    if (!path || !path[0])
        path = "/terminal";
    snprintf(s_ws_path, sizeof(s_ws_path), "%s/ws", path);

    server.on(path, HTTP_GET, term_page_handler);
    server.on_ws(s_ws_path, term_ws_connect, term_ws_message, term_ws_close);
}

void detws_web_terminal_on_command(TermCommandCb cb)
{
    s_cb = cb;
}

void detws_web_terminal_print(const char *s)
{
    if (!s_srv || !s)
        return;
    for (uint8_t i = 0; i < MAX_WS_CONNS; i++)
    {
        if (s_is_client[i] && ws_pool[i].active)
            s_srv->ws_send_text(i, s);
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
        if (s_is_client[i] && ws_pool[i].active)
            n++;
    return n;
}

#endif // DETWS_ENABLE_WEB_TERMINAL
