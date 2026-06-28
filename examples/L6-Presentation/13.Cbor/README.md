# 13.Cbor - compact binary telemetry with CBOR

**Layer:** L6 Presentation · **Build flags:** `DETWS_ENABLE_CBOR`

## What this example teaches

CBOR (RFC 8949) encodes the same data as JSON in a fraction of the bytes, which
matters for high-rate telemetry or constrained uplinks. This serves a small
`{heap, uptime, rssi}` map as `application/cbor` using the zero-heap CBOR writer,
streamed through the binary-safe chunked writer.

**Encoding with `CborWriter`.** You initialize the writer over a stack buffer,
declare the map size, then emit key/value pairs; `cbor_ok()` reports overflow and
`cbor_len()` gives the encoded length:

```cpp
uint8_t buf[64];
CborWriter w;
cbor_init(&w, buf, sizeof(buf));
cbor_map(&w, 3);                    // a 3-entry map
cbor_text(&w, "heap"); cbor_uint(&w, ESP.getFreeHeap());
cbor_text(&w, "uptime"); cbor_uint(&w, millis() / 1000);
cbor_text(&w, "rssi"); cbor_int(&w, WiFi.RSSI());   // signed
if (cbor_ok(&w)) res.write((const char *)buf, cbor_len(&w));
```

**Why the chunked writer.** The response is binary, so it is sent with
`send_chunked()` (which is binary-safe) rather than the C-string `send()`. The
encode callback receives a `ChunkedResponse &` and writes the raw bytes:

```cpp
server.on("/telemetry.cbor", HTTP_GET,
          [](uint8_t id, HttpReq *) { server.send_chunked(id, 200, "application/cbor", cbor_telemetry); });
```

For a text-based compact binary alternative, see [14.MsgPack](../14.MsgPack).

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_CBOR=1" \
  --lib="." examples/L6-Presentation/13.Cbor/13.Cbor.ino
```

```sh
curl -s http://<ip>/telemetry.cbor | xxd      # inspect the compact binary
```

## Annotated source

The complete sketch ([13.Cbor.ino](13.Cbor.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_CBOR 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/cbor/cbor.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Streams one CBOR map: {"heap": <uint>, "uptime": <uint>, "rssi": <int>}.
static void cbor_telemetry(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    uint8_t buf[64];
    CborWriter w;
    cbor_init(&w, buf, sizeof(buf));
    cbor_map(&w, 3);              // declare a 3-pair map up front
    cbor_text(&w, "heap");
    cbor_uint(&w, ESP.getFreeHeap());
    cbor_text(&w, "uptime");
    cbor_uint(&w, millis() / 1000);
    cbor_text(&w, "rssi");
    cbor_int(&w, WiFi.RSSI());    // signed value
    if (cbor_ok(&w))             // false if buf overflowed
        res.write((const char *)buf, cbor_len(&w)); // raw bytes via the binary-safe writer
}

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

    server.on("/telemetry.cbor", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send_chunked(id, 200, "application/cbor", cbor_telemetry); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
