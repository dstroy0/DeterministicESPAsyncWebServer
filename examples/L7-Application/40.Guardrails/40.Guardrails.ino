// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 40.Guardrails.ino
 * @brief Runtime heap/stack guardrails (DETWS_ENABLE_GUARDRAILS).
 *
 * Installs a breach callback and checks the guardrails once a second: free heap,
 * heap low-water, largest free block (fragmentation), and this task's remaining
 * stack. When any crosses its DETWS_GUARDRAIL_* floor the callback fires so the
 * app can shed load / drop to a safe state / reboot before exhaustion bites. The
 * live snapshot is also served as JSON at GET /health.
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_GUARDRAILS=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_GUARDRAILS 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/guardrails/guardrails.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static void on_breach(uint8_t breaches, const DetwsHealth *h)
{
    Serial.printf("[guardrail] breach=0x%02x heap=%u frag=%u stack=%u\n", breaches, (unsigned)h->free_heap,
                  (unsigned)h->largest_free_block, (unsigned)h->stack_free);
    // Real app: shed load, drop to a safe state, or ESP.restart().
}

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    detws_guardrails_begin(on_breach);

    server.on("/health", HTTP_GET, [](uint8_t id, HttpReq *) {
        DetwsHealth h;
        detws_guardrails_sample(&h);
        char buf[128];
        detws_health_json(&h, buf, sizeof(buf));
        server.send(id, 200, "application/json", buf);
    });
    server.begin(80);
}

void loop()
{
    static uint32_t last = 0;
    if (millis() - last >= 1000)
    {
        last = millis();
        detws_guardrails_check(); // fires on_breach() if any floor is crossed
    }
}
