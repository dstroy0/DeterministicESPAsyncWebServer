// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_client.cpp
 * @brief Layer 4 outbound TCP client transport (pooled). See det_client.h.
 *
 * Mirrors the server transport's cross-thread rule: every raw lwIP call runs in
 * tcpip_thread via tcpip_api_call(). Each slot owns its pcb and an SPSC wire ring
 * (producer = the lwIP recv callback in tcpip_thread; consumer = the caller's
 * loop/blocking task). The rings use `volatile` indices, matching the shipped
 * per-client implementations this consolidates.
 */

#include "det_client.h"

#if defined(ARDUINO)

#include "lwip/priv/tcpip_priv.h"
#include "lwip/tcp.h"
#include "services/det_clock.h"                 // detws_millis()
#include "services/dns_resolver/dns_resolver.h" // shared host->IP resolve (one DNS owner)
#include "shared_primitives/det_ring.h"         // shared DetAtomic + SPSC ring drain (same primitive as the server)
#include <Arduino.h>                            // delay()
#include <string.h>

struct ClientConn
{
    struct tcp_pcb *pcb;
    volatile bool in_use;
    volatile bool connected;
    volatile bool closed; // peer FIN or error
    uint8_t rx[DETWS_CLIENT_RX_BUF];
    DetAtomic<size_t> head; // producer (lwIP recv cb); acquire/release SPSC, same as the server ring
    DetAtomic<size_t> tail; // consumer (caller)
};

static ClientConn s_cc[DETWS_CLIENT_CONNS];

// Hostname resolution is delegated to the shared DNS resolver (detws_dns_resolve,
// services/dns_resolver) so there is one owner of the gethostbyname-marshal +
// deadline-poll pattern instead of a private copy here.

// --- lwIP callbacks (tcpip_thread); arg = the owning ClientConn* -------------

static err_t cc_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    (void)err;
    ClientConn *c = (ClientConn *)arg;
    if (!c)
        return ERR_OK;
    if (p == nullptr)
    {
        c->closed = true; // peer closed
        return ERR_OK;
    }
    // Wire bytes -> ring via the shared producer primitive (same as the server): if
    // the whole segment will not fit, refuse it (lwIP retains + redelivers); else
    // bulk-memcpy each pbuf span and publish head once.
    (void)tpcb;
    if (p->tot_len > det_ring_free(c->head, c->tail, DETWS_CLIENT_RX_BUF))
        return ERR_MEM;
    size_t h = c->head; // sole producer of head; advance a local and publish once
    for (struct pbuf *q = p; q; q = q->next)
        h = det_ring_write_span(c->rx, DETWS_CLIENT_RX_BUF, h, (const uint8_t *)q->payload, q->len);
    c->head = h; // single release store publishes the whole segment
    // Do NOT tcp_recved() here. The window is reopened by det_client_read() as the
    // caller drains (ack-on-consume), so it tracks ring occupancy and the peer can
    // never overflow the ring - same model as the server transport. ACKing on copy
    // would decouple the window from drainage and deadlock a large inbound transfer
    // once DETWS_CLIENT_RX_BUF < TCP_WND.
    pbuf_free(p);
    return ERR_OK;
}

static err_t cc_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    (void)tpcb;
    ClientConn *c = (ClientConn *)arg;
    if (c)
    {
        if (err == ERR_OK)
            c->connected = true;
        else
            c->closed = true;
    }
    return ERR_OK;
}

static void cc_err(void *arg, err_t err)
{
    (void)err;
    ClientConn *c = (ClientConn *)arg;
    if (c)
    {
        c->pcb = nullptr; // lwIP already freed it
        c->closed = true;
    }
}

// --- tcpip_thread-marshaled ops ---------------------------------------------

struct CcConnCall
{
    struct tcpip_api_call_data base;
    ClientConn *c;
    ip_addr_t addr;
    uint16_t port;
    err_t result;
};
struct CcSendCall
{
    struct tcpip_api_call_data base;
    ClientConn *c;
    const void *data;
    u16_t len;
    err_t result;
};
struct CcRecvedCall
{
    struct tcpip_api_call_data base;
    ClientConn *c;
    u16_t len;
};

static err_t cc_do_connect(struct tcpip_api_call_data *cd)
{
    CcConnCall *k = (CcConnCall *)cd;
    ClientConn *c = k->c;
    c->pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!c->pcb)
    {
        k->result = ERR_MEM;
        return ERR_OK;
    }
    tcp_arg(c->pcb, c);
    tcp_recv(c->pcb, cc_recv);
    tcp_err(c->pcb, cc_err);
    k->result = tcp_connect(c->pcb, &k->addr, k->port, cc_connected);
    return ERR_OK;
}

static err_t cc_do_send(struct tcpip_api_call_data *cd)
{
    CcSendCall *k = (CcSendCall *)cd;
    ClientConn *c = k->c;
    if (!c->pcb)
    {
        k->result = ERR_CONN;
        return ERR_OK;
    }
    k->result = tcp_write(c->pcb, k->data, k->len, TCP_WRITE_FLAG_COPY);
    if (k->result == ERR_OK)
        tcp_output(c->pcb);
    return ERR_OK;
}

static err_t cc_do_close(struct tcpip_api_call_data *cd)
{
    CcSendCall *k = (CcSendCall *)cd;
    ClientConn *c = k->c;
    if (c->pcb)
    {
        tcp_arg(c->pcb, nullptr);
        tcp_recv(c->pcb, nullptr);
        tcp_err(c->pcb, nullptr);
        if (tcp_close(c->pcb) != ERR_OK)
            tcp_abort(c->pcb);
        c->pcb = nullptr;
    }
    return ERR_OK;
}

static err_t cc_do_recved(struct tcpip_api_call_data *cd)
{
    CcRecvedCall *k = (CcRecvedCall *)cd;
    if (k->c->pcb)
        tcp_recved(k->c->pcb, k->len); // reopen the window by the consumed bytes
    return ERR_OK;
}

// --- public API --------------------------------------------------------------

int det_client_open(const char *host, uint16_t port, uint32_t timeout_ms)
{
    int cid = -1;
    for (int i = 0; i < DETWS_CLIENT_CONNS; i++)
        if (!s_cc[i].in_use)
        {
            cid = i;
            break;
        }
    if (cid < 0)
        return -1; // pool full

    ClientConn *c = &s_cc[cid];
    c->pcb = nullptr;
    c->connected = false;
    c->closed = false;
    c->head = 0;
    c->tail = 0;
    c->in_use = true;

    // Resolve through the shared DNS owner (its own DETWS_DNS_TIMEOUT_MS budget),
    // then give the connect its full timeout_ms.
    uint32_t ip = 0;
    if (!detws_dns_resolve(host, &ip))
    {
        c->in_use = false;
        return -2; // DNS failure
    }
    uint32_t deadline = detws_millis() + timeout_ms;

    CcConnCall k;
    memset(&k, 0, sizeof(k));
    k.c = c;
    IP_ADDR4(&k.addr, (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)ip);
    k.port = port;
    tcpip_api_call(cc_do_connect, &k.base);
    if (k.result != ERR_OK)
    {
        det_client_close(cid);
        return -3; // connect issue
    }
    while (!c->connected && !c->closed && (int32_t)(deadline - detws_millis()) > 0)
        delay(5);
    if (!c->connected)
    {
        det_client_close(cid);
        return -4; // connect timeout / refused
    }
    return cid;
}

bool det_client_connected(int cid)
{
    return cid >= 0 && cid < DETWS_CLIENT_CONNS && s_cc[cid].in_use && s_cc[cid].connected && !s_cc[cid].closed;
}

bool det_client_is_closed(int cid)
{
    if (cid < 0 || cid >= DETWS_CLIENT_CONNS)
        return true;
    return s_cc[cid].closed;
}

bool det_client_send(int cid, const void *data, size_t len)
{
    if (cid < 0 || cid >= DETWS_CLIENT_CONNS || !s_cc[cid].in_use)
        return false;
    CcSendCall k;
    memset(&k, 0, sizeof(k));
    k.c = &s_cc[cid];
    k.data = data;
    k.len = (u16_t)(len > 0xFFFF ? 0xFFFF : len);
    tcpip_api_call(cc_do_send, &k.base);
    return k.result == ERR_OK;
}

size_t det_client_available(int cid)
{
    if (cid < 0 || cid >= DETWS_CLIENT_CONNS)
        return 0;
    ClientConn *c = &s_cc[cid];
    return det_ring_available(c->head, c->tail, DETWS_CLIENT_RX_BUF);
}

size_t det_client_read(int cid, uint8_t *buf, size_t cap)
{
    if (cid < 0 || cid >= DETWS_CLIENT_CONNS)
        return 0;
    ClientConn *c = &s_cc[cid];
    size_t n = det_ring_read(c->rx, DETWS_CLIENT_RX_BUF, c->head, c->tail, buf, cap);
    if (n > 0 && c->pcb)
    {
        // Ack-on-consume: reopen the receive window by exactly what we just drained.
        CcRecvedCall k;
        memset(&k, 0, sizeof(k));
        k.c = c;
        k.len = (u16_t)n;
        tcpip_api_call(cc_do_recved, &k.base);
    }
    return n;
}

void det_client_close(int cid)
{
    if (cid < 0 || cid >= DETWS_CLIENT_CONNS || !s_cc[cid].in_use)
        return;
    CcSendCall k;
    memset(&k, 0, sizeof(k));
    k.c = &s_cc[cid];
    tcpip_api_call(cc_do_close, &k.base);
    s_cc[cid].in_use = false;
}

#else // !ARDUINO - host stub (the clients are ARDUINO-only; host builds no-op)

int det_client_open(const char *, uint16_t, uint32_t)
{
    return -1;
}
bool det_client_connected(int)
{
    return false;
}
bool det_client_is_closed(int)
{
    return true;
}
bool det_client_send(int, const void *, size_t)
{
    return false;
}
size_t det_client_available(int)
{
    return 0;
}
size_t det_client_read(int, uint8_t *, size_t)
{
    return 0;
}
void det_client_close(int)
{
}

#endif // ARDUINO
