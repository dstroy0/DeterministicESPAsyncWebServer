# 14.MsgPack - compact binary telemetry with MessagePack

**Layer:** L6 Presentation · **Build flags:** `DETWS_ENABLE_MSGPACK`

## What this example teaches

This is the [CBOR example](../13.Cbor) in a different wire format, in both
directions: `GET /telemetry.msgpack` encodes the same `{heap, uptime, rssi}` map
with the zero-heap MessagePack writer and streams it as `application/msgpack`, and
`POST /decode` parses a posted MessagePack map with the zero-heap cursor decoder.
MessagePack is widely supported across languages, so pick it over CBOR when your
consuming stack already speaks it - the API and the zero-heap pattern are
identical.

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
ctx.len = msgpack_ok(&w) ? msgpack_len(&w) : 0;   // page these bytes out below
```

As with CBOR, the payload is binary so it is delivered through the binary-safe
`send_chunked()`, which pulls the encoded bytes from a `ChunkSource` generator
(slice by slice, returning 0 to finish) rather than the C-string `send()`. The
generator's `ctx` must outlive the call, so it is `static`.

**Decoding with `MsgpackReader`.** The cursor decoder is the mirror image: bind a
reader to the bytes, read the map header, then each key/value, and check
`msgpack_reader_ok()` once at the end (it is sticky, so a single check covers the
whole parse). Strings point straight into the source buffer, no copy:

```cpp
MsgpackReader r;
msgpack_reader_init(&r, req->body, req->body_len);
size_t count;
if (!msgpack_read_map(&r, &count)) { /* not a map */ }
for (size_t i = 0; i < count && msgpack_reader_ok(&r); i++) {
    const char *key; size_t klen; int64_t val;
    if (!msgpack_read_str(&r, &key, &klen) || !msgpack_read_int(&r, &val)) break;
    // use key[0..klen) and val
}
if (!msgpack_reader_ok(&r)) { /* malformed / truncated */ }
```

Every read is bounds-checked, so malformed or truncated input fails closed rather
than over-reading. `msgpack_peek()` reports the next object's type if you need to
branch on it.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_MSGPACK=1" \
  --lib="." examples/L6-Presentation/14.MsgPack/14.MsgPack.ino
```

```sh
curl -s http://<ip>/telemetry.msgpack | xxd
# decode side: post a one-pair map {"led": 1} (0x81 0xa3 'led' 0x01)
printf '\x81\xa3led\x01' | curl -s --data-binary @- http://<ip>/decode
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
#include "network_drivers/presentation/msgpack/msgpack.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// One MessagePack map {"heap","uptime","rssi"}, encoded into a ctx buffer and
// paged out by the chunk source (the same pattern scales to an arbitrarily large body).
struct MpCtx
{
    uint8_t buf[64];
    size_t len, off;
};
static size_t msgpack_source(uint8_t *out, size_t cap, void *vctx)
{
    MpCtx *c = (MpCtx *)vctx;
    if (c->off >= c->len)
        return 0; // done
    size_t n = c->len - c->off;
    if (n > cap)
        n = cap;
    memcpy(out, c->buf + c->off, n);
    c->off += n;
    return n;
}

// Decodes a posted MessagePack map of {string: integer} and echoes "key=value".
static void on_decode(uint8_t id, HttpReq *req)
{
    MsgpackReader r;
    msgpack_reader_init(&r, req->body, req->body_len); // cursor over the request body
    size_t count;
    if (!msgpack_read_map(&r, &count)) // header must be a map
    {
        server.send(id, 400, "text/plain", "expected a MessagePack map");
        return;
    }
    char out[160];
    size_t o = 0;
    for (size_t i = 0; i < count && msgpack_reader_ok(&r); i++)
    {
        const char *key;
        size_t klen;
        int64_t val;
        if (!msgpack_read_str(&r, &key, &klen) || !msgpack_read_int(&r, &val)) // key then value
            break;
        o += snprintf(out + o, sizeof(out) - o, "%.*s=%lld\n", (int)klen, key, (long long)val);
    }
    if (!msgpack_reader_ok(&r)) // one sticky check covers the whole parse
    {
        server.send(id, 400, "text/plain", "malformed MessagePack");
        return;
    }
    server.send(id, 200, "text/plain", out);
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

    server.on("/telemetry.msgpack", HTTP_GET, [](uint8_t id, HttpReq *) {
        static MpCtx ctx; // static: must outlive send_chunked
        MsgpackWriter w;
        msgpack_init(&w, ctx.buf, sizeof(ctx.buf));
        msgpack_map(&w, 3);
        msgpack_str(&w, "heap");
        msgpack_uint(&w, ESP.getFreeHeap());
        msgpack_str(&w, "uptime");
        msgpack_uint(&w, millis() / 1000);
        msgpack_str(&w, "rssi");
        msgpack_int(&w, WiFi.RSSI());
        ctx.len = msgpack_ok(&w) ? msgpack_len(&w) : 0;
        ctx.off = 0;
        server.send_chunked(id, 200, "application/msgpack", msgpack_source, &ctx);
    });
    server.on("/decode", HTTP_POST, on_decode); // decode side
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
