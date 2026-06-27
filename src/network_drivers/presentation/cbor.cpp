// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file cbor.cpp
 * @brief Zero-heap CBOR (RFC 8949) encoder implementation.
 */

#include "cbor.h"

#if DETWS_ENABLE_CBOR

#include <string.h>

void cbor_init(CborWriter *w, uint8_t *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->pos = 0;
    w->overflow = false;
}

size_t cbor_len(const CborWriter *w)
{
    return w->pos;
}

bool cbor_ok(const CborWriter *w)
{
    return !w->overflow;
}

static void put(CborWriter *w, uint8_t b)
{
    if (w->pos < w->cap)
        w->buf[w->pos] = b;
    else
        w->overflow = true;
    w->pos++; // keep counting so cbor_len() reports the needed size
}

// Write a CBOR head: the major type (top 3 bits) plus the argument, choosing the
// shortest of the 1/2/3/5/9-byte forms (RFC 8949 section 3).
static void head(CborWriter *w, uint8_t major, uint64_t val)
{
    uint8_t m = (uint8_t)(major << 5);
    if (val < 24)
    {
        put(w, (uint8_t)(m | val));
    }
    else if (val < 0x100ULL)
    {
        put(w, (uint8_t)(m | 24));
        put(w, (uint8_t)val);
    }
    else if (val < 0x10000ULL)
    {
        put(w, (uint8_t)(m | 25));
        put(w, (uint8_t)(val >> 8));
        put(w, (uint8_t)val);
    }
    else if (val < 0x100000000ULL)
    {
        put(w, (uint8_t)(m | 26));
        put(w, (uint8_t)(val >> 24));
        put(w, (uint8_t)(val >> 16));
        put(w, (uint8_t)(val >> 8));
        put(w, (uint8_t)val);
    }
    else
    {
        put(w, (uint8_t)(m | 27));
        for (int s = 56; s >= 0; s -= 8)
            put(w, (uint8_t)(val >> s));
    }
}

void cbor_uint(CborWriter *w, uint64_t v)
{
    head(w, 0, v);
}

void cbor_int(CborWriter *w, int64_t v)
{
    if (v >= 0)
        head(w, 0, (uint64_t)v);
    else
        head(w, 1, (uint64_t)(-1 - v)); // major 1 encodes -1 - n
}

void cbor_bytes(CborWriter *w, const uint8_t *data, size_t len)
{
    head(w, 2, (uint64_t)len);
    for (size_t i = 0; i < len; i++)
        put(w, data[i]);
}

void cbor_text_n(CborWriter *w, const char *s, size_t len)
{
    head(w, 3, (uint64_t)len);
    for (size_t i = 0; i < len; i++)
        put(w, (uint8_t)s[i]);
}

void cbor_text(CborWriter *w, const char *s)
{
    cbor_text_n(w, s, s ? strlen(s) : 0);
}

void cbor_bool(CborWriter *w, bool b)
{
    put(w, b ? 0xf5 : 0xf4);
}

void cbor_null(CborWriter *w)
{
    put(w, 0xf6);
}

void cbor_float(CborWriter *w, float f)
{
    uint32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    put(w, 0xfa); // major 7, single-precision
    put(w, (uint8_t)(bits >> 24));
    put(w, (uint8_t)(bits >> 16));
    put(w, (uint8_t)(bits >> 8));
    put(w, (uint8_t)bits);
}

void cbor_array(CborWriter *w, size_t count)
{
    head(w, 4, (uint64_t)count);
}

void cbor_map(CborWriter *w, size_t count)
{
    head(w, 5, (uint64_t)count);
}

#endif // DETWS_ENABLE_CBOR
