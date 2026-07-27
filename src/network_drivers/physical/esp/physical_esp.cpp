// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file physical_esp.cpp
 * @brief Layer 1 (Physical) - ESP backend: 802.11 radio + wired Ethernet (RMII / W5500 SPI) bring-up and
 *        live egress readout, straight off the Arduino-ESP32 WiFi/ETH wrappers and lwIP's default netif.
 *
 * The vendor-specific half of the physical layer. The common API is in physical.h and the vendor is chosen
 * by the DWS_VENDOR_* selector (board_profiles/dws_platform.h); this whole TU compiles to nothing on any
 * non-ESP vendor, where physical.cpp's fallback stubs stand in (DWS_PHYSICAL_HAS_BACKEND == 0). WiFi station
 * bring-up is asynchronous (poll wifi_ready()); dws_net_egress() reads the live default-route netif and hands
 * it to the pure dws_net_classify_ip() that lives in physical.cpp. Adding STM/RP/TI = a sibling
 * physical/<vendor>/ TU guarded by that vendor's macro, no change here.
 */

#include "network_drivers/physical/physical.h"

#if DWS_VENDOR_ESP

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include <WiFi.h>
#include <esp_wifi.h> // esp_wifi_set_channel / esp_wifi_sta_get_ap_info (raw-radio bring-up + SSID readout)
#include <string.h>   // strnlen / memcpy
#if DWS_ENABLE_ETHERNET
#include <ETH.h>
#endif
#if DWS_ENABLE_IPV6
#include "lwip/ip6_addr.h"
#endif

bool init_wifi_physical(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    return true;
}

bool wifi_ready()
{
    return WiFi.isConnected();
}

bool init_wifi_radio_physical(uint8_t channel)
{
    // Radio up but not associated: ESP-NOW and promiscuous capture want the PHY without an IP link.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (channel)
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    return true;
}

bool init_wifi_ap_physical(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_AP_STA); // coexist so a station link can run alongside the softAP
    return WiFi.softAP(ssid, password);
}

#if DWS_ENABLE_ETHERNET
bool init_eth_physical(void)
{
#if defined(DWS_ETH_W5500) && DWS_ETH_W5500 && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // W5500 SPI Ethernet (arduino-esp32 3.x ETH SPI API): the HSPI host (SPI3) clocks the W5500 on the
    // DWS_ETH_W5500_* pins (CS/INT/RST + SCK/MISO/MOSI). Needs CONFIG_ETH_SPI_ETHERNET_W5500 in the SDK
    // (default on for the S3). W5500 SPI is arduino-esp32 3.x only - the 2.x ETH library has no W5500.
    return ETH.begin(ETH_PHY_W5500, 1 /*phy addr*/, DWS_ETH_W5500_CS, DWS_ETH_W5500_INT, DWS_ETH_W5500_RST, SPI3_HOST,
                     DWS_ETH_W5500_SCK, DWS_ETH_W5500_MISO, DWS_ETH_W5500_MOSI, DWS_ETH_W5500_SPI_MHZ);
#else
    // RMII PHY: pins / type / clock come from the ETH_PHY_* build flags (ETH.begin() defaults).
    return ETH.begin();
#endif
}
bool eth_ready(void)
{
    return ETH.linkUp() && (uint32_t)ETH.localIP() != 0;
}
#else
bool init_eth_physical(void)
{
    return false; // Ethernet not enabled (DWS_ENABLE_ETHERNET)
}
bool eth_ready(void)
{
    return false;
}
#endif

#if DWS_ENABLE_IPV6

bool init_ipv6_physical(void)
{
    // The WiFi wrapper's enable call was renamed in Arduino-ESP32 3.0; the address readout
    // below goes straight to lwIP, which is stable across both cores.
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    return WiFi.enableIPv6(true);
#else
    return WiFi.enableIpV6();
#endif
}

bool net_global_ipv6(DWSIp *out)
{
    if (!out || !netif_default)
        return false;
    for (int8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        if (!ip6_addr_isvalid(netif_ip6_addr_state(netif_default, i)))
            continue;
        const ip6_addr_t *a6 = netif_ip6_addr(netif_default, i);
        if (!ip6_addr_isglobal(a6))
            continue;
        out->family = DWSIpFamily::DWS_IP_V6;
        memcpy(out->bytes, a6->addr, 16); // lwIP holds the 16 bytes in network order
        return true;
    }
    return false;
}

bool dws_ipv6_ready(void)
{
    DWSIp tmp;
    return net_global_ipv6(&tmp);
}
#else
bool init_ipv6_physical(void)
{
    return false; // IPv6 not enabled (DWS_ENABLE_IPV6)
}
bool net_global_ipv6(DWSIp *)
{
    return false;
}
bool dws_ipv6_ready(void)
{
    return false;
}
#endif

uint32_t dws_net_egress_ip(void)
{
    // netif_default is the current default-route interface (the egress).
    return netif_default ? ip4_addr_get_u32(ip_2_ip4(&netif_default->ip_addr)) : 0;
}

DWSIface dws_net_egress(void)
{
    uint32_t egress = dws_net_egress_ip();
    if (egress == 0)
        return DWSIface::DETIFACE_ANY;
    uint32_t sta = WiFi.isConnected() ? (uint32_t)WiFi.localIP() : 0;
    uint32_t ap = dws_net_ap_ip();
    return dws_net_classify_ip(egress, sta, ap);
}

uint32_t dws_net_ap_ip(void)
{
    return (WiFi.getMode() & WIFI_AP) ? (uint32_t)WiFi.softAPIP() : 0;
}

int8_t dws_net_rssi(void)
{
    return WiFi.isConnected() ? (int8_t)WiFi.RSSI() : 0;
}

bool dws_net_mac(uint8_t out[6])
{
    if (!out)
        return false;
    WiFi.macAddress(out);
    return true;
}

bool dws_net_egress_mac(uint8_t out[6])
{
    // The egress interface's own link-layer address, straight off the live default netif - the Ethernet PHY's
    // MAC on a wired link, the WiFi STA MAC on a wireless one. Independent of which driver started.
    if (!out || !netif_default || netif_default->hwaddr_len < 6)
        return false;
    memcpy(out, netif_default->hwaddr, 6);
    return true;
}

size_t dws_net_ssid(char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    wifi_ap_record_t info; // heap-free SSID readout (WiFi.SSID() would allocate an Arduino String)
    if (esp_wifi_sta_get_ap_info(&info) != ESP_OK)
    {
        out[0] = '\0';
        return 0;
    }
    size_t n = strnlen((const char *)info.ssid, sizeof(info.ssid));
    if (n >= cap)
        n = cap - 1;
    memcpy(out, info.ssid, n);
    out[n] = '\0';
    return n;
}

uint8_t dws_net_channel(void)
{
    return (uint8_t)WiFi.channel();
}

#endif // DWS_VENDOR_ESP
