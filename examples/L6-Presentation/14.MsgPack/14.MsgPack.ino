// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 14.MsgPack.ino
 * @brief Encode and decode compact binary MessagePack (DETWS_ENABLE_MSGPACK).
 *
 * GET /telemetry.msgpack encodes a small {heap, uptime, rssi} map with the
 * zero-heap MessagePack writer into a stack buffer and streams the bytes as
 * application/msgpack (via the binary-safe chunked writer). POST /decode runs the
 * other direction: it parses a posted MessagePack map with the cursor decoder
 * (msgpack_peek / msgpack_read_*, no heap, pointing into the request buffer) and
 * echoes the parsed integer fields as text. MessagePack is a widely-supported
 * compact binary format - the same idea as the CBOR example, in the format your
 * stack may prefer.
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_MSGPACK=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Try: curl -s http://<ip>/telemetry.msgpack | xxd
 *      printf '\x81\xa3led\x01' | curl -s --data-binary @- http://<ip>/decode
 */

#define DETWS_ENABLE_MSGPACK 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/msgpack/msgpack.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// One MessagePack map {"heap","uptime","rssi"}, encoded into a ctx buffer and
// paged out by the chunk source (the same pattern scales to an arbitrarily large
// body: produce the next slice each call, return 0 when drained).
struct MpCtx
{
    uint8_t buf[64];
    size_t len, off;
};
static size_t msgpack_source(uint8_t *out, size_t cap, void *vctx)
{
    MpCtx *c = (MpCtx *)vctx;
    if (c->off >= c->len)
        return 0;
    size_t n = c->len - c->off;
    if (n > cap)
        n = cap;
    memcpy(out, c->buf + c->off, n);
    c->off += n;
    return n;
}

// Decodes a posted MessagePack map of {string: integer} and echoes each parsed
// field as "key=value" text. Shows the cursor decoder: read the map header, then
// each key (str) and value (int), checking msgpack_reader_ok() at the end.
static void on_decode(uint8_t id, HttpReq *req)
{
    MsgpackReader r;
    msgpack_reader_init(&r, req->body, req->body_len);
    size_t count;
    if (!msgpack_read_map(&r, &count))
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
        if (!msgpack_read_str(&r, &key, &klen) || !msgpack_read_int(&r, &val))
            break;
        o += snprintf(out + o, sizeof(out) - o, "%.*s=%lld\n", (int)klen, key, (long long)val);
    }
    if (!msgpack_reader_ok(&r))
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
    server.on("/decode", HTTP_POST, on_decode);
    server.begin(80);
}

void loop()
{
    server.handle();
}
