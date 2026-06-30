// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file physical.cpp
 * @brief Layer 1 (Physical) - link bring-up and egress reporting.
 *
 * WiFi station bring-up is asynchronous (poll wifi_ready()). det_net_egress()
 * reports the live default-route interface (lwIP `netif_default`) and classifies
 * it against the WiFi IPs; the stack owns failover, so there is nothing to poll
 * or track here. The IP classifier is pure and host-tested.
 */

#include "network_drivers/physical/physical.h"

#ifdef ARDUINO

bool init_wifi_physical(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    return true;
}

bool wifi_ready()
{
    return WiFi.isConnected();
}

uint32_t det_net_egress_ip(void)
{
    // netif_default is the current default-route interface (the egress).
    return netif_default ? ip4_addr_get_u32(ip_2_ip4(&netif_default->ip_addr)) : 0;
}

DetIface det_net_egress(void)
{
    uint32_t egress = det_net_egress_ip();
    if (egress == 0)
        return DETIFACE_ANY;
    uint32_t sta = WiFi.isConnected() ? (uint32_t)WiFi.localIP() : 0;
    uint32_t ap = (WiFi.getMode() & WIFI_AP) ? (uint32_t)WiFi.softAPIP() : 0;
    return det_net_classify_ip(egress, sta, ap);
}

#else // host build - no radio / stack

bool init_wifi_physical(const char *, const char *)
{
    return true;
}
bool wifi_ready()
{
    return true;
}
uint32_t det_net_egress_ip(void)
{
    return 0;
}
DetIface det_net_egress(void)
{
    return DETIFACE_ANY;
}

#endif // ARDUINO

// Pure classifier (always compiled, host-tested).
DetIface det_net_classify_ip(uint32_t egress_ip, uint32_t sta_ip, uint32_t ap_ip)
{
    if (egress_ip == 0)
        return DETIFACE_ANY;
    if (sta_ip != 0 && egress_ip == sta_ip)
        return DETIFACE_STA;
    if (ap_ip != 0 && egress_ip == ap_ip)
        return DETIFACE_AP;
    return DETIFACE_ETH; // a live route that is neither WiFi IP -> wired
}
