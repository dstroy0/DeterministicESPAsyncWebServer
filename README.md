# DeterministicESPAsyncWebServer

An HTTP/1.1 web server for ESP32 with deterministic memory usage, full RFC 7230 request parsing, and an OSI-layered architecture.

## Features

- **Zero heap allocation after `begin()`** — all buffers are statically sized at compile time
- **RFC 7230 compliant request parser** — validates method, path, header field-names, and field-values byte-by-byte before storing anything
- **Correct automatic error responses** — 400, 413, 414, and 501 sent by the framework without any application code
- **FNV-1a HTTP version detection** — distinguishes HTTP/1.0 from HTTP/1.1 via compile-time hash; exposes `HttpReq::version` to handlers
- **Backpressure-aware TCP** — shrinks the receive window instead of dropping data when the ring buffer fills
- **CORS preflight short-circuit** — OPTIONS requests answered with 204 automatically when CORS is enabled
- **222 tests** across five Unity test suites, runnable on native (no hardware required)

## Architecture

The library maps to OSI layers with no cross-layer leakage:

```
L7  DeterministicESPAsyncWebServer.h/cpp   Route table, dispatch, send()
L6  presentation.h/cpp                      Drains ring buffer → parser
L6  http_parser.h/cpp                       RFC 7230 byte-stream state machine
L5  session.h/cpp                           FreeRTOS event queue drain
L4  transport.h/cpp                         lwIP callbacks, ring buffers, timeouts
L3  network.h/cpp                           lwIP stub (no-op placeholder)
L2  datalink.h/cpp                          Espressif WiFi driver stub (no-op placeholder)
L1  physical.h/cpp                          WiFi.begin() wrapper
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
- CR mid header field-name (not at the start of a new key) → 400
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

## Configuration

All sizing constants live in `src/DetWebServerConfig.h` and can be overridden via build flags:

```ini
# platformio.ini
build_flags =
    -DMAX_CONNS=8
    -DRX_BUF_SIZE=2048
    -DMAX_HEADERS=16
    -DMAX_PATH_LEN=128
    -DBODY_BUF_SIZE=512
    -DMAX_ROUTES=32
```

| Constant | Default | Description |
|---|---|---|
| `MAX_CONNS` | 4 | Simultaneous TCP connections |
| `RX_BUF_SIZE` | 1024 | Ring buffer bytes per connection |
| `CONN_TIMEOUT_MS` | 5000 | Compile-time idle timeout default (ms) |
| `MAX_HEADERS` | 8 | Headers stored per request |
| `MAX_PATH_LEN` | 64 | URL path bytes (including leading `/`) |
| `MAX_KEY_LEN` | 24 | Header field-name bytes |
| `MAX_VAL_LEN` | 48 | Header field-value bytes |
| `MAX_QUERY_LEN` | 128 | Raw query string bytes (after `?`) |
| `MAX_QUERY_PARAMS` | 8 | Parsed query parameters |
| `QUERY_KEY_LEN` | 24 | Query parameter key bytes |
| `QUERY_VAL_LEN` | 48 | Query parameter value bytes |
| `BODY_BUF_SIZE` | 256 | Request body bytes |
| `MAX_ROUTES` | 16 | Registered route handlers |

The connection timeout can also be set at runtime without a rebuild:

```cpp
WebServerConfig cfg = { .conn_timeout_ms = 10000 };
server.begin(80, &cfg);
```

## Quick Start

```cpp
#include <WiFi.h>
#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical.h"

DetWebServer server;

void handle_status(uint8_t slot_id, HttpReq *req)
{
    // req->version is HTTP_10, HTTP_11, or HTTP_UNKNOWN
    const char *body = req->version == HTTP_11
        ? "{\"ok\":true,\"version\":\"1.1\"}"
        : "{\"ok\":true,\"version\":\"1.0\"}";
    server.send(slot_id, 200, "application/json", body);
}

void setup()
{
    init_wifi_physical("SSID", "PASSWORD");
    while (!wifi_ready()) delay(250);

    server.on("/status", HTTP_GET, handle_status);
    server.set_cors("*");

    int32_t result = server.begin(80);
    if (result < 0) {
        // abs(result) bytes of heap were needed but unavailable
    }
}

void loop()
{
    server.handle(); // call every iteration — O(MAX_CONNS) per call
}
```

## API Reference

### `DetWebServer`

| Method | Description |
|---|---|
| `begin(port, cfg = nullptr)` | Open TCP listener. Returns `+1` on success; negative value whose abs = heap bytes needed on failure. |
| `stop()` | Abort all connections, close listener, free queue. `begin()` may be called again after. |
| `on(path, method, handler)` | Register a route. Trailing `*` enables prefix matching (`"/api/*"` matches `"/api/users"`, etc.). |
| `on_not_found(handler)` | Called when no route matches. Default behavior sends 404. |
| `set_cors(origin)` | Enable CORS and intercept OPTIONS with 204. Pass `""` to disable. |
| `handle()` | Call every `loop()` iteration. Runs timeout sweep, event drain, and dispatch. |
| `send(slot_id, code, type, body)` | Send a response with a body and close the connection. |
| `send_empty(slot_id, code)` | Send a headers-only response and close the connection. |

### Handler signature

```cpp
void my_handler(uint8_t slot_id, HttpReq *req)
{
    // read from req, call server.send() or server.send_empty()
}
```

### `HttpReq` fields

| Field | Type | Description |
|---|---|---|
| `method` | `char[8]` | HTTP method string, e.g. `"GET"`. |
| `path` | `char[MAX_PATH_LEN]` | URL path, e.g. `"/api/status"`. |
| `version` | `HttpVersion` | `HTTP_10`, `HTTP_11`, or `HTTP_UNKNOWN`. |
| `query` | `char[MAX_QUERY_LEN]` | Raw query string (everything after `?`). |
| `query_params` | `QueryParam[MAX_QUERY_PARAMS]` | Parsed key=value pairs. |
| `query_count` | `uint8_t` | Valid entries in `query_params[]`. |
| `headers` | `Header[MAX_HEADERS]` | Captured header fields. |
| `header_count` | `uint8_t` | Valid entries in `headers[]`. |
| `content_length` | `size_t` | Value of the `Content-Length` header (0 if absent). |
| `body` | `uint8_t[BODY_BUF_SIZE+1]` | Request body, always null-terminated. |
| `body_len` | `size_t` | Bytes stored in `body[]` (≤ `BODY_BUF_SIZE`). |

### Helper functions

```cpp
// Case-insensitive header lookup
const char *http_get_header(const HttpReq *req, const char *key);

// Case-sensitive query parameter lookup
const char *http_get_query(const HttpReq *req, const char *key);
```

## Testing

The library ships with 222 Unity tests across five suites, all runnable on a native (x86/x64) host — no ESP32 hardware required:

```
pio test -e native -e native_app
```

| Suite | Tests | Coverage |
|---|---|---|
| `test_transport` | 25 | Ring buffer integrity, timeouts, pool lifecycle |
| `test_session` | 19 | Event queue drain, slot lifecycle, millis wraparound |
| `test_presentation` | 61 | Parser integration via ring buffer, race condition simulations |
| `test_http_parser` | 79 | All parser states, RFC 7230 compliance, 413/414, version hash |
| `test_application` | 38 | Route dispatch, CORS, wildcard matching, 400/414/501 auto-responses |

## Installation

**PlatformIO:**
```ini
lib_deps = https://github.com/dstroy0/DeterministicESPAsyncWebServer.git
```

**Arduino IDE:** Copy the repository into your `libraries/` directory.

## License

AGPL-3.0-or-later. See `LICENSE` for details.
