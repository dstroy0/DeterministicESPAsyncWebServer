// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 02.AcceptThrottle.ino
 * @brief Connection-flood defense via the accept-rate throttle (DETWS_ENABLE_ACCEPT_THROTTLE).
 *
 * When enabled, the accept callback rejects new connections once more than
 * DETWS_ACCEPT_THROTTLE_MAX have been accepted within
 * DETWS_ACCEPT_THROTTLE_WINDOW_MS (a global fixed window, two counters, no per-IP
 * table). It bounds connection churn (e.g. reconnect brute force) on top of the
 * bounded connection pool. There is no runtime API - it is purely a build-time
 * defense; this sketch just shows enabling it.
 *
 * NOTE: this feature is compiled into the library only when the flag is set for
 * the whole build (a .ino #define does not reach the separately compiled
 * library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_ACCEPT_THROTTLE=1
 *                   -DDETWS_ACCEPT_THROTTLE_MAX=20
 *                   -DDETWS_ACCEPT_THROTTLE_WINDOW_MS=1000
 * (Arduino IDE: set them in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_ACCEPT_THROTTLE 1

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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "throttled server"); });
    server.begin(80); // accept throttle is active automatically when the flag is built in
}

void loop()
{
    server.handle();
}
