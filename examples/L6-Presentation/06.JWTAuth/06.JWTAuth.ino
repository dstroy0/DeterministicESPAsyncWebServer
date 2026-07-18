// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 06.JWTAuth.ino
 * @brief Stateless route protection with JWT bearer tokens (HS256).
 *
 * A client presents `Authorization: Bearer <jwt>`; the device verifies the
 * token's HMAC-SHA-256 signature against a shared secret (no sessions, no
 * per-client state, no heap). Reads the `sub` claim... well, here it reads `exp`
 * to show claim access. Only HS256 is supported (the deterministic, shared-secret
 * choice for a constrained device).
 *
 * Flash, open Serial @ 115200 for the IP. With the demo secret "s3cr3t-key",
 * a token for {"sub":"alice","role":"admin","exp":2000000000} is:
 *   T=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJhbGljZSIsInJvbGUiOiJhZG1pbiIsImV4cCI6MjAwMDAwMDAwMCwiaWF0IjoxNzAwMDAwMDAwfQ.oaEaMu7USfUlYDaLYQlogmRd_1ZPBr7cKrPIo5lXdxc
 *   curl -H "Authorization: Bearer $T" http://<ip>/protected   # 200
 *   curl http://<ip>/protected                                 # 401
 * Mint your own with any JWT library/jwt.io using HS256 + the secret below.
 *
 * NOTE: optional services are gated by a compile flag the *library* sources must
 * also see; for PlatformIO enable it for the whole build, e.g.:
 *     build_flags = -DDWS_ENABLE_JWT=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DWS_ENABLE_JWT 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/jwt/jwt.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// DEMO shared secret - the issuer signs tokens with this; keep it secret in production.
static const char *JWT_SECRET = "s3cr3t-key";

DWS server;

static void protected_handler(uint8_t id, HttpReq *req)
{
    // req->authorization holds the FULL Authorization header (JWTs exceed
    // MAX_VAL_LEN; the parser captures it whole when DWS_ENABLE_JWT is set).
    if (!jwt_bearer_valid(req->authorization, (const uint8_t *)JWT_SECRET, strlen(JWT_SECRET)))
    {
        server.add_response_header(id, "WWW-Authenticate", "Bearer");
        server.send(id, 401, "text/plain", "invalid or missing token");
        return;
    }

    // Granular authorization from a token claim. jwt_claim_str / jwt_scope_allows
    // take the bare token, so step past the "Bearer " scheme first.
    const char *tok = req->authorization + 7;
    while (*tok == ' ')
        tok++;
    char role[16];
    if (!jwt_claim_str(tok, strlen(tok), "role", role, sizeof(role)) || strcmp(role, "admin") != 0)
    {
        server.send(id, 403, "text/plain", "forbidden: admin role required");
        return;
    }
    // For OAuth2 space-separated scopes, gate on the "scope" claim instead:
    //   char scope[64];
    //   if (jwt_claim_str(tok, strlen(tok), "scope", scope, sizeof(scope)) &&
    //       jwt_scope_allows(scope, "telemetry:write")) { ... }

    server.send(id, 200, "text/plain", "welcome admin - your token is valid");
}

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

    server.on("/protected", HttpMethod::HTTP_GET, protected_handler);
    server.on("/", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "public"); });

    int32_t result = server.begin(80);
    if (result < 0)
        Serial.printf("begin() failed (error %d)\n", result);
    else
        Serial.println("JWT-protected server on :80 (GET /protected with a Bearer token)");
}

void loop()
{
    server.handle();
}
