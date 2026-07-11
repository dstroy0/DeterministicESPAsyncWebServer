// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file gateway.cpp
 * @brief Radio / wireless gateway bridge - implementation.
 *
 * A static port table; det_gw_uplink() envelopes a received frame and publishes it through
 * the installed northbound callback (per-port rate-capped, fail-closed), det_gw_downlink()
 * routes a command to a port's transmit callback, and det_gw_topic() formats a routing key.
 * Zero heap.
 */

#include "services/gateway/gateway.h"

#if DETWS_ENABLE_GATEWAY

#include <string.h>

#ifdef ARDUINO
#include "services/clock.h" // detws_millis()
#endif

namespace
{
struct port
{
    det_gw_tx_fn tx;
    void *ctx;
    uint32_t window_start; // ms of the current uplink rate window
    uint16_t rate_cap;     // uplink frames per second (0 = unlimited)
    uint16_t count;        // uplinks in the current window
    uint8_t id;
    det_gw_kind kind;
    bool used;
};

// All gateway state, owned by one instance (internal linkage): the port table, the
// northbound uplink callback + context, the topic prefix, the uplink sequence, and stats,
// grouped so it is one named owner, unreachable from any other translation unit.
struct GatewayCtx
{
    port ports[DETWS_GW_MAX_PORTS];
    det_gw_uplink_fn uplink = nullptr;
    void *uplink_ctx = nullptr;
    const char *prefix = DETWS_GW_DEFAULT_PREFIX;
    uint32_t seq = 0;
    det_gw_stats stats;
#ifndef ARDUINO
    uint32_t now_ms = 0; // host test clock (real builds use detws_millis())
#endif
};
GatewayCtx s_gw;

#ifdef ARDUINO
uint32_t gw_now()
{
    return detws_millis();
}
#else
uint32_t gw_now()
{
    return s_gw.now_ms;
}
#endif

// Returns a mutable port (callers mutate it), so it takes the owner by non-const reference.
port *find_port(GatewayCtx &g, uint8_t id)
{
    for (uint8_t i = 0; i < DETWS_GW_MAX_PORTS; i++)
        if (g.ports[i].used && g.ports[i].id == id)
            return &g.ports[i];
    return nullptr;
}

// Fixed 1-second window uplink rate cap; fail-closed (true = drop) once the cap is hit.
bool rate_exceeded(port *p)
{
    if (p->rate_cap == 0)
        return false;
    uint32_t now = gw_now();
    if ((uint32_t)(now - p->window_start) >= 1000)
    {
        p->window_start = now;
        p->count = 0;
    }
    if (p->count >= p->rate_cap)
        return true;
    p->count++;
    return false;
}

bool put_ch(char *buf, uint16_t *pos, uint16_t cap, char c)
{
    if ((uint16_t)(*pos + 1) >= cap) // keep room for the NUL
        return false;
    buf[(*pos)++] = c;
    return true;
}

bool put_u32(char *buf, uint16_t *pos, uint16_t cap, uint32_t v)
{
    char tmp[10];
    uint8_t n = 0;
    if (v == 0)
        tmp[n++] = '0';
    while (v)
    {
        tmp[n++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (n > 0)
        if (!put_ch(buf, pos, cap, tmp[--n]))
            return false;
    return true;
}
} // namespace

void det_gw_reset(void)
{
    memset(s_gw.ports, 0, sizeof(s_gw.ports));
    s_gw.uplink = nullptr;
    s_gw.uplink_ctx = nullptr;
    s_gw.prefix = DETWS_GW_DEFAULT_PREFIX;
    s_gw.seq = 0;
    memset(&s_gw.stats, 0, sizeof(s_gw.stats));
}

bool det_gw_add_port(const det_gw_port_config *cfg)
{
    if (!cfg || find_port(s_gw, cfg->port_id))
        return false;
    for (uint8_t i = 0; i < DETWS_GW_MAX_PORTS; i++)
    {
        if (s_gw.ports[i].used)
            continue;
        s_gw.ports[i].tx = cfg->tx;
        s_gw.ports[i].ctx = cfg->ctx;
        s_gw.ports[i].window_start = 0;
        s_gw.ports[i].rate_cap = cfg->rate_cap;
        s_gw.ports[i].count = 0;
        s_gw.ports[i].id = cfg->port_id;
        s_gw.ports[i].kind = cfg->kind;
        s_gw.ports[i].used = true;
        return true;
    }
    return false; // table full
}

void det_gw_set_uplink(det_gw_uplink_fn fn, void *ctx)
{
    s_gw.uplink = fn;
    s_gw.uplink_ctx = ctx;
}

void det_gw_set_topic_prefix(const char *prefix)
{
    s_gw.prefix = prefix ? prefix : DETWS_GW_DEFAULT_PREFIX;
}

bool det_gw_uplink(uint8_t port_id, uint16_t src_addr, const uint8_t *payload, uint16_t len, int16_t rssi)
{
    s_gw.stats.up_in++;
    port *p = find_port(s_gw, port_id);
    if (!p || !s_gw.uplink || rate_exceeded(p))
    {
        s_gw.stats.up_dropped++;
        return false;
    }
    det_gw_msg msg;
    msg.payload = payload;
    msg.seq = s_gw.seq++;
    msg.len = len;
    msg.src_addr = src_addr;
    msg.rssi = rssi;
    msg.port_id = port_id;
    msg.kind = p->kind;
    if (s_gw.uplink(&msg, s_gw.uplink_ctx))
    {
        s_gw.stats.up_published++;
        return true;
    }
    s_gw.stats.up_dropped++;
    return false;
}

bool det_gw_downlink(uint8_t port_id, uint16_t dst_addr, const uint8_t *payload, uint16_t len)
{
    s_gw.stats.down_in++;
    port *p = find_port(s_gw, port_id);
    if (!p || !p->tx || !p->tx(port_id, dst_addr, payload, len, p->ctx))
    {
        s_gw.stats.down_dropped++;
        return false;
    }
    s_gw.stats.down_sent++;
    return true;
}

uint16_t det_gw_topic(const det_gw_msg *msg, char *buf, uint16_t buflen)
{
    if (!msg || !buf || buflen == 0)
        return 0;
    uint16_t pos = 0;
    for (const char *s = s_gw.prefix; *s; s++)
        if (!put_ch(buf, &pos, buflen, *s))
            return 0;
    // Sequential, not one `||` chain: the two '/' separators are written at different
    // positions (put_ch advances pos), so writing them as separate steps keeps each
    // append distinct rather than repeating an identical-looking subexpression.
    if (!put_ch(buf, &pos, buflen, '/'))
        return 0;
    if (!put_u32(buf, &pos, buflen, msg->port_id))
        return 0;
    if (!put_ch(buf, &pos, buflen, '/'))
        return 0;
    if (!put_u32(buf, &pos, buflen, msg->src_addr))
        return 0;
    buf[pos] = '\0';
    return pos;
}

void det_gw_get_stats(det_gw_stats *out)
{
    if (out)
        *out = s_gw.stats;
}

#if !defined(ARDUINO)
void det_gw_test_set_now(uint32_t ms)
{
    s_gw.now_ms = ms;
}
#endif

#endif // DETWS_ENABLE_GATEWAY
