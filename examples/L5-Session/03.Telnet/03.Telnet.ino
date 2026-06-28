// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 03.Telnet.ino
 * @brief Line-oriented Telnet console (RFC 854) on port 23 (DETWS_ENABLE_TELNET).
 *
 * Opens a Telnet listener via server.listen(23, PROTO_TELNET). The server
 * negotiates echo + character mode, edits the line for you (backspace works),
 * and delivers each completed line to the command callback; respond with
 * telnet_print/println/printf.
 *
 * Telnet is PLAINTEXT - no auth, no encryption. Use only on a trusted LAN;
 * prefer SSH (29.SSH) or the WebSocket terminal (35.WebTerminal) otherwise.
 *
 * NOTE: this feature is compiled into the library only when DETWS_ENABLE_TELNET
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_TELNET=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 *
 * Flash, open Serial @ 115200 for the IP, then: telnet <ip> 23  (type "help").
 */

#define DETWS_ENABLE_TELNET 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/telnet/telnet.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

void on_command(const char *line, uint8_t conn_id)
{
    (void)conn_id;
    if (strcmp(line, "help") == 0)
        telnet_println("commands: help, heap, uptime, <echo>");
    else if (strcmp(line, "heap") == 0)
        telnet_printf("free heap: %u bytes\r\n", ESP.getFreeHeap());
    else if (strcmp(line, "uptime") == 0)
        telnet_printf("uptime: %lu ms\r\n", millis());
    else if (line[0])
        telnet_printf("echo: %s\r\n", line);
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

    server.listen(23, PROTO_TELNET); // open the Telnet port
    telnet_on_command(on_command);

    server.begin(80); // also start HTTP (begin() activates all listeners)
    Serial.println("Telnet on port 23 (try: telnet <ip>)");
}

void loop()
{
    server.handle();
}
