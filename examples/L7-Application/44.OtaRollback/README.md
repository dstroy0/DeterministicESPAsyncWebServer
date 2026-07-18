# 44.OtaRollback - OTA rollback protection / soft-brick safeguard

**Layer:** L7 Application · **Build flags:** `DWS_ENABLE_OTA_ROLLBACK`

## What this example teaches

A bad OTA image that boots but cannot do its job will soft-brick a remote device.
The ESP32 bootloader can mark a freshly flashed image `PENDING_VERIFY` and roll
back to the previous one unless the new firmware confirms itself. This wraps that
mechanism: each loop it runs a self-test (here WiFi up + healthy heap) and ticks the
rollback service - a passing self-test commits the image, a failing one (or no
confirm within `DWS_OTA_CONFIRM_WINDOW_MS`) rolls back. So a bad update self-heals
instead of bricking. It is the safety net for the OTA upload in
[16.OTA](../16.OTA).

**Define a self-test, then tick until committed:**

```cpp
static bool self_test() { return wifi_ready() && ESP.getFreeHeap() > 20000; }

void loop() {
    static bool done = false;
    if (!done) {
        DWSOtaAction a = dws_ota_rollback_tick(self_test());
        if (a == DWS_OTA_COMMIT) { Serial.println("[ota] image committed"); done = true; }
    }
    server.handle();
}
```

`dws_ota_rollback_tick(ok)` is a no-op once the image is committed or on a
normally-booted image, so it is safe to call every loop. `dws_ota_img_state()`
reports the current image state for the `/ota-state` endpoint.

**Requirement.** Actual rollback needs the bootloader's app-rollback support
(`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`) enabled in the build.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_OTA_ROLLBACK=1" \
  --lib="." examples/L7-Application/44.OtaRollback/44.OtaRollback.ino
```

```sh
curl http://<ip>/ota-state   # {"img_state":...} - the running image's verify state
```

## Annotated source

The complete sketch ([44.OtaRollback.ino](44.OtaRollback.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DWS_ENABLE_OTA_ROLLBACK 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/ota_rollback/ota_rollback.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

// The health check that decides commit vs rollback. Put your real checks here.
static bool self_test()
{
    return wifi_ready() && ESP.getFreeHeap() > 20000;
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

    server.on("/ota-state", HTTP_GET, [](uint8_t id, HttpReq *) {
        char b[48];
        snprintf(b, sizeof(b), "{\"img_state\":%u}", dws_ota_img_state());
        server.send(id, 200, "application/json", b);
    });
    server.begin(80);
}

void loop()
{
    // Confirm (or roll back) a freshly-updated image. A no-op once committed or on
    // a normally-booted image.
    static bool done = false;
    if (!done)
    {
        DWSOtaAction a = dws_ota_rollback_tick(self_test());
        if (a == DWS_OTA_COMMIT)
        {
            Serial.println("[ota] image committed");
            done = true;
        }
    }
    server.handle();
}
```
