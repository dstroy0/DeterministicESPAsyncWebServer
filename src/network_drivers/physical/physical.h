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
 * dws_net_egress(), read on demand from the live default route so the answer is
 * always current.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H
#define DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H

#include "ServerConfig.h" // DWSIface
#include "network_drivers/network/ip.h"
#include <stddef.h>
#include <stdint.h>

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
 * @brief Bring the WiFi radio up in station mode WITHOUT associating to an AP.
 *
 * For raw-radio use that needs the PHY running but no IP link: ESP-NOW peer messaging and
 * promiscuous capture. Pins the radio to @p channel (1..14) when non-zero; pass 0 to leave the
 * channel to a capture layer that sets its own (services/promisc). Returns immediately.
 *
 * @return true once the radio is started (always true on host builds).
 */
bool init_wifi_radio_physical(uint8_t channel);

/**
 * @brief Bring up a softAP, enabling AP+STA coexistence so a station link can run alongside it.
 *
 * @param ssid     softAP SSID (null-terminated).
 * @param password softAP passphrase (null-terminated; >= 8 chars for WPA2, "" for an open AP).
 * @return true if the softAP started (false on host builds).
 */
bool init_wifi_ap_physical(const char *ssid, const char *password);

/**
 * @brief Bring up a wired Ethernet link (DWS_ENABLE_ETHERNET).
 *
 * A thin wrapper over the Arduino ETH library (`ETH.begin()`); the RMII PHY pins / type /
 * clock come from the standard `ETH_PHY_*` build flags for your board. Returns immediately
 * (bring-up is asynchronous); poll eth_ready(). The egress reporting already classifies a
 * wired route as DWSIface::DETIFACE_ETH, so the server accepts on the link once it has an IP.
 *
 * @return true if ETH.begin() started the driver; false if Ethernet is disabled at build
 *         time or the driver failed to start (and always false on host builds).
 */
bool init_eth_physical(void);

/** @brief True if the Ethernet link is up and an IP is assigned. */
bool eth_ready(void);

/**
 * @brief Enable IPv6 (dual-stack) on the Wi-Fi interface (DWS_ENABLE_IPV6).
 *
 * Turns on IPv6 for the netif so it acquires a SLAAC link-local address and, if the network
 * advertises a prefix, a global address. Returns immediately (address configuration is
 * asynchronous); poll dws_ipv6_ready(). The listeners already bind IPADDR_TYPE_ANY, so the server
 * answers over IPv6 as soon as an address is up.
 *
 * @return true if IPv6 was enabled; false if disabled at build time or on host builds.
 */
bool init_ipv6_physical(void);

/**
 * @brief The interface's global (routable) IPv6 address, if it has one.
 * @param[out] out receives the address (family DWSIpFamily::DWS_IP_V6) when true is returned.
 * @return true if a valid global IPv6 address is assigned; false otherwise (incl. host builds).
 */
bool net_global_ipv6(DWSIp *out);

/** @brief True once the interface has a global IPv6 address (see net_global_ipv6()). */
bool dws_ipv6_ready(void);

/**
 * @brief Which interface currently carries outbound traffic.
 *
 * Reads the live lwIP default route, so it reflects the current state after any
 * failover the stack performed - no polling, no cached state. Returns
 * DWSIface::DETIFACE_ETH / DWSIface::DETIFACE_STA / DWSIface::DETIFACE_AP, or DWSIface::DETIFACE_ANY when no route is
 * up (and on host builds).
 */
DWSIface dws_net_egress(void);

/** @brief IPv4 (network byte order) of the current egress interface, or 0 if none. */
uint32_t dws_net_egress_ip(void);

/** @brief softAP IPv4 (network byte order), or 0 if the softAP is not up (and on host builds). */
uint32_t dws_net_ap_ip(void);

/** @brief Station link RSSI in dBm, or 0 if not associated (and on host builds). */
int8_t dws_net_rssi(void);

/**
 * @brief Copy the station interface MAC (6 bytes) into @p out.
 * @return true on success; false if @p out is null or on a host build (out is left untouched).
 */
bool dws_net_mac(uint8_t out[6]);

/**
 * @brief Copy the associated SSID (null-terminated) into @p out.
 * @return the SSID length in bytes, or 0 if not associated, @p cap is 0, or on a host build.
 */
size_t dws_net_ssid(char *out, size_t cap);

/** @brief Station WiFi channel (1..14), or 0 if not associated (and on host builds). */
uint8_t dws_net_channel(void);

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
DWSIface dws_net_classify_ip(uint32_t egress_ip, uint32_t sta_ip, uint32_t ap_ip);

#endif
