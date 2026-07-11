// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 33.Csrf.ino
 * @brief CSRF protection for state-changing requests (DETWS_ENABLE_CSRF).
 *
 * When enabled, every POST / PUT / PATCH / DELETE must carry a valid
 * X-CSRF-Token header (a stateless, HMAC-signed token); requests without one get
 * 403. GET / HEAD / OPTIONS are exempt. The built-in GET /csrf endpoint issues a
 * token (also set as the `csrf` cookie). No server-side session storage - the
 * token self-validates against an HMAC secret seeded at begin().
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_CSRF=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Try:
 *   curl -s http://<ip>/csrf                  returns {"token":"..."}
 *   curl -X POST http://<ip>/submit -H "X-CSRF-Token: <token>"   -> 200
 *   curl -X POST http://<ip>/submit                              -> 403 (missing token)
 */

#define DETWS_ENABLE_CSRF 1

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

    // Safe method: never requires a token. GET /csrf (built-in) hands one out.
    server.on("/", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
        server.send(id, 200, "text/plain", "GET /csrf for a token, then POST /submit");
    });

    // State-changing route: the library rejects it with 403 unless the request
    // carries a valid X-CSRF-Token (no per-route code needed - it is global).
    server.on("/submit", HttpMethod::HTTP_POST,
              [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "accepted"); });

    server.begin(80);
}

void loop()
{
    server.handle();
}
