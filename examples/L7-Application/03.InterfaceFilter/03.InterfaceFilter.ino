// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 03.InterfaceFilter.ino
 * @brief Gate routes to the station or softAP interface (ON_STA / ON_AP style).
 *
 * Brings up WiFi in AP+STA mode and registers the same kinds of routes on
 * different interfaces: a setup/config page visible only on the softAP, and an
 * app API visible only on the station link. The interface is determined by
 * comparing each connection's local IP to the softAP IP, so call
 * set_ap_ip(WiFi.softAPIP()) once after starting the AP.
 *
 * Test from a station client (your LAN):    curl http://<sta-ip>/api/data   -> 200
 *                                            curl http://<sta-ip>/setup      -> 404
 * Test from a softAP client (join the AP):  curl http://192.168.4.1/setup   -> 200
 *                                            curl http://192.168.4.1/api/data-> 404
 */

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";
static const char *AP_SSID = "DetWS-Setup";
static const char *AP_PASS = "configme123";

DetWebServer server;

void handle_setup(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/html", "<h1>Setup</h1><p>softAP only</p>");
}

void handle_api(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "application/json", "{\"data\":42,\"iface\":\"sta\"}");
}

void handle_root(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send(slot_id, 200, "text/plain", "available on both interfaces");
}

void setup()
{
    Serial.begin(115200);

    // AP + STA so both interfaces exist simultaneously.
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nSTA IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP  IP: ");
    Serial.println(WiFi.softAPIP());

    WiFi.setSleep(false);

    // Required for STA/AP classification (IPAddress converts to uint32_t).
    server.set_ap_ip(WiFi.softAPIP());

    server.on("/setup", HTTP_GET, handle_setup, DETIFACE_AP);   // softAP only
    server.on("/api/data", HTTP_GET, handle_api, DETIFACE_STA); // station only
    server.on("/", HTTP_GET, handle_root);                      // any interface

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
}
