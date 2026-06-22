// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file advanced.ino
 * @brief Advanced example demonstrating RESTful CRUD APIs, header verification, and zero-allocation JSON parsing.
 *
 * This example showcases:
 *   1. RESTful CRUD endpoints (GET, POST, PATCH, DELETE) for a mock sensor database.
 *   2. Strict HTTP request validation:
 *      - Authorization headers (bearer token) verified before modifications.
 *      - Content-Type header checked for incoming POST/PATCH requests.
 *   3. Extraction of wildcard ID parameters from routes (e.g., /api/sensors/2).
 *   4. Zero-heap manual JSON scanning for incoming payloads.
 *   5. Response diversity using different HTTP status codes:
 *      - 200 OK
 *      - 201 Created
 *      - 204 No Content
 *      - 400 Bad Request
 *      - 401 Unauthorized
 *      - 404 Not Found
 *   6. Query string filtering based on parameter values.
 *
 * To run this example:
 *   - Configure SSID/PASSWORD, and upload to an ESP32.
 *   - Use a REST client (like Postman or curl) to interact with /api/sensors.
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Security token required for write/delete actions
static const char *EXPECTED_TOKEN = "Bearer secret_admin_token";

// Zero-allocation static "database" model
struct SensorDevice
{
    int id;
    char name[16];
    float temperature;
    bool active;
    bool in_use; // Slot flag
};

#define MAX_SENSORS 4
static SensorDevice sensor_db[MAX_SENSORS] = {
    {0, "Living Room", 22.4, true, true},
    {1, "Kitchen", 24.1, true, true},
    {2, "Garage", 15.8, false, true},
    {3, "", 0.0, false, false} // Empty slot
};

// --- Helper Functions ---

/**
 * @brief Checks if a request has a valid Authorization header.
 * @return true if authenticated, false otherwise.
 */
bool is_authorized(const HttpReq *req)
{
    const char *auth_hdr = http_get_header(req, "Authorization");
    if (auth_hdr && strcmp(auth_hdr, EXPECTED_TOKEN) == 0)
    {
        return true;
    }
    return false;
}

/**
 * @brief Custom helper to scan float values out of a simple JSON body.
 * Works without heap allocation by scanning the buffer.
 */
bool json_get_float(const char *json, const char *key, float &out_val)
{
    // Search for key pattern, e.g., "temp":
    char key_pattern[32];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);

    const char *ptr = strstr(json, key_pattern);
    if (!ptr)
        return false;

    // Move pointer past key
    ptr += strlen(key_pattern);

    // Skip spaces
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    out_val = (float)atof(ptr);
    return true;
}

/**
 * @brief Custom helper to scan string values out of a simple JSON body.
 */
bool json_get_string(const char *json, const char *key, char *out_buf, size_t max_len)
{
    char key_pattern[32];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);

    const char *ptr = strstr(json, key_pattern);
    if (!ptr)
        return false;

    ptr += strlen(key_pattern);

    // Skip spaces or quotes
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '"')
        ptr++;

    size_t i = 0;
    while (*ptr && *ptr != '"' && *ptr != ',' && *ptr != '}' && i < (max_len - 1))
    {
        out_buf[i++] = *ptr++;
    }
    out_buf[i] = '\0';
    return true;
}

/**
 * @brief Custom helper to scan boolean values out of a simple JSON body.
 */
bool json_get_bool(const char *json, const char *key, bool &out_val)
{
    char key_pattern[32];
    snprintf(key_pattern, sizeof(key_pattern), "\"%s\":", key);

    const char *ptr = strstr(json, key_pattern);
    if (!ptr)
        return false;

    ptr += strlen(key_pattern);
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    if (strncmp(ptr, "true", 4) == 0)
    {
        out_val = true;
        return true;
    }
    else if (strncmp(ptr, "false", 5) == 0)
    {
        out_val = false;
        return true;
    }
    return false;
}

// --- Route Handlers ---

/**
 * @brief GET /api/sensors
 * Serves list of sensors. Supports query filter: ?active=1 or ?active=0
 */
void handle_get_sensors(uint8_t slot_id, HttpReq *req)
{
    const char *active_filter = http_get_query(req, "active");
    bool filter_by_active = (active_filter != nullptr);
    bool active_target_val = (filter_by_active && strcmp(active_filter, "1") == 0);

    // Build the JSON list response. We construct this incrementally on stack.
    char response_buf[512];
    int len = snprintf(response_buf, sizeof(response_buf), "[");

    bool first = true;
    for (int i = 0; i < MAX_SENSORS; i++)
    {
        if (!sensor_db[i].in_use)
            continue;
        if (filter_by_active && (sensor_db[i].active != active_target_val))
            continue;

        if (!first)
        {
            len += snprintf(response_buf + len, sizeof(response_buf) - len, ",");
        }
        first = false;

        len += snprintf(response_buf + len, sizeof(response_buf) - len,
                        "{\"id\":%d,\"name\":\"%s\",\"temp\":%.1f,\"active\":%s}", sensor_db[i].id, sensor_db[i].name,
                        sensor_db[i].temperature, sensor_db[i].active ? "true" : "false");
    }
    snprintf(response_buf + len, sizeof(response_buf) - len, "]");

    server.send(slot_id, 200, "application/json", response_buf);
}

/**
 * @brief GET /api/sensors/* (wildcard match)
 * Extracts sensor ID from the path prefix /api/sensors/
 */
void handle_get_sensor_by_id(uint8_t slot_id, HttpReq *req)
{
    // Length of "/api/sensors/" is 13
    if (strlen(req->path) <= 13)
    {
        server.send(slot_id, 400, "text/plain", "Missing sensor ID");
        return;
    }

    int id = atoi(req->path + 13);
    if (id < 0 || id >= MAX_SENSORS || !sensor_db[id].in_use)
    {
        server.send(slot_id, 404, "application/json", "{\"error\":\"Sensor not found\"}");
        return;
    }

    char response_buf[192];
    snprintf(response_buf, sizeof(response_buf), "{\"id\":%d,\"name\":\"%s\",\"temp\":%.1f,\"active\":%s}",
             sensor_db[id].id, sensor_db[id].name, sensor_db[id].temperature, sensor_db[id].active ? "true" : "false");

    server.send(slot_id, 200, "application/json", response_buf);
}

/**
 * @brief POST /api/sensors
 * Creates a new sensor. Requires authentication and JSON payload.
 */
void handle_create_sensor(uint8_t slot_id, HttpReq *req)
{
    // 1. Verify Authorization
    if (!is_authorized(req))
    {
        server.send(slot_id, 401, "text/plain", "401 Unauthorized: Invalid token");
        return;
    }

    // 2. Verify Content-Type
    const char *content_type = http_get_header(req, "Content-Type");
    if (!content_type || strstr(content_type, "application/json") == nullptr)
    {
        server.send(slot_id, 400, "text/plain", "400 Bad Request: Content-Type must be application/json");
        return;
    }

    // Find an empty slot
    int empty_slot = -1;
    for (int i = 0; i < MAX_SENSORS; i++)
    {
        if (!sensor_db[i].in_use)
        {
            empty_slot = i;
            break;
        }
    }

    if (empty_slot == -1)
    {
        server.send(slot_id, 409, "application/json", "{\"error\":\"Database table full\"}");
        return;
    }

    // 3. Extract JSON keys
    const char *body = (const char *)req->body;
    char name[16] = "";
    float temp = 0.0;
    bool active = false;

    if (!json_get_string(body, "name", name, sizeof(name)) || !json_get_float(body, "temp", temp) ||
        !json_get_bool(body, "active", active))
    {
        server.send(slot_id, 400, "application/json", "{\"error\":\"Invalid JSON format or missing keys\"}");
        return;
    }

    // Save item
    sensor_db[empty_slot].id = empty_slot;
    strncpy(sensor_db[empty_slot].name, name, sizeof(sensor_db[empty_slot].name) - 1);
    sensor_db[empty_slot].temperature = temp;
    sensor_db[empty_slot].active = active;
    sensor_db[empty_slot].in_use = true;

    char response_buf[192];
    snprintf(response_buf, sizeof(response_buf), "{\"id\":%d,\"name\":\"%s\",\"status\":\"created\"}", empty_slot,
             sensor_db[empty_slot].name);

    server.send(slot_id, 201, "application/json", response_buf);
}

/**
 * @brief PATCH /api/sensors/*
 * Partially updates a sensor's temperature or activity state. Requires authentication.
 */
void handle_patch_sensor(uint8_t slot_id, HttpReq *req)
{
    if (!is_authorized(req))
    {
        server.send(slot_id, 401, "text/plain", "401 Unauthorized: Invalid token");
        return;
    }

    if (strlen(req->path) <= 13)
    {
        server.send(slot_id, 400, "text/plain", "Missing sensor ID");
        return;
    }

    int id = atoi(req->path + 13);
    if (id < 0 || id >= MAX_SENSORS || !sensor_db[id].in_use)
    {
        server.send(slot_id, 404, "application/json", "{\"error\":\"Sensor not found\"}");
        return;
    }

    const char *body = (const char *)req->body;
    float new_temp;
    bool new_active;

    if (json_get_float(body, "temp", new_temp))
    {
        sensor_db[id].temperature = new_temp;
    }
    if (json_get_bool(body, "active", new_active))
    {
        sensor_db[id].active = new_active;
    }

    char response_buf[192];
    snprintf(response_buf, sizeof(response_buf), "{\"id\":%d,\"name\":\"%s\",\"temp\":%.1f,\"active\":%s}",
             sensor_db[id].id, sensor_db[id].name, sensor_db[id].temperature, sensor_db[id].active ? "true" : "false");

    server.send(slot_id, 200, "application/json", response_buf);
}

/**
 * @brief DELETE /api/sensors/*
 * Removes a sensor from the database. Requires authentication.
 */
void handle_delete_sensor(uint8_t slot_id, HttpReq *req)
{
    if (!is_authorized(req))
    {
        server.send(slot_id, 401, "text/plain", "401 Unauthorized: Invalid token");
        return;
    }

    if (strlen(req->path) <= 13)
    {
        server.send(slot_id, 400, "text/plain", "Missing sensor ID");
        return;
    }

    int id = atoi(req->path + 13);
    if (id < 0 || id >= MAX_SENSORS || !sensor_db[id].in_use)
    {
        server.send(slot_id, 404, "application/json", "{\"error\":\"Sensor not found\"}");
        return;
    }

    // Free slot in database
    sensor_db[id].in_use = false;

    // 204 status requires no response body
    server.send_empty(slot_id, 204);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- DetWebServer Advanced REST CRUD Example ---");

    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Associated!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    server.set_cors("*");

    // Map REST routes using methods
    server.on("/api/sensors", HTTP_GET, handle_get_sensors);
    server.on("/api/sensors/*", HTTP_GET, handle_get_sensor_by_id);
    server.on("/api/sensors", HTTP_POST, handle_create_sensor);
    server.on("/api/sensors/*", HTTP_PATCH, handle_patch_sensor);
    server.on("/api/sensors/*", HTTP_DELETE, handle_delete_sensor);

    if (server.begin(80))
    {
        Serial.println("REST API Server running on port 80");
        Serial.println("Admin token expected in headers: 'Authorization: Bearer secret_admin_token'");
    }
}

void loop()
{
    server.handle();
}
