// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 62.Dashboard.ino
 * @brief Real-time SVG telemetry dashboard (DETWS_ENABLE_DASHBOARD).
 *
 * Declares a fixed compile-time widget table and serves it at /dashboard: the
 * self-contained SVG page renders one widget per entry (gauge / value /
 * sparkline / bar) and updates them live over SSE. The sketch feeds readings with
 * detws_dashboard_set(key, value) and pushes a frame with detws_dashboard_publish().
 *
 * NOTE: enable the dashboard for the whole build (a .ino #define does not reach
 * the separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_DASHBOARD=1
 * (DETWS_ENABLE_SSE is on by default. Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Flash, then open http://<ip>/dashboard.
 */

#define DETWS_ENABLE_DASHBOARD 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/dashboard/dashboard.h"
#include <WiFi.h>
#include <math.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Compile-time widget table - the dashboard's deterministic source of truth.
static const DetwsWidget WIDGETS[] = {
    {DETWS_WIDGET_GAUGE, "Free heap", "heap", 0, 320000, "B"},
    {DETWS_WIDGET_VALUE, "Uptime", "uptime", 0, 0, "s"},
    {DETWS_WIDGET_SPARKLINE, "WiFi RSSI", "rssi", -100, 0, "dBm"},
    {DETWS_WIDGET_BAR, "Demo signal", "demo", 0, 100, "%"},
};

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

    detws_dashboard_begin(server, "/dashboard", WIDGETS, sizeof(WIDGETS) / sizeof(WIDGETS[0]));
    server.begin(80);
}

void loop()
{
    server.handle();

    // Push a telemetry frame once a second.
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms >= 1000)
    {
        last_ms = now;
        detws_dashboard_set("heap", (float)ESP.getFreeHeap());
        detws_dashboard_set("uptime", (float)(now / 1000));
        detws_dashboard_set("rssi", (float)WiFi.RSSI());
        detws_dashboard_set("demo", 50.0f + 50.0f * sinf((float)now / 2000.0f));
        detws_dashboard_publish();
    }
}
