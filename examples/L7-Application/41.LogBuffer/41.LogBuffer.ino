// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 41.LogBuffer.ino
 * @brief Fixed-RAM rotating log buffer with severity traps (DETWS_ENABLE_LOGBUF).
 *
 * Keeps the last DETWS_LOG_LINES log lines in RAM (oldest pruned on overflow),
 * serves them at GET /logs, and fires a trap on WARN+ lines (here it just prints,
 * but a real app could forward an SNMP trap or a webhook). Try: curl http://<ip>/logs
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_LOGBUF=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_LOGBUF 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/logbuf/logbuf.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static void on_trap(uint8_t level, const char *line)
{
    Serial.printf("[trap] %s\n", line); // forward criticals here (SNMP trap / webhook)
    (void)level;
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

    detws_log_set_trap(DETWS_LOG_WARN, on_trap); // trap on WARN and ERROR
    detws_log(DETWS_LOG_INFO, "boot complete");

    server.on("/logs", HTTP_GET, [](uint8_t id, HttpReq *) {
        char buf[DETWS_LOG_LINES * DETWS_LOG_LINE_LEN];
        detws_log_dump(buf, sizeof(buf));
        server.send(id, 200, "text/plain", buf);
    });
    server.begin(80);
}

void loop()
{
    static uint32_t last = 0;
    if (millis() - last >= 5000)
    {
        last = millis();
        char msg[64];
        uint32_t heap = ESP.getFreeHeap();
        snprintf(msg, sizeof(msg), "heap=%u uptime=%lus", (unsigned)heap, millis() / 1000);
        detws_log(heap < 20000 ? DETWS_LOG_WARN : DETWS_LOG_INFO, msg);
    }
    server.handle();
}
