// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file exc_decoder.cpp
 * @brief ESP32 panic / exception decoder (see exc_decoder.h).
 */

#include "services/exc_decoder/exc_decoder.h"

#if DETWS_ENABLE_EXC_DECODER

#include <string.h>

#include "shared_primitives/strbuf.h"

namespace
{
bool hexval(char c, uint8_t *v)
{
    if (c >= '0' && c <= '9')
        *v = (uint8_t)(c - '0');
    else if (c >= 'a' && c <= 'f')
        *v = (uint8_t)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
        *v = (uint8_t)(c - 'A' + 10);
    else
        return false;
    return true;
}

const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t')
        p++;
    return p;
}

// Parse a "0x...." hex literal at p; on success write *out and return the char after the last digit.
const char *parse_hex(const char *p, uint32_t *out)
{
    if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X'))
        return nullptr;
    p += 2;
    uint32_t v = 0;
    int n = 0;
    uint8_t d = 0;
    while (hexval(*p, &d) && n < 8)
    {
        v = (v << 4) | d;
        p++;
        n++;
    }
    if (n == 0)
        return nullptr;
    *out = v;
    return p;
}

void put_json_str(DetSb *b, const char *s)
{
    det_sb_put(b, "\"");
    for (const char *p = s ? s : ""; *p; p++)
    {
        if (*p == '"' || *p == '\\')
        {
            char esc[3] = {'\\', *p, '\0'};
            det_sb_put(b, esc);
        }
        else if (b->len + 1 < b->cap)
            b->p[b->len++] = *p;
        else
            b->ok = false;
    }
    det_sb_put(b, "\"");
}

// Emit a 32-bit value as a JSON string literal "0x........".
void put_hex32(DetSb *b, uint32_t v)
{
    char t[13] = "\"0x00000000\"";
    static const char *H = "0123456789abcdef";
    for (int i = 0; i < 8; i++)
        t[3 + i] = H[(v >> ((7 - i) * 4)) & 0xF];
    det_sb_put(b, t);
}

void put_int(DetSb *b, int v)
{
    char t[12];
    int n = 0;
    bool neg = v < 0;
    unsigned u = neg ? (unsigned)(-(long)v) : (unsigned)v;
    do
    {
        t[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    char o[13];
    int k = 0;
    if (neg)
        o[k++] =
            '-'; // GCOVR_EXCL_LINE detws_exc_json's only put_int call is guarded by core >= 0, so v is never negative
    for (int i = 0; i < n; i++)
        o[k++] = t[n - 1 - i];
    o[k] = '\0';
    det_sb_put(b, o);
}
} // namespace

bool detws_exc_parse(const char *text, ExcInfo *out)
{
    if (!text || !out)
        return false;
    out->core = -1;
    out->cause[0] = '\0';
    out->pc = 0;
    out->excvaddr = 0;
    out->has_excvaddr = false;
    out->frame_count = 0;

    // Cause: "...panic'ed (LoadProhibited)."
    const char *c = strstr(text, "panic'ed (");
    if (c)
    {
        c += 10;
        size_t i = 0;
        while (i < sizeof(out->cause) - 1 && c[i] && c[i] != ')') // range check first (short-circuits the read)
        {
            out->cause[i] = c[i];
            i++;
        }
        out->cause[i] = '\0';
    }

    // Core number: "Core  N ...".
    const char *co = strstr(text, "Core ");
    if (co)
    {
        const char *p = skip_ws(co + 5);
        if (*p >= '0' && *p <= '9')
        {
            int n = 0;
            while (*p >= '0' && *p <= '9')
            {
                if (n < 100000) // clamp: a core id is tiny; avoid signed-overflow UB on a huge number
                    n = n * 10 + (*p - '0');
                p++;
            }
            out->core = n;
        }
    }

    // EXCVADDR (faulting data address).
    const char *e = strstr(text, "EXCVADDR");
    if (e)
    {
        const char *colon = strchr(e, ':');
        if (colon)
        {
            uint32_t v = 0;
            if (parse_hex(skip_ws(colon + 1), &v))
            {
                out->excvaddr = v;
                out->has_excvaddr = true;
            }
        }
    }

    // Register-dump PC: a line that starts with "PC" (not "EPC..."). Anchor to a line break.
    const char *pcl = nullptr;
    if (strncmp(text, "PC", 2) == 0)
        pcl = text;
    else
        pcl = strstr(text, "\nPC");
    if (pcl)
    {
        const char *colon = strchr(pcl, ':');
        if (colon)
        {
            uint32_t v = 0;
            if (parse_hex(skip_ws(colon + 1), &v))
                out->pc = v;
        }
    }

    // Backtrace: "Backtrace: pc:sp pc:sp ...".
    const char *bt = strstr(text, "Backtrace:");
    if (bt)
    {
        const char *p = bt + 10;
        while (out->frame_count < DETWS_EXC_MAX_FRAMES)
        {
            p = skip_ws(p);
            uint32_t pc = 0, sp = 0;
            const char *q = parse_hex(p, &pc);
            if (!q || *q != ':')
                break;
            const char *r = parse_hex(q + 1, &sp);
            if (!r)
                break;
            out->frames[out->frame_count].pc = pc;
            out->frames[out->frame_count].sp = sp;
            out->frame_count++;
            p = r;
        }
    }

    if (out->pc == 0 && out->frame_count > 0)
        out->pc = out->frames[0].pc;

    return out->cause[0] != '\0' || out->pc != 0 || out->frame_count > 0;
}

size_t detws_exc_json(const ExcInfo *info, char *out, size_t cap)
{
    if (!info || !out || cap == 0)
        return 0;
    DetSb b = {out, cap, 0, true};
    det_sb_put(&b, "{");
    bool first = true;
    if (info->core >= 0)
    {
        det_sb_put(&b, "\"core\":");
        put_int(&b, info->core);
        first = false;
    }
    if (!first)
        det_sb_put(&b, ",");
    det_sb_put(&b, "\"cause\":");
    put_json_str(&b, info->cause);
    det_sb_put(&b, ",\"pc\":");
    put_hex32(&b, info->pc);
    if (info->has_excvaddr)
    {
        det_sb_put(&b, ",\"excvaddr\":");
        put_hex32(&b, info->excvaddr);
    }
    det_sb_put(&b, ",\"backtrace\":[");
    for (size_t i = 0; i < info->frame_count; i++)
    {
        if (i)
            det_sb_put(&b, ",");
        put_hex32(&b, info->frames[i].pc);
    }
    det_sb_put(&b, "]}");
    if (!b.ok)
        return 0;
    out[b.len] = '\0';
    return b.len;
}

#endif // DETWS_ENABLE_EXC_DECODER
