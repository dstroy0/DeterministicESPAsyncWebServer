// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file Vxi11.ino
 * @brief VXI-11 instrument controller - drive an instrument over the legacy LXI transport
 *        (VXI-11 / ONC RPC) carrying SCPI (DWS_ENABLE_VXI11).
 *
 * VXI-11 rides on ONC RPC (Sun RPC) with XDR over TCP. services/vxi11 is a pure codec (it builds
 * RPC calls + parses replies); the sketch owns the sockets and runs the standard session:
 *
 *   portmap GETPORT(0x0607AF/1/TCP) on port 111  -> the dynamic DEVICE_CORE port
 *   connect there, create_link("inst0")          -> a link id
 *   device_write("*IDN?\n")                       -> ask for the identity
 *   device_read()                                 -> the identity string
 *   destroy_link()
 *
 * Point INSTRUMENT_IP at a real VXI-11 / LXI instrument (or a python-vxi11 server). See the README.
 *
 * Build flags (platformio.ini):  build_flags = -DDWS_ENABLE_VXI11=1
 */

#define DWS_ENABLE_VXI11 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/vxi11/vxi11.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *INSTRUMENT_IP = "192.168.1.60"; // a VXI-11 instrument

static uint8_t c_req[256];
static uint8_t c_resp[512];

// Write one record-marked RPC call, read one single-fragment RPC reply (the bytes after the record
// mark) into c_resp. Returns the reply length, or 0 on timeout / overflow.
static size_t rpc_call(WiFiClient &sock, size_t req_len)
{
    if (req_len == 0 || sock.write(c_req, req_len) != req_len)
        return 0;
    uint8_t rm[4];
    size_t got = 0;
    unsigned long deadline = millis() + 3000;
    while (got < 4 && millis() < deadline)
        if (sock.available())
            got += sock.read(rm + got, 4 - got);
    bool last = false;
    uint32_t frag = 0;
    if (got < 4 || !dws_rpc_parse_record_mark(rm, 4, &last, &frag) || frag > sizeof(c_resp))
        return 0;
    got = 0;
    while (got < frag && millis() < deadline)
        if (sock.available())
            got += sock.read(c_resp + got, frag - got);
    return got == frag ? got : 0;
}

static void run_session(IPAddress ip)
{
    // 1) Ask the portmapper (TCP 111) for the DEVICE_CORE port.
    WiFiClient pmap;
    if (!pmap.connect(ip, DWS_RPC_PMAP_PORT))
    {
        Serial.println("[vxi11] portmap connect failed");
        return;
    }
    size_t n =
        dws_vxi11_build_getport(c_req, sizeof(c_req), 1, DWS_VXI11_CORE_PROG, DWS_VXI11_CORE_VERS, DWS_RPC_PROTO_TCP);
    uint32_t core_port = 0;
    if (!dws_vxi11_parse_getport_resp(c_resp, rpc_call(pmap, n), &core_port) || core_port == 0)
    {
        Serial.println("[vxi11] GETPORT failed");
        pmap.stop();
        return;
    }
    pmap.stop();
    Serial.printf("[vxi11] DEVICE_CORE port = %u\n", core_port);

    // 2) Open the core channel and create a link to "inst0".
    WiFiClient core;
    if (!core.connect(ip, (uint16_t)core_port))
    {
        Serial.println("[vxi11] core connect failed");
        return;
    }
    n = dws_vxi11_build_create_link(c_req, sizeof(c_req), 2, 0x44575345 /* "DWSE" */, false, 0, "inst0");
    Vxi11CreateLinkResp link;
    if (!dws_vxi11_parse_create_link_resp(c_resp, rpc_call(core, n), &link) || link.error != DWS_VXI11_ERR_NONE)
    {
        Serial.println("[vxi11] create_link failed");
        core.stop();
        return;
    }
    Serial.printf("[vxi11] link=%d maxRecv=%u\n", link.lid, link.max_recv_size);

    // 3) Write "*IDN?" (END-terminated), then read the identity back.
    n = dws_vxi11_build_device_write(c_req, sizeof(c_req), 3, link.lid, 10000, 0, DWS_VXI11_FLAG_END,
                                     (const uint8_t *)"*IDN?\n", 6);
    Vxi11WriteResp wr;
    dws_vxi11_parse_write_resp(c_resp, rpc_call(core, n), &wr);

    n = dws_vxi11_build_device_read(c_req, sizeof(c_req), 4, link.lid, 1024, 10000, 0, 0, 0);
    Vxi11ReadResp rd;
    if (dws_vxi11_parse_read_resp(c_resp, rpc_call(core, n), &rd) && rd.error == DWS_VXI11_ERR_NONE)
        Serial.printf("[vxi11] *IDN? -> %.*s\n", (int)rd.data_len, (const char *)rd.data);
    else
        Serial.println("[vxi11] device_read failed");

    // 4) Close the link.
    n = dws_vxi11_build_destroy_link(c_req, sizeof(c_req), 5, link.lid);
    int32_t err = 0;
    dws_vxi11_parse_error_resp(c_resp, rpc_call(core, n), &err);
    core.stop();
    Serial.println("[vxi11] done");
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
        delay(250);
    WiFi.setSleep(false);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    static bool done = false;
    if (!done && millis() > 2000)
    {
        done = true;
        IPAddress ip;
        if (ip.fromString(INSTRUMENT_IP))
            run_session(ip);
        else
            Serial.println("[vxi11] bad INSTRUMENT_IP");
    }
    delay(10);
}
