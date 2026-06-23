# DeterministicESPAsyncWebServer (@dstroy0)

An HTTP/1.1 web server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture.

![Version](https://img.shields.io/badge/version-v1.2.4-blue)
[![Test Build Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/test-report.yml)
[![Docs Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/docs.yml)
[![Changelog Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/changelog.yml)
[![C++ Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/clang-format.yml)
[![Markdown Formatting Status](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml/badge.svg)](https://github.com/dstroy0/DeterministicESPAsyncWebServer/actions/workflows/markdown-format.yml)

**[API Documentation](https://dstroy0.github.io/DeterministicESPAsyncWebServer/)**

## 📚 Documentation

**[Read the Full Documentation Here 📖](https://dstroy0.github.io/DeterministicESPAsyncWebServer/)**

The technical reference documentation has been moved to a dedicated landing page to provide a better reading experience. You can also view the local markdown copy at [docs/README.md](docs/README.md).

## Overview

Asynchronous HTTP, WebSocket, and SSH Server Library for ESP32 with zero heap allocations.

**Key Features:**

- **Zero Heap Allocation**: All request, connection, WebSocket, and events storage pools are statically sized in BSS. Zero heap memory is requested after `begin()`.
- **RFC 7230 Compliant Request Parser**: Validates requests byte-by-byte and auto-sends correct status codes (e.g., 400, 404, 405, 413, 414, 501) with required headers.
- **WebSocket (RFC 6455)**: Built-in frame parser with SHA-1 handshake and ping/pong management.
- **SSE (Server-Sent Events)**: Persistent client connections and push broadcasting.
- **Basic Authentication**: Built-in per-route HTTP Basic Auth support.
- **File Serving**: Stream static assets with chunked reads from LittleFS, SPIFFS, and SD. One-call `serve_static()` subtree mount with `index.html` fallback, MIME auto-detection, pre-compressed `.gz` serving, and `ETag`/`304` conditional GET.
- **Multipart Parser**: In-place form-data parser supporting file uploads.
- **SSH 2.0 and Telnet Server**: Integrated zero-heap SSH/Telnet stacks with host-key verification and password/publickey authentication.
- **mDNS & NTP Services**: Hostname advertisement via the ESP-IDF mDNS component and SNTP wall-clock time synchronization for request logging.
- **OTA Updates**: Secure, authenticated over-the-air firmware updates that stream the POST body straight into flash (no full-image RAM buffer).
- **Captive Portal Provisioning**: Setup wizard (SoftAP + catch-all DNS portal) for first-boot WiFi credential configuration.
- **Observability**: Optional runtime stats endpoint (uptime, request/error counts, pool usage, heap) and a per-request access-log callback.
- **Minimal Dependencies**: Beyond the Arduino/ESP-IDF SDK, the only external dependency is mbedTLS (crypto); optional services use the ESP-IDF mDNS component and raw lwIP UDP rather than add-on libraries. Every optional feature is gated by a `DETWS_ENABLE_*` build flag (default off).

## Compatibility

- **Chips**: ESP32 (all variants)
- **Frameworks**: Arduino Core (2.x and 3.x), PlatformIO

## License

This project is licensed under the AGPL-3.0-or-later License - see the `LICENSE` file for details.
