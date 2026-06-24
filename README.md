<p align="center">
  <img src="docs/banner.svg" alt="DeterministicESPAsyncWebServer" width="100%">
</p>

# DeterministicESPAsyncWebServer (@dstroy0)

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1, WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, and SNMP.

![Version](https://img.shields.io/badge/version-v2.0.0-blue)
[![Test Build Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml)
[![Docs Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml)
[![Changelog Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml)
[![C++ Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml)
[![Markdown Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml)

## 📚 Documentation

**[Read the Full Documentation Here 📖](https://dstroy0.github.io/DeterministicESPAsyncWebServer/)**

The technical reference documentation has been moved to a dedicated landing page to provide a better reading experience. You can also view the local markdown copy at [docs/README.md](docs/README.md).

## Overview

A zero-heap, asynchronous multi-protocol server library for ESP32. Network events fire asynchronously from the lwIP stack (driven by the WiFi ISRs) into a fixed event queue that the main loop drains, and every connection, request, and protocol buffer is statically allocated in BSS — so the memory footprint is fixed at link time and no heap is touched after `begin()`. It serves HTTP/1.1 (with WebSocket and Server-Sent Events) and, optionally, HTTPS/TLS, SSH, Telnet, and SNMP.

**Key Features:**

- **Zero Heap Allocation**: All request, connection, WebSocket, SSE, TLS, and SNMP storage pools are statically sized in BSS. Zero heap memory is requested after `begin()`.
- **RFC 7230 Compliant Request Parser**: Validates requests byte-by-byte and auto-sends correct status codes (e.g., 400, 404, 405, 413, 414, 501) with required headers.
- **Flexible Routing**: Exact, wildcard (`/*`), and `:param` path-parameter routes, bounded allocation-free regex routes, and per-interface (station / softAP) route filters.
- **Request Data**: Query-string, `x-www-form-urlencoded` form fields, in-place `multipart/form-data` file uploads, and a zero-heap JSON writer/reader.
- **Response Tools**: Custom headers + cookies, CORS (with preflight), `{{var}}` templating, and chunked/streaming responses of unbounded length in constant memory.
- **WebSocket (RFC 6455)**: Built-in frame parser with SHA-1 handshake and ping/pong management, plus a browser "web serial" terminal over WebSocket.
- **SSE (Server-Sent Events)**: Persistent client connections and push broadcasting.
- **Authentication**: Per-route HTTP Basic (RFC 7617) and Digest (RFC 7616, SHA-256, `qop=auth`).
- **File Serving**: Stream static assets with chunked reads from LittleFS, SPIFFS, and SD. One-call `serve_static()` subtree mount with `index.html` fallback, MIME auto-detection, pre-compressed `.gz` serving, and `ETag`/`304` conditional GET.
- **HTTPS / TLS**: Optional deterministic TLS via mbedTLS over a fixed static memory pool - encrypted transport with no heap (`ECDHE-ECDSA-AES256-GCM-SHA384`, TLS 1.2+).
- **SSH 2.0 Server**: Zero-heap SSH stack with host-key verification and password/publickey authentication (hardware-accelerated crypto).
- **Telnet Console**: Plaintext line-oriented console (RFC 854, IAC negotiation + server echo) for trusted networks.
- **SNMP Agent**: v1/v2c plus optional v3 USM (HMAC-SHA-256 auth + AES-128 privacy) over UDP, with a zero-heap ASN.1 BER codec and a fixed MIB.
- **Middleware & Rate Limiting**: Composable middleware pipeline, a built-in fixed-window rate limiter, and an opt-in connection accept-throttle.
- **mDNS & NTP Services**: Hostname advertisement via the ESP-IDF mDNS component and SNTP wall-clock time synchronization for request logging.
- **OTA Updates**: Secure, authenticated over-the-air firmware updates that stream the POST body straight into flash (no full-image RAM buffer).
- **Captive Portal Provisioning**: Setup wizard (SoftAP + catch-all DNS portal) for first-boot WiFi credential configuration.
- **Observability**: Optional runtime stats endpoint (uptime, request/error counts, pool usage, heap), a per-request access-log callback, and a diagnostics endpoint.
- **Minimal Dependencies**: Beyond the Arduino/ESP-IDF SDK, the only external dependency is mbedTLS (crypto); optional services use the ESP-IDF mDNS component and raw lwIP UDP rather than add-on libraries. Every optional feature is gated by a `DETWS_ENABLE_*` build flag (default off).

## Build Footprint

Measured on `esp32dev` (Arduino core, `pio ci`). The jump from the bare baseline to a running server is almost entirely the WiFi/lwIP stack; the library and most HTTP features add little on top. Each row enables one optional subsystem over the default server.

| Build                                                                               | Flash (bytes) | RAM (bytes) |
| ----------------------------------------------------------------------------------- | ------------: | ----------: |
| Empty sketch (no WiFi, no library) - _RTOS/Arduino baseline_                        |       233,257 |      21,032 |
| Minimal REST server (WS/SSE/multipart/file/auth stripped)                           |       734,745 |      57,936 |
| **Default server** (HTTP + WebSocket + SSE + multipart + file serving + Basic auth) |       745,133 |      64,264 |
| &nbsp;&nbsp;+ HTTPS / TLS (static-pool mbedTLS)                                     |       847,185 |     115,164 |
| &nbsp;&nbsp;+ SSH 2.0 server                                                        |       798,005 |      76,556 |
| &nbsp;&nbsp;+ SNMP agent (v1/v2c)                                                   |       751,277 |      76,648 |
| &nbsp;&nbsp;+ mDNS                                                                  |       768,037 |      66,160 |
| &nbsp;&nbsp;+ SNTP                                                                  |       768,861 |      66,808 |
| &nbsp;&nbsp;+ OTA update                                                            |       748,417 |      64,544 |
| &nbsp;&nbsp;+ Captive-portal provisioning                                           |       750,709 |      65,836 |
| &nbsp;&nbsp;+ Static files via LittleFS (incl. ETag)                                |       784,361 |      64,288 |
| &nbsp;&nbsp;+ Telnet console                                                        |       745,137 |      64,784 |
| &nbsp;&nbsp;+ Web terminal (WebSocket)                                              |       747,613 |      64,336 |
| SSH crypto self-test (Serial only, no WiFi)                                         |       269,585 |      21,476 |

TLS's larger RAM is the fixed mbedTLS arena (`DETWS_TLS_ARENA_SIZE`, 48 KB default). Small HTTP features (CORS, JSON, middleware, regex / path / form params, templating, chunked, response headers, Digest auth, stats, diagnostics, accept-throttle) stay within a few KB of the default server. ESP32 capacity: 1,310,720 B flash / 327,680 B RAM. Per-feature examples are under [`examples/`](examples/).

## Compatibility

- **Chips**: ESP32 (all variants)
- **Frameworks**: Arduino Core (2.x and 3.x), PlatformIO

## License

This project is licensed under the AGPL-3.0-or-later License - see the `LICENSE` file for details.

---

<p align="center">
  <img src="docs/squirty.svg" alt="Squirty the Injection Squid" width="64" height="64"><br>
  <b>Squirty the Injection Squid</b> — the official library mascot.<br>
  <sub>Copyright &copy; Douglas Quigg (dstroy0). All rights reserved.</sub>
</p>
