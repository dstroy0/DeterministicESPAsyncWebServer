// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 17.Provisioning.ino
 * @brief First-boot WiFi provisioning via a captive portal (DETWS_ENABLE_PROVISIONING).
 *
 * On first boot (no stored credentials) the device starts a softAP
 * "DetWS-Setup" and a catch-all DNS responder (raw lwIP UDP - no add-on
 * library), so connecting a phone pops the credentials form. Submitted SSID/PSK
 * persist to NVS and the device reboots into station mode and serves normally.
 *
 * No external libraries: only WiFi (softAP), lwIP UDP, and Preferences (NVS).
 * To re-provision, call detws_provisioning_clear() (e.g. from a button handler).
 */

#define DETWS_ENABLE_PROVISIONING 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/provisioning_service/provisioning_service.h"
#include <WiFi.h>

DetWebServer server;

void handle_root(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/plain", "Provisioned - hello from station mode!");
}

void setup()
{
    Serial.begin(115200);

    char ssid[33];
    char psk[64];
    if (detws_provisioning_load(ssid, sizeof(ssid), psk, sizeof(psk)))
    {
        // Credentials present: connect as a normal station.
        init_wifi_physical(ssid, psk);
        Serial.print("Connecting to ");
        Serial.println(ssid);
        while (!wifi_ready())
            delay(250);
        Serial.print("\nIP: ");
        Serial.println(WiFi.localIP());

        server.on("/", HttpMethod::HTTP_GET, handle_root);
        server.begin(80);
        Serial.println("Station mode; serving on port 80");
    }
    else
    {
        // No credentials: bring up the captive portal.
        server.begin(80);
        detws_provisioning_begin(server, "DetWS-Setup");
        Serial.println("Provisioning: join WiFi 'DetWS-Setup' and open any page");
    }
}

void loop()
{
    server.handle();
}
