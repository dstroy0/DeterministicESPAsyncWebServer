# 07.SecureWebSocket - wss:// and SSE over TLS

**Layer:** L6 Presentation · **Build flags:** `DETWS_ENABLE_TLS` (WebSocket + SSE on by default)

## What this example teaches

Both WebSocket and Server-Sent Events run over the deterministic TLS engine: the
`wss://` handshake and every frame are encrypted, and SSE events are pushed over
the same TLS record layer. The key point: **it is transparent to handler code.**
You register the same `on_ws()` / `on_sse()` routes as in the plaintext examples;
serving them on a TLS listener is all that changes.

**Same callbacks, TLS listener.** The WebSocket echo and SSE broadcast are
identical to [09.WebSocket](../09.WebSocket) / [08.ServerSentEvents](../08.ServerSentEvents);
only `begin_tls()` (instead of `begin()`) makes them encrypted on 443:

```cpp
server.on_ws("/ws", ws_connect, ws_message, ws_close);
server.on_sse("/events", sse_connect);
int32_t result = server.begin_tls(443, SERVER_CERT_PEM, sizeof(SERVER_CERT_PEM),
                                       SERVER_KEY_PEM,  sizeof(SERVER_KEY_PEM));
```

Under the hood, `wss` receive decrypts TLS records straight into the frame parser,
and SSE is send-only over the same encrypted stream - so handlers still read
plaintext from `ws_pool[ws_id].buf` and call `ws_send_text` / `sse_broadcast`
normally.

> Demo cert/key as in [03.HTTPS](../03.HTTPS) - public, demo-only; the PEM blocks
> are elided below (see the `.ino`).

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_TLS=1" \
  --lib="." examples/L6-Presentation/07.SecureWebSocket/07.SecureWebSocket.ino
```

```sh
# browser console: new WebSocket("wss://<ip>/ws")
curl -k -N https://<ip>/events     # live SSE stream over TLS
```

## Annotated source

The complete sketch ([07.SecureWebSocket.ino](07.SecureWebSocket.ino)). The demo
PEM cert/key are elided here (see the `.ino`); the C++ is verbatim with comments.

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_TLS 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/websocket/websocket.h" // ws_pool[] for the echo payload
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Throwaway self-signed-style ECDSA P-256 server cert + key. DEMO ONLY.
static const char SERVER_CERT_PEM[] = R"PEM(-----BEGIN CERTIFICATE-----
... demo server cert (full PEM in the .ino) ...
-----END CERTIFICATE-----
)PEM";
static const char SERVER_KEY_PEM[] = R"PEM(-----BEGIN EC PRIVATE KEY-----
... demo server key - PUBLIC (full PEM in the .ino) ...
-----END EC PRIVATE KEY-----
)PEM";

void ws_connect(uint8_t ws_id)
{
    server.ws_send_text(ws_id, "secure channel up - type something");
}

void ws_message(uint8_t ws_id) // payload is plaintext even though the frame was encrypted
{
    char out[WS_FRAME_SIZE + 8];
    snprintf(out, sizeof(out), "echo: %s", (const char *)ws_pool[ws_id].buf);
    server.ws_send_text(ws_id, out);
}

void ws_close(uint8_t ws_id)
{
    (void)ws_id;
}

void sse_connect(uint8_t sse_id)
{
    server.sse_send(sse_id, "subscribed", "tick");
}

void setup()
{
    Serial.begin(115200);

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

    // Identical callbacks to the plaintext examples...
    server.on_ws("/ws", ws_connect, ws_message, ws_close);
    server.on_sse("/events", sse_connect);

    // ...only begin_tls() differs: now everything is encrypted on :443.
    int32_t result = server.begin_tls(443, (const uint8_t *)SERVER_CERT_PEM, sizeof(SERVER_CERT_PEM),
                                      (const uint8_t *)SERVER_KEY_PEM, sizeof(SERVER_KEY_PEM));
    if (result < 0)
        Serial.printf("begin_tls() failed (error %d)\n", result);
    else
        Serial.println("wss:// + TLS SSE server on :443");
}

void loop()
{
    server.handle();

    // Push an SSE counter once a second (encrypted over TLS to subscribers).
    static uint32_t last = 0;
    static uint32_t n = 0;
    if (millis() - last >= 1000)
    {
        last = millis();
        char buf[24];
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)n++);
        server.sse_broadcast("/events", buf, "tick");
    }
}
```
