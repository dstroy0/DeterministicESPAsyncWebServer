// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_delivery.cpp
 * @brief HTTP delivery optimizations (see http_delivery.h).
 */

#include "services/http_delivery/http_delivery.h"

#if DETWS_ENABLE_HTTP_DELIVERY

#include <string.h>

namespace
{
// Parse a run of decimal digits at *p (advancing it); write *out. Returns false on no digit or
// > 10 digits (uint32 overflow guard).
bool read_u32(const char **p, uint32_t *out)
{
    const char *s = *p;
    if (*s < '0' || *s > '9')
        return false; // GCOVR_EXCL_LINE both call sites pre-check *p is a digit before calling, so this never fires
    uint64_t v = 0;
    int n = 0;
    while (*s >= '0' && *s <= '9')
    {
        v = v * 10 + (uint32_t)(*s - '0');
        s++;
        n++;
        if (n > 10 || v > 0xFFFFFFFFu)
            return false;
    }
    *p = s;
    *out = (uint32_t)v;
    return true;
}

struct Buf
{
    char *p;
    size_t cap;
    size_t len;
    bool ok;
};

void put(Buf *b, const char *s)
{
    if (!b->ok)
        return;
    size_t sl = strlen(s);
    if (b->len + sl >= b->cap)
    {
        b->ok = false;
        return;
    }
    memcpy(b->p + b->len, s, sl);
    b->len += sl;
}

void put_u32(Buf *b, uint32_t v)
{
    char t[10];
    int n = 0;
    do
    {
        t[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    char o[11];
    for (int i = 0; i < n; i++)
        o[i] = t[n - 1 - i];
    o[n] = '\0';
    put(b, o);
}

void put_json_str(Buf *b, const char *s)
{
    put(b, "\"");
    for (const char *p = s ? s : ""; *p; p++)
    {
        if (*p == '"' || *p == '\\')
        {
            char esc[3] = {'\\', *p, '\0'};
            put(b, esc);
        }
        else if (b->len + 1 < b->cap)
            b->p[b->len++] = *p;
        else
            b->ok = false;
    }
    put(b, "\"");
}
} // namespace

int detws_delivery_swr(uint32_t age_s, uint32_t max_age_s, uint32_t swr_s)
{
    if (age_s <= max_age_s)
        return DELIVERY_FRESH;
    uint64_t window = (uint64_t)max_age_s + swr_s;
    if ((uint64_t)age_s <= window)
        return DELIVERY_STALE_REVALIDATE;
    return DELIVERY_EXPIRED;
}

size_t detws_delivery_cache_control(uint32_t max_age_s, uint32_t swr_s, char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    Buf b = {out, cap, 0, true};
    put(&b, "public, max-age=");
    put_u32(&b, max_age_s);
    if (swr_s)
    {
        put(&b, ", stale-while-revalidate=");
        put_u32(&b, swr_s);
    }
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

int detws_delivery_range(const char *range_header, uint32_t total, uint32_t *start, uint32_t *end)
{
    if (!range_header || !start || !end || total == 0)
        return 0;
    const char *p = range_header;
    while (*p == ' ' || *p == '\t')
        p++;
    if (strncmp(p, "bytes=", 6) != 0)
        return 0;
    p += 6;
    if (strchr(p, ',')) // multi-range: unsupported
        return 0;

    bool have_start = false, have_end = false;
    uint32_t s = 0, e = 0;
    if (*p >= '0' && *p <= '9')
    {
        if (!read_u32(&p, &s))
            return 0;
        have_start = true;
    }
    if (*p != '-')
        return 0;
    p++;
    if (*p >= '0' && *p <= '9')
    {
        if (!read_u32(&p, &e))
            return 0;
        have_end = true;
    }
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p != '\0' && *p != '\r' && *p != '\n')
        return 0;

    uint32_t rs = 0, re = 0;
    if (!have_start)
    {
        if (!have_end || e == 0) // "bytes=-" or "bytes=-0" are unsatisfiable
            return 0;
        rs = (e >= total) ? 0 : (total - e); // last e bytes
        re = total - 1;
    }
    else
    {
        rs = s;
        if (rs >= total) // 416: start past end of resource
            return 0;
        re = have_end ? e : (total - 1);
        if (re >= total)
            re = total - 1;
        if (rs > re)
            return 0;
    }
    *start = rs;
    *end = re;
    return 1;
}

size_t detws_delivery_content_range(uint32_t start, uint32_t end, uint32_t total, char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    Buf b = {out, cap, 0, true};
    put(&b, "bytes ");
    put_u32(&b, start);
    put(&b, "-");
    put_u32(&b, end);
    put(&b, "/");
    put_u32(&b, total);
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

size_t detws_delivery_sw_manifest(const char *const *paths, size_t n, const char *version, char *out, size_t cap)
{
    if (!out || cap == 0 || (n && !paths))
        return 0;
    Buf b = {out, cap, 0, true};
    put(&b, "{\"version\":");
    put_json_str(&b, version ? version : "");
    put(&b, ",\"precache\":[");
    for (size_t i = 0; i < n; i++)
    {
        if (i)
            put(&b, ",");
        put_json_str(&b, paths[i]);
    }
    put(&b, "]}");
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

#endif // DETWS_ENABLE_HTTP_DELIVERY
