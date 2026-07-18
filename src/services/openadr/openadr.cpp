// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file openadr.cpp
 * @brief OpenADR 3.0 JSON codec (see openadr.h).
 */

#include "services/openadr/openadr.h"

#if DWS_ENABLE_OPENADR

#include <string.h>

#include "shared_primitives/strbuf.h"

namespace
{
void put_json_str(DWSSb *b, const char *s)
{
    dws_sb_put(b, "\"");
    for (const char *p = s ? s : ""; *p; p++)
    {
        if (*p == '"' || *p == '\\')
        {
            char esc[3] = {'\\', *p, '\0'};
            dws_sb_put(b, esc);
        }
        else if (*p == '\n')
            dws_sb_put(b, "\\n");
        else
        {
            if (b->len + 1 >= b->cap)
            {
                b->ok = false;
                return;
            }
            b->p[b->len++] = *p;
        }
    }
    dws_sb_put(b, "\"");
}

void put_u64(DWSSb *b, uint64_t v)
{
    char tmp[21];
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + (int)(v % 10));
        v /= 10;
    } while (v);
    char out[22];
    for (int i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    dws_sb_put(b, out);
}

// Format a double with 3 decimal places (no stdlib). Rounds to milli-units; handles the sign.
void put_double(DWSSb *b, double v)
{
    if (v < 0)
    {
        dws_sb_put(b, "-");
        v = -v;
    }
    // scale by 1000 and round.
    uint64_t scaled = (uint64_t)(v * 1000.0 + 0.5);
    uint64_t whole = scaled / 1000;
    uint32_t frac = (uint32_t)(scaled % 1000);
    put_u64(b, whole);
    dws_sb_put(b, ".");
    // three digits, zero-padded.
    char f[4] = {(char)('0' + (frac / 100) % 10), (char)('0' + (frac / 10) % 10), (char)('0' + frac % 10), '\0'};
    dws_sb_put(b, f);
}
} // namespace

size_t dws_openadr_event(const char *program_id, const char *event_name, const OpenAdrInterval *intervals, size_t count,
                         char *out, size_t cap)
{
    if (!out || (count && !intervals))
        return 0;
    DWSSb b = {out, cap, 0, cap > 0};
    dws_sb_put(&b, "{\"objectType\":\"EVENT\",\"programID\":");
    put_json_str(&b, program_id);
    dws_sb_put(&b, ",\"eventName\":");
    put_json_str(&b, event_name);
    dws_sb_put(&b, ",\"intervals\":[");
    for (size_t i = 0; i < count; i++)
    {
        if (i)
            dws_sb_put(&b, ",");
        dws_sb_put(&b, "{\"id\":");
        put_u64(&b, i);
        dws_sb_put(&b, ",\"interval\":{\"start\":");
        put_u64(&b, intervals[i].start);
        dws_sb_put(&b, ",\"duration\":");
        put_u64(&b, intervals[i].duration);
        dws_sb_put(&b, "},\"payloads\":[{\"type\":");
        put_json_str(&b, intervals[i].type);
        dws_sb_put(&b, ",\"values\":[");
        put_double(&b, intervals[i].value);
        dws_sb_put(&b, "]}]}");
    }
    dws_sb_put(&b, "]}");
    return dws_sb_finish(&b);
}

size_t dws_openadr_report(const char *program_id, const char *event_id, const char *resource_name, double value,
                          uint32_t timestamp, char *out, size_t cap)
{
    if (!out)
        return 0;
    DWSSb b = {out, cap, 0, cap > 0};
    dws_sb_put(&b, "{\"objectType\":\"REPORT\",\"programID\":");
    put_json_str(&b, program_id);
    dws_sb_put(&b, ",\"eventID\":");
    put_json_str(&b, event_id);
    dws_sb_put(&b, ",\"resources\":[{\"resourceName\":");
    put_json_str(&b, resource_name);
    dws_sb_put(&b, ",\"intervals\":[{\"interval\":{\"start\":");
    put_u64(&b, timestamp);
    dws_sb_put(&b, "},\"payloads\":[{\"type\":\"READING\",\"values\":[");
    put_double(&b, value);
    dws_sb_put(&b, "]}]}]}]}");
    return dws_sb_finish(&b);
}

#endif // DWS_ENABLE_OPENADR
