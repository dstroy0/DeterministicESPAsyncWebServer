// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 32.DeviceUuid.ino
 * @brief Stable MAC-derived device UUID (DETWS_ENABLE_DEVICE_ID).
 *
 * detws_device_uuid() derives a deterministic RFC 4122 v5 UUID from the chip's
 * factory MAC - the same value on every boot, with no storage. Use it as a
 * stable identity for mDNS hostnames, MQTT client IDs, telemetry tags, etc.
 *
 * Flash, open Serial @ 115200 for the IP + UUID, then GET http://<ip>/id.
 */

#define DETWS_ENABLE_DEVICE_ID 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/device_id/device_id.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;
static char g_uuid[DETWS_UUID_STR_LEN];

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

    detws_device_uuid(g_uuid); // stable per-chip UUID
    Serial.printf("device UUID: %s\n", g_uuid);

    server.on("/id", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
        char body[64];
        snprintf(body, sizeof(body), "{\"uuid\":\"%s\"}", g_uuid);
        server.send(id, 200, "application/json", body);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
