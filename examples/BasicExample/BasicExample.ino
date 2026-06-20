// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file BasicExample.ino
 * @brief Minimal example demonstrating core DetWebServer features.
 *
 * Demonstrates:
 *   - WiFi connection via init_wifi_physical() + wifi_ready() polling
 *   - Checking begin() return value for heap exhaustion
 *   - CORS headers via set_cors()
 *   - Exact route: GET /api/status → JSON with HTTP version field
 *   - Wildcard route: GET /files/* → echoes the requested path
 *   - POST route with body access
 *   - Custom 404 handler via on_not_found()
 *   - Query parameter access via http_get_query()
 *   - Request header access via http_get_header()
 *
 * The framework handles these automatically — no application code needed:
 *   - 400 Bad Request  — RFC 7230 character violation in method/path/headers
 *   - 413 Payload Too Large — Content-Length exceeds BODY_BUF_SIZE
 *   - 414 URI Too Long — path exceeds MAX_PATH_LEN
 *   - 501 Not Implemented — Transfer-Encoding header present (chunked not supported)
 *
 * Flash to any ESP32 board.  Open the Serial Monitor at 115200 baud to
 * see the assigned IP address, then use curl or a browser to test routes.
 *
 * Example curl commands:
 *   curl http://<ip>/api/status
 *   curl http://<ip>/api/status?verbose=1
 *   curl -X POST http://<ip>/api/echo -d "hello world"
 *   curl http://<ip>/files/image.png
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static int request_count = 0;

// GET /api/status
// Returns a JSON object with a rolling request counter and the HTTP version
// the client used.  Demonstrates req->version and query param access.
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

    // ?verbose=1 includes the path in the response
    const char *verbose = http_get_query(req, "verbose");

    char body[160];
    if (verbose && strcmp(verbose, "1") == 0)
        snprintf(body, sizeof(body), "{\"status\":\"ok\",\"count\":%d,\"http\":\"%s\",\"path\":\"%s\"}", request_count,
                 version_str, req->path);
    else
        snprintf(body, sizeof(body), "{\"status\":\"ok\",\"count\":%d,\"http\":\"%s\"}", request_count, version_str);

    server.send(slot_id, 200, "application/json", body);
}

// POST /api/echo
// Echoes the request body back as plain text.
// req->body is always null-terminated; body_len holds the byte count.
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

// GET /files/* (wildcard)
// A placeholder for static file serving.  req->path holds the full request
// path, not just the suffix after the wildcard prefix.
void handle_files(uint8_t slot_id, HttpReq *req)
{
    // Demonstrate header lookup — check if the client prefers gzip
    const char *accept_enc = http_get_header(req, "Accept-Encoding");
    bool wants_gzip = accept_enc && strstr(accept_enc, "gzip") != nullptr;

    char msg[MAX_PATH_LEN + 64];
    snprintf(msg, sizeof(msg), "Requested: %s%s", req->path, wants_gzip ? " (gzip accepted)" : "");
    server.send(slot_id, 200, "text/plain", msg);
}

// Fallback for any unmatched route
void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    char msg[MAX_PATH_LEN + 32];
    snprintf(msg, sizeof(msg), "Not found: %s", req->path);
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

    // Allow cross-origin requests from any origin (swap "*" for your
    // frontend origin in production).
    server.set_cors("*");

    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/echo", HTTP_POST, handle_echo);
    server.on("/files/*", HTTP_GET, handle_files);
    server.on_not_found(handle_not_found);

    int32_t result = server.begin(80);
    if (result < 0)
    {
        // Negative return means we ran out of heap.
        // abs(result) is how many bytes were needed.
        Serial.printf("begin() failed — need %d more heap bytes\n", -result);
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
    //   4. Auto-sends 400 / 413 / 414 for parser error states
    server.handle();
}
