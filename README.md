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

## Quick Start

```cpp
#include <WiFi.h>
#include "DeterministicESPAsyncWebServer_Layer7.h"

DeterministicESPAsyncWebServer server;

void handleAPI(uint8_t slot_id, HttpPresentationBlock* request) {
    server.send(slot_id, 200, "application/json", "{\"status\":\"ok\"}");
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID", "PASSWORD");

    server.on("/api/status", HTTP_GET, handleAPI);
    server.begin(80);
}

void loop() {
    server.handle();  // Must be called regularly
}
```

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPL-3.0).
See LICENSE file for details.
