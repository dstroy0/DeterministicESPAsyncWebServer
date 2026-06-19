// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file BasicExample.ino
 * @brief Minimal example demonstrating core DetWebServer features.
 *
 * Demonstrates:
 *   - WiFi connection via init_wifi_physical() + wifi_ready() polling
 *   - CORS headers via set_cors()
 *   - Exact route: GET /api/status → JSON response
 *   - Wildcard route: GET /files/* → serves a canned message
 *   - POST route with body access
 *   - Custom 404 handler via on_not_found()
 *   - Query parameter access via http_get_query()
 *   - Request header access via http_get_header()
 *
 * Flash to any ESP32 board.  Open the Serial Monitor at 115200 baud to
 * see the assigned IP address, then use curl or a browser to test routes.
 */

#include <WiFi.h>
#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"

static const char *SSID     = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static int request_count = 0;

// GET /api/status
// Returns a small JSON object with a rolling request counter.
void handle_status(uint8_t slot_id, HttpReq *req)
{
    request_count++;

    // Demonstrate query param lookup: ?verbose=1
    const char *verbose = http_get_query(req, "verbose");

    char body[128];
    if (verbose && strcmp(verbose, "1") == 0)
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%d,\"path\":\"%s\"}",
                 request_count, req->path);
    else
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%d}",
                 request_count);

    server.send(slot_id, 200, "application/json", body);
}

// POST /api/echo
// Echoes the request body back as plain text.
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    // req->body is always null-terminated; body_len holds the byte count.
    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

// GET /files/* (wildcard)
// A placeholder for static file serving.
void handle_files(uint8_t slot_id, HttpReq *req)
{
    char msg[MAX_PATH_LEN + 32];
    snprintf(msg, sizeof(msg), "Requested file: %s", req->path);
    server.send(slot_id, 200, "text/plain", msg);
}

// Fallback: any unmatched route
void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 404, "text/plain", "Not found");
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

    // Allow cross-origin requests from any origin (development convenience).
    // Replace "*" with your frontend origin in production.
    server.set_cors("*");

    server.on("/api/status", HTTP_GET,  handle_status);
    server.on("/api/echo",   HTTP_POST, handle_echo);
    server.on("/files/*",    HTTP_GET,  handle_files);
    server.on_not_found(handle_not_found);

    server.begin(80);
    Serial.println("Server started on port 80");
}

void loop()
{
    // Runs the session layer (timeout sweep + event drain) and dispatches
    // any completed HTTP requests to their registered handlers.
    server.handle();
}
