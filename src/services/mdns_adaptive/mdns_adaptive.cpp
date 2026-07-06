// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mdns_adaptive.cpp
 * @brief Adaptive mDNS beacon scheduling (see mdns_adaptive.h).
 */

#include "services/mdns_adaptive/mdns_adaptive.h"

#if DETWS_ENABLE_MDNS_ADAPTIVE

uint32_t detws_mdns_refresh_interval(uint32_t ttl_s)
{
    // Half the TTL, in ms; guard the *1000 against overflow.
    uint64_t half_ms = (uint64_t)ttl_s * 1000 / 2;
    return half_ms > 0xFFFFFFFFu ? 0xFFFFFFFFu : (uint32_t)half_ms;
}

void detws_mdns_beacon_init(MdnsBeacon *b, uint32_t base_ms, uint32_t max_ms, uint16_t hi_thresh)
{
    if (!b)
        return;
    b->base_ms = base_ms;
    b->max_ms = max_ms < base_ms ? base_ms : max_ms;
    b->cur_ms = base_ms;
    b->hi_thresh = hi_thresh ? hi_thresh : 1;
}

uint32_t detws_mdns_beacon_adapt(MdnsBeacon *b, uint16_t contention)
{
    if (!b)
        return 0;
    if (contention >= b->hi_thresh)
    {
        uint32_t up = b->cur_ms << 1;
        if (up < b->cur_ms || up > b->max_ms) // overflow or past ceiling
            up = b->max_ms;
        b->cur_ms = up;
    }
    else if (contention == 0)
    {
        uint32_t down = b->cur_ms >> 1;
        if (down < b->base_ms)
            down = b->base_ms;
        b->cur_ms = down;
    }
    return b->cur_ms;
}

bool detws_mdns_beacon_due(const MdnsBeacon *b, uint32_t last_ms, uint32_t now_ms)
{
    if (!b)
        return false;
    uint32_t elapsed = now_ms - last_ms; // wrap-safe modular subtraction
    return elapsed >= b->cur_ms;
}

bool detws_mdns_beacon_presleep_due(const MdnsBeacon *b, uint32_t last_ms, uint32_t now_ms, uint32_t sleep_ms)
{
    if (!b)
        return false;
    uint32_t elapsed = now_ms - last_ms;
    // Would the record lapse during the sleep? Compute in 64-bit so elapsed + sleep_ms cannot wrap.
    uint64_t after = (uint64_t)elapsed + sleep_ms;
    return after >= b->cur_ms;
}

#endif // DETWS_ENABLE_MDNS_ADAPTIVE
