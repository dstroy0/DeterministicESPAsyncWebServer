// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 26.ServerSentEvents.ino
 * @brief Server-Sent Events (text/event-stream) push via on_sse() + sse_broadcast().
 *
 * Subscribes browsers at /events; the loop pushes a counter to every subscriber
 * once a second with sse_broadcast(). A test page at / shows the live stream.
 *
 * Flash, open Serial @ 115200 for the IP, then browse to http://<ip>/.
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static const char PAGE[] = "<!doctype html><meta charset=utf-8><title>SSE</title><pre id=o></pre><script>"
                           "var s=new EventSource('/events');"
                           "s.addEventListener('tick',function(e){o.textContent+=e.data+'\\n'})</script>";

void sse_connect(uint8_t sse_id)
{
    server.sse_send(sse_id, "subscribed", "tick");
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

    server.on("/", HTTP_GET, [](uint8_t id, HttpReq *) { server.send(id, 200, "text/html", PAGE); });
    server.on_sse("/events", sse_connect);
    server.begin(80);
}

void loop()
{
    server.handle();

    static unsigned long last = 0;
    static unsigned long n = 0;
    if (millis() - last >= 1000)
    {
        last = millis();
        char buf[24];
        snprintf(buf, sizeof(buf), "%lu", n++);
        server.sse_broadcast("/events", buf, "tick");
    }
}
