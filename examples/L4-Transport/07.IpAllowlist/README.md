# 07.IpAllowlist - a source-IP accept-time firewall

**Layer:** L4 Transport · **Build flags:** `DETWS_ENABLE_IP_ALLOWLIST`

## What this example teaches

This drops any TCP connection whose source address falls outside a set of CIDR
rules - a coarse first-line firewall in front of _every_ listener (HTTP, WS,
TLS, etc.), evaluated at accept time before any bytes are read. Rules live in a
fixed BSS table, so there is no heap cost.

**Adding rules.** Unlike the throttles (which are flag-only), the allowlist has a
small API: add CIDR rules with `listener_ip_allow_add(network, prefix_len)`,
using the `DETWS_IPV4(a,b,c,d)` helper to build the address:

```cpp
listener_ip_allow_add(DETWS_IPV4(192, 168, 1, 0), 24); // the local /24
listener_ip_allow_add(DETWS_IPV4(10, 0, 0, 5), 32);    // one trusted host
```

**Fail-open until you add a rule.** An _empty_ allowlist allows everything (so
enabling the feature before adding rules never locks you out). Add at least one
rule to actually restrict access.

**Know its limits.** This filters by source IP, which a determined attacker can
spoof, so treat it as a coarse first layer and pair it with the
[accept throttles](../02.AcceptThrottle) and real authentication. It is excellent
for "only my LAN may even open a socket."

The `listener.h` include is what brings in `listener_ip_allow_add` and the
`DETWS_IPV4` macro.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_IP_ALLOWLIST=1" \
  --lib="." examples/L4-Transport/07.IpAllowlist/07.IpAllowlist.ino
```

Connect from an address inside `192.168.1.0/24` (allowed) and from one outside it
(connection dropped at accept).

## Annotated source

The complete sketch ([07.IpAllowlist.ino](07.IpAllowlist.ino)), reproduced
verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_IP_ALLOWLIST 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/transport/listener.h" // listener_ip_allow_add / DETWS_IPV4
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

    // Only these sources may connect; everything else is dropped at accept time.
    // (An empty allowlist would allow everything - add at least one rule.)
    listener_ip_allow_add(DETWS_IPV4(192, 168, 1, 0), 24); // local /24
    listener_ip_allow_add(DETWS_IPV4(10, 0, 0, 5), 32);    // one trusted host

    server.on("/", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "hello from an allowed address"); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
