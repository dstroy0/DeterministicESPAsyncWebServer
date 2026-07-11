// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 18.SNTP.ino
 * @brief Wall-clock time via SNTP (DETWS_ENABLE_NTP).
 *
 * detws_ntp_begin(tz) starts the ESP-IDF SNTP client; the first sync lands a few
 * seconds later. detws_ntp_http_date() formats the current time as an RFC 7231
 * date string (returns 0 until synced). GET /time returns it.
 *
 * NOTE: this service is compiled into the library only when DETWS_ENABLE_NTP
 * is set for the whole build (a .ino #define does not reach the separately
 * compiled library). In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_NTP=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 *
 * Flash, open Serial @ 115200 for the IP, then GET http://<ip>/time.
 */

#define DETWS_ENABLE_NTP 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/ntp_service.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

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
    WiFi.setSleep(false);

    server.on("/time", HttpMethod::HTTP_GET, handle_time);
    server.begin(80);

    detws_ntp_begin("UTC0"); // POSIX TZ string; set your zone for local time
}

void loop()
{
    server.handle();
}
