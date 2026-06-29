// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file protobuf.cpp
 * @brief Protocol Buffers wire codec (pure, host-tested against spec vectors).
 */

#include "services/protobuf/protobuf.h"

#if DETWS_ENABLE_PROTOBUF

#include <string.h>

void pb_writer_init(PbWriter *w, uint8_t *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->pos = 0;
    w->error = false;
}

bool pb_write_varint(PbWriter *w, uint64_t v)
{
    if (w->error)
        return false;
    uint8_t tmp[10];
    size_t n = 0;
    do
    {
        uint8_t b = (uint8_t)(v & 0x7F);
        v >>= 7;
        if (v)
            b |= 0x80;
        tmp[n++] = b;
    } while (v);
    if (w->pos + n > w->cap)
    {
        w->error = true;
        return false;
    }
    memcpy(w->buf + w->pos, tmp, n);
    w->pos += n;
    return true;
}

bool pb_write_tag(PbWriter *w, uint32_t field, uint8_t wire_type)
{
    return pb_write_varint(w, ((uint64_t)field << 3) | (wire_type & 0x07));
}

// Append @p n raw little-endian octets of @p v.
static bool pb_write_le(PbWriter *w, uint64_t v, size_t n)
{
    if (w->error)
        return false;
    if (w->pos + n > w->cap)
    {
        w->error = true;
        return false;
    }
    for (size_t i = 0; i < n; i++)
        w->buf[w->pos++] = (uint8_t)(v >> (8 * i));
    return true;
}

bool pb_uint64(PbWriter *w, uint32_t field, uint64_t v)
{
    return pb_write_tag(w, field, PB_WT_VARINT) && pb_write_varint(w, v);
}

bool pb_int64(PbWriter *w, uint32_t field, int64_t v)
{
    return pb_uint64(w, field, (uint64_t)v); // two's complement; negatives take 10 bytes
}

bool pb_sint64(PbWriter *w, uint32_t field, int64_t v)
{
    uint64_t zz = ((uint64_t)v << 1) ^ (uint64_t)(v >> 63); // ZigZag
    return pb_uint64(w, field, zz);
}

bool pb_bool(PbWriter *w, uint32_t field, bool v)
{
    return pb_uint64(w, field, v ? 1 : 0);
}

bool pb_fixed32(PbWriter *w, uint32_t field, uint32_t v)
{
    return pb_write_tag(w, field, PB_WT_I32) && pb_write_le(w, v, 4);
}

bool pb_fixed64(PbWriter *w, uint32_t field, uint64_t v)
{
    return pb_write_tag(w, field, PB_WT_I64) && pb_write_le(w, v, 8);
}

bool pb_float(PbWriter *w, uint32_t field, float v)
{
    uint32_t bits;
    memcpy(&bits, &v, 4);
    return pb_fixed32(w, field, bits);
}

bool pb_double(PbWriter *w, uint32_t field, double v)
{
    uint64_t bits;
    memcpy(&bits, &v, 8);
    return pb_fixed64(w, field, bits);
}

bool pb_bytes(PbWriter *w, uint32_t field, const uint8_t *data, size_t len)
{
    if (!pb_write_tag(w, field, PB_WT_LEN) || !pb_write_varint(w, len))
        return false;
    if (w->error)
        return false;
    if (len == 0)
        return true;
    if (!data || w->pos + len > w->cap)
    {
        w->error = true;
        return false;
    }
    memcpy(w->buf + w->pos, data, len);
    w->pos += len;
    return true;
}

bool pb_string(PbWriter *w, uint32_t field, const char *s)
{
    if (!s)
    {
        w->error = true;
        return false;
    }
    return pb_bytes(w, field, (const uint8_t *)s, strlen(s));
}

size_t pb_writer_finish(PbWriter *w)
{
    return w->error ? 0 : w->pos;
}

bool pb_read_varint(const uint8_t *buf, size_t len, size_t *pos, uint64_t *out)
{
    if (!buf || !pos || !out)
        return false;
    uint64_t v = 0;
    size_t shift = 0;
    size_t i = *pos;
    for (size_t b = 0; b < 10; b++) // a 64-bit varint is at most 10 bytes
    {
        if (i >= len)
            return false; // truncated
        uint8_t c = buf[i++];
        v |= (uint64_t)(c & 0x7F) << shift;
        if (!(c & 0x80))
        {
            *out = v;
            *pos = i;
            return true;
        }
        shift += 7;
    }
    return false; // overlong / unterminated
}

bool pb_read_field(const uint8_t *buf, size_t len, size_t *pos, PbField *out)
{
    if (!buf || !pos || !out || *pos >= len)
        return false;
    uint64_t tag;
    if (!pb_read_varint(buf, len, pos, &tag))
        return false;
    out->field_number = (uint32_t)(tag >> 3);
    out->wire_type = (uint8_t)(tag & 0x07);
    out->value = 0;
    out->data = nullptr;
    out->len = 0;

    switch (out->wire_type)
    {
    case PB_WT_VARINT:
        return pb_read_varint(buf, len, pos, &out->value);
    case PB_WT_I64: {
        if (*pos + 8 > len)
            return false;
        uint64_t v = 0;
        for (size_t i = 0; i < 8; i++)
            v |= (uint64_t)buf[*pos + i] << (8 * i);
        *pos += 8;
        out->value = v;
        return true;
    }
    case PB_WT_I32: {
        if (*pos + 4 > len)
            return false;
        uint32_t v = 0;
        for (size_t i = 0; i < 4; i++)
            v |= (uint32_t)buf[*pos + i] << (8 * i);
        *pos += 4;
        out->value = v;
        return true;
    }
    case PB_WT_LEN: {
        uint64_t l;
        if (!pb_read_varint(buf, len, pos, &l))
            return false;
        if (*pos + l > len)
            return false; // payload not fully buffered
        out->data = buf + *pos;
        out->len = (size_t)l;
        *pos += (size_t)l;
        return true;
    }
    default:
        return false; // groups (3/4) / reserved (6/7) are not supported
    }
}

int64_t pb_zigzag64(uint64_t v)
{
    return (int64_t)(v >> 1) ^ -(int64_t)(v & 1);
}

int32_t pb_zigzag32(uint32_t v)
{
    return (int32_t)(v >> 1) ^ -(int32_t)(v & 1);
}

float pb_float_bits(uint32_t bits)
{
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

double pb_double_bits(uint64_t bits)
{
    double d;
    memcpy(&d, &bits, 8);
    return d;
}

#endif // DETWS_ENABLE_PROTOBUF
