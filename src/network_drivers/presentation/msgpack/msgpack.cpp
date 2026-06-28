// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file msgpack.cpp
 * @brief Zero-heap MessagePack encoder implementation.
 */

#include "msgpack.h"

#if DETWS_ENABLE_MSGPACK

#include <string.h>

void msgpack_init(MsgpackWriter *w, uint8_t *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->pos = 0;
    w->overflow = false;
}

size_t msgpack_len(const MsgpackWriter *w)
{
    return w->pos;
}

bool msgpack_ok(const MsgpackWriter *w)
{
    return !w->overflow;
}

static void put(MsgpackWriter *w, uint8_t b)
{
    if (w->pos < w->cap)
        w->buf[w->pos] = b;
    else
        w->overflow = true;
    w->pos++; // keep counting so msgpack_len() reports the needed size
}

// Write the low @p nbytes of @p val, big-endian (MessagePack is network order).
static void put_be(MsgpackWriter *w, uint64_t val, int nbytes)
{
    for (int s = (nbytes - 1) * 8; s >= 0; s -= 8)
        put(w, (uint8_t)(val >> s));
}

void msgpack_uint(MsgpackWriter *w, uint64_t v)
{
    if (v <= 0x7f)
        put(w, (uint8_t)v); // positive fixint
    else if (v <= 0xff)
    {
        put(w, 0xcc);
        put(w, (uint8_t)v);
    }
    else if (v <= 0xffff)
    {
        put(w, 0xcd);
        put_be(w, v, 2);
    }
    else if (v <= 0xffffffffULL)
    {
        put(w, 0xce);
        put_be(w, v, 4);
    }
    else
    {
        put(w, 0xcf);
        put_be(w, v, 8);
    }
}

void msgpack_int(MsgpackWriter *w, int64_t v)
{
    if (v >= 0)
    {
        msgpack_uint(w, (uint64_t)v);
        return;
    }
    if (v >= -32)
        put(w, (uint8_t)v); // negative fixint (two's-complement byte 0xe0..0xff)
    else if (v >= -128)
    {
        put(w, 0xd0);
        put(w, (uint8_t)v);
    }
    else if (v >= -32768)
    {
        put(w, 0xd1);
        put_be(w, (uint64_t)(uint16_t)v, 2);
    }
    else if (v >= -2147483648LL)
    {
        put(w, 0xd2);
        put_be(w, (uint64_t)(uint32_t)v, 4);
    }
    else
    {
        put(w, 0xd3);
        put_be(w, (uint64_t)v, 8);
    }
}

void msgpack_str_n(MsgpackWriter *w, const char *s, size_t len)
{
    if (len <= 31)
        put(w, (uint8_t)(0xa0 | len)); // fixstr
    else if (len <= 0xff)
    {
        put(w, 0xd9);
        put(w, (uint8_t)len);
    }
    else if (len <= 0xffff)
    {
        put(w, 0xda);
        put_be(w, len, 2);
    }
    else
    {
        put(w, 0xdb);
        put_be(w, len, 4);
    }
    for (size_t i = 0; i < len; i++)
        put(w, (uint8_t)s[i]);
}

void msgpack_str(MsgpackWriter *w, const char *s)
{
    msgpack_str_n(w, s, s ? strlen(s) : 0);
}

void msgpack_bytes(MsgpackWriter *w, const uint8_t *data, size_t len)
{
    if (len <= 0xff)
    {
        put(w, 0xc4);
        put(w, (uint8_t)len);
    }
    else if (len <= 0xffff)
    {
        put(w, 0xc5);
        put_be(w, len, 2);
    }
    else
    {
        put(w, 0xc6);
        put_be(w, len, 4);
    }
    for (size_t i = 0; i < len; i++)
        put(w, data[i]);
}

void msgpack_bool(MsgpackWriter *w, bool b)
{
    put(w, b ? 0xc3 : 0xc2);
}

void msgpack_nil(MsgpackWriter *w)
{
    put(w, 0xc0);
}

void msgpack_float(MsgpackWriter *w, float f)
{
    uint32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    put(w, 0xca); // float32
    put_be(w, bits, 4);
}

void msgpack_array(MsgpackWriter *w, size_t count)
{
    if (count <= 15)
        put(w, (uint8_t)(0x90 | count)); // fixarray
    else if (count <= 0xffff)
    {
        put(w, 0xdc);
        put_be(w, count, 2);
    }
    else
    {
        put(w, 0xdd);
        put_be(w, count, 4);
    }
}

void msgpack_map(MsgpackWriter *w, size_t count)
{
    if (count <= 15)
        put(w, (uint8_t)(0x80 | count)); // fixmap
    else if (count <= 0xffff)
    {
        put(w, 0xde);
        put_be(w, count, 2);
    }
    else
    {
        put(w, 0xdf);
        put_be(w, count, 4);
    }
}

#endif // DETWS_ENABLE_MSGPACK
