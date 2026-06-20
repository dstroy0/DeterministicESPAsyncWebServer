// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file CustomConfig.ino
 * @brief Demonstrates overriding every compile-time sizing constant.
 *
 * All buffer and pool dimensions are set via build flags in platformio.ini.
 * The values below cut memory use roughly in half compared to the defaults,
 * which is useful for sensor nodes or builds that share the ESP32 heap with
 * other subsystems.
 *
 * Add these to your platformio.ini [env] block to use this profile:
 *
 * @code{.ini}
 * build_flags =
 *     -DMAX_CONNS=2          ; simultaneous TCP connections   (default 4)
 *     -DRX_BUF_SIZE=512      ; ring-buffer bytes / connection (default 1024)
 *     -DCONN_TIMEOUT_MS=3000 ; idle timeout in ms            (default 5000)
 *     -DMAX_HEADERS=4        ; headers stored per request     (default 8)
 *     -DMAX_PATH_LEN=48      ; URL path bytes incl. leading / (default 64)
 *     -DMAX_KEY_LEN=16       ; header field-name bytes        (default 24)
 *     -DMAX_VAL_LEN=32       ; header field-value bytes       (default 48)
 *     -DMAX_QUERY_LEN=64     ; raw query-string bytes         (default 128)
 *     -DMAX_QUERY_PARAMS=4   ; parsed query parameters        (default 8)
 *     -DQUERY_KEY_LEN=16     ; query parameter key bytes      (default 24)
 *     -DQUERY_VAL_LEN=24     ; query parameter value bytes    (default 48)
 *     -DBODY_BUF_SIZE=128    ; request body bytes             (default 256)
 *     -DMAX_ROUTES=8         ; registered route handlers      (default 16)
 * @endcode
 *
 * Example curl commands:
 *   curl http://<ip>/config
 *   curl http://<ip>/echo -X POST -d "hello"
 *   curl "http://<ip>/search?q=esp32&sort=date"
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// GET /config
// Returns every active sizing constant as JSON so you can verify your
// build flags took effect without needing a debugger.
void handle_config(uint8_t slot_id, HttpReq *req)
{
    char body[384];
    snprintf(body, sizeof(body),
             "{"
             "\"MAX_CONNS\":%u,"
             "\"RX_BUF_SIZE\":%u,"
             "\"CONN_TIMEOUT_MS\":%u,"
             "\"MAX_HEADERS\":%u,"
             "\"MAX_PATH_LEN\":%u,"
             "\"MAX_KEY_LEN\":%u,"
             "\"MAX_VAL_LEN\":%u,"
             "\"MAX_QUERY_LEN\":%u,"
             "\"MAX_QUERY_PARAMS\":%u,"
             "\"QUERY_KEY_LEN\":%u,"
             "\"QUERY_VAL_LEN\":%u,"
             "\"BODY_BUF_SIZE\":%u,"
             "\"MAX_ROUTES\":%u"
             "}",
             (unsigned)MAX_CONNS, (unsigned)RX_BUF_SIZE, (unsigned)CONN_TIMEOUT_MS, (unsigned)MAX_HEADERS,
             (unsigned)MAX_PATH_LEN, (unsigned)MAX_KEY_LEN, (unsigned)MAX_VAL_LEN, (unsigned)MAX_QUERY_LEN,
             (unsigned)MAX_QUERY_PARAMS, (unsigned)QUERY_KEY_LEN, (unsigned)QUERY_VAL_LEN, (unsigned)BODY_BUF_SIZE,
             (unsigned)MAX_ROUTES);

    server.send(slot_id, 200, "application/json", body);
}

// POST /echo
// Echoes the request body back.  With -DBODY_BUF_SIZE=128, bodies longer
// than 128 bytes trigger an automatic 413 before this handler is called.
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

// GET /search
// Reads up to MAX_QUERY_PARAMS query parameters and echoes them back.
// With -DMAX_QUERY_PARAMS=4, a fifth parameter is parsed but silently
// dropped — this handler shows exactly what was retained.
void handle_search(uint8_t slot_id, HttpReq *req)
{
    char body[256] = "params: ";
    for (uint8_t i = 0; i < req->query_count; i++)
    {
        if (i > 0)
            strncat(body, ", ", sizeof(body) - strlen(body) - 1);
        strncat(body, req->query_params[i].key, sizeof(body) - strlen(body) - 1);
        strncat(body, "=", sizeof(body) - strlen(body) - 1);
        strncat(body, req->query_params[i].val, sizeof(body) - strlen(body) - 1);
    }
    server.send(slot_id, 200, "text/plain", body);
}

void setup()
{
    Serial.begin(115200);

    // Print the active config so you can confirm build flags without curl
    Serial.println("\n--- Active DetWebServer config ---");
    Serial.printf("  MAX_CONNS       = %u\n", (unsigned)MAX_CONNS);
    Serial.printf("  RX_BUF_SIZE     = %u\n", (unsigned)RX_BUF_SIZE);
    Serial.printf("  CONN_TIMEOUT_MS = %u\n", (unsigned)CONN_TIMEOUT_MS);
    Serial.printf("  MAX_HEADERS     = %u\n", (unsigned)MAX_HEADERS);
    Serial.printf("  MAX_PATH_LEN    = %u\n", (unsigned)MAX_PATH_LEN);
    Serial.printf("  MAX_KEY_LEN     = %u\n", (unsigned)MAX_KEY_LEN);
    Serial.printf("  MAX_VAL_LEN     = %u\n", (unsigned)MAX_VAL_LEN);
    Serial.printf("  MAX_QUERY_LEN   = %u\n", (unsigned)MAX_QUERY_LEN);
    Serial.printf("  MAX_QUERY_PARAMS= %u\n", (unsigned)MAX_QUERY_PARAMS);
    Serial.printf("  QUERY_KEY_LEN   = %u\n", (unsigned)QUERY_KEY_LEN);
    Serial.printf("  QUERY_VAL_LEN   = %u\n", (unsigned)QUERY_VAL_LEN);
    Serial.printf("  BODY_BUF_SIZE   = %u\n", (unsigned)BODY_BUF_SIZE);
    Serial.printf("  MAX_ROUTES      = %u\n", (unsigned)MAX_ROUTES);
    Serial.println("----------------------------------");

    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());

    server.on("/config", HTTP_GET, handle_config);
    server.on("/echo", HTTP_POST, handle_echo);
    server.on("/search", HTTP_GET, handle_search);

    // Pass a runtime config to override CONN_TIMEOUT_MS without a rebuild
    WebServerConfig cfg = {.conn_timeout_ms = CONN_TIMEOUT_MS};
    int32_t result = server.begin(80, &cfg);
    if (result < 0)
    {
        Serial.printf("begin() failed — need %d more heap bytes\n", -result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    server.handle();
}
