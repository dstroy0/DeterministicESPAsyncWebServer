// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 84.OpcUa.ino
 * @brief OPC UA Binary server - handshake + SecureChannel + Session + Read (DETWS_ENABLE_OPCUA).
 *
 * Opens an OPC UA endpoint on TCP/4840. Implements the OPC UA Binary type codec,
 * UA-TCP (UACP) framing, the Hello/Acknowledge handshake, the SecureChannel
 * (OpenSecureChannel, SecurityPolicy None), the Session (CreateSession +
 * ActivateSession), and the Read service - so a client (UaExpert, open62541,
 * Python asyncua, ...) completes the handshake, opens a secure channel, activates
 * a session, and Reads node values via a registered resolver. Browse and the Close
 * calls are later increments.
 *
 *   listen(4840, PROTO_OPCUA)  -> HEL/ACK, OPN, CreateSession, ActivateSession, Read
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
#include "services/opcua/opcua.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Read resolver: map a tiny address space (namespace 1) onto live values. A client
// Reads node ns=1;i=1 (uptime), i=2 (free heap) or i=3 (a Double). Return false for
// anything else and the server answers BadNodeIdUnknown for that node.
static bool opcua_read(uint16_t ns, uint32_t id, uint32_t attribute, OpcUaVariant *out)
{
    if (ns != 1 || attribute != OPCUA_ATTR_VALUE)
        return false;
    switch (id)
    {
    case 1:
        out->type = OPCUA_VAR_UINT32;
        out->u32 = millis() / 1000;
        return true;
    case 2:
        out->type = OPCUA_VAR_UINT32;
        out->u32 = ESP.getFreeHeap();
        return true;
    case 3:
        out->type = OPCUA_VAR_DOUBLE;
        out->f64 = 23.5;
        return true;
    default:
        return false;
    }
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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "OPC UA on :4840"); });
    opcua_set_read_handler(opcua_read); // serve Reads for ns=1;i=1..3
    server.listen(4840, PROTO_OPCUA);   // OPC UA Binary endpoint - before begin() (it activates listeners)
    server.begin(80);
    Serial.println("OPC UA endpoint: opc.tcp://<ip>:4840 (handshake + SecureChannel + Session + Read)");
}

void loop()
{
    server.handle();
}
