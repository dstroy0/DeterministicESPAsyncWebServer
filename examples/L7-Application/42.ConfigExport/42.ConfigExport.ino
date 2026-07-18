// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 42.ConfigExport.ino
 * @brief Schema-driven config export / restore (DWS_ENABLE_CONFIG_IO).
 *
 * Declares a schema of persisted fields and serves them as a portable text blob:
 *   GET  /config            -> dumps `key=value` lines (backup / migrate)
 *   POST /config (that body)-> restores them into NVS (bulk provisioning)
 * Schema-driven over the typed NVS config store - deterministic, zero-heap.
 *
 * NOTE: enable both flags for the whole build. In platformio.ini:
 *     build_flags = -DDWS_ENABLE_CONFIG_STORE=1 -DDWS_ENABLE_CONFIG_IO=1
 * (Arduino IDE: they are already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DWS_ENABLE_CONFIG_STORE 1
#define DWS_ENABLE_CONFIG_IO 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/config_io/config_io.h"
#include "services/config_store/config_store.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

// The persisted fields to back up / restore.
static const DetwsCfgField SCHEMA[] = {
    {"hostname", DetwsCfgType::DWS_CFG_STR},
    {"http_port", DetwsCfgType::DWS_CFG_U32},
    {"location", DetwsCfgType::DWS_CFG_STR},
};
static const size_t SCHEMA_N = sizeof(SCHEMA) / sizeof(SCHEMA[0]);

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    // Seed a couple of values (normally set at provisioning).
    dws_config_begin("app");
    dws_config_set_str("hostname", "sensor-01");
    dws_config_set_u32("http_port", 80);
    dws_config_set_str("location", "lab");

    server.on("/config", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
        char buf[512];
        dws_config_export("app", SCHEMA, SCHEMA_N, buf, sizeof(buf));
        server.send(id, 200, "text/plain", buf);
    });
    server.on("/config", HttpMethod::HTTP_POST, [](uint8_t id, HttpReq *req) {
        int n = dws_config_import("app", SCHEMA, SCHEMA_N, (const char *)req->body, req->body_len);
        char msg[48];
        snprintf(msg, sizeof(msg), "imported %d field(s)\n", n);
        server.send(id, 200, "text/plain", msg);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
