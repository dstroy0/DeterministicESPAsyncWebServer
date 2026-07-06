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

struct acl_entry
{
    uint8_t pattern[DETWS_FWD_ACL_PATLEN];
    uint8_t mask[DETWS_FWD_ACL_PATLEN];
    uint16_t offset;
    uint8_t src;    // source interface, or DET_FWD_IF_ANY
    uint8_t patlen; // 0 = match any content
    uint8_t action;
    bool used;
};

// All forwarding-plane state, owned by one instance (internal linkage): interfaces,
// rules, ACL, and stats grouped so it is one named owner, unreachable cross-TU.
struct ForwardCtx
{
    iface if_[DETWS_FWD_MAX_IFACES];
    rule rules[DETWS_FWD_MAX_RULES];
    acl_entry acl[DETWS_FWD_MAX_ACL];
    uint8_t acl_default = DET_FWD_ALLOW; // frames matching no ACL entry (opt-in ACL)
    det_forward_stats stats;
#ifndef ARDUINO
    uint32_t now_ms = 0; // host test clock (real builds use detws_millis())
#endif
};
ForwardCtx s_fwd;

#ifdef ARDUINO
uint32_t fwd_now()
{
    return detws_millis();
}
#else
uint32_t fwd_now()
{
    return s_fwd.now_ms;
}
#endif

const iface *find_if(const ForwardCtx &f, uint8_t id)
{
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
        if (f.if_[i].used && f.if_[i].id == id)
            return &f.if_[i];
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
resolve_result resolve(const ForwardCtx &f, uint8_t src, uint8_t dst, int *allow_idx)
{
    int allow = -1;
    bool deny = false;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_RULES; i++)
    {
        if (!f.rules[i].used || f.rules[i].src != src || f.rules[i].dst != dst)
            continue;
        if (f.rules[i].action == DET_FWD_DENY)
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

// Does an ACL entry match this frame? (interface + byte pattern under mask). A frame too
// short for the pattern does not match, so evaluation falls through to the next entry.
bool acl_match(const acl_entry *a, uint8_t src, const uint8_t *data, uint16_t len)
{
    if (a->src != DET_FWD_IF_ANY && a->src != src)
        return false;
    if (a->patlen == 0)
        return true; // interface-only entry, any content
    if ((uint32_t)a->offset + a->patlen > len)
        return false;
    for (uint8_t i = 0; i < a->patlen; i++)
        if ((data[a->offset + i] & a->mask[i]) != a->pattern[i])
            return false;
    return true;
}

// Ingress ACL: the first matching entry's action decides; otherwise the default.
bool acl_permits(const ForwardCtx &f, uint8_t src, const uint8_t *data, uint16_t len)
{
    for (uint8_t i = 0; i < DETWS_FWD_MAX_ACL; i++)
        if (f.acl[i].used && acl_match(&f.acl[i], src, data, len))
            return f.acl[i].action == DET_FWD_ALLOW;
    return f.acl_default == DET_FWD_ALLOW;
}
} // namespace

void det_forward_reset(void)
{
    memset(s_fwd.if_, 0, sizeof(s_fwd.if_));
    memset(s_fwd.rules, 0, sizeof(s_fwd.rules));
    memset(s_fwd.acl, 0, sizeof(s_fwd.acl));
    s_fwd.acl_default = DET_FWD_ALLOW;
    memset(&s_fwd.stats, 0, sizeof(s_fwd.stats));
}

void det_forward_acl_set_default(uint8_t action)
{
    s_fwd.acl_default = action;
}

bool det_forward_acl_add(uint8_t src_if, uint16_t offset, const uint8_t *pattern, const uint8_t *mask, uint8_t patlen,
                         uint8_t action)
{
    if (patlen > DETWS_FWD_ACL_PATLEN || (patlen > 0 && (!pattern || !mask)))
        return false;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_ACL; i++)
    {
        if (s_fwd.acl[i].used)
            continue;
        memset(s_fwd.acl[i].pattern, 0, sizeof(s_fwd.acl[i].pattern));
        memset(s_fwd.acl[i].mask, 0, sizeof(s_fwd.acl[i].mask));
        for (uint8_t k = 0; k < patlen; k++)
        {
            s_fwd.acl[i].pattern[k] = (uint8_t)(pattern[k] & mask[k]); // store already masked
            s_fwd.acl[i].mask[k] = mask[k];
        }
        s_fwd.acl[i].offset = offset;
        s_fwd.acl[i].src = src_if;
        s_fwd.acl[i].patlen = patlen;
        s_fwd.acl[i].action = action;
        s_fwd.acl[i].used = true;
        return true;
    }
    return false; // table full
}

bool det_forward_add_if(uint8_t if_id, uint8_t kind, det_if_send_fn send, void *ctx)
{
    if (!send || find_if(s_fwd, if_id))
        return false;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
    {
        if (s_fwd.if_[i].used)
            continue;
        s_fwd.if_[i].send = send;
        s_fwd.if_[i].ctx = ctx;
        s_fwd.if_[i].id = if_id;
        s_fwd.if_[i].kind = kind;
        s_fwd.if_[i].used = true;
        return true;
    }
    return false; // table full
}

bool det_forward_add_rule(uint8_t src_if, uint8_t dst_if, uint8_t action, uint16_t rate_cap_per_sec)
{
    for (uint8_t i = 0; i < DETWS_FWD_MAX_RULES; i++)
    {
        if (s_fwd.rules[i].used)
            continue;
        s_fwd.rules[i].window_start = 0;
        s_fwd.rules[i].rate_cap = rate_cap_per_sec;
        s_fwd.rules[i].count = 0;
        s_fwd.rules[i].src = src_if;
        s_fwd.rules[i].dst = dst_if;
        s_fwd.rules[i].action = action;
        s_fwd.rules[i].used = true;
        return true;
    }
    return false; // table full
}

uint8_t det_forward_ingress(uint8_t src_if, const uint8_t *data, uint16_t len)
{
    s_fwd.stats.frames_in++;
    if (!acl_permits(s_fwd, src_if, data, len)) // ingress ACL runs before any forwarding rule
    {
        s_fwd.stats.acl_denied++;
        return 0;
    }
    uint8_t n = 0;
    for (uint8_t i = 0; i < DETWS_FWD_MAX_IFACES; i++)
    {
        if (!s_fwd.if_[i].used || s_fwd.if_[i].id == src_if) // never reflect to the source interface
            continue;
        int idx = -1;
        resolve_result r = resolve(s_fwd, src_if, s_fwd.if_[i].id, &idx);
        if (r == R_NOROUTE)
            continue; // default-deny, silent
        if (r == R_DENY)
        {
            s_fwd.stats.blocked++;
            continue;
        }
        if (rate_exceeded(&s_fwd.rules[idx]))
        {
            s_fwd.stats.rate_dropped++;
            continue;
        }
        if (s_fwd.if_[i].send(s_fwd.if_[i].id, data, len, s_fwd.if_[i].ctx))
        {
            s_fwd.stats.forwarded++;
            n++;
        }
        else
        {
            s_fwd.stats.send_fail++;
        }
    }
    return n;
}

void det_forward_get_stats(det_forward_stats *out)
{
    if (out)
        *out = s_fwd.stats;
}

#if !defined(ARDUINO)
void det_forward_test_set_now(uint32_t ms)
{
    s_fwd.now_ms = ms;
}
#endif

#endif // DETWS_ENABLE_FORWARD
