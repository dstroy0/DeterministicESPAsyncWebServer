# DeterministicESPAsyncWebServer v0.1.0

A high-performance, deterministic async web server library for ESP32 microcontrollers with AGPL-3.0 licensing.

## Features

- **Deterministic Performance**: No dynamic memory allocation, static pre-allocated connection pools
- **Lock-Free Architecture**: FreeRTOS queue-based event processing with zero mutex contention
- **Interrupt-Safe**: Designed to work seamlessly with ESP32's interrupt-driven model
- **Layer-Based Design**:
  - Layer 4: TCP connection management with ring buffers
  - Layer 5: Event queue processing
  - Layer 6: HTTP stream parsing
  - Layer 7: Route matching and user callbacks
- **Fixed Resource Limits**: 4 concurrent connections, 1KB RX buffer per connection
- **AGPL-3.0 License**: Free and open-source with community contribution requirements

## Installation

This is an Arduino library. Copy this folder to your `Arduino/libraries/` directory or install via the Arduino IDE Library Manager.

### Summary of Examples

1. basic.ino
   • Target Audience: Beginners getting started with deterministic web applications.
   • Features: Connecting to WiFi via init_wifi_physical() and wifi_ready() polling, enabling CORS, setting up simple GET/POST routes, parameter scanning with http_get_query() , wildcard path prefix matching, and registering a custom 404 fallback handler.
2. advanced.ino
   • Target Audience: Developers building standard RESTful APIs.
   • Features: Full mock-database CRUD implementation (GET, POST, PATCH, DELETE) matching a static struct array. Demonstrates bearer token validation ( Authorization header checks), strict request body header verification ( Content-Type: application/json ), list filtering via query strings, and custom zero-allocation JSON
   extraction helpers for floats, strings, and booleans.
3. expert.ino
   • Target Audience: Performance-oriented firmware engineers.
   • Features: Introspection of the low-level transport.h pool to report live slot states and ring buffer occupancy metrics. It implements a zero-allocation, time-based token-bucket rate limiter (returning 429 Too Many Requests ), measures route handling durations in microseconds, and profiles stack memory watermark
   margins using uxTaskGetStackHighWaterMark() .
4. sysadmin.ino
   • Target Audience: Administrators managing ESP32 nodes in production.
   • Features: Serves a premium dark-themed HTML/CSS single-page console dashboard complete with responsive layout, neon highlights, live telemetry charts, and micro-animations. Exposes REST endpoints to query hardware stats (free heap memory bounds, WiFi RSSI/BSSID, reset reason) and schedules safe software reboots by
   answering the client first and postponing ESP.restart() to flush lwIP TCP buffers cleanly.

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).
See LICENSE file for details.
