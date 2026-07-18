// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_delivery.cpp
 * @brief HTTP delivery optimizations (see http_delivery.h).
 */

#include "services/http_delivery/http_delivery.h"

#if DETWS_ENABLE_HTTP_DELIVERY

#include <string.h>

#include "shared_primitives/strbuf.h"

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

// Parse "bytes=<s>-<e>" (either bound optional) from a Range header. Fills the out-params and returns
// true, or returns false for a malformed / multi-range / non-bytes header. *have_start / *have_end flag
// which bounds were present; resolving them against the resource size is the caller's job.
bool parse_range_spec(const char *range_header, bool *have_start, bool *have_end, uint32_t *s, uint32_t *e)
{
    const char *p = range_header;
    while (*p == ' ' || *p == '\t')
        p++;
    if (strncmp(p, "bytes=", 6) != 0)
        return false;
    p += 6;
    if (strchr(p, ',')) // multi-range: unsupported
        return false;

    *have_start = false;
    *have_end = false;
    *s = 0;
    *e = 0;
    if (*p >= '0' && *p <= '9')
    {
        if (!read_u32(&p, s))
            return false;
        *have_start = true;
    }
    if (*p != '-')
        return false;
    p++;
    if (*p >= '0' && *p <= '9')
    {
        if (!read_u32(&p, e))
            return false;
        *have_end = true;
    }
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p != '\0' && *p != '\r' && *p != '\n')
        return false;
    return true;
}
} // namespace

DeliveryVerdict detws_delivery_swr(uint32_t age_s, uint32_t max_age_s, uint32_t swr_s)
{
    if (age_s <= max_age_s)
        return DeliveryVerdict::DELIVERY_FRESH;
    uint64_t window = (uint64_t)max_age_s + swr_s;
    if ((uint64_t)age_s <= window)
        return DeliveryVerdict::DELIVERY_STALE_REVALIDATE;
    return DeliveryVerdict::DELIVERY_EXPIRED;
}

size_t detws_delivery_cache_control(uint32_t max_age_s, uint32_t swr_s, char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    DetSb b = {out, cap, 0, true};
    det_sb_put(&b, "public, max-age=");
    det_sb_u32(&b, max_age_s);
    if (swr_s)
    {
        det_sb_put(&b, ", stale-while-revalidate=");
        det_sb_u32(&b, swr_s);
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

    bool have_start = false;
    bool have_end = false;
    uint32_t s = 0;
    uint32_t e = 0;
    if (!parse_range_spec(range_header, &have_start, &have_end, &s, &e))
        return 0;

    uint32_t rs = 0;
    uint32_t re = 0;
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
    DetSb b = {out, cap, 0, true};
    det_sb_put(&b, "bytes ");
    det_sb_u32(&b, start);
    det_sb_put(&b, "-");
    det_sb_u32(&b, end);
    det_sb_put(&b, "/");
    det_sb_u32(&b, total);
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

size_t detws_delivery_sw_manifest(const char *const *paths, size_t n, const char *version, char *out, size_t cap)
{
    if (!out || cap == 0 || (n && !paths))
        return 0;
    DetSb b = {out, cap, 0, true};
    det_sb_put(&b, "{\"version\":");
    det_sb_json(&b, version ? version : "");
    det_sb_put(&b, ",\"precache\":[");
    for (size_t i = 0; i < n; i++)
    {
        if (i)
            det_sb_put(&b, ",");
        det_sb_json(&b, paths[i]);
    }
    det_sb_put(&b, "]}");
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

#endif // DETWS_ENABLE_HTTP_DELIVERY
