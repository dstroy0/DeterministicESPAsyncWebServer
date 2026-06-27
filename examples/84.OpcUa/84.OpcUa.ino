// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 84.OpcUa.ino
 * @brief OPC UA Binary server, increment 1 - UA-TCP handshake (DETWS_ENABLE_OPCUA).
 *
 * Opens an OPC UA endpoint on TCP/4840. Increment 1 implements the OPC UA Binary
 * type codec, UA-TCP (UACP) framing, and the Hello/Acknowledge handshake, so a
 * client (UaExpert, open62541, Python asyncua, ...) completes the transport
 * handshake. SecureChannel / Session / Read are later increments.
 *
 *   listen(4840, PROTO_OPCUA)  -> client HEL is answered with ACK
 *
 * The HTTP server on :80 runs alongside, sharing the same connection pool and
 * event loop - OPC UA is just another protocol on its own port.
 *
 * NOTE: enable it for the whole build. In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_OPCUA=1
 * (Arduino IDE: set it in DetWebServerConfig.h.)
 */

#define DETWS_ENABLE_OPCUA 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
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
    WiFi.setSleep(false);

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "OPC UA on :4840"); });
    server.listen(4840, PROTO_OPCUA); // OPC UA Binary endpoint - before begin() (it activates listeners)
    server.begin(80);
    Serial.println("OPC UA endpoint: opc.tcp://<ip>:4840 (Hello/Acknowledge handshake)");
}

void loop()
{
    server.handle();
}
