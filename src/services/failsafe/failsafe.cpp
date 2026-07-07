// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file failsafe.cpp
 * @brief Software watchdog / deadlock detection + safe-state (see failsafe.h).
 */

#include "services/failsafe/failsafe.h"

#if DETWS_ENABLE_FAILSAFE

#include "services/clock.h" // detws_millis() - the pluggable monotonic clock

namespace
{
// All failsafe state, owned by one instance (internal linkage via the anon namespace):
// grouped for auditability, unreachable from any other translation unit.
struct FailsafeCtx
{
    DetwsLifeline lines[DETWS_FAILSAFE_MAX_LIFELINES];
    DetwsFailsafeCb cb = nullptr;
    void *cb_arg = nullptr;
};
FailsafeCtx s_fs;

// Minimal unsigned -> decimal, no stdlib; returns chars written.
size_t u32_dec(uint32_t v, char *out)
{
    char tmp[10];
    size_t n = 0;
    do
    {
        tmp[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    for (size_t i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    return n;
}
} // namespace

void detws_failsafe_reset(void)
{
    for (int i = 0; i < DETWS_FAILSAFE_MAX_LIFELINES; i++)
        s_fs.lines[i] = DetwsLifeline{};
    s_fs.cb = nullptr;
    s_fs.cb_arg = nullptr;
}

int detws_failsafe_register_at(const char *name, uint32_t deadline_ms, uint32_t now)
{
    for (int i = 0; i < DETWS_FAILSAFE_MAX_LIFELINES; i++)
    {
        if (!s_fs.lines[i].armed)
        {
            s_fs.lines[i].name = name;
            s_fs.lines[i].deadline_ms = deadline_ms;
            s_fs.lines[i].last_feed_ms = now; // starts fed, so it is not instantly overdue
            s_fs.lines[i].armed = true;
            s_fs.lines[i].breached = false;
            return i;
        }
    }
    return -1;
}

int detws_failsafe_register(const char *name, uint32_t deadline_ms)
{
    return detws_failsafe_register_at(name, deadline_ms, detws_millis());
}

bool detws_failsafe_feed_at(int id, uint32_t now)
{
    if (id < 0 || id >= DETWS_FAILSAFE_MAX_LIFELINES || !s_fs.lines[id].armed)
        return false;
    s_fs.lines[id].last_feed_ms = now;
    s_fs.lines[id].breached = false; // a fresh check-in clears the breach so it can fire again next time
    return true;
}

bool detws_failsafe_feed(int id)
{
    return detws_failsafe_feed_at(id, detws_millis());
}

void detws_failsafe_on_breach(DetwsFailsafeCb cb, void *arg)
{
    s_fs.cb = cb;
    s_fs.cb_arg = arg;
}

uint32_t detws_failsafe_check_at(uint32_t now)
{
    uint32_t mask = 0;
    for (int i = 0; i < DETWS_FAILSAFE_MAX_LIFELINES; i++)
    {
        DetwsLifeline &l = s_fs.lines[i];
        if (!l.armed)
            continue;
        if (detws_lifeline_overdue(now, l.last_feed_ms, l.deadline_ms))
        {
            mask |= (1u << i);
            if (!l.breached) // fire once per stuck episode
            {
                l.breached = true;
                if (s_fs.cb)
                    s_fs.cb(i, l.name, s_fs.cb_arg);
            }
        }
    }
    return mask;
}

uint32_t detws_failsafe_check(void)
{
    return detws_failsafe_check_at(detws_millis());
}

int detws_failsafe_json_at(uint32_t now, char *out, size_t cap)
{
    // {"lifelines":[{"name":"...","overdue":false,"age_ms":N,"deadline_ms":N},...]}
    if (!out || cap == 0)
        return 0;
    size_t n = 0;
    // append a literal; truncates safely if it would overflow.
    auto put = [&](const char *s) {
        while (*s && n + 1 < cap)
            out[n++] = *s++;
    };
    auto put_u32 = [&](uint32_t v) {
        char b[10];
        size_t k = u32_dec(v, b);
        for (size_t i = 0; i < k && n + 1 < cap; i++)
            out[n++] = b[i];
    };
    put("{\"lifelines\":[");
    bool first = true;
    for (int i = 0; i < DETWS_FAILSAFE_MAX_LIFELINES; i++)
    {
        DetwsLifeline &l = s_fs.lines[i];
        if (!l.armed)
            continue;
        if (!first)
            put(",");
        first = false;
        put("{\"name\":\"");
        put(l.name ? l.name : "");
        put("\",\"overdue\":");
        put(detws_lifeline_overdue(now, l.last_feed_ms, l.deadline_ms) ? "true" : "false");
        put(",\"age_ms\":");
        put_u32((uint32_t)(now - l.last_feed_ms));
        put(",\"deadline_ms\":");
        put_u32(l.deadline_ms);
        put("}");
    }
    put("]}");
    out[n < cap ? n : cap - 1] = '\0';
    return (int)n;
}

#endif // DETWS_ENABLE_FAILSAFE
