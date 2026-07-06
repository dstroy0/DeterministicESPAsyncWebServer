// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file happy_eyeballs.cpp
 * @brief Dual-stack destination selection + Happy Eyeballs fallback (see happy_eyeballs.h).
 */

#include "services/happy_eyeballs/happy_eyeballs.h"

#if DETWS_ENABLE_HAPPY_EYEBALLS

namespace
{
// Effective family for interleave: an IPv4-mapped IPv6 address is treated as IPv4.
bool eff_is_v6(const DetIp *ip)
{
    return ip->family == DET_IP_V6 && !det_ip_is_v4_mapped(ip);
}

int scope_rank(const DetIp *ip)
{
    switch (det_ip_classify(ip))
    {
    case DET_IP_SCOPE_GLOBAL:
        return 5;
    case DET_IP_SCOPE_PRIVATE:
        return 4;
    case DET_IP_SCOPE_LINK_LOCAL:
        return 3;
    case DET_IP_SCOPE_LOOPBACK:
        return 2;
    case DET_IP_SCOPE_MULTICAST:
        return 1;
    default:
        return 0; // unspecified
    }
}
} // namespace

int detws_he_pref(const DetIp *ip)
{
    if (!ip || ip->family == DET_IP_NONE)
        return -1;
    // Scope dominates; within a scope a native IPv6 outranks IPv4 (RFC 6724 default policy).
    return scope_rank(ip) * 2 + (eff_is_v6(ip) ? 1 : 0);
}

void detws_he_order(DetIp *list, size_t n)
{
    if (!list || n < 2)
        return;

    // Stable insertion sort by preference (descending).
    for (size_t i = 1; i < n; i++)
    {
        DetIp key = list[i];
        int kp = detws_he_pref(&key);
        size_t j = i;
        while (j > 0 && detws_he_pref(&list[j - 1]) < kp)
        {
            list[j] = list[j - 1];
            j--;
        }
        list[j] = key;
    }

    if (n > DETWS_HE_MAX)
        return; // too large to interleave in the fixed scratch; sorted order stands.

    // RFC 8305 sec 4: interleave families so successive attempts alternate v6/v4. Preserve the
    // preference order within each family; start with the family of the top-preference address.
    DetIp out[DETWS_HE_MAX];
    size_t o = 0;
    size_t iv6 = 0, iv4 = 0;
    // Collect indices per family in preference order.
    size_t v6[DETWS_HE_MAX], v4[DETWS_HE_MAX];
    size_t nv6 = 0, nv4 = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (eff_is_v6(&list[i]))
            v6[nv6++] = i;
        else
            v4[nv4++] = i;
    }
    bool take_v6 = eff_is_v6(&list[0]); // whichever family the best address belongs to goes first
    while (iv6 < nv6 || iv4 < nv4)
    {
        if (take_v6 && iv6 < nv6)
            out[o++] = list[v6[iv6++]];
        else if (!take_v6 && iv4 < nv4)
            out[o++] = list[v4[iv4++]];
        else if (iv6 < nv6) // the preferred family is exhausted; drain the other
            out[o++] = list[v6[iv6++]];
        else
            out[o++] = list[v4[iv4++]];
        take_v6 = !take_v6;
    }
    for (size_t i = 0; i < n; i++)
        list[i] = out[i];
}

bool detws_he_attempt_due(uint32_t last_start_ms, uint32_t now_ms, uint32_t attempt_delay_ms)
{
    uint32_t elapsed = now_ms - last_start_ms; // wrap-safe modular subtraction
    return elapsed >= attempt_delay_ms;
}

#endif // DETWS_ENABLE_HAPPY_EYEBALLS
