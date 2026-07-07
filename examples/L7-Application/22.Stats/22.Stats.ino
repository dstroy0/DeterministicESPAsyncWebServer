// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 22.Stats.ino
 * @brief Runtime statistics endpoint (DETWS_ENABLE_STATS).
 *
 * server.stats(slot_id) writes a JSON snapshot - uptime, request/error counts,
 * connection-pool usage, free heap - straight to the response. Wire it to a
 * route to expose live diagnostics.
 *
 * NOTE: this feature is compiled into the library only when DETWS_ENABLE_STATS
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_STATS=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Flash, open Serial @ 115200 for the IP, then GET http://<ip>/stats.
 */

#define DETWS_ENABLE_STATS 1

#include "dwserver.h"
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

    server.on("/stats", HTTP_GET, [](uint8_t id, HttpReq *) { server.stats(id); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
