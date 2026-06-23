# DeterministicESPAsyncWebServer

An HTTP/1.1 web server for ESP32 with a fully deterministic memory footprint, RFC 7230 compliant request parsing, and an OSI-layered architecture.

![Version](https://img.shields.io/badge/version-v1.2.1-blue)
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
- **File Serving**: Stream static assets with chunked reads from LittleFS, SPIFFS, and SD.
- **Multipart Parser**: In-place form-data parser supporting file uploads.
- **SSH 2.0 and Telnet Server**: Integrated zero-heap SSH/Telnet stacks with host-key verification and password/publickey authentication.
- **mDNS & NTP Services**: Embedded hostname advertisement via ESPmDNS and SNTP wall-clock time synchronization for request logging.
- **OTA Updates**: Secure, authenticated over-the-air firmware updates via streaming POST request body.
- **Captive Portal Provisioning**: Setup wizard (SoftAP + DNS portal) for first-boot WiFi credential configuration.

## Compatibility

- **Chips**: ESP32 (all variants)
- **Frameworks**: Arduino Core (2.x and 3.x), PlatformIO

## License

This project is licensed under the AGPL-3.0-or-later License - see the `LICENSE` file for details.
