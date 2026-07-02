// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 21.PrometheusMetrics.ino
 * @brief Prometheus `/metrics` endpoint (text exposition format 0.0.4).
 *
 * Exposes the server's runtime counters as Prometheus metrics so a Prometheus
 * server can scrape the device directly - uptime, total requests, responses by
 * status class, active connections, slot capacity, and free heap. The counters
 * come from the built-in stats subsystem (DETWS_ENABLE_STATS); metrics() just
 * renders them in Prometheus format.
 *
 * Flash, open Serial @ 115200 for the IP, then:
 *   curl http://<ip>/metrics
 *   # or point Prometheus at it:  scrape_configs: [{ static_configs: [{ targets: ['<ip>'] }] }]
 *
 * NOTE: optional features are gated by a compile flag the *library* sources must
 * also see; for PlatformIO enable them for the whole build, e.g.:
 *     build_flags = -DDETWS_ENABLE_STATS=1 -DDETWS_ENABLE_METRICS=1
 * (Arduino IDE: they are already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DETWS_ENABLE_STATS 1
#define DETWS_ENABLE_METRICS 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

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

    // A couple of normal routes so the counters have something to report.
    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "hello"); });
    server.on("/work", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "did work"); });

    // The Prometheus scrape endpoint.
    server.on("/metrics", HTTP_GET, [](uint8_t id, HttpReq *) { server.metrics(id); });

    int32_t result = server.begin(80);
    if (result < 0)
        Serial.printf("begin() failed (error %d)\n", result);
    else
        Serial.println("Prometheus metrics on :80 (curl http://<ip>/metrics)");
}

void loop()
{
    server.handle();
}
