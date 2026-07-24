// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file roaming.cpp
 * @brief Wi-Fi roaming decision layer - see roaming.h.
 */

#include "services/roaming/roaming.h"

#if DWS_ENABLE_ROAMING

#include <string.h>

namespace
{
bool mac_eq(const uint8_t *a, const uint8_t *b)
{
    return memcmp(a, b, 6) == 0;
}

// Index of the strongest candidate that is not the current AP, or -1 if there is none.
int best_other(const uint8_t *current, const DwsRoamNeighbor *nb, uint8_t n)
{
    int best = -1;
    for (uint8_t i = 0; i < n; i++)
    {
        if (mac_eq(nb[i].bssid, current))
            continue;
        if (best < 0 || nb[i].rssi_dbm > nb[best].rssi_dbm)
            best = (int)i;
    }
    return best;
}

// Index of the candidate whose BSSID equals target, or -1.
int find_bssid(const uint8_t *target, const DwsRoamNeighbor *nb, uint8_t n)
{
    for (uint8_t i = 0; i < n; i++)
        if (mac_eq(nb[i].bssid, target))
            return (int)i;
    return -1;
}

void pick(DwsRoamDecision *out, const DwsRoamNeighbor *nb, int idx, DwsRoamReason reason)
{
    out->roam = true;
    memcpy(out->target_bssid, nb[idx].bssid, 6);
    out->target_channel = nb[idx].channel;
    out->reason = reason;
}
} // namespace

void dws_roam_decide(const uint8_t current_bssid[6], int8_t current_rssi_dbm, const DwsRoamNeighbor *neighbors,
                     uint8_t n, const DwsRoamBtm *btm, const DwsRoamPolicy *policy, DwsRoamDecision *out)
{
    if (!out)
        return;
    out->roam = false;
    memset(out->target_bssid, 0, 6);
    out->target_channel = 0;
    out->reason = DWS_ROAM_NONE;
    if (!current_bssid || (n && !neighbors))
        return;

    // A conservative default when the caller passes no policy.
    int8_t threshold = policy ? policy->roam_rssi_threshold_dbm : (int8_t)-75;
    uint8_t hysteresis = policy ? policy->hysteresis_db : (uint8_t)8;

    int best = best_other(current_bssid, neighbors, n);

    // 1. A disassociation-imminent BTM forces us off: roam to the preferred candidate if present, else the
    //    strongest one. (If there is nowhere to go, we cannot roam.)
    if (btm && btm->present && btm->disassoc_imminent)
    {
        int target = -1;
        if (btm->has_preferred)
            target = find_bssid(btm->preferred_bssid, neighbors, n);
        if (target < 0)
            target = best;
        if (target >= 0)
            pick(out, neighbors, target, DWS_ROAM_BTM_IMMINENT);
        return;
    }

    // 2. A non-imminent BTM steering hint: honour it when the preferred AP is a real candidate that is not
    //    weaker than the current link (do not chase the network into a worse AP).
    if (btm && btm->present && btm->has_preferred)
    {
        int target = find_bssid(btm->preferred_bssid, neighbors, n);
        if (target >= 0 && neighbors[target].rssi_dbm >= current_rssi_dbm)
        {
            pick(out, neighbors, target, DWS_ROAM_BTM_SUGGESTED);
            return;
        }
    }

    // 3. RSSI-driven: only when the current link is at/below the threshold and the strongest candidate beats
    //    it by at least the hysteresis margin (so we do not ping-pong on a marginal difference).
    if (best >= 0 && current_rssi_dbm <= threshold &&
        (int)neighbors[best].rssi_dbm >= (int)current_rssi_dbm + (int)hysteresis)
    {
        pick(out, neighbors, best, DWS_ROAM_LOW_RSSI);
        return;
    }

    // 4. Otherwise stay put.
}

#endif // DWS_ENABLE_ROAMING
