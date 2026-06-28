# 13.CoAP - a zero-heap CoAP server

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_COAP`

## What this example teaches

CoAP (RFC 7252) is a compact request/response protocol for constrained devices,
running over UDP. This stands up a CoAP server on UDP/5683 alongside the HTTP
server, exposing a few resources dispatched by Uri-Path. It is callback-driven
over the transport-layer UDP service (no per-loop servicing) and every buffer is
static.

**Build a resource table, then bind.** Each resource has a path, an allowed-method
mask, and a handler:

```cpp
coap_server_init();
coap_server_add_resource("/info", COAP_ALLOW_GET, coap_info);
coap_server_add_resource("/led", COAP_ALLOW_GET | COAP_ALLOW_PUT, coap_led);
coap_server_add_resource("/hello", COAP_ALLOW_GET, coap_hello);
coap_server_begin_udp(5683);
```

**The handler shape.** A handler fills a `CoapResponse` from the `CoapRequest`:
write into `resp->payload` (bounded by `resp->payload_cap`), set `payload_len`,
the `content_format`, and the response `code`. The LED handler shows reading the
method and the request payload:

```cpp
static void coap_led(const CoapRequest *req, CoapResponse *resp) {
    if (req->method == COAP_PUT) {
        g_led_state = (req->payload_len && req->payload[0] != '0') ? 1 : 0;
        digitalWrite(LED_BUILTIN, g_led_state ? HIGH : LOW);
        resp->code = COAP_RSP_CHANGED; resp->payload_len = 0; return;
    }
    resp->payload[0] = g_led_state ? '1' : '0';
    resp->payload_len = 1; resp->content_format = COAP_CF_TEXT; resp->code = COAP_RSP_CONTENT;
}
```

CON requests get a piggybacked ACK; NON requests get a NON response - handled by
the library. For server-push and large transfers, see
[27.CoapObserve](../27.CoapObserve) and [28.CoapBlock](../28.CoapBlock).

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_COAP=1" \
  --lib="." examples/L7-Application/13.CoAP/13.CoAP.ino
```

```sh
coap-client -m get coap://<ip>/info
coap-client -m put -e 1 coap://<ip>/led
coap-client -m get coap://<ip>/hello
```

## Annotated source

The complete sketch ([13.CoAP.ino](13.CoAP.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_COAP 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/coap/coap.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

DetWebServer server;
static int g_led_state = 0;

// GET /info -> a small JSON document with uptime and free heap.
static void coap_info(const CoapRequest *req, CoapResponse *resp)
{
    (void)req;
    int n = snprintf((char *)resp->payload, resp->payload_cap, "{\"uptime_ms\":%lu,\"free_heap\":%u}",
                     (unsigned long)millis(), (unsigned)ESP.getFreeHeap());
    if (n < 0)
        n = 0;
    resp->payload_len = (size_t)n;
    resp->content_format = COAP_CF_JSON;
    resp->code = COAP_RSP_CONTENT;
}

// GET/PUT /led -> read or drive the on-board LED.
static void coap_led(const CoapRequest *req, CoapResponse *resp)
{
    if (req->method == COAP_PUT)
    {
        // Treat any payload beginning with a non-'0' character as "on".
        g_led_state = (req->payload_len && req->payload[0] != '0') ? 1 : 0;
        digitalWrite(LED_BUILTIN, g_led_state ? HIGH : LOW);
        resp->code = COAP_RSP_CHANGED;
        resp->payload_len = 0;
        return;
    }
    resp->payload[0] = g_led_state ? '1' : '0';
    resp->payload_len = 1;
    resp->content_format = COAP_CF_TEXT;
    resp->code = COAP_RSP_CONTENT;
}

// GET /hello -> a constant greeting.
static void coap_hello(const CoapRequest *req, CoapResponse *resp)
{
    (void)req;
    static const char msg[] = "hello from a deterministic CoAP server";
    size_t n = sizeof(msg) - 1;
    if (n > resp->payload_cap)
        n = resp->payload_cap;
    memcpy(resp->payload, msg, n);
    resp->payload_len = n;
    resp->content_format = COAP_CF_TEXT;
    resp->code = COAP_RSP_CONTENT;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    // Build the resource table, then bind the server to UDP/5683.
    coap_server_init();
    coap_server_add_resource("/info", COAP_ALLOW_GET, coap_info);
    coap_server_add_resource("/led", COAP_ALLOW_GET | COAP_ALLOW_PUT, coap_led);
    coap_server_add_resource("/hello", COAP_ALLOW_GET, coap_hello);
    coap_server_begin_udp(5683);
    Serial.println("CoAP server listening on UDP/5683 (try: coap-client -m get coap://<ip>/info)");

    int32_t result = server.begin(80);
    if (result < 0)
        Serial.printf("begin() failed (error %d)\n", result);
}

void loop()
{
    server.handle(); // CoAP is serviced by lwIP callbacks; this drives the TCP server.
}
```
