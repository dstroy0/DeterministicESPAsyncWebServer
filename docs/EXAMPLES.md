# Examples

The library ships 85 runnable examples under [examples/](../examples). They are
grouped by the OSI layer the feature lives at and numbered within each group.
Every example is a self-contained `.ino` sketch whose header comment documents
what it shows, the build flags it needs, and (where relevant) `curl` commands to
exercise it. This page is the index plus the build, run, and troubleshooting
guide that applies to all of them.

-   [Foundation](#foundation) - the tutorial path: start here.
-   [L4 Transport](#l4-transport) - connections, encryption, flood defense.
-   [L5 Session](#l5-session) - interactive consoles.
-   [L6 Presentation](#l6-presentation) - parsing, codecs, auth, WebSocket/SSE.
-   [L7 Application](#l7-application) - routing, protocols, services, clients.

## Building and running an example

Most examples need WiFi: open the `.ino` and set `SSID` / `PASSWORD` (shown as
`YOUR_SSID` / `YOUR_PASSWORD`) to your network before flashing.

### PlatformIO (recommended)

Compile one example for an ESP32 board with `pio ci`:

```sh
pio ci --board=esp32dev \
  --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_WEBSOCKET=1" \
  --lib="." examples/L6-Presentation/09.WebSocket/09.WebSocket.ino
```

To flash and watch the serial monitor, copy the example into a normal
PlatformIO project's `src/` (as `main.cpp` or keep the `.ino`), set the same
`build_flags` in `platformio.ini`, then:

```sh
pio run -t upload && pio device monitor -b 115200
```

> **The build_flags gotcha (read this).** A sketch enables a feature with a
> `#define DETWS_ENABLE_X 1` _before_ including the library. That `#define` only
> affects the sketch's own translation unit. The library is compiled separately,
> so its `.cpp` files do **not** see the sketch's `#define` and the feature stays
> compiled out, producing link errors like `undefined reference to begin_tls`.
> When you build with `pio ci` (or split the sketch and library across
> translation units), pass the feature flags as `build_flags` (the `-D...` form
> above) so the **library** is compiled with them too. The flags each example
> needs are listed in the tables below and in the example's header comment.

### Arduino IDE

Copy the example folder into your Arduino `libraries`/sketch area. Because the
Arduino IDE compiles the library with your sketch, the in-sketch
`#define DETWS_ENABLE_X 1` (placed before `#include`) is enough; alternatively
set the flags in [src/DetWebServerConfig.h](../src/DetWebServerConfig.h). Select
an ESP32 board and upload.

## Troubleshooting

-   **`undefined reference to ...` when a feature should be enabled** - the
    `build_flags` gotcha above. Pass the feature's `-DDETWS_ENABLE_*` flags to the
    library build, not just the sketch.
-   **`#error "... requires ..."` at compile time** - you enabled a feature
    without its prerequisite. See the
    [build-flag dependency tree](../README.md#build-flag-dependencies); for
    example `WS_DEFLATE` needs `WEBSOCKET`, and `MQTT_TLS` needs both `MQTT` and
    `TLS`.
-   **The device never connects to WiFi** - set `SSID` / `PASSWORD` in the sketch.
    TLS examples also need wall-clock time for certificate validity; pair them with
    the SNTP example if your certificates are time-sensitive.
-   **`begin()` returns a negative value** - a capacity constant is too small for
    the configured pools (for example `MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS`).
    The compile-time checks in `DetWebServerConfig.h` catch most of these first.
-   **A board is attached but the build is not flashed** - `pio ci` only compiles.
    Use `pio run -t upload` from a project that contains the sketch.

## Foundation

The numbered tutorial path. Work through these in order; they use only
always-on core features (HTTP/1.1, routing, middleware, JSON, templating).

| Example                                                     | Build flags | Demonstrates                                                                      |
| ----------------------------------------------------------- | ----------- | --------------------------------------------------------------------------------- |
| [01.Basic](../examples/Foundation/01.Basic)                 | core        | Entry-level tour of the core DetWebServer features.                               |
| [02.Advanced](../examples/Foundation/02.Advanced)           | core        | RESTful CRUD APIs, header verification, and richer routing.                       |
| [03.Expert](../examples/Foundation/03.Expert)               | core        | Connection-pool inspection, request profiling, and advanced control.              |
| [04.Sysadmin](../examples/Foundation/04.Sysadmin)           | core        | A dark-themed admin dashboard with system controls and diagnostics.               |
| [05.Configuration](../examples/Foundation/05.Configuration) | core        | Reference for every configurable flag/constant, with routes that echo the config. |

## L4 Transport

| Example                                                         | Build flags                                                    | Demonstrates                                                |
| --------------------------------------------------------------- | -------------------------------------------------------------- | ----------------------------------------------------------- |
| [01.KeepAlive](../examples/L4-Transport/01.KeepAlive)           | `DETWS_ENABLE_KEEPALIVE`                                       | HTTP/1.1 persistent connections (keep-alive).               |
| [02.AcceptThrottle](../examples/L4-Transport/02.AcceptThrottle) | `DETWS_ENABLE_ACCEPT_THROTTLE`                                 | Connection-flood defense via a global accept-rate throttle. |
| [03.HTTPS](../examples/L4-Transport/03.HTTPS)                   | `DETWS_ENABLE_TLS`                                             | HTTPS via the deterministic static-pool mbedTLS engine.     |
| [04.mTLS](../examples/L4-Transport/04.mTLS)                     | `DETWS_ENABLE_TLS`, `DETWS_ENABLE_MTLS`                        | Mutual TLS: require and verify a client certificate.        |
| [05.PerIpThrottle](../examples/L4-Transport/05.PerIpThrottle)   | `DETWS_ENABLE_ACCEPT_THROTTLE`, `DETWS_ENABLE_PER_IP_THROTTLE` | Per-source-IP connection-flood defense.                     |
| [06.TlsResumption](../examples/L4-Transport/06.TlsResumption)   | `DETWS_ENABLE_TLS`, `DETWS_ENABLE_TLS_RESUMPTION`              | HTTPS with TLS session resumption (RFC 5077 tickets).       |
| [07.IpAllowlist](../examples/L4-Transport/07.IpAllowlist)       | `DETWS_ENABLE_IP_ALLOWLIST`                                    | Restrict who may connect with a source-IP allowlist.        |

## L5 Session

| Example                                                             | Build flags           | Demonstrates                                                         |
| ------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------- |
| [01.SSH](../examples/L5-Session/01.SSH)                             | `DETWS_ENABLE_SSH`    | SSH server: host key from NVS, auth callbacks, channel echo.         |
| [02.SSHCryptoSelfTest](../examples/L5-Session/02.SSHCryptoSelfTest) | core                  | On-device check that the hardware-accelerated SSH crypto is correct. |
| [03.Telnet](../examples/L5-Session/03.Telnet)                       | `DETWS_ENABLE_TELNET` | Line-oriented Telnet console (RFC 854) on port 23.                   |

## L6 Presentation

| Example                                                                        | Build flags                                      | Demonstrates                                                       |
| ------------------------------------------------------------------------------ | ------------------------------------------------ | ------------------------------------------------------------------ |
| [01.FormParams](../examples/L6-Presentation/01.FormParams)                     | core                                             | Reading `application/x-www-form-urlencoded` POST fields.           |
| [02.Json](../examples/L6-Presentation/02.Json)                                 | core                                             | Zero-heap JSON: `JsonWriter` responses, `json_get_*` requests.     |
| [03.Multipart](../examples/L6-Presentation/03.Multipart)                       | core                                             | Parse a `multipart/form-data` POST body (RFC 7578) in place.       |
| [04.BasicAuth](../examples/L6-Presentation/04.BasicAuth)                       | `DETWS_ENABLE_AUTH`                              | Per-route HTTP Basic authentication (RFC 7617).                    |
| [05.DigestAuth](../examples/L6-Presentation/05.DigestAuth)                     | core                                             | Per-route HTTP Digest authentication (RFC 7616, SHA-256).          |
| [06.JWTAuth](../examples/L6-Presentation/06.JWTAuth)                           | `DETWS_ENABLE_JWT`                               | Stateless route protection with JWT bearer tokens (HS256).         |
| [07.SecureWebSocket](../examples/L6-Presentation/07.SecureWebSocket)           | `DETWS_ENABLE_TLS`                               | Secure WebSocket (`wss://`) and Server-Sent Events over TLS.       |
| [08.ServerSentEvents](../examples/L6-Presentation/08.ServerSentEvents)         | core                                             | SSE (`text/event-stream`) push via `on_sse()` + `sse_broadcast()`. |
| [09.WebSocket](../examples/L6-Presentation/09.WebSocket)                       | core                                             | Bidirectional WebSocket echo (RFC 6455) via `on_ws()`.             |
| [10.WebTerminal](../examples/L6-Presentation/10.WebTerminal)                   | `DETWS_ENABLE_WEB_TERMINAL`                      | Browser "web serial" terminal over WebSocket.                      |
| [11.WebSocketCompression](../examples/L6-Presentation/11.WebSocketCompression) | `DETWS_ENABLE_WS_DEFLATE`                        | WebSocket permessage-deflate (RFC 7692), two-way compression.      |
| [12.AuthLockout](../examples/L6-Presentation/12.AuthLockout)                   | `DETWS_ENABLE_AUTH`, `DETWS_ENABLE_AUTH_LOCKOUT` | Brute-force lockout for HTTP auth.                                 |
| [13.Cbor](../examples/L6-Presentation/13.Cbor)                                 | `DETWS_ENABLE_CBOR`                              | Serve telemetry as compact binary CBOR.                            |
| [14.MsgPack](../examples/L6-Presentation/14.MsgPack)                           | `DETWS_ENABLE_MSGPACK`                           | Serve telemetry as compact binary MessagePack.                     |

## L7 Application

| Example                                                                   | Build flags                                                                    | Demonstrates                                                      |
| ------------------------------------------------------------------------- | ------------------------------------------------------------------------------ | ----------------------------------------------------------------- |
| [01.ChunkedResponse](../examples/L7-Application/01.ChunkedResponse)       | core                                                                           | Streaming a response of unknown length with chunked transfer.     |
| [02.CORS](../examples/L7-Application/02.CORS)                             | core                                                                           | Cross-Origin Resource Sharing headers via `set_cors()`.           |
| [03.InterfaceFilter](../examples/L7-Application/03.InterfaceFilter)       | core                                                                           | Gate routes to the station or softAP interface.                   |
| [04.Middleware](../examples/L7-Application/04.Middleware)                 | core                                                                           | Composable middleware chain plus the built-in rate limiter.       |
| [05.PathParams](../examples/L7-Application/05.PathParams)                 | core                                                                           | Capturing `:name` segments from the request path.                 |
| [06.RegexRoutes](../examples/L7-Application/06.RegexRoutes)               | core                                                                           | Match routes with a bounded, allocation-free regular expression.  |
| [07.ResponseHeaders](../examples/L7-Application/07.ResponseHeaders)       | core                                                                           | Custom response headers and cookies.                              |
| [08.Templating](../examples/L7-Application/08.Templating)                 | core                                                                           | Response templating with `{{name}}` placeholder substitution.     |
| [09.ETag](../examples/L7-Application/09.ETag)                             | `DETWS_ENABLE_ETAG`                                                            | Conditional GET with ETag for served files.                       |
| [10.FileServing](../examples/L7-Application/10.FileServing)               | `DETWS_ENABLE_FILE_SERVING`                                                    | Serve a static site from LittleFS with `serve_static()`.          |
| [11.FileUpload](../examples/L7-Application/11.FileUpload)                 | `DETWS_ENABLE_UPLOAD`                                                          | Streaming file upload: POST a body straight into a LittleFS file. |
| [12.Range](../examples/L7-Application/12.Range)                           | `DETWS_ENABLE_RANGE`                                                           | HTTP Range requests / 206 Partial Content (RFC 7233).             |
| [13.CoAP](../examples/L7-Application/13.CoAP)                             | `DETWS_ENABLE_COAP`                                                            | Zero-heap CoAP server (RFC 7252) on UDP/5683.                     |
| [14.SNMP](../examples/L7-Application/14.SNMP)                             | `DETWS_ENABLE_SNMP`, `DETWS_ENABLE_SNMP_V3`                                    | SNMP v1/v2c/v3 agent on UDP/161 (Get/GetNext/GetBulk/Set).        |
| [15.mDNS](../examples/L7-Application/15.mDNS)                             | `DETWS_ENABLE_MDNS`                                                            | Advertise the device over mDNS / DNS-SD.                          |
| [16.OTA](../examples/L7-Application/16.OTA)                               | `DETWS_ENABLE_OTA`                                                             | Authenticated over-the-air firmware update.                       |
| [17.Provisioning](../examples/L7-Application/17.Provisioning)             | `DETWS_ENABLE_PROVISIONING`                                                    | First-boot WiFi provisioning via a captive portal.                |
| [18.SNTP](../examples/L7-Application/18.SNTP)                             | `DETWS_ENABLE_NTP`                                                             | Wall-clock time via SNTP.                                         |
| [19.Syslog](../examples/L7-Application/19.Syslog)                         | `DETWS_ENABLE_SYSLOG`                                                          | Remote logging to a syslog server (RFC 5424 over UDP).            |
| [20.Diagnostics](../examples/L7-Application/20.Diagnostics)               | `DETWS_ENABLE_DIAG`                                                            | Compile-time configuration endpoint.                              |
| [21.PrometheusMetrics](../examples/L7-Application/21.PrometheusMetrics)   | `DETWS_ENABLE_STATS`, `DETWS_ENABLE_METRICS`                                   | Prometheus `/metrics` endpoint (text exposition 0.0.4).           |
| [22.Stats](../examples/L7-Application/22.Stats)                           | `DETWS_ENABLE_STATS`                                                           | Runtime statistics endpoint.                                      |
| [23.HttpClient](../examples/L7-Application/23.HttpClient)                 | `DETWS_ENABLE_HTTP_CLIENT`, `DETWS_ENABLE_TLS`, `DETWS_ENABLE_HTTP_CLIENT_TLS` | Outbound HTTP(S) client requests to a remote server.              |
| [24.MqttClient](../examples/L7-Application/24.MqttClient)                 | `DETWS_ENABLE_MQTT`, `DETWS_ENABLE_TLS`, `DETWS_ENABLE_MQTT_TLS`               | MQTT 3.1.1 client: publish/subscribe to a broker.                 |
| [25.WebSocketClient](../examples/L7-Application/25.WebSocketClient)       | `DETWS_ENABLE_WS_CLIENT`, `DETWS_ENABLE_TLS`, `DETWS_ENABLE_WS_CLIENT_TLS`     | Outbound WebSocket client to a WS server.                         |
| [26.SnmpTrap](../examples/L7-Application/26.SnmpTrap)                     | `DETWS_ENABLE_SNMP`, `DETWS_ENABLE_SNMP_V3`, `DETWS_ENABLE_SNMP_TRAP`          | Outbound SNMP notifications: push traps to a manager.             |
| [27.CoapObserve](../examples/L7-Application/27.CoapObserve)               | `DETWS_ENABLE_COAP`, `DETWS_ENABLE_COAP_OBSERVE`                               | CoAP resource observation (RFC 7641): server pushes updates.      |
| [28.CoapBlock](../examples/L7-Application/28.CoapBlock)                   | `DETWS_ENABLE_COAP`, `DETWS_ENABLE_COAP_BLOCK`                                 | CoAP block-wise transfer (RFC 7959): large responses/uploads.     |
| [29.WebDav](../examples/L7-Application/29.WebDav)                         | `DETWS_ENABLE_WEBDAV`                                                          | WebDAV file share (RFC 4918) backed by LittleFS.                  |
| [30.ModbusTcp](../examples/L7-Application/30.ModbusTcp)                   | `DETWS_ENABLE_MODBUS`                                                          | Modbus TCP slave/server on TCP/502.                               |
| [31.TimeSourceFallback](../examples/L7-Application/31.TimeSourceFallback) | `DETWS_ENABLE_NTP`, `DETWS_ENABLE_TIME_SOURCE`                                 | Multi-source time fallback by priority.                           |
| [32.DeviceUuid](../examples/L7-Application/32.DeviceUuid)                 | `DETWS_ENABLE_DEVICE_ID`                                                       | Stable MAC-derived device UUID.                                   |
| [33.Csrf](../examples/L7-Application/33.Csrf)                             | `DETWS_ENABLE_CSRF`                                                            | CSRF protection for state-changing requests.                      |
| [34.Telemetry](../examples/L7-Application/34.Telemetry)                   | `DETWS_ENABLE_TELEMETRY`                                                       | Moving-window stats, rate-of-change, and a totalizer.             |
| [35.Dashboard](../examples/L7-Application/35.Dashboard)                   | `DETWS_ENABLE_SSE`, `DETWS_ENABLE_DASHBOARD`, `DETWS_ENABLE_WEBSOCKET`         | Real-time SVG dashboard with live telemetry and WS controls.      |
| [36.NetEgress](../examples/L7-Application/36.NetEgress)                   | core                                                                           | Report which interface outbound traffic is using.                 |
| [37.PartitionMonitor](../examples/L7-Application/37.PartitionMonitor)     | `DETWS_ENABLE_PARTITION_MONITOR`                                               | Flash partition-map monitor endpoint.                             |
| [38.GpioMap](../examples/L7-Application/38.GpioMap)                       | `DETWS_ENABLE_GPIO_MAP`                                                        | Browser GPIO pin-mapper / diagnostics panel.                      |
| [39.UdpTelemetry](../examples/L7-Application/39.UdpTelemetry)             | `DETWS_ENABLE_UDP_TELEMETRY`                                                   | Fire-and-forget UDP telemetry cast.                               |
| [40.Guardrails](../examples/L7-Application/40.Guardrails)                 | `DETWS_ENABLE_GUARDRAILS`                                                      | Runtime heap/stack guardrails.                                    |
| [41.LogBuffer](../examples/L7-Application/41.LogBuffer)                   | `DETWS_ENABLE_LOGBUF`                                                          | Fixed-RAM rotating log buffer with severity traps.                |
| [42.ConfigExport](../examples/L7-Application/42.ConfigExport)             | `DETWS_ENABLE_CONFIG_STORE`, `DETWS_ENABLE_CONFIG_IO`                          | Schema-driven config export / restore.                            |
| [43.ModbusScan](../examples/L7-Application/43.ModbusScan)                 | `DETWS_ENABLE_MODBUS`, `DETWS_ENABLE_MODBUS_MASTER`                            | Modbus master codec + register scanner.                           |
| [44.OtaRollback](../examples/L7-Application/44.OtaRollback)               | `DETWS_ENABLE_OTA_ROLLBACK`                                                    | OTA rollback protection / soft-brick safeguard.                   |
| [45.Totp](../examples/L7-Application/45.Totp)                             | `DETWS_ENABLE_TOTP`                                                            | TOTP two-factor auth (RFC 6238).                                  |
| [46.Webhook](../examples/L7-Application/46.Webhook)                       | `DETWS_ENABLE_WEBHOOK`, `DETWS_ENABLE_HTTP_CLIENT`                             | Outbound webhooks / IFTTT.                                        |
| [47.RadioPower](../examples/L7-Application/47.RadioPower)                 | `DETWS_ENABLE_RADIO_POWER`                                                     | WiFi radio modem-sleep / TX-power controls.                       |
| [48.DnsResolver](../examples/L7-Application/48.DnsResolver)               | `DETWS_ENABLE_DNS_RESOLVER`                                                    | DNS resolver with answer verification.                            |
| [49.AuditLog](../examples/L7-Application/49.AuditLog)                     | `DETWS_ENABLE_AUDIT_LOG`, `DETWS_ENABLE_WEBHOOK`                               | Tamper-evident, hash-chained audit log.                           |
| [50.OidcAuth](../examples/L7-Application/50.OidcAuth)                     | `DETWS_ENABLE_OIDC`                                                            | OpenID Connect ID-token auth (RS256).                             |
| [51.Vfs](../examples/L7-Application/51.Vfs)                               | `DETWS_ENABLE_VFS`                                                             | Unified VFS over a real filesystem.                               |
| [52.GraphQL](../examples/L7-Application/52.GraphQL)                       | `DETWS_ENABLE_GRAPHQL`                                                         | GraphQL query endpoint.                                           |
| [53.EspNow](../examples/L7-Application/53.EspNow)                         | `DETWS_ENABLE_ESPNOW`                                                          | ESP-NOW connectionless peer messaging.                            |
| [54.OAuth2](../examples/L7-Application/54.OAuth2)                         | `DETWS_ENABLE_OAUTH2`, `DETWS_ENABLE_HTTP_CLIENT`                              | OAuth2 authorization-code exchange.                               |
| [55.OpcUa](../examples/L7-Application/55.OpcUa)                           | `DETWS_ENABLE_OPCUA`                                                           | OPC UA Binary server: SecureChannel/Session/Read/Write/Browse.    |
| [56.OpcUaClient](../examples/L7-Application/56.OpcUaClient)               | `DETWS_ENABLE_OPCUA`, `DETWS_ENABLE_OPCUA_CLIENT`                              | OPC UA Binary client driving a server over TCP.                   |

---

For the full per-example detail (route list, `curl` commands, configuration
notes) read the header comment at the top of each `.ino`. For feature
descriptions see [FEATURES.md](FEATURES.md); for which flags depend on which,
see the [build-flag dependency tree](../README.md#build-flag-dependencies).
