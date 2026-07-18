// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file atc.cpp
 * @brief ATC field-I/O interop snapshot (see atc.h).
 */

#include "services/atc/atc.h"

#if DWS_ENABLE_ATC

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

void put_u8(DWSSb *b, uint8_t v)
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
    dws_sb_put(b, o);
}

// Append the points of one direction (outputs or inputs) as a JSON array.
void put_array(DWSSb *b, const AtcFieldIo *io, bool outputs)
{
    dws_sb_put(b, "[");
    bool first = true;
    for (size_t i = 0; i < io->count; i++)
    {
        if (io->points[i].is_output != outputs)
            continue;
        if (!first)
            dws_sb_put(b, ",");
        first = false;
        dws_sb_put(b, "{\"name\":");
        put_json_str(b, io->points[i].name);
        dws_sb_put(b, ",\"value\":");
        put_u8(b, io->points[i].value);
        dws_sb_put(b, "}");
    }
    dws_sb_put(b, "]");
}
} // namespace

size_t dws_atc_snapshot_json(const AtcFieldIo *io, char *out, size_t cap)
{
    if (!io || !out || (io->count && !io->points))
        return 0;
    DWSSb b = {out, cap, 0, cap > 0};
    dws_sb_put(&b, "{\"inputs\":");
    put_array(&b, io, false);
    dws_sb_put(&b, ",\"outputs\":");
    put_array(&b, io, true);
    dws_sb_put(&b, "}");
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

bool dws_atc_set_output(AtcFieldIo *io, const char *name, uint8_t value)
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

uint8_t dws_atc_get(const AtcFieldIo *io, const char *name, bool *found)
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

#endif // DWS_ENABLE_ATC
