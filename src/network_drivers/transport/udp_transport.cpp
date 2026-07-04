// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_transport.cpp
 * @brief Layer 4 UDP datagram service - the only place lwIP UDP is touched.
 */

#include "network_drivers/transport/udp_transport.h"

#include <string.h> // memcpy (both the lwIP and host builds)

#if defined(ARDUINO)

#include "lwip/pbuf.h"
#include "lwip/priv/tcpip_priv.h" // tcpip_api_call - marshal raw udp_* onto tcpip_thread
#include "lwip/udp.h"

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
static struct udp_pcb *s_out = nullptr;     // one shared outbound PCB for det_udp_sendto()

// True while a udp_recv trampoline (or a marshaled op) is running, i.e. while we are
// already inside tcpip_thread. A handler replying from the trampoline then sends directly
// instead of re-marshaling (which would deadlock on the tcpip mailbox) - the UDP mirror of
// transport.cpp's s_in_tcpip_thread.
static volatile bool s_in_tcpip_thread = false;

// Concrete peer: lwIP source address/port plus the receiving PCB to reply on.
struct DetUdpPeer
{
    const ip_addr_t *addr;
    u16_t port;
    struct udp_pcb *pcb;
};

// Raw send (alloc + copy + sendto + free). Only ever called in tcpip_thread.
static bool udp_pbuf_send(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port, const uint8_t *data, size_t len)
{
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)len, PBUF_RAM);
    if (!p)
        return false;
    memcpy(p->payload, data, len);
    err_t e = udp_sendto(pcb, p, addr, port);
    pbuf_free(p);
    return e == ERR_OK;
}

// lwIP udp_recv trampoline: copy the (possibly chained) pbuf into a contiguous
// scratch buffer and hand it to the registered handler. Runs in tcpip_thread, so a
// reply the handler sends is already in-thread (flagged for the send helpers).
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
        bool prev = s_in_tcpip_thread;
        s_in_tcpip_thread = true;
        l->handler(s_rx, n, &peer, l->ctx);
        s_in_tcpip_thread = prev;
    }
}

// Raw lwIP UDP must run in tcpip_thread: with lwIP core-locking (arduino-esp32 3.x /
// IDF 5.x) a udp_new/bind/recv/sendto from any other task asserts ("Required to lock
// TCPIP core functionality"), and without it, it races the stack. det_udp_* therefore
// marshal these ops via tcpip_api_call(), the same as the TCP transport.
enum DetUdpOp
{
    UDP_OP_LISTEN,  // udp_new + bind + arm recv on s_listeners[slot]
    UDP_OP_SEND,    // send to addr:port on an existing pcb
    UDP_OP_SEND_OUT // send to addr:port on the shared lazy outbound pcb
};

struct DetUdpCall
{
    struct tcpip_api_call_data base;
    DetUdpOp op;
    int slot;            // LISTEN: index into s_listeners
    struct udp_pcb *pcb; // SEND: target pcb
    ip_addr_t addr;      // SEND / SEND_OUT: destination (by value - caller's may be transient)
    u16_t port;          // LISTEN: bind port; SEND / SEND_OUT: destination port
    const uint8_t *data; // SEND / SEND_OUT
    size_t len;          // SEND / SEND_OUT
    bool result;
};

// Runs in tcpip_thread via tcpip_api_call.
static err_t udp_do(struct tcpip_api_call_data *c)
{
    DetUdpCall *k = (DetUdpCall *)c;
    bool prev = s_in_tcpip_thread;
    s_in_tcpip_thread = true;
    k->result = false;
    switch (k->op)
    {
    case UDP_OP_LISTEN: {
        struct udp_pcb *pcb = udp_new();
        if (pcb)
        {
            if (udp_bind(pcb, IP_ANY_TYPE, k->port) == ERR_OK)
            {
                s_listeners[k->slot].pcb = pcb;
                udp_recv(pcb, udp_trampoline, &s_listeners[k->slot]);
                k->result = true;
            }
            else
            {
                udp_remove(pcb);
            }
        }
        break;
    }
    case UDP_OP_SEND:
        k->result = udp_pbuf_send(k->pcb, &k->addr, k->port, k->data, k->len);
        break;
    case UDP_OP_SEND_OUT:
        if (!s_out)
            s_out = udp_new();
        if (s_out)
            k->result = udp_pbuf_send(s_out, &k->addr, k->port, k->data, k->len);
        break;
    }
    s_in_tcpip_thread = prev;
    return ERR_OK;
}

bool det_udp_listen(uint16_t port, DetUdpHandler handler, void *ctx)
{
    for (int i = 0; i < DETWS_MAX_UDP_LISTENERS; i++)
    {
        if (s_listeners[i].used)
            continue;
        // The trampoline reads handler/ctx once recv is armed, so set them first.
        s_listeners[i].handler = handler;
        s_listeners[i].ctx = ctx;
        s_listeners[i].pcb = nullptr;
        DetUdpCall k;
        memset(&k, 0, sizeof(k));
        k.op = UDP_OP_LISTEN;
        k.slot = i;
        k.port = port;
        tcpip_api_call(udp_do, &k.base); // always called off tcpip_thread (service begin())
        if (!k.result)
        {
            s_listeners[i].handler = nullptr;
            return false;
        }
        s_listeners[i].used = true;
        return true;
    }
    return false; // pool exhausted
}

bool det_udp_send(struct DetUdpPeer *peer, const uint8_t *data, size_t len)
{
    if (!peer || !peer->pcb || !peer->addr || !data || len == 0)
        return false;
    if (s_in_tcpip_thread) // replying from a handler (already in tcpip_thread)
        return udp_pbuf_send(peer->pcb, peer->addr, peer->port, data, len);
    DetUdpCall k;
    memset(&k, 0, sizeof(k));
    k.op = UDP_OP_SEND;
    k.pcb = peer->pcb;
    k.addr = *peer->addr;
    k.port = peer->port;
    k.data = data;
    k.len = len;
    tcpip_api_call(udp_do, &k.base);
    return k.result;
}

bool det_udp_sendto(const char *dst_ip, uint16_t dst_port, const uint8_t *data, size_t len)
{
    if (!dst_ip || !data || len == 0)
        return false;
    ip_addr_t dst;
    if (!ipaddr_aton(dst_ip, &dst))
        return false;
    if (s_in_tcpip_thread)
    {
        if (!s_out)
        {
            s_out = udp_new();
            if (!s_out)
                return false;
        }
        return udp_pbuf_send(s_out, &dst, dst_port, data, len);
    }
    DetUdpCall k;
    memset(&k, 0, sizeof(k));
    k.op = UDP_OP_SEND_OUT;
    k.addr = dst;
    k.port = dst_port;
    k.data = data;
    k.len = len;
    tcpip_api_call(udp_do, &k.base);
    return k.result;
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
    struct udp_pcb *pcb = nullptr;
    for (int i = 0; i < DETWS_MAX_UDP_LISTENERS; i++)
    {
        if (s_listeners[i].used && s_listeners[i].pcb && s_listeners[i].pcb->local_port == listen_port)
        {
            pcb = s_listeners[i].pcb;
            break;
        }
    }
    if (!pcb)
        return false;
    if (s_in_tcpip_thread)
        return udp_pbuf_send(pcb, &dst, dst_port, data, len);
    DetUdpCall k;
    memset(&k, 0, sizeof(k));
    k.op = UDP_OP_SEND;
    k.pcb = pcb;
    k.addr = dst;
    k.port = dst_port;
    k.data = data;
    k.len = len;
    tcpip_api_call(udp_do, &k.base);
    return k.result;
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
