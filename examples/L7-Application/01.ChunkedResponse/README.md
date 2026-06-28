# 01.ChunkedResponse - streaming a response of unknown length

**Layer:** L7 Application · **Build flags:** none (core features only)

## What this example teaches

When you do not know the response size up front (or it is large), you stream it
with HTTP chunked transfer encoding instead of building the whole body in a
buffer. `send_chunked()` writes the status and headers
(`Transfer-Encoding: chunked`), then calls your filler, then writes the
terminating chunk - so output size is unbounded but memory stays constant.

**A filler callback that writes pieces.** You pass a `ChunkFiller` that receives a
`ChunkedResponse &`; each `write()` / `printf()` emits one chunk straight to the
socket - nothing is buffered whole:

```cpp
static void stream_lines(ChunkedResponse &res, HttpReq *req) {
    for (int i = 1; i <= 50; i++) res.printf("line %d of 50\n", i);   // 50 chunks
}
...
server.send_chunked(slot_id, 200, "text/plain", stream_lines);
```

You can mix literal and formatted writes freely (the `/count` route does), and
the same `send_chunked()` is what the binary [CBOR](../../L6-Presentation/13.Cbor)
and [MessagePack](../../L6-Presentation/14.MsgPack) examples use to stream binary
payloads.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --lib="." examples/L7-Application/01.ChunkedResponse/01.ChunkedResponse.ino
```

```sh
curl -N http://<ip>/stream     # 50 lines arrive incrementally
curl -N http://<ip>/count      # printf-built chunks
```

## Annotated source

The complete sketch ([01.ChunkedResponse.ino](01.ChunkedResponse.ino)),
reproduced verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Filler: emit 50 lines, one chunk each, with no large intermediate buffer.
static void stream_lines(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    for (int i = 1; i <= 50; i++)
        res.printf("line %d of 50\n", i); // each printf() is one chunk on the wire
}

// Filler: mix literal writes and formatted writes.
static void stream_count(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("counting up:\n");
    for (int i = 0; i <= 10; i++)
        res.printf("  %d\n", i);
    res.write("done\n");
}

void handle_stream(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send_chunked(slot_id, 200, "text/plain", stream_lines); // headers + filler + terminator
}

void handle_count(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send_chunked(slot_id, 200, "text/plain", stream_count);
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

    server.on("/stream", HTTP_GET, handle_stream);
    server.on("/count", HTTP_GET, handle_count);

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
