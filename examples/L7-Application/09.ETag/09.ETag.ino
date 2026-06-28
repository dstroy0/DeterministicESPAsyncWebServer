// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 09.ETag.ino
 * @brief Conditional GET with ETag for served files (DETWS_ENABLE_ETAG).
 *
 * With ETag enabled, serve_file()/serve_static() emit a strong ETag (derived
 * from the file size + mtime) and answer a matching If-None-Match with
 * 304 Not Modified - saving bandwidth on repeat fetches of static assets.
 *
 * NOTE: this feature is compiled into the library only when DETWS_ENABLE_ETAG
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_ETAG=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Put a file at `data/www/index.html`, upload the LittleFS image, then:
 *   curl -i http://<ip>/            # 200 + ETag: "..."
 *   curl -i -H 'If-None-Match: "<etag>"' http://<ip>/   # 304 Not Modified
 */

#define DETWS_ENABLE_ETAG 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <LittleFS.h>
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

void setup()
{
    Serial.begin(115200);
    if (!LittleFS.begin(true))
        Serial.println("LittleFS mount failed (upload a filesystem image)");

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

    server.serve_static("/", LittleFS, "/www"); // ETag + If-None-Match handled automatically
    server.begin(80);
}

void loop()
{
    server.handle();
}
