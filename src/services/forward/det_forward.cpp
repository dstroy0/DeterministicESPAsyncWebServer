// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_forward.cpp
 * @brief Interface forwarding plane - implementation.
 *
 * Static interface + rule tables. A frame on one interface is resolved against the rules
 * (a DENY wins, otherwise a matching ALLOW forwards, otherwise default-deny) for every
 * other registered interface, rate-capped per rule, then handed to that interface's send
 * callback. Zero heap, fail-closed.
 */

#include "services/forward/det_forward.h"

#if DETWS_ENABLE_FORWARD

#include <string.h>

#ifdef ARDUINO
#include "services/det_clock.h" // detws_millis()
#endif

namespace
{
struct iface
{
    det_if_send_fn send;
    void *ctx;
    uint8_t id;
    uint8_t kind;
    bool used;
};

struct rule
{
    uint32_t window_start; // ms of the current rate window
    uint16_t rate_cap;     // frames per second (0 = unlimited)
    uint16_t count;        // frames forwarded in the current window
    uint8_t src;
    uint8_t dst;
    uint8_t action;
    bool used;
};

iface s_if[DETWS_FWD_MAX_IFACES];
rule s_rule[DETWS_FWD_MAX_RULES];
det_forward_stats s_stats;

#ifdef ARDUINO
uint32_t fwd_now()
{
    return detws_millis();
}
#else
uint32_t s_now_ms = 0;
uint32_t fwd_now()
{
    return s_now_ms;
}
#endif

iface *find_if(uint8_t id)
{
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
        if (s_if[i].used && s_if[i].id == id)
            return &s_if[i];
    return nullptr;
}

// Resolve the action for (src -> dst): a DENY wins; otherwise the first matching ALLOW
// governs (its index is returned via @p allow_idx); otherwise default-deny (no route).
enum resolve_result
{
    R_NOROUTE,
    R_DENY,
    R_ALLOW,
};
resolve_result resolve(uint8_t src, uint8_t dst, int *allow_idx)
{
    int allow = -1;
    bool deny = false;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_RULES; i++)
    {
        if (!s_rule[i].used || s_rule[i].src != src || s_rule[i].dst != dst)
            continue;
        if (s_rule[i].action == DET_FWD_DENY)
            deny = true;
        else if (allow < 0)
            allow = (int)i;
    }
    if (deny)
        return R_DENY;
    if (allow >= 0)
    {
        *allow_idx = allow;
        return R_ALLOW;
    }
    return R_NOROUTE;
}

// Fixed 1-second window rate cap; fail-closed (returns true = drop) once the cap is hit.
bool rate_exceeded(rule *r)
{
    if (r->rate_cap == 0)
        return false; // unlimited
    uint32_t now = fwd_now();
    if ((uint32_t)(now - r->window_start) >= 1000)
    {
        r->window_start = now;
        r->count = 0;
    }
    if (r->count >= r->rate_cap)
        return true;
    r->count++;
    return false;
}
} // namespace

void det_forward_reset(void)
{
    memset(s_if, 0, sizeof(s_if));
    memset(s_rule, 0, sizeof(s_rule));
    memset(&s_stats, 0, sizeof(s_stats));
}

bool det_forward_add_if(uint8_t if_id, uint8_t kind, det_if_send_fn send, void *ctx)
{
    if (!send || find_if(if_id))
        return false;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
    {
        if (s_if[i].used)
            continue;
        s_if[i].send = send;
        s_if[i].ctx = ctx;
        s_if[i].id = if_id;
        s_if[i].kind = kind;
        s_if[i].used = true;
        return true;
    }
    return false; // table full
}

bool det_forward_add_rule(uint8_t src_if, uint8_t dst_if, uint8_t action, uint16_t rate_cap_per_sec)
{
    for (uint8_t i = 0; i < DETWS_FWD_MAX_RULES; i++)
    {
        if (s_rule[i].used)
            continue;
        s_rule[i].window_start = 0;
        s_rule[i].rate_cap = rate_cap_per_sec;
        s_rule[i].count = 0;
        s_rule[i].src = src_if;
        s_rule[i].dst = dst_if;
        s_rule[i].action = action;
        s_rule[i].used = true;
        return true;
    }
    return false; // table full
}

uint8_t det_forward_ingress(uint8_t src_if, const uint8_t *data, uint16_t len)
{
    s_stats.frames_in++;
    uint8_t n = 0;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
    {
        if (!s_if[i].used || s_if[i].id == src_if) // never reflect to the source interface
            continue;
        int idx = -1;
        resolve_result r = resolve(src_if, s_if[i].id, &idx);
        if (r == R_NOROUTE)
            continue; // default-deny, silent
        if (r == R_DENY)
        {
            s_stats.blocked++;
            continue;
        }
        if (rate_exceeded(&s_rule[idx]))
        {
            s_stats.rate_dropped++;
            continue;
        }
        if (s_if[i].send(s_if[i].id, data, len, s_if[i].ctx))
        {
            s_stats.forwarded++;
            n++;
        }
        else
        {
            s_stats.send_fail++;
        }
    }
    return n;
}

void det_forward_get_stats(det_forward_stats *out)
{
    if (out)
        *out = s_stats;
}

#if !defined(ARDUINO)
void det_forward_test_set_now(uint32_t ms)
{
    s_now_ms = ms;
}
#endif

#endif // DETWS_ENABLE_FORWARD
