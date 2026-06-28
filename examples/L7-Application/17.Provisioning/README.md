# 17.Provisioning - first-boot WiFi setup via a captive portal

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_PROVISIONING`

## What this example teaches

On first boot, with no stored credentials, the device brings up a softAP
("DetWS-Setup") and a catch-all DNS responder (raw lwIP UDP, no add-on library),
so joining the AP with a phone pops a credentials form. The submitted SSID/PSK
persist to NVS; on the next boot the device finds them, connects as a station, and
serves normally. No external libraries are needed: just WiFi softAP, lwIP UDP, and
Preferences (NVS).

**The boot decision: load credentials, else open the portal.**

```cpp
char ssid[33], psk[64];
if (detws_provisioning_load(ssid, sizeof(ssid), psk, sizeof(psk))) {
    init_wifi_physical(ssid, psk);          // we have creds -> normal station
    server.on("/", HTTP_GET, handle_root);
    server.begin(80);
} else {
    server.begin(80);
    detws_provisioning_begin(server, "DetWS-Setup"); // no creds -> captive portal
}
```

`detws_provisioning_load()` returns false until the device has been provisioned;
`detws_provisioning_begin()` stands up the softAP, the catch-all DNS, and the
form handler that writes NVS and reboots. To re-provision later, call
`detws_provisioning_clear()` (for example from a button handler).

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_PROVISIONING=1" \
  --lib="." examples/L7-Application/17.Provisioning/17.Provisioning.ino
```

Flash, join the WiFi network "DetWS-Setup" with a phone, open any page to reach
the captive portal, and submit your network's SSID/password. The device reboots
into station mode.

## Annotated source

The complete sketch ([17.Provisioning.ino](17.Provisioning.ino)), reproduced
verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_PROVISIONING 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/provisioning_service.h"
#include <WiFi.h>

DetWebServer server;

void handle_root(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/plain", "Provisioned - hello from station mode!");
}

void setup()
{
    Serial.begin(115200);

    char ssid[33];
    char psk[64];
    if (detws_provisioning_load(ssid, sizeof(ssid), psk, sizeof(psk)))
    {
        // Credentials present: connect as a normal station.
        init_wifi_physical(ssid, psk);
        Serial.print("Connecting to ");
        Serial.println(ssid);
        while (!wifi_ready())
            delay(250);
        Serial.print("\nIP: ");
        Serial.println(WiFi.localIP());

        server.on("/", HTTP_GET, handle_root);
        server.begin(80);
        Serial.println("Station mode; serving on port 80");
    }
    else
    {
        // No credentials: bring up the captive portal (softAP + catch-all DNS + form).
        server.begin(80);
        detws_provisioning_begin(server, "DetWS-Setup");
        Serial.println("Provisioning: join WiFi 'DetWS-Setup' and open any page");
    }
}

void loop()
{
    server.handle();
}
```
