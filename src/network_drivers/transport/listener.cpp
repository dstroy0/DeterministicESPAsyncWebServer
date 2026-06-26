// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file listener.cpp
 * @brief Layer 4 (Listener) - TCP accept callback and port lifecycle.
 *
 * `listener_accept_cb` is the single lwIP accept callback registered for
 * every listener.  The listener index is embedded in the PCB user-data via
 * `tcp_arg(listen_pcb, (void*)(uintptr_t)idx)` so this one function handles
 * all ports without a lookup table.
 *
 * The non-static per-connection callbacks (lowlevel_recv_cb, lowlevel_sent_cb,
 * lowlevel_err_cb) are defined in transport.cpp and declared extern here.
 * The transport layer's enqueue() helper calls listener_enqueue(), which is
 * defined in this file - that indirection breaks the circular header dependency
 * (listener.h includes transport.h; transport.cpp includes listener.h).
 */

#include "listener.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"
#include "network_drivers/tls/det_tls.h" // TLS handshake begin (self-stubbing)
#ifdef ARDUINO
#include "lwip/def.h"     // lwip_ntohl - allowlist host-order conversion
#include "lwip/ip_addr.h" // ip_2_ip4 / ip4_addr_get_u32 for interface tagging
#endif
#include <Arduino.h>

// Listener pool - all storage in BSS.
Listener listener_pool[MAX_LISTENERS];

// Per-connection callbacks defined in transport.cpp.
extern err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
extern err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);
extern void lowlevel_err_cb(void *arg, err_t err);

// ---------------------------------------------------------------------------
// Accept-rate throttle (fixed window, global). State persists across accepts.
// Always compiled (unit-testable); only consulted when the feature is enabled.
// ---------------------------------------------------------------------------

static uint32_t g_accept_window_start = 0;
static uint16_t g_accept_count = 0;

bool listener_accept_allowed(uint32_t now_ms)
{
    // Unsigned subtraction wraps correctly across the millis() rollover.
    if ((uint32_t)(now_ms - g_accept_window_start) >= DETWS_ACCEPT_THROTTLE_WINDOW_MS)
    {
        g_accept_window_start = now_ms;
        g_accept_count = 0;
    }
    if (g_accept_count >= DETWS_ACCEPT_THROTTLE_MAX)
        return false;
    g_accept_count++;
    return true;
}

void listener_accept_throttle_reset(void)
{
    g_accept_window_start = 0;
    g_accept_count = 0;
}

// ---------------------------------------------------------------------------
// Per-IP accept-rate throttle (fixed window per source IPv4). A bounded BSS table
// of buckets - no heap. Always compiled (unit-testable); only consulted when the
// feature is enabled.
// ---------------------------------------------------------------------------

struct IpThrottleBucket
{
    uint32_t ip;           ///< source IPv4 word; 0 marks an empty bucket.
    uint32_t window_start; ///< millis() at the start of this bucket's current window.
    uint16_t count;        ///< connections counted from this address in the window.
};
static IpThrottleBucket g_ip_buckets[DETWS_PER_IP_THROTTLE_SLOTS];

bool listener_accept_allowed_ip(uint32_t ip, uint32_t now_ms)
{
    if (ip == 0)
        return true; // untrackable source (0 is the empty-bucket sentinel) - defer to the global throttle

    int empty = -1, expired = -1, lru = 0;
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_SLOTS; i++)
    {
        IpThrottleBucket *b = &g_ip_buckets[i];
        if (b->ip == ip)
        {
            // Unsigned subtraction wraps correctly across the millis() rollover.
            if ((uint32_t)(now_ms - b->window_start) >= DETWS_PER_IP_THROTTLE_WINDOW_MS)
            {
                b->window_start = now_ms;
                b->count = 0;
            }
            if (b->count >= DETWS_PER_IP_THROTTLE_MAX)
                return false;
            b->count++;
            return true;
        }
        if (b->ip == 0)
        {
            if (empty < 0)
                empty = i;
        }
        else
        {
            if (expired < 0 && (uint32_t)(now_ms - b->window_start) >= DETWS_PER_IP_THROTTLE_WINDOW_MS)
                expired = i;
            // Track the oldest active bucket (largest elapsed) as the eviction victim.
            if ((uint32_t)(now_ms - b->window_start) > (uint32_t)(now_ms - g_ip_buckets[lru].window_start))
                lru = i;
        }
    }

    // No bucket yet for this address: claim one - empty, else expired, else evict
    // the least-recently-started active bucket.
    int slot = (empty >= 0) ? empty : (expired >= 0) ? expired : lru;
    IpThrottleBucket *b = &g_ip_buckets[slot];
    b->ip = ip;
    b->window_start = now_ms;
    b->count = 1;
    return true; // first connection of a fresh window is always allowed
}

void listener_per_ip_throttle_reset(void)
{
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_SLOTS; i++)
    {
        g_ip_buckets[i].ip = 0;
        g_ip_buckets[i].window_start = 0;
        g_ip_buckets[i].count = 0;
    }
}

// ---------------------------------------------------------------------------
// Source-IP allowlist (accept-time firewall). A bounded BSS table of CIDR rules
// in host byte order. Always compiled (unit-testable); only consulted when
// DETWS_ENABLE_IP_ALLOWLIST is set. An empty table allows everything so enabling
// the feature before adding rules cannot lock the device out.
// ---------------------------------------------------------------------------

struct IpAllowRule
{
    uint32_t network; ///< host-order network address, already masked to the prefix.
    uint32_t mask;    ///< host-order netmask derived from the prefix length.
};
static IpAllowRule g_ip_allow[DETWS_IP_ALLOWLIST_SLOTS];
static uint8_t g_ip_allow_count = 0;

bool listener_ip_allow_add(uint32_t network, uint8_t prefix_len)
{
    if (prefix_len > 32)
        return false;
    if (g_ip_allow_count >= DETWS_IP_ALLOWLIST_SLOTS)
        return false;
    // prefix_len 0 -> mask 0 (matches all); 1..32 -> top prefix_len bits set.
    // (a full 32-bit shift is undefined, so the zero-prefix case is handled apart.)
    uint32_t mask = (prefix_len == 0) ? 0u : (0xFFFFFFFFu << (32 - prefix_len));
    g_ip_allow[g_ip_allow_count].network = network & mask;
    g_ip_allow[g_ip_allow_count].mask = mask;
    g_ip_allow_count++;
    return true;
}

bool listener_ip_allowed(uint32_t ip)
{
    if (g_ip_allow_count == 0)
        return true; // no rules configured -> allow all (fail-open by design)
    for (uint8_t i = 0; i < g_ip_allow_count; i++)
    {
        if ((ip & g_ip_allow[i].mask) == g_ip_allow[i].network)
            return true;
    }
    return false;
}

void listener_ip_allowlist_reset(void)
{
    for (int i = 0; i < DETWS_IP_ALLOWLIST_SLOTS; i++)
    {
        g_ip_allow[i].network = 0;
        g_ip_allow[i].mask = 0;
    }
    g_ip_allow_count = 0;
}

void listener_enqueue(uint8_t listener_id, const TcpEvt *evt)
{
    if (listener_id >= MAX_LISTENERS)
        return;
    Listener *lst = &listener_pool[listener_id];
    if (!lst->active || !lst->queue)
        return;
    xQueueSend(lst->queue, evt, 0);
}

/**
 * @brief lwIP accept callback - single handler for all listener ports.
 *
 * @p arg carries the listener index cast to a pointer via
 * `tcp_arg(listen_pcb, (void*)(uintptr_t)idx)`.  Finds a free TcpConn slot,
 * sets its protocol, wires the per-connection callbacks, and posts EVT_CONNECT
 * to the owning listener's queue.  Rejects the connection with ERR_ABRT when
 * the pool is full - ERR_ABRT tells lwIP the PCB is already gone from our side.
 */
static err_t listener_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    if (err != ERR_OK || newpcb == nullptr)
        return ERR_VAL;

    uint8_t idx = (uint8_t)(uintptr_t)arg;
    if (idx >= MAX_LISTENERS)
        return ERR_VAL;
    Listener *lst = &listener_pool[idx];

#if DETWS_ENABLE_ACCEPT_THROTTLE
    // Connection-flood defense: drop accepts beyond the per-window budget before
    // claiming a pool slot or doing any per-connection work.
    if (!listener_accept_allowed(millis()))
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }
#endif

#if DETWS_ENABLE_PER_IP_THROTTLE
    // Per-source-IP flood defense: drop accepts beyond one address's per-window
    // budget (the global throttle cannot tell one noisy client from many).
    {
        uint32_t rip = 0;
#ifdef ARDUINO
        rip = ip4_addr_get_u32(ip_2_ip4(&newpcb->remote_ip));
#endif
        if (!listener_accept_allowed_ip(rip, millis()))
        {
            tcp_abort(newpcb);
            return ERR_ABRT;
        }
    }
#endif

#if DETWS_ENABLE_IP_ALLOWLIST
    // Source-IP firewall: drop connections from addresses outside the configured
    // allowlist (an empty allowlist allows all, so this is a no-op until rules are
    // added). Uses host byte order; lwIP stores remote_ip in network order.
    {
        uint32_t rip_host = 0;
#ifdef ARDUINO
        rip_host = lwip_ntohl(ip4_addr_get_u32(ip_2_ip4(&newpcb->remote_ip)));
#endif
        if (!listener_ip_allowed(rip_host))
        {
            tcp_abort(newpcb);
            return ERR_ABRT;
        }
    }
#endif

    int free_slot = -1;
    for (int i = 0; i < MAX_CONNS; i++)
    {
        if (conn_pool[i].state == CONN_FREE)
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }

    TcpConn *slot = &conn_pool[free_slot];
    slot->state = CONN_ACTIVE;
    slot->pcb = newpcb;
    slot->last_activity_ms = millis();
    slot->rx_head = 0;
    slot->rx_tail = 0;
    slot->listener_id = idx;
    slot->proto = lst->proto;

    // Tag the ingress interface for per-route STA/AP filtering. On ESP32 compare
    // the connection's local IP to the configured softAP IP; on native (no real
    // pcb IP) leave it unclassified for tests to set directly.
#ifdef ARDUINO
    {
        uint32_t lip = ip4_addr_get_u32(ip_2_ip4(&newpcb->local_ip));
        slot->iface = (detws_ap_ip != 0 && lip == detws_ap_ip) ? DETIFACE_AP : DETIFACE_STA;
    }
#else
    slot->iface = DETIFACE_ANY;
#endif

    tcp_arg(newpcb, slot);

#if DETWS_ENABLE_TLS
    // TLS listeners begin a handshake immediately; the session loop pumps it.
    slot->tls = lst->tls ? 1 : 0;
    if (lst->tls)
        det_tls_conn_begin(free_slot);
#else
    slot->tls = 0;
#endif
    tcp_recv(newpcb, lowlevel_recv_cb);
    tcp_sent(newpcb, lowlevel_sent_cb);
    tcp_err(newpcb, lowlevel_err_cb);

    TcpEvt evt = {EVT_CONNECT, (uint8_t)free_slot, 0};
    listener_enqueue(idx, &evt);

    return ERR_OK;
}

int32_t listener_add(uint8_t idx, uint16_t port, ConnProto proto, bool tls)
{
    if (idx >= MAX_LISTENERS)
        return -1;

    listener_stop(idx); // clean up if already active

    Listener *lst = &listener_pool[idx];
    lst->port = port;
    lst->proto = proto;
    lst->tls = tls;

    lst->queue = xQueueCreateStatic(EVT_QUEUE_DEPTH, sizeof(TcpEvt), lst->_queue_storage, &lst->_queue_struct);
    if (!lst->queue)
        return -1;

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb)
        return -1;

    err_t bind_err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (bind_err != ERR_OK)
    {
        tcp_abort(pcb);
        return -1;
    }

    lst->listen_pcb = tcp_listen_with_backlog(pcb, MAX_CONNS);
    if (!lst->listen_pcb)
    {
        tcp_abort(pcb);
        return -1;
    }

    tcp_arg(lst->listen_pcb, (void *)(uintptr_t)idx);
    tcp_accept(lst->listen_pcb, listener_accept_cb);
    lst->active = true;

    return 1;
}

void listener_stop(uint8_t idx)
{
    if (idx >= MAX_LISTENERS)
        return;
    Listener *lst = &listener_pool[idx];
    if (!lst->active)
        return;
    lst->active = false;
    if (lst->listen_pcb)
    {
        tcp_close(lst->listen_pcb);
        lst->listen_pcb = nullptr;
    }
    if (lst->queue)
    {
        vQueueDelete(lst->queue);
        lst->queue = nullptr;
    }
}

void listener_stop_all()
{
    for (uint8_t i = 0; i < MAX_LISTENERS; i++)
        listener_stop(i);
}
