// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file h3_frame.cpp
 * @brief HTTP/3 framing - implementation. See h3_frame.h.
 */

#include "network_drivers/presentation/http3/h3_frame.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>

bool h3_frame_parse(const uint8_t *buf, size_t len, H3Frame *out)
{
    size_t c1 = 0, c2 = 0;
    uint64_t type = 0, length = 0;
    if (!quic_varint_decode(buf, len, &type, &c1))
        return false;
    if (!quic_varint_decode(buf + c1, len - c1, &length, &c2))
        return false;
    out->type = type;
    out->length = length;
    out->header_len = c1 + c2;
    return true;
}

size_t h3_frame_write_header(uint8_t *out, size_t cap, uint64_t type, uint64_t length)
{
    size_t n = quic_varint_encode(out, cap, type);
    if (!n)
        return 0;
    size_t m = quic_varint_encode(out + n, cap - n, length);
    if (!m)
        return 0;
    return n + m;
}

bool h3_frame_type_reserved(uint64_t type)
{
    // The HTTP/2 frame types that have no HTTP/3 meaning (RFC 9114 sec 11.2.1).
    return type == 0x02 || type == 0x06 || type == 0x08 || type == 0x09;
}

void h3_settings_defaults(H3Settings *s)
{
    s->qpack_max_table_capacity = 0;
    s->max_field_section_size = 0xFFFFFFFFFFFFFFFFULL; // unlimited
    s->qpack_blocked_streams = 0;
}

bool h3_parse_settings(const uint8_t *payload, size_t len, H3Settings *s)
{
    size_t off = 0;
    while (off < len)
    {
        size_t c1 = 0, c2 = 0;
        uint64_t id = 0, val = 0;
        if (!quic_varint_decode(payload + off, len - off, &id, &c1))
            return false;
        off += c1;
        if (!quic_varint_decode(payload + off, len - off, &val, &c2))
            return false;
        off += c2;
        switch (id)
        {
        case H3Setting::H3_SETTINGS_QPACK_MAX_TABLE_CAPACITY:
            s->qpack_max_table_capacity = val;
            break;
        case H3Setting::H3_SETTINGS_MAX_FIELD_SECTION_SIZE:
            s->max_field_section_size = val;
            break;
        case H3Setting::H3_SETTINGS_QPACK_BLOCKED_STREAMS:
            s->qpack_blocked_streams = val;
            break;
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            return false; // reserved HTTP/2 settings identifiers (RFC 9114 sec 7.2.4.1)
        default:
            break; // unknown / greased settings are ignored
        }
    }
    return true;
}

size_t h3_build_data(uint8_t *out, size_t cap, const uint8_t *data, size_t len)
{
    size_t hn = h3_frame_write_header(out, cap, H3FrameType::H3_DATA, len);
    if (!hn || hn + len > cap)
        return 0;
    if (len)
        memcpy(out + hn, data, len);
    return hn + len;
}

size_t h3_build_headers(uint8_t *out, size_t cap, const uint8_t *block, size_t len)
{
    size_t hn = h3_frame_write_header(out, cap, H3FrameType::H3_HEADERS, len);
    if (!hn || hn + len > cap)
        return 0;
    if (len)
        memcpy(out + hn, block, len);
    return hn + len;
}

size_t h3_build_settings(uint8_t *out, size_t cap, const uint64_t *ids, const uint64_t *vals, size_t n)
{
    size_t plen = 0;
    for (size_t i = 0; i < n; i++)
        plen += quic_varint_len(ids[i]) + quic_varint_len(vals[i]);
    size_t o = h3_frame_write_header(out, cap, H3FrameType::H3_SETTINGS, plen);
    if (!o)
        return 0;
    for (size_t i = 0; i < n; i++)
    {
        size_t a = quic_varint_encode(out + o, cap - o, ids[i]);
        if (!a)
            return 0;
        o += a;
        size_t b = quic_varint_encode(out + o, cap - o, vals[i]);
        if (!b)
            return 0;
        o += b;
    }
    return o;
}

size_t h3_build_goaway(uint8_t *out, size_t cap, uint64_t stream_id)
{
    size_t plen = quic_varint_len(stream_id);
    size_t o = h3_frame_write_header(out, cap, H3FrameType::H3_GOAWAY, plen);
    if (!o)
        return 0;
    size_t a = quic_varint_encode(out + o, cap - o, stream_id);
    if (!a)
        return 0;
    return o + a;
}

#endif // DETWS_ENABLE_HTTP3
