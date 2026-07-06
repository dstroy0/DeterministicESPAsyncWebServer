// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file openadr.cpp
 * @brief OpenADR 3.0 JSON codec (see openadr.h).
 */

#include "services/openadr/openadr.h"

#if DETWS_ENABLE_OPENADR

#include <string.h>

namespace
{
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
        else if (*p == '\n')
            put(b, "\\n");
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
    put(b, "\"");
}

void put_u64(Buf *b, uint64_t v)
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
    put(b, out);
}

// Format a double with 3 decimal places (no stdlib). Rounds to milli-units; handles the sign.
void put_double(Buf *b, double v)
{
    if (v < 0)
    {
        put(b, "-");
        v = -v;
    }
    // scale by 1000 and round.
    uint64_t scaled = (uint64_t)(v * 1000.0 + 0.5);
    uint64_t whole = scaled / 1000;
    uint32_t frac = (uint32_t)(scaled % 1000);
    put_u64(b, whole);
    put(b, ".");
    // three digits, zero-padded.
    char f[4] = {(char)('0' + (frac / 100) % 10), (char)('0' + (frac / 10) % 10), (char)('0' + frac % 10), '\0'};
    put(b, f);
}

size_t finish(Buf *b)
{
    if (!b->ok)
        return 0;
    b->p[b->len] = '\0';
    return b->len;
}
} // namespace

size_t detws_openadr_event(const char *program_id, const char *event_name, const OpenAdrInterval *intervals,
                           size_t count, char *out, size_t cap)
{
    if (!out || (count && !intervals))
        return 0;
    Buf b = {out, cap, 0, cap > 0};
    put(&b, "{\"objectType\":\"EVENT\",\"programID\":");
    put_json_str(&b, program_id);
    put(&b, ",\"eventName\":");
    put_json_str(&b, event_name);
    put(&b, ",\"intervals\":[");
    for (size_t i = 0; i < count; i++)
    {
        if (i)
            put(&b, ",");
        put(&b, "{\"id\":");
        put_u64(&b, i);
        put(&b, ",\"interval\":{\"start\":");
        put_u64(&b, intervals[i].start);
        put(&b, ",\"duration\":");
        put_u64(&b, intervals[i].duration);
        put(&b, "},\"payloads\":[{\"type\":");
        put_json_str(&b, intervals[i].type);
        put(&b, ",\"values\":[");
        put_double(&b, intervals[i].value);
        put(&b, "]}]}");
    }
    put(&b, "]}");
    return finish(&b);
}

size_t detws_openadr_report(const char *program_id, const char *event_id, const char *resource_name, double value,
                            uint32_t timestamp, char *out, size_t cap)
{
    if (!out)
        return 0;
    Buf b = {out, cap, 0, cap > 0};
    put(&b, "{\"objectType\":\"REPORT\",\"programID\":");
    put_json_str(&b, program_id);
    put(&b, ",\"eventID\":");
    put_json_str(&b, event_id);
    put(&b, ",\"resources\":[{\"resourceName\":");
    put_json_str(&b, resource_name);
    put(&b, ",\"intervals\":[{\"interval\":{\"start\":");
    put_u64(&b, timestamp);
    put(&b, "},\"payloads\":[{\"type\":\"READING\",\"values\":[");
    put_double(&b, value);
    put(&b, "]}]}]}]}");
    return finish(&b);
}

#endif // DETWS_ENABLE_OPENADR
