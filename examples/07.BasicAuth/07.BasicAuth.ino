// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 07.BasicAuth.ino
 * @brief Per-route HTTP Basic authentication (RFC 7617).
 *
 * The auth-aware on() overload protects a route with a realm + username +
 * password; the handler runs only after the credentials check passes, otherwise
 * the server answers 401 with a WWW-Authenticate challenge. Pass digest=true for
 * Digest auth instead (see the DigestAuth example). Auth is on by default
 * (DETWS_ENABLE_AUTH).
 *
 * NOTE: Basic credentials are base64 (not encryption) - use it over HTTPS or an
 * SSH tunnel on untrusted networks.
 *
 * Flash, open Serial @ 115200 for the IP, then visit http://<ip>/secret
 * (user "admin", password "s3cret").
 */

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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "public page"); });

    // Basic auth (digest defaults to false): realm, username, password.
    server.on(
        "/secret", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "authenticated!"); },
        "Restricted", "admin", "s3cret");

    server.begin(80);
}

void loop()
{
    server.handle();
}
