// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file physical.h
 * @brief Layer 1 (Physical) - link bring-up and live egress-interface reporting.
 *
 * On ESP32 the "physical" link is the 802.11 radio (or a wired Ethernet PHY).
 * WiFi station bring-up is a thin wrapper over the Arduino WiFi library. Failover
 * between interfaces is owned by the stack itself (lwIP/esp_netif reselect the
 * default route when a link drops) - this layer adds no manager and no polling
 * tick; it only *reports* which interface currently carries outbound traffic via
 * det_net_egress(), read on demand from the live default route so the answer is
 * always current.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H
#define DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H

#include "DetWebServerConfig.h" // DetIface
#include "shared_primitives/shim.h"

/**
 * @brief Connect to a WiFi access point.
 *
 * Calls `WiFi.begin()` and returns immediately; it does not block waiting for
 * association. Poll wifi_ready() to check link status.
 *
 * @param ssid     Network SSID (null-terminated).
 * @param password WPA2 passphrase (null-terminated).
 * @return Always returns true (WiFi.begin() is fire-and-forget).
 */
bool init_wifi_physical(const char *ssid, const char *password);

/** @brief True if the WiFi station link is up (associated + an IP is assigned). */
bool wifi_ready();

/**
 * @brief Which interface currently carries outbound traffic.
 *
 * Reads the live lwIP default route, so it reflects the current state after any
 * failover the stack performed - no polling, no cached state. Returns
 * DETIFACE_ETH / DETIFACE_STA / DETIFACE_AP, or DETIFACE_ANY when no route is up
 * (and on host builds).
 */
DetIface det_net_egress(void);

/** @brief IPv4 (network byte order) of the current egress interface, or 0 if none. */
uint32_t det_net_egress_ip(void);

/**
 * @brief Classify an egress IPv4 against the WiFi station / softAP IPs (pure helper,
 *        exposed for unit testing).
 *
 * A live egress IP equal to the station or softAP IP is that WiFi interface; any
 * other live IP is a wired (Ethernet) route; 0 is no route.
 *
 * @param egress_ip Current default-route IPv4 (network order), 0 if none.
 * @param sta_ip    WiFi station IPv4 (network order), 0 if not connected.
 * @param ap_ip     softAP IPv4 (network order), 0 if the softAP is not up.
 */
DetIface det_net_classify_ip(uint32_t egress_ip, uint32_t sta_ip, uint32_t ap_ip);

#endif
