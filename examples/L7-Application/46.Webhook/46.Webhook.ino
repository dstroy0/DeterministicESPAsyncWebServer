// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 46.Webhook.ino
 * @brief Outbound webhooks / IFTTT (DETWS_ENABLE_WEBHOOK).
 *
 * Pushes an event FROM the device: builds an IFTTT Maker URL + value1/2/3 JSON
 * and POSTs it through the outbound http_client. GET /fire triggers it on demand;
 * point WEBHOOK_URL at IFTTT, a Slack/Discord incoming webhook, or your own API.
 *
 * NOTE: enable both flags for the whole build. In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_HTTP_CLIENT=1 -DDETWS_ENABLE_WEBHOOK=1
 * (Arduino IDE: they are already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DETWS_ENABLE_HTTP_CLIENT 1
#define DETWS_ENABLE_WEBHOOK 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/webhook/webhook.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// A plain webhook endpoint (Slack/Discord/your API). For IFTTT use the helper below.
static const char *WEBHOOK_URL = "http://192.168.1.10:8080/hook";

DetWebServer server;

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    server.begin(80);
}

void loop()
{
    // Fire a webhook once at startup (and you could fire on any event). Done from
    // loop() (not a handler) so the blocking POST never stalls the worker that
    // serves this server.
    static bool fired = false;
    if (!fired && wifi_ready())
    {
        fired = true;
        char body[128];
        detws_ifttt_payload("boot", "esp32", nullptr, body, sizeof(body));
        int status = detws_webhook_post(WEBHOOK_URL, body);
        Serial.printf("[webhook] POST -> status %d\n", status);

        // IFTTT Maker form (needs your real key):
        //   detws_ifttt_trigger("device_boot", "YOUR_IFTTT_KEY", "esp32", nullptr, nullptr);
    }
}
