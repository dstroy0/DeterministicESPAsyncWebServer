// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 07.IpAllowlist.ino
 * @brief Restrict who may connect with a source-IP allowlist (DETWS_ENABLE_IP_ALLOWLIST).
 *
 * The accept callback drops any TCP connection whose source address falls
 * outside the configured CIDR rules - a coarse first-line firewall in front of
 * every listener (HTTP, WS, etc.). An empty allowlist allows everything, so add
 * at least one rule. Rules live in a fixed BSS table (no heap).
 *
 * NOTE: enable the feature for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_IP_ALLOWLIST=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Here only the 192.168.1.0/24 LAN and the single host 10.0.0.5 may connect; a
 * spoofed source can still pass, so pair it with the accept throttles.
 */

#define DETWS_ENABLE_IP_ALLOWLIST 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/transport/listener.h" // listener_ip_allow_add / DETWS_IPV4
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

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
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    // Only these sources may connect; everything else is dropped at accept time.
    listener_ip_allow_add(DETWS_IPV4(192, 168, 1, 0), 24); // local /24
    listener_ip_allow_add(DETWS_IPV4(10, 0, 0, 5), 32);    // one trusted host

    server.on("/", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "hello from an allowed address"); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
