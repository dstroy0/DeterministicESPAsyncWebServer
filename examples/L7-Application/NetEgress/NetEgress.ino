// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file NetEgress.ino
 * @brief Report which interface outbound traffic is using (dws_net_egress()).
 *
 * The OS (esp_netif) handles failover between links by reselecting the default
 * route; this sketch just *reports* the live egress interface and its IP, queried
 * on demand - no manager, no polling loop. Useful for logging, telemetry tags, or
 * showing "online via Ethernet / WiFi" in a UI. (Wire an Ethernet PHY alongside
 * WiFi to watch it flip on a cable pull.)
 *
 * Flash, open Serial @ 115200 for the IP, then GET http://<ip>/net.
 */

#include "dwserver.h"
#include "network_drivers/physical/physical.h"

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

static const char *iface_name(DWSIface i)
{
    switch (i)
    {
    case DWSIface::DETIFACE_ETH:
        return "ethernet";
    case DWSIface::DETIFACE_AP:
        return "softap";
    case DWSIface::DETIFACE_STA:
        return "wifi-sta";
    default:
        return "none";
    }
}

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    uint32_t ip = dws_net_egress_ip(); // library egress IP (network byte order), no Arduino WiFi
    Serial.printf("\nIP: %u.%u.%u.%u\n", (unsigned)(ip & 0xFF), (unsigned)((ip >> 8) & 0xFF),
                  (unsigned)((ip >> 16) & 0xFF), (unsigned)((ip >> 24) & 0xFF));

    Serial.printf("egress interface: %s\n", iface_name(dws_net_egress()));

    server.on("/net", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
        uint32_t ip = dws_net_egress_ip(); // network byte order
        char body[96];
        snprintf(body, sizeof(body), "{\"egress\":\"%s\",\"ip\":\"%u.%u.%u.%u\"}", iface_name(dws_net_egress()),
                 (unsigned)(ip & 0xFF), (unsigned)((ip >> 8) & 0xFF), (unsigned)((ip >> 16) & 0xFF),
                 (unsigned)((ip >> 24) & 0xFF));
        server.send(id, 200, "application/json", body);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
