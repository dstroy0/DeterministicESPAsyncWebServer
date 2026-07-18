# 05.PerIpThrottle - per-source-IP flood defense

**Layer:** L4 Transport · **Build flags:** `DWS_ENABLE_PER_IP_THROTTLE`

## What this example teaches

The global [accept throttle](../02.AcceptThrottle) caps total accepts but cannot
tell one noisy client from many legitimate ones. This per-IP throttle closes that
gap: the accept callback rejects a new connection once a single source IPv4 has
opened more than `DWS_PER_IP_THROTTLE_MAX` connections within
`DWS_PER_IP_THROTTLE_WINDOW_MS`, so one abusive host is throttled without
affecting everyone else.

**Bounded memory, no heap.** A fixed BSS table of `DWS_PER_IP_THROTTLE_SLOTS`
buckets tracks the busiest recent addresses (an LRU-ish set, not one slot per
possible IP), so the defense itself stays deterministic.

**Build-time only.** Like the global throttle, there is no runtime API - the
handler is plain; enabling the flag activates the defense in the accept path:

```cpp
server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "per-IP throttled"); });
server.begin(80); // per-IP throttle is active automatically when the flag is built in
```

**Tuning + pairing.** Set the knobs as build flags (cap, window, table size), and
pair it with the global accept throttle for layered defense:

```text
build_flags = -DDWS_ENABLE_PER_IP_THROTTLE=1 -DDWS_PER_IP_THROTTLE_MAX=10 \
              -DDWS_PER_IP_THROTTLE_WINDOW_MS=10000 -DDWS_PER_IP_THROTTLE_SLOTS=16
```

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_PER_IP_THROTTLE=1" \
  --lib="." examples/L4-Transport/05.PerIpThrottle/05.PerIpThrottle.ino
```

From one host, open many rapid connections and watch that host get refused while
another host still connects.

## Annotated source

The complete sketch ([05.PerIpThrottle.ino](05.PerIpThrottle.ino)), reproduced
verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DWS_ENABLE_PER_IP_THROTTLE 1

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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "per-IP throttled"); });
    server.begin(80); // per-IP throttle is active automatically when the flag is built in
}

void loop()
{
    server.handle();
}
```
