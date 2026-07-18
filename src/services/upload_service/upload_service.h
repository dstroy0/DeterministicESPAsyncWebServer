// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file upload_service.h
 * @brief Streaming file upload to an Arduino FS (DWS_ENABLE_UPLOAD).
 *
 * Registers a POST route whose request body is streamed straight into a file on
 * a filesystem (LittleFS / SPIFFS / SD) in FILE_CHUNK_SIZE pieces - the upload
 * never has to fit in RAM. Reuses the parser's streaming-body hook (the same
 * mechanism OTA uses), so it is zero-heap and bounded.
 *
 * One upload at a time (the device runs a single loop task). Only one streaming
 * sink can be installed, so DWS_ENABLE_UPLOAD and DWS_ENABLE_OTA share the
 * parser hook - register whichever you need (not both on the same build).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_UPLOAD_SERVICE_H
#define DETERMINISTICESPASYNCWEBSERVER_UPLOAD_SERVICE_H

#include "ServerConfig.h"

#if DWS_ENABLE_UPLOAD

#include <FS.h>
#include <stddef.h>

class DWS;

/**
 * @brief Register a streaming-upload endpoint.
 *
 * A `POST @p path` request streams its body into @p dest_path on @p fs (the file
 * is truncated/created). The route handler replies `200 OK <n> bytes` on success
 * or 500 on a write failure.
 *
 * @param server    the web server.
 * @param path      the upload URL (e.g. "/upload").
 * @param fs        target filesystem (LittleFS / SPIFFS / SD).
 * @param dest_path destination file path (e.g. "/uploads/data.bin").
 */
void dws_upload_begin(DWS &server, const char *path, fs::FS &fs, const char *dest_path);

/** @brief Bytes written by the most recent upload (for handlers / tests). */
size_t dws_upload_last_size();

#endif // DWS_ENABLE_UPLOAD

#endif // DETERMINISTICESPASYNCWEBSERVER_UPLOAD_SERVICE_H
