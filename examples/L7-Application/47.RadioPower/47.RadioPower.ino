// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 47.RadioPower.ino
 * @brief WiFi radio power controls (DETWS_ENABLE_RADIO_POWER).
 *
 * Applies a WiFi modem-sleep mode (and an optional max-TX cap) after the link is
 * up, to lower average power on a battery device at the cost of some latency.
 * GET /radio reports the mode read back from the radio.
 *
 * NOTE: set the mode via build flags so it reaches the separately-compiled library:
 *     build_flags = -DDETWS_ENABLE_RADIO_POWER=1 -DDETWS_RADIO_WIFI_PS=1
 *   (0 = none, 1 = min modem, 2 = max modem; + optional -DDETWS_RADIO_MAX_TX_DBM=11)
 * (Arduino IDE: they are already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DETWS_ENABLE_RADIO_POWER 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/radio_power/radio_power.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Apply the configured modem-sleep / TX settings AFTER the link is up (the
    // WiFi connect path may set its own default first).
    detws_radio_power_apply();
    Serial.printf("radio modem-sleep: %s\n", detws_radio_ps_name(detws_radio_ps_get()));

    server.on("/radio", HTTP_GET, [](uint8_t id, HttpReq *) {
        char b[48];
        snprintf(b, sizeof(b), "{\"modem_sleep\":\"%s\"}", detws_radio_ps_name(detws_radio_ps_get()));
        server.send(id, 200, "application/json", b);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
