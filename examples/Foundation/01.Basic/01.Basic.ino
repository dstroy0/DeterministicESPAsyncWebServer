// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 01.Basic.ino
 * @brief Entry-level example demonstrating the core DetWebServer features.
 *
 * Demonstrates:
 *   - WiFi connection via init_wifi_physical() + wifi_ready() polling
 *   - Checking the begin() return value (negative == failure)
 *   - CORS headers via set_cors()
 *   - GET /            → plain-text greeting
 *   - GET /api/status  → JSON with request count, HTTP version, uptime, heap
 *                        (?verbose=1 adds the request path)
 *   - POST /api/echo   → echoes the request body (empty-body guard)
 *   - GET /files/*     → wildcard route; demonstrates header lookup (gzip)
 *   - on_not_found()   → custom 404 handler
 *   - http_get_query() / http_get_header() accessors
 *
 * The framework handles these automatically: no application code needed:
 *   - 400 Bad Request      : RFC 7230 character violation in method/path/headers
 *   - 413 Payload Too Large: Content-Length exceeds BODY_BUF_SIZE
 *   - 414 URI Too Long     : path exceeds MAX_PATH_LEN
 *   - 501 Not Implemented  : Transfer-Encoding present (chunked not supported)
 *
 * Flash to any ESP32 board.  Open the Serial Monitor at 115200 baud to see the
 * assigned IP address, then use curl or a browser to test routes.
 *
 * Example curl commands:
 *   curl http://<ip>/
 *   curl http://<ip>/api/status
 *   curl "http://<ip>/api/status?verbose=1"
 *   curl -X POST http://<ip>/api/echo -d "hello world"
 *   curl http://<ip>/files/image.png
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static unsigned long request_count = 0;

// GET /
// Plain-text greeting; the simplest possible handler.
void handle_root(uint8_t slot_id, HttpReq *req)
{
    request_count++;
    server.send(slot_id, 200, "text/plain", "Welcome to DeterministicESPAsyncWebServer!");
}

// GET /api/status
// JSON status with the HTTP version the client used, plus uptime and free heap.
// ?verbose=1 additionally includes the request path.
void handle_status(uint8_t slot_id, HttpReq *req)
{
    request_count++;

    const char *version_str;
    switch (req->version)
    {
    case HTTP_11:
        version_str = "1.1";
        break;
    case HTTP_10:
        version_str = "1.0";
        break;
    default:
        version_str = "?";
        break;
    }

    const char *verbose = http_get_query(req, "verbose");

    char body[192];
    if (verbose && strcmp(verbose, "1") == 0)
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%lu,\"http\":\"%s\",\"uptime_ms\":%lu,\"free_heap\":%u,\"path\":\"%s\"}",
                 request_count, version_str, millis(), ESP.getFreeHeap(), req->path);
    else
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%lu,\"http\":\"%s\",\"uptime_ms\":%lu,\"free_heap\":%u}", request_count,
                 version_str, millis(), ESP.getFreeHeap());

    server.send(slot_id, 200, "application/json", body);
}

// POST /api/echo
// Echoes the request body back as plain text.  req->body is always
// null-terminated; body_len holds the byte count.
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    if (req->body_len == 0)
    {
        server.send(slot_id, 400, "text/plain", "Error: request body is empty");
        return;
    }
    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

// GET /files/* (wildcard)
// A placeholder for static file serving.  req->path holds the full request
// path, not just the suffix after the wildcard prefix.
void handle_files(uint8_t slot_id, HttpReq *req)
{
    // Demonstrate header lookup: check whether the client accepts gzip.
    const char *accept_enc = http_get_header(req, "Accept-Encoding");
    bool wants_gzip = accept_enc && strstr(accept_enc, "gzip") != nullptr;

    char msg[MAX_PATH_LEN + 64];
    snprintf(msg, sizeof(msg), "Requested: %s%s", req->path, wants_gzip ? " (gzip accepted)" : "");
    server.send(slot_id, 200, "text/plain", msg);
}

// Fallback for any unmatched route.
void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    char msg[MAX_PATH_LEN + 48];
    snprintf(msg, sizeof(msg), "Not found: %s %s", req->method, req->path);
    server.send(slot_id, 404, "text/plain", msg);
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

    // Allow cross-origin requests from any origin (swap "*" for your frontend
    // origin in production).
    server.set_cors("*");

    server.on("/", HTTP_GET, handle_root);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/echo", HTTP_POST, handle_echo);
    server.on("/files/*", HTTP_GET, handle_files);
    server.on_not_found(handle_not_found);

    // begin() returns 1 on success and a negative value on failure (listener
    // pool full or lwIP error).  -1 is truthy, so test for "< 0", not "!result".
    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    // Drives the full pipeline each iteration:
    //   1. Timeout sweep (force-closes idle connections)
    //   2. Event queue drain (TCP connect/data/disconnect events)
    //   3. Route dispatch for completed requests
    //   4. Auto-sends 400 / 413 / 414 / 501 for parser error states
    server.handle();
}
