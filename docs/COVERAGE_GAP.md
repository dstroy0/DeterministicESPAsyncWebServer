# Feature coverage gap analysis

The living backlog for the three coverage dimensions the library aims to hit for **every protocol
feature**:

1. **Interop** - a real third-party peer in the harness (`test/servers/peers/`, driven by
   `test/servers/interop.py`) so the wire format is validated against an independent implementation.
2. **Throughput bench** - host ns/op + on-device µs/op recorded in
   [`FEATURE_PERFORMANCE.md`](FEATURE_PERFORMANCE.md) (device numbers via the rig `/bench` CCOUNT harness).
3. **Advanced attack** - an EXTREMELY ADVANCED exploit set in `pentesting/detws_pentest.py` (not just a
   random fuzzer) with the determinism + liveness oracles.

`DONE` for the coverage loop = every protocol feature has all three. This file is updated each loop tick.

## Status at a glance

| Dimension        |   Covered | Notes                                                                                                                                         |
| ---------------- | --------: | --------------------------------------------------------------------------------------------------------------------------------------------- |
| Interop peers    |         7 | http, ws, coap, mqtt, modbus, snmp, opcua                                                                                                     |
| Advanced attack  | ~HTTP + 4 | HTTP/1.1 desync+chunk+obs-fold+CRLF+range+upload, WS frame-abuse+ping, auth, csrf, throttle; coap/snmp/modbus/opcua basic fuzz; tls handshake |
| Throughput bench |       ~10 | codecs (base64/hex/mime/mtconnect), storage, request-path (host), smb/dnc/relay/radio, CCOUNT primitives                                      |

The library implements **~50+ network protocols**; interop is the biggest gap by far (7 of ~50).

## Legend

`OK` covered · `~` partial/basic · `-` gap · `n/a` no external peer (device-internal or sensor)

## Protocol matrix (interop-relevant features)

| Protocol / feature                                                                                                | Interop |  Bench  | Adv. attack | Next / notes                                                                 |
| ----------------------------------------------------------------------------------------------------------------- | :-----: | :-----: | :---------: | ---------------------------------------------------------------------------- |
| HTTP/1.1                                                                                                          |   OK    |   OK    |     OK      | desync matrix, chunk abuse, obs-fold, CRLF, oversized, slowloris             |
| WebSocket                                                                                                         |   OK    |    ~    |     OK      | frame-abuse + ping-flood; add permessage-deflate decompression bomb          |
| Server-Sent Events                                                                                                |    -    |    -    |      ~      | interop peer (curl EventSource) + connection-exhaustion attack               |
| HTTP/2                                                                                                            |    -    |    -    |      -      | rapid-reset (CVE-2023-44487) + CONTINUATION flood + HPACK bomb (PSRAM build) |
| HTTP/3 / QUIC                                                                                                     |    -    |    -    |      -      | QUIC initial flood, amplification, QPACK bomb (PSRAM build)                  |
| TLS                                                                                                               |   n/a   |    -    |      ~      | downgrade / renegotiation / 0-RTT replay; ALPN confusion                     |
| Basic/Digest auth                                                                                                 |   n/a   |   OK    |     OK      | auth_bypass (null-trunc fixed); add Digest nonce replay + JWT/OIDC alg=none  |
| CoAP                                                                                                              |   OK    |    -    |      ~      | blockwise reassembly bomb, observe flood; add device bench                   |
| MQTT                                                                                                              |   OK    |    -    |      -      | malformed CONNECT/PUBLISH, topic-wildcard abuse, QoS2 dedup flood            |
| Modbus                                                                                                            |   OK    |   OK    |      ~      | illegal function/quantity, PDU-length lies                                   |
| SNMP                                                                                                              |   OK    |    -    |      ~      | GETBULK amplification (max-repetitions abuse)                                |
| OPC-UA                                                                                                            |   OK    |    -    |      ~      | UACP chunk abuse, oversized message                                          |
| SMB2 client                                                                                                       |    -    |   OK    |      -      | samba peer (device is client); already HW-verified informally                |
| SSH                                                                                                               |    -    |    -    |      -      | real-OpenSSH peer; KEX/cipher negotiation; the big one                       |
| FTP client                                                                                                        |    -    |    -    |      -      | pyftpdlib peer; RFC 959/2428                                                 |
| WebDAV                                                                                                            |    -    |    -    |      -      | litmus/cadaver peer (RFC 4918)                                               |
| Redis (RESP)                                                                                                      |    -    |    -    |      -      | real redis-server peer                                                       |
| AMQP / STOMP / NATS / WAMP / XMPP                                                                                 |    -    |    -    |      -      | messaging: reference brokers exist for each                                  |
| DDS                                                                                                               |    -    |    -    |      -      | RTPS; a real DDS peer (CycloneDDS)                                           |
| DNP3 / IEC-60870 / MMS+GOOSE / S7comm                                                                             |    -    |    -    |      -      | industrial SCADA; reference stacks exist                                     |
| EtherNet/IP / Profinet / Profibus / CANopen / DeviceNet / CC-Link / Sercos / Powerlink / Interbus / FINS / Melsec |    -    | ~(some) |      -      | industrial fieldbus family; large interop gap                                |
| BACnet                                                                                                            |    -    |    -    |      -      | building automation; reference stack (bacnet-stack)                          |
| SunSpec / Sparkplug / SEP2 / OpenADR                                                                              |    -    |    -    |      -      | energy; reference tools exist                                                |
| gRPC-web / GraphQL                                                                                                |    -    |    -    |      -      | web-API; grpcurl / a GraphQL client                                          |
| SMTP / syslog                                                                                                     |    -    |    -    |      -      | server-side; a real mail/syslog client                                       |
| NTP / NTS                                                                                                         |    -    |    -    |      -      | ntpd / an NTS client                                                         |
| mDNS / DNS                                                                                                        |    -    |    -    |      -      | avahi / dig                                                                  |
| MTConnect                                                                                                         |    -    |   OK    |      -      | agent conformance                                                            |
| Zigbee / Z-Wave / LoRa / Sigfox / Wi-SUN / Thread / ESP-NOW                                                       |    -    |    -    |      -      | RF; needs radios (HW plan) - interop is device-to-device                     |

## Priorities (tractable next, easiest reference peer first)

1. **SSE interop peer** + connection-exhaustion attack - the device already serves it; peer is a plain
   EventSource client. Lowest effort, closes an HTTP-family gap.
2. **HTTP/2 rapid-reset + CONTINUATION flood** attacks - high-value CVE class; needs a PSRAM rig variant.
3. **Interop peers with off-the-shelf servers**: Redis (`redis-server`), FTP (`pyftpdlib`), WebDAV
   (`wsgidav` + `litmus`), SMTP (`aiosmtpd`), syslog (`rsyslog`) - the device is the client for most, so
   the peer is just the reference server + a transcript check.
4. **SSH interop** against real OpenSSH - the biggest single gap; needs the transport-negotiation work
   (see the SSH memory) + a device-as-server or device-as-client transcript.
5. **CCOUNT device benches** for the remaining hot paths (request parse, response build, CoAP/MQTT codec)
   through the rig `/bench` endpoint.

## How each dimension is extended

- **Interop**: add `test/servers/peers/<proto>_peer.py` (`NAME`/`HELP`/`add_args`/`run`), register in
  `interop.py` `_MODULES`; see the existing peers for the shape.
- **Bench**: pure host op -> `perf/*.cpp` (host ns/op); device µs/op -> add the op to the rig `/bench`
  endpoint (`pentesting/rig_firmware`) and read it over HTTP; record both in `FEATURE_PERFORMANCE.md`.
- **Attack**: add an `@attack(...)` to `pentesting/detws_pentest.py` gated on the feature flag; keep the
  determinism + liveness oracles and `--authorized`; live-fire against the rig.
