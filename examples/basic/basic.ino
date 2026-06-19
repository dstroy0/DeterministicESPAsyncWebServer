// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file basic.ino
 * @brief Thorough entry-level example demonstrating core DetWebServer features.
 *
 * This example showcases:
 *   1. WiFi provisioning and polling connection status using wifi_ready().
 *   2. Initializing DetWebServer with standard configurations.
 *   3. Enabling cross-origin resource sharing (CORS) with set_cors().
 *   4. Creating route handlers for specific HTTP methods:
 *      - GET /            -> Serving basic text greetings.
 *      - GET /api/status  -> Generating custom JSON payloads and reading query strings.
 *      - POST /api/echo   -> Reading the incoming request body safely.
 *      - GET /files/*     -> Using wildcard path patterns.
 *   5. Registering a custom 404 handler for unmatched routes.
 *
 * To run this example:
 *   - Update SSID and PASSWORD with your network credentials.
 *   - Upload to an ESP32 board.
 *   - Monitor serial output at 115200 baud.
 */

#include <WiFi.h>
#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"

// WiFi Credentials - Replace with your network settings
static const char *SSID     = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// Server instance on default HTTP port
DetWebServer server;

// Simple state to track requests since boot
static unsigned long boot_requests = 0;

/**
 * @brief Handler for GET /
 * Serves a simple plain text welcome message.
 */
void handle_root(uint8_t slot_id, HttpReq *req)
{
    boot_requests++;
    server.send(slot_id, 200, "text/plain", "Welcome to the Deterministic ESP Async Web Server!");
}

/**
 * @brief Handler for GET /api/status
 * Reads optional query parameter "?detailed=1" and replies with a JSON status payload.
 */
void handle_status(uint8_t slot_id, HttpReq *req)
{
    boot_requests++;

    // Query parameter lookup is O(N) where N is number of parsed parameters (max 8)
    const char *detailed_param = http_get_query(req, "detailed");
    bool detailed = (detailed_param && strcmp(detailed_param, "1") == 0);

    char response_buf[256];
    if (detailed)
    {
        snprintf(response_buf, sizeof(response_buf),
                 "{\"status\":\"online\",\"uptime_ms\":%lu,\"requests\":%lu,\"free_heap\":%u}",
                 millis(), boot_requests, ESP.getFreeHeap());
    }
    else
    {
        snprintf(response_buf, sizeof(response_buf),
                 "{\"status\":\"online\",\"requests\":%lu}",
                 boot_requests);
    }

    server.send(slot_id, 200, "application/json", response_buf);
}

/**
 * @brief Handler for POST /api/echo
 * Echoes the raw request body back to the client.
 */
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    boot_requests++;

    // req->body is guaranteed to be null-terminated by the presentation layer parser.
    // Length is capped by BODY_BUF_SIZE (default 256 bytes) to ensure zero dynamic allocations.
    if (req->body_len == 0)
    {
        server.send(slot_id, 400, "text/plain", "Error: Request body is empty");
        return;
    }

    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

/**
 * @brief Handler for GET /files/*
 * Matches any request prefixing /files/ and prints the wildcard sub-path.
 */
void handle_files(uint8_t slot_id, HttpReq *req)
{
    boot_requests++;

    // For /files/* route, req->path holds the full path (e.g. "/files/images/logo.png")
    char message[128];
    snprintf(message, sizeof(message), "Requested path details: %s", req->path);
    server.send(slot_id, 200, "text/plain", message);
}

/**
 * @brief Custom Not Found (404) Fallback handler
 * Triggered when no registered route matches the request path or method.
 */
void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    char error_msg[128];
    snprintf(error_msg, sizeof(error_msg), "Error 404: Resource '%s' with method '%s' not found.", 
             req->path, req->method);
    server.send(slot_id, 404, "text/plain", error_msg);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- DetWebServer Basic Example ---");

    // Initialize WiFi using the physical network driver layer
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to SSID: ");
    Serial.println(SSID);

    // Poll until connection is established
    while (!wifi_ready())
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Enable CORS from any origin for development ease
    server.set_cors("*");

    // Register routes
    server.on("/", HTTP_GET, handle_root);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/echo", HTTP_POST, handle_echo);
    server.on("/files/*", HTTP_GET, handle_files);

    // Register custom fallback handler
    server.on_not_found(handle_not_found);

    // Start server on port 80
    if (server.begin(80))
    {
        Serial.println("Deterministic HTTP server started successfully on port 80");
    }
    else
    {
        Serial.println("Failed to start server. lwIP error occurred.");
    }
}

void loop()
{
    // The core handle() function executes the parser state machines, performs session
    // timeout sweeps (O(MAX_CONNS)), and dispatches requests. It must be called
    // continuously without blocking delay() calls in loop().
    server.handle();
}
