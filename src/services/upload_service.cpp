// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file upload_service.cpp
 * @brief Streaming file upload: POST body -> Arduino FS file (DETWS_ENABLE_UPLOAD).
 */

#include "upload_service.h"

#if DETWS_ENABLE_UPLOAD

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/presentation/http_parser.h"
#include <stdio.h>
#include <string.h>

static DetWebServer *g_server = nullptr;
static const char *g_path = nullptr;
static fs::FS *g_fs = nullptr;
static const char *g_dest = nullptr;

// Per-upload state (one upload at a time on this single-task device).
static fs::File g_file;
static bool g_active = false; ///< Destination file opened for the current upload.
static bool g_error = false;  ///< A write failed during the current upload.
static size_t g_written = 0;  ///< Bytes written so far / in the last upload.

/// @brief Stream-begin hook: accept POST @p g_path and open the destination file.
static bool upload_stream_begin(HttpReq *req)
{
    if (strcmp(req->method, "POST") != 0)
        return false;
    if (!g_path || strcmp(req->path, g_path) != 0)
        return false;

    g_active = false;
    g_error = false;
    g_written = 0;
    if (g_fs && g_dest)
    {
        g_file = g_fs->open(g_dest, "w");
        if (g_file)
            g_active = true;
        else
            g_error = true;
    }
    // Stream regardless so the body is consumed and the route handler can reply.
    return true;
}

/// @brief Stream-data hook: write one body chunk to the destination file.
static void upload_stream_data(const uint8_t *data, size_t len)
{
    if (g_active && !g_error)
    {
        if (g_file.write(data, len) != len)
            g_error = true;
        else
            g_written += len;
    }
}

/// @brief Route handler (runs at PARSE_COMPLETE): close the file and reply.
static void upload_handle(uint8_t slot_id, HttpReq *req)
{
    if (!req->body_streaming)
    {
        g_server->send(slot_id, 400, "text/plain", "POST a file body");
        return;
    }
    if (g_active)
        g_file.close();
    if (!g_active || g_error)
    {
        g_server->send(slot_id, 500, "text/plain", "upload failed");
        return;
    }
    char msg[48];
    snprintf(msg, sizeof(msg), "OK %u bytes", (unsigned)g_written);
    g_server->send(slot_id, 200, "text/plain", msg);
}

size_t detws_upload_last_size()
{
    return g_written;
}

void detws_upload_begin(DetWebServer &server, const char *path, fs::FS &fs, const char *dest_path)
{
    g_server = &server;
    g_path = path;
    g_fs = &fs;
    g_dest = dest_path;

    http_parser_set_stream_hooks(upload_stream_begin, upload_stream_data);
    server.on(path, HTTP_POST, upload_handle);
}

#endif // DETWS_ENABLE_UPLOAD
