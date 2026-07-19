// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file HeidenhainLsv2.ino
 * @brief Heidenhain LSV/2 collector - talk to a Heidenhain TNC control over LSV/2-over-TCP and read its
 *        run state (DWS_ENABLE_LSV2).
 *
 * services/lsv2 is a pure codec for the LSV/2 telegram protocol; the sketch owns the TCP socket to the
 * control (default port 19000). Each LSV/2 telegram is a 4-byte big-endian payload-length prefix, a
 * 4-character mnemonic, then the payload; the codec frames the requests and slices the replies. A
 * session here:
 *
 *   A_LG "INSPECT"          -> gain read access (control replies T_OK)
 *   R_RI <EXEC_STATE>       -> program execution state  (reply S_RI ... or T_ER)
 *   R_RI <SELECTED_PGM>     -> currently selected program
 *   A_LO                    -> drop all access rights
 *
 * Point TNC_IP at a Heidenhain control that has the LSV/2 / DNC interface enabled. See README.
 *
 * Build flags (platformio.ini):  build_flags = -DDWS_ENABLE_LSV2=1
 */

#define DWS_ENABLE_LSV2 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/lsv2/lsv2.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *TNC_IP = "192.168.1.50"; // a Heidenhain TNC with the LSV/2 interface enabled

static uint8_t c_tx[128];
static uint8_t c_rx[512];

// Read one LSV/2 response telegram: first the 8-byte header (length prefix + mnemonic), then the
// payload-length bytes the header declares (or a timeout).
static size_t read_response(WiFiClient &sock, uint8_t *out, size_t cap)
{
    size_t got = 0;
    unsigned long deadline = millis() + 3000;
    while (got < DWS_LSV2_HEADER_LEN && millis() < deadline)
    {
        if (!sock.available())
            continue;
        int ch = sock.read();
        if (ch < 0)
            continue;
        if (got < cap)
            out[got] = (uint8_t)ch;
        got++;
    }
    if (got < DWS_LSV2_HEADER_LEN)
        return got;
    uint32_t plen = ((uint32_t)out[0] << 24) | ((uint32_t)out[1] << 16) | ((uint32_t)out[2] << 8) | out[3];
    size_t need = DWS_LSV2_HEADER_LEN + plen;
    while (got < need && got < cap && millis() < deadline)
    {
        if (!sock.available())
            continue;
        int ch = sock.read();
        if (ch < 0)
            continue;
        out[got++] = (uint8_t)ch;
    }
    return got;
}

// Send a pre-built request (tx_len bytes in c_tx), parse the reply into r; false on build / no-frame.
static bool txrx(WiFiClient &sock, size_t tx_len, Lsv2Telegram *r, const char *label)
{
    if (tx_len == 0)
    {
        Serial.printf("[lsv2] %s: request build failed\n", label);
        return false;
    }
    sock.write(c_tx, tx_len);
    size_t n = read_response(sock, c_rx, sizeof(c_rx));
    size_t consumed = 0;
    if (!dws_lsv2_parse(c_rx, n, r, &consumed))
    {
        Serial.printf("[lsv2] %s: no / short response (%u bytes)\n", label, (unsigned)n);
        return false;
    }
    return true;
}

// Read one run-info selector and print the reply (S_RI payload, or the T_ER error class/code).
static void query_run_info(WiFiClient &sock, uint16_t sel, const char *label)
{
    Lsv2Telegram r;
    if (!txrx(sock, dws_lsv2_build_run_info(c_tx, sizeof(c_tx), sel), &r, label))
        return;
    uint8_t err_class = 0, err_code = 0;
    if (dws_lsv2_error(&r, &err_class, &err_code))
    {
        Serial.printf("[lsv2] %s: error class %u code %u\n", label, (unsigned)err_class, (unsigned)err_code);
        return;
    }
    Serial.printf("[lsv2] %s: %.4s, %u payload bytes:", label, r.mnemonic, (unsigned)r.payload_len);
    for (size_t i = 0; i < r.payload_len && i < 16; i++)
        Serial.printf(" %02X", r.payload[i]);
    Serial.println();
}

static void run_session(IPAddress ip)
{
    WiFiClient tnc;
    if (!tnc.connect(ip, DWS_LSV2_TCP_PORT))
    {
        Serial.println("[lsv2] connect failed");
        return;
    }

    // gain INSPECT (read) access rights
    Lsv2Telegram r;
    if (!txrx(tnc, dws_lsv2_build_login(c_tx, sizeof(c_tx), DWS_LSV2_LOGIN_INSPECT, nullptr), &r, "login") ||
        !dws_lsv2_is_ok(&r))
    {
        Serial.println("[lsv2] login INSPECT failed");
        tnc.stop();
        return;
    }
    Serial.println("[lsv2] login INSPECT: T_OK");

    query_run_info(tnc, LSV2_RI_EXEC_STATE, "exec-state");
    query_run_info(tnc, LSV2_RI_SELECTED_PGM, "selected-pgm");
    query_run_info(tnc, LSV2_RI_OVERRIDE, "override");

    // drop all access rights
    txrx(tnc, dws_lsv2_build_logout(c_tx, sizeof(c_tx), nullptr), &r, "logout");
    tnc.stop();
    Serial.println("[lsv2] done");
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
        if (ip.fromString(TNC_IP))
            run_session(ip);
        else
            Serial.println("[lsv2] bad TNC_IP");
    }
    delay(10);
}
