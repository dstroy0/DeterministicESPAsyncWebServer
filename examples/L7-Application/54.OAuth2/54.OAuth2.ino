// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 54.OAuth2.ino
 * @brief OAuth2 authorization-code exchange (DETWS_ENABLE_OAUTH2).
 *
 * The redirect-callback half of an OAuth/OIDC login: after the user authorizes at
 * the provider, the browser is redirected back with `?code=...`; this handler
 * exchanges that code at the provider's token endpoint for tokens.
 *
 *   GET /callback?code=<auth_code>
 *     -> exchanges the code, returns {"expires_in":3600,...}
 *
 * Pair it with services/oidc to verify the returned id_token, and call
 * detws_oauth2_refresh() later with the refresh_token to get fresh access tokens.
 * Needs the HTTP(S) client (DETWS_ENABLE_HTTP_CLIENT); use https:// token URLs in
 * production and set a CA / pin on the client.
 *
 * NOTE: enable it (and the HTTP client) for the whole build. In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_OAUTH2=1 -DDETWS_ENABLE_HTTP_CLIENT=1
 */

#define DETWS_ENABLE_OAUTH2 1
#define DETWS_ENABLE_HTTP_CLIENT 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/oauth2/oauth2.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// From your OAuth provider:
static const char *TOKEN_URL = "https://provider.example/oauth/token";
static const char *CLIENT_ID = "your-client-id";
static const char *CLIENT_SECRET = "your-client-secret"; // or nullptr + PKCE code_verifier
static const char *REDIRECT_URI = "http://device.local/callback";

DetWebServer server;

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    server.on("/callback", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *req) {
        const char *code = http_get_query(req, "code");
        if (!code)
        {
            server.send(id, 400, "application/json", "{\"error\":\"missing code\"}");
            return;
        }
        DetwsOAuth2Tokens t;
        int st = detws_oauth2_exchange_code(TOKEN_URL, code, REDIRECT_URI, CLIENT_ID, CLIENT_SECRET, nullptr, &t);
        if (st != 200)
        {
            char b[48];
            snprintf(b, sizeof(b), "{\"error\":\"exchange failed\",\"status\":%d}", st);
            server.send(id, 502, "application/json", b);
            return;
        }
        // t.access_token / t.id_token / t.refresh_token are now populated.
        char b[96];
        snprintf(b, sizeof(b), "{\"token_type\":\"%s\",\"expires_in\":%ld}", t.token_type, t.expires_in);
        server.send(id, 200, "application/json", b);
    });

    server.begin(80);
}

void loop()
{
    server.handle();
}
