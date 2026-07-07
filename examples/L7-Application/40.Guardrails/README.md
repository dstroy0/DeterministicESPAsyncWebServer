# 40.Guardrails - runtime heap/stack guardrails

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_GUARDRAILS`

## What this example teaches

A long-running device dies quietly when it runs out of heap or stack. Guardrails
watch four resources - free heap, heap low-water, largest free block
(fragmentation), and this task's remaining stack - and fire a callback when any
crosses its `DETWS_GUARDRAIL_*` floor, so the app can shed load, drop to a safe
state, or reboot _before_ exhaustion bites. The live snapshot is also served as
JSON at `/health`.

**Install a breach callback, then check periodically:**

```cpp
detws_guardrails_begin(on_breach);

void loop() {
    if (/* once a second */)
        detws_guardrails_check(); // fires on_breach() if any floor is crossed
}
```

The callback receives a bitmask of which floors were breached and the full health
snapshot:

```cpp
static void on_breach(uint8_t breaches, const DetwsHealth *h) {
    Serial.printf("[guardrail] breach=0x%02x heap=%u frag=%u stack=%u\n",
                  breaches, (unsigned)h->free_heap, (unsigned)h->largest_free_block, (unsigned)h->stack_free);
    // Real app: shed load, drop to a safe state, or ESP.restart().
}
```

`detws_guardrails_sample(&h)` fills a snapshot on demand and `detws_health_json()`
serializes it for the `/health` endpoint.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_GUARDRAILS=1" \
  --lib="." examples/L7-Application/40.Guardrails/40.Guardrails.ino
```

```sh
curl http://<ip>/health   # JSON: free heap, largest block, stack free
```

## Annotated source

The complete sketch ([40.Guardrails.ino](40.Guardrails.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_GUARDRAILS 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/guardrails/guardrails.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Fired when any guardrail floor is crossed; breaches is a bitmask.
static void on_breach(uint8_t breaches, const DetwsHealth *h)
{
    Serial.printf("[guardrail] breach=0x%02x heap=%u frag=%u stack=%u\n", breaches, (unsigned)h->free_heap,
                  (unsigned)h->largest_free_block, (unsigned)h->stack_free);
    // Real app: shed load, drop to a safe state, or ESP.restart().
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

    detws_guardrails_begin(on_breach);

    server.on("/health", HTTP_GET, [](uint8_t id, HttpReq *) {
        DetwsHealth h;
        detws_guardrails_sample(&h);
        char buf[128];
        detws_health_json(&h, buf, sizeof(buf));
        server.send(id, 200, "application/json", buf);
    });
    server.begin(80);
}

void loop()
{
    static uint32_t last = 0;
    if (millis() - last >= 1000)
    {
        last = millis();
        detws_guardrails_check(); // fires on_breach() if any floor is crossed
    }
}
```
