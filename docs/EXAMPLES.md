# Examples

The library ships 106 runnable examples under [examples/](../examples), grouped by
the OSI layer the feature lives at and numbered within each group. **Each example
has its own README** with a detailed walkthrough, the build flags it needs, how
to build and run it, and the full source reproduced with teaching comments - so
this page is just the index. Click any example below.

## Building and running an example

Most examples need WiFi: open the `.ino` and set `SSID` / `PASSWORD` before
flashing. Compile one for an ESP32 board with `pio ci`:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_WEBSOCKET=1" \
  --lib="." examples/L6-Presentation/09.WebSocket/09.WebSocket.ino
```

> **The build_flags gotcha.** A sketch's `#define DETWS_ENABLE_X 1` only affects
> the sketch's own translation unit; the library is compiled separately and will
> not see it, producing link errors like `undefined reference to begin_tls`. When
> building with `pio ci`, pass each feature's flag as `build_flags` (the `-D...`
> form above) so the **library** is compiled with it too. Each example's README
> lists the exact flags. In the Arduino IDE the library compiles with your sketch,
> so the in-sketch `#define` (or setting it in `DetWebServerConfig.h`) is enough.

## Troubleshooting

- **`undefined reference to ...`** - the build_flags gotcha above; pass the flags to the library build.
- **`#error "... requires ..."`** - an illegal flag combination; see the [build-flag dependency tree](../README.md#build-flag-dependencies).
- **No WiFi** - set `SSID`/`PASSWORD`; TLS examples also need wall-clock time (pair with the SNTP example).
- **`begin()` returns negative** - a capacity constant is too small for the configured pools (the compile-time checks in `DetWebServerConfig.h` catch most first).
- **Built but not flashed** - `pio ci` only compiles; use `pio run -t upload` from a project containing the sketch.

<!-- BEGIN GENERATED EXAMPLE INDEX (docs/utilities/gen_examples.py) -->

## Foundation

Start here: the core tutorial path (01-05), then the v5 hardware-integration examples - DMA, the preempting queue, forwarding, radio gateways, and capture:

[01.Basic](../examples/Foundation/01.Basic) ·
[02.Advanced](../examples/Foundation/02.Advanced) ·
[03.Expert](../examples/Foundation/03.Expert) ·
[04.Sysadmin](../examples/Foundation/04.Sysadmin) ·
[05.Configuration](../examples/Foundation/05.Configuration) ·
[06.PreemptQueue](../examples/Foundation/06.PreemptQueue) ·
[07.DmaIngest](../examples/Foundation/07.DmaIngest) ·
[08.PreemptLanes](../examples/Foundation/08.PreemptLanes) ·
[09.InterfaceForward](../examples/Foundation/09.InterfaceForward) ·
[10.RadioGateway](../examples/Foundation/10.RadioGateway) ·
[11.LoRaGateway](../examples/Foundation/11.LoRaGateway) ·
[12.Nrf24Gateway](../examples/Foundation/12.Nrf24Gateway) ·
[13.EnOceanGateway](../examples/Foundation/13.EnOceanGateway) ·
[14.NfcGateway](../examples/Foundation/14.NfcGateway) ·
[15.SigfoxUplink](../examples/Foundation/15.SigfoxUplink) ·
[16.ZWaveGateway](../examples/Foundation/16.ZWaveGateway) ·
[17.ZigbeeGateway](../examples/Foundation/17.ZigbeeGateway) ·
[18.ThreadGateway](../examples/Foundation/18.ThreadGateway) ·
[19.Ethernet](../examples/Foundation/19.Ethernet) ·
[20.IPv6](../examples/Foundation/20.IPv6) ·
[21.WifiCapture](../examples/Foundation/21.WifiCapture) ·
[22.CanCapture](../examples/Foundation/22.CanCapture)

## L4 Transport

Connections, encryption, and flood defense:

[01.KeepAlive](../examples/L4-Transport/01.KeepAlive) ·
[02.AcceptThrottle](../examples/L4-Transport/02.AcceptThrottle) ·
[03.HTTPS](../examples/L4-Transport/03.HTTPS) ·
[04.mTLS](../examples/L4-Transport/04.mTLS) ·
[05.PerIpThrottle](../examples/L4-Transport/05.PerIpThrottle) ·
[06.TlsResumption](../examples/L4-Transport/06.TlsResumption) ·
[07.IpAllowlist](../examples/L4-Transport/07.IpAllowlist)

## L5 Session

Interactive consoles:

[01.SSH](../examples/L5-Session/01.SSH) ·
[02.SSHCryptoSelfTest](../examples/L5-Session/02.SSHCryptoSelfTest) ·
[03.Telnet](../examples/L5-Session/03.Telnet)

## L6 Presentation

Parsing, codecs, auth, WebSocket/SSE:

[01.FormParams](../examples/L6-Presentation/01.FormParams) ·
[02.Json](../examples/L6-Presentation/02.Json) ·
[03.Multipart](../examples/L6-Presentation/03.Multipart) ·
[04.BasicAuth](../examples/L6-Presentation/04.BasicAuth) ·
[05.DigestAuth](../examples/L6-Presentation/05.DigestAuth) ·
[06.JWTAuth](../examples/L6-Presentation/06.JWTAuth) ·
[07.SecureWebSocket](../examples/L6-Presentation/07.SecureWebSocket) ·
[08.ServerSentEvents](../examples/L6-Presentation/08.ServerSentEvents) ·
[09.WebSocket](../examples/L6-Presentation/09.WebSocket) ·
[10.WebTerminal](../examples/L6-Presentation/10.WebTerminal) ·
[11.WebSocketCompression](../examples/L6-Presentation/11.WebSocketCompression) ·
[12.AuthLockout](../examples/L6-Presentation/12.AuthLockout) ·
[13.Cbor](../examples/L6-Presentation/13.Cbor) ·
[14.MsgPack](../examples/L6-Presentation/14.MsgPack)

## L7 Application

Routing, protocols, services, and clients:

[01.ChunkedResponse](../examples/L7-Application/01.ChunkedResponse) ·
[02.CORS](../examples/L7-Application/02.CORS) ·
[03.InterfaceFilter](../examples/L7-Application/03.InterfaceFilter) ·
[04.Middleware](../examples/L7-Application/04.Middleware) ·
[05.PathParams](../examples/L7-Application/05.PathParams) ·
[06.RegexRoutes](../examples/L7-Application/06.RegexRoutes) ·
[07.ResponseHeaders](../examples/L7-Application/07.ResponseHeaders) ·
[08.Templating](../examples/L7-Application/08.Templating) ·
[09.ETag](../examples/L7-Application/09.ETag) ·
[10.FileServing](../examples/L7-Application/10.FileServing) ·
[11.FileUpload](../examples/L7-Application/11.FileUpload) ·
[12.Range](../examples/L7-Application/12.Range) ·
[13.CoAP](../examples/L7-Application/13.CoAP) ·
[14.SNMP](../examples/L7-Application/14.SNMP) ·
[15.mDNS](../examples/L7-Application/15.mDNS) ·
[16.OTA](../examples/L7-Application/16.OTA) ·
[17.Provisioning](../examples/L7-Application/17.Provisioning) ·
[18.SNTP](../examples/L7-Application/18.SNTP) ·
[19.Syslog](../examples/L7-Application/19.Syslog) ·
[20.Diagnostics](../examples/L7-Application/20.Diagnostics) ·
[21.PrometheusMetrics](../examples/L7-Application/21.PrometheusMetrics) ·
[22.Stats](../examples/L7-Application/22.Stats) ·
[23.HttpClient](../examples/L7-Application/23.HttpClient) ·
[24.MqttClient](../examples/L7-Application/24.MqttClient) ·
[25.WebSocketClient](../examples/L7-Application/25.WebSocketClient) ·
[26.SnmpTrap](../examples/L7-Application/26.SnmpTrap) ·
[27.CoapObserve](../examples/L7-Application/27.CoapObserve) ·
[28.CoapBlock](../examples/L7-Application/28.CoapBlock) ·
[29.WebDav](../examples/L7-Application/29.WebDav) ·
[30.ModbusTcp](../examples/L7-Application/30.ModbusTcp) ·
[31.TimeSourceFallback](../examples/L7-Application/31.TimeSourceFallback) ·
[32.DeviceUuid](../examples/L7-Application/32.DeviceUuid) ·
[33.Csrf](../examples/L7-Application/33.Csrf) ·
[34.Telemetry](../examples/L7-Application/34.Telemetry) ·
[35.Dashboard](../examples/L7-Application/35.Dashboard) ·
[36.NetEgress](../examples/L7-Application/36.NetEgress) ·
[37.PartitionMonitor](../examples/L7-Application/37.PartitionMonitor) ·
[38.GpioMap](../examples/L7-Application/38.GpioMap) ·
[39.UdpTelemetry](../examples/L7-Application/39.UdpTelemetry) ·
[40.Guardrails](../examples/L7-Application/40.Guardrails) ·
[41.LogBuffer](../examples/L7-Application/41.LogBuffer) ·
[42.ConfigExport](../examples/L7-Application/42.ConfigExport) ·
[43.ModbusScan](../examples/L7-Application/43.ModbusScan) ·
[44.OtaRollback](../examples/L7-Application/44.OtaRollback) ·
[45.Totp](../examples/L7-Application/45.Totp) ·
[46.Webhook](../examples/L7-Application/46.Webhook) ·
[47.RadioPower](../examples/L7-Application/47.RadioPower) ·
[48.DnsResolver](../examples/L7-Application/48.DnsResolver) ·
[49.AuditLog](../examples/L7-Application/49.AuditLog) ·
[50.OidcAuth](../examples/L7-Application/50.OidcAuth) ·
[51.Vfs](../examples/L7-Application/51.Vfs) ·
[52.GraphQL](../examples/L7-Application/52.GraphQL) ·
[53.EspNow](../examples/L7-Application/53.EspNow) ·
[54.OAuth2](../examples/L7-Application/54.OAuth2) ·
[55.OpcUa](../examples/L7-Application/55.OpcUa) ·
[56.OpcUaClient](../examples/L7-Application/56.OpcUaClient) ·
[57.SmtpAlert](../examples/L7-Application/57.SmtpAlert) ·
[58.NtpServer](../examples/L7-Application/58.NtpServer) ·
[59.StatsdMetrics](../examples/L7-Application/59.StatsdMetrics) ·
[60.DnsServer](../examples/L7-Application/60.DnsServer)

<!-- END GENERATED EXAMPLE INDEX -->
