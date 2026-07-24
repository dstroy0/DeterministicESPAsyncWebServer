// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea2000.cpp
 * @brief NMEA 2000 codec (Fast Packet over J1939; pure, host-tested).
 */

#include "services/nmea2000/nmea2000.h"

#if DWS_ENABLE_NMEA2000

#include <string.h>

uint8_t dws_n2k_fastpacket_num_frames(uint16_t total_len)
{
    if (total_len <= N2K_FP_F0_DATA)
        return 1;
    return (uint8_t)(1u + (total_len - N2K_FP_F0_DATA + (N2K_FP_FN_DATA - 1)) / N2K_FP_FN_DATA);
}

bool dws_n2k_fastpacket_build_frame(CanFrame *out, uint8_t seq, uint8_t frame_idx, uint8_t priority, uint32_t pgn,
                                    uint8_t sa, uint8_t da, const uint8_t *data, uint16_t total_len)
{
    if (!out || !data || seq > 7 || total_len == 0 || total_len > DWS_N2K_FP_MAX)
        return false;
    if (frame_idx >= dws_n2k_fastpacket_num_frames(total_len))
        return false;
    uint32_t id;
    if (!dws_j1939_encode_id(&id, priority, pgn, sa, da))
        return false;
    out->id = id;
    out->extended = true;
    out->rtr = false;
    out->dlc = DWS_CAN_MAX_DLC;                 // Fast Packet frames are full 8-octet frames
    memset(out->data, 0xFF, sizeof(out->data)); // pad unused octets with 0xFF

    out->data[0] = (uint8_t)((seq << N2K_FP_SEQ_SHIFT) | (frame_idx & N2K_FP_FRAME_MASK));
    if (frame_idx == 0)
    {
        out->data[1] = (uint8_t)total_len;
        uint8_t n = total_len < N2K_FP_F0_DATA ? (uint8_t)total_len : (uint8_t)N2K_FP_F0_DATA;
        memcpy(out->data + 2, data, n);
    }
    else
    {
        uint16_t off = (uint16_t)(N2K_FP_F0_DATA + (frame_idx - 1) * N2K_FP_FN_DATA);
        uint16_t remaining = (uint16_t)(total_len - off);
        uint8_t n = remaining < N2K_FP_FN_DATA ? (uint8_t)remaining : (uint8_t)N2K_FP_FN_DATA;
        memcpy(out->data + 1, data + off, n);
    }
    return true;
}

void dws_n2k_fastpacket_reset(N2kFastPacketRx *rx)
{
    if (rx)
        memset(rx, 0, sizeof(*rx));
}

N2kFpResult dws_n2k_fastpacket_feed(N2kFastPacketRx *rx, const CanFrame *f)
{
    if (!rx || !f || !f->extended || f->dlc < 2)
        return N2kFpResult::N2K_FP_IGNORED;
    J1939Id id;
    if (!dws_j1939_decode_id(f->id, &id))   // GCOVR_EXCL_LINE  unreachable: dws_j1939_decode_id only fails on a null
                                            // out, and &id is non-null
        return N2kFpResult::N2K_FP_IGNORED; // GCOVR_EXCL_LINE  unreachable: dws_j1939_decode_id only fails on a null
                                            // out, and &id is non-null

    uint8_t seq = (uint8_t)(f->data[0] >> N2K_FP_SEQ_SHIFT);
    uint8_t frame_idx = (uint8_t)(f->data[0] & N2K_FP_FRAME_MASK);

    if (frame_idx == 0) // first frame: total length + first 6 data octets
    {
        uint16_t total = f->data[1];
        if (total == 0 || total > DWS_N2K_FP_MAX)
            return N2kFpResult::N2K_FP_ERR;
        dws_n2k_fastpacket_reset(rx);
        rx->active = true;
        rx->seq = seq;
        rx->sa = id.sa;
        rx->pgn = id.pgn;
        rx->total_len = total;
        uint8_t n = total < N2K_FP_F0_DATA ? (uint8_t)total : (uint8_t)N2K_FP_F0_DATA;
        memcpy(rx->buf, f->data + 2, n);
        rx->received = n;
        rx->next_frame = 1;
        if (rx->received >= total)
        {
            rx->active = false;
            return N2kFpResult::N2K_FP_COMPLETE;
        }
        return N2kFpResult::N2K_FP_STARTED;
    }

    // continuation frame: must match the active sequence / source / PGN and be in order.
    if (!rx->active || seq != rx->seq || id.sa != rx->sa || id.pgn != rx->pgn)
        return N2kFpResult::N2K_FP_IGNORED;
    if (frame_idx != rx->next_frame)
    {
        dws_n2k_fastpacket_reset(rx);
        return N2kFpResult::N2K_FP_ERR;
    }
    uint16_t remaining = (uint16_t)(rx->total_len - rx->received);
    uint8_t n = remaining < N2K_FP_FN_DATA ? (uint8_t)remaining : (uint8_t)N2K_FP_FN_DATA;
    memcpy(rx->buf + rx->received, f->data + 1, n);
    rx->received = (uint16_t)(rx->received + n);
    rx->next_frame++;
    if (rx->received >= rx->total_len)
    {
        rx->active = false;
        return N2kFpResult::N2K_FP_COMPLETE;
    }
    return N2kFpResult::N2K_FP_PROGRESS;
}

bool dws_n2k_build_single(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                          uint8_t len)
{
    return dws_j1939_build_message(out, priority, pgn, sa, da, data, len);
}

// --- typed PGN decoders ---

namespace
{
uint16_t rd_u16le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
int32_t rd_i32le(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}
uint32_t rd_u32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
int16_t rd_i16le(const uint8_t *p)
{
    return (int16_t)rd_u16le(p);
}
} // namespace

bool dws_n2k_decode_position_rapid(const uint8_t *payload, size_t len, N2kPositionRapid *out)
{
    if (!payload || !out || len < 8)
        return false;
    int32_t lat = rd_i32le(payload); // 1e-7 deg/bit
    int32_t lon = rd_i32le(payload + 4);
    out->valid = (lat != (int32_t)0x7FFFFFFF && lon != (int32_t)0x7FFFFFFF); // 0x7FFFFFFF = not available
    out->lat_deg = (double)lat * 1e-7;
    out->lon_deg = (double)lon * 1e-7;
    return true;
}

bool dws_n2k_decode_cog_sog_rapid(const uint8_t *payload, size_t len, N2kCogSogRapid *out)
{
    if (!payload || !out || len < 6) // SID(1) + ref(1) + COG(2) + SOG(2)
        return false;
    out->sid = payload[0];
    out->cog_ref = (uint8_t)(payload[1] & 0x03u);
    uint16_t cog = rd_u16le(payload + 2); // 0.0001 rad per bit
    uint16_t sog = rd_u16le(payload + 4); // 0.01 m/s per bit
    out->cog_valid = (cog != 0xFFFFu);
    out->cog_rad = (float)cog * 0.0001f;
    out->sog_valid = (sog != 0xFFFFu);
    out->sog_mps = (float)sog * 0.01f;
    return true;
}

bool dws_n2k_decode_engine_rapid(const uint8_t *payload, size_t len, N2kEngineRapid *out)
{
    if (!payload || !out || len < 6) // instance(1) + speed(2) + boost(2) + tilt(1)
        return false;
    out->instance = payload[0];
    uint16_t rpm = rd_u16le(payload + 1); // 0.25 rpm per bit
    out->speed_valid = (rpm != 0xFFFFu);
    out->speed_rpm = (float)rpm * 0.25f;
    uint16_t boost = rd_u16le(payload + 3); // 100 Pa per bit
    out->boost_valid = (boost != 0xFFFFu);
    out->boost_pa = (float)boost * 100.0f;
    out->tilt_valid = (payload[5] != 0x7Fu); // 0x7F = not-available for a signed 1-octet field
    out->tilt_pct = (int8_t)payload[5];
    return true;
}

bool dws_n2k_decode_wind_data(const uint8_t *payload, size_t len, N2kWindData *out)
{
    if (!payload || !out || len < 6)
        return false;
    out->sid = payload[0];
    uint16_t speed = rd_u16le(payload + 1); // 0.01 m/s per bit
    uint16_t angle = rd_u16le(payload + 3); // 0.0001 rad per bit
    out->speed_valid = (speed != 0xFFFFu);
    out->speed_mps = (float)speed * 0.01f;
    out->angle_valid = (angle != 0xFFFFu);
    out->angle_rad = (float)angle * 0.0001f;
    out->reference = (uint8_t)(payload[5] & 0x07u);
    return true;
}

bool dws_n2k_decode_water_depth(const uint8_t *payload, size_t len, N2kWaterDepth *out)
{
    if (!payload || !out || len < 7) // SID(1) + depth(4) + offset(2)
        return false;
    out->sid = payload[0];
    uint32_t depth = rd_u32le(payload + 1); // 0.01 m per bit
    out->depth_valid = (depth != 0xFFFFFFFFu);
    out->depth_m = (float)depth * 0.01f;
    out->offset_m = (float)rd_i16le(payload + 5) * 0.001f; // 0.001 m per bit
    return true;
}

bool dws_n2k_decode_vessel_heading(const uint8_t *payload, size_t len, N2kVesselHeading *out)
{
    if (!payload || !out || len < 8)
        return false;
    out->sid = payload[0];
    uint16_t heading = rd_u16le(payload + 1); // 0.0001 rad per bit
    out->heading_valid = (heading != 0xFFFFu);
    out->heading_rad = (float)heading * 0.0001f;
    out->deviation_rad = (float)rd_i16le(payload + 3) * 0.0001f;
    out->variation_rad = (float)rd_i16le(payload + 5) * 0.0001f;
    out->reference = (uint8_t)(payload[7] & 0x03u);
    return true;
}

#endif // DWS_ENABLE_NMEA2000
