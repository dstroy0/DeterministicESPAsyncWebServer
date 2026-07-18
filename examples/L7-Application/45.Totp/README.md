# 45.Totp - TOTP two-factor authentication (RFC 6238)

**Layer:** L7 Application · **Build flags:** `DWS_ENABLE_TOTP`

## What this example teaches

TOTP is the six-digit code an authenticator app (Google Authenticator, Authy)
shows. This decodes a base32 shared secret, computes the current code, and verifies
a submitted one within a +/-1 step window - so you can add a second factor to a
protected route. `GET /totp` shows the current code (demo only) and
`GET /totp/verify?code=NNNNNN` checks one.

**Decode the secret once, then compute / verify against the clock:**

```cpp
int n = dws_base32_decode(SECRET_B32, g_secret, sizeof(g_secret)); // base32 -> bytes
g_secret_len = (n > 0) ? (size_t)n : 0;
```

```cpp
uint32_t code = dws_totp(g_secret, g_secret_len, now_unix(), 30, 6);          // 30s step, 6 digits
bool ok = dws_totp_verify(g_secret, g_secret_len, now_unix(), code, 30, 6, 1); // +/-1 step tolerance
```

The last argument to `dws_totp_verify` is the allowed step skew, which absorbs
clock drift between the device and the authenticator. For codes that match a real
app, sync the device clock to real time via NTP ([18.SNTP](../18.SNTP)); this
example uses a fixed clock base so it is self-contained.

**Security note.** The `/totp` endpoint that reveals the live code exists only to
make the demo testable - never expose the current code in production.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_TOTP=1" \
  --lib="." examples/L7-Application/45.Totp/45.Totp.ino
```

```sh
code=$(curl -s http://<ip>/totp)
curl "http://<ip>/totp/verify?code=$code"   # {"ok":true}
```

## Annotated source

The complete sketch ([45.Totp.ino](45.Totp.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DWS_ENABLE_TOTP 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/totp/totp.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// The shared secret, base32 (share this with the authenticator app at enrolment).
static const char *SECRET_B32 = "JBSWY3DPEHPK3PXP";
static uint8_t g_secret[32];
static size_t g_secret_len = 0;

DWS server;

static uint64_t now_unix()
{
    // Self-contained demo clock; replace with real (NTP) time for app-matching codes.
    return 1700000000ull + millis() / 1000;
}

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    int n = dws_base32_decode(SECRET_B32, g_secret, sizeof(g_secret));
    g_secret_len = (n > 0) ? (size_t)n : 0;

    server.on("/totp", HTTP_GET, [](uint8_t id, HttpReq *) {
        uint32_t code = dws_totp(g_secret, g_secret_len, now_unix(), 30, 6);
        char b[16];
        snprintf(b, sizeof(b), "%06u", code); // zero-pad to 6 digits
        server.send(id, 200, "text/plain", b);
    });
    server.on("/totp/verify", HTTP_GET, [](uint8_t id, HttpReq *req) {
        const char *code_s = http_get_query(req, "code");
        uint32_t code = code_s ? (uint32_t)strtoul(code_s, nullptr, 10) : 0;
        bool ok = dws_totp_verify(g_secret, g_secret_len, now_unix(), code, 30, 6, 1);
        server.send(id, 200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
