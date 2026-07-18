// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file profibus.cpp
 * @brief PROFIBUS-DP FDL telegram codec (see profibus.h).
 */

#include "services/profibus/profibus.h"

#if DWS_ENABLE_PROFIBUS

#include <string.h>

uint8_t dws_pb_fcs(const uint8_t *bytes, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++)
        sum = (uint8_t)(sum + bytes[i]);
    return sum;
}

size_t dws_pb_build_sd1(uint8_t da, uint8_t sa, uint8_t fc, uint8_t *out, size_t cap)
{
    if (!out || cap < 6)
        return 0;
    out[0] = Profibus::PB_SD1;
    out[1] = da;
    out[2] = sa;
    out[3] = fc;
    uint8_t body[3] = {da, sa, fc};
    out[4] = dws_pb_fcs(body, 3);
    out[5] = Profibus::PB_ED;
    return 6;
}

size_t dws_pb_build_sd2(uint8_t da, uint8_t sa, uint8_t fc, const uint8_t *data, size_t data_len, uint8_t *out,
                        size_t cap)
{
    if (!out || (data_len && !data) || data_len > 246)
        return 0;
    // SD2 LE LEr SD2 DA SA FC [data] FCS ED
    size_t n = 4 + 3 + data_len + 2; // (SD2,LE,LEr,SD2) + (DA,SA,FC) + data + (FCS,ED)
    if (n > cap)
        return 0;
    uint8_t le = (uint8_t)(3 + data_len); // length of DA+SA+FC+data
    size_t i = 0;
    out[i++] = Profibus::PB_SD2;
    out[i++] = le;
    out[i++] = le; // LEr (redundant length)
    out[i++] = Profibus::PB_SD2;
    out[i++] = da;
    out[i++] = sa;
    out[i++] = fc;
    if (data_len)
    {
        memcpy(out + i, data, data_len);
        i += data_len;
    }
    // FCS over DA+SA+FC+data (out[4 .. 4+le-1]).
    out[i++] = dws_pb_fcs(out + 4, le);
    out[i++] = Profibus::PB_ED;
    return i;
}

bool dws_pb_parse(const uint8_t *frame, size_t len, PbTelegram *out)
{
    if (!frame || !out || len < 6)
        return false;

    if (frame[0] == Profibus::PB_SD1)
    {
        // SD1 DA SA FC FCS ED (len >= 6 already guaranteed above)
        uint8_t body[3] = {frame[1], frame[2], frame[3]};
        if (dws_pb_fcs(body, 3) != frame[4] || frame[5] != Profibus::PB_ED)
            return false;
        out->sd = Profibus::PB_SD1;
        out->da = frame[1];
        out->sa = frame[2];
        out->fc = frame[3];
        out->data = nullptr;
        out->data_len = 0;
        return true;
    }
    if (frame[0] == Profibus::PB_SD2)
    {
        // SD2 LE LEr SD2 DA SA FC [data] FCS ED
        if (len < 9)
            return false;
        uint8_t le = frame[1];
        if (frame[2] != le || frame[3] != Profibus::PB_SD2)
            return false;
        if (le < 3)
            return false;
        size_t total = 4 + le + 2; // header(4) + le body + FCS + ED
        if (len < total)
            return false;
        if (dws_pb_fcs(frame + 4, le) != frame[4 + le] || frame[4 + le + 1] != Profibus::PB_ED)
            return false;
        out->sd = Profibus::PB_SD2;
        out->da = frame[4];
        out->sa = frame[5];
        out->fc = frame[6];
        size_t dl = le - 3;
        out->data = dl ? (frame + 7) : nullptr;
        out->data_len = dl;
        return true;
    }
    return false;
}

#endif // DWS_ENABLE_PROFIBUS
