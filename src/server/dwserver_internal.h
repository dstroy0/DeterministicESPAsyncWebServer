// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dwserver_internal.h
 * @brief Library-private declarations shared between dwserver.cpp and the src/server/*.cpp
 *        request-handler translation units it is split into (WebDAV, file serving, ...).
 *
 * These are NOT part of the public API - they are the handful of dwserver.cpp file-scope helpers
 * that a split-out handler still needs, promoted from `static` to external linkage so the pieces
 * link together. Everything else a handler needs is either a public DetWebServer method (declared
 * in dwserver.h) or already an extern in the transport headers (e.g. conn_pool in tcp.h).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H
#define DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H

#include "dwserver.h"
#include <time.h>

/** @brief Reason phrase for an HTTP status code (e.g. 404 -> "Not Found"). */
const char *status_text(int code);

/** @brief Initialize the common fields (path, flags) of a route-table entry from its pattern. */
void fill_route_base(Route *r, const char *path);

/** @brief Format @p t as an RFC 1123 GMT date into @p out (cap bytes); @p out is emptied for t <= 0. */
void http_rfc1123(time_t t, char *out, size_t cap);

/** @brief True if the request in slot @p slot_id used the HEAD method (send headers, no body). */
bool req_is_head(uint8_t slot_id);

// ---------------------------------------------------------------------------
// Outbound-transfer continuations (owned by dwserver.cpp, shared with the split handlers)
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_FILE_SERVING
// Cross-loop file-send continuation. A file response larger than the TCP send
// buffer cannot be blasted out in one dispatch (tcp_write returns ERR_MEM once the
// window fills and the rest would be dropped). Instead serve_file_internal sends
// the headers, opens the file, and hands it to this per-slot state; file_send_pump
// pages out at most det_conn_sndbuf() bytes per worker loop and resumes on the next
// loop (woken promptly by the sent callback) as the window drains - no truncation,
// no blocking the worker. One transfer per slot at a time.
struct FileSend
{
    fs::File file;    ///< open source file (held across loops).
    size_t off;       ///< absolute file offset of the next byte to send.
    size_t remaining; ///< body bytes still to send.
    int status;       ///< response status (200 / 206) for note_response.
    int total;        ///< total body length, for the access log.
    bool keep;        ///< keep-alive vs close at completion.
    bool active;      ///< a transfer is in progress on this slot.
};
#endif

// Per-slot chunked-send continuation. Mirrors FileSend but pulls body pieces from
// a ChunkSource generator and adds the HTTP chunk framing; paged across loops.
struct ChunkSend
{
    ChunkSource source; ///< body generator (active==false means none).
    void *ctx;          ///< caller state passed to source (must outlive the send).
    int status;         ///< response status, for note_response.
    int total;          ///< body bytes emitted so far (excludes framing).
    bool keep;          ///< keep-alive vs close at completion.
    bool active;        ///< a chunked response is in progress on this slot.
    bool raw;           ///< HTTP/1.0 client: stream the body unframed, close-delimited (no chunk wrapping).
};

// All per-slot outbound-transfer continuations, owned by one instance: the cross-loop file-send
// state (when file serving is enabled) and the chunked-send state. Grouped so it is one named
// owner. Defined once in dwserver.cpp; the file_serving / chunked handler TUs reference it.
struct SendCtx
{
#if DETWS_ENABLE_FILE_SERVING
    FileSend file[MAX_CONNS];
#endif
    ChunkSend chunk[MAX_CONNS];
};
extern SendCtx s_send;

// ---------------------------------------------------------------------------
// WebSocket / SSE upgrade entry points (defined in server/websocket_sse.cpp, called
// by the route dispatcher in dwserver.cpp when a matched route is a WS/SSE endpoint).
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_WEBSOCKET
/** @brief Perform the RFC 6455 101 handshake and hand the slot to the WS frame parser. */
bool ws_do_upgrade(uint8_t slot_id, HttpReq *req, WsConnectHandler on_connect);

/** @brief Reject an unsupported Sec-WebSocket-Version with a 426 (RFC 6455 4.2.1) and close. */
void ws_send_version_required(uint8_t slot_id);
#endif

#if DETWS_ENABLE_SSE
/** @brief Send the SSE 200 headers and promote the slot to server-sent-events mode. */
bool sse_do_upgrade(uint8_t slot_id, HttpReq *req, SseConnectHandler on_connect);
#endif

#endif // DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H
