# Documentation

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1, WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, SNMP, CoAP, Modbus TCP, MQTT, and OPC UA.

> [!WARNING]
> **Extremely active development - expect breaking changes.** This library ships fast: on a busy day that can mean dozens of new features and several public-API breaks. **We fix things the right way and put security and correctness first, even when that breaks backwards compatibility** - include paths, method signatures, defaults, and wire behavior can change between releases. **We do not write backwards-compatibility shims** (the only compatibility we maintain is platform/toolchain support); removing cruft is the price of a clean, auditable, deterministic core. Pin an exact version if you need stability, and read [CHANGELOG.md](CHANGELOG.md) and [MIGRATION.md](MIGRATION.md) before every upgrade.

![Version](https://img.shields.io/badge/version-v4.10.0-blue)
[![Test Build Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml)
[![Docs Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml)
[![Changelog Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml)
[![C++ Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml)
[![Markdown Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml)

## Features

A compile-time menu: each cell is an optional `DETWS_ENABLE_*` subsystem (core HTTP/1.1, routing, middleware, JSON, templating, and chunked responses are always on). **Hover a feature for its summary; click through to [FEATURES.md](FEATURES.md) for the full description.**

| Feature | Feature | Feature | Feature | Feature |
|---|---|---|---|---|
| [Accept Throttle](FEATURES.md#accept-throttle "Opt-in global accept-rate throttle (connection-flood defense). Default off (zero cost / no behavior change). When set to 1 the accept callback rejects new connections once more than DETWS_ACCEPT_THROTTLE_MAX have been accepted within a DETWS_ACCEPT_THROTTLE_WINDOW_MS fixed window (global across all listeners, two static counters - no per-IP table). This bounds connection churn (e.g") | [Audit Log](FEATURES.md#audit-log "Tamper-evident audit log") | [Auth](FEATURES.md#auth "HTTP Basic Authentication per-route") | [Auth Lockout](FEATURES.md#auth-lockout "Opt-in per-IP brute-force lockout for HTTP auth (requires AUTH). Default off (zero cost / no behavior change). When set, the auth gate counts consecutive failed authentications per source IPv4 in a fixed BSS table; after DETWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for DETWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to DETWS_AUTH_LOCKOUT_MAX_MS") | [CBOR](FEATURES.md#cbor "Zero-heap CBOR (RFC 8949) encoder for compact binary payloads") |
| [Chunked Responses](FEATURES.md#chunked-responses "Streaming / chunked responses of unbounded length in constant memory via send_chunked(). Always on") | [CoAP](FEATURES.md#coap "CoAP server (RFC 7252) over UDP/5683") | [CoAP Block](FEATURES.md#coap-block "CoAP block-wise transfer - RFC 7959 (requires COAP). Default off") | [CoAP Observe](FEATURES.md#coap-observe "CoAP resource observation - RFC 7641 (requires COAP). Default off") | [Config IO](FEATURES.md#config-io "Opt-in schema-driven config export / restore") |
| [Config Store](FEATURES.md#config-store "Typed NVS configuration store (WiFi creds, IP config,... as blobs). When set, src/services/config_store/config_store.h provides a typed key/value API (string / u32 / blob) that routes core settings into the ESP32's native NVS partition (via `Preferences`) instead of a JSON file on the filesystem - which survives FS corruption and is the corruption-resistant home for credentials") | [CORS](FEATURES.md#cors "Cross-origin resource sharing with automatic preflight handling") | [CSRF](FEATURES.md#csrf "Opt-in CSRF protection for state-changing HTTP requests") | [Dashboard](FEATURES.md#dashboard "Real-time SVG dashboard (DASHBOARD; requires SSE). Default off") | [Device ID](FEATURES.md#device-id "Stable device UUID derived from the chip MAC (RFC 4122 v5). When set, src/services/device_id/device_id.h derives a deterministic v5 UUID from a MAC (via the library's SHA-1) - a storage-free, stable identity for mDNS hostnames, MQTT client IDs, etc") |
| [Diag](FEATURES.md#diag "Expose a diagnostic JSON endpoint via server.diag(). Disabled by default - enabling it exposes compile-time configuration (buffer sizes, feature flags) which could aid an attacker") | [Dns Resolver](FEATURES.md#dns-resolver "Opt-in DNS resolver with answer verification") | [ESP-NOW](FEATURES.md#esp-now "ESP-NOW peer messaging") | [ETag](FEATURES.md#etag "Conditional GET via ETag for served files") | [File Serving](FEATURES.md#file-serving "Static file serving via Arduino FS (LittleFS, SPIFFS, SD)") |
| [GPIO Map](FEATURES.md#gpio-map "Opt-in browser GPIO pin-mapper / diagnostics endpoint") | [GraphQL](FEATURES.md#graphql "GraphQL query subset") | [Guardrails](FEATURES.md#guardrails "Opt-in runtime heap/stack guardrails") | [HTTP Client](FEATURES.md#http-client "Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS). Default off") | [HTTP Client TLS](FEATURES.md#http-client-tls "HTTPS client support inside the HTTP client (needs TLS)") |
| [HTTP/1.1 Parser](FEATURES.md#http11-parser "RFC 7230 request parser - validates method, path, header names and values byte-by-byte before storing anything") | [IP Allowlist](FEATURES.md#ip-allowlist "Opt-in source-IP allowlist (accept-time firewall, keyed by source IPv4). Default off (zero cost / no behavior change). When set, the accept callback drops any connection whose source address does not match a configured CIDR rule (see listener_ip_allow_add()). An empty allowlist allows everything, so enabling the feature before adding rules never locks the device out") | [JSON](FEATURES.md#json "Zero-heap JSON writer/reader (json.h) for request bodies and responses") | [JWT](FEATURES.md#jwt "JWT bearer-token authentication (HS256). Default off") | [Keep-Alive](FEATURES.md#keep-alive "HTTP/1.1 persistent connections (keep-alive). Default off (every response carries `Connection: close` and the connection is closed after one request - the long-standing behavior). When set to 1, a cleanly-parsed request is answered with `Connection: keep-alive` and the slot is recycled for the next request on the same socket: HTTP/1.1 keeps the connection open unless the client sends `Connection: close`; HTTP/1.0 closes unless the client sends `Connection: keep-alive`. Error responses (400/413/414 and any non-PARSE_COMPLETE path) always close, since the next request boundary is unknown") |
| [Log-Buffer](FEATURES.md#log-buffer "Opt-in fixed-RAM rotating log buffer with severity traps") | [MDNS](FEATURES.md#mdns "mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS") | [MessagePack](FEATURES.md#messagepack "Zero-heap MessagePack encoder for compact binary payloads") | [Metrics](FEATURES.md#metrics "Prometheus `/metrics` endpoint (text exposition format 0.0.4). Default off (requires STATS for the underlying counters). When set, DetWebServer::metrics() emits the runtime stats as Prometheus metrics (`detws_uptime_seconds`, `detws_http_requests_total`, `detws_http_responses_total{class=...}`, `detws_active_connections`, `detws_free_heap_bytes`,...) so a Prometheus server can scrape the device") | [Middleware](FEATURES.md#middleware "Composable use() pipeline with a fixed-window rate limiter") |
| [Modbus](FEATURES.md#modbus "Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502") | [Modbus Master](FEATURES.md#modbus-master "Opt-in Modbus master codec + register scanner") | [MQTT](FEATURES.md#mqtt "MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS). Default off") | [MQTT TLS](FEATURES.md#mqtt-tls "MQTTS: run the MQTT client over client-side TLS (needs TLS)") | [MTLS](FEATURES.md#mtls "Mutual TLS - require and verify a client certificate (mTLS). Default off") |
| [Multipart](FEATURES.md#multipart "multipart/form-data body parser") | [NTP](FEATURES.md#ntp "SNTP wall-clock time sync via the ESP-IDF SNTP client") | [OAuth2](FEATURES.md#oauth2 "OAuth2 token-endpoint client") | [OIDC](FEATURES.md#oidc "OpenID Connect ID-token verification, RS256") | [OPC-UA](FEATURES.md#opc-ua "OPC UA Binary server: SecureChannel + Session + Read/Write + Browse") |
| [OTA](FEATURES.md#ota "Authenticated OTA firmware update (streaming POST to the ESP32 Update API)") | [OTA Rollback](FEATURES.md#ota-rollback "Opt-in OTA rollback protection / soft-brick safeguard") | [Partition Monitor](FEATURES.md#partition-monitor "Opt-in flash partition-map monitor endpoint") | [Per IP Throttle](FEATURES.md#per-ip-throttle "Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4). Default off (zero cost / no behavior change). Complements the global accept throttle: the accept callback rejects a new connection once one source IPv4 address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within a DETWS_PER_IP_THROTTLE_WINDOW_MS fixed window") | [Provisioning](FEATURES.md#provisioning "First-boot WiFi provisioning: softAP + captive-portal credentials form") |
| [Radio Power](FEATURES.md#radio-power "Opt-in radio power controls") | [Range](FEATURES.md#range "HTTP Range requests / 206 Partial Content for served files") | [Routing](FEATURES.md#routing "Exact, wildcard (/*), :param path parameters, bounded allocation-free regex routes, and per-interface STA/softAP route filters") | [SNMP](FEATURES.md#snmp "SNMP agent (v1/v2c, + v3 USM when SNMP_V3) over lwIP UDP") | [SNMP Trap](FEATURES.md#snmp-trap "Outbound SNMP notifications - traps and informs (requires SNMP). Default off") |
| [SNMP V3](FEATURES.md#snmp-v3 "Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off") | [SSE](FEATURES.md#sse "Server-Sent Events push support") | [SSH](FEATURES.md#ssh "SSH server support (RFC 4253/4252/4254)") | [Stats](FEATURES.md#stats "Runtime stats endpoint (uptime, request/error counts, pool usage, heap)") | [Syslog](FEATURES.md#syslog "Syslog client (RFC 5424 over UDP). Default off") |
| [Telemetry](FEATURES.md#telemetry "Telemetry math helpers (moving-window stats, rate-of-change, totalizer). Default off") | [Telnet](FEATURES.md#telnet "Telnet server support (RFC 854 / IAC option negotiation)") | [Templating](FEATURES.md#templating "{{var}} response templating via send_template(). Always on") | [Time Source](FEATURES.md#time-source "Multi-source time fallback (NTP / RTC / GPS /... by priority). When set, src/services/time_source/time_source.h provides a small registry of user-defined time sources, each a callback returning Unix epoch seconds (0 when that source has no valid time). detws_time_now() queries them in priority order (lowest value first) and returns the first valid result, so the device falls back automatically when its preferred clock is unavailable") | [TLS](FEATURES.md#tls "TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only). When set, the server can accept TLS connections using mbedTLS configured with MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DETWS_TLS_ARENA_SIZE) - no system heap, so the determinism guarantee is preserved") |
| [TLS Resumption](FEATURES.md#tls-resumption "TLS session resumption via RFC 5077 session tickets (requires TLS). Default off") | [TOTP](FEATURES.md#totp "Opt-in TOTP two-factor auth (RFC 6238). Default off") | [UDP Telemetry](FEATURES.md#udp-telemetry "Opt-in fire-and-forget UDP telemetry cast") | [Upload](FEATURES.md#upload "Streaming file upload: POST a body straight to a file on the filesystem") | [VFS](FEATURES.md#vfs "Unified virtual filesystem wrapper") |
| [Web Terminal](FEATURES.md#web-terminal "Browser 'web serial' terminal over WebSocket (src/services/web_terminal). Serves a self-contained terminal page and a WebSocket endpoint: device output is broadcast to all connected browsers, browser input is delivered to a command callback") | [WebDAV](FEATURES.md#webdav "WebDAV server (RFC 4918, class 1 + advisory locks) over the file system") | [Webhook](FEATURES.md#webhook "Opt-in outbound webhooks / IFTTT") | [WebSocket](FEATURES.md#websocket "WebSocket support (RFC 6455 framing + SHA-1/base64 handshake)") | [WS Client](FEATURES.md#ws-client "Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS). Default off") |
| [WS Client TLS](FEATURES.md#ws-client-tls "wss://: run the WebSocket client over client-side TLS (needs TLS)") | [WS Deflate](FEATURES.md#ws-deflate "WebSocket permessage-deflate (RFC 7692) - bidirectional compression") | [OPC-UA Client](FEATURES.md#opc-ua-client "OPC UA Binary client: request builders + response parsers (transport-agnostic)") |  |  |


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

See `examples/Foundation/05.Configuration/05.Configuration.ino` for a full reference of every configurable flag and constant.

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

    src/network_drivers/tls/                      mbedTLS over a fixed static pool (HTTPS / wss)
    src/network_drivers/application/              Generated web assets (dashboard, terminal)
    src/network_drivers/presentation/ssh/         Zero-heap SSH-2.0 server
    src/services/                                 Optional L7 subsystems, one folder each:
        opcua/ + opcua_client/, modbus/, mqtt/, coap/, snmp/, dns_resolver/, oidc/,
        oauth2/, totp/, audit_log/, vfs/, graphql/, espnow/, ...  (see FEATURES.md)
```

(Representative, not exhaustive - the full file set is under `src/`; each optional
service is gated by a `DETWS_ENABLE_*` flag.)

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

[`begin()`](@ref DetWebServer::begin) calls `xQueueCreateStatic()` - no `pvPortMalloc`, no fragmentation risk. The library makes no heap allocations.

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
| [`MAX_KEY_LEN`](@ref MAX_KEY_LEN)           | 32      | Header field-name bytes                               |
| [`MAX_VAL_LEN`](@ref MAX_VAL_LEN)           | 48      | Header field-value bytes                              |
| [`MAX_QUERY_LEN`](@ref MAX_QUERY_LEN)       | 128     | Raw query string bytes (after `?`)                    |
| [`MAX_QUERY_PARAMS`](@ref MAX_QUERY_PARAMS) | 8       | Parsed query key=value pairs                          |
| [`QUERY_KEY_LEN`](@ref QUERY_KEY_LEN)       | 24      | Query parameter key bytes                             |
| [`QUERY_VAL_LEN`](@ref QUERY_VAL_LEN)       | 48      | Query parameter value bytes                           |

**Response Buffers**

| Constant                                      | Default | Minimum | Description                                                           |
| --------------------------------------------- | ------- | ------- | --------------------------------------------------------------------- |
| [`RESP_HDR_BUF_SIZE`](@ref RESP_HDR_BUF_SIZE) | 768     | 128     | Stack buffer for HTTP response headers                                |
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

SSH                ssh_pkt[1 × ~1,034]                    1,034     Pkt state + RX buf
                   ssh_keys[1 × ~240]                       240     AES CTR ctx + MAC keys
                   ssh_dh[1 × ~800]                          800     DH scalars + H
                   crypto_work[1536]                        1,536     Bignum scratch (zeroed)
                   group14_p + group14_g                     512     RFC 3526 constants
                   ssh_host_pubkey                            260     RSA-2048 public key

Application        _routes[16 × ~32]                        512     Route table

────────────────── ──────────────────────────────── ─────────────── ──────────────────────────
GRAND TOTAL (all features, default config)               ~18,088 B  ≈ 18 KB
```

</details>

### Default Build Footprint

The **Default server** in the Build Footprint table above - HTTP + WebSocket + SSE + multipart + file serving + Basic auth, including the Arduino/ESP-IDF framework core, ESP32 WiFi drivers, and the lwIP TCP/IP stack - measures:

- **Flash**: **745,133 bytes** (~56.8% of a 1.31 MB application partition); the WiFi/lwIP stack dominates (an empty no-WiFi sketch is already ~233 KB).
- **Static RAM**: **64,264 bytes** (~19.6% of the 320 KB internal SRAM), all statically placed.
- **Zero dynamic allocations**: the library requests exactly **0 bytes** of heap after `begin()`, managing every session and packet buffer statically (event queues via `xQueueCreateStatic()`).

Optional subsystems (HTTPS/TLS, SSH, SNMP, ...) add to this - see the per-row deltas in the Build Footprint table above.

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

**600+ Unity tests** across the native suites, all runnable on a native x86/x64 host
(no hardware required). See **[TEST_REPORT.md](TEST_REPORT.md)** for the current
per-suite breakdown and totals. Run a representative subset with:

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
| [TUNING.md](TUNING.md)                         | Performance tuning: worker count, core/affinity, poll knobs       |
| [MIGRATION.md](MIGRATION.md)                   | Breaking-change migration guide (3.x to 4.0.0)                    |
| [CHANGELOG.md](CHANGELOG.md)                   | Release history                                                   |

</details>

### Generating Docs Locally

To generate the HTML API documentation locally, run the following command from the repository root:

```bash
doxygen docs/Doxyfile
```

The output will be generated in `docs/html/index.html`.

If you are viewing the offline version of this documentation, you can access the latest online version at the [GitHub Pages documentation site](https://dstroy0.github.io/DeterministicESPAsyncWebServer/).

## Licensing & Commercial Use

This library is dual-licensed.

**Open Source.** This library is, and will **ALWAYS REMAIN, FULLY OPEN-SOURCE** under the AGPLv3 (or later). We commit to maintaining a fully featured, parity-matched open-source version available to everyone - from hobbyists and educators to professionals - without hiding any non-proprietary (e.g. custom protocols, intellectual property, confidential telemetry configurations, etc.) feature behind a commercial paywall. It will always be free to use under the AGPLv3 (or later) in any environment that complies with the AGPLv3 (or later) terms. See the `LICENSE` file.

**Commercial.** For teams and applications that cannot meet the AGPLv3 copyleft requirements, a commercial license is available. Contact: Douglas Quigg (dstroy0), dquigg123@gmail.com

**Educators.** Teaching with this? We'd love that. **SERIOUSLY**. Squirty is meant to keep children engaged on the docs page. The docs and styling are set up to appeal to them, hobbyists, and anyone who wants to learn but doesn't know how to style things or glue services together. The library documentation is extensive, extremely thorough, and useful to professionals as well as educators as a teaching tool/classroom prop. If sharing your source under the AGPLv3 isn't practical for a classroom or lab, or you have concerns that have stopped you from using copyleft licensed software before, email Douglas Quigg (dstroy0) at dquigg123@gmail.com from your school address and we'll see what we can do. ESP32 boards are cheap and a hands-on HTTP / IoT-edge stack is a great way into embedded networking, so we're glad to look at education-focused requests one by one. We can't promise an exception for every situation, but please ask. (This is just for genuine educational use; for products, see the commercial option above.) I can help you set up a github repo your students can push to that will help you review their submissions, and walk you through setting up flags for your rubric items. We really need to make an effort to get as many people as possible into the profession, looking at how things work, figuring out how they work on a deeper level, and entering the profession, we need their ideas, we need them now. All great discoveries have come from fresh perspective.

---

<p align="center">
  <img src="squirty.svg" alt="Squirty the Injection Squid" width="64" height="64"><br>
  <b>Squirty the Injection Squid</b>: the official library mascot.<br>
  <sub>Copyright &copy; Douglas Quigg (dstroy0). All rights reserved.</sub>
</p>
