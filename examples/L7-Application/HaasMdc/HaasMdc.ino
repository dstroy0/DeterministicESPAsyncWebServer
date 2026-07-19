// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file HaasMdc.ino
 * @brief Haas Machine Data Collection (MDC) collector - poll a Haas CNC control's `?Q` commands over
 *        its Ethernet port and decode the framed replies (DWS_ENABLE_HAAS_MDC).
 *
 * services/haas_mdc is a pure codec for the documented Haas MDC protocol; the sketch owns the TCP
 * socket to the control (raw socket on the Setting-143 port, default 5051). Each poll:
 *
 *   ?Q100  -> serial number      ?Q102 -> model        ?Q104 -> mode
 *   ?Q500  -> program + status + parts    (two branches: PROGRAM,... vs STATUS, BUSY)
 *   ?Q600 <var> -> a macro / system variable
 *
 * A reply's payload is framed between STX (0x02) and ETB (0x17), then CR LF and a `>` prompt; the
 * codec scans for those delimiters, so read_frame just accumulates bytes through the ETB. Point
 * HAAS_IP at a control with MDC enabled (Setting 143). See README.
 *
 * Build flags (platformio.ini):  build_flags = -DDWS_ENABLE_HAAS_MDC=1
 */

#define DWS_ENABLE_HAAS_MDC 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/haas_mdc/haas_mdc.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *HAAS_IP = "192.168.1.60"; // a Haas control with MDC (Setting 143) enabled

static char c_cmd[24];

// Accumulate one framed reply: read bytes until the ETB (0x17) closes the payload, or a timeout.
static size_t read_frame(WiFiClient &sock, char *out, size_t cap)
{
    size_t o = 0;
    unsigned long deadline = millis() + 3000;
    while (o + 1 < cap && millis() < deadline)
    {
        if (!sock.available())
            continue;
        char ch = (char)sock.read();
        out[o++] = ch;
        if (ch == (char)DWS_HAAS_MDC_ETB)
            break;
    }
    out[o] = '\0';
    return o;
}

// Send a query, parse its reply into r; return false on no-frame / UNKNOWN.
static bool poll(WiFiClient &sock, size_t n, HaasMdcResp *r, const char *label)
{
    sock.write((const uint8_t *)c_cmd, n);
    char frame[192];
    size_t fl = read_frame(sock, frame, sizeof(frame));
    if (!dws_haas_mdc_parse(frame, fl, r))
    {
        Serial.printf("[haas] %s: no response\n", label);
        return false;
    }
    if (dws_haas_mdc_is_error(r))
    {
        Serial.printf("[haas] %s: UNKNOWN (unsupported)\n", label);
        return false;
    }
    return true;
}

static void query_value(WiFiClient &sock, uint16_t q, const char *label)
{
    HaasMdcResp r;
    if (!poll(sock, dws_haas_mdc_build_q(c_cmd, sizeof(c_cmd), q), &r, label))
        return;
    const char *v = nullptr;
    size_t vl = 0;
    if (dws_haas_mdc_value(&r, &v, &vl))
        Serial.printf("[haas] %s: %.*s\n", label, (int)vl, v);
}

static void run_session(IPAddress ip)
{
    WiFiClient haas;
    if (!haas.connect(ip, DWS_HAAS_MDC_TCP_PORT))
    {
        Serial.println("[haas] connect failed");
        return;
    }

    query_value(haas, HAAS_Q_SERIAL, "serial");
    query_value(haas, HAAS_Q_MODEL, "model");
    query_value(haas, HAAS_Q_MODE, "mode");

    // Q500: program + run status + parts counter (two-branch response).
    HaasMdcResp r;
    if (poll(haas, dws_haas_mdc_build_q(c_cmd, sizeof(c_cmd), HAAS_Q_PROGRAM_STATUS), &r, "status"))
    {
        HaasMdcStatus s;
        if (dws_haas_mdc_parse_status(&r, &s))
        {
            if (s.busy)
                Serial.println("[haas] status: BUSY");
            else
                Serial.printf("[haas] program %.*s  status %.*s  parts %u\n", (int)s.program_len, s.program,
                              (int)s.status_len, s.status, (unsigned)s.parts);
        }
    }

    // Q600: read macro variable #100.
    if (poll(haas, dws_haas_mdc_build_var(c_cmd, sizeof(c_cmd), 100), &r, "macro"))
    {
        uint32_t var = 0;
        const char *val = nullptr;
        size_t vl = 0;
        if (dws_haas_mdc_parse_macro(&r, &var, &val, &vl))
            Serial.printf("[haas] macro #%u = %.*s\n", (unsigned)var, (int)vl, val);
    }

    haas.stop();
    Serial.println("[haas] done");
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
        if (ip.fromString(HAAS_IP))
            run_session(ip);
        else
            Serial.println("[haas] bad HAAS_IP");
    }
    delay(10);
}
