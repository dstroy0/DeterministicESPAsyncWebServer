// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file rtcm3.cpp
 * @brief RTCM 3.x framing + 1005/1006 codec - implementation. See rtcm3.h.
 */

#include "services/gnss/rtcm3.h"

#if DETWS_ENABLE_NTRIP_CASTER

#include <string.h>

// ---------------------------------------------------------------------------------------------
// CRC-24Q (poly 0x1864CFB, init 0). Computed over the preamble + header + payload of a frame.
// ---------------------------------------------------------------------------------------------

uint32_t rtcm3_crc24q(const uint8_t *data, size_t len)
{
    uint32_t crc = 0;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (uint32_t)data[i] << 16;
        for (int b = 0; b < 8; b++)
        {
            crc <<= 1;
            if (crc & 0x1000000u)
                crc ^= 0x1864CFBu;
        }
    }
    return crc & 0xFFFFFFu;
}

// ---------------------------------------------------------------------------------------------
// MSB-first bit I/O.
// ---------------------------------------------------------------------------------------------

void rtcm_bw_init(RtcmBitWriter *w, uint8_t *buf, size_t cap)
{
    w->buf = buf;
    w->cap_bits = cap * 8;
    w->pos = 0;
    w->ok = true;
}

void rtcm_bw_u(RtcmBitWriter *w, uint64_t val, uint8_t nbits)
{
    if (!w->ok)
        return;
    if (nbits == 0 || nbits > 64 || w->pos + nbits > w->cap_bits)
    {
        w->ok = false;
        return;
    }
    for (int i = (int)nbits - 1; i >= 0; i--)
    {
        if ((val >> i) & 1u) // buffer is pre-zeroed, so only set the 1 bits
        {
            size_t bp = w->pos;
            w->buf[bp >> 3] |= (uint8_t)(0x80u >> (bp & 7u));
        }
        w->pos++;
    }
}

void rtcm_bw_s(RtcmBitWriter *w, int64_t val, uint8_t nbits)
{
    uint64_t mask = (nbits >= 64) ? ~0ULL : ((1ULL << nbits) - 1ULL);
    rtcm_bw_u(w, (uint64_t)val & mask, nbits);
}

uint64_t rtcm_br_u(const uint8_t *buf, size_t *pos, uint8_t nbits)
{
    uint64_t v = 0;
    for (uint8_t i = 0; i < nbits; i++)
    {
        size_t bp = (*pos)++;
        uint64_t bit = (buf[bp >> 3] >> (7u - (bp & 7u))) & 1u;
        v = (v << 1) | bit;
    }
    return v;
}

int64_t rtcm_br_s(const uint8_t *buf, size_t *pos, uint8_t nbits)
{
    uint64_t v = rtcm_br_u(buf, pos, nbits);
    if (nbits < 64 && (v & (1ULL << (nbits - 1))))
        v |= ~((1ULL << nbits) - 1ULL); // sign-extend the two's-complement value
    return (int64_t)v;
}

// ---------------------------------------------------------------------------------------------
// Transport frame.
// ---------------------------------------------------------------------------------------------

size_t rtcm3_sync(const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        if (buf[i] == RTCM3_PREAMBLE)
            return i;
    return len;
}

size_t rtcm3_frame_parse(const uint8_t *buf, size_t len, Rtcm3Frame *out)
{
    if (len < RTCM3_HDR_LEN || buf[0] != RTCM3_PREAMBLE)
        return 0; // not aligned to a preamble (caller runs rtcm3_sync first)
    uint16_t payload_len = (uint16_t)(((buf[1] & 0x03u) << 8) | buf[2]);
    size_t frame_len = (size_t)RTCM3_HDR_LEN + payload_len + RTCM3_CRC_LEN;
    if (len < frame_len)
        return 0; // whole frame not buffered yet
    const uint8_t *payload = buf + RTCM3_HDR_LEN;
    uint32_t crc_calc = rtcm3_crc24q(buf, (size_t)RTCM3_HDR_LEN + payload_len);
    uint32_t crc_frame =
        ((uint32_t)payload[payload_len] << 16) | ((uint32_t)payload[payload_len + 1] << 8) | payload[payload_len + 2];
    if (out)
    {
        out->payload = payload;
        out->payload_len = payload_len;
        out->crc_ok = (crc_calc == crc_frame);
        size_t p = 0;
        out->msg_type = (payload_len >= 2) ? (uint16_t)rtcm_br_u(payload, &p, 12) : 0;
    }
    return frame_len;
}

size_t rtcm3_frame_build(uint8_t *out, size_t cap, const uint8_t *payload, uint16_t payload_len)
{
    if (payload_len > RTCM3_MAX_PAYLOAD)
        return 0;
    size_t frame_len = (size_t)RTCM3_HDR_LEN + payload_len + RTCM3_CRC_LEN;
    if (!out || cap < frame_len)
        return 0;
    out[0] = RTCM3_PREAMBLE;
    out[1] = (uint8_t)((payload_len >> 8) & 0x03u); // top 6 bits reserved = 0
    out[2] = (uint8_t)(payload_len & 0xFFu);
    if (payload_len && payload)
        memcpy(out + RTCM3_HDR_LEN, payload, payload_len);
    uint32_t crc = rtcm3_crc24q(out, (size_t)RTCM3_HDR_LEN + payload_len);
    out[RTCM3_HDR_LEN + payload_len] = (uint8_t)((crc >> 16) & 0xFFu);
    out[RTCM3_HDR_LEN + payload_len + 1] = (uint8_t)((crc >> 8) & 0xFFu);
    out[RTCM3_HDR_LEN + payload_len + 2] = (uint8_t)(crc & 0xFFu);
    return frame_len;
}

// ---------------------------------------------------------------------------------------------
// Message 1005 / 1006 - Stationary Antenna Reference Point (fields per RTCM 10403.x).
// ---------------------------------------------------------------------------------------------

namespace
{
size_t build_arp(uint8_t *out, size_t cap, uint16_t msg, uint16_t station_id, int64_t x, int64_t y, int64_t z,
                 bool with_h, uint16_t h)
{
    uint8_t payload[21];
    memset(payload, 0, sizeof payload);
    RtcmBitWriter w;
    rtcm_bw_init(&w, payload, sizeof payload);
    rtcm_bw_u(&w, msg, 12);                // DF002 message number
    rtcm_bw_u(&w, station_id & 0xFFF, 12); // DF003 reference station id
    rtcm_bw_u(&w, 0, 6);                   // DF021 ITRF realization year (unspecified)
    rtcm_bw_u(&w, 1, 1);                   // DF022 GPS indicator
    rtcm_bw_u(&w, 0, 1);                   // DF023 GLONASS indicator
    rtcm_bw_u(&w, 0, 1);                   // DF024 Galileo indicator
    rtcm_bw_u(&w, 0, 1);                   // DF141 reference-station indicator (0 = real/physical)
    rtcm_bw_s(&w, x, 38);                  // DF025 ECEF-X
    rtcm_bw_u(&w, 0, 1);                   // DF142 single receiver oscillator indicator
    rtcm_bw_u(&w, 0, 1);                   // reserved
    rtcm_bw_s(&w, y, 38);                  // DF026 ECEF-Y
    rtcm_bw_u(&w, 0, 2);                   // DF364 quarter cycle indicator
    rtcm_bw_s(&w, z, 38);                  // DF027 ECEF-Z
    uint16_t body_bytes = 19;
    if (with_h)
    {
        rtcm_bw_u(&w, h, 16); // DF028 antenna height (1006)
        body_bytes = 21;
    }
    if (!w.ok)
        return 0;
    return rtcm3_frame_build(out, cap, payload, body_bytes);
}
} // namespace

size_t rtcm3_build_1005(uint8_t *out, size_t cap, uint16_t station_id, int64_t ecef_x_01mm, int64_t ecef_y_01mm,
                        int64_t ecef_z_01mm)
{
    return build_arp(out, cap, 1005, station_id, ecef_x_01mm, ecef_y_01mm, ecef_z_01mm, false, 0);
}

size_t rtcm3_build_1006(uint8_t *out, size_t cap, uint16_t station_id, int64_t ecef_x_01mm, int64_t ecef_y_01mm,
                        int64_t ecef_z_01mm, uint16_t antenna_height_01mm)
{
    return build_arp(out, cap, 1006, station_id, ecef_x_01mm, ecef_y_01mm, ecef_z_01mm, true, antenna_height_01mm);
}

bool rtcm3_parse_1005(const uint8_t *payload, uint16_t payload_len, Rtcm3StationArp *out)
{
    if (!payload || !out || payload_len < 19)
        return false;
    size_t p = 0;
    uint16_t msg = (uint16_t)rtcm_br_u(payload, &p, 12);
    if (msg != 1005 && msg != 1006)
        return false;
    if (msg == 1006 && payload_len < 21)
        return false;
    out->station_id = (uint16_t)rtcm_br_u(payload, &p, 12);
    rtcm_br_u(payload, &p, 6); // DF021
    rtcm_br_u(payload, &p, 1); // DF022
    rtcm_br_u(payload, &p, 1); // DF023
    rtcm_br_u(payload, &p, 1); // DF024
    rtcm_br_u(payload, &p, 1); // DF141
    out->ecef_x_01mm = rtcm_br_s(payload, &p, 38);
    rtcm_br_u(payload, &p, 1); // DF142
    rtcm_br_u(payload, &p, 1); // reserved
    out->ecef_y_01mm = rtcm_br_s(payload, &p, 38);
    rtcm_br_u(payload, &p, 2); // DF364
    out->ecef_z_01mm = rtcm_br_s(payload, &p, 38);
    if (msg == 1006)
    {
        out->antenna_height_01mm = (uint16_t)rtcm_br_u(payload, &p, 16);
        out->has_height = true;
    }
    else
    {
        out->antenna_height_01mm = 0;
        out->has_height = false;
    }
    return true;
}

#endif // DETWS_ENABLE_NTRIP_CASTER
