# 02.CORS - cross-origin headers and preflight

**Layer:** L7 Application · **Build flags:** none (core features only)

## What this example teaches

If a web app served from another origin needs to call this device's JSON API from
the browser, the device must send CORS headers. `set_cors(origin)` builds the
`Access-Control-Allow-*` block once and injects it into every response - and
answers the CORS preflight `OPTIONS` request automatically.

**One call enables it.** Configure the policy at setup; every handler's response
then carries the headers, and preflights are handled for you:

```cpp
server.set_cors("*"); // allow any origin (tighten to your web app's origin in production)
server.on("/api", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "application/json", "{\"ok\":true}"); });
```

The header block is built once into `CORS_HDR_BUF_SIZE` and reused, so there is no
per-request cost. Use a specific origin in production rather than `*`.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --lib="." examples/L7-Application/02.CORS/02.CORS.ino
```

From a page on another origin: `fetch('http://<ip>/api').then(r=>r.json()).then(console.log)`.

## Annotated source

The complete sketch ([02.CORS.ino](02.CORS.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

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

    // Builds the Access-Control-Allow-* block once and answers preflight OPTIONS.
    server.set_cors("*"); // allow any origin (tighten to your web app's origin in production)
    server.on("/api", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send(id, 200, "application/json", "{\"ok\":true}"); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
