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
