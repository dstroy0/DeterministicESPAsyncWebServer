// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 10.FileServing.ino
 * @brief Serve a static site from LittleFS with serve_static().
 *
 * serve_static(url_prefix, fs, fs_root) mounts a filesystem subtree at a URL
 * prefix: a request for "/" maps to "/www/index.html", "/app.js" to
 * "/www/app.js", and so on (content types inferred from the extension). File
 * serving is on by default (DETWS_ENABLE_FILE_SERVING).
 *
 * Put your assets under a `data/www/` folder and upload the LittleFS image
 * ("Upload Filesystem Image" in PlatformIO / Arduino) before running.
 *
 * Flash, open Serial @ 115200 for the IP, then browse to http://<ip>/.
 */

#include "dwserver.h"
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

    // Map the URL tree "/" onto the "/www" directory in LittleFS.
    server.serve_static("/", LittleFS, "/www");
    // Cache assets for an hour; browsers still revalidate cheaply via the ETag.
    server.set_cache_control("max-age=3600");
    server.begin(80);
}

void loop()
{
    server.handle();
}
