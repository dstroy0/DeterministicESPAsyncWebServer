// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 48.DnsResolver.ino
 * @brief DNS resolver with answer verification (DETWS_ENABLE_DNS_RESOLVER).
 *
 * Resolves a hostname to an IPv4 address and rejects suspicious answers (0.0.0.0,
 * loopback, broadcast, multicast - DNS-rebinding / spoof indicators).
 *   GET /resolve?host=dns.google -> {"ip":"8.8.8.8","verified":true}
 *
 * The resolve is blocking; this demo runs it in the handler for clarity. In a real
 * app, resolve off the request hot path (e.g. from loop() / a setup step) and cache.
 *
 * NOTE: enable it for the whole build. In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_DNS_RESOLVER=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DETWS_ENABLE_DNS_RESOLVER 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/dns_resolver/dns_resolver.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

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

    server.on("/resolve", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *req) {
        const char *host = http_get_query(req, "host");
        if (!host)
        {
            server.send(id, 400, "application/json", "{\"error\":\"missing host\"}");
            return;
        }
        uint32_t ip = 0;
        bool ok = det_dns_resolver_resolve(host, &ip);
        if (!ok)
        {
            server.send(id, 502, "application/json", "{\"error\":\"resolve failed\"}");
            return;
        }
        char b[80];
        snprintf(b, sizeof(b), "{\"ip\":\"%u.%u.%u.%u\",\"verified\":%s}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                 (ip >> 8) & 0xFF, ip & 0xFF, det_dns_resolver_verify(ip) ? "true" : "false");
        server.send(id, 200, "application/json", b);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
