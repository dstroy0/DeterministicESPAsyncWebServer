# Examples

The library ships 129 runnable examples under [examples/](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples), grouped by
the OSI layer the feature lives at and numbered within each group. **Each example
has its own README** with a detailed walkthrough, the build flags it needs, how
to build and run it, and the full source reproduced with teaching comments - so
this page is just the index. Click any example below.

## Building and running an example

Most examples need WiFi: open the `.ino` and set `SSID` / `PASSWORD` before
flashing. Compile one for an ESP32 board with `pio ci`:

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_WEBSOCKET=1" \
  --lib="." examples/L6-Presentation/09.WebSocket/09.WebSocket.ino
```

> **The build_flags gotcha.** A sketch's `#define DWS_ENABLE_X 1` only affects
> the sketch's own translation unit; the library is compiled separately and will
> not see it, producing link errors like `undefined reference to begin_tls`. When
> building with `pio ci`, pass each feature's flag as `build_flags` (the `-D...`
> form above) so the **library** is compiled with it too. Each example's README
> lists the exact flags. In the Arduino IDE the library compiles with your sketch,
> so the in-sketch `#define` (or setting it in `ServerConfig.h`) is enough.

## Troubleshooting

- **`undefined reference to ...`** - the build_flags gotcha above; pass the flags to the library build.
- **`#error "... requires ..."`** - an illegal flag combination; see the [build-flag dependency tree](../README.md#build-flag-dependencies).
- **No WiFi** - set `SSID`/`PASSWORD`; TLS examples also need wall-clock time (pair with the SNTP example).
- **`begin()` returns negative** - a capacity constant is too small for the configured pools (the compile-time checks in `ServerConfig.h` catch most first).
- **Built but not flashed** - `pio ci` only compiles; use `pio run -t upload` from a project containing the sketch.

<!-- BEGIN GENERATED EXAMPLE INDEX (docs/utilities/gen_examples.py) -->

## Foundation

Start here: the core tutorial path (01-05), then the v5 hardware-integration examples - DMA, the preempting queue, forwarding, radio gateways, and capture:

[01.Basic](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/01.Basic) ·
[02.Advanced](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/02.Advanced) ·
[03.Expert](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/03.Expert) ·
[04.Sysadmin](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/04.Sysadmin) ·
[05.Configuration](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/05.Configuration) ·
[06.PreemptQueue](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/06.PreemptQueue) ·
[07.DmaIngest](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/07.DmaIngest) ·
[08.PreemptLanes](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/08.PreemptLanes) ·
[09.InterfaceForward](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/09.InterfaceForward) ·
[10.RadioGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/10.RadioGateway) ·
[11.LoRaGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/11.LoRaGateway) ·
[12.Nrf24Gateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/12.Nrf24Gateway) ·
[13.EnOceanGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/13.EnOceanGateway) ·
[14.NfcGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/14.NfcGateway) ·
[15.SigfoxUplink](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/15.SigfoxUplink) ·
[16.ZWaveGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/16.ZWaveGateway) ·
[17.ZigbeeGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/17.ZigbeeGateway) ·
[18.ThreadGateway](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/18.ThreadGateway) ·
[19.Ethernet](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/19.Ethernet) ·
[20.IPv6](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/20.IPv6) ·
[21.WifiCapture](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/21.WifiCapture) ·
[22.CanCapture](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/22.CanCapture) ·
[23.EthernetW5500](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/Foundation/23.EthernetW5500)

## L4 Transport

Connections, encryption, and flood defense:

[01.KeepAlive](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/01.KeepAlive) ·
[02.AcceptThrottle](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/02.AcceptThrottle) ·
[03.HTTPS](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/03.HTTPS) ·
[04.mTLS](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/04.mTLS) ·
[05.PerIpThrottle](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/05.PerIpThrottle) ·
[06.TlsResumption](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/06.TlsResumption) ·
[07.IpAllowlist](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L4-Transport/07.IpAllowlist)

## L5 Session

Interactive consoles:

[01.SSH](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L5-Session/01.SSH) ·
[02.SSHCryptoSelfTest](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L5-Session/02.SSHCryptoSelfTest) ·
[03.SSHHostKey](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L5-Session/03.SSHHostKey) ·
[04.Telnet](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L5-Session/04.Telnet) ·
[04.SSHSftp](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L5-Session/04.SSHSftp)

## L6 Presentation

Parsing, codecs, auth, WebSocket/SSE:

[01.FormParams](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/01.FormParams) ·
[02.Json](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/02.Json) ·
[03.Multipart](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/03.Multipart) ·
[04.BasicAuth](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/04.BasicAuth) ·
[05.DigestAuth](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/05.DigestAuth) ·
[06.JWTAuth](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/06.JWTAuth) ·
[07.SecureWebSocket](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/07.SecureWebSocket) ·
[08.ServerSentEvents](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/08.ServerSentEvents) ·
[09.WebSocket](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/09.WebSocket) ·
[10.WebTerminal](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/10.WebTerminal) ·
[11.WebSocketCompression](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/11.WebSocketCompression) ·
[12.AuthLockout](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/12.AuthLockout) ·
[13.Cbor](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/13.Cbor) ·
[14.MsgPack](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L6-Presentation/14.MsgPack)

## L7 Application

Routing, protocols, services, and clients:

[01.ChunkedResponse](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/01.ChunkedResponse) ·
[02.CORS](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/02.CORS) ·
[03.InterfaceFilter](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/03.InterfaceFilter) ·
[04.Middleware](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/04.Middleware) ·
[05.PathParams](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/05.PathParams) ·
[06.RegexRoutes](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/06.RegexRoutes) ·
[07.ResponseHeaders](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/07.ResponseHeaders) ·
[08.Templating](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/08.Templating) ·
[09.ETag](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/09.ETag) ·
[10.FileServing](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/10.FileServing) ·
[11.FileUpload](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/11.FileUpload) ·
[12.Range](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/12.Range) ·
[13.CoAP](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/13.CoAP) ·
[14.SNMP](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/14.SNMP) ·
[15.mDNS](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/15.mDNS) ·
[16.OTA](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/16.OTA) ·
[17.Provisioning](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/17.Provisioning) ·
[18.SNTP](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/18.SNTP) ·
[19.Syslog](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/19.Syslog) ·
[20.Diagnostics](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/20.Diagnostics) ·
[21.PrometheusMetrics](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/21.PrometheusMetrics) ·
[22.Stats](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/22.Stats) ·
[23.HttpClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/23.HttpClient) ·
[24.MqttClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/24.MqttClient) ·
[25.WebSocketClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/25.WebSocketClient) ·
[26.SnmpTrap](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/26.SnmpTrap) ·
[27.CoapObserve](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/27.CoapObserve) ·
[28.CoapBlock](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/28.CoapBlock) ·
[29.WebDav](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/29.WebDav) ·
[30.ModbusTcp](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/30.ModbusTcp) ·
[31.TimeSourceFallback](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/31.TimeSourceFallback) ·
[32.DeviceUuid](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/32.DeviceUuid) ·
[33.Csrf](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/33.Csrf) ·
[34.Telemetry](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/34.Telemetry) ·
[35.Dashboard](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/35.Dashboard) ·
[36.NetEgress](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/36.NetEgress) ·
[37.PartitionMonitor](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/37.PartitionMonitor) ·
[38.GpioMap](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/38.GpioMap) ·
[39.UdpTelemetry](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/39.UdpTelemetry) ·
[40.Guardrails](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/40.Guardrails) ·
[41.LogBuffer](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/41.LogBuffer) ·
[42.ConfigExport](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/42.ConfigExport) ·
[43.ModbusScan](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/43.ModbusScan) ·
[44.OtaRollback](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/44.OtaRollback) ·
[45.Totp](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/45.Totp) ·
[46.Webhook](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/46.Webhook) ·
[47.RadioPower](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/47.RadioPower) ·
[48.DnsResolver](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/48.DnsResolver) ·
[49.AuditLog](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/49.AuditLog) ·
[50.OidcAuth](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/50.OidcAuth) ·
[51.Vfs](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/51.Vfs) ·
[52.GraphQL](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/52.GraphQL) ·
[53.EspNow](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/53.EspNow) ·
[54.OAuth2](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/54.OAuth2) ·
[55.OpcUa](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/55.OpcUa) ·
[56.OpcUaClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/56.OpcUaClient) ·
[57.SmtpAlert](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/57.SmtpAlert) ·
[58.NtpServer](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/58.NtpServer) ·
[59.StatsdMetrics](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/59.StatsdMetrics) ·
[60.DnsServer](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/60.DnsServer) ·
[61.Rtc](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/61.Rtc) ·
[62.Ld2410](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/62.Ld2410) ·
[63.Mpr121](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/63.Mpr121) ·
[64.Sht3x](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/64.Sht3x) ·
[65.Pca9685](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/65.Pca9685) ·
[66.Ads1115](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/66.Ads1115) ·
[67.Ina219](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/67.Ina219) ·
[68.SmbFileClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/68.SmbFileClient) ·
[69.EthernetDnc](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/69.EthernetDnc) ·
[70.PortForward](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/70.PortForward) ·
[71.MediaStreaming](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/71.MediaStreaming) ·
[72.Umati](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/72.Umati) ·
[73.AdsClient](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/73.AdsClient) ·
[74.PidTuning](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/74.PidTuning) ·
[75.InterfaceBridge](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/75.InterfaceBridge) ·
[76.NtripCaster](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/76.NtripCaster) ·
[77.Sen0192](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/77.Sen0192) ·
[78.CoapSecure](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/78.CoapSecure) ·
[79.EdgeCache](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/79.EdgeCache) ·
[80.MeshCache](https://github.com/dstroy0/DeterministicESPAsyncWebServer/tree/main/examples/L7-Application/80.MeshCache)

<!-- END GENERATED EXAMPLE INDEX -->
