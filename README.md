<p align="center">
  <img src="docs/banner.svg" alt="DeterministicESPAsyncWebServer" width="100%">
</p>

# DeterministicESPAsyncWebServer (@dstroy0)

A multi-protocol network server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture. It serves HTTP/1.1, WebSocket, and Server-Sent Events, with optional HTTPS/TLS, SSH, Telnet, SNMP, and CoAP.

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

A zero-heap, asynchronous multi-protocol server library for ESP32. Network events fire asynchronously from the lwIP stack (driven by the WiFi ISRs) into a fixed event queue that the main loop drains, and every connection, request, and protocol buffer is statically allocated in BSS, so the memory footprint is fixed at link time and no heap is touched after `begin()`. It serves HTTP/1.1 (with WebSocket and Server-Sent Events) and, optionally, HTTPS/TLS, SSH, Telnet, SNMP, and CoAP.

**Key Features:**

- **Zero Heap Allocation**: All request, connection, WebSocket, SSE, TLS, and SNMP storage pools are statically sized in BSS. Zero heap memory is requested after `begin()`.
- **RFC 7230 Compliant Request Parser**: Validates requests byte-by-byte and auto-sends correct status codes (e.g., 400, 404, 405, 413, 414, 501) with required headers.
- **Flexible Routing**: Exact, wildcard (`/*`), and `:param` path-parameter routes, bounded allocation-free regex routes, and per-interface (station / softAP) route filters.
- **Request Data**: Query-string, `x-www-form-urlencoded` form fields, in-place `multipart/form-data` file uploads, and a zero-heap JSON writer/reader.
- **Response Tools**: Custom headers + cookies, CORS (with preflight), `{{var}}` templating, and chunked/streaming responses of unbounded length in constant memory.
- **WebSocket (RFC 6455)**: Built-in frame parser with SHA-1 handshake and ping/pong management, plus a browser "web serial" terminal over WebSocket. Runs encrypted over TLS (`wss://`) when TLS is enabled.
- **SSE (Server-Sent Events)**: Persistent client connections and push broadcasting, including over TLS (encrypted `text/event-stream`).
- **Authentication**: Per-route HTTP Basic (RFC 7617) and Digest (RFC 7616, SHA-256, `qop=auth`), plus optional stateless JWT bearer-token verification (HS256, constant-time).
- **File Serving**: Stream static assets with chunked reads from LittleFS, SPIFFS, and SD. One-call `serve_static()` subtree mount with `index.html` fallback, MIME auto-detection, pre-compressed `.gz` serving, and `ETag`/`304` conditional GET.
- **WebDAV (RFC 4918)**: Optional (`DETWS_ENABLE_WEBDAV`) read/write file share over the filesystem - one-call `dav()` subtree mount answering OPTIONS, PROPFIND (Depth 0/1, 207 Multi-Status), GET, HEAD, PUT, DELETE (recursive), MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK, so rclone, cadaver, curl, or a mounted network drive can browse and edit files. The 207 builder and header parsing are host-tested.
- **HTTPS / TLS**: Optional deterministic TLS via mbedTLS over a fixed static memory pool - encrypted transport with no heap (`ECDHE-ECDSA-AES256-GCM-SHA384`, TLS 1.2+).
- **Mutual TLS (mTLS)**: Optional client-certificate authentication - the handshake requires and verifies a client cert against a configured CA, and the verified peer's subject DN is exposed to handlers. Strong transport-level client auth with no passwords.
- **SSH 2.0 Server**: Zero-heap SSH stack with host-key verification and password/publickey authentication (hardware-accelerated crypto).
- **Modbus TCP Slave**: Optional (`DETWS_ENABLE_MODBUS`) Modbus Application Protocol server on TCP/502 - a fixed BSS data model (coils, discrete inputs, holding + input registers) served via Read/Write Coils (FC 1/5/15), Read Discrete Inputs (FC 2), Read/Write Holding Registers (FC 3/6/16), and Read Input Registers (FC 4), with proper exception responses. The MBAP/PDU codec is host-tested; the application owns the data model and is notified of client writes.
- **Telnet Console**: Plaintext line-oriented console (RFC 854, IAC negotiation + server echo) for trusted networks.
- **SNMP Agent**: v1/v2c plus optional v3 USM (HMAC-SHA-256 auth + AES-128 privacy) over UDP, with a zero-heap ASN.1 BER codec and a fixed MIB.
- **SNMP Notifications**: Optional outbound Traps and InformRequests (`DETWS_ENABLE_SNMP_TRAP`) so the agent pushes alerts to a manager - SNMPv2c, and SNMPv3 USM (authPriv) traps reusing the agent's engine ID and localized keys. Each notification carries `sysUpTime.0` + `snmpTrapOID.0` plus caller varbinds; the PDU builder is host-tested.
- **CoAP Server (RFC 7252)**: Zero-heap Constrained Application Protocol endpoint over UDP - a fixed resource table dispatched on Uri-Path, GET/POST/PUT/DELETE with piggybacked responses, Uri-Query and Content-Format options. Optional resource **Observe** (RFC 7641, `DETWS_ENABLE_COAP_OBSERVE`): clients subscribe with a GET + Observe option and `coap_notify()` pushes the resource's current value to all observers from the server port with an increasing sequence. Optional **block-wise transfer** (RFC 7959, `DETWS_ENABLE_COAP_BLOCK`): the Block2 option pages a large representation one block at a time, and the Block1 option reassembles a chunked POST/PUT upload (`2.31 Continue` per block) before dispatching the handler with the whole payload.
- **HTTP Keep-Alive**: Optional HTTP/1.1 persistent connections - one TCP connection serves many requests (with a per-connection request cap and the existing idle timeout), transparent to handler code.
- **HTTP Range Requests**: Optional `206 Partial Content` (RFC 7233) for served files - single-range `Range: bytes=...` requests stream just the requested bytes (resumable downloads, media seeking), with `Accept-Ranges` advertisement and `416` for unsatisfiable ranges.
- **Middleware & Rate Limiting**: Composable middleware pipeline, a built-in fixed-window rate limiter, an opt-in global connection accept-throttle, and an opt-in per-source-IP accept-throttle (`DETWS_ENABLE_PER_IP_THROTTLE`) that bounds reconnect/brute-force churn from a single host.
- **mDNS & NTP Services**: Hostname advertisement via the ESP-IDF mDNS component and SNTP wall-clock time synchronization for request logging.
- **OTA Updates**: Secure, authenticated over-the-air firmware updates that stream the POST body straight into flash (no full-image RAM buffer).
- **Streaming Uploads**: Optional POST-body streaming straight into a filesystem file (LittleFS / SPIFFS / SD), so an upload never has to fit in RAM.
- **Captive Portal Provisioning**: Setup wizard (SoftAP + catch-all DNS portal) for first-boot WiFi credential configuration.
- **Observability**: Optional runtime stats endpoint (uptime, request/error counts, pool usage, heap), a Prometheus `/metrics` endpoint (text exposition format 0.0.4) for scraping, a per-request access-log callback, and a diagnostics endpoint.
- **Remote Logging**: Optional RFC 5424 syslog client - ship structured log lines to a central syslog server over UDP (zero-heap, fire-and-forget).
- **Outbound HTTP(S) Client**: Optional zero-heap client for requests _from_ the device (webhooks, telemetry push, REST calls): blocking `http_get()` / `http_post()` over raw lwIP with DNS resolution, Content-Length / chunked response decoding, and `https://` via client-side mbedTLS over the same static arena - encrypt-only by default with optional server authentication (CA trust anchor or SHA-256 certificate pin). It links no code unless a sketch actually calls it.
- **MQTT Client (3.1.1)**: Optional persistent publish/subscribe client for IoT messaging - connect to a broker with Last-Will and credentials, `PUBLISH` / `SUBSCRIBE` / `UNSUBSCRIBE` at QoS 0, 1, or 2 (full acknowledgement flows with bounded in-flight DUP retransmit and inbound QoS-2 de-duplication), receive messages via a callback, and keep-alive pings; `mqtts://` runs over the same client-side mbedTLS (encrypt-only or CA/pin-verified). The packet codec is host-tested.
- **WebSocket Client (RFC 6455)**: Optional outbound WebSocket client - the device connects to a remote endpoint (`ws://`, or `wss://` over the same client-side mbedTLS), performs the `Sec-WebSocket-Key`/`Accept` handshake, sends masked text/binary frames, receives server frames via a callback, and answers ping/pong - for streaming to cloud dashboards or bidirectional control. The frame/handshake codec is host-tested.
- **Minimal Dependencies**: Beyond the Arduino/ESP-IDF SDK, the only external dependency is mbedTLS (crypto); optional services use the ESP-IDF mDNS component and raw lwIP UDP rather than add-on libraries. Every optional feature is gated by a `DETWS_ENABLE_*` build flag (default off).

## Build Footprint

Measured on `esp32dev` (Arduino core, `pio ci`). The jump from the bare baseline to a running server is almost entirely the WiFi/lwIP stack; the library and most HTTP features add little on top. Each row enables one optional subsystem over the default server.

| Build                                                                               | Flash (bytes) | RAM (bytes) |
| ----------------------------------------------------------------------------------- | ------------: | ----------: |
| Empty sketch (no WiFi, no library) - _RTOS/Arduino baseline_                        |       233,257 |      21,032 |
| Minimal REST server (WS/SSE/multipart/file/auth stripped)                           |       734,745 |      57,936 |
| **Default server** (HTTP + WebSocket + SSE + multipart + file serving + Basic auth) |       745,133 |      64,264 |
| &nbsp;&nbsp;+ HTTPS / TLS (static-pool mbedTLS)                                     |       847,185 |     115,164 |
| &nbsp;&nbsp;&nbsp;&nbsp;+ Mutual TLS (client-cert auth)                             |       848,241 |     115,500 |
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

TLS's larger RAM is the fixed mbedTLS arena (`DETWS_TLS_ARENA_SIZE`, 48 KB default). Small HTTP features (CORS, JSON, middleware, regex / path / form params, templating, chunked, response headers, Digest auth, stats, diagnostics, accept-throttle) stay within a few KB of the default server. HTTP keep-alive (`DETWS_ENABLE_KEEPALIVE`) is essentially free - on a fixed build it adds ~556 B flash and 8 B RAM (one `uint16` per connection slot). HTTP Range requests (`DETWS_ENABLE_RANGE`) add ~760 B flash and no RAM over file serving. The outbound HTTP client (`DETWS_ENABLE_HTTP_CLIENT`) links no code unless a sketch actually calls `http_get()` / `http_post()` (unused, the linker strips it - enabling the flag on a server that never calls it costs nothing); the standalone client example builds to 732,961 B flash / 46,752 B RAM, and adding `https://` (`DETWS_ENABLE_TLS` + `DETWS_ENABLE_HTTP_CLIENT_TLS`, which pulls in mbedTLS) makes it 827,853 B / 100,620 B. The MQTT client (`DETWS_ENABLE_MQTT`) example builds to 734,293 B flash / 48,896 B RAM; adding `mqtts://` (`DETWS_ENABLE_TLS` + `DETWS_ENABLE_MQTT_TLS`) makes it 830,285 B / 108,732 B. The WebSocket client (`DETWS_ENABLE_WS_CLIENT`) example builds to 734,329 B / 48,824 B; adding `wss://` makes it 830,165 B / 108,660 B. CoAP resource Observe (`DETWS_ENABLE_COAP_OBSERVE`) adds ~2.7 KB flash over the CoAP server. CoAP block-wise transfer (`DETWS_ENABLE_COAP_BLOCK`) adds ~1.6 KB flash plus the Block1 reassembly buffer (`DETWS_COAP_BLOCK1_MAX`, 1 KB default) and a larger message buffer in RAM. The per-IP accept-throttle (`DETWS_ENABLE_PER_IP_THROTTLE`) is negligible: a small accept-path check plus the bucket table (`DETWS_PER_IP_THROTTLE_SLOTS` × 12 bytes, 192 B default). WebDAV (`DETWS_ENABLE_WEBDAV`) adds ~24 KB flash and ~3 KB RAM over a file-serving server (the handler, the 207 builder, RFC 1123 date formatting, and the `DETWS_WEBDAV_BUF_SIZE` Multi-Status buffer, 2 KB default). The Modbus TCP slave (`DETWS_ENABLE_MODBUS`) adds ~2.1 KB flash and ~0.25 KB RAM (the codec plus the four data-model tables, sized by `DETWS_MODBUS_*`). ESP32 capacity: 1,310,720 B flash / 327,680 B RAM. Per-feature examples are under [`examples/`](examples/).

## Compatibility

- **Chips**: ESP32 (all variants)
- **Frameworks**: Arduino Core (2.x and 3.x), PlatformIO

## License

This project is licensed under the AGPL-3.0-or-later License - see the `LICENSE` file for details.

---

<p align="center">
  <img src="docs/squirty.svg" alt="Squirty the Injection Squid" width="64" height="64"><br>
  <b>Squirty the Injection Squid</b> official library mascot.<br>
  <sub>Copyright &copy; Douglas Quigg (dstroy0). All rights reserved.</sub>
</p>
