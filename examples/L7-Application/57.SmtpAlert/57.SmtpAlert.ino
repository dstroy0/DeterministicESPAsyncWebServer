// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 57.SmtpAlert.ino
 * @brief Send an email alert from the ESP32 with the SMTP client (DWS_ENABLE_SMTP).
 *
 * At boot the board joins WiFi and emails one short message to a mail server, then
 * prints whether it worked. The README beside this sketch walks you - from scratch -
 * through standing up a Postfix mail server to receive it (no prior mail-server
 * experience needed).
 *
 * Edit the six lines marked "CHANGE ME" below, flash, and open Serial @ 115200.
 *
 * NOTE (PlatformIO): SMTP is compiled into the *library*, so the flag must reach the
 * whole build: `build_flags = -DDWS_ENABLE_SMTP=1`. In the Arduino IDE it is already
 * set for you in build_opt.h beside this sketch.
 */

#define DWS_ENABLE_SMTP 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/smtp/smtp.h"
#include <WiFi.h>
#include <string.h>

// --- CHANGE ME: your WiFi ---
static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// --- CHANGE ME: your mail server (see the README to set up Postfix on a Raspberry Pi) ---
static const char *MAIL_SERVER = "192.168.1.50"; // the Postfix box's IP address
static const uint16_t MAIL_PORT = 25;            // plain SMTP (no encryption) on a trusted LAN
static const char *MAIL_FROM = "esp32@rpi5.local";
static const char *MAIL_TO = "dstroy0@rpi5.local"; // a real mailbox on that server

void send_alert(const char *subject, const char *body)
{
    SmtpConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.host = MAIL_SERVER;
    cfg.port = MAIL_PORT;
    cfg.tls = false;    // plaintext to a LAN relay; for Gmail-style use 465 + set tls = true
    cfg.user = nullptr; // no login needed for a LAN relay that trusts your network
    cfg.pass = nullptr;
    cfg.from = MAIL_FROM;
    cfg.helo = "esp32";

    SmtpMessage msg;
    memset(&msg, 0, sizeof(msg));
    msg.to = MAIL_TO;
    msg.subject = subject;
    msg.body = body;

    SmtpResult rc = smtp_send(&cfg, &msg);
    if (rc == SmtpResult::SMTP_OK)
        Serial.println("email sent - check the mailbox on your mail server");
    else
        Serial.printf("email failed (SmtpResult %d) - see the README troubleshooting table\n", (int)rc);
}

void setup()
{
    Serial.begin(115200);

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

    send_alert("ESP32 alert", "Hello from your ESP32.\nThis is a test alert.\n");
}

void loop()
{
    delay(1000);
}
