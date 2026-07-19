# FileUpload - stream a POST body straight to a file

**Layer:** L7 Application · **Build flags:** `DWS_ENABLE_UPLOAD`

## What this example teaches

This streams a request body into a LittleFS file in `FILE_CHUNK_SIZE` pieces via
the parser's streaming-body hook - the same mechanism OTA uses - so an upload
never has to fit in RAM. A GET route serves the stored file back to verify the
round-trip.

**One call wires the upload sink.** `dws_upload_begin(server, path, fs, dest)`
registers a POST route whose body is streamed to `dest` as it arrives:

```cpp
dws_upload_begin(server, "/upload", LittleFS, DEST);     // POST body -> file, chunk by chunk
server.on("/file", HTTP_GET, [](uint8_t id, HttpReq *) {   // read it back
    server.serve_file(id, LittleFS, DEST, "application/octet-stream");
});
```

**Two important constraints (from the header):**

- The upload sink shares the parser's streaming hook with OTA - **enable one or
  the other, not both.**
- `RX_BUF_SIZE` must exceed the largest inbound TCP segment (`TCP_MSS`, ~1460) so
  a full segment fits the receive ring. The default 1024 is fine for ordinary
  requests but too small here, so build with e.g. `-DRX_BUF_SIZE=2048`. (The
  transport refuses-and-redelivers an oversize segment rather than losing data,
  but a ring smaller than one segment would stall.)

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_UPLOAD=1 -DRX_BUF_SIZE=2048" \
  --lib="." examples/L7-Application/FileUpload/FileUpload.ino
```

```sh
curl --data-binary @somefile.bin http://<ip>/upload   # 200 OK <n> bytes
curl http://<ip>/file > roundtrip.bin                 # read it back
```

## Annotated source

The complete sketch ([FileUpload.ino](FileUpload.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DWS_ENABLE_UPLOAD 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/upload_service.h"
#include <LittleFS.h>
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *DEST = "/uploaded.bin";

DWS server;

void setup()
{
    Serial.begin(115200);

    if (!LittleFS.begin(true)) // format on first use
        Serial.println("LittleFS mount failed");

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

    // POST /upload -> stream the body into DEST on LittleFS (chunked, never in RAM).
    dws_upload_begin(server, "/upload", LittleFS, DEST);

    // GET /file -> serve the stored file back.
    server.on("/file", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.serve_file(id, LittleFS, DEST, "application/octet-stream"); });

    int32_t result = server.begin(80);
    if (result < 0)
        Serial.printf("begin() failed (error %d)\n", result);
    else
        Serial.println("Upload server on :80 (curl --data-binary @file http://<ip>/upload)");
}

void loop()
{
    server.handle();
}
```
