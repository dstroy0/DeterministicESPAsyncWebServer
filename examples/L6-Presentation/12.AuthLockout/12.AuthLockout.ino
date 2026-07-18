// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 12.AuthLockout.ino
 * @brief Brute-force lockout for HTTP auth (DWS_ENABLE_AUTH_LOCKOUT).
 *
 * Adds a per-source-IP guard in front of authenticated routes: after a few wrong
 * passwords from one address, that address is locked out with an exponential
 * backoff and gets 429 Too Many Requests + Retry-After (no credential check)
 * instead of unlimited guesses. A correct login clears the address immediately.
 * State lives in a fixed BSS table (no heap).
 *
 * NOTE: enable the lockout for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDWS_ENABLE_AUTH_LOCKOUT=1
 * (DWS_ENABLE_AUTH is on by default. Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so
 * it builds as-is.)
 *
 * Try: repeat `curl -u admin:wrong http://<ip>/secret` until you get 429, then
 * `curl -u admin:s3cret http://<ip>/secret` once the Retry-After elapses.
 */

#define DWS_ENABLE_AUTH_LOCKOUT 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

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

    server.on("/", HttpMethod::HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "public page"); });

    // Protected route. Repeated wrong passwords from one IP trip the lockout
    // (429) with exponential backoff; the tuning lives in ServerConfig.h
    // (DWS_AUTH_LOCKOUT_THRESHOLD / _BASE_MS / _MAX_MS).
    server.on(
        "/secret", HttpMethod::HTTP_GET,
        [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "authenticated!"); }, "Restricted", "admin",
        "s3cret");

    server.begin(80);
}

void loop()
{
    server.handle();
}
