// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 13.Cbor.ino
 * @brief Serve telemetry as compact binary CBOR (DETWS_ENABLE_CBOR).
 *
 * Encodes a small {heap, uptime, rssi} map with the zero-heap CBOR writer into a
 * stack buffer and streams the bytes as application/cbor (via the chunked writer,
 * which is binary-safe). CBOR is a few bytes where the JSON equivalent is dozens,
 * which matters for high-rate telemetry or constrained uplinks.
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_CBOR=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Try: curl -s http://<ip>/telemetry.cbor | xxd
 */

#define DETWS_ENABLE_CBOR 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/cbor.h"
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
    cbor_map(&w, 3);
    cbor_text(&w, "heap");
    cbor_uint(&w, ESP.getFreeHeap());
    cbor_text(&w, "uptime");
    cbor_uint(&w, millis() / 1000);
    cbor_text(&w, "rssi");
    cbor_int(&w, WiFi.RSSI());
    if (cbor_ok(&w))
        res.write((const char *)buf, cbor_len(&w));
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
