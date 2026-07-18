// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file wifi_sniffer.cpp
 * @brief 802.11 frame decode + traffic tally + RSSI roaming decision (see wifi_sniffer.h).
 */

#include "services/wifi_sniffer/wifi_sniffer.h"

#if DWS_ENABLE_WIFI_SNIFFER

#include <string.h>

bool dws_wifi_parse(const uint8_t *frame, size_t len, WifiFrame *out)
{
    if (!frame || !out || len < 10) // FrameControl(2) + Duration(2) + Address1(6)
        return false;

    memset(out, 0, sizeof(*out));

    uint8_t fc0 = frame[0];
    uint8_t fc1 = frame[1];
    out->version = fc0 & 0x03;
    out->type = (fc0 >> 2) & 0x03;
    out->subtype = (fc0 >> 4) & 0x0F;
    out->to_ds = (fc1 & 0x01) != 0;
    out->from_ds = (fc1 & 0x02) != 0;
    out->retry = (fc1 & 0x08) != 0;
    out->protected_frame = (fc1 & 0x40) != 0;

    // Address1 always present at this length (offset 4).
    memcpy(out->addr1, frame + 4, 6);
    out->naddr = 1;
    if (len >= 16)
    {
        memcpy(out->addr2, frame + 10, 6);
        out->naddr = 2;
    }
    if (len >= 24)
    {
        memcpy(out->addr3, frame + 16, 6);
        out->naddr = 3;
    }
    return true;
}

void dws_wifi_stats_reset(WifiStats *s)
{
    if (s)
        memset(s, 0, sizeof(*s));
}

void dws_wifi_stats_add(WifiStats *s, const WifiFrame *f)
{
    if (!s || !f)
        return;
    switch (f->type)
    {
    case WifiType::WIFI_TYPE_MGMT:
        s->mgmt++;
        break;
    case WifiType::WIFI_TYPE_CTRL:
        s->ctrl++;
        break;
    case WifiType::WIFI_TYPE_DATA:
        s->data++;
        break;
    default:
        s->other++;
        break;
    }
    s->total++;
}

bool dws_wifi_should_roam(int8_t cur_rssi, int8_t cand_rssi, uint8_t hysteresis_db)
{
    // Both are negative dBm (stronger = closer to 0). Roam only if the candidate clears the current
    // by more than the hysteresis, computed in a wide signed type to avoid int8 overflow.
    int32_t margin = (int32_t)cand_rssi - (int32_t)cur_rssi;
    return margin > (int32_t)hysteresis_db;
}

#endif // DWS_ENABLE_WIFI_SNIFFER
