// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 20.Diagnostics.ino
 * @brief Compile-time configuration endpoint (DETWS_ENABLE_DIAG).
 *
 * server.diag(slot_id) serves DETWS_DIAG_JSON - a compile-time snapshot of the
 * enabled features and buffer sizes. Handy while developing; it exposes the
 * build configuration, so keep it OFF (or behind auth) in production.
 *
 * NOTE: this feature is compiled into the library only when DETWS_ENABLE_DIAG
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_DIAG=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Flash, open Serial @ 115200 for the IP, then GET http://<ip>/diag.
 */

#define DETWS_ENABLE_DIAG 1

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

    server.on("/diag", HTTP_GET, [](uint8_t id, HttpReq *) { server.diag(id); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
