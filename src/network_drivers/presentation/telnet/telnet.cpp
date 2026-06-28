// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telnet.cpp
 * @brief Minimal RFC 854 Telnet server implementation.
 */

#include "network_drivers/presentation/telnet/telnet.h"

#if DETWS_ENABLE_TELNET

#include "network_drivers/transport/transport.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Telnet protocol bytes (RFC 854 / 858 / 857).
enum
{
    T_SE = 240,
    T_SB = 250,
    T_WILL = 251,
    T_WONT = 252,
    T_DO = 253,
    T_DONT = 254,
    T_IAC = 255,
    OPT_ECHO = 1,
    OPT_SGA = 3
};

// IAC parser state per connection.
enum
{
    TN_NORMAL,
    TN_IAC,
    TN_OPT,
    TN_SB
};

struct TelnetConn
{
    uint8_t slot;
    bool used;
    uint8_t st;   // IAC parser state
    uint8_t cmd;  // pending WILL/WONT/DO/DONT
    uint16_t len; // bytes in line[]
    char line[TELNET_BUF_SIZE];
};

static TelnetConn g_tn[MAX_TELNET_CONNS];
static TelnetCommandCb g_cmd_cb = nullptr;

static TelnetConn *find_conn(uint8_t slot)
{
    for (int i = 0; i < MAX_TELNET_CONNS; i++)
        if (g_tn[i].used && g_tn[i].slot == slot)
            return &g_tn[i];
    return nullptr;
}

static void raw_send(uint8_t slot, const void *data, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE || !c->pcb || n == 0)
        return;
    det_conn_send(c->id, c->pcb, data, (u16_t)n);
    det_conn_flush(c->id, c->pcb);
}

// ---------------------------------------------------------------------------
// Connection lifecycle (called from the session layer)
// ---------------------------------------------------------------------------

void telnet_accept(uint8_t slot)
{
    TelnetConn *t = nullptr;
    for (int i = 0; i < MAX_TELNET_CONNS; i++)
        if (!g_tn[i].used)
        {
            t = &g_tn[i];
            break;
        }
    if (!t)
    {
        // No Telnet capacity: drop the connection.
        TcpConn *c = &conn_pool[slot];
        if (c->pcb)
        {
            struct tcp_pcb *p = c->pcb;
            det_conn_detach(p);
            c->state = CONN_FREE;
            c->pcb = nullptr;
            det_conn_close(slot, p);
        }
        return;
    }
    memset(t, 0, sizeof(*t));
    t->used = true;
    t->slot = slot;
    t->st = TN_NORMAL;

    // Server-side echo + character-at-a-time (suppress go-ahead).
    static const uint8_t neg[] = {T_IAC, T_WILL, OPT_ECHO, T_IAC, T_WILL, OPT_SGA};
    raw_send(slot, neg, sizeof(neg));
    raw_send(slot, "DetWS Telnet ready\r\n> ", 22);
}

void telnet_close(uint8_t slot)
{
    TelnetConn *t = find_conn(slot);
    if (t)
        t->used = false;
}

// Process one decoded data byte (not part of an IAC sequence).
static void handle_data(uint8_t slot, TelnetConn *t, uint8_t b)
{
    if (b == '\r')
        return; // wait for the LF of CRLF
    if (b == '\n')
    {
        t->line[t->len] = '\0';
        raw_send(slot, "\r\n", 2);
        if (g_cmd_cb)
            g_cmd_cb(t->line, (uint8_t)(t - g_tn));
        t->len = 0;
        raw_send(slot, "> ", 2);
        return;
    }
    if (b == 0x08 || b == 0x7F) // backspace / delete
    {
        if (t->len > 0)
        {
            t->len--;
            raw_send(slot, "\b \b", 3);
        }
        return;
    }
    if (b < 0x20) // ignore other control characters
        return;
    if (t->len < sizeof(t->line) - 1)
    {
        t->line[t->len++] = (char)b;
        raw_send(slot, &b, 1); // echo
    }
}

void telnet_rx(uint8_t slot)
{
    TelnetConn *t = find_conn(slot);
    if (!t)
        return;
    TcpConn *conn = &conn_pool[slot];

    while (conn->rx_tail != conn->rx_head)
    {
        uint8_t b = conn->rx_buffer[conn->rx_tail];
        conn->rx_tail = (conn->rx_tail + 1) % RX_BUF_SIZE;

        switch (t->st)
        {
        case TN_NORMAL:
            if (b == T_IAC)
                t->st = TN_IAC;
            else
                handle_data(slot, t, b);
            break;
        case TN_IAC:
            if (b == T_SB)
                t->st = TN_SB;
            else if (b == T_WILL || b == T_WONT || b == T_DO || b == T_DONT)
            {
                t->cmd = b;
                t->st = TN_OPT;
            }
            else if (b == T_IAC)
            {
                handle_data(slot, t, 0xFF); // escaped literal 0xFF
                t->st = TN_NORMAL;
            }
            else
                t->st = TN_NORMAL; // other 2-byte command (GA, NOP, ...) - consume
            break;
        case TN_OPT: {
            // Refuse what we don't actively support; stay quiet on options we
            // already offered (ECHO/SGA) to avoid negotiation loops.
            uint8_t reply = 0;
            if (t->cmd == T_DO && b != OPT_ECHO && b != OPT_SGA)
                reply = T_WONT;
            else if (t->cmd == T_WILL)
                reply = T_DONT;
            if (reply)
            {
                uint8_t resp[3] = {T_IAC, reply, b};
                raw_send(slot, resp, 3);
            }
            t->st = TN_NORMAL;
            break;
        }
        case TN_SB:
            if (b == T_SE)
                t->st = TN_NORMAL; // end of subnegotiation (contents ignored)
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Application API
// ---------------------------------------------------------------------------

void telnet_on_command(TelnetCommandCb cb)
{
    g_cmd_cb = cb;
}

static void broadcast(const char *s, size_t n)
{
    for (int i = 0; i < MAX_TELNET_CONNS; i++)
        if (g_tn[i].used)
            raw_send(g_tn[i].slot, s, n);
}

void telnet_print(const char *s)
{
    if (s)
        broadcast(s, strlen(s));
}

void telnet_println(const char *s)
{
    if (s)
        broadcast(s, strlen(s));
    broadcast("\r\n", 2);
}

void telnet_printf(const char *fmt, ...)
{
    char buf[TELNET_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0)
        broadcast(buf, (size_t)(n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1));
}

uint8_t telnet_client_count()
{
    uint8_t c = 0;
    for (int i = 0; i < MAX_TELNET_CONNS; i++)
        if (g_tn[i].used)
            c++;
    return c;
}

#endif // DETWS_ENABLE_TELNET
