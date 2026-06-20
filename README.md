# DeterministicESPAsyncWebServer

An HTTP/1.1 web server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture.

**[API Documentation](https://dstroy0.github.io/DeterministicESPAsyncWebServer/)**

## Features

- **Zero heap allocation — ever** — the event queue, connection pool, HTTP pool, WebSocket pool, and SSE pool are all statically sized in BSS; the entire memory footprint is fixed at link time
- **RFC 7230 compliant request parser** — validates method, path, header field-names, and field-values byte-by-byte before storing anything
- **WebSocket support** (RFC 6455) — SHA-1 handshake via mbedTLS hardware accelerator, frame parser, ping/pong/close handled automatically
- **Server-Sent Events** — persistent connections, per-connection and broadcast push
- **HTTP Basic Authentication** — per-route credential checking via mbedTLS base64
- **Static file serving** — chunked reads from any Arduino `FS` (LittleFS, SPIFFS, SD)
- **Multipart form-data parser** — in-place, no allocation, up to `MAX_MULTIPART_PARTS` parts
- **Compile-time feature flags** — strip unused subsystems entirely; a REST-only build includes none of the above
- **Compile-time configuration** — every buffer, pool, and timeout is a `#define`; illegal combinations produce `#error` messages
- **Diagnostic JSON endpoint** — optional `DETWS_ENABLE_DIAG` build-config dump, disabled by default for security
- **Backpressure-aware TCP** — shrinks the receive window instead of dropping data when the ring buffer fills
- **CORS preflight short-circuit** — OPTIONS answered with 204 automatically when CORS is enabled
- **`restart()`** — hard-resets all connections and reinitialises on the same port without touching the WiFi/TCP stack
- **321 tests** across nine Unity test suites, runnable on native x86/x64 (no hardware required)

## Architecture

```
L7  DeterministicESPAsyncWebServer.h/cpp   Route table, dispatch, send()
L6  presentation.h/cpp                      Drains ring buffer → parser
L6  http_parser.h/cpp                       RFC 7230 byte-stream state machine
L5  session.h/cpp                           FreeRTOS event queue drain
L4  transport.h/cpp                         lwIP callbacks, ring buffers, timeouts
L3  network.h/cpp                           lwIP stub
L2  datalink.h/cpp                          Espressif WiFi driver stub
L1  physical.h/cpp                          WiFi.begin() wrapper
```

## Zero Heap Allocation

Every byte of memory the library uses is accounted for at compile time:

| Storage | Location |
|---|---|
| `conn_pool[MAX_CONNS]` — TCP connections + ring buffers | BSS |
| `http_pool[MAX_CONNS]` — HTTP request structs | BSS |
| `ws_pool[MAX_WS_CONNS]` — WebSocket connection state | BSS |
| `sse_pool[MAX_SSE_CONNS]` — SSE connection state | BSS |
| `_queue_storage[EVT_QUEUE_DEPTH * sizeof(TcpEvt)]` — event queue backing store | BSS |
| `_queue_struct` — FreeRTOS `StaticQueue_t` | BSS |
| Route table `_routes[MAX_ROUTES]` | BSS (inside `DetWebServer`) |

`begin()` calls `xQueueCreateStatic()` — no `pvPortMalloc`, no fragmentation risk. `DetWebServer::heap_needed()` returns 0 and `heap_available()` returns `true`.

The only post-`begin()` allocation that can occur is inside `fs::File` construction in `serve_file()`, which is an Arduino FS implementation detail outside the library's control.

## Feature Flags

Define these **before** including the library header. Any flag set to `0` strips the corresponding code and its includes from the build entirely.

```cpp
#define DETWS_ENABLE_WEBSOCKET    0  // default 1 — RFC 6455, SHA-1/base64 via mbedTLS
#define DETWS_ENABLE_SSE          0  // default 1 — Server-Sent Events push
#define DETWS_ENABLE_MULTIPART    0  // default 1 — multipart/form-data body parser
#define DETWS_ENABLE_FILE_SERVING 0  // default 1 — static files via Arduino FS
#define DETWS_ENABLE_AUTH         0  // default 1 — HTTP Basic Auth per-route
#define DETWS_ENABLE_DIAG         1  // default 0 — JSON build-config endpoint (disable in production)
#include "DeterministicESPAsyncWebServer.h"
```

Illegal combinations (e.g. `MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS`) produce `#error` messages at compile time with a descriptive reason string.

## Configuration

All constants can be overridden via build flags or `#define` before the library include. Defaults live in `src/DetWebServerConfig.h`.

### Capacity

| Constant | Default | Description |
|---|---|---|
| `MAX_CONNS` | 4 | Simultaneous TCP connections (1–255) |
| `EVT_QUEUE_DEPTH` | 16 | FreeRTOS event queue depth; must be ≥ `MAX_CONNS * 4` |
| `RX_BUF_SIZE` | 1024 | Ring buffer bytes per connection |
| `BODY_BUF_SIZE` | 256 | Request body bytes; must be ≤ `RX_BUF_SIZE` |
| `MAX_ROUTES` | 16 | Registered route handlers |
| `MAX_HEADERS` | 8 | Headers stored per request |
| `MAX_PATH_LEN` | 64 | URL path bytes including leading `/` |
| `MAX_KEY_LEN` | 24 | Header field-name bytes |
| `MAX_VAL_LEN` | 48 | Header field-value bytes |
| `MAX_QUERY_LEN` | 128 | Raw query string bytes (after `?`) |
| `MAX_QUERY_PARAMS` | 8 | Parsed query key=value pairs |
| `QUERY_KEY_LEN` | 24 | Query parameter key bytes |
| `QUERY_VAL_LEN` | 48 | Query parameter value bytes |

### Response buffers

| Constant | Default | Minimum | Description |
|---|---|---|---|
| `RESP_HDR_BUF_SIZE` | 512 | 128 | Stack buffer for HTTP response headers |
| `WS_HDR_BUF_SIZE` | 256 | 128 | Stack buffer for WebSocket 101 response |
| `CORS_HDR_BUF_SIZE` | 192 | 64 | Buffer for pre-built CORS header block; must be ≤ `RESP_HDR_BUF_SIZE` |

### WebSocket (`DETWS_ENABLE_WEBSOCKET`)

| Constant | Default | Description |
|---|---|---|
| `MAX_WS_CONNS` | 2 | WebSocket slots; each consumes one `MAX_CONNS` slot |
| `WS_FRAME_SIZE` | 512 | Max WebSocket frame payload bytes |

### SSE (`DETWS_ENABLE_SSE`)

| Constant | Default | Description |
|---|---|---|
| `MAX_SSE_CONNS` | 2 | SSE slots; each consumes one `MAX_CONNS` slot |
| `SSE_BUF_SIZE` | 256 | Stack buffer for one formatted SSE event |

### File serving (`DETWS_ENABLE_FILE_SERVING`)

| Constant | Default | Description |
|---|---|---|
| `FILE_CHUNK_SIZE` | 512 | Bytes read from FS per `tcp_write()` call; must be ≤ `RX_BUF_SIZE` |

### Auth (`DETWS_ENABLE_AUTH`)

| Constant | Default | Description |
|---|---|---|
| `MAX_AUTH_LEN` | 32 | Max username or password length including null terminator |

### Multipart (`DETWS_ENABLE_MULTIPART`)

| Constant | Default | Description |
|---|---|---|
| `MAX_MULTIPART_PARTS` | 4 | Max form parts per request |
| `MAX_BOUNDARY_LEN` | 72 | Max MIME boundary length |

### Runtime config

The connection idle timeout can be changed without a rebuild:

```cpp
const WebServerConfig cfg PROGMEM = { .conn_timeout_ms = 10000 }; // flash, no RAM cost
server.begin(80, &cfg);
```

Pass `nullptr` (or omit) to use the compile-time default `CONN_TIMEOUT_MS` (5000 ms).

## Quick Start

```cpp
#include <WiFi.h>
#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"

DetWebServer server;

void handle_status(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "application/json", "{\"ok\":true}");
}

void setup()
{
    init_wifi_physical("SSID", "PASSWORD");
    while (!wifi_ready()) delay(250);

    server.on("/status", HTTP_GET, handle_status);
    server.set_cors("*");
    server.begin(80);
}

void loop()
{
    server.handle();
}
```

See `examples/ConfigurationExample/ConfigurationExample.ino` for a full reference of every configurable flag and constant.

## API Reference

### `DetWebServer` — lifecycle

| Method | Description |
|---|---|
| `begin(port, cfg = nullptr)` | Bind and listen. Returns `+1` on success, `-1` on lwIP error. |
| `stop()` | Abort all connections, close listener, reset all pools. |
| `restart(cfg = nullptr)` | `stop()` + `begin()` on the same port. Returns `-1` if called before `begin()`. |
| `handle()` | Call every `loop()`. Runs timeout sweep, event drain, and dispatch. |
| `static heap_needed()` | Returns 0 — no heap allocation. |
| `static heap_available()` | Returns `true` — always safe to call `begin()`. |

### `DetWebServer` — HTTP routes

| Method | Description |
|---|---|
| `on(path, method, handler)` | Register a route. Trailing `*` enables prefix matching. |
| `on(path, method, handler, realm, user, pass)` | Same, with Basic Auth (`DETWS_ENABLE_AUTH`). |
| `on_not_found(handler)` | Fallback handler; default sends 404. |
| `set_cors(origin)` | Enable CORS and answer OPTIONS with 204. Pass `""` to disable. |
| `send(slot_id, code, type, body)` | Send a response with body and close the connection. |
| `send_empty(slot_id, code)` | Send a headers-only response and close the connection. |
| `serve_file(slot_id, fs, path, type)` | Stream a file from an Arduino FS (`DETWS_ENABLE_FILE_SERVING`). |

### `DetWebServer` — WebSocket (`DETWS_ENABLE_WEBSOCKET`)

| Method | Description |
|---|---|
| `on_ws(path, on_connect, on_message, on_close)` | Register a WebSocket route. |
| `ws_send_text(ws_id, text)` | Send a UTF-8 text frame to a client. |
| `ws_send_binary(ws_id, data, len)` | Send a binary frame to a client. |
| `ws_disconnect(ws_id)` | Send Close frame and mark slot for cleanup. |

In `on_message`, read the received payload from `ws_pool[ws_id].buf` (length in `ws_pool[ws_id].payload_len`).

### `DetWebServer` — SSE (`DETWS_ENABLE_SSE`)

| Method | Description |
|---|---|
| `on_sse(path, on_connect)` | Register an SSE route. |
| `sse_send(sse_id, data, event = nullptr, id = nullptr)` | Push an event to one client. |
| `sse_broadcast(path, data, event = nullptr, id = nullptr)` | Push an event to all clients on a path. |

### `DetWebServer` — Diagnostic (`DETWS_ENABLE_DIAG`)

| Method | Description |
|---|---|
| `diag(slot_id)` | Send a JSON object with all active feature flags and configuration constants. Disable in production. |

### Handler signatures

```cpp
// HTTP
void handler(uint8_t slot_id, HttpReq *req);

// WebSocket (DETWS_ENABLE_WEBSOCKET)
void ws_connect(uint8_t ws_id);
void ws_message(uint8_t ws_id);  // payload in ws_pool[ws_id].buf
void ws_close(uint8_t ws_id);

// SSE (DETWS_ENABLE_SSE)
void sse_connect(uint8_t sse_id);
```

### `HttpReq` fields

| Field | Type | Description |
|---|---|---|
| `method` | `char[8]` | HTTP method string, e.g. `"GET"` |
| `path` | `char[MAX_PATH_LEN]` | URL path, e.g. `"/api/status"` |
| `version` | `HttpVersion` | `HTTP_10`, `HTTP_11`, or `HTTP_UNKNOWN` |
| `query` | `char[MAX_QUERY_LEN]` | Raw query string (everything after `?`) |
| `query_params` | `QueryParam[MAX_QUERY_PARAMS]` | Parsed key=value pairs |
| `query_count` | `uint8_t` | Valid entries in `query_params[]` |
| `headers` | `Header[MAX_HEADERS]` | Captured header fields |
| `header_count` | `uint8_t` | Valid entries in `headers[]` |
| `content_length` | `size_t` | Value of `Content-Length` header (0 if absent) |
| `body` | `uint8_t[BODY_BUF_SIZE+1]` | Request body, always null-terminated |
| `body_len` | `size_t` | Bytes stored in `body[]` |

### Helper functions

```cpp
const char *http_get_header(const HttpReq *req, const char *key); // case-insensitive
const char *http_get_query (const HttpReq *req, const char *key); // case-sensitive
```

## RFC Compliance

The HTTP/1.1 parser enforces RFC 7230 rules byte-by-byte during parsing:

| Field | Allowed characters | RFC reference | Violation response |
|---|---|---|---|
| Method | `tchar` (`ALPHA DIGIT ! # $ % & ' * + - . ^ _ \` \| ~`) | §3.1.1 | 400 |
| Path / Query | `VCHAR` (%x21–7E) | RFC 3986 §3.3 | 400 |
| Header field-name | `tchar` | §3.2 | 400 |
| Header field-value | `VCHAR`, SP, HTAB, obs-text (%x80–FF) | §3.2 | 400 |
| Path length | ≤ `MAX_PATH_LEN − 1` bytes | §3.1.1 | 414 |
| Body size | ≤ `BODY_BUF_SIZE` bytes (via `Content-Length`) | §3.3.2 | 413 |
| Transfer-Encoding | Not supported — rejected at dispatch | §3.3.1 | 501 |
| HTTP version | FNV-1a hash match; sets `HttpReq::version` | §2.6 | `HTTP_UNKNOWN` |

Additional behaviors:
- CR mid header field-name → 400
- Leading SP/HTAB in header values stripped per OWS rules (§3.2.3)
- Excess headers beyond `MAX_HEADERS` are consumed and discarded, not rejected
- Query string overflow silently truncates (capacity limit, not a protocol error)

## Automatic Error Responses

`handle()` sends these before dispatching to any route handler:

| Parser state | Response | Trigger |
|---|---|---|
| `PARSE_ERROR` | 400 Bad Request | Any RFC 7230 character violation or malformed CRLF |
| `PARSE_ENTITY_TOO_LARGE` | 413 Payload Too Large | `Content-Length` > `BODY_BUF_SIZE` |
| `PARSE_URI_TOO_LONG` | 414 URI Too Long | Path exceeds `MAX_PATH_LEN − 1` bytes |

`handle()` also sends these during dispatch:

| Condition | Response |
|---|---|
| `Transfer-Encoding` header present | 501 Not Implemented |
| No matching route, no `on_not_found` handler | 404 Not Found |
| WebSocket upgrade on a non-WS route | 400 Bad Request |
| WebSocket or SSE pool full | 503 Service Unavailable |

## Testing

321 Unity tests across nine suites, all runnable on a native x86/x64 host:

```
pio test -e native -e native_app
```

| Suite | Tests | Coverage |
|---|---|---|
| `test_http_parser` | 79 | All parser states, RFC 7230 compliance, 413/414, version hash |
| `test_presentation` | 61 | Parser integration via ring buffer, race condition simulations |
| `test_transport` | 25 | Ring buffer integrity, timeouts, pool lifecycle |
| `test_session` | 19 | Event queue drain, slot lifecycle, millis wraparound |
| `test_websocket` | 38 | Frame parser, masking, control frames, error paths |
| `test_sse` | 21 | Pool lifecycle, event formatting, broadcast |
| `test_auth` | 31 | Base64 decode, credential matching, 401 responses |
| `test_file_serving` | 18 | Chunked send, 404 on missing file, FS stub |
| `test_multipart` | 29 | Boundary parsing, field extraction, stress |

## Documentation

Full API documentation generated by Doxygen: **[https://dstroy0.github.io/DeterministicESPAsyncWebServer/](https://dstroy0.github.io/DeterministicESPAsyncWebServer/)**

To build locally:

```bash
doxygen docs/Doxyfile
# output: docs/html/index.html
```

## Installation

**PlatformIO:**

```ini
lib_deps = https://github.com/dstroy0/DeterministicESPAsyncWebServer.git
```

**Arduino IDE:** Download the repository as a ZIP and use *Sketch → Include Library → Add .ZIP Library*.

## License

AGPL-3.0-or-later. See `LICENSE` for details.
