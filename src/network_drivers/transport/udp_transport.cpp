// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_transport.cpp
 * @brief Layer 4 UDP datagram service - the only place lwIP UDP is touched.
 */

#include "network_drivers/transport/udp_transport.h"

#if defined(ARDUINO)

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

bool det_udp_peer_addr(const struct DetUdpPeer *peer, char *ip_out, size_t ip_cap, uint16_t *port_out)
{
    if (!peer || !peer->addr || !ip_out || ip_cap < 8)
        return false;
    ipaddr_ntoa_r(peer->addr, ip_out, (int)ip_cap);
    if (port_out)
        *port_out = peer->port;
    return true;
}

bool det_udp_listener_sendto(uint16_t listen_port, const char *dst_ip, uint16_t dst_port, const uint8_t *data,
                             size_t len)
{
    if (!dst_ip || !data || len == 0)
        return false;
    ip_addr_t dst;
    if (!ipaddr_aton(dst_ip, &dst))
        return false;
    for (int i = 0; i < DETWS_MAX_UDP_LISTENERS; i++)
    {
        if (s_listeners[i].used && s_listeners[i].pcb && s_listeners[i].pcb->local_port == listen_port)
        {
            struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)len, PBUF_RAM);
            if (!p)
                return false;
            memcpy(p->payload, data, len);
            err_t e = udp_sendto(s_listeners[i].pcb, p, &dst, dst_port);
            pbuf_free(p);
            return e == ERR_OK;
        }
    }
    return false;
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

// Host capture seam (test-only): the last datagram handed to det_udp_sendto().
static bool s_udp_cap_on = false;
static uint8_t s_udp_cap_buf[2048];
static size_t s_udp_cap_len = 0;

void det_udp_capture_enable()
{
    s_udp_cap_on = true;
    s_udp_cap_len = 0;
}
void det_udp_capture_reset()
{
    s_udp_cap_len = 0;
}
const uint8_t *det_udp_captured()
{
    return s_udp_cap_len ? s_udp_cap_buf : nullptr;
}
size_t det_udp_captured_len()
{
    return s_udp_cap_len;
}

bool det_udp_sendto(const char *dst_ip, uint16_t dst_port, const uint8_t *data, size_t len)
{
    (void)dst_ip;
    (void)dst_port;
    if (s_udp_cap_on && data && len && len <= sizeof(s_udp_cap_buf))
    {
        memcpy(s_udp_cap_buf, data, len);
        s_udp_cap_len = len;
        return true; // a captured "send" succeeds so the caller's success path is exercised
    }
    (void)data;
    (void)len;
    return false;
}

bool det_udp_peer_addr(const struct DetUdpPeer *peer, char *ip_out, size_t ip_cap, uint16_t *port_out)
{
    (void)peer;
    (void)ip_out;
    (void)ip_cap;
    (void)port_out;
    return false;
}

bool det_udp_listener_sendto(uint16_t listen_port, const char *dst_ip, uint16_t dst_port, const uint8_t *data,
                             size_t len)
{
    (void)listen_port;
    (void)dst_ip;
    (void)dst_port;
    (void)data;
    (void)len;
    return false;
}

#endif // ARDUINO
