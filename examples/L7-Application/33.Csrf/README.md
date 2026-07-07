# 33.Csrf - CSRF protection for state-changing requests

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_CSRF`

## What this example teaches

Cross-Site Request Forgery tricks a logged-in browser into making a state-changing
request the user did not intend. With `DETWS_ENABLE_CSRF`, every
`POST`/`PUT`/`PATCH`/`DELETE` must carry a valid `X-CSRF-Token` header or it gets
`403`; the safe methods `GET`/`HEAD`/`OPTIONS` are exempt. The token is stateless -
HMAC-signed and self-validating against a secret seeded at `begin()` - so there is
**no server-side session storage**.

**Protection is global, not per-route.** You write ordinary handlers; the library
enforces the token check on unsafe methods automatically:

```cpp
// Safe method: never requires a token. The built-in GET /csrf hands one out.
server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) {
    server.send(id, 200, "text/plain", "GET /csrf for a token, then POST /submit");
});

// State-changing route: rejected with 403 unless a valid X-CSRF-Token is present.
server.on("/submit", HTTP_POST, [](uint8_t id, HttpReq *) {
    server.send(id, 200, "text/plain", "accepted");
});
```

The built-in `GET /csrf` endpoint issues a token (returned as JSON and set as the
`csrf` cookie). A client fetches a token, then echoes it in the `X-CSRF-Token`
header on each unsafe request.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_CSRF=1" \
  --lib="." examples/L7-Application/33.Csrf/33.Csrf.ino
```

```sh
curl -s http://<ip>/csrf                                    # {"token":"..."}
curl -X POST http://<ip>/submit -H "X-CSRF-Token: <token>" # 200 accepted
curl -X POST http://<ip>/submit                            # 403 (missing token)
```

## Annotated source

The complete sketch ([33.Csrf.ino](33.Csrf.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

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
    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) {
        server.send(id, 200, "text/plain", "GET /csrf for a token, then POST /submit");
    });

    // State-changing route: the library rejects it with 403 unless the request
    // carries a valid X-CSRF-Token (no per-route code needed - it is global).
    server.on("/submit", HTTP_POST, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "accepted"); });

    server.begin(80);
}

void loop()
{
    server.handle();
}
```
