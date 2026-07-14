# 16.OTA - authenticated over-the-air firmware update

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_OTA`

## What this example teaches

This registers a streaming `POST /update` endpoint: a firmware image POSTed with
valid HTTP Basic credentials is fed straight into the ESP32 `Update` API through
the parser's streaming-body hook, so the image never has to fit in RAM, then the
device reboots into the new firmware.

**One call wires authenticated streaming OTA:**

```cpp
detws_ota_begin(server, "/update", "admin", "s3cret"); // POST body -> Update, then reboot
```

The body is streamed chunk by chunk into flash as it arrives; only requests with
the configured Basic credentials are accepted. Because it uses the same
streaming-body hook as file upload, **enable OTA or upload, not both** (see
[11.FileUpload](../11.FileUpload)). Build your `firmware.bin` with `pio run`
(it lands at `.pio/build/<env>/firmware.bin`).

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_OTA=1 -DMAX_CONNS=4" \
  --lib="." examples/L7-Application/16.OTA/16.OTA.ino
```

```sh
curl -u admin:s3cret --data-binary @firmware.bin http://<ip>/update
```

## Annotated source

The complete sketch ([16.OTA.ino](16.OTA.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_OTA 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/ota_service.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

void handle_root(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/plain", "OTA demo - POST a firmware image to /update");
}

void setup()
{
    Serial.begin(115200);

    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handle_root);
    if (server.begin(80) < 0)
    {
        Serial.println("begin() failed");
        return;
    }

    // Authenticated streaming OTA at POST /update (Basic admin:s3cret).
    detws_ota_begin(server, "/update", "admin", "s3cret");

    Serial.println("Server up; OTA at POST /update");
}

void loop()
{
    server.handle();
}
```
