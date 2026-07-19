# InterfaceFilter - per-interface (STA / softAP) routes

**Layer:** L7 Application · **Build flags:** none (core features only)

## What this example teaches

A device running both a station link (your LAN) and a softAP (its own network) can
expose different routes on each. This brings up AP+STA mode and gates a config
page to the softAP and an app API to the station, so a first-boot setup page is
never reachable from the LAN and vice versa.

**Tell the server which IP is the AP.** The interface is decided by comparing each
connection's local IP to the softAP IP, so you must register it once after
starting the AP:

```cpp
WiFi.mode(WIFI_AP_STA);
WiFi.softAP(AP_SSID, AP_PASS);
init_wifi_physical(SSID, PASSWORD);
...
server.set_ap_ip(WiFi.softAPIP());   // required for STA/AP classification
```

**Gate routes by interface.** Pass `DETIFACE_AP` or `DETIFACE_STA` as the last
`on()` argument; omit it for "any interface":

```cpp
server.on("/setup",    HTTP_GET, handle_setup, DETIFACE_AP);  // softAP only
server.on("/api/data", HTTP_GET, handle_api,   DETIFACE_STA); // station only
server.on("/",         HTTP_GET, handle_root);                // any interface
```

A request for `/setup` arriving on the station link is treated as unmatched (404),
and `/api/data` on the softAP likewise - so each surface is isolated to its
network.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --lib="." examples/L7-Application/InterfaceFilter/InterfaceFilter.ino
```

```sh
# from the LAN (station):     curl http://<sta-ip>/api/data -> 200 ; /setup -> 404
# joined to the softAP:       curl http://192.168.4.1/setup -> 200 ; /api/data -> 404
```

## Annotated source

The complete sketch ([InterfaceFilter.ino](InterfaceFilter.ino)),
reproduced verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";
static const char *AP_SSID = "DWS-Setup";
static const char *AP_PASS = "configme123";

DWS server;

void handle_setup(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/html", "<h1>Setup</h1><p>softAP only</p>");
}

void handle_api(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "application/json", "{\"data\":42,\"iface\":\"sta\"}");
}

void handle_root(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/plain", "available on both interfaces");
}

void setup()
{
    Serial.begin(115200);

    // AP + STA so both interfaces exist simultaneously.
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nSTA IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP  IP: ");
    Serial.println(WiFi.softAPIP());

    WiFi.setSleep(false);

    // Required for STA/AP classification (IPAddress converts to uint32_t).
    server.set_ap_ip(WiFi.softAPIP());

    server.on("/setup", HTTP_GET, handle_setup, DETIFACE_AP);   // softAP only
    server.on("/api/data", HTTP_GET, handle_api, DETIFACE_STA); // station only
    server.on("/", HTTP_GET, handle_root);                      // any interface

    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    server.handle();
}
```
