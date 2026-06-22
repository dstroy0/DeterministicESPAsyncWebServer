// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 08.Services.ino
 * @brief Optional network services: mDNS advertisement + SNTP time sync.
 *
 * Demonstrates the opt-in service wrappers (each gated by a DETWS_ENABLE_*
 * macro, default-off):
 *   - DETWS_ENABLE_MDNS: reach the device at `detws-demo.local` and advertise
 *     `_http._tcp` (detws_mdns_begin()).
 *   - DETWS_ENABLE_NTP: SNTP wall-clock sync (detws_ntp_begin()); GET /time
 *     returns the current RFC 7231 date once synced.
 *
 * After flashing, browse to http://detws-demo.local/time (mDNS) or the IP.
 */

#define DETWS_ENABLE_MDNS 1
#define DETWS_ENABLE_NTP 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/mdns_service.h"
#include "services/ntp_service.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";
static const char *HOSTNAME = "detws-demo";

DetWebServer server;

// GET /time - current wall-clock time, or 503 until the first SNTP sync lands.
void handle_time(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    char date[40];
    if (detws_ntp_http_date(date, sizeof(date)) == 0)
    {
        server.send(slot_id, 503, "text/plain", "Time not synced yet");
        return;
    }
    server.send(slot_id, 200, "text/plain", date);
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

    server.on("/time", HTTP_GET, handle_time);
    if (server.begin(80) < 0)
    {
        Serial.println("begin() failed");
        return;
    }

    // Advertise over mDNS so the device is reachable at <hostname>.local.
    if (detws_mdns_begin(HOSTNAME, 80))
        Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);

    // Start SNTP (UTC); the first sync arrives a few seconds later.
    detws_ntp_begin("UTC0");

    Serial.println("Server started");
}

void loop()
{
    server.handle();
}
