// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 29.WebDav.ino
 * @brief WebDAV file share (RFC 4918) backed by LittleFS.
 *
 * Mounts the LittleFS subtree "/dav" as a WebDAV share at the URL "/dav". A
 * client can browse and edit the files over standard WebDAV methods:
 *     curl -X PROPFIND  -H "Depth: 1" http://<ip>/dav/
 *     curl -T file.txt  http://<ip>/dav/file.txt          # PUT
 *     curl -X MKCOL     http://<ip>/dav/sub               # make a collection
 *     curl -X MOVE -H "Destination: /dav/b.txt" http://<ip>/dav/file.txt
 *     curl -X DELETE    http://<ip>/dav/b.txt
 *     rclone lsd :webdav: --webdav-url http://<ip>/dav --webdav-vendor other
 *
 * Supported: OPTIONS, PROPFIND (Depth 0/1), GET, HEAD, PUT, DELETE, MKCOL, COPY
 * (files), MOVE, and advisory LOCK/UNLOCK. PUT buffers the body (bounded by
 * BODY_BUF_SIZE); locks are advisory. Add per-route auth + HTTPS (and the per-IP
 * throttle) before exposing a writable share.
 *
 * NOTE: optional services are gated by a compile flag the *library* sources must
 * also see; for PlatformIO enable it for the whole build, e.g.:
 *     build_flags = -DDETWS_ENABLE_WEBDAV=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_WEBDAV 1

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

    if (!LittleFS.begin(true)) // format on first run
    {
        Serial.println("LittleFS mount failed");
        return;
    }
    LittleFS.mkdir("/dav");
    // Seed one file so a fresh share is not empty.
    File f = LittleFS.open("/dav/hello.txt", "w");
    if (f)
    {
        f.print("hello from DeterministicESPAsyncWebServer\n");
        f.close();
    }

    server.dav("/dav", LittleFS, "/dav");
    server.begin(80);
    Serial.println("WebDAV share at http://<ip>/dav");
}

void loop()
{
    server.handle();
}
