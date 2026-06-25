// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 37.mDNS.ino
 * @brief Advertise the device over mDNS / DNS-SD (DETWS_ENABLE_MDNS).
 *
 * detws_mdns_begin(hostname, port) makes the device reachable at
 * `<hostname>.local` and advertises an `_http._tcp` service, so clients on the
 * LAN can find it without knowing its IP.
 *
 * NOTE: this service is compiled into the library only when DETWS_ENABLE_MDNS
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_MDNS=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Flash, then browse to http://detws-demo.local/.
 */

#define DETWS_ENABLE_MDNS 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/mdns_service.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";
static const char *HOSTNAME = "detws-demo";

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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "hello via mDNS"); });
    server.begin(80);

    if (detws_mdns_begin(HOSTNAME, 80))
        Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);
}

void loop()
{
    server.handle();
}
