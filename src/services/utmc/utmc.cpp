// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file utmc.cpp
 * @brief UTMC common-database codec (see utmc.h).
 */

#include "services/utmc/utmc.h"

#if DETWS_ENABLE_UTMC

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

void put_escaped(Buf *b, const char *s)
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
            put(b, rep);
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

void put_u(Buf *b, uint32_t v)
{
    char tmp[11];
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + (int)(v % 10));
        v /= 10;
    } while (v);
    char out[12];
    for (int i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    put(b, out);
}

size_t finish(Buf *b)
{
    if (!b->ok)
        return 0;
    b->p[b->len] = '\0';
    return b->len;
}
} // namespace

size_t detws_utmc_request(const char *object_id, char *out, size_t cap)
{
    Buf b = {out, cap, 0, out != nullptr && cap > 0};
    put(&b, "<?xml version=\"1.0\"?><UTMCRequest><object id=\"");
    put_escaped(&b, object_id);
    put(&b, "\"/></UTMCRequest>");
    return finish(&b);
}

size_t detws_utmc_response(const char *object_id, const char *value, uint8_t quality, const char *timestamp, char *out,
                           size_t cap)
{
    Buf b = {out, cap, 0, out != nullptr && cap > 0};
    put(&b, "<?xml version=\"1.0\"?><UTMCResponse><object id=\"");
    put_escaped(&b, object_id);
    put(&b, "\" value=\"");
    put_escaped(&b, value);
    put(&b, "\" quality=\"");
    put_u(&b, quality);
    put(&b, "\" timestamp=\"");
    put_escaped(&b, timestamp);
    put(&b, "\"/></UTMCResponse>");
    return finish(&b);
}

size_t detws_utmc_parse_request(const char *xml, size_t len, char *out, size_t cap)
{
    if (!xml || !out || cap == 0)
        return 0;
    // Find `id="` and copy up to the next quote.
    const char *key = "id=\"";
    size_t kl = 4;
    for (size_t i = 0; i + kl < len; i++)
    {
        if (memcmp(xml + i, key, kl) == 0)
        {
            size_t j = i + kl, k = 0;
            while (j < len && xml[j] != '"')
            {
                if (k + 1 >= cap)
                    return 0;
                out[k++] = xml[j++];
            }
            if (j >= len) // unterminated
                return 0;
            out[k] = '\0';
            return k;
        }
    }
    return 0;
}

#endif // DETWS_ENABLE_UTMC
