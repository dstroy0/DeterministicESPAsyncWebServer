# Documentation

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1, WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, SNMP, and CoAP.

![Version](https://img.shields.io/badge/version-v2.5.0-blue)
[![Test Build Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml)
[![Docs Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml)
[![Changelog Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml)
[![C++ Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml)
[![Markdown Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml)

## Features

- **Zero heap allocation - ever** - the event queue, connection pool, HTTP pool, WebSocket/SSE pools, TLS arena, and SNMP buffers are all statically sized in BSS; the entire memory footprint is fixed at link time
- **RFC 7230 compliant request parser** - validates method, path, header field-names, and field-values byte-by-byte before storing anything
- **Flexible routing** - exact, wildcard (`/*`), `:param` path parameters, bounded allocation-free regex routes ([`on_regex()`](@ref DetWebServer::on_regex)), and per-interface STA/softAP route filters
- **Request data** - query-string, `x-www-form-urlencoded` form fields, in-place `multipart/form-data` uploads (up to [`MAX_MULTIPART_PARTS`](@ref MAX_MULTIPART_PARTS)), and a zero-heap JSON writer/reader (`json.h`)
- **Response tools** - custom headers + cookies, CORS with preflight, `{{var}}` templating ([`send_template()`](@ref DetWebServer::send_template)), and chunked/streaming responses ([`send_chunked()`](@ref DetWebServer::send_chunked)) of unbounded length in constant memory
- **WebSocket support** (RFC 6455) - SHA-1 handshake via mbedTLS hardware accelerator, frame parser, ping/pong/close handled automatically; plus a browser web-serial terminal over WebSocket
- **Server-Sent Events** - persistent connections, per-connection and broadcast push
- **Authentication** - per-route HTTP Basic (RFC 7617) and Digest (RFC 7616, SHA-256, `qop=auth`)
- **Static file serving** - chunked reads from any Arduino `FS` (LittleFS, SPIFFS, SD), `index.html` fallback, MIME detection, pre-compressed `.gz`, and `ETag`/`304` conditional GET
- **HTTPS / TLS** - optional ([`DETWS_ENABLE_TLS`](@ref DETWS_ENABLE_TLS)) mbedTLS over a fixed static memory pool, so encrypted transport keeps the no-heap guarantee; optional **session resumption** (RFC 5077 tickets, [`DETWS_ENABLE_TLS_RESUMPTION`](@ref DETWS_ENABLE_TLS_RESUMPTION)) gives a returning client an abbreviated handshake while the server stays stateless
- **Mutual TLS (mTLS)** - optional ([`DETWS_ENABLE_MTLS`](@ref DETWS_ENABLE_MTLS)) client-certificate authentication: the handshake requires a client cert chaining to a configured CA and exposes the verified peer subject DN to handlers
- **Secure WebSocket & SSE** - WebSocket (`wss://`) and Server-Sent Events run over the TLS record layer when TLS is enabled: the upgrade and every frame/event are encrypted, transparent to handler code
- **Prometheus metrics** - optional ([`DETWS_ENABLE_METRICS`](@ref DETWS_ENABLE_METRICS)) `/metrics` endpoint in Prometheus text exposition format (0.0.4): uptime, request/response-class counters, active/max connections, free heap - point a Prometheus server straight at the device
- **Syslog client** - optional ([`DETWS_ENABLE_SYSLOG`](@ref DETWS_ENABLE_SYSLOG)) RFC 5424 remote logging over UDP: ship structured log lines (facility/severity, hostname, app-name) to a central syslog server, zero-heap and fire-and-forget
- **JWT bearer auth** - optional ([`DETWS_ENABLE_JWT`](@ref DETWS_ENABLE_JWT)) stateless `Authorization: Bearer` verification (HS256, constant-time signature check, integer-claim access) - gate routes on a shared-secret token with no server-side session
- **SSH 2.0 server** - zero-heap SSH with host-key verification and password/publickey auth (hardware-accelerated crypto)
- **Telnet console** (RFC 854) - plaintext line console with IAC negotiation + server echo, for trusted networks
- **Modbus TCP slave** - optional ([`DETWS_ENABLE_MODBUS`](@ref DETWS_ENABLE_MODBUS)) Modbus Application Protocol server on TCP/502: a fixed data model (coils, discrete inputs, holding + input registers) served via FC 1/2/3/4/5/6/15/16 with exception responses; the MBAP/PDU codec is host-tested, dispatched through the PROTO_MODBUS handler
- **SNMP agent** - v1/v2c plus optional v3 USM (HMAC-SHA-256 + AES-128) over UDP, with a zero-heap ASN.1 BER codec and fixed MIB
- **SNMP notifications** - optional ([`DETWS_ENABLE_SNMP_TRAP`](@ref DETWS_ENABLE_SNMP_TRAP)) outbound Traps / InformRequests: [`snmp_trap_v2c()`](@ref snmp_trap_v2c) / [`snmp_inform_v2c()`](@ref snmp_inform_v2c), and SNMPv3 USM authPriv [`snmp_trap_v3()`](@ref snmp_trap_v3) reusing the agent engine + keys; each carries `sysUpTime.0` + `snmpTrapOID.0` plus caller varbinds; host-tested PDU builder
- **CoAP server** (RFC 7252) - optional ([`DETWS_ENABLE_COAP`](@ref DETWS_ENABLE_COAP)) zero-heap Constrained Application Protocol over UDP: a fixed resource table dispatched on Uri-Path, GET/POST/PUT/DELETE with piggybacked responses, Uri-Query and Content-Format options; optional resource **Observe** (RFC 7641, [`DETWS_ENABLE_COAP_OBSERVE`](@ref DETWS_ENABLE_COAP_OBSERVE)) - a GET + Observe option subscribes, [`coap_notify()`](@ref coap_notify) pushes updates to observers; optional **block-wise transfer** (RFC 7959, [`DETWS_ENABLE_COAP_BLOCK`](@ref DETWS_ENABLE_COAP_BLOCK)) - Block2 pages a large representation, Block1 reassembles a chunked upload
- **HTTP keep-alive** - optional ([`DETWS_ENABLE_KEEPALIVE`](@ref DETWS_ENABLE_KEEPALIVE)) HTTP/1.1 persistent connections: one TCP connection serves many requests (bounded by a per-connection request cap and the idle timeout), transparent to handler code
- **HTTP Range requests** - optional ([`DETWS_ENABLE_RANGE`](@ref DETWS_ENABLE_RANGE)) `206 Partial Content` (RFC 7233) for served files: single-range `Range: bytes=...` streams only the requested bytes (resumable downloads, media seeking), with `Accept-Ranges` advertisement and `416` for unsatisfiable ranges
- **WebDAV** (RFC 4918) - optional ([`DETWS_ENABLE_WEBDAV`](@ref DETWS_ENABLE_WEBDAV)) read/write file share: one-call [`dav()`](@ref DetWebServer::dav) subtree mount answering OPTIONS, PROPFIND (Depth 0/1, 207 Multi-Status), GET, HEAD, PUT, DELETE (recursive), MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK; the 207 builder and header parsing are host-tested, the FS handling HW-tested
- **Middleware & rate limiting** - composable [`use()`](@ref DetWebServer::use) pipeline, fixed-window rate limiter, an opt-in global accept-throttle, and an opt-in per-source-IP accept-throttle ([`DETWS_ENABLE_PER_IP_THROTTLE`](@ref DETWS_ENABLE_PER_IP_THROTTLE)) that bounds reconnect churn from a single host
- **mDNS & NTP services** - zero-dependency multicast DNS hostname advertisement and SNTP wall-clock time synchronization
- **OTA updates** - secure, authenticated over-the-air firmware updates via streaming POST request body
- **Streaming uploads** - optional ([`DETWS_ENABLE_UPLOAD`](@ref DETWS_ENABLE_UPLOAD)) POST body streamed straight into a filesystem file (LittleFS / SPIFFS / SD); the upload never has to fit in RAM
- **Outbound HTTP(S) client** - optional ([`DETWS_ENABLE_HTTP_CLIENT`](@ref DETWS_ENABLE_HTTP_CLIENT)) zero-heap requests _from_ the device (webhooks, telemetry push, REST calls): blocking [`http_get()`](@ref http_get) / [`http_post()`](@ref http_post) over raw lwIP with DNS, Content-Length / chunked decoding, and `https://` via client-side mbedTLS over the same static arena ([`DETWS_ENABLE_HTTP_CLIENT_TLS`](@ref DETWS_ENABLE_HTTP_CLIENT_TLS)) - encrypt-only by default, with optional server authentication via a CA ([`http_client_set_ca()`](@ref http_client_set_ca)) or SHA-256 cert pin ([`http_client_set_pin()`](@ref http_client_set_pin)); links no code unless a sketch calls it
- **MQTT client (3.1.1)** - optional ([`DETWS_ENABLE_MQTT`](@ref DETWS_ENABLE_MQTT)) persistent publish/subscribe client: connect with Last-Will + credentials, [`mqtt_publish()`](@ref mqtt_publish) / [`mqtt_subscribe()`](@ref mqtt_subscribe) / [`mqtt_unsubscribe()`](@ref mqtt_unsubscribe) at QoS 0/1/2 (complete acknowledgement flows, bounded in-flight DUP retransmit, inbound QoS-2 de-dup), inbound messages via a callback, keep-alive; `mqtts://` over client TLS ([`DETWS_ENABLE_MQTT_TLS`](@ref DETWS_ENABLE_MQTT_TLS)); host-tested packet codec
- **WebSocket client** (RFC 6455) - optional ([`DETWS_ENABLE_WS_CLIENT`](@ref DETWS_ENABLE_WS_CLIENT)) outbound client: [`ws_client_connect()`](@ref ws_client_connect) does the `Sec-WebSocket-Key`/`Accept` handshake, [`ws_client_send_text()`](@ref ws_client_send_text) / [`ws_client_send_binary()`](@ref ws_client_send_binary) send masked frames, inbound frames arrive via a callback, ping/pong + fragmentation handled; `wss://` over client TLS ([`DETWS_ENABLE_WS_CLIENT_TLS`](@ref DETWS_ENABLE_WS_CLIENT_TLS)); host-tested frame/handshake codec
- **Captive portal provisioning** - setup wizard (SoftAP + DNS portal) for first-boot WiFi credential configuration
- **Compile-time feature flags & configuration** - every subsystem is gated by a `DETWS_ENABLE_*` flag (default off) and every buffer/pool/timeout is a `#define`; illegal combinations produce `#error` messages
- **Diagnostic JSON endpoint** - optional [`DETWS_ENABLE_DIAG`](@ref DETWS_ENABLE_DIAG) build-config dump, disabled by default for security
- **Lossless TCP backpressure** - when a connection's receive ring is full, an oversized segment is refused (lwIP retains it as `refused_data` and redelivers once the app drains the ring) rather than dropped, so no received byte is ever lost; all socket operations are marshaled to the lwIP thread to stay race-free
- **[`restart()`](@ref DetWebServer::restart)** - hard-resets all connections and reinitialises on the same port without touching the WiFi/TCP stack
- **Native-testable** - the protocol/parser/codec logic runs on host x86/x64 under Unity (no hardware required)

## Build Footprint

Measured on `esp32dev` (Arduino core, `pio ci`). The baseline → server jump is almost entirely the WiFi/lwIP stack; the library and most HTTP features add little on top. Each indented row enables one optional subsystem over the default server.

| Build                                                                               | Flash (bytes) | RAM (bytes) |
| ----------------------------------------------------------------------------------- | ------------: | ----------: |
| Empty sketch (no WiFi, no library) - _RTOS/Arduino baseline_                        |       233,257 |      21,032 |
| Minimal REST server (WS/SSE/multipart/file/auth stripped)                           |       734,745 |      57,936 |
| **Default server** (HTTP + WebSocket + SSE + multipart + file serving + Basic auth) |       745,133 |      64,264 |
| &nbsp;&nbsp;+ HTTPS / TLS (static-pool mbedTLS)                                     |       847,185 |     115,164 |
| &nbsp;&nbsp;+ SSH 2.0 server                                                        |       798,005 |      76,556 |
| &nbsp;&nbsp;+ SNMP agent (v1/v2c)                                                   |       751,277 |      76,648 |
| &nbsp;&nbsp;+ CoAP server (RFC 7252, UDP)                                           |       747,921 |      66,760 |
| &nbsp;&nbsp;+ mDNS                                                                  |       768,037 |      66,160 |
| &nbsp;&nbsp;+ SNTP                                                                  |       768,861 |      66,808 |
| &nbsp;&nbsp;+ OTA update                                                            |       748,417 |      64,544 |
| &nbsp;&nbsp;+ Captive-portal provisioning                                           |       750,709 |      65,836 |
| &nbsp;&nbsp;+ Static files via LittleFS (incl. ETag)                                |       784,361 |      64,288 |
| &nbsp;&nbsp;+ Telnet console                                                        |       745,137 |      64,784 |
| &nbsp;&nbsp;+ Web terminal (WebSocket)                                              |       747,613 |      64,336 |
| SSH crypto self-test (Serial only, no WiFi)                                         |       269,585 |      21,476 |

TLS's larger RAM is the fixed mbedTLS arena ([`DETWS_TLS_ARENA_SIZE`](@ref DETWS_TLS_ARENA_SIZE), 48 KB default). Small HTTP features (CORS, JSON, middleware, regex / path / form params, templating, chunked, response headers, Digest auth, stats, diagnostics, accept-throttle) stay within a few KB of the default server. The outbound HTTP client ([`DETWS_ENABLE_HTTP_CLIENT`](@ref DETWS_ENABLE_HTTP_CLIENT)) links no code unless a sketch actually calls [`http_get()`](@ref http_get) / [`http_post()`](@ref http_post); the standalone client example builds to 732,961 B flash / 46,752 B RAM, and adding `https://` (which pulls in mbedTLS) makes it 827,853 B / 100,620 B. The MQTT client ([`DETWS_ENABLE_MQTT`](@ref DETWS_ENABLE_MQTT)) example builds to 734,293 B flash / 48,896 B RAM; `mqtts://` makes it 830,285 B / 108,732 B. The WebSocket client ([`DETWS_ENABLE_WS_CLIENT`](@ref DETWS_ENABLE_WS_CLIENT)) example builds to 734,329 B / 48,824 B; `wss://` makes it 830,165 B / 108,660 B. ESP32 capacity: 1,310,720 B flash / 327,680 B RAM.

## Installation

**PlatformIO:**

```ini
lib_deps = https://github.com/dstroy0/DeterministicESPAsyncWebServer.git
```

**Arduino IDE:** Download the repository as a ZIP and use _Sketch → Include Library → Add .ZIP Library_.

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

See `examples/05.Configuration/05.Configuration.ino` for a full reference of every configurable flag and constant.

## Architecture

Each OSI layer lives in its own subdirectory under `src/network_drivers/`:

<details>
<summary><b>View Directory and OSI Layer Layout</b></summary>

```
L7  src/DeterministicESPAsyncWebServer.h/cpp     Route table, dispatch, send()
L6  src/network_drivers/presentation/
        presentation.h/cpp                        Drains ring buffer → parser
        http_parser.h/cpp                         RFC 7230 byte-stream state machine
        sha1.h/cpp  base64.h/cpp                  mbedTLS hardware-accelerated helpers
        websocket.h/cpp  sse.h/cpp                WS frame parser; SSE connection pool
        multipart.h/cpp                           Multipart form-data parser
L5  src/network_drivers/session/
        session.h/cpp                             FreeRTOS event queue drain
L4  src/network_drivers/transport/
        transport.h/cpp                           lwIP callbacks, ring buffers, timeouts
        listener.h/cpp                            Per-port TCP listener, per-listener queue
L3  src/network_drivers/network/
        network.h/cpp                             lwIP stub
L2  src/network_drivers/datalink/
        datalink.h/cpp                            Espressif WiFi driver stub
L1  src/network_drivers/physical/
        physical.h/cpp                            WiFi.begin() wrapper
```

</details>

## Zero Heap Allocation

Every byte of memory the library uses is accounted for at compile time:

<details>
<summary><b>View Zero Heap Allocation Storage Details</b></summary>

| Storage                                                                        | Location                                         |
| ------------------------------------------------------------------------------ | ------------------------------------------------ |
| `conn_pool[MAX_CONNS]` - TCP connections + ring buffers                        | BSS                                              |
| `http_pool[MAX_CONNS]` - HTTP request structs                                  | BSS                                              |
| `ws_pool[MAX_WS_CONNS]` - WebSocket connection state                           | BSS                                              |
| `sse_pool[MAX_SSE_CONNS]` - SSE connection state                               | BSS                                              |
| `_queue_storage[EVT_QUEUE_DEPTH * sizeof(TcpEvt)]` - event queue backing store | BSS                                              |
| `_queue_struct` - FreeRTOS `StaticQueue_t`                                     | BSS                                              |
| Route table `_routes[MAX_ROUTES]`                                              | BSS (inside [`DetWebServer`](@ref DetWebServer)) |

</details>

[`begin()`](@ref DetWebServer::begin) calls `xQueueCreateStatic()` - no `pvPortMalloc`, no fragmentation risk. [`DetWebServer::heap_needed()`](@ref DetWebServer::heap_needed) returns 0 and [`heap_available()`](@ref DetWebServer::heap_available) returns `true`.

The only post-`begin()` allocation that can occur is inside `fs::File` construction in `serve_file()`, which is an Arduino FS implementation detail outside the library's control.

## Feature Flags & Configuration

> [!IMPORTANT]
> **Use Build Flags (`-D...`), Not Sketch `#define`s!**
>
> Because PlatformIO (and standard Arduino IDE builds) compiles the library's source files (`.cpp`) independently from your sketch (`.ino` / `.cpp`), `#define` macros inside your sketch files **do not propagate** to the library's pre-compiled objects.
>
> Declaring configuration or feature macros like `#define DETWS_ENABLE_PROVISIONING 1` inside your `.ino` sketch file before the `#include` will result in configuration mismatches, linker errors (such as undefined symbols), or unstable behavior at runtime.
>
> To enable/disable features or override configuration constants, you **must** pass them as compiler build flags. For example, in PlatformIO, define them inside `platformio.ini` under `build_flags`:
>
> ```ini
> [env:esp32dev]
> platform = espressif32
> board = esp32dev
> framework = arduino
> build_flags =
>     -DDETWS_ENABLE_PROVISIONING=1
>     -DDETWS_ENABLE_WEBSOCKET=0
>     -DMAX_CONNS=6
> ```

Any feature flag set to `0` strips the corresponding code and its includes from the build entirely.

### Feature Flags

Here are the available compile-time feature flags and their default values:

| Flag                                                          | Default | Description                                                   |
| :------------------------------------------------------------ | :------ | :------------------------------------------------------------ |
| [`DETWS_ENABLE_WEBSOCKET`](@ref DETWS_ENABLE_WEBSOCKET)       | `1`     | WebSocket support (RFC 6455, SHA-1/base64 via mbedTLS)        |
| [`DETWS_ENABLE_SSE`](@ref DETWS_ENABLE_SSE)                   | `1`     | Server-Sent Events push support                               |
| [`DETWS_ENABLE_MULTIPART`](@ref DETWS_ENABLE_MULTIPART)       | `1`     | `multipart/form-data` body parser                             |
| [`DETWS_ENABLE_FILE_SERVING`](@ref DETWS_ENABLE_FILE_SERVING) | `1`     | Static file serving via Arduino `FS`                          |
| [`DETWS_ENABLE_AUTH`](@ref DETWS_ENABLE_AUTH)                 | `1`     | HTTP Basic Auth per-route                                     |
| `DETWS_ENABLE_DIAG`                                           | `0`     | JSON build-config diagnostic endpoint (disable in production) |
| [`DETWS_ENABLE_MDNS`](@ref DETWS_ENABLE_MDNS)                 | `0`     | mDNS/DNS-SD advertisement via ESPmDNS                         |
| [`DETWS_ENABLE_NTP`](@ref DETWS_ENABLE_NTP)                   | `0`     | SNTP wall-clock time synchronization                          |
| [`DETWS_ENABLE_OTA`](@ref DETWS_ENABLE_OTA)                   | `0`     | Authenticated OTA firmware updates                            |
| [`DETWS_ENABLE_PROVISIONING`](@ref DETWS_ENABLE_PROVISIONING) | `0`     | WiFi provisioning wizard (SoftAP + captive portal)            |
| [`DETWS_ENABLE_TELNET`](@ref DETWS_ENABLE_TELNET)             | `0`     | RFC 854 Telnet server                                         |
| [`DETWS_ENABLE_SSH`](@ref DETWS_ENABLE_SSH)                   | `0`     | RFC 4253/4252/4254 SSH server                                 |

Illegal combinations (e.g. `MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS`) produce `#error` messages at compile time with a descriptive reason string.

## Configuration Overrides

All constants can be overridden using compiler build flags (e.g. `-DMAX_CONNS=6`). Default limits and sizes reside in [DetWebServerConfig.h](@ref DetWebServerConfig.h).

<details>
<summary><b>Expand Configuration constants and options</b></summary>

**Capacity**

| Constant                                    | Default | Description                                           |
| ------------------------------------------- | ------- | ----------------------------------------------------- |
| [`MAX_CONNS`](@ref MAX_CONNS)               | 4       | Simultaneous TCP connections (1–255)                  |
| [`EVT_QUEUE_DEPTH`](@ref EVT_QUEUE_DEPTH)   | 16      | FreeRTOS event queue depth; must be ≥ `MAX_CONNS * 4` |
| [`RX_BUF_SIZE`](@ref RX_BUF_SIZE)           | 1024    | Ring buffer bytes per connection                      |
| [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE)       | 256     | Request body bytes; must be ≤ `RX_BUF_SIZE`           |
| [`MAX_ROUTES`](@ref MAX_ROUTES)             | 16      | Registered route handlers                             |
| [`MAX_HEADERS`](@ref MAX_HEADERS)           | 8       | Headers stored per request                            |
| [`MAX_PATH_LEN`](@ref MAX_PATH_LEN)         | 64      | URL path bytes including leading `/`                  |
| [`MAX_KEY_LEN`](@ref MAX_KEY_LEN)           | 24      | Header field-name bytes                               |
| [`MAX_VAL_LEN`](@ref MAX_VAL_LEN)           | 48      | Header field-value bytes                              |
| [`MAX_QUERY_LEN`](@ref MAX_QUERY_LEN)       | 128     | Raw query string bytes (after `?`)                    |
| [`MAX_QUERY_PARAMS`](@ref MAX_QUERY_PARAMS) | 8       | Parsed query key=value pairs                          |
| [`QUERY_KEY_LEN`](@ref QUERY_KEY_LEN)       | 24      | Query parameter key bytes                             |
| [`QUERY_VAL_LEN`](@ref QUERY_VAL_LEN)       | 48      | Query parameter value bytes                           |

**Response Buffers**

| Constant                                      | Default | Minimum | Description                                                           |
| --------------------------------------------- | ------- | ------- | --------------------------------------------------------------------- |
| [`RESP_HDR_BUF_SIZE`](@ref RESP_HDR_BUF_SIZE) | 512     | 128     | Stack buffer for HTTP response headers                                |
| [`WS_HDR_BUF_SIZE`](@ref WS_HDR_BUF_SIZE)     | 256     | 128     | Stack buffer for WebSocket 101 response                               |
| [`CORS_HDR_BUF_SIZE`](@ref CORS_HDR_BUF_SIZE) | 192     | 64      | Buffer for pre-built CORS header block; must be ≤ `RESP_HDR_BUF_SIZE` |

**WebSocket (DETWS_ENABLE_WEBSOCKET)**

| Constant                              | Default | Description                                         |
| ------------------------------------- | ------- | --------------------------------------------------- |
| [`MAX_WS_CONNS`](@ref MAX_WS_CONNS)   | 2       | WebSocket slots; each consumes one `MAX_CONNS` slot |
| [`WS_FRAME_SIZE`](@ref WS_FRAME_SIZE) | 512     | Max WebSocket frame payload bytes                   |

**SSE (DETWS_ENABLE_SSE)**

| Constant                              | Default | Description                                   |
| ------------------------------------- | ------- | --------------------------------------------- |
| [`MAX_SSE_CONNS`](@ref MAX_SSE_CONNS) | 2       | SSE slots; each consumes one `MAX_CONNS` slot |
| [`SSE_BUF_SIZE`](@ref SSE_BUF_SIZE)   | 256     | Stack buffer for one formatted SSE event      |

**File Serving (DETWS_ENABLE_FILE_SERVING)**

| Constant                                  | Default | Description                                                        |
| ----------------------------------------- | ------- | ------------------------------------------------------------------ |
| [`FILE_CHUNK_SIZE`](@ref FILE_CHUNK_SIZE) | 512     | Bytes read from FS per `tcp_write()` call; must be ≤ `RX_BUF_SIZE` |

**Auth (DETWS_ENABLE_AUTH)**

| Constant                            | Default | Description                                               |
| ----------------------------------- | ------- | --------------------------------------------------------- |
| [`MAX_AUTH_LEN`](@ref MAX_AUTH_LEN) | 32      | Max username or password length including null terminator |

**Multipart (DETWS_ENABLE_MULTIPART)**

| Constant                                    | Default | Description                |
| ------------------------------------------- | ------- | -------------------------- |
| `MAX_MULTIPART_PARTS`                       | 4       | Max form parts per request |
| [`MAX_BOUNDARY_LEN`](@ref MAX_BOUNDARY_LEN) | 72      | Max MIME boundary length   |

**Runtime Config**

The connection idle timeout can be changed without a rebuild:

```cpp
const WebServerConfig cfg PROGMEM = { .conn_timeout_ms = 10000 }; // flash, no RAM cost
server.begin(80, &cfg);
```

Pass `nullptr` (or omit) to use the compile-time default [`CONN_TIMEOUT_MS`](@ref CONN_TIMEOUT_MS) (5000 ms).

</details>

## API Reference

<details>
<summary><b>Expand API Reference</b></summary>

**DetWebServer - Lifecycle**

| Method                                  | Description                                                                     |
| --------------------------------------- | ------------------------------------------------------------------------------- |
| `begin(port, cfg = nullptr)`            | Bind and listen. Returns `+1` on success, `-1` on lwIP error.                   |
| [`stop()`](@ref DetWebServer::stop)     | Abort all connections, close listener, reset all pools.                         |
| `restart(cfg = nullptr)`                | `stop()` + `begin()` on the same port. Returns `-1` if called before `begin()`. |
| [`handle()`](@ref DetWebServer::handle) | Call every `loop()`. Runs timeout sweep, event drain, and dispatch.             |
| `static heap_needed()`                  | Returns 0 - no heap allocation.                                                 |
| `static heap_available()`               | Returns `true` - always safe to call `begin()`.                                 |

**DetWebServer - HTTP Routes**

| Method                                         | Description                                                     |
| ---------------------------------------------- | --------------------------------------------------------------- |
| `on(path, method, handler)`                    | Register a route. Trailing `*` enables prefix matching.         |
| `on(path, method, handler, realm, user, pass)` | Same, with Basic Auth (`DETWS_ENABLE_AUTH`).                    |
| `on_not_found(handler)`                        | Fallback handler; default sends 404.                            |
| `set_cors(origin)`                             | Enable CORS and answer OPTIONS with 204. Pass `""` to disable.  |
| `send(slot_id, code, type, body)`              | Send a response with body and close the connection.             |
| `send_empty(slot_id, code)`                    | Send a headers-only response and close the connection.          |
| `serve_file(slot_id, fs, path, type)`          | Stream a file from an Arduino FS (`DETWS_ENABLE_FILE_SERVING`). |

**DetWebServer - WebSocket (DETWS_ENABLE_WEBSOCKET)**

| Method                                          | Description                                 |
| ----------------------------------------------- | ------------------------------------------- |
| `on_ws(path, on_connect, on_message, on_close)` | Register a WebSocket route.                 |
| `ws_send_text(ws_id, text)`                     | Send a UTF-8 text frame to a client.        |
| `ws_send_binary(ws_id, data, len)`              | Send a binary frame to a client.            |
| `ws_disconnect(ws_id)`                          | Send Close frame and mark slot for cleanup. |

In `on_message`, read the received payload from `ws_pool[ws_id].buf` (length in `ws_pool[ws_id].payload_len`).

**DetWebServer - SSE (DETWS_ENABLE_SSE)**

| Method                                                     | Description                             |
| ---------------------------------------------------------- | --------------------------------------- |
| `on_sse(path, on_connect)`                                 | Register an SSE route.                  |
| `sse_send(sse_id, data, event = nullptr, id = nullptr)`    | Push an event to one client.            |
| `sse_broadcast(path, data, event = nullptr, id = nullptr)` | Push an event to all clients on a path. |

**DetWebServer - Diagnostic (DETWS_ENABLE_DIAG)**

| Method          | Description                                                                                          |
| --------------- | ---------------------------------------------------------------------------------------------------- |
| `diag(slot_id)` | Send a JSON object with all active feature flags and configuration constants. Disable in production. |

**Handler Signatures**

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

**HttpReq Fields**

| Field            | Type                              | Description                                                                                  |
| ---------------- | --------------------------------- | -------------------------------------------------------------------------------------------- |
| `method`         | `char[8]`                         | HTTP method string, e.g. `"GET"`                                                             |
| `path`           | `char[MAX_PATH_LEN]`              | URL path, e.g. `"/api/status"`                                                               |
| `version`        | [`HttpVersion`](@ref HttpVersion) | [`HTTP_10`](@ref HTTP_10), [`HTTP_11`](@ref HTTP_11), or [`HTTP_UNKNOWN`](@ref HTTP_UNKNOWN) |
| `query`          | `char[MAX_QUERY_LEN]`             | Raw query string (everything after `?`)                                                      |
| `query_params`   | `QueryParam[MAX_QUERY_PARAMS]`    | Parsed key=value pairs                                                                       |
| `query_count`    | `uint8_t`                         | Valid entries in `query_params[]`                                                            |
| `headers`        | `Header[MAX_HEADERS]`             | Captured header fields                                                                       |
| `header_count`   | `uint8_t`                         | Valid entries in `headers[]`                                                                 |
| `content_length` | `size_t`                          | Value of `Content-Length` header (0 if absent)                                               |
| `body`           | `uint8_t[BODY_BUF_SIZE+1]`        | Request body, always null-terminated                                                         |
| `body_len`       | `size_t`                          | Bytes stored in `body[]`                                                                     |

**Helper Functions**

```cpp
const char *http_get_header(const HttpReq *req, const char *key); // case-insensitive
const char *http_get_query (const HttpReq *req, const char *key); // case-sensitive
```

</details>

## RFC Compliance

The HTTP/1.1 parser enforces RFC 7230 byte-by-byte; the dispatcher returns the
correct status codes (400/404/405/413/414/426/501) with `Allow`/`Sec-WebSocket-Version`
headers where required; the WebSocket layer enforces RFC 6455 framing rules.

See **[RFC.md](RFC.md)** for the full conformance tables (HTTP, WebSocket,
automatic error responses).

## SSH Support

DeterministicESPAsyncWebServer includes a **complete SSH-2.0 server protocol** -
banner exchange → `KEXINIT` negotiation → DH-group14-SHA256 key exchange →
`NEWKEYS` → user authentication (**publickey** and password) → `ssh-connection`
session channel, with transparent in-session re-keys. All state is static (BSS),
the RSA private key never touches static memory, and password auth can be
compiled out (`DETWS_SSH_ALLOW_PASSWORD=0`) for publickey-only hardening.

See **[SSH.md](SSH.md)** for the feature summary, RFC/FIPS compliance
table, authentication/hardening details, and memory footprint, and
**[SECURITY.md](SECURITY.md)** for the security treatment.

## Memory Table

<details>
<summary><b>Memory Usage Breakdown Table</b></summary>

```
Feature            Pool / symbol                    Size (bytes)    Notes
────────────────── ──────────────────────────────── ─────────────── ──────────────────────────
Transport          conn_pool[4 × (1024 + 22)]             4,184     Ring bufs + TCP state
                   listener_pool[3 × ~218]                  654     Per-port listener state
                   _queue_storage[16 × sizeof(TcpEvt)]      384     FreeRTOS queue backing
                   _queue_struct (StaticQueue_t)             ~88     FreeRTOS struct

HTTP               http_pool[4 × ~1,667]                  6,668     HttpReq + body buf

WebSocket          ws_pool[2 × (512 + 29)]                1,082     WS frame + state

SSE                sse_pool[2 × (64 + 3)]                   134     SSE path + state

SSH                ssh_pkt[2 × ~1,034]                    2,068     Pkt state + RX buf
                   ssh_keys[2 × ~240]                       480     AES CTR ctx + MAC keys
                   ssh_dh[2 × ~800]                        1,600     DH scalars + H
                   crypto_work[1536]                        1,536     Bignum scratch (zeroed)
                   group14_p + group14_g                     512     RFC 3526 constants
                   ssh_host_pubkey                            260     RSA-2048 public key

Application        _routes[16 × ~32]                        512     Route table

────────────────── ──────────────────────────────── ─────────────── ──────────────────────────
GRAND TOTAL (all features, default config)               ~20,162 B  ≈ 20 KB
```

</details>

### Default Build Footprint

A default compiled build of the minimal web server (including the Arduino/ESP-IDF framework core, ESP32 WiFi drivers, lwIP TCP/IP stack, and all library features active) consumes:

- **Flash Footprint**: **307,857 bytes (~307.9 KB)** (approx. **23.5%** of a standard 1.3 MB application partition).
- **RAM Footprint (BSS + Core RAM)**: **39,264 bytes (~39.3 KB)** (approx. **12.0%** of the 320 KB internal SRAM).
- **Zero Dynamic Allocations**: The library requests exactly **0 bytes** of heap memory after `begin()`, managing all session and packet buffers statically (including event queues created via `xQueueCreateStatic()`).

ESP32 has 320 KB of DRAM; this library consumes < 7% of that at maximum configuration.

## Utility Tools

A set of Python utility tools for formatting documentation, managing the test report directories, and styling/compressing web page assets.

<details>
<summary><b>Expand Utility Tools and Scripts Guide</b></summary>

**1. Interactive Theme Wizard**

The wizard guides developers through styling choices, compiles the customized assets, and prints the gzipped C++ hex array to the console.

```bash
python src/utilities/theme_wizard.py
```

**2. HTML Beautification and Decoration Tool**

Processes raw HTML source files to beautify, minify, or inject modern premium dark-mode styling.

```bash
# Beautify, minify, and inject CSS theme
python src/utilities/process_html.py --input src/html/index.html --output src/html/index_processed.html --minify --decorate-css
```

**3. Gzip HTML to Hex Array Converter**

Compresses a processed HTML template and outputs a C++ hex byte array in a timestamped text file to prevent naming collisions.

```bash
python src/utilities/gzip_html_to_hex.py --input src/html/index_processed.html
```

**4. Test Documentation Deep Dive Generator**

Scans Unity C++ test suites and auto-generates a nested, collapsible directory of test cases inside `TEST_DOCUMENTATION.md`.

```bash
python docs/utilities/generate_deep_dive.py
```

**5. Changelog Collapsible Decorator**

Parses `CHANGELOG.md` and wraps individual release versions in collapsible details sections. Used dynamically inside the CI pipeline.

```bash
python docs/utilities/decorate_changelog.py
```

</details>

## Testing

**498 Unity tests across 19 suites**, all runnable on a native x86/x64 host (no
hardware required):

```
pio test -e native -e native_app -e native_ssh \
         -e native_ssh_hardened -e native_ssh_conn -e native_compliance
```

See **[TEST_DOCUMENTATION.md](TEST_DOCUMENTATION.md)** for the suite
breakdown and environment descriptions, and
**[TEST_REPORT.md](TEST_REPORT.md)** for the latest results
(auto-generated by the _Test Report_ GitHub workflow).

## Documentation

Other documentation files in this repository:

<details>
<summary><b>View Documentation Reference Directory</b></summary>

| Document                                       | Contents                                                          |
| ---------------------------------------------- | ----------------------------------------------------------------- |
| [RFC.md](RFC.md)                               | HTTP/1.1, WebSocket, and error-response RFC conformance tables    |
| [SSH.md](SSH.md)                               | SSH-2.0 server: features, RFC/FIPS compliance, auth, memory       |
| [SECURITY.md](SECURITY.md)                     | Security posture (good/ok/bad) and per-feature security treatment |
| [CODEQL.md](CODEQL.md)                         | CodeQL static-analysis setup, coverage, and findings disposition  |
| [TEST_DOCUMENTATION.md](TEST_DOCUMENTATION.md) | Test suites, environments, and how to run them                    |
| [TEST_REPORT.md](TEST_REPORT.md)               | Latest test results (auto-generated)                              |
| [TODO.md](TODO.md)                             | Outstanding fixes and maintenance                                 |
| [ROADMAP.md](ROADMAP.md)                       | Forward-looking feature backlog (sized S/M/L)                     |
| [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md)   | Deliberate constraints and caveats                                |
| [CHANGELOG.md](CHANGELOG.md)                   | Release history                                                   |

</details>

### Generating Docs Locally

To generate the HTML API documentation locally, run the following command from the repository root:

```bash
doxygen docs/Doxyfile
```

The output will be generated in `docs/html/index.html`.

If you are viewing the offline version of this documentation, you can access the latest online version at the [GitHub Pages documentation site](https://dstroy0.github.io/DeterministicESPAsyncWebServer/).

## License

AGPL-3.0-or-later. See `LICENSE` for details.

---

<p align="center">
  <img src="squirty.svg" alt="Squirty the Injection Squid" width="64" height="64"><br>
  <b>Squirty the Injection Squid</b>: the official library mascot.<br>
  <sub>Copyright &copy; Douglas Quigg (dstroy0). All rights reserved.</sub>
</p>
