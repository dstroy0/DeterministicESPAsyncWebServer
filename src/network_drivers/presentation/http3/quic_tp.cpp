// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_tp.cpp
 * @brief QUIC transport parameters codec (see quic_tp.h).
 */

#include "network_drivers/presentation/http3/quic_tp.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>

void quic_tp_defaults(QuicTransportParams *tp)
{
    memset(tp, 0, sizeof(*tp));
    tp->max_udp_payload_size = 65527;
    tp->ack_delay_exponent = 3;
    tp->max_ack_delay = 25;
    tp->active_connection_id_limit = 2;
}

namespace
{
// Append one parameter: ID (varint) || Length (varint) || raw value bytes.
bool put_param(uint8_t *out, size_t cap, size_t *p, uint64_t id, const uint8_t *val, size_t val_len)
{
    size_t n = quic_varint_encode(out + *p, cap - *p, id);
    if (!n)
        return false;
    *p += n;
    n = quic_varint_encode(out + *p, cap - *p, val_len);
    if (!n)
        return false;
    *p += n;
    if (val_len)
    {
        if (*p + val_len > cap)
            return false;
        memcpy(out + *p, val, val_len);
        *p += val_len;
    }
    return true;
}

// Append a varint-valued parameter (Value is itself a varint).
bool put_varint_param(uint8_t *out, size_t cap, size_t *p, uint64_t id, uint64_t value)
{
    uint8_t v[8];
    size_t vlen = quic_varint_encode(v, sizeof(v), value);
    if (!vlen)
        return false;
    return put_param(out, cap, p, id, v, vlen);
}
} // namespace

size_t quic_tp_encode(const QuicTransportParams *tp, uint8_t *out, size_t cap)
{
    size_t p = 0;
    bool ok = true;
    if (tp->has_original_dcid)
        ok = ok && put_param(out, cap, &p, QUIC_TP_ORIGINAL_DCID, tp->original_dcid, tp->original_dcid_len);
    if (tp->has_initial_scid)
        ok = ok && put_param(out, cap, &p, QUIC_TP_INITIAL_SCID, tp->initial_scid, tp->initial_scid_len);
    if (tp->has_retry_scid)
        ok = ok && put_param(out, cap, &p, QUIC_TP_RETRY_SCID, tp->retry_scid, tp->retry_scid_len);

    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_DATA, tp->initial_max_data);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_SD_BIDI_LOCAL, tp->initial_max_sd_bidi_local);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_SD_BIDI_REMOTE, tp->initial_max_sd_bidi_remote);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_SD_UNI, tp->initial_max_sd_uni);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_STREAMS_BIDI, tp->initial_max_streams_bidi);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_INITIAL_MAX_STREAMS_UNI, tp->initial_max_streams_uni);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_MAX_IDLE_TIMEOUT, tp->max_idle_timeout);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_MAX_UDP_PAYLOAD_SIZE, tp->max_udp_payload_size);
    ok = ok && put_varint_param(out, cap, &p, QUIC_TP_ACTIVE_CID_LIMIT, tp->active_connection_id_limit);
    if (tp->disable_active_migration)
        ok = ok && put_param(out, cap, &p, QUIC_TP_DISABLE_ACTIVE_MIGRATION, nullptr, 0);

    return ok ? p : 0;
}

namespace
{
// Decode the varint that IS the whole value of a varint-valued parameter (must consume exactly len).
bool value_varint(const uint8_t *val, size_t len, uint64_t *out)
{
    size_t consumed = 0;
    if (!quic_varint_decode(val, len, out, &consumed))
        return false;
    return consumed == len;
}

// Copy a connection-ID value (<= QUIC_MAX_CID_LEN) into a fixed field.
bool copy_cid(const uint8_t *val, size_t len, uint8_t *dst, uint8_t *dst_len, bool *has)
{
    if (len > QUIC_MAX_CID_LEN)
        return false;
    memcpy(dst, val, len);
    *dst_len = (uint8_t)len;
    *has = true;
    return true;
}
} // namespace

bool quic_tp_parse(const uint8_t *buf, size_t len, QuicTransportParams *tp)
{
    quic_tp_defaults(tp);
    uint32_t seen = 0; // dup-guard bitmask over the known IDs (all < 32)

    size_t off = 0;
    while (off < len)
    {
        uint64_t id = 0, vlen = 0;
        size_t c = 0;
        if (!quic_varint_decode(buf + off, len - off, &id, &c))
            return false;
        off += c;
        if (!quic_varint_decode(buf + off, len - off, &vlen, &c))
            return false;
        off += c;
        if (off + vlen > len)
            return false;
        const uint8_t *val = buf + off;
        off += vlen;

        if (id < 32)
        {
            uint32_t bit = 1u << id;
            if (seen & bit)
                return false; // a known parameter must not appear twice
            seen |= bit;
        }

        switch (id)
        {
        case QUIC_TP_ORIGINAL_DCID:
            if (!copy_cid(val, vlen, tp->original_dcid, &tp->original_dcid_len, &tp->has_original_dcid))
                return false;
            break;
        case QUIC_TP_INITIAL_SCID:
            if (!copy_cid(val, vlen, tp->initial_scid, &tp->initial_scid_len, &tp->has_initial_scid))
                return false;
            break;
        case QUIC_TP_RETRY_SCID:
            if (!copy_cid(val, vlen, tp->retry_scid, &tp->retry_scid_len, &tp->has_retry_scid))
                return false;
            break;
        case QUIC_TP_MAX_IDLE_TIMEOUT:
            if (!value_varint(val, vlen, &tp->max_idle_timeout))
                return false;
            break;
        case QUIC_TP_MAX_UDP_PAYLOAD_SIZE:
            if (!value_varint(val, vlen, &tp->max_udp_payload_size) || tp->max_udp_payload_size < 1200)
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_DATA:
            if (!value_varint(val, vlen, &tp->initial_max_data))
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_SD_BIDI_LOCAL:
            if (!value_varint(val, vlen, &tp->initial_max_sd_bidi_local))
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_SD_BIDI_REMOTE:
            if (!value_varint(val, vlen, &tp->initial_max_sd_bidi_remote))
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_SD_UNI:
            if (!value_varint(val, vlen, &tp->initial_max_sd_uni))
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_STREAMS_BIDI:
            if (!value_varint(val, vlen, &tp->initial_max_streams_bidi))
                return false;
            break;
        case QUIC_TP_INITIAL_MAX_STREAMS_UNI:
            if (!value_varint(val, vlen, &tp->initial_max_streams_uni))
                return false;
            break;
        case QUIC_TP_ACK_DELAY_EXPONENT:
            if (!value_varint(val, vlen, &tp->ack_delay_exponent) || tp->ack_delay_exponent > 20)
                return false;
            break;
        case QUIC_TP_MAX_ACK_DELAY:
            if (!value_varint(val, vlen, &tp->max_ack_delay) || tp->max_ack_delay >= (1u << 14))
                return false;
            break;
        case QUIC_TP_ACTIVE_CID_LIMIT:
            if (!value_varint(val, vlen, &tp->active_connection_id_limit) || tp->active_connection_id_limit < 2)
                return false;
            break;
        case QUIC_TP_DISABLE_ACTIVE_MIGRATION:
            if (vlen != 0)
                return false;
            tp->disable_active_migration = true;
            break;
        default:
            break; // unknown / GREASE: skip
        }
    }
    return true;
}

#endif // DETWS_ENABLE_HTTP3
