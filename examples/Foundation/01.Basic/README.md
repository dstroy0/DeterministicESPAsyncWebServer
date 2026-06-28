# 01.Basic - core DetWebServer walkthrough

**Layer:** Foundation (tutorial) · **Build flags:** none (core features only)

## What this example teaches

This is the smallest complete server in the library and the right place to start.
It brings up WiFi, registers a handful of routes, and then services HTTP requests
forever. Nothing here is optional or gated behind a feature flag: HTTP/1.1
parsing, routing, query/header access, CORS, and the automatic error responses
are always compiled in.

**The shape of every sketch.** A DetWebServer program is three things: one global
server object, a `setup()` that connects WiFi and registers routes before calling
`begin()`, and a `loop()` that pumps the pipeline:

```cpp
DetWebServer server; // one global; all state lives in BSS, no heap

void setup() { /* connect WiFi, register routes */ server.begin(80); }
void loop()  { server.handle(); } // the heartbeat: every request flows through here
```

`handle()` is the heartbeat. Each call sweeps idle connections, drains the TCP
event queue, dispatches completed requests to your handlers, and emits the
automatic protocol error responses. No request is processed off this call, so
`loop()` must never block.

**The handler signature.** Every route handler takes the connection's `slot_id`
and the parsed request, and replies with `server.send()`:

```cpp
void handle_root(uint8_t slot_id, HttpReq *req)
{
    server.send(slot_id, 200, "text/plain", "Welcome to DeterministicESPAsyncWebServer!");
}
```

`slot_id` identifies the connection you are answering (pass it back to `send()`);
`req` carries the method, path, version, headers, query parameters, and body.
Every buffer in `req` is fixed-size and bounds-checked at parse time, and
`req->body` is always null-terminated, so handlers never allocate.

**Reading request data.** The status route branches on the negotiated HTTP
version and an optional query parameter, then builds JSON into a fixed stack
buffer with `snprintf` (never a heap `String`):

```cpp
const char *verbose = http_get_query(req, "verbose"); // value of ?verbose=, or nullptr
switch (req->version) { case HTTP_11: /* "1.1" */ break; case HTTP_10: /* "1.0" */ break; }

char body[192];
snprintf(body, sizeof(body), "{\"status\":\"ok\",\"count\":%lu,...}", request_count);
server.send(slot_id, 200, "application/json", body);
```

**The request body.** The echo route shows that `req->body` is a null-terminated
C string and `req->body_len` is its length, with a guard for the empty case:

```cpp
if (req->body_len == 0) { server.send(slot_id, 400, "text/plain", "Error: request body is empty"); return; }
server.send(slot_id, 200, "text/plain", (const char *)req->body);
```

**Wildcards and header lookup.** A trailing `*` makes a route a prefix match, and
`http_get_header()` reads a request header by name (case-insensitive). Note
`req->path` is the **full** path, not just the part after the wildcard:

```cpp
server.on("/files/*", HTTP_GET, handle_files);          // matches /files/anything
const char *accept_enc = http_get_header(req, "Accept-Encoding");
```

**A custom 404.** `on_not_found()` installs a fallback for any unmatched route.

**What the framework does for you.** You write no code for malformed input. The
parser auto-sends `400 Bad Request` for an RFC 7230 character violation,
`413 Payload Too Large` when `Content-Length` exceeds `BODY_BUF_SIZE`,
`414 URI Too Long` when the path exceeds `MAX_PATH_LEN`, and `501 Not Implemented`
when a `Transfer-Encoding` header is present (chunked request bodies are not
accepted). Those limits are the compile-time capacity constants in
[DetWebServerConfig.h](../../../src/DetWebServerConfig.h).

**Checking `begin()`.** It returns `1` on success and a negative error code on
failure. Because `-1` is "truthy", test `result < 0`, not `!result`:

```cpp
int32_t result = server.begin(80);
if (result < 0) { Serial.printf("begin() failed (error %d)\n", result); return; }
```

## Build and run

This example needs no feature flags. Set `SSID` / `PASSWORD` in the sketch first,
then compile for an ESP32 board:

```sh
pio ci --board=esp32dev \
  --project-option="framework=arduino" \
  --lib="." examples/Foundation/01.Basic/01.Basic.ino
```

Flash it from a PlatformIO project (`pio run -t upload`), open the serial monitor
at 115200 baud to read the assigned IP, then exercise the routes:

```sh
curl http://<ip>/
curl http://<ip>/api/status
curl "http://<ip>/api/status?verbose=1"
curl -X POST http://<ip>/api/echo -d "hello world"
curl http://<ip>/files/image.png
```

## Annotated source

The complete sketch ([01.Basic.ino](01.Basic.ino)), reproduced verbatim with
added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// The main library header. Including it pulls in the DetWebServer class, the
// HttpReq type, the HTTP_GET/HTTP_POST method constants, and the accessor
// helpers (http_get_query / http_get_header). Any DETWS_ENABLE_* overrides must
// be #defined BEFORE this include to take effect in the sketch.
#include "DeterministicESPAsyncWebServer.h"
// init_wifi_physical() / wifi_ready(): the physical-layer (L1) WiFi bring-up
// helpers. They wrap the Arduino WiFi join so the example stays terse.
#include "network_drivers/physical/physical.h"
#include <WiFi.h> // WiFi.localIP() for printing the assigned address

// Credentials. Replace these with your network before flashing.
static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

// The server instance. One global object owns all connection state; every
// buffer it uses lives in BSS, sized at compile time. There is no heap use.
DetWebServer server;

// Simple application state shared across handlers. Single-threaded with respect
// to handlers (they run from server.handle() in loop()), so no locking needed.
static unsigned long request_count = 0;

// GET /
// The simplest possible handler: take the connection's slot_id, send a status,
// content-type, and body. server.send() owns the response framing.
void handle_root(uint8_t slot_id, HttpReq *req)
{
    request_count++;
    server.send(slot_id, 200, "text/plain", "Welcome to DeterministicESPAsyncWebServer!");
}

// GET /api/status
// Builds a JSON object into a fixed stack buffer with snprintf (never a heap
// String). Shows reading request metadata: the HTTP version and a query param.
void handle_status(uint8_t slot_id, HttpReq *req)
{
    request_count++;

    // req->version is the version the client sent; map it to a display string.
    const char *version_str;
    switch (req->version)
    {
    case HTTP_11:
        version_str = "1.1";
        break;
    case HTTP_10:
        version_str = "1.0";
        break;
    default:
        version_str = "?";
        break;
    }

    // http_get_query() returns the value of a query-string parameter, or nullptr
    // if it is absent. Here: /api/status?verbose=1
    const char *verbose = http_get_query(req, "verbose");

    // 192 bytes is comfortably larger than the largest response below. If you add
    // fields, grow this and watch it against the stack.
    char body[192];
    if (verbose && strcmp(verbose, "1") == 0)
        // Verbose form also reports the request path (req->path).
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%lu,\"http\":\"%s\",\"uptime_ms\":%lu,\"free_heap\":%u,\"path\":\"%s\"}",
                 request_count, version_str, millis(), ESP.getFreeHeap(), req->path);
    else
        snprintf(body, sizeof(body),
                 "{\"status\":\"ok\",\"count\":%lu,\"http\":\"%s\",\"uptime_ms\":%lu,\"free_heap\":%u}", request_count,
                 version_str, millis(), ESP.getFreeHeap());

    server.send(slot_id, 200, "application/json", body);
}

// POST /api/echo
// Echoes the request body. req->body is always null-terminated and req->body_len
// is the byte count, so it is safe to treat body as a C string here.
void handle_echo(uint8_t slot_id, HttpReq *req)
{
    if (req->body_len == 0)
    {
        // Reject an empty body with a 400 instead of echoing nothing.
        server.send(slot_id, 400, "text/plain", "Error: request body is empty");
        return;
    }
    server.send(slot_id, 200, "text/plain", (const char *)req->body);
}

// GET /files/* (wildcard)
// Placeholder for static file serving (see the FileServing example for the real
// thing). req->path is the FULL path, not just the suffix after "/files/".
void handle_files(uint8_t slot_id, HttpReq *req)
{
    // http_get_header() looks up a request header by name (case-insensitive).
    const char *accept_enc = http_get_header(req, "Accept-Encoding");
    bool wants_gzip = accept_enc && strstr(accept_enc, "gzip") != nullptr;

    // Size the buffer off the configured max path length plus room for the label.
    char msg[MAX_PATH_LEN + 64];
    snprintf(msg, sizeof(msg), "Requested: %s%s", req->path, wants_gzip ? " (gzip accepted)" : "");
    server.send(slot_id, 200, "text/plain", msg);
}

// Fallback for any route that did not match. Installed via on_not_found().
void handle_not_found(uint8_t slot_id, HttpReq *req)
{
    char msg[MAX_PATH_LEN + 48];
    // req->method and req->path let you report exactly what was requested.
    snprintf(msg, sizeof(msg), "Not found: %s %s", req->method, req->path);
    server.send(slot_id, 404, "text/plain", msg);
}

void setup()
{
    Serial.begin(115200);

    // Bring up WiFi (station mode) and block until the link + IP are ready.
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());

    // Add a permissive CORS policy. The header block is built once and injected
    // into every response. Use your real frontend origin in production.
    server.set_cors("*");

    // Register routes BEFORE begin(). The route table is fixed-capacity
    // (MAX_ROUTES) and is immutable once the server is serving.
    server.on("/", HTTP_GET, handle_root);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/echo", HTTP_POST, handle_echo);
    server.on("/files/*", HTTP_GET, handle_files); // trailing * = wildcard match
    server.on_not_found(handle_not_found);

    // Start listening on port 80. Returns 1 on success, negative on failure.
    // -1 is truthy, so test "< 0" rather than "!result".
    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    // The single pipeline pump. Every iteration it:
    //   1. sweeps and force-closes idle connections (the timeout sweep),
    //   2. drains the TCP event queue (connect / data / disconnect),
    //   3. dispatches any completed request to its route handler,
    //   4. auto-sends 400 / 413 / 414 / 501 for parser error states.
    // Keep loop() non-blocking so this runs promptly.
    server.handle();
}
```
