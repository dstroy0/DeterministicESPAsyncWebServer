// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file upload_service.cpp
 * @brief Streaming file upload: POST body -> Arduino FS file (DWS_ENABLE_UPLOAD).
 */

#include "upload_service.h"

#if DWS_ENABLE_UPLOAD

#include "dwserver.h"
#include "network_drivers/presentation/http_parser/http_parser.h"
#include "shared_primitives/mime.h"
#include <stdio.h>
#include <string.h>

// All upload-service state, owned by one instance (internal linkage): the server handle, the
// route path, the destination filesystem + path, and the per-upload file/flags/counter (one
// upload at a time on this single-task device). Grouped so it is one named owner, cross-TU
// unreachable.
struct UploadCtx
{
    DWS *server = nullptr;
    const char *path = nullptr;
    fs::FS *fs = nullptr;
    const char *dest = nullptr;
    fs::File file;
    bool active = false; ///< Destination file opened for the current upload.
    bool error = false;  ///< A write failed during the current upload.
    size_t written = 0;  ///< Bytes written so far / in the last upload.
};
static UploadCtx s_upl;

/// @brief Stream-begin hook: accept POST @p s_upl.path and open the destination file.
static bool upload_stream_begin(HttpReq *req)
{
    if (strcmp(req->method, "POST") != 0)
        return false;
    if (!s_upl.path || strcmp(req->path, s_upl.path) != 0)
        return false;

    s_upl.active = false;
    s_upl.error = false;
    s_upl.written = 0;
    if (s_upl.fs && s_upl.dest)
    {
        s_upl.file = s_upl.fs->open(s_upl.dest, "w");
        if (s_upl.file)
            s_upl.active = true;
        else
            s_upl.error = true;
    }
    // Stream regardless so the body is consumed and the route handler can reply.
    return true;
}

/// @brief Stream-data hook: write one body chunk to the destination file.
static void upload_stream_data(HttpReq *req, const uint8_t *data, size_t len)
{
    (void)req; // a single upload streams at a time
    if (s_upl.active && !s_upl.error)
    {
        if (s_upl.file.write(data, len) != len)
            s_upl.error = true;
        else
            s_upl.written += len;
    }
}

/// @brief Route handler (runs at ParseState::PARSE_COMPLETE): close the file and reply.
static void upload_handle(uint8_t slot_id, HttpReq *req)
{
    if (!req->body_streaming)
    {
        s_upl.server->send(slot_id, 400, DWS_MIME_TEXT_PLAIN, "POST a file body");
        return;
    }
    if (s_upl.active)
        s_upl.file.close();
    if (!s_upl.active || s_upl.error)
    {
        s_upl.server->send(slot_id, 500, DWS_MIME_TEXT_PLAIN, "upload failed");
        return;
    }
    char msg[48];
    snprintf(msg, sizeof(msg), "OK %u bytes", (unsigned)s_upl.written);
    s_upl.server->send(slot_id, 200, DWS_MIME_TEXT_PLAIN, msg);
}

size_t dws_upload_last_size()
{
    return s_upl.written;
}

void dws_upload_begin(DWS &server, const char *path, fs::FS &fs, const char *dest_path)
{
    s_upl.server = &server;
    s_upl.path = path;
    s_upl.fs = &fs;
    s_upl.dest = dest_path;

    http_parser_set_stream_hooks(upload_stream_begin, upload_stream_data);
    server.on(path, HttpMethod::HTTP_POST, upload_handle);
}

#endif // DWS_ENABLE_UPLOAD
