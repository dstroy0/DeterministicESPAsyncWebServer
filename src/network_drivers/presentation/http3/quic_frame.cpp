// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_frame.cpp
 * @brief QUIC frame parsing and building - implementation. See quic_frame.h.
 */

#include "network_drivers/presentation/http3/quic_frame.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>

namespace
{
// Decode a varint at buf[*pos], advancing *pos. Returns false on truncation.
bool rd(const uint8_t *buf, size_t len, size_t *pos, uint64_t *v)
{
    size_t c = 0;
    if (!quic_varint_decode(buf + *pos, len - *pos, v, &c))
        return false;
    *pos += c;
    return true;
}
} // namespace

size_t quic_frame_parse(const uint8_t *buf, size_t len, QuicFrame *out)
{
    size_t pos = 0;
    uint64_t type = 0;
    if (!rd(buf, len, &pos, &type))
        return 0;
    out->type = type;

    if (type == QUIC_FT_PADDING || type == QUIC_FT_PING || type == QUIC_FT_HANDSHAKE_DONE)
        return pos;

    if (type == QUIC_FT_ACK || type == QUIC_FT_ACK_ECN)
    {
        if (!rd(buf, len, &pos, &out->ack.largest) || !rd(buf, len, &pos, &out->ack.delay) ||
            !rd(buf, len, &pos, &out->ack.range_count) || !rd(buf, len, &pos, &out->ack.first_range))
            return 0;
        for (uint64_t i = 0; i < out->ack.range_count; i++) // skip Gap + ACK Range Length pairs
        {
            uint64_t tmp = 0;
            if (!rd(buf, len, &pos, &tmp) || !rd(buf, len, &pos, &tmp))
                return 0;
        }
        if (type == QUIC_FT_ACK_ECN) // skip the three ECN counts
        {
            uint64_t tmp = 0;
            if (!rd(buf, len, &pos, &tmp) || !rd(buf, len, &pos, &tmp) || !rd(buf, len, &pos, &tmp))
                return 0;
        }
        return pos;
    }

    if (type == QUIC_FT_CRYPTO)
    {
        if (!rd(buf, len, &pos, &out->crypto.offset) || !rd(buf, len, &pos, &out->crypto.length))
            return 0;
        if (pos + out->crypto.length > len)
            return 0;
        out->crypto.data = buf + pos;
        pos += out->crypto.length;
        return pos;
    }

    if (type >= QUIC_FT_STREAM && type <= 0x0f)
    {
        if (!rd(buf, len, &pos, &out->stream.id))
            return 0;
        out->stream.offset = 0;
        if (type & QUIC_STREAM_OFF)
            if (!rd(buf, len, &pos, &out->stream.offset))
                return 0;
        if (type & QUIC_STREAM_LEN)
        {
            if (!rd(buf, len, &pos, &out->stream.length))
                return 0;
        }
        else
        {
            out->stream.length = len - pos; // absent Length -> Stream Data runs to the packet end
        }
        if (pos + out->stream.length > len)
            return 0;
        out->stream.data = buf + pos;
        out->stream.fin = (uint8_t)((type & QUIC_STREAM_FIN) ? 1 : 0);
        pos += out->stream.length;
        return pos;
    }

    if (type == QUIC_FT_MAX_DATA)
    {
        if (!rd(buf, len, &pos, &out->max_data.max))
            return 0;
        return pos;
    }

    if (type == QUIC_FT_CONNECTION_CLOSE || type == QUIC_FT_CONNECTION_CLOSE_APP)
    {
        out->close.app = (uint8_t)((type == QUIC_FT_CONNECTION_CLOSE_APP) ? 1 : 0);
        out->close.frame_type = 0;
        if (!rd(buf, len, &pos, &out->close.error_code))
            return 0;
        if (type == QUIC_FT_CONNECTION_CLOSE) // the transport variant carries the triggering frame type
            if (!rd(buf, len, &pos, &out->close.frame_type))
                return 0;
        if (!rd(buf, len, &pos, &out->close.reason_len))
            return 0;
        if (pos + out->close.reason_len > len)
            return 0;
        out->close.reason = buf + pos;
        pos += out->close.reason_len;
        return pos;
    }

    return 0; // a frame type this minimal server does not handle
}

size_t quic_build_padding(uint8_t *out, size_t cap, size_t n)
{
    if (n > cap)
        return 0;
    memset(out, 0, n);
    return n;
}

size_t quic_build_ping(uint8_t *out, size_t cap)
{
    if (cap < 1)
        return 0;
    out[0] = QUIC_FT_PING;
    return 1;
}

size_t quic_build_handshake_done(uint8_t *out, size_t cap)
{
    if (cap < 1)
        return 0;
    out[0] = QUIC_FT_HANDSHAKE_DONE;
    return 1;
}

namespace
{
// Append a varint; returns false on overflow.
bool wr(uint8_t *out, size_t cap, size_t *pos, uint64_t v)
{
    size_t c = quic_varint_encode(out + *pos, cap - *pos, v);
    if (!c)
        return false;
    *pos += c;
    return true;
}
} // namespace

size_t quic_build_ack(uint8_t *out, size_t cap, uint64_t largest, uint64_t delay, uint64_t first_range)
{
    size_t pos = 0;
    if (!wr(out, cap, &pos, QUIC_FT_ACK) || !wr(out, cap, &pos, largest) || !wr(out, cap, &pos, delay) ||
        !wr(out, cap, &pos, 0) /* ACK Range Count */ || !wr(out, cap, &pos, first_range))
        return 0;
    return pos;
}

size_t quic_build_crypto(uint8_t *out, size_t cap, uint64_t offset, const uint8_t *data, size_t len)
{
    size_t pos = 0;
    if (!wr(out, cap, &pos, QUIC_FT_CRYPTO) || !wr(out, cap, &pos, offset) || !wr(out, cap, &pos, len))
        return 0;
    if (pos + len > cap)
        return 0;
    if (len)
        memcpy(out + pos, data, len);
    return pos + len;
}

size_t quic_build_stream(uint8_t *out, size_t cap, uint64_t id, uint64_t offset, const uint8_t *data, size_t len,
                         bool fin)
{
    uint64_t type = QUIC_FT_STREAM | QUIC_STREAM_LEN | (offset ? QUIC_STREAM_OFF : 0) | (fin ? QUIC_STREAM_FIN : 0);
    size_t pos = 0;
    if (!wr(out, cap, &pos, type) || !wr(out, cap, &pos, id))
        return 0;
    if (offset && !wr(out, cap, &pos, offset))
        return 0;
    if (!wr(out, cap, &pos, len))
        return 0;
    if (pos + len > cap)
        return 0;
    if (len)
        memcpy(out + pos, data, len);
    return pos + len;
}

size_t quic_build_max_data(uint8_t *out, size_t cap, uint64_t max)
{
    size_t pos = 0;
    if (!wr(out, cap, &pos, QUIC_FT_MAX_DATA) || !wr(out, cap, &pos, max))
        return 0;
    return pos;
}

size_t quic_build_connection_close(uint8_t *out, size_t cap, uint64_t error_code, uint64_t frame_type,
                                   const char *reason, size_t reason_len)
{
    size_t pos = 0;
    if (!wr(out, cap, &pos, QUIC_FT_CONNECTION_CLOSE) || !wr(out, cap, &pos, error_code) ||
        !wr(out, cap, &pos, frame_type) || !wr(out, cap, &pos, reason_len))
        return 0;
    if (pos + reason_len > cap)
        return 0;
    if (reason_len)
        memcpy(out + pos, reason, reason_len);
    return pos + reason_len;
}

#endif // DETWS_ENABLE_HTTP3
