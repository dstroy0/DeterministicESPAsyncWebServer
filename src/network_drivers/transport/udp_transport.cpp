// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_transport.cpp
 * @brief Layer 4 UDP datagram service - the only place lwIP UDP is touched.
 */

#include "network_drivers/transport/udp_transport.h"

#if defined(ARDUINO)

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <string.h>

// A small fixed pool of bound UDP ports (e.g. SNMP :161 + captive DNS :53). No
// heap: the pool and the shared receive scratch live in BSS.
struct UdpListener
{
    struct udp_pcb *pcb;
    DetUdpHandler handler;
    void *ctx;
    bool used;
};

static UdpListener s_listeners[DETWS_MAX_UDP_LISTENERS];
static uint8_t s_rx[DETWS_UDP_RX_BUF_SIZE]; // shared: lwIP delivers one datagram at a time

// Concrete peer: lwIP source address/port plus the receiving PCB to reply on.
struct DetUdpPeer
{
    const ip_addr_t *addr;
    u16_t port;
    struct udp_pcb *pcb;
};

// lwIP udp_recv trampoline: copy the (possibly chained) pbuf into a contiguous
// scratch buffer and hand it to the registered handler.
static void udp_trampoline(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    UdpListener *l = (UdpListener *)arg;
    if (!p)
        return;
    u16_t n = (p->tot_len < sizeof(s_rx)) ? p->tot_len : (u16_t)sizeof(s_rx);
    pbuf_copy_partial(p, s_rx, n, 0);
    pbuf_free(p);
    if (l && l->handler)
    {
        DetUdpPeer peer = {addr, port, pcb};
        l->handler(s_rx, n, &peer, l->ctx);
    }
}

bool det_udp_listen(uint16_t port, DetUdpHandler handler, void *ctx)
{
    for (int i = 0; i < DETWS_MAX_UDP_LISTENERS; i++)
    {
        if (s_listeners[i].used)
            continue;
        struct udp_pcb *pcb = udp_new();
        if (!pcb)
            return false;
        if (udp_bind(pcb, IP_ANY_TYPE, port) != ERR_OK)
        {
            udp_remove(pcb);
            return false;
        }
        s_listeners[i].pcb = pcb;
        s_listeners[i].handler = handler;
        s_listeners[i].ctx = ctx;
        s_listeners[i].used = true;
        udp_recv(pcb, udp_trampoline, &s_listeners[i]);
        return true;
    }
    return false; // pool exhausted
}

bool det_udp_send(struct DetUdpPeer *peer, const uint8_t *data, size_t len)
{
    if (!peer || !peer->pcb || len == 0)
        return false;
    struct pbuf *out = pbuf_alloc(PBUF_TRANSPORT, (u16_t)len, PBUF_RAM);
    if (!out)
        return false;
    memcpy(out->payload, data, len);
    err_t e = udp_sendto(peer->pcb, out, peer->addr, peer->port);
    pbuf_free(out);
    return e == ERR_OK;
}

bool det_udp_sendto(const char *dst_ip, uint16_t dst_port, const uint8_t *data, size_t len)
{
    if (!dst_ip || !data || len == 0)
        return false;
    ip_addr_t dst;
    if (!ipaddr_aton(dst_ip, &dst))
        return false;

    // One shared outbound PCB for all det_udp_sendto() users (lazy-created).
    static struct udp_pcb *s_out = nullptr;
    if (!s_out)
    {
        s_out = udp_new();
        if (!s_out)
            return false;
    }
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)len, PBUF_RAM);
    if (!p)
        return false;
    memcpy(p->payload, data, len);
    err_t e = udp_sendto(s_out, p, &dst, dst_port);
    pbuf_free(p);
    return e == ERR_OK;
}

#else // host build: no lwIP. Stubs keep UDP-using services host-compilable.

bool det_udp_listen(uint16_t port, DetUdpHandler handler, void *ctx)
{
    (void)port;
    (void)handler;
    (void)ctx;
    return false;
}

bool det_udp_send(struct DetUdpPeer *peer, const uint8_t *data, size_t len)
{
    (void)peer;
    (void)data;
    (void)len;
    return false;
}

bool det_udp_sendto(const char *dst_ip, uint16_t dst_port, const uint8_t *data, size_t len)
{
    (void)dst_ip;
    (void)dst_port;
    (void)data;
    (void)len;
    return false;
}

#endif // ARDUINO
