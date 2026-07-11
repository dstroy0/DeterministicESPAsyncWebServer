// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 61.Rtc.ino
 * @brief Keep accurate time with a DS3231/DS1307 RTC module (DETWS_ENABLE_RTC).
 *
 * A real-time-clock chip has its own coin-cell battery, so it keeps time even when the ESP32
 * is unplugged - and needs no network. This reads it over I2C and plugs it into the
 * time-source chain, so `detws_time_now()` has the correct time the instant the board boots,
 * offline. It also self-initializes: the first time the board reaches the internet it sets the
 * RTC from NTP, so from then on the RTC is the source of truth even with no network.
 *
 * Together with the GPS + NTP fallback (example 58), this is the middle of the chain:
 * GPS (best) -> RTC (offline, battery-backed) -> upstream NTP. Feed it to the NTP server to
 * hand accurate time to your whole LAN.
 *
 * Wiring (I2C): module SDA -> GPIO 21, SCL -> GPIO 22, VCC -> 3V3, GND -> GND (ESP32 default
 * I2C pins; change with Wire.begin(sda, scl) if yours differ).
 *
 * Build flags (PlatformIO): `-DDETWS_ENABLE_RTC=1 -DDETWS_ENABLE_TIME_SOURCE=1 -DDETWS_ENABLE_NTP=1`
 */

#define DETWS_ENABLE_RTC 1
#define DETWS_ENABLE_TIME_SOURCE 1
#define DETWS_ENABLE_NTP 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/ntp_service/ntp_service.h"
#include "services/rtc/rtc.h"
#include "services/time_source/time_source.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// Fallback used only to *set* the RTC the first time we reach the internet.
static uint32_t ntp_source()
{
    return detws_ntp_synced() ? (uint32_t)detws_ntp_epoch() : 0;
}

void setup()
{
    Serial.begin(115200);
    rtc_begin();

    // The RTC gives us time immediately - before WiFi, before anything - if it is set.
    uint32_t boot = rtc_read_epoch();
    Serial.printf("RTC at boot: %lu %s\n", (unsigned long)boot, boot ? "(battery-backed time!)" : "(not set yet)");

    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    // The RTC is the primary source; upstream NTP is only for setting it.
    detws_time_source_add("rtc", 1, rtc_time_source);
    detws_time_source_add("ntp", 2, ntp_source);
    detws_ntp_begin();
}

void loop()
{
    // Self-initialize: once we have accurate internet time and the RTC is unset/wrong, write it.
    static bool rtc_set = false;
    if (!rtc_set && detws_ntp_synced())
    {
        uint32_t rtc_now = rtc_read_epoch();
        uint32_t ntp_now = (uint32_t)detws_ntp_epoch();
        if (rtc_now == 0 || (ntp_now > rtc_now ? ntp_now - rtc_now : rtc_now - ntp_now) > 5)
        {
            if (rtc_set_epoch(ntp_now))
                Serial.printf("RTC set from NTP: %lu\n", (unsigned long)ntp_now);
        }
        rtc_set = true;
    }

    static uint32_t last = 0;
    if (millis() - last > 5000)
    {
        last = millis();
        Serial.printf("[time] now=%lu source=%s\n", (unsigned long)detws_time_now(),
                      detws_time_source_active() ? detws_time_source_active() : "none");
    }
}
