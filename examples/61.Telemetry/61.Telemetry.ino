// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 61.Telemetry.ino
 * @brief Moving-window stats, rate-of-change, and a totalizer (DETWS_ENABLE_TELEMETRY).
 *
 * Turns a periodic sensor reading into dashboard figures with zero heap: a
 * moving-window mean/stddev/min/max, the rate of change (slope) of the signal,
 * and a run-time totalizer that integrates the reading over time (an odometer).
 * GET /telemetry returns them as JSON for a dashboard or alert rule.
 *
 * NOTE: enable the helpers for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_TELEMETRY=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_TELEMETRY 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/telemetry/telemetry.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static float g_window_buf[16];  // caller-owned window storage (no heap)
static DetwsWindow g_window;
static DetwsRate g_rate;
static DetwsTotalizer g_total;
static float g_last_rate = 0.0f;

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

    detws_window_init(&g_window, g_window_buf, 16);
    detws_rate_init(&g_rate);
    detws_totalizer_init(&g_total);

    server.on("/telemetry", HTTP_GET, [](uint8_t id, HttpReq *) {
        char body[192];
        snprintf(body, sizeof(body),
                 "{\"samples\":%u,\"mean\":%.3f,\"stddev\":%.3f,\"min\":%.3f,\"max\":%.3f,"
                 "\"rate_per_s\":%.3f,\"total\":%.3f}",
                 (unsigned)detws_window_count(&g_window), detws_window_mean(&g_window),
                 detws_window_stddev(&g_window), detws_window_min(&g_window), detws_window_max(&g_window),
                 g_last_rate, detws_totalizer_total(&g_total));
        server.send(id, 200, "application/json", body);
    });

    server.begin(80);
}

void loop()
{
    server.handle();

    // Sample once a second and fold it into the telemetry helpers.
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms >= 1000)
    {
        last_ms = now;
        float sample = (float)analogRead(34) * (3.3f / 4095.0f); // example: ADC voltage

        detws_window_push(&g_window, sample);          // stats over the last 16 readings
        g_last_rate = detws_rate_update(&g_rate, sample, now); // slope (units/s)
        detws_totalizer_add(&g_total, sample, now);    // integrate the reading over time
    }
}
