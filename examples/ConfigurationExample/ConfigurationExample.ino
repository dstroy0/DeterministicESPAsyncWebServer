// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ConfigurationExample.ino
 * @brief Reference for every user-configurable flag and constant.
 *
 * All defines must appear BEFORE the library include.  Any that are omitted
 * get their documented defaults.  Illegal combinations (e.g. pool sizes that
 * exceed MAX_CONNS) produce a #error message in the compiler output.
 *
 * ============================================================
 * FEATURE FLAGS  (set to 0 to strip from the build entirely)
 * ============================================================
 *
 *   DETWS_ENABLE_WEBSOCKET    default 1
 *     WebSocket support: RFC 6455 framing, SHA-1 handshake via mbedTLS,
 *     base64 encode/decode.  Pulls in websocket.h/cpp, sha1.cpp, base64.cpp.
 *     Set to 0 if your project only needs plain HTTP.
 *
 *   DETWS_ENABLE_SSE          default 1
 *     Server-Sent Events: keeps connections open and pushes text events.
 *     Pulls in sse.h/cpp.  Set to 0 if you do not need server push.
 *
 *   DETWS_ENABLE_MULTIPART    default 1
 *     multipart/form-data body parser for file and field uploads.
 *     Pulls in multipart.h/cpp.  Set to 0 for REST-only APIs.
 *
 *   DETWS_ENABLE_FILE_SERVING default 1
 *     Static file serving via any Arduino FS (LittleFS, SPIFFS, SD).
 *     Pulls in FS.h and the serve_file() method.  Set to 0 if you serve
 *     all responses programmatically.
 *
 *   DETWS_ENABLE_AUTH         default 1
 *     HTTP Basic Authentication per-route.  Pulls in base64.cpp.
 *     Set to 0 if all routes are public.
 *
 * ============================================================
 * CAPACITY CONSTANTS  (affect static array sizes → RAM / flash)
 * ============================================================
 *
 *   MAX_CONNS         default 4
 *     Maximum simultaneous TCP connections.  Each slot holds one TcpConn
 *     struct (id, state, pcb pointer, timestamp, RX_BUF_SIZE ring buffer).
 *     Constraint: MAX_WS_CONNS + MAX_SSE_CONNS <= MAX_CONNS.
 *     Range: 1 – 255.
 *
 *   RX_BUF_SIZE       default 1024
 *     Ring-buffer capacity in bytes per connection slot.  Must be >=
 *     BODY_BUF_SIZE and >= FILE_CHUNK_SIZE.  Larger values tolerate bursts
 *     of inbound data without applying TCP backpressure.
 *
 *   MAX_HEADERS       default 8
 *     Maximum HTTP headers stored per request.  Headers beyond this limit
 *     are consumed but not stored (no error, no truncation of the key/value
 *     that was already stored).
 *
 *   MAX_PATH_LEN      default 64
 *     Maximum URL path length including the leading '/'.  Requests whose
 *     path exceeds this receive 414 URI Too Long automatically.
 *     Range: >= 2.
 *
 *   MAX_KEY_LEN       default 24
 *     Maximum header field-name length (e.g. "Content-Type" = 12 chars).
 *     Keys that exceed this for a stored header slot trigger PARSE_ERROR.
 *     Range: >= 4.
 *
 *   MAX_VAL_LEN       default 48
 *     Maximum header field-value length.  Values that exceed this are
 *     silently truncated — no protocol error.  Increase if you expect long
 *     Authorization or Content-Type values.
 *
 *   MAX_QUERY_LEN     default 128
 *     Maximum raw query string length (everything after '?').  Overflow is
 *     silently truncated.
 *
 *   MAX_QUERY_PARAMS  default 8
 *     Maximum number of key=value pairs parsed from the query string.
 *     Extra pairs are silently ignored.
 *
 *   QUERY_KEY_LEN     default 24
 *     Maximum query-parameter key length.
 *
 *   QUERY_VAL_LEN     default 48
 *     Maximum query-parameter value length.
 *
 *   BODY_BUF_SIZE     default 256
 *     Maximum request body stored in HttpReq::body.  Requests with a
 *     Content-Length larger than this receive 413 Payload Too Large
 *     before any body bytes are read.
 *     Constraint: BODY_BUF_SIZE <= RX_BUF_SIZE.
 *
 *   MAX_ROUTES        default 16
 *     Maximum simultaneously registered routes (on(), on_ws(), on_sse()).
 *     Registrations beyond this limit are silently ignored.
 *     Range: >= 1.
 *
 * ============================================================
 * WEBSOCKET CONSTANTS  (DETWS_ENABLE_WEBSOCKET must be 1)
 * ============================================================
 *
 *   MAX_WS_CONNS      default 2
 *     Maximum simultaneous WebSocket connections.  Each consumes one TCP
 *     slot from MAX_CONNS and one WsConn struct (id, state, WS_FRAME_SIZE
 *     payload buffer).
 *     Constraint: MAX_WS_CONNS + MAX_SSE_CONNS <= MAX_CONNS.
 *
 *   WS_FRAME_SIZE     default 512
 *     Maximum WebSocket frame payload in bytes.  Frames larger than this
 *     are rejected with Close code 1009 (Message Too Big).  Fragmented
 *     messages are not supported.
 *     Range: >= 2.
 *
 * ============================================================
 * SSE CONSTANTS  (DETWS_ENABLE_SSE must be 1)
 * ============================================================
 *
 *   MAX_SSE_CONNS     default 2
 *     Maximum simultaneous SSE connections.  Each consumes one TCP slot
 *     from MAX_CONNS and one SseConn struct.
 *     Constraint: MAX_WS_CONNS + MAX_SSE_CONNS <= MAX_CONNS.
 *
 *   SSE_BUF_SIZE      default 256
 *     Output buffer in bytes for one formatted SSE event
 *     ("event: ...\ndata: ...\nid: ...\n\n").  Events that would exceed
 *     this after formatting are not sent (sse_write returns false).
 *     Range: >= 8.
 *
 * ============================================================
 * FILE SERVING CONSTANTS  (DETWS_ENABLE_FILE_SERVING must be 1)
 * ============================================================
 *
 *   FILE_CHUNK_SIZE   default 512
 *     Bytes read from the filesystem and passed to tcp_write() per chunk.
 *     Smaller values reduce peak stack usage; larger values improve
 *     throughput.  Constraint: FILE_CHUNK_SIZE <= RX_BUF_SIZE.
 *
 * ============================================================
 * AUTH CONSTANTS  (DETWS_ENABLE_AUTH must be 1)
 * ============================================================
 *
 *   MAX_AUTH_LEN      default 32
 *     Maximum username or password length including the null terminator.
 *     Credentials longer than this are rejected with 401.
 *     Range: >= 2.
 *
 * ============================================================
 * MULTIPART CONSTANTS  (DETWS_ENABLE_MULTIPART must be 1)
 * ============================================================
 *
 *   MAX_MULTIPART_PARTS  default 4
 *     Maximum number of form parts parsed per request.  Extra parts are
 *     silently ignored.  Range: >= 1.
 *
 *   MAX_BOUNDARY_LEN     default 72
 *     Maximum MIME boundary length.  RFC 2046 allows up to 70 characters;
 *     72 adds two bytes of margin.  Note: the actual boundary available is
 *     further constrained by MAX_VAL_LEN (the stored Content-Type value
 *     must fit within MAX_VAL_LEN including the "multipart/form-data;
 *     boundary=" prefix which is 30 chars).
 *
 * ============================================================
 * RUNTIME CONFIGURATION  (passed to server.begin())
 * ============================================================
 *
 *   WebServerConfig::conn_timeout_ms   default 5000
 *     Milliseconds of inactivity before a connection is force-closed via
 *     tcp_abort().  Can live in flash (PROGMEM) or RAM.
 *
 *     Flash (no RAM cost):
 *       const WebServerConfig cfg PROGMEM = { .conn_timeout_ms = 10000 };
 *
 *     RAM (changeable at runtime):
 *       WebServerConfig cfg = { .conn_timeout_ms = 10000 };
 *
 *     Pass nullptr (or omit) to use the built-in default of 5000 ms.
 */

// -------------------------------------------------------------------
// Example: a low-footprint REST-only server — no WS, SSE, or file IO
// -------------------------------------------------------------------

#define DETWS_ENABLE_WEBSOCKET    0
#define DETWS_ENABLE_SSE          0
#define DETWS_ENABLE_MULTIPART    0
#define DETWS_ENABLE_FILE_SERVING 0
#define DETWS_ENABLE_AUTH         0

// Enable diagnostic endpoint (disable before production deployment)
#define DETWS_ENABLE_DIAG         1

// Tighten capacity to match a small REST API
#define MAX_CONNS        2
#define EVT_QUEUE_DEPTH  8    // >= MAX_CONNS * 4
#define RX_BUF_SIZE      512
#define BODY_BUF_SIZE    128
#define MAX_ROUTES       4
#define MAX_HEADERS      6
#define MAX_PATH_LEN     48
#define MAX_VAL_LEN      64

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"
#include <WiFi.h>

static const char *SSID     = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// Runtime config — 10 s timeout, stored in flash
static const WebServerConfig cfg PROGMEM = { .conn_timeout_ms = 10000 };

DetWebServer server;

void handle_status(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "application/json", "{\"status\":\"ok\"}");
}

void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 404, "text/plain", "Not found");
}

void setup()
{
    Serial.begin(115200);

    // Pre-flight: verify enough contiguous heap exists before WiFi/TCP init
    // consumes more RAM.  heap_needed() = sizeof(StaticQueue_t) +
    // EVT_QUEUE_DEPTH * sizeof(TcpEvt) — the only allocation begin() makes.
    if (!DetWebServer::heap_available())
    {
        Serial.printf("Insufficient contiguous heap: need %u bytes, largest block %u bytes\n",
                      DetWebServer::heap_needed(),
                      heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        return;
    }

    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting");
    while (!wifi_ready()) { delay(250); Serial.print('.'); }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());

    server.on("/status", HTTP_GET, handle_status);

    // Diagnostic route — remove or protect with auth in production
    server.on("/diag", HTTP_GET, [](uint8_t id, HttpReq *) { server.diag(id); });

    server.on_not_found(handle_not_found);

    int32_t result = server.begin(80, &cfg);
    if (result < 0)
    {
        Serial.printf("begin() failed — need %d contiguous heap bytes\n", -result);
        return;
    }
    Serial.println("Server ready on port 80");
}

void loop()
{
    server.handle();
}
