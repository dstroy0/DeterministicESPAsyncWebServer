// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea2000.cpp
 * @brief NMEA 2000 codec (Fast Packet over J1939; pure, host-tested).
 */

#include "services/nmea2000/nmea2000.h"

#if DETWS_ENABLE_NMEA2000

#include <string.h>

uint8_t n2k_fastpacket_num_frames(uint16_t total_len)
{
    if (total_len <= N2K_FP_F0_DATA)
        return 1;
    return (uint8_t)(1u + (total_len - N2K_FP_F0_DATA + (N2K_FP_FN_DATA - 1)) / N2K_FP_FN_DATA);
}

bool n2k_fastpacket_build_frame(CanFrame *out, uint8_t seq, uint8_t frame_idx, uint8_t priority, uint32_t pgn,
                                uint8_t sa, uint8_t da, const uint8_t *data, uint16_t total_len)
{
    if (!out || !data || seq > 7 || total_len == 0 || total_len > DETWS_N2K_FP_MAX)
        return false;
    if (frame_idx >= n2k_fastpacket_num_frames(total_len))
        return false;
    uint32_t id;
    if (!j1939_encode_id(&id, priority, pgn, sa, da))
        return false;
    out->id = id;
    out->extended = true;
    out->rtr = false;
    out->dlc = DET_CAN_MAX_DLC;                 // Fast Packet frames are full 8-octet frames
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

void n2k_fastpacket_reset(N2kFastPacketRx *rx)
{
    if (rx)
        memset(rx, 0, sizeof(*rx));
}

N2kFpResult n2k_fastpacket_feed(N2kFastPacketRx *rx, const CanFrame *f)
{
    if (!rx || !f || !f->extended || f->dlc < 2)
        return N2kFpResult::N2K_FP_IGNORED;
    J1939Id id;
    if (!j1939_decode_id(f->id, &id))
        return N2kFpResult::N2K_FP_IGNORED; // GCOVR_EXCL_LINE  unreachable: j1939_decode_id only fails on a null out,
                                            // and &id is non-null

    uint8_t seq = (uint8_t)(f->data[0] >> N2K_FP_SEQ_SHIFT);
    uint8_t frame_idx = (uint8_t)(f->data[0] & N2K_FP_FRAME_MASK);

    if (frame_idx == 0) // first frame: total length + first 6 data octets
    {
        uint16_t total = f->data[1];
        if (total == 0 || total > DETWS_N2K_FP_MAX)
            return N2kFpResult::N2K_FP_ERR;
        n2k_fastpacket_reset(rx);
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
        n2k_fastpacket_reset(rx);
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

bool n2k_build_single(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                      uint8_t len)
{
    return j1939_build_message(out, priority, pgn, sa, da, data, len);
}

#endif // DETWS_ENABLE_NMEA2000
