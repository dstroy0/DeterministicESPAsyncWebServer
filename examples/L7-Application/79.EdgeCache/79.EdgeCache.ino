// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 79.EdgeCache.ino
 * @brief Cache an upstream origin at the edge (DETWS_ENABLE_EDGE_CACHE).
 *
 * The board acts as a caching reverse-proxy: a GET/HEAD under a mapped prefix is fetched from the
 * upstream origin once, cached in RAM, and subsequent hits are served from the board - honoring
 * Cache-Control / ETag / Last-Modified, revalidating stale entries with a conditional request, and
 * never stalling the server (the origin fetch runs asynchronously while the client request is
 * suspended). A /cache/stats route reports the counters and /cache/purge invalidates by prefix.
 *
 * Wiring is two calls: det_edge_cache_map() binds a path prefix to an origin, det_edge_cache_enable()
 * installs the cache on the server. Edit the "CHANGE ME" lines, flash, open Serial @ 115200, then
 * request GET /cdn/<path> and watch the X-Cache header flip from MISS to HIT.
 *
 * NOTE (PlatformIO): the cache is compiled into the *library*, so the flags must reach the whole build:
 * build_flags = -DDETWS_ENABLE_EDGE_CACHE=1 -DDETWS_ENABLE_HTTP_CACHE=1 -DDETWS_ENABLE_HTTP_CLIENT=1.
 * In the Arduino IDE they are set for you in build_opt.h.
 */

#define DETWS_ENABLE_EDGE_CACHE 1
#define DETWS_ENABLE_HTTP_CACHE 1
#define DETWS_ENABLE_HTTP_CLIENT 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/edge_cache/edge_cache_proxy.h"
#include <Arduino.h>
#include <WiFi.h>

// --- CHANGE ME: your WiFi ---
static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// --- CHANGE ME: the upstream origin the board caches in front of (plaintext http:// only in v1) ---
static const char *ORIGIN = "http://192.168.1.60:8000";

DetWebServer server;

// GET /cache/stats - the cache counters as JSON (not itself cached: it is outside the /cdn/ prefix).
static void handle_stats(uint8_t slot, HttpReq *req)
{
    (void)req;
    EdgeCacheStats st;
    det_edge_cache_stats(&st);
    char body[224];
    snprintf(body, sizeof(body),
             "{\"hits\":%u,\"misses\":%u,\"revalidations\":%u,\"replaces\":%u,\"stores\":%u,\"evictions\":%u,"
             "\"purges\":%u}",
             (unsigned)st.hits, (unsigned)st.misses, (unsigned)st.revalidations_304, (unsigned)st.replaces_200,
             (unsigned)st.stores, (unsigned)st.evictions, (unsigned)st.purges);
    server.send(slot, 200, "application/json", body);
}

// POST /cache/purge - invalidate everything cached under the mapped prefix.
static void handle_purge(uint8_t slot, HttpReq *req)
{
    (void)req;
    uint32_t n = det_edge_cache_purge_prefix("/cdn/");
    char body[48];
    snprintf(body, sizeof(body), "{\"purged\":%u}", (unsigned)n);
    server.send(slot, 200, "application/json", body);
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

    // Cache everything under /cdn/ from the origin, then enable the cache on the server.
    det_edge_cache_map("/cdn/", ORIGIN);
    det_edge_cache_enable(server);
    server.on("/cache/stats", HttpMethod::HTTP_GET, handle_stats);
    server.on("/cache/purge", HttpMethod::HTTP_POST, handle_purge);
    server.begin();

    Serial.printf("edge cache in front of %s\n", ORIGIN);
    Serial.printf("GET http://%s/cdn/<path> - X-Cache: MISS then HIT\n", WiFi.localIP().toString().c_str());
    Serial.println("GET /cache/stats for counters; POST /cache/purge to invalidate /cdn/");
}

void loop()
{
    server.handle(); // the server poll loop drives the async origin fetch + the cached send
}
