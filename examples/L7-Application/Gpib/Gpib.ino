// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file Gpib.ino
 * @brief GPIB-over-LAN controller - drive a legacy IEEE-488 instrument through a Prologix
 *        GPIB-Ethernet adapter (DWS_ENABLE_GPIB), carrying SCPI.
 *
 * services/gpib is a pure codec for the Prologix `++` command set; the sketch owns the socket to
 * the adapter (raw TCP 1234). A line starting with `++` is a controller command; anything else is
 * data forwarded over GPIB to the addressed instrument. The session:
 *
 *   ++mode 1      (controller)     ++addr <N>   (the instrument's GPIB address)
 *   ++eos 2 (LF)  ++eoi 1  ++auto 0
 *   "*IDN?"  (escaped data)        ++read eoi   -> the identity string
 *   ++ver                         -> the adapter version
 *
 * Point ADAPTER_IP at a Prologix GPIB-Ethernet controller (or an AR488-Ethernet build). See README.
 *
 * Build flags (platformio.ini):  build_flags = -DDWS_ENABLE_GPIB=1
 */

#define DWS_ENABLE_GPIB 1

#include "dwserver.h" // library entry header (also sets the src/ include root)
#include "services/gpib/gpib.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *ADAPTER_IP = "192.168.1.70"; // a Prologix GPIB-Ethernet adapter
static const uint8_t INSTRUMENT_ADDR = 9;       // the instrument's primary GPIB address

static char c_cmd[64];
static uint8_t c_data[96];

// Read one newline-terminated line into out (CR stripped); return its length (0 on timeout).
static size_t read_line(WiFiClient &sock, char *out, size_t cap)
{
    size_t o = 0;
    unsigned long deadline = millis() + 3000;
    while (o + 1 < cap && millis() < deadline)
    {
        if (!sock.available())
            continue;
        char ch = (char)sock.read();
        if (ch == '\n')
            break;
        if (ch != '\r')
            out[o++] = ch;
    }
    out[o] = '\0';
    return o;
}

static void run_session(IPAddress ip)
{
    WiFiClient adapter;
    if (!adapter.connect(ip, DWS_GPIB_PORT))
    {
        Serial.println("[gpib] connect failed");
        return;
    }

    // Configure the adapter as the controller-in-charge, targeting the instrument.
    adapter.write((const uint8_t *)c_cmd, dws_gpib_command(c_cmd, sizeof(c_cmd), "mode 1"));
    adapter.write((const uint8_t *)c_cmd, dws_gpib_addr(c_cmd, sizeof(c_cmd), INSTRUMENT_ADDR, -1));
    adapter.write((const uint8_t *)c_cmd, dws_gpib_eos(c_cmd, sizeof(c_cmd), GpibEos::LF));
    adapter.write((const uint8_t *)c_cmd, dws_gpib_command(c_cmd, sizeof(c_cmd), "eoi 1"));
    adapter.write((const uint8_t *)c_cmd, dws_gpib_command(c_cmd, sizeof(c_cmd), "auto 0"));

    // Send "*IDN?" as (escaped) data, then request a read until EOI.
    adapter.write(c_data, dws_gpib_build_data(c_data, sizeof(c_data), (const uint8_t *)"*IDN?", 5));
    adapter.write((const uint8_t *)c_cmd, dws_gpib_read(c_cmd, sizeof(c_cmd), GpibRead::UNTIL_EOI, 0));

    char resp[160];
    size_t r = read_line(adapter, resp, sizeof(resp));
    if (r)
        Serial.printf("[gpib] *IDN? -> %s\n", resp);
    else
        Serial.println("[gpib] no *IDN? response");

    // Query the adapter's own version.
    adapter.write((const uint8_t *)c_cmd, dws_gpib_command(c_cmd, sizeof(c_cmd), "ver"));
    r = read_line(adapter, resp, sizeof(resp));
    const char *ver = nullptr;
    size_t vlen = 0;
    if (dws_gpib_parse_version(resp, r, &ver, &vlen))
        Serial.printf("[gpib] adapter version %.*s\n", (int)vlen, ver);

    adapter.stop();
    Serial.println("[gpib] done");
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
        if (ip.fromString(ADAPTER_IP))
            run_session(ip);
        else
            Serial.println("[gpib] bad ADAPTER_IP");
    }
    delay(10);
}
