// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 68.UdpTelemetry.ino
 * @brief Fire-and-forget UDP telemetry cast (DETWS_ENABLE_UDP_TELEMETRY).
 *
 * Builds an InfluxDB line-protocol record (`esp32 heap=...i,rssi=...i,temp=...`)
 * and casts it to a collector over UDP once a second - zero-heap, no ACK, no
 * retry. Point it at Telegraf/InfluxDB's UDP listener, or just watch the packets:
 *     nc -u -l 8094        # then run this sketch casting to your host:8094
 *
 * NOTE: enable it for the whole build (a .ino #define does not reach the
 * separately compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_UDP_TELEMETRY=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_UDP_TELEMETRY 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/udp_telemetry/udp_telemetry.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// Where the telemetry datagrams go (your collector's UDP host + port).
static const char *COLLECTOR_IP = "192.168.1.10";
static const uint16_t COLLECTOR_PORT = 8094;

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

    detws_udp_telemetry_begin(COLLECTOR_IP, COLLECTOR_PORT);
}

void loop()
{
    static uint32_t last = 0;
    if (millis() - last >= 1000)
    {
        last = millis();
        char buf[DETWS_UDP_TELEMETRY_BUF];
        DetwsLine line;
        detws_line_init(&line, buf, sizeof(buf), "esp32");
        detws_line_add_uint(&line, "heap", ESP.getFreeHeap());
        detws_line_add_int(&line, "rssi", WiFi.RSSI());
        detws_line_add_float(&line, "temp", temperatureRead(), 1);
        if (detws_udp_telemetry_cast(&line))
            Serial.printf("cast: %s\n", buf);
    }
}
