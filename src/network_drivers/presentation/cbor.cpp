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

// ---------------------------------------------------------------------------
// Decoder
// ---------------------------------------------------------------------------

void cbor_reader_init(CborReader *r, const uint8_t *buf, size_t len)
{
    r->buf = buf;
    r->len = len;
    r->pos = 0;
    r->err = false;
}

bool cbor_reader_ok(const CborReader *r)
{
    return !r->err;
}

// Read a CBOR head at r->pos: major type + argument, advancing pos. Sets err and
// returns false on out-of-bounds or a reserved/indefinite additional-info value.
static bool read_head(CborReader *r, uint8_t *major, uint64_t *val)
{
    if (r->err || r->pos >= r->len)
    {
        r->err = true;
        return false;
    }
    uint8_t b = r->buf[r->pos];
    uint8_t info = (uint8_t)(b & 0x1f);
    *major = (uint8_t)(b >> 5);
    if (info < 24)
    {
        *val = info;
        r->pos += 1;
        return true;
    }
    size_t need;
    switch (info)
    {
    case 24:
        need = 1;
        break;
    case 25:
        need = 2;
        break;
    case 26:
        need = 4;
        break;
    case 27:
        need = 8;
        break;
    default:
        r->err = true; // 28-31: reserved / indefinite-length, unsupported
        return false;
    }
    if (r->pos + 1 + need > r->len)
    {
        r->err = true;
        return false;
    }
    uint64_t v = 0;
    for (size_t i = 0; i < need; i++)
        v = (v << 8) | r->buf[r->pos + 1 + i];
    *val = v;
    r->pos += 1 + need;
    return true;
}

CborType cbor_peek(CborReader *r)
{
    if (r->err || r->pos >= r->len)
        return CBOR_TYPE_INVALID;
    uint8_t b = r->buf[r->pos];
    switch (b >> 5)
    {
    case 0:
        return CBOR_TYPE_UINT;
    case 1:
        return CBOR_TYPE_INT;
    case 2:
        return CBOR_TYPE_BYTES;
    case 3:
        return CBOR_TYPE_TEXT;
    case 4:
        return CBOR_TYPE_ARRAY;
    case 5:
        return CBOR_TYPE_MAP;
    case 7: {
        uint8_t info = (uint8_t)(b & 0x1f);
        if (info == 20 || info == 21)
            return CBOR_TYPE_BOOL;
        if (info == 22)
            return CBOR_TYPE_NULL;
        if (info == 26 || info == 27)
            return CBOR_TYPE_FLOAT;
        return CBOR_TYPE_INVALID;
    }
    default:
        return CBOR_TYPE_INVALID; // major 6 (tags) unsupported
    }
}

bool cbor_read_uint(CborReader *r, uint64_t *out)
{
    uint8_t m;
    uint64_t v;
    if (!read_head(r, &m, &v))
        return false;
    if (m != 0)
    {
        r->err = true;
        return false;
    }
    *out = v;
    return true;
}

bool cbor_read_int(CborReader *r, int64_t *out)
{
    uint8_t m;
    uint64_t v;
    if (!read_head(r, &m, &v))
        return false;
    if (m == 0)
        *out = (int64_t)v;
    else if (m == 1)
        *out = -1 - (int64_t)v;
    else
    {
        r->err = true;
        return false;
    }
    return true;
}

bool cbor_read_bool(CborReader *r, bool *out)
{
    if (r->err || r->pos >= r->len)
    {
        r->err = true;
        return false;
    }
    uint8_t b = r->buf[r->pos];
    if (b == 0xf4)
        *out = false;
    else if (b == 0xf5)
        *out = true;
    else
    {
        r->err = true;
        return false;
    }
    r->pos += 1;
    return true;
}

bool cbor_read_null(CborReader *r)
{
    if (r->err || r->pos >= r->len || r->buf[r->pos] != 0xf6)
    {
        r->err = true;
        return false;
    }
    r->pos += 1;
    return true;
}

bool cbor_read_float(CborReader *r, float *out)
{
    if (r->err || r->pos >= r->len)
    {
        r->err = true;
        return false;
    }
    uint8_t b = r->buf[r->pos];
    if (b == 0xfa) // single
    {
        if (r->pos + 5 > r->len)
        {
            r->err = true;
            return false;
        }
        uint32_t bits = ((uint32_t)r->buf[r->pos + 1] << 24) | ((uint32_t)r->buf[r->pos + 2] << 16) |
                        ((uint32_t)r->buf[r->pos + 3] << 8) | (uint32_t)r->buf[r->pos + 4];
        memcpy(out, &bits, sizeof(*out));
        r->pos += 5;
        return true;
    }
    if (b == 0xfb) // double -> narrow to float
    {
        if (r->pos + 9 > r->len)
        {
            r->err = true;
            return false;
        }
        uint64_t bits = 0;
        for (int i = 0; i < 8; i++)
            bits = (bits << 8) | r->buf[r->pos + 1 + i];
        double d;
        memcpy(&d, &bits, sizeof(d));
        *out = (float)d;
        r->pos += 9;
        return true;
    }
    r->err = true;
    return false;
}

// Shared body for text (major 3) and byte (major 2) strings.
static bool read_str(CborReader *r, uint8_t want_major, const uint8_t **out, size_t *len)
{
    uint8_t m;
    uint64_t v;
    if (!read_head(r, &m, &v))
        return false;
    if (m != want_major || r->pos + v > r->len)
    {
        r->err = true;
        return false;
    }
    *out = &r->buf[r->pos];
    *len = (size_t)v;
    r->pos += (size_t)v;
    return true;
}

bool cbor_read_text(CborReader *r, const char **out, size_t *len)
{
    return read_str(r, 3, (const uint8_t **)out, len);
}

bool cbor_read_bytes(CborReader *r, const uint8_t **out, size_t *len)
{
    return read_str(r, 2, out, len);
}

bool cbor_read_array(CborReader *r, size_t *count)
{
    uint8_t m;
    uint64_t v;
    if (!read_head(r, &m, &v))
        return false;
    if (m != 4)
    {
        r->err = true;
        return false;
    }
    *count = (size_t)v;
    return true;
}

bool cbor_read_map(CborReader *r, size_t *count)
{
    uint8_t m;
    uint64_t v;
    if (!read_head(r, &m, &v))
        return false;
    if (m != 5)
    {
        r->err = true;
        return false;
    }
    *count = (size_t)v;
    return true;
}

#endif // DETWS_ENABLE_CBOR
