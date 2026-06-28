// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 05.PerIpThrottle.ino
 * @brief Per-source-IP connection-flood defense (DETWS_ENABLE_PER_IP_THROTTLE).
 *
 * When enabled, the accept callback rejects a new connection once one source IPv4
 * address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within
 * DETWS_PER_IP_THROTTLE_WINDOW_MS. A fixed BSS table of DETWS_PER_IP_THROTTLE_SLOTS
 * buckets tracks the busiest recent addresses, so one noisy client is throttled
 * without affecting others - the gap the global accept throttle cannot close (it
 * cannot tell one noisy client from many). There is no runtime API: it is a
 * build-time defense, and this sketch just shows enabling it. Pairs well with the
 * global accept throttle (DETWS_ENABLE_ACCEPT_THROTTLE) - see example 18.
 *
 * NOTE: this feature is compiled into the library only when the flag is set for
 * the whole build (a .ino #define does not reach the separately compiled
 * library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_PER_IP_THROTTLE=1
 *                   -DDETWS_PER_IP_THROTTLE_MAX=10
 *                   -DDETWS_PER_IP_THROTTLE_WINDOW_MS=10000
 *                   -DDETWS_PER_IP_THROTTLE_SLOTS=16
 * (Arduino IDE: set them in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_PER_IP_THROTTLE 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "per-IP throttled"); });
    server.begin(80); // per-IP throttle is active automatically when the flag is built in
}

void loop()
{
    server.handle();
}
