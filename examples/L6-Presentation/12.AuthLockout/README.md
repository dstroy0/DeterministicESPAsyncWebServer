# 12.AuthLockout - brute-force lockout for HTTP auth

**Layer:** L6 Presentation · **Build flags:** `DWS_ENABLE_AUTH_LOCKOUT` (requires `DWS_ENABLE_AUTH`, on by default)

## What this example teaches

This puts a per-source-IP guard in front of authenticated routes. After a few
wrong passwords from one address, that address is locked out with exponential
backoff and gets `429 Too Many Requests` + `Retry-After` (without even checking
credentials) instead of unlimited guesses. A correct login clears the address
immediately. State lives in a fixed BSS table - no heap.

**It is transparent to your route.** You protect the route exactly as in
[04.BasicAuth](../04.BasicAuth); enabling the flag adds the lockout in front of
the credential check automatically:

```cpp
server.on(
    "/secret", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "authenticated!"); },
    "Restricted", "admin", "s3cret");
```

**Tuning.** The thresholds live in `ServerConfig.h`:
`DWS_AUTH_LOCKOUT_THRESHOLD` (failures before locking),
`DWS_AUTH_LOCKOUT_BASE_MS` and `DWS_AUTH_LOCKOUT_MAX_MS` (the backoff window,
which doubles per subsequent failure up to the max), and
`DWS_AUTH_LOCKOUT_SLOTS` (how many addresses are tracked). Set them as build
flags alongside the enable flag.

**Build dependency.** `DWS_ENABLE_AUTH_LOCKOUT` requires `DWS_ENABLE_AUTH`
(which is on by default) - enforced by a compile-time `#error`.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_AUTH_LOCKOUT=1" \
  --lib="." examples/L6-Presentation/12.AuthLockout/12.AuthLockout.ino
```

```sh
# repeat until you get 429, then wait out Retry-After and use the right password:
curl -u admin:wrong  http://<ip>/secret    # ...x several -> 429 + Retry-After
curl -u admin:s3cret http://<ip>/secret    # clears the lockout for your IP
```

## Annotated source

The complete sketch ([12.AuthLockout.ino](12.AuthLockout.ino)), reproduced
verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "public page"); });

    // Protected route. Repeated wrong passwords from one IP trip the lockout
    // (429) with exponential backoff; the tuning lives in ServerConfig.h
    // (DWS_AUTH_LOCKOUT_THRESHOLD / _BASE_MS / _MAX_MS).
    server.on(
        "/secret", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "authenticated!"); },
        "Restricted", "admin", "s3cret");

    server.begin(80);
}

void loop()
{
    server.handle();
}
```
