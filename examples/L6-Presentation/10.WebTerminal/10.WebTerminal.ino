// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 10.WebTerminal.ino
 * @brief Browser "web serial" terminal over WebSocket (docs CRT theme).
 *
 * Serves a self-contained green-phosphor terminal page (matching the docs site)
 * and a WebSocket endpoint: device output streams to every open browser, and
 * each line you type is delivered to a command callback. Zero-heap - rides the
 * library's existing WebSocket layer. The page auto-selects ws:// or wss://, so
 * it works unchanged once TLS is enabled.
 *
 * Flash, open Serial @ 115200 for the IP, then browse to:
 *   http://<ip>/terminal
 * Type "help". The device also prints the uptime every few seconds.
 *
 * NOTE: optional services are gated by a compile flag the *library* sources must
 * also see. The `#define` below documents intent, but for PlatformIO you must
 * enable it for the whole build, e.g. in platformio.ini:
 *     build_flags = -DDETWS_ENABLE_WEB_TERMINAL=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.) A define in the
 * sketch alone does not reach the separately-compiled library .cpp.
 */

#define DETWS_ENABLE_WEB_TERMINAL 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/web_terminal/web_terminal.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Browser -> device: handle a typed command line.
void on_command(const char *line, uint8_t client_id)
{
    (void)client_id;
    if (strcmp(line, "help") == 0)
        detws_web_terminal_println("commands: help, heap, uptime, <echo>");
    else if (strcmp(line, "heap") == 0)
        detws_web_terminal_printf("free heap: %u bytes\n", ESP.getFreeHeap());
    else if (strcmp(line, "uptime") == 0)
        detws_web_terminal_printf("uptime: %lu ms\n", millis());
    else
        detws_web_terminal_printf("echo: %s\n", line);
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
    Serial.println("Open http://<ip>/terminal in a browser");

    WiFi.setSleep(false);

    detws_web_terminal_begin(server, "/terminal");
    detws_web_terminal_on_command(on_command);

    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    server.handle();

    // Device -> browsers: heartbeat every 3 s (only sent when someone's watching).
    static unsigned long last = 0;
    if (millis() - last >= 3000)
    {
        last = millis();
        if (detws_web_terminal_client_count() > 0)
            detws_web_terminal_printf("uptime %lu ms, heap %u\n", millis(), ESP.getFreeHeap());
    }
}
