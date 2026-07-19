// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file HiSlip.ino
 * @brief HiSLIP instrument controller - drive an instrument over the modern LXI transport on TCP
 *        4880 (DWS_ENABLE_HISLIP), carrying SCPI (DWS_ENABLE_SCPI).
 *
 * HiSLIP (IVI-6.1) is VXI-11's higher-throughput successor. A session uses TWO TCP connections to
 * port 4880 - a synchronous channel (the SCPI command/response stream) and an asynchronous channel
 * (out-of-band control) - correlated by a SessionID negotiated in the handshake. services/hislip is
 * a pure codec (it frames + parses messages); the sketch owns the two sockets and runs:
 *
 *   sync : Initialize            -> InitializeResponse       (learn the SessionID)
 *   async: AsyncInitialize(id)   -> AsyncInitializeResponse  (bind the second channel)
 *   sync : DataEND("*IDN?\n")    -> DataEND(<identity>)      (a SCPI query over HiSLIP)
 *
 * Point INSTRUMENT_IP at a real HiSLIP instrument or a simulator (e.g. python `pyvisa` with a
 * HiSLIP server, or the `PyHiSLIP` reference). See the README.
 *
 * Build flags (platformio.ini):  build_flags = -DDWS_ENABLE_HISLIP=1
 */

#define DWS_ENABLE_HISLIP 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/hislip/hislip.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *INSTRUMENT_IP = "192.168.1.60"; // a HiSLIP instrument on port 4880

static uint8_t c_buf[128];
static uint8_t c_resp[512];

// Read a full HiSLIP message (the 16-byte header + its payload) into c_resp. Returns true on a
// complete message; fills @p h with the parsed header (payload starts at c_resp + 16).
static bool hislip_recv(WiFiClient &sock, HislipHeader *h)
{
    size_t got = 0;
    unsigned long deadline = millis() + 3000;
    while (got < DWS_HISLIP_HEADER_LEN && millis() < deadline)
        if (sock.available())
            got += sock.read(c_resp + got, DWS_HISLIP_HEADER_LEN - got);
    if (got < DWS_HISLIP_HEADER_LEN || !dws_hislip_parse_header(c_resp, got, h))
        return false;
    size_t total = DWS_HISLIP_HEADER_LEN + (size_t)h->payload_len;
    if (total > sizeof(c_resp))
        return false;
    while (got < total && millis() < deadline)
        if (sock.available())
            got += sock.read(c_resp + got, total - got);
    return got == total;
}

static void run_session(IPAddress ip)
{
    WiFiClient sync_ch, async_ch;
    if (!sync_ch.connect(ip, DWS_HISLIP_PORT))
    {
        Serial.println("[hislip] sync connect failed");
        return;
    }

    // 1) Initialize on the sync channel (offer v1.1, vendor "DW", sub-address "hislip0").
    size_t n = dws_hislip_build_initialize(c_buf, sizeof(c_buf), DWS_HISLIP_VERSION_1_1, 0x4457, "hislip0");
    sync_ch.write(c_buf, n);
    HislipHeader h;
    HislipInitializeResponse ir;
    if (!hislip_recv(sync_ch, &h) || !dws_hislip_parse_initialize_response(c_resp, DWS_HISLIP_HEADER_LEN, &ir))
    {
        Serial.println("[hislip] no InitializeResponse");
        return;
    }
    Serial.printf("[hislip] session=%u server-version=0x%04X overlap=%d\n", ir.session_id, ir.protocol_version,
                  ir.overlap);

    // 2) Bind the async channel with the negotiated SessionID.
    if (async_ch.connect(ip, DWS_HISLIP_PORT))
    {
        n = dws_hislip_build_async_initialize(c_buf, sizeof(c_buf), ir.session_id);
        async_ch.write(c_buf, n);
        hislip_recv(async_ch, &h); // AsyncInitializeResponse (server vendor id in h.parameter)
    }

    // 3) Send "*IDN?" as a DataEND on the sync channel, read the identity back.
    uint32_t msg_id = DWS_HISLIP_MESSAGE_ID_INIT;
    n = dws_hislip_build_data(c_buf, sizeof(c_buf), true, 0, msg_id, (const uint8_t *)"*IDN?\n", 6);
    sync_ch.write(c_buf, n);
    msg_id = dws_hislip_next_message_id(msg_id);
    if (hislip_recv(sync_ch, &h) && (h.type == HislipMsg::DATA_END || h.type == HislipMsg::DATA))
        Serial.printf("[hislip] *IDN? -> %.*s\n", (int)h.payload_len, (const char *)(c_resp + DWS_HISLIP_HEADER_LEN));
    else
        Serial.println("[hislip] no *IDN? response");

    async_ch.stop();
    sync_ch.stop();
    Serial.println("[hislip] done");
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
            Serial.println("[hislip] bad INSTRUMENT_IP");
    }
    delay(10);
}
