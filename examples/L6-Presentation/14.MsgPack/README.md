# 14.MsgPack - compact binary telemetry with MessagePack

**Layer:** L6 Presentation · **Build flags:** `DETWS_ENABLE_MSGPACK`

## What this example teaches

This is the [CBOR example](../13.Cbor) in a different wire format: it encodes the
same `{heap, uptime, rssi}` map with the zero-heap MessagePack writer and streams
it as `application/msgpack`. MessagePack is widely supported across languages, so
pick it over CBOR when your consuming stack already speaks it - the API and the
zero-heap pattern are identical.

**Encoding with `MsgpackWriter`.** Initialize over a stack buffer, declare the map
size, emit pairs, check `msgpack_ok()`, write `msgpack_len()` bytes:

```cpp
uint8_t buf[64];
MsgpackWriter w;
msgpack_init(&w, buf, sizeof(buf));
msgpack_map(&w, 3);
msgpack_str(&w, "heap"); msgpack_uint(&w, ESP.getFreeHeap());
msgpack_str(&w, "uptime"); msgpack_uint(&w, millis() / 1000);
msgpack_str(&w, "rssi"); msgpack_int(&w, WiFi.RSSI());
if (msgpack_ok(&w)) res.write((const char *)buf, msgpack_len(&w));
```

As with CBOR, the payload is binary so it is delivered through the binary-safe
`send_chunked()` writer rather than the C-string `send()`.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_MSGPACK=1" \
  --lib="." examples/L6-Presentation/14.MsgPack/14.MsgPack.ino
```

```sh
curl -s http://<ip>/telemetry.msgpack | xxd
```

## Annotated source

The complete sketch ([14.MsgPack.ino](14.MsgPack.ino)), reproduced verbatim with
added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_MSGPACK 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/msgpack.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Streams one MessagePack map: {"heap": <uint>, "uptime": <uint>, "rssi": <int>}.
static void msgpack_telemetry(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    uint8_t buf[64];
    MsgpackWriter w;
    msgpack_init(&w, buf, sizeof(buf));
    msgpack_map(&w, 3);
    msgpack_str(&w, "heap");
    msgpack_uint(&w, ESP.getFreeHeap());
    msgpack_str(&w, "uptime");
    msgpack_uint(&w, millis() / 1000);
    msgpack_str(&w, "rssi");
    msgpack_int(&w, WiFi.RSSI()); // signed
    if (msgpack_ok(&w))
        res.write((const char *)buf, msgpack_len(&w)); // raw bytes via the binary-safe writer
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

    server.on("/telemetry.msgpack", HTTP_GET,
              [](uint8_t id, HttpReq *) { server.send_chunked(id, 200, "application/msgpack", msgpack_telemetry); });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
