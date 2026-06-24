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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Embedded terminal page - green-phosphor CRT theme matching the docs site
// (near-black bg, glowing green text, scanlines + vignette). Kept compact so it
// fits one tcp_write below the lwIP send buffer. The page auto-selects ws:// or
// wss:// from its own scheme, so it works unchanged behind TLS.
// ---------------------------------------------------------------------------
static const char TERM_PAGE[] PROGMEM =
    "<!doctype html><html><head><meta charset=utf-8>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<title>DetWS Terminal</title><style>"
    ":root{--g:#2bb35a;--g2:#00c000;--bg:#080c08}"
    "*{box-sizing:border-box}"
    "html,body{margin:0;height:100%;background:var(--bg);color:var(--g);"
    "font:14px/1.45 'Consolas','DejaVu Sans Mono',monospace}"
    "#wrap{display:flex;flex-direction:column;height:100%;position:relative}"
    "#out{flex:1;overflow:auto;padding:10px;white-space:pre-wrap;word-break:break-word;"
    "text-shadow:0 0 4px rgba(43,179,90,.55)}"
    "#bar{display:flex;border-top:1px solid #14401f;background:#0a0f0a}"
    "#ps{padding:8px 4px 8px 10px;color:var(--g2)}"
    "#in{flex:1;background:transparent;border:0;color:var(--g);font:inherit;padding:8px;outline:none}"
    "#st{padding:4px 10px;font-size:11px;color:#1c6e38;border-top:1px solid #14401f}"
    "#crt{position:absolute;inset:0;pointer-events:none;z-index:9;"
    "background:repeating-linear-gradient(rgba(0,0,0,0) 0 2px,rgba(0,0,0,.18) 3px);"
    "box-shadow:inset 0 0 120px rgba(0,0,0,.6)}"
    ".e{color:#d2553a}</style></head><body><div id=wrap>"
    "<div id=out></div>"
    "<div id=bar><span id=ps>&gt;</span><input id=in autocomplete=off autofocus></div>"
    "<div id=st>connecting\xE2\x80\xA6</div><div id=crt></div></div><script>"
    "var out=document.getElementById('out'),inp=document.getElementById('in'),st=document.getElementById('st');"
    "var hist=[],hi=0,ws,"
    "url=(location.protocol=='https:'?'wss':'ws')+'://'+location.host+location.pathname.replace(/\\/$/,'')+'/ws';"
    "function add(t,c){var b=out.scrollHeight-out.clientHeight-out.scrollTop<40;"
    "var s=document.createElement('span');if(c)s.className=c;s.textContent=t;out.appendChild(s);"
    "if(b)out.scrollTop=out.scrollHeight}"
    "function conn(){ws=new WebSocket(url);"
    "ws.onopen=function(){st.textContent='connected'};"
    "ws.onmessage=function(e){add(e.data)};"
    "ws.onclose=function(){st.textContent='disconnected \xE2\x80\x94 retrying\xE2\x80\xA6';setTimeout(conn,1000)};"
    "ws.onerror=function(){try{ws.close()}catch(_){}}}"
    "inp.addEventListener('keydown',function(e){"
    "if(e.key=='Enter'){var v=inp.value;if(v){"
    "if(ws&&ws.readyState==1){ws.send(v);add('> '+v+'\\n')}else add('> '+v+' (offline)\\n','e');"
    "hist.push(v);hi=hist.length}inp.value=''}"
    "else if(e.key=='ArrowUp'){if(hi>0){hi--;inp.value=hist[hi]||''}}"
    "else if(e.key=='ArrowDown'){if(hi<hist.length){hi++;inp.value=hist[hi]||''}}});"
    "conn()</script></body></html>";

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
        s_srv->send(slot_id, 200, "text/html", TERM_PAGE);
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
