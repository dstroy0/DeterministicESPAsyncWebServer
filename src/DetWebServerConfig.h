// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DetWebServerConfig.h
 * @brief User-facing configuration for DeterministicESPAsyncWebServer.
 *
 * **Compile-time sizing constants**
 * These govern static array dimensions and must be set before the first
 * library header is included.  Define any of them in your sketch or in a
 * build flag before including this file to override the defaults:
 * @code
 *   // platformio.ini
 *   build_flags = -DMAX_CONNS=8 -DBODY_BUF_SIZE=512
 * @endcode
 *
 * **Runtime parameters — flash or RAM, your choice**
 * `WebServerConfig` holds values that can be changed without a rebuild.
 * On ESP32, `PROGMEM` is a no-op (const data lands in DROM automatically).
 * On AVR it places data in flash and requires `pgm_read_*` accessors — this
 * library targets ESP32 only, so both forms read identically via pointer:
 * @code
 *   // Flash (PROGMEM, no RAM cost at runtime):
 *   const WebServerConfig my_cfg PROGMEM = { .conn_timeout_ms = 10000 };
 *
 *   // RAM (can be changed at runtime):
 *   WebServerConfig my_cfg = { .conn_timeout_ms = 10000 };
 *
 *   server.begin(80, &my_cfg);
 * @endcode
 * Pass `nullptr` (or omit the argument) to use `DET_DEFAULT_CONFIG`.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CONFIG_H
#define DETERMINISTICESPASYNCWEBSERVER_CONFIG_H

#include <stdint.h>

// ---------------------------------------------------------------------------
// Compile-time capacity constants (affect static array sizes)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous TCP connections. */
#ifndef MAX_CONNS
#define MAX_CONNS 4
#endif

/** @brief Ring-buffer capacity in bytes per connection slot. */
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1024
#endif

/**
 * @brief Compile-time default for connection idle timeout in milliseconds.
 *
 * The actual runtime value is stored in `WebServerConfig::conn_timeout_ms`
 * and loaded into `DeterministicAsyncTCP::conn_timeout_ms` by init().
 */
#ifndef CONN_TIMEOUT_MS
#define CONN_TIMEOUT_MS 5000
#endif

/** @brief Maximum HTTP headers stored per request. */
#ifndef MAX_HEADERS
#define MAX_HEADERS 8
#endif

/** @brief Maximum URL path length (including leading `/`). */
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 64
#endif

/** @brief Maximum header field-name length (e.g. `"Content-Type"`). */
#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 24
#endif

/** @brief Maximum header field-value length. */
#ifndef MAX_VAL_LEN
#define MAX_VAL_LEN 48
#endif

/** @brief Maximum raw query-string length (everything after `?`). */
#ifndef MAX_QUERY_LEN
#define MAX_QUERY_LEN 128
#endif

/** @brief Maximum number of parsed query-string parameters. */
#ifndef MAX_QUERY_PARAMS
#define MAX_QUERY_PARAMS 8
#endif

/** @brief Maximum query-parameter key length. */
#ifndef QUERY_KEY_LEN
#define QUERY_KEY_LEN 24
#endif

/** @brief Maximum query-parameter value length. */
#ifndef QUERY_VAL_LEN
#define QUERY_VAL_LEN 48
#endif

/**
 * @brief Maximum request body bytes stored in `HttpReq::body`.
 *
 * Bodies larger than this trigger a 413 Payload Too Large response —
 * the parser detects the overflow via `Content-Length` before any body
 * bytes arrive, so no data is read or stored for oversized requests.
 */
#ifndef BODY_BUF_SIZE
#define BODY_BUF_SIZE 256
#endif

/** @brief Maximum simultaneously registered routes. */
#ifndef MAX_ROUTES
#define MAX_ROUTES 16
#endif

// ---------------------------------------------------------------------------
// WebSocket sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous WebSocket connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * ws_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_WS_CONNS
#define MAX_WS_CONNS 2
#endif

/**
 * @brief Maximum WebSocket frame payload in bytes.
 *
 * Frames larger than this are rejected with Close code 1009 (Message Too Big).
 * Fragmented messages are not supported; each message must fit in one frame.
 */
#ifndef WS_FRAME_SIZE
#define WS_FRAME_SIZE 512
#endif

// ---------------------------------------------------------------------------
// Server-Sent Events sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous SSE connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * sse_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_SSE_CONNS
#define MAX_SSE_CONNS 2
#endif

/**
 * @brief Output buffer size in bytes for a single SSE event.
 *
 * An event larger than this is silently truncated.  The buffer holds the
 * formatted `data: ...\n\n` line before it is handed to tcp_write().
 */
#ifndef SSE_BUF_SIZE
#define SSE_BUF_SIZE 256
#endif

// ---------------------------------------------------------------------------
// Static file serving sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Bytes read from the filesystem and passed to tcp_write() per loop().
 *
 * Smaller values reduce peak stack use; larger values improve throughput.
 * Must be <= RX_BUF_SIZE to avoid stalling the TCP send window.
 */
#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 512
#endif

// ---------------------------------------------------------------------------
// Basic Auth sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum username or password length for HTTP Basic Authentication.
 *
 * Both username and password must fit in this many bytes including the
 * null terminator.  Longer credentials are silently rejected with 401.
 */
#ifndef MAX_AUTH_LEN
#define MAX_AUTH_LEN 32
#endif

// ---------------------------------------------------------------------------
// Multipart form-data sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneously parsed multipart parts per request.
 *
 * Parts beyond this limit are silently ignored.  A typical upload form
 * has 1-4 fields; increase this for forms with more.
 */
#ifndef MAX_MULTIPART_PARTS
#define MAX_MULTIPART_PARTS 4
#endif

/**
 * @brief Maximum MIME boundary length (RFC 2046 allows up to 70 characters).
 */
#ifndef MAX_BOUNDARY_LEN
#define MAX_BOUNDARY_LEN 72
#endif

// ---------------------------------------------------------------------------
// Event queue depth
// ---------------------------------------------------------------------------

/**
 * @brief Depth of the FreeRTOS event queue shared between lwIP callbacks and
 *        the main-loop task.
 *
 * Each slot holds one TcpEvt (8 bytes).  The queue is the only heap
 * allocation the library makes at begin() time:
 *
 *   heap = sizeof(StaticQueue_t) + EVT_QUEUE_DEPTH * sizeof(TcpEvt)
 *
 * Must be large enough to absorb a burst of MAX_CONNS * 4 events without
 * blocking the lwIP thread.  DeterministicAsyncTCP::heap_needed() returns
 * the exact byte count; call DetWebServer::heap_available() before begin()
 * to verify a contiguous block exists.
 */
#ifndef EVT_QUEUE_DEPTH
#define EVT_QUEUE_DEPTH 16
#endif

// ---------------------------------------------------------------------------
// Internal response buffer sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Stack buffer for HTTP response header lines in send() / send_empty() /
 *        send_unauth() / serve_file().
 *
 * Must be large enough to hold the status line, Content-Type, Content-Length,
 * Connection, and any CORS headers.  The CORS block alone can reach
 * CORS_HDR_BUF_SIZE bytes, so this value should be at least
 * CORS_HDR_BUF_SIZE + 96.
 */
#ifndef RESP_HDR_BUF_SIZE
#define RESP_HDR_BUF_SIZE 512
#endif

/**
 * @brief Stack buffer for the HTTP 101 Switching Protocols response sent during
 *        the WebSocket handshake.
 *
 * Must hold: status line + Upgrade + Connection + Sec-WebSocket-Accept (28
 * base64 chars) + CRLF pairs.  Minimum is ~120 bytes; default leaves margin.
 */
#ifndef WS_HDR_BUF_SIZE
#define WS_HDR_BUF_SIZE 256
#endif

/**
 * @brief Size of the pre-built CORS header block stored in DetWebServer.
 *
 * Built once by set_cors() and injected into every response.  Must hold
 * Access-Control-Allow-Origin, Access-Control-Allow-Methods, and
 * Access-Control-Allow-Headers lines for the configured origin.
 */
#ifndef CORS_HDR_BUF_SIZE
#define CORS_HDR_BUF_SIZE 192
#endif

// ---------------------------------------------------------------------------
// Feature flags
// ---------------------------------------------------------------------------
// Set any of these to 0 in your sketch BEFORE including this library to strip
// the feature from the build entirely (no code, no RAM, no flash cost).
//
//   #define DETWS_ENABLE_WEBSOCKET 0
//   #include <DeterministicESPAsyncWebServer.h>

/** @brief WebSocket support (RFC 6455 framing + SHA-1/base64 handshake). */
#ifndef DETWS_ENABLE_WEBSOCKET
#define DETWS_ENABLE_WEBSOCKET 1
#endif

/** @brief Server-Sent Events push support. */
#ifndef DETWS_ENABLE_SSE
#define DETWS_ENABLE_SSE 1
#endif

/** @brief multipart/form-data body parser. */
#ifndef DETWS_ENABLE_MULTIPART
#define DETWS_ENABLE_MULTIPART 1
#endif

/** @brief Static file serving via Arduino FS (LittleFS, SPIFFS, SD). */
#ifndef DETWS_ENABLE_FILE_SERVING
#define DETWS_ENABLE_FILE_SERVING 1
#endif

/** @brief HTTP Basic Authentication per-route. */
#ifndef DETWS_ENABLE_AUTH
#define DETWS_ENABLE_AUTH 1
#endif

/**
 * @brief Expose a diagnostic JSON endpoint via server.diag().
 *
 * Disabled by default — enabling it exposes compile-time configuration
 * (buffer sizes, feature flags) which could aid an attacker.  Only
 * enable in development or behind an authenticated route.
 *
 * When enabled, DETWS_DIAG_JSON is a compile-time string constant you can
 * serve from any route handler:
 * @code
 *   server.on("/diag", HTTP_GET, [](uint8_t id, HttpReq *) {
 *       server.diag(id);        // convenience wrapper
 *       // or:
 *       server.send(id, 200, "application/json", DETWS_DIAG_JSON);
 *   });
 * @endcode
 */
#ifndef DETWS_ENABLE_DIAG
#define DETWS_ENABLE_DIAG 0
#endif

// ---------------------------------------------------------------------------
// Runtime configuration struct
// ---------------------------------------------------------------------------

/**
 * @brief Runtime-tunable server parameters.
 *
 * Can be declared as `const PROGMEM` (flash) or as a mutable variable (RAM).
 * Pass a pointer to DetWebServer::begin() or DeterministicAsyncTCP::init().
 */
struct WebServerConfig
{
    /** Milliseconds of inactivity before a connection is force-closed. */
    uint32_t conn_timeout_ms;
};

/** @brief Built-in defaults used when no config is supplied to begin(). */
static const WebServerConfig DET_DEFAULT_CONFIG = {5000u};

// ---------------------------------------------------------------------------
// Diagnostic JSON string  (only defined when DETWS_ENABLE_DIAG == 1)
// ---------------------------------------------------------------------------
// DETWS_DIAG_JSON is a compile-time string literal — zero runtime cost.
// Adjacent string literals are concatenated by the compiler; DETWS_STR()
// stringifies an integer macro value without evaluating it twice.

#if DETWS_ENABLE_DIAG

#define _DETWS_STR_(x) #x
#define _DETWS_STR(x)  _DETWS_STR_(x)

#if DETWS_ENABLE_WEBSOCKET
#define _DETWS_F_WS "true"
#else
#define _DETWS_F_WS "false"
#endif

#if DETWS_ENABLE_SSE
#define _DETWS_F_SSE "true"
#else
#define _DETWS_F_SSE "false"
#endif

#if DETWS_ENABLE_MULTIPART
#define _DETWS_F_MP "true"
#else
#define _DETWS_F_MP "false"
#endif

#if DETWS_ENABLE_FILE_SERVING
#define _DETWS_F_FS "true"
#else
#define _DETWS_F_FS "false"
#endif

#if DETWS_ENABLE_AUTH
#define _DETWS_F_AUTH "true"
#else
#define _DETWS_F_AUTH "false"
#endif

#define DETWS_DIAG_JSON \
    "{"                                                                    \
    "\"lib\":\"DeterministicESPAsyncWebServer\","                          \
    "\"features\":{"                                                       \
      "\"websocket\":"    _DETWS_F_WS    ","                              \
      "\"sse\":"          _DETWS_F_SSE   ","                              \
      "\"multipart\":"    _DETWS_F_MP    ","                              \
      "\"file_serving\":" _DETWS_F_FS    ","                              \
      "\"auth\":"         _DETWS_F_AUTH                                   \
    "},"                                                                   \
    "\"config\":{"                                                         \
      "\"MAX_CONNS\":"       _DETWS_STR(MAX_CONNS)       ","              \
      "\"RX_BUF_SIZE\":"     _DETWS_STR(RX_BUF_SIZE)     ","              \
      "\"BODY_BUF_SIZE\":"   _DETWS_STR(BODY_BUF_SIZE)   ","              \
      "\"MAX_ROUTES\":"      _DETWS_STR(MAX_ROUTES)       ","              \
      "\"MAX_HEADERS\":"     _DETWS_STR(MAX_HEADERS)      ","              \
      "\"MAX_PATH_LEN\":"    _DETWS_STR(MAX_PATH_LEN)     ","              \
      "\"MAX_KEY_LEN\":"     _DETWS_STR(MAX_KEY_LEN)      ","              \
      "\"MAX_VAL_LEN\":"     _DETWS_STR(MAX_VAL_LEN)      ","              \
      "\"MAX_QUERY_LEN\":"   _DETWS_STR(MAX_QUERY_LEN)    ","              \
      "\"MAX_QUERY_PARAMS\":" _DETWS_STR(MAX_QUERY_PARAMS) ","             \
      "\"CONN_TIMEOUT_MS\":"    _DETWS_STR(CONN_TIMEOUT_MS)    ","          \
      "\"RESP_HDR_BUF_SIZE\":" _DETWS_STR(RESP_HDR_BUF_SIZE) ","          \
      "\"WS_HDR_BUF_SIZE\":"   _DETWS_STR(WS_HDR_BUF_SIZE)   ","          \
      "\"CORS_HDR_BUF_SIZE\":"  _DETWS_STR(CORS_HDR_BUF_SIZE)  ","         \
      "\"EVT_QUEUE_DEPTH\":"   _DETWS_STR(EVT_QUEUE_DEPTH)                \
    "}"                                                                    \
    "}"

#endif // DETWS_ENABLE_DIAG

// ---------------------------------------------------------------------------
// Compile-time sanity checks
// ---------------------------------------------------------------------------
// These produce a clear #error message in the compiler output rather than a
// cryptic linker failure or silent misbehaviour.

#if EVT_QUEUE_DEPTH < MAX_CONNS * 4
#error "DeterministicESPAsyncWebServer: EVT_QUEUE_DEPTH must be >= MAX_CONNS * 4 to absorb event bursts without blocking lwIP"
#endif

#if MAX_CONNS < 1
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be >= 1"
#endif

#if MAX_CONNS > 255
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be <= 255 (slot IDs are uint8_t)"
#endif

#if DETWS_ENABLE_WEBSOCKET && DETWS_ENABLE_SSE
#if MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_WEBSOCKET
#if MAX_WS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_SSE
#if MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#endif

#if BODY_BUF_SIZE < 1
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must be >= 1"
#endif

#if BODY_BUF_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must not exceed RX_BUF_SIZE (parser reads from the ring buffer)"
#endif

#if DETWS_ENABLE_FILE_SERVING && FILE_CHUNK_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: FILE_CHUNK_SIZE must not exceed RX_BUF_SIZE"
#endif

#if MAX_KEY_LEN < 4
#error "DeterministicESPAsyncWebServer: MAX_KEY_LEN must be >= 4 (minimum valid HTTP header name length)"
#endif

#if MAX_VAL_LEN < 1
#error "DeterministicESPAsyncWebServer: MAX_VAL_LEN must be >= 1"
#endif

#if MAX_PATH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_PATH_LEN must be >= 2 (minimum: \"/\")"
#endif

#if MAX_ROUTES < 1
#error "DeterministicESPAsyncWebServer: MAX_ROUTES must be >= 1"
#endif

#if DETWS_ENABLE_AUTH && MAX_AUTH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_AUTH_LEN must be >= 2 when DETWS_ENABLE_AUTH is set"
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_FRAME_SIZE < 2
#error "DeterministicESPAsyncWebServer: WS_FRAME_SIZE must be >= 2 when DETWS_ENABLE_WEBSOCKET is set"
#endif

#if DETWS_ENABLE_SSE && SSE_BUF_SIZE < 8
#error "DeterministicESPAsyncWebServer: SSE_BUF_SIZE must be >= 8 when DETWS_ENABLE_SSE is set"
#endif

#if DETWS_ENABLE_MULTIPART && MAX_MULTIPART_PARTS < 1
#error "DeterministicESPAsyncWebServer: MAX_MULTIPART_PARTS must be >= 1 when DETWS_ENABLE_MULTIPART is set"
#endif

#if RESP_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= 128 (status line + headers + CORS block)"
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: WS_HDR_BUF_SIZE must be >= 128 when DETWS_ENABLE_WEBSOCKET is set"
#endif

#if CORS_HDR_BUF_SIZE < 64
#error "DeterministicESPAsyncWebServer: CORS_HDR_BUF_SIZE must be >= 64"
#endif

#if RESP_HDR_BUF_SIZE < CORS_HDR_BUF_SIZE
#error "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= CORS_HDR_BUF_SIZE (CORS block is injected into response headers)"
#endif

#endif
