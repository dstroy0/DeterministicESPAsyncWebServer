// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file Scpi.ino
 * @brief SCPI / IEEE 488.2 instrument controller - drive a bench instrument over TCP 5025
 *        (DWS_ENABLE_SCPI).
 *
 * services/scpi is a transport-agnostic codec: it builds command lines and parses replies; the
 * app owns the socket. This sketch opens a plain Arduino WiFiClient to an instrument's raw SCPI
 * socket (port 5025 - DMMs, scopes, supplies, function generators, SMUs, loads all speak it) and
 * runs a small session:
 *
 *   *IDN?            -> manufacturer, model, serial, firmware (4 comma-separated fields)
 *   *CLS             -> clear the status byte + error queue
 *   MEAS:VOLT:DC?    -> a DC voltage reading, parsed as a number
 *   SYST:ERR?        -> pop the instrument's error/event queue (0,"No error" when clean)
 *
 * The parsed values print over Serial. Point INSTRUMENT_IP at a real instrument, a Python
 * `pyvisa` server, or any SCPI simulator listening on port 5025. See the README to fake one with
 * a two-line `socat` / netcat responder for a dry run.
 *
 * Build flag (platformio.ini):  build_flags = -DDWS_ENABLE_SCPI=1
 */

#define DWS_ENABLE_SCPI 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/scpi/scpi.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// --- the target instrument ---
static const char *INSTRUMENT_IP = "192.168.1.60"; // its raw SCPI socket host
// DWS_SCPI_PORT (5025) is the de-facto raw-socket port; override if your instrument differs.

static char c_cmd[128];
static char c_resp[256];

// Send one command, read one newline-terminated response line. Returns the response length
// (excluding the newline / NUL), or 0 on timeout. @p cmd must already be newline-terminated
// (dws_scpi_build does this).
static size_t scpi_exchange(WiFiClient &sock, const char *cmd, size_t cmd_len)
{
    if (cmd_len == 0 || sock.write((const uint8_t *)cmd, cmd_len) != cmd_len)
        return 0;
    size_t got = 0;
    unsigned long deadline = millis() + 3000;
    while (got < sizeof(c_resp) - 1 && millis() < deadline)
    {
        if (!sock.available())
            continue;
        char ch = (char)sock.read();
        if (ch == '\n')
            break;
        if (ch != '\r')
            c_resp[got++] = ch;
    }
    c_resp[got] = '\0';
    return got;
}

static void run_session(IPAddress ip)
{
    WiFiClient sock;
    if (!sock.connect(ip, DWS_SCPI_PORT))
    {
        Serial.println("[scpi] connect failed");
        return;
    }

    size_t n;

    // 1) *IDN? - identify the instrument (4 comma-separated fields).
    n = dws_scpi_build(c_cmd, sizeof(c_cmd), dws_scpi_common(ScpiCommon::SCPI_IDN_Q), nullptr, 0);
    if (scpi_exchange(sock, c_cmd, n))
        Serial.printf("[scpi] *IDN? -> %s\n", c_resp);
    else
        Serial.println("[scpi] *IDN? no reply");

    // 2) *CLS - clear status byte + error queue (no response).
    n = dws_scpi_build(c_cmd, sizeof(c_cmd), dws_scpi_common(ScpiCommon::SCPI_CLS), nullptr, 0);
    sock.write((const uint8_t *)c_cmd, n);

    // 3) MEAS:VOLT:DC? - take a DC voltage reading and parse it as a number.
    n = dws_scpi_build(c_cmd, sizeof(c_cmd), "MEASure:VOLTage:DC?", nullptr, 0);
    if (scpi_exchange(sock, c_cmd, n))
    {
        double volts = 0.0;
        if (dws_scpi_parse_number(c_resp, strlen(c_resp), &volts))
            Serial.printf("[scpi] DC voltage = %.6f V\n", volts);
        else
            Serial.printf("[scpi] unparseable reading: %s\n", c_resp);
    }

    // 4) SYST:ERR? - read one entry off the instrument's error/event queue.
    n = dws_scpi_build(c_cmd, sizeof(c_cmd), "SYSTem:ERRor?", nullptr, 0);
    if (scpi_exchange(sock, c_cmd, n))
        Serial.printf("[scpi] SYST:ERR? -> %s\n", c_resp);

    sock.stop();
    Serial.println("[scpi] done");
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
            Serial.println("[scpi] bad INSTRUMENT_IP");
    }
    delay(10);
}
