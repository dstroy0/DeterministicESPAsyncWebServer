// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file atc.cpp
 * @brief ATC field-I/O interop snapshot (see atc.h).
 */

#include "services/atc/atc.h"

#if DETWS_ENABLE_ATC

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

void put_u8(Buf *b, uint8_t v)
{
    char t[4];
    int n = 0;
    do
    {
        t[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    char o[4];
    for (int i = 0; i < n; i++)
        o[i] = t[n - 1 - i];
    o[n] = '\0';
    put(b, o);
}

// Append the points of one direction (outputs or inputs) as a JSON array.
void put_array(Buf *b, const AtcFieldIo *io, bool outputs)
{
    put(b, "[");
    bool first = true;
    for (size_t i = 0; i < io->count; i++)
    {
        if (io->points[i].is_output != outputs)
            continue;
        if (!first)
            put(b, ",");
        first = false;
        put(b, "{\"name\":");
        put_json_str(b, io->points[i].name);
        put(b, ",\"value\":");
        put_u8(b, io->points[i].value);
        put(b, "}");
    }
    put(b, "]");
}
} // namespace

size_t detws_atc_snapshot_json(const AtcFieldIo *io, char *out, size_t cap)
{
    if (!io || !out || (io->count && !io->points))
        return 0;
    Buf b = {out, cap, 0, cap > 0};
    put(&b, "{\"inputs\":");
    put_array(&b, io, false);
    put(&b, ",\"outputs\":");
    put_array(&b, io, true);
    put(&b, "}");
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

bool detws_atc_set_output(AtcFieldIo *io, const char *name, uint8_t value)
{
    if (!io || !name || !io->points)
        return false;
    for (size_t i = 0; i < io->count; i++)
        if (io->points[i].is_output && io->points[i].name && strcmp(io->points[i].name, name) == 0)
        {
            io->points[i].value = value;
            return true;
        }
    return false;
}

uint8_t detws_atc_get(const AtcFieldIo *io, const char *name, bool *found)
{
    if (found)
        *found = false;
    if (!io || !name || !io->points)
        return 0;
    for (size_t i = 0; i < io->count; i++)
        if (io->points[i].name && strcmp(io->points[i].name, name) == 0)
        {
            if (found)
                *found = true;
            return io->points[i].value;
        }
    return 0;
}

#endif // DETWS_ENABLE_ATC
