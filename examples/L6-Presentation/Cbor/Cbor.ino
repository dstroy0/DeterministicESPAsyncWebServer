// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file Cbor.ino
 * @brief Serve telemetry as compact binary CBOR (DWS_ENABLE_CBOR).
 *
 * Encodes a small {heap, uptime, rssi} map with the zero-heap CBOR writer into a
 * stack buffer and streams the bytes as application/cbor (via the chunked writer,
 * which is binary-safe). CBOR is a few bytes where the JSON equivalent is dozens,
 * which matters for high-rate telemetry or constrained uplinks.
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDWS_ENABLE_CBOR=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Try: curl -s http://<ip>/telemetry.cbor | xxd
 */

#define DWS_ENABLE_CBOR 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/cbor/cbor.h"

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

// One CBOR map {"heap","uptime","rssi"}, encoded once into a ctx buffer and then
// paged out by the chunk source. (The body is tiny, but the same pattern serves an
// arbitrarily large one: produce the next slice each call, return 0 when drained.)
struct CborCtx
{
    uint8_t buf[64];
    size_t len, off;
};
static size_t dws_cbor_source(uint8_t *out, size_t cap, void *vctx)
{
    CborCtx *c = (CborCtx *)vctx;
    if (c->off >= c->len)
        return 0;
    size_t n = c->len - c->off;
    if (n > cap)
        n = cap;
    memcpy(out, c->buf + c->off, n);
    c->off += n;
    return n;
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
    uint32_t ip = dws_net_egress_ip(); // library egress IP (network byte order), no Arduino WiFi
    Serial.printf("\nIP: %u.%u.%u.%u\n", (unsigned)(ip & 0xFF), (unsigned)((ip >> 8) & 0xFF),
                  (unsigned)((ip >> 16) & 0xFF), (unsigned)((ip >> 24) & 0xFF));

    server.on("/telemetry.cbor", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
        static CborCtx ctx; // static: must outlive send_chunked
        CborWriter w;
        dws_cbor_init(&w, ctx.buf, sizeof(ctx.buf));
        dws_cbor_map(&w, 3);
        dws_cbor_text(&w, "heap");
        dws_cbor_uint(&w, ESP.getFreeHeap());
        dws_cbor_text(&w, "uptime");
        dws_cbor_uint(&w, millis() / 1000);
        dws_cbor_text(&w, "rssi");
        dws_cbor_int(&w, dws_net_rssi());
        ctx.len = dws_cbor_ok(&w) ? dws_cbor_len(&w) : 0;
        ctx.off = 0;
        server.send_chunked(id, 200, "application/cbor", dws_cbor_source, &ctx);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
