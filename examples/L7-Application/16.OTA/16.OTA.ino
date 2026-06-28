// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 38.OTA.ino
 * @brief Authenticated over-the-air firmware update (DETWS_ENABLE_OTA).
 *
 * Registers a streaming `POST /update` endpoint: a firmware image POSTed with
 * valid HTTP Basic credentials is fed straight into the ESP32 Update API via
 * the parser's streaming-body hook (the image never has to fit in RAM), then
 * the device reboots into the new firmware.
 *
 * Upload with:
 *   curl -u admin:s3cret --data-binary @firmware.bin http://<ip>/update
 *
 * (Generate firmware.bin with `pio run` - it is .pio/build/<env>/firmware.bin.)
 */

#define DETWS_ENABLE_OTA 1

#include "DeterministicESPAsyncWebServer.h"
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

    // Authenticated streaming OTA at POST /update.
    detws_ota_begin(server, "/update", "admin", "s3cret");

    Serial.println("Server up; OTA at POST /update");
}

void loop()
{
    server.handle();
}
