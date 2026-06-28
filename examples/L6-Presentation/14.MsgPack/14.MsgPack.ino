// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 66.MsgPack.ino
 * @brief Serve telemetry as compact binary MessagePack (DETWS_ENABLE_MSGPACK).
 *
 * Encodes a small {heap, uptime, rssi} map with the zero-heap MessagePack writer
 * into a stack buffer and streams the bytes as application/msgpack (via the
 * binary-safe chunked writer). MessagePack is a widely-supported compact binary
 * format - the same idea as the CBOR example, in the format your stack may prefer.
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_MSGPACK=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Try: curl -s http://<ip>/telemetry.msgpack | xxd
 */

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
    msgpack_int(&w, WiFi.RSSI());
    if (msgpack_ok(&w))
        res.write((const char *)buf, msgpack_len(&w));
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
