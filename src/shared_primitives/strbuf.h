// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file strbuf.h
 * @brief Bounded no-heap string builder that fails closed on overflow (one shared copy).
 *
 * The same little `Buf` appender was open-coded inside the anonymous namespace of ~5
 * codecs (utmc, sep2, openadr, atc, exc_decoder). It bump-appends into a caller-owned
 * `char[]` and latches @c ok to false the first time something would not fit, so every
 * later append is a no-op and callers test one flag at the end. These header-only inline
 * helpers are the single home for it, mirroring hex.h / numparse.h - no `<stdlib.h>`,
 * no heap, and zero link cost when unused. The verbatim pieces (struct + raw append + XML
 * escape + decimal + JSON-string + terminate) live here; the JSON emitters (hw_health,
 * http_delivery, ble_gatt, ...) shared them byte-for-byte, so they are no longer per-codec.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_STRBUF_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_STRBUF_H

#include <stddef.h>
#include <string.h>

/** @brief Bump-append target; @c ok latches false once an append would overflow @c cap. */
struct DWSSb
{
    char *p;
    size_t cap;
    size_t len;
    bool ok;
};

/** @brief Append NUL-terminated @p s; leaves the buffer untouched and clears @c ok if it would not fit. */
inline void dws_sb_put(DWSSb *b, const char *s)
{
    if (!b->ok)
        return;
    size_t sl = strnlen(s, b->cap);
    if (b->len + sl >= b->cap)
    {
        b->ok = false;
        return;
    }
    memcpy(b->p + b->len, s, sl);
    b->len += sl;
}

/** @brief Append @p s XML-escaped (&amp; &lt; &gt; &quot;); a NULL @p s appends nothing. */
inline void dws_sb_xml(DWSSb *b, const char *s)
{
    if (!b->ok || !s)
        return;
    for (; *s; s++)
    {
        const char *rep = nullptr;
        switch (*s)
        {
        case '&':
            rep = "&amp;";
            break;
        case '<':
            rep = "&lt;";
            break;
        case '>':
            rep = "&gt;";
            break;
        case '"':
            rep = "&quot;";
            break;
        default:
            break;
        }
        if (rep)
            dws_sb_put(b, rep);
        else
        {
            if (b->len + 1 >= b->cap)
            {
                b->ok = false;
                return;
            }
            b->p[b->len++] = *s;
        }
    }
}

/** @brief Append @p v as decimal (no leading zeros; "0" for zero). */
inline void dws_sb_u32(DWSSb *b, uint32_t v)
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
    dws_sb_put(b, o);
}

/** @brief Append @p s as a JSON string literal: double-quoted, with `"` and `\` backslash-escaped. A NULL
 * @p s emits `""`. (Control chars are passed through, matching the emitters this replaced.) */
inline void dws_sb_json(DWSSb *b, const char *s)
{
    dws_sb_put(b, "\"");
    for (const char *p = s ? s : ""; *p; p++)
    {
        if (*p == '"' || *p == '\\')
        {
            char esc[3] = {'\\', *p, '\0'};
            dws_sb_put(b, esc);
        }
        else if (b->len + 1 < b->cap)
            b->p[b->len++] = *p;
        else
            b->ok = false;
    }
    dws_sb_put(b, "\"");
}

/** @brief NUL-terminate and return the built length, or 0 if the build overflowed. */
inline size_t dws_sb_finish(DWSSb *b)
{
    if (!b->ok)
        return 0;
    b->p[b->len] = '\0';
    return b->len;
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_STRBUF_H
