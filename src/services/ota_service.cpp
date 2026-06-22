// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ota_service.cpp
 * @brief Authenticated streaming OTA firmware update (DETWS_ENABLE_OTA).
 */

#include "ota_service.h"

#if DETWS_ENABLE_OTA && defined(ARDUINO)

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/presentation/base64.h"
#include "network_drivers/presentation/http_parser.h"
#include <Arduino.h>
#include <Update.h>
#include <string.h>

static DetWebServer *g_server = nullptr;
static const char *g_path = nullptr;
static char g_user[MAX_AUTH_LEN];
static char g_pass[MAX_AUTH_LEN];

// Per-upload state (one upload at a time on this single-task device).
static bool g_authed = false; ///< Credentials validated for the current upload.
static bool g_active = false; ///< Update.begin() succeeded for the current upload.
static bool g_error = false;  ///< A write failed during the current upload.

/// @brief Validate the request's HTTP Basic credentials against g_user/g_pass.
static bool ota_check_auth(HttpReq *req)
{
    const char *h = http_get_header(req, "Authorization");
    if (!h || strncmp(h, "Basic ", 6) != 0)
        return false;

    uint8_t decoded[MAX_AUTH_LEN * 2 + 2];
    size_t n = base64_decode(h + 6, decoded, sizeof(decoded) - 1);
    if (n == 0)
        return false;
    decoded[n] = '\0';

    const char *colon = (const char *)memchr(decoded, ':', n);
    if (!colon)
        return false;
    size_t ulen = (size_t)(colon - (const char *)decoded);
    const char *pass = colon + 1;
    return (ulen == strlen(g_user)) && (memcmp(decoded, g_user, ulen) == 0) && (strcmp(pass, g_pass) == 0);
}

/// @brief Stream-begin hook: accept POST @p g_path; begin Update if authorized.
static bool ota_stream_begin(HttpReq *req)
{
    if (strcmp(req->method, "POST") != 0)
        return false;
    if (!g_path || strcmp(req->path, g_path) != 0)
        return false;

    g_authed = ota_check_auth(req);
    g_active = false;
    g_error = false;
    if (g_authed)
    {
        if (Update.begin(UPDATE_SIZE_UNKNOWN))
            g_active = true;
        else
            g_error = true;
    }
    // Stream regardless so the body is consumed and the route handler can reply;
    // when unauthorized/!active the data hook simply discards.
    return true;
}

/// @brief Stream-data hook: write one chunk of the image to Update.
static void ota_stream_data(const uint8_t *data, size_t len)
{
    if (g_authed && g_active && !g_error)
    {
        if (Update.write((uint8_t *)data, len) != len)
            g_error = true;
    }
}

/// @brief Route handler (runs at PARSE_COMPLETE): finalize and reply, then reboot.
static void ota_handle(uint8_t slot_id, HttpReq *req)
{
    if (!req->body_streaming)
    {
        g_server->send(slot_id, 400, "text/plain", "POST a raw firmware image");
        return;
    }
    if (!g_authed)
    {
        g_server->send(slot_id, 401, "text/plain", "Unauthorized");
        return;
    }
    bool ok = g_active && !g_error && Update.end(true);
    if (!ok)
    {
        if (g_active)
            Update.abort();
        g_server->send(slot_id, 400, "text/plain", "Update failed");
        return;
    }
    g_server->send(slot_id, 200, "text/plain", "OK - rebooting");
    delay(150); // let the response flush before the reboot
    ESP.restart();
}

void detws_ota_begin(DetWebServer &server, const char *path, const char *user, const char *pass)
{
    g_server = &server;
    g_path = path;
    strncpy(g_user, user ? user : "", sizeof(g_user) - 1);
    g_user[sizeof(g_user) - 1] = '\0';
    strncpy(g_pass, pass ? pass : "", sizeof(g_pass) - 1);
    g_pass[sizeof(g_pass) - 1] = '\0';

    http_parser_set_stream_hooks(ota_stream_begin, ota_stream_data);
    server.on(path, HTTP_POST, ota_handle);
}

#else

void detws_ota_begin(DetWebServer &server, const char *path, const char *user, const char *pass)
{
    (void)server;
    (void)path;
    (void)user;
    (void)pass;
}

#endif // DETWS_ENABLE_OTA && ARDUINO
