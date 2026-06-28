// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 85.OpcUaClient.ino
 * @brief OPC UA Binary client - drive a server over TCP (DETWS_ENABLE_OPCUA_CLIENT).
 *
 * services/opcua_client builds OPC UA requests and parses responses; it is
 * transport-agnostic, so the app owns the socket. This sketch runs the OPC UA
 * server (services/opcua) on :4840 AND, once connected, opens a plain Arduino
 * WiFiClient to its own address and walks the full client sequence against it:
 *
 *   Hello/Ack -> OpenSecureChannel -> CreateSession -> ActivateSession
 *             -> Browse(Objects) -> Read(values) -> CloseSession -> CloseSecureChannel
 *
 * The results are printed over Serial. Point SERVER_HOST at any OPC UA server to use
 * the client on its own; the bundled server just makes the example self-contained.
 *
 * Enable both flags for the build (platformio.ini):
 *     build_flags = -DDETWS_ENABLE_OPCUA=1 -DDETWS_ENABLE_OPCUA_CLIENT=1
 */

#define DETWS_ENABLE_OPCUA 1
#define DETWS_ENABLE_OPCUA_CLIENT 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/opcua/opcua.h"
#include "services/opcua_client/opcua_client.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// --- server side: a tiny address space (ns=1;i=1..2) under the Objects folder ---
static bool srv_read(uint16_t ns, uint32_t id, uint32_t attr, OpcUaVariant *out)
{
    if (ns != 1 || attr != OPCUA_ATTR_VALUE)
        return false;
    if (id == 1)
    {
        out->type = OPCUA_VAR_UINT32;
        out->u32 = millis() / 1000;
        return true;
    }
    if (id == 2)
    {
        out->type = OPCUA_VAR_DOUBLE;
        out->f64 = 23.5;
        return true;
    }
    return false;
}

static int32_t srv_browse(uint16_t ns, uint32_t id, OpcUaReference *out, uint32_t max)
{
    if (ns != 0 || id != 85)
        return -1;
    static const char *names[2] = {"Uptime", "Temperature"};
    int32_t n = 0;
    for (uint32_t i = 0; i < 2 && (uint32_t)n < max; i++, n++)
    {
        out[n].ref_type_id = OPCUA_REFTYPE_ORGANIZES;
        out[n].is_forward = true;
        out[n].target_ns = 1;
        out[n].target_id = i + 1;
        out[n].browse_name_ns = 1;
        out[n].browse_name = names[i];
        out[n].display_name = names[i];
        out[n].node_class = OPCUA_NODECLASS_VARIABLE;
        out[n].type_def_id = OPCUA_TYPEDEF_BASE_DATA_VARIABLE;
    }
    return n;
}

// --- client side: send one framed message, read one framed reply over WiFiClient ---
static uint8_t c_req[512];
static uint8_t c_resp[2048];

static size_t exchange(WiFiClient &sock, size_t reqlen)
{
    if (reqlen == 0 || sock.write(c_req, reqlen) != reqlen)
        return 0;
    // Read the 8-byte UACP header, then the rest of MessageSize.
    size_t got = 0;
    uint32_t deadline = millis() + 3000;
    while (got < 8 && millis() < deadline)
        if (sock.available())
            got += sock.read(c_resp + got, 8 - got);
    if (got < 8)
        return 0;
    uint32_t size =
        (uint32_t)c_resp[4] | ((uint32_t)c_resp[5] << 8) | ((uint32_t)c_resp[6] << 16) | ((uint32_t)c_resp[7] << 24);
    if (size < 8 || size > sizeof(c_resp))
        return 0;
    while (got < size && millis() < deadline)
        if (sock.available())
            got += sock.read(c_resp + got, size - got);
    return got == size ? size : 0;
}

static void run_client(IPAddress ip)
{
    WiFiClient sock;
    if (!sock.connect(ip, 4840))
    {
        Serial.println("[opcua-client] connect failed");
        return;
    }
    OpcUaClient c;
    opcua_client_init(&c);
    OpcUaAckInfo ack;
    size_t n;

    n = exchange(sock, opcua_client_hello("opc.tcp://self:4840", c_req, sizeof(c_req)));
    if (!n || !opcua_client_on_ack(c_resp, n, &ack))
    {
        Serial.println("[opcua-client] HELLO/ACK failed");
        return;
    }
    n = exchange(sock, opcua_client_open(&c, c_req, sizeof(c_req)));
    if (!n || !opcua_client_on_open(&c, c_resp, n))
    {
        Serial.println("[opcua-client] OpenSecureChannel failed");
        return;
    }
    n = exchange(sock, opcua_client_get_endpoints(&c, "opc.tcp://self:4840", c_req, sizeof(c_req)));
    Serial.printf("[opcua-client] GetEndpoints -> %d endpoint(s)\n",
                  n ? (int)opcua_client_on_get_endpoints(c_resp, n) : -1);
    n = exchange(sock, opcua_client_create_session(&c, "esp32", "opc.tcp://self:4840", c_req, sizeof(c_req)));
    if (!n || !opcua_client_on_create_session(&c, c_resp, n))
    {
        Serial.println("[opcua-client] CreateSession failed");
        return;
    }
    n = exchange(sock, opcua_client_activate_session(&c, c_req, sizeof(c_req)));
    if (!n || !opcua_client_on_activate_session(c_resp, n))
    {
        Serial.println("[opcua-client] ActivateSession failed");
        return;
    }
    Serial.printf("[opcua-client] session active (channel=%u token=%u)\n", c.channel_id, c.token_id);

    // Browse the Objects folder.
    n = exchange(sock, opcua_client_browse(&c, 0, 85, c_req, sizeof(c_req)));
    OpcUaClientRef refs[4];
    int32_t nrefs = n ? opcua_client_on_browse(c_resp, n, refs, 4) : -1;
    for (int32_t i = 0; i < nrefs; i++)
        Serial.printf("[opcua-client] browse: %s -> ns%u;i=%u\n", refs[i].browse_name, refs[i].target_ns,
                      refs[i].target_id);

    // Read the two variables.
    OpcUaReadItem items[2] = {{1, 1, true, OPCUA_ATTR_VALUE}, {1, 2, true, OPCUA_ATTR_VALUE}};
    n = exchange(sock, opcua_client_read(&c, items, 2, c_req, sizeof(c_req)));
    OpcUaVariant vals[2];
    uint32_t sts[2];
    int32_t nv = n ? opcua_client_on_read(c_resp, n, vals, sts, 2) : -1;
    if (nv == 2)
        Serial.printf("[opcua-client] read: Uptime=%u Temperature=%.1f\n", vals[0].u32, vals[1].f64);

    exchange(sock, opcua_client_close_session(&c, c_req, sizeof(c_req)));
    sock.write(c_req, opcua_client_close_channel(c_req, sizeof(c_req)));
    sock.stop();
    Serial.println("[opcua-client] done");
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

    static char url[48];
    snprintf(url, sizeof(url), "opc.tcp://%s:4840", WiFi.localIP().toString().c_str());
    opcua_set_endpoint_url(url); // advertised in GetEndpoints / CreateSession
    opcua_set_read_handler(srv_read);
    opcua_set_browse_handler(srv_browse);
    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/plain", "OPC UA client demo"); });
    server.listen(4840, PROTO_OPCUA); // server endpoint, before begin()
    server.begin(80);
}

void loop()
{
    server.handle();

    // Run the client exchange once, a few seconds after boot (server is up by then).
    static bool done = false;
    if (!done && millis() > 4000)
    {
        done = true;
        run_client(WiFi.localIP());
    }
}
