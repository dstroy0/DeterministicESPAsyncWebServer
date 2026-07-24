// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file UbloxGnss.ino
 * @brief Read a u-blox GNSS receiver as NMEA + UBX (DWS_ENABLE_NMEA0183 + DWS_ENABLE_UBX).
 *
 * A u-blox module (GT-U7 / NEO-6/7/8/M8) streams ASCII NMEA sentences AND binary UBX frames on the
 * same UART. This sketch reads BOTH: `dws_ubx_stream_feed` demultiplexes the byte stream - it pulls
 * complete, checksum-valid UBX frames out and hands every other byte to a plain NMEA line assembler.
 * It also SENDS UBX: at boot it polls UBX-MON-VER (the receiver's version) and enables UBX-NAV-PVT
 * (the single-frame position/velocity/time fix), then decodes the version reply, the config ACK, the
 * NAV-PVT fix, and any NMEA GGA/RMC.
 *
 * "Accept and send" in one link: config/polls go out as UBX; fixes come back as UBX or NMEA.
 *
 * Wiring (UART): module TX -> ESP32 GPIO 16 (RX), module RX -> ESP32 GPIO 17 (TX), VCC -> 3V3,
 * GND -> GND, PPS -> GPIO 4 (optional). 9600 baud 8N1. Change the pins below for your board.
 *
 * Build flags (PlatformIO): `-DDWS_ENABLE_NMEA0183=1 -DDWS_ENABLE_UBX=1`
 */

#define DWS_ENABLE_NMEA0183 1
#define DWS_ENABLE_UBX 1

#include "dwserver.h" // declares the library dependency (Arduino build)
#include "services/nmea0183/nmea0183.h"
#include "services/ubx/ubx.h"

static const int GNSS_RX = 16; // ESP32 RX <- module TX
static const int GNSS_TX = 17; // ESP32 TX -> module RX
static const int GNSS_PPS = 4; // 1 Hz pulse-per-second (optional)
static const uint32_t GNSS_BAUD = 9600;

static HardwareSerial GNSS(1);
static DwsUbxStream ubx;

static char line[128]; // NMEA line assembler, fed the demux passthrough bytes
static size_t ln = 0;

static volatile uint32_t pps = 0;
static void IRAM_ATTR on_pps()
{
    pps++;
}

// -- send: build a UBX frame and write it to the receiver --

static void poll_mon_ver()
{
    uint8_t f[8];
    size_t n = dws_ubx_build_poll(f, sizeof(f), 0x0A, 0x04); // UBX-MON-VER
    GNSS.write(f, n);
}

static void enable_nav_pvt()
{
    // UBX-CFG-MSG: set UBX-NAV-PVT (class 0x01 id 0x07) rate to 1 (every nav solution).
    const uint8_t pay[] = {0x01, 0x07, 0x01};
    uint8_t f[16];
    size_t n = dws_ubx_build(f, sizeof(f), 0x06, 0x01, pay, sizeof(pay));
    GNSS.write(f, n);
}

// -- accept: NMEA --

#define FLEN(m, i) ((i) < (m).field_count ? (int)(m).field_len[i] : 0)
#define FPTR(m, i) ((i) < (m).field_count ? (m).fields[i] : "")

static void handle_nmea(const char *s, size_t n)
{
    Nmea0183 m;
    if (!dws_nmea0183_parse(s, n, &m))
        return;
    if (memcmp(m.type, "GGA", 3) == 0)
    {
        long q = 0, sats = 0;
        dws_nmea0183_field_int(&m, 6, &q);
        dws_nmea0183_field_int(&m, 7, &sats);
        Serial.printf("NMEA GGA fix=%ld sats=%ld lat=%.*s%.*s lon=%.*s%.*s\n", q, sats, FLEN(m, 2), FPTR(m, 2),
                      FLEN(m, 3), FPTR(m, 3), FLEN(m, 4), FPTR(m, 4), FLEN(m, 5), FPTR(m, 5));
    }
}

// -- accept: UBX --

static void handle_ubx(const DwsUbx *u)
{
    uint8_t ac = 0, ai = 0;
    int ack = dws_ubx_ack(u, &ac, &ai);
    if (ack >= 0)
    {
        Serial.printf("UBX ACK-%s for %02X %02X\n", ack ? "ACK" : "NAK", ac, ai);
        return;
    }
    if (u->cls == 0x0A && u->id == 0x04) // MON-VER: swVersion is a 30-byte NUL-padded string
    {
        char sw[31];
        size_t k = 0;
        for (; k < 30 && k < u->len && u->payload[k]; k++)
            sw[k] = (char)u->payload[k];
        sw[k] = 0;
        Serial.printf("UBX MON-VER sw=\"%s\"\n", sw);
        return;
    }
    if (u->cls == 0x01 && u->id == 0x07 && u->len >= 40) // NAV-PVT (fields used are within 40 B)
    {
        uint8_t fix = u->payload[20];
        uint8_t sats = u->payload[23];
        int32_t lon = dws_ubx_i32(u->payload, 24);  // 1e-7 deg
        int32_t lat = dws_ubx_i32(u->payload, 28);  // 1e-7 deg
        int32_t hmsl = dws_ubx_i32(u->payload, 36); // mm
        Serial.printf("UBX NAV-PVT fix=%u sats=%u lat=%.7f lon=%.7f alt=%.2fm pps=%lu\n", fix, sats, lat * 1e-7,
                      lon * 1e-7, hmsl / 1000.0, (unsigned long)pps);
        return;
    }
    Serial.printf("UBX %02X %02X len=%u\n", u->cls, u->id, (unsigned)u->len);
}

void setup()
{
    Serial.begin(115200);
    delay(400);
    Serial.println("\n=== DWS u-blox GNSS (NMEA + UBX) ===");
    GNSS.begin(GNSS_BAUD, SERIAL_8N1, GNSS_RX, GNSS_TX);
    pinMode(GNSS_PPS, INPUT);
    attachInterrupt(GNSS_PPS, on_pps, RISING);
    dws_ubx_stream_init(&ubx);

    delay(250);
    poll_mon_ver();   // send: ask the receiver its version
    enable_nav_pvt(); // send: turn on the binary PVT fix
    Serial.println("sent: UBX-MON-VER poll + UBX-NAV-PVT enable");
}

void loop()
{
    while (GNSS.available())
    {
        uint8_t b = (uint8_t)GNSS.read();
        DwsUbx u;
        uint8_t pass = 0;
        int r = dws_ubx_stream_feed(&ubx, b, &u, &pass);
        if (r == DWS_UBX_FRAME)
        {
            handle_ubx(&u);
        }
        else if (r == DWS_UBX_PASSTHROUGH)
        {
            if (pass == '\n' || pass == '\r')
            {
                if (ln > 0)
                {
                    line[ln] = 0;
                    handle_nmea(line, ln);
                    ln = 0;
                }
            }
            else if (ln < sizeof(line) - 1)
            {
                line[ln++] = (char)pass;
            }
        }
    }
}
