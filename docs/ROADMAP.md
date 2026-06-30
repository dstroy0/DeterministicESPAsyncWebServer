# Roadmap

Forward-looking feature ideas for DeterministicESPAsyncWebServer, organized by
theme and sized **S / M / L**. This is a backlog of possibilities, not a
commitment or a schedule.

- `(shipped)` - already exists in the library today.
- a leading `*` - explicitly requested but not yet built.

Bugfixes, maintenance, known limitations, and the record of shipped work live in
[TODO.md](TODO.md); released changes are in [CHANGELOG.md](CHANGELOG.md). Every
item here must keep the library's core guarantees: no heap after `begin()`,
fixed-size buffers, a host-testable core where possible, and a `DETWS_ENABLE_*`
flag (default off) so it costs nothing when unused.

## Recommended near-term order (value vs. risk)

1. **Quick wins (all done):** Cache-Control beside ETag, runtime build-flag
   endpoint (`server.diag()`), MAC-derived UUID, raw-UDP telemetry cast.
2. **Security hardening cluster:** IP allowlist + brute-force lockout + CSRF
   tokens - all shipped.
3. **Partition-map monitor** (done) + config export / restore - the latter still
   needs NVS enumeration added to the config store first.
4. **Dashboard configurator** - done: telemetry math, the SVG dashboard over SSE,
   and the WebSocket controls + Canvas chart.
5. **Architectural (deliberate):** egress-interface reporting done (the stack
   already owns failover); next Ethernet PHY, GraphQL, OPC UA.

## Concurrency / performance

- [x] Thread-safe transport boundary _(shipped)_ - the cross-thread connection
      fields (state, RX ring head/tail) are `DetAtomic` (acquire/release), so the
      single-producer/single-consumer ring is race-free by construction; proven
      under ThreadSanitizer in CI (env `native_tsan`).
- [x] Dedicated server task _(shipped)_ - `begin()` runs the pipeline in its own
      pinned FreeRTOS task (Core 1 by default) instead of the user's `loop()`,
      which is freed; this also removes the first-connection-after-idle stall that
      came from `loop()`-cadence coupling.
- [x] Core-partitioned parallel workers _(shipped)_ - `DETWS_WORKER_COUNT` (default 1) workers each own a disjoint set of connection slots (round-robin at
      accept) with their own event queue + scratch arena: shared-nothing, no
      hot-path locks, so both cores process disjoint connections in parallel with
      bounded latency (determinism preserved). N=1 is byte-for-byte the original
      single pipeline.
- [x] Thread-safe app push path _(shipped)_ - `DetWebServer::defer(slot, fn, arg)`
      runs a callback in the slot's owning worker, so application code on `loop()`
      or another task can push (SSE/WS sends) without racing the worker.
- [x] Throughput benchmark _(measured)_ - a CPU-bound handler (~0.2s) under 4-way
      concurrency: N=1 ~5.9 req/s vs N=2 ~9.1 req/s (~1.5x; not a full 2x because
      worker 1 shares Core 0 with WiFi/lwIP), single-request latency unchanged.
      Confirms real parallel scaling with determinism intact (no hot-path locks).
- [x] Pluggable monotonic clock _(shipped)_ - `services/det_clock.h`: all library
      timing flows through `detws_millis()` (a single 1000 Hz source). Feed your own
      clock with `detws_set_clock(fn, ticks_per_second)` and it is divided down to
      the internal 1000 Hz, so timeouts/polling keep the tested 1 ms granularity
      whatever your clock's rate; default is the platform `millis()` (host-tested).
- [x] Notification-driven worker drain _(shipped)_ - the worker blocks on its
      FreeRTOS task notification instead of free-running the `vTaskDelay` poll;
      producers (`listener_enqueue`, `detws_defer`) nudge it the moment work is
      queued, so events are serviced immediately. This decouples event latency from
      the idle-sweep cadence: `DETWS_WORKER_POLL_TICKS` is now purely the idle
      interval (default 1, unchanged), so raising it cuts idle wakeups (CPU/power)
      at no latency cost (HW: GET latency identical at `POLL_TICKS` 1 vs 100). Zero
      heap (notifications, not a QueueSet).
- [x] Tuning _(shipped)_ - leaner tcpip recv callback (a received segment is now
      bulk-copied into the slot ring with a single SPSC publish instead of a
      per-byte loop with a per-byte atomic store), and a worker count / core /
      poll-knob tuning guide with the measured latency and idle-wakeup data in
      [TUNING.md](TUNING.md).

### v5 milestone: user-configurable preempting task queue + hardware ingest

The next big concurrency step (the **v5 milestone**): turn the worker model into a
real-time, hardware-ingest pipeline where data lands in a queue and the scheduler
preempts immediately to process it, with the priorities exposed to the user.

- [x] \*Preempting task queue (L) _(shipped)_ - `DETWS_ENABLE_PREEMPT_QUEUE`:
      `services/preempt_queue`, one static (zero-heap) queue feeding a dedicated
      high-priority core-pinned task. **From a task**, `detws_pq_post()` /
      `detws_pq_post_urgent()` (`xQueueSendToBack` / `xQueueSendToFront`) with a
      wait timeout; the scheduler preempts the lower-priority producer the instant
      the item is queued. **From an ISR**, `detws_pq_post_from_isr()`
      (`xQueueSendFromISR` + `portYIELD_FROM_ISR`) switches right after posting,
      not on the next tick. Fail-closed on a full queue. HW-verified (~12 us
      ISR-to-handler latency); host-tested via the fixed-ring core (example
      Foundation/06.PreemptQueue).
- [ ] \*User-configurable task priorities / affinity (M). The processing task is
      created high (e.g. priority 5) and the producer low (e.g. priority 1) via
      `xTaskCreatePinnedToCore`, but the **priority, core pinning, and queue depth
      are user-settable** (config / API) rather than hard-coded - users own their
      task priorities.
- [ ] \*DMA UART / I2C / SPI transfer (L). DMA-driven peripheral transfers so the
      CPU is free during byte movement; the DMA-complete ISR posts to the
      preempting queue. This is the ingest path for the cheap-breakout field-bus
      codecs (CAN over SPI, RS-485 UART, IO-Link, ...).
- [x] \*Deeper clock-awareness (M) _(shipped)_ - `services/det_clock` gains a
      microsecond time base beside the 1000 Hz `detws_millis()`: `detws_micros()`
      (pluggable like the ms clock, ISR-safe, for hardware-event timestamps) plus a
      `DetwsLatencyStat` budget helper (`detws_lat_begin` / `_end` / `_avg_us`:
      count, min/max/mean, over-budget count). Header-only, zero heap. HW-verified
      measuring the preempting queue's ISR-to-handler latency against a budget
      (avg 12 us, 0 over a 50 us budget). Host-tested in `native_clock`.
- [ ] \*Interface forwarding (L), **DMA-driven**. A forwarding plane over the ingest
      pipeline: a frame arriving on one interface (Wi-Fi STA / AP, Ethernet over SPI,
      or a peripheral bus / radio) is forwarded to another by a user-set rule, so the
      device acts as a bridge / router between its interfaces instead of only
      terminating traffic. The byte movement is **DMA** (the DMA UART / I2C / SPI
      transfer above): the inbound DMA-complete ISR hands the buffer straight to the
      outbound DMA descriptor and posts to the preempting queue, so the CPU is free
      during the copy and forwarding is true zero-copy where the peripherals allow
      (DMA descriptor reuse, no intermediate buffer). Fail-closed when a destination
      queue is full; every rule is user-configurable (per-interface allow / deny,
      destination, rate cap). This is the generic data path the wireless gateway
      bridges below sit on top of.

### post-v5: RF / wireless gateway bridges

Once the hardware-ingest pipeline lands (the preempting queue + DMA peripheral
transfers above), the same codec-to-northbound pattern that bridges the wired field
buses (CAN over SPI, RS-485 / IO-Link over UART) extends to **wireless radios**: each
radio is a southbound peripheral reached over **SPI / I2C / UART**, and a gateway
function bridges its frames northbound to the existing WiFi / MQTT / HTTP / WebSocket
stack. Deterministic, zero-heap, fixed buffers; the DMA-complete / data-ready ISR posts
into the preempting queue, so wireless ingest rides the same real-time path as the wired
codecs. (ESP32 Wi-Fi / classic + BLE are already on-chip; everything below is an
external module we wire to a bus.)

SPI radios:

- [ ] \*LoRa / LoRaWAN gateway (L) - Semtech SX127x / SX126x, RFM95/96 over SPI.
      Long-range sub-GHz; bridge raw LoRa frames first, then a bounded LoRaWAN
      Class A uplink/downlink, to MQTT.
- [ ] \*nRF24L01+ gateway (M) - Nordic 2.4 GHz over SPI; cheap point-to-multipoint
      sensor links bridged to the web stack.
- [ ] \*CC1101 sub-GHz gateway (M) - TI 300-928 MHz OOK / 2-FSK over SPI; generic
      ISM-band remotes and sensors.
- [ ] \*Thread / Matter RCP (L) - OpenThread radio co-processor (nRF52840 / EFR32)
      over SPI (spinel framing); 802.15.4 mesh bridged to IP.

UART radios:

- [ ] \*Zigbee NCP gateway (L) - Silicon Labs EZSP (EFR32) / Digi XBee / TI ZNP over
      UART; join as coordinator and bridge Zigbee devices to MQTT.
- [ ] \*Z-Wave Serial API gateway (M) - Silicon Labs 500 / 700-series over UART.
- [ ] \*EnOcean gateway (M) - energy-harvesting 868 MHz ESP3 protocol over UART.
- [ ] \*Sigfox uplink (S) - Wisol / Murata module over UART; tiny low-power uplinks.
- [ ] \*Wi-SUN FAN gateway (L) - sub-GHz IPv6 mesh (Renesas / SiLabs) over UART.

I2C / SPI / UART:

- [ ] \*NFC / RFID gateway (M) - PN532 (I2C / SPI / UART) or MFRC522 (SPI); tag
      read / write bridged to an HTTP / MQTT event.

Built-in radio:

- [ ] \*BLE GATT bridge (M) - on-chip ESP32 BLE (and external HCI-UART modules):
      scan / expose GATT characteristics and bridge them to the web stack.

### post-v5: promiscuous / monitor capture

A read-only capture mode across the same interfaces: instead of joining a network and
terminating traffic, listen to **every** frame on a channel and surface it northbound
(live over WebSocket, or batched to a PCAP / log) for diagnostics, an on-device IDS, and
field debugging. Capture is strictly passive (no injection on the capture path) and
fail-closed: a full capture queue drops frames rather than stalling the live data path.

- [ ] \*Wi-Fi promiscuous / monitor mode (M) - on-chip ESP32 raw 802.11 capture
      (`esp_wifi_set_promiscuous`), with a channel / type filter, streamed to a
      WebSocket or written as PCAP.
- [ ] \*Bus listen-only capture (M) - the wired field-bus codecs in listen-only mode
      (CAN / TWAI listen-only, RS-485 receive-only) decode every frame on the wire
      without ACKing, bridged to the same capture sink.
- [ ] \*Radio channel sniff (L) - the RF gateways above in receive-only mode (sniff a
      LoRa / sub-GHz / 802.15.4 channel without joining) feeding the capture pipeline.

### post-v5: field-perturbation sensing

Sensors that read the environment by measuring a perturbation in an emitted or ambient
field, the same southbound-peripheral pattern pointed at sensing instead of comms: the
device reads the sensor over SPI / I2C / UART and bridges the readings northbound to the
dashboard, telemetry math, and MQTT / WebSocket. The data-ready ISR posts into the
preempting queue, so sensing shares the real-time ingest path.

- [ ] \*EM / radar presence + motion (M) - mmWave radar (24 / 60 GHz: LD2410 / MR60BHA
      over UART, Infineon BGT60 over SPI) and Doppler motion (HB100 / RCWL-0516) for
      presence, motion, distance, and vital-sign (breathing / heart-rate) sensing.
- [ ] \*Capacitive / proximity field sensing (S) - FDC2x14 / MPR121 (I2C): touch,
      proximity, liquid level, and material sensing from capacitance shifts.
- [ ] \*Inductive / EM field sensing (S) - LDC1614 (I2C) inductance-to-digital for
      metal / displacement, plus magnetometer-based EM-field perturbation detection.
- [ ] \*Time-of-flight ranging (S) - VL53L0X / VL53L1X (I2C) optical ToF distance and
      gesture, bridged to the same sink.

## Web / API / UI

- [x] WebSocket permessage-deflate, inbound and outbound _(shipped)_ - bounded fixed-Huffman DEFLATE compresses server-to-client data frames (RSV1), with an uncompressed fallback when the result would not shrink.
- [x] REST substrate, AJAX _(shipped)_.
- [x] Real-time **dashboard** _(shipped, phase 1)_ - `DETWS_ENABLE_DASHBOARD`: a compile-time `DetwsWidget` table served as hand-rolled SVG (gauge / value / bar / sparkline, no external JS) via the `web_assets` pipeline, live over SSE; `detws_dashboard_set` / `_publish` (example 62.Dashboard). **Flagship.**
- [x] Dashboard phase 2 _(shipped)_ - WebSocket controls (button / toggle / slider widgets send values to a `detws_dashboard_on_control` callback) and a Canvas chart widget for dense series.
- [x] **Telemetry math** cluster _(shipped)_ - `services/telemetry`: moving-window stats (mean / variance / stddev / min / max), a rate-of-change tracker, and a trapezoidal run-time totalizer (example 61.Telemetry).
- [x] HTTP caching: `Cache-Control` beside ETag _(shipped)_ - `set_cache_control()` injects it into serve_file / serve_static responses.
- [ ] HTTP delivery (S-M): stale-while-revalidate, service-worker cache injection, delta/offset log fetching.
- [x] CBOR encoder + decoder _(shipped)_ - `DETWS_ENABLE_CBOR`: a zero-heap RFC 8949 writer plus a cursor decoder (`cbor_peek` / `cbor_read_*`, no-copy strings) over caller buffers - ints / strings / bytes / arrays / maps / bool / null / float; host-tested against the spec vectors + round-trip (example 65.Cbor).
- [x] MessagePack encoder + decoder _(shipped)_ - `DETWS_ENABLE_MSGPACK`: a zero-heap streaming writer over a caller buffer - shortest-form ints (fixint / 8 / 16 / 32 / 64) / str / bin / arrays / maps / bool / nil / float32; overflow tracked, fails closed - plus a cursor decoder (`msgpack_peek` / `msgpack_read_*`, no-copy strings, fail-closed on malformed/truncated input, ext reported as INVALID) host-tested against spec vectors + round-trip and fuzzed in the pentest harness (example 66.MsgPack, both directions). Remaining (M-L): Protobuf / FlatBuffers zero-copy.
- [x] GraphQL bounded subset _(shipped)_ - `DETWS_ENABLE_GRAPHQL`: `services/graphql` parses a query into a fixed AST pool (no heap) and emits `{"data":{...}}` shaped by the selection; schema-free (sub-selection = object, leaf = one resolver call, args collected along the path), with nesting + arguments (example 81.GraphQL). Feature-dependent schema generation remains open (M).
- [x] Browser diag tools _(shipped, GPIO mapper)_ - `DETWS_ENABLE_GPIO_MAP`: a compile-time pin table (number / label / direction / live level) served at GET `/gpio` as JSON, with a POST control (`pin`, `level`) that drives a mapped output; the serializer + control parser are host-tested, and the example ships a zero-dependency browser panel (example 67.GpioMap). Remaining (M): ping / tracert panel, web logic analyzer.
- [ ] SPA micro-routing + conditional UI streaming (M); local SCADA/HMI fallback (M).
- [ ] WS MTU-aligned chunking / fragmentation control (M).

## Protocols & integrations

- [x] OPC UA server + client _(shipped, SecurityPolicy None)_ - `DETWS_ENABLE_OPCUA`: the OPC UA Binary built-in-type codec, UA-TCP (UACP) framing, Hello/Acknowledge handshake, OpenSecureChannel, CreateSession/ActivateSession, GetEndpoints, Read/Write (Variant/DataValue), and Browse, served on PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`; example 55.OpcUa), plus an OPC UA client (example 56.OpcUaClient). Host-tested (`native_opcua`, `native_opcua_client`) and HW-verified end-to-end against the `asyncua` reference stack via the interop harness (`opcua-client` 3/3: connect+session, browse Objects, read node). Remaining (L): a secure SecurityPolicy (Basic256Sha256 encryption/signing), subscriptions / monitored items, and Node-RED integration (M).
- [x] Modbus master codec + register scanner _(shipped)_ - `DETWS_ENABLE_MODBUS_MASTER`: `services/modbus/modbus_master` builds read-request ADUs and parses responses (register values or exception) so an app can poll / auto-discover a slave's registers; pure, host-tested as a full round-trip against the slave codec, HW-verified via self-scan (example 72.ModbusScan).
- [ ] Southbound protocol-driver framework (L; Modbus is one today); Modbus atomic register matrix (M).
- [x] Webhooks + IFTTT _(shipped)_ - `DETWS_ENABLE_WEBHOOK`: `services/webhook` builds an IFTTT Maker URL + value1/2/3 JSON payload (pure, host-tested with JSON escaping) and fires it - or any JSON to any URL (Slack/Discord/your API) - via the outbound http_client; HW-verified by a self-loopback POST (example 75.Webhook).
- [ ] Email + SMS fallbacks (SMTP + gateway) (M).
- [x] ESP-NOW peer messaging _(shipped)_ - `DETWS_ENABLE_ESPNOW`: `services/espnow` adds a typed 3-byte envelope (demux by type + length check) over connectionless ESP-NOW, a bounded peer registry, and the esp_now radio binding (begin / add_peer / send / broadcast, decoded frames to a callback for bridging to WS/SSE) (example 82.EspNow). Codec + registry host-tested; ESP-NOW<->MQTT auto-bridge remains open (M).

## Networking / connectivity

- [x] Egress-interface reporting _(shipped)_ - `det_net_egress()` / `det_net_egress_ip()` read the live lwIP default route so the app always knows which interface (WiFi STA / softAP / Ethernet) its traffic is leaving on; the stack owns the actual failover, so no manager or polling tick is added (example 63.NetEgress).
- [ ] Ethernet PHY bring-up (L, greenlit) - a wired-PHY init alongside WiFi; failover + the egress report above already work once both links exist. Multi-interface bridge / graceful escalation (L).
- [ ] IPv6 dual-stack + fallback (L); VPN tunneling + reverse-SSH tunnel to a relay (L).
- [ ] WiFi (M): sniffer / traffic analyzer / RF diag, channel-agility roaming.
- [x] DNS resolver + answer verification _(shipped)_ - `DETWS_ENABLE_DNS_RESOLVER`: `services/dns_resolver` resolves a hostname to IPv4 (lwIP dns_gethostbyname marshalled to tcpip_thread, dotted-quad fast path) and verifies the answer - rejecting 0.0.0.0 / broadcast / loopback / multicast as spoof / DNS-rebinding indicators; classifier + verifier host-tested, HW-verified against live DNS (example 77.DnsResolver). Remaining (M): captive-portal DNS-spoof mitigation, captive-portal auto-teardown timer.
- [x] mDNS TXT / `_https._tcp` / extra services _(shipped)_ - `detws_mdns_txt` / `detws_mdns_add_service`.
- [ ] mDNS adaptive / auto-sleep beacons + a continuous refresher for crowded RF (M).
- [x] Raw-UDP telemetry cast _(shipped)_ - `DETWS_ENABLE_UDP_TELEMETRY`: `services/udp_telemetry` builds InfluxDB line-protocol records (`measurement field=val,...`, host-tested) and casts them to a collector over UDP via `det_udp_sendto`, zero-heap fire-and-forget (example 68.UdpTelemetry).
- [ ] Static-IP fallback automation, dynamic socket recycling, TCP window auto-scaling by free RAM (M).

## Power & radio management

- [x] Radio power _(shipped)_ - `DETWS_ENABLE_RADIO_POWER`: `services/radio_power` applies a WiFi modem-sleep mode (`DETWS_RADIO_WIFI_PS` none/min/max) + an optional max-TX cap (`DETWS_RADIO_MAX_TX_DBM`) in one call (esp_wifi_set_ps / set_max_tx_power), trading throughput for lower average power; mode names host-tested, apply/readback HW-verified (example 76.RadioPower). Remaining: BT-coexistence preference (only relevant on a BT-enabled build).
- [ ] Dynamic network sleep modes / sleep-cycle scheduler (M); dynamic power scaling, thermal throttling, brownout recovery, peripheral power gating (M).

## Security & auth

- [x] Source-IP allowlist / firewall in the accept callback _(shipped)_ - `listener_ip_allow_add` / `listener_ip_allowed` (CIDR rules, `DETWS_ENABLE_IP_ALLOWLIST`; example 58.IpAllowlist).
- [x] Brute-force per-IP exponential lockout _(shipped)_ - `DETWS_ENABLE_AUTH_LOCKOUT`; `auth_lockout_*` table issues 429 + Retry-After on the HTTP auth gate (example 59.AuthLockout).
- [x] CSRF token verification _(shipped)_ - `DETWS_ENABLE_CSRF`; global enforcement on POST/PUT/PATCH/DELETE via a stateless HMAC-signed `X-CSRF-Token` (built-in `GET /csrf` issues it; example 60.Csrf).
- [x] Granular API-token scoping _(shipped)_ - `jwt_claim_str()` reads string claims (sub / role / scope) and `jwt_scope_allows()` matches a space-separated OAuth2 scope claim, so a handler can authorize per role/scope on the verified JWT (example 21.JWTAuth).
- [x] MFA - TOTP two-factor _(shipped)_ - `DETWS_ENABLE_TOTP`: `services/totp` computes / verifies RFC 6238 time-based one-time passwords (HMAC-SHA1 on the software SHA-1, Authenticator-compatible) and decodes base32 secrets, for a second factor on top of password / JWT; host-tested against the RFC vectors, HW-verified (example 74.Totp). An external-API verifier can also be called from a handler via the http_client.
- [x] OIDC ID-token verification + OAuth2 token exchange _(shipped)_ - RS256 relying-party verifier (`services/oidc`: JWKS key selection by kid, real RSA-2048 signature check, iss/aud/exp/nbf, claim extraction) plus the OAuth2 token-endpoint client (`services/oauth2`: authorization_code + refresh_token grants, confidential-client or PKCE, JSON token parsing; example 83.OAuth2). SAML remains open (heavy XML/canonicalization - poor fit) (L).
- [x] Secure boot + flash encryption _(shipped, docs)_ - [docs/SECURE_BOOT.md](SECURE_BOOT.md): a hardening guide for ESP32 Secure Boot v2 + Flash Encryption (and NVS encryption) mapped to the secrets this library holds (SSH host key, TLS key, SNMPv3/JWT secrets, config blobs, audit-log sink). Encrypted config handshake during onboarding remains open (M).
- [x] MAC-derived UUID _(shipped)_ - `detws_uuid_from_mac` / `detws_device_uuid` (RFC 4122 v5; example 57.DeviceUuid).
- [ ] **Modern SSH ECC: Curve25519 key exchange + Ed25519 keys** (L) - today the SSH server offers only `diffie-hellman-group14-sha256` + `rsa-sha2-256`, so a stock client negotiates _down_ to those (interop works as of v4.64.0, but it is the client's last-resort set). Add `curve25519-sha256` KEX and `ssh-ed25519` host + client keys so the client uses its preferred, faster algorithms with no fallback. Evaluate a small constant-time backend that fits the zero-heap / no-stdlib idiom: **micro-ecc** (P-256/ECDSA, tiny), **ed25519-donna** (Ed25519 + X25519, fast portable C), and **wolfSSL Ed25519/Curve25519 with ESP32 hardware acceleration**. The last matters because pure-software curve math is slow on the ESP32 (an Ed25519 verify / X25519 scalar multiplication is heavy); the ESP32-S3 / C-series crypto accelerators (or wolfSSL `WOLFSSL_ESP32_CRYPT`) can cut the handshake cost. Pick one backend, keep RSA as a fallback, add Ed25519 host-key provisioning alongside the RSA DER path, and re-run the OpenSSH interop test with zero forced algorithms.

## Storage & config

- [x] Typed NVS config store _(shipped)_.
- [x] Partition-map status monitor endpoint _(shipped)_ - `DETWS_ENABLE_PARTITION_MONITOR`: `detws_partition_monitor_begin()` serves the flash partition table (label, kind, type/subtype, offset, size, running app slot) as JSON via `esp_partition` / `esp_ota_ops`; kind classifier + serializer host-tested (example 64.PartitionMonitor).
- [x] Config export / restore _(shipped, schema-driven)_ - `DETWS_ENABLE_CONFIG_IO`: `services/config_io` serializes a declared schema of fields to a portable `key=value` text blob and parses one back into the NVS config store - backup / migrate / bulk-provision, deterministic and zero-heap (host-tested round-trip; example 71.ConfigExport). Remaining (M): full enumeration-based export (needs NVS key iteration), ZTP multi-stage provisioning.
- [x] Unified VFS wrapper _(shipped)_ - `DETWS_ENABLE_VFS`: `services/vfs` gives one file API (open/read/write/close, exists/size/remove/rename, whole-file helpers) over a pluggable backend - a zero-heap RAM pool (deterministic, host-tested) or a real `fs::FS` (LittleFS / SD / SPIFFS) on ESP32 - so features target storage without knowing the medium (example 80.Vfs).
- [ ] Wear leveling + log offload (server/SD) (M); hot-swap storage safeties (M).
- [x] OTA rollback protection + soft-brick safeguard _(shipped)_ - `DETWS_ENABLE_OTA_ROLLBACK`: `services/ota_rollback` commits a freshly-updated image once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses, so a bad update self-heals; decision logic pure + host-tested, commit/rollback via esp_ota_ops, HW-verified (example 73.OtaRollback). Remaining: modular partition swapping (M).
- [ ] PSRAM web buffers / zero-copy net buffers (`heap_caps_calloc(MALLOC_CAP_SPIRAM)` at begin) + asset offloading + COMPONENT_EMBED_TXTFILES (M); SPI DMA ping-pong buffers (M).

## Observability, diagnostics & reliability

- [x] Immutable audit logs _(shipped)_ - tamper-evident SHA-256 hash chain in a fixed RAM ring (moving anchor keeps the retained window verifiable), with a pluggable sink for store-and-forward to SD / syslog / an HTTP log service. (FDA 21 CFR Part 11 attestation/e-sign workflow still open.)
- [x] Rotating log buffer + severity traps _(shipped)_ - `DETWS_ENABLE_LOGBUF`: `services/logbuf` keeps the last `DETWS_LOG_LINES` lines in a fixed RAM ring (oldest pruned, no heap), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback on lines at/above a severity threshold (forward critical lines as an SNMP trap / webhook); pure + host-tested (example 70.LogBuffer).
- [ ] Core dump to SD/FTP + live exception-decoder panel (M); zero-overhead abstract logging (M).
- [x] Runtime heap/stack guardrails _(shipped)_ - `DETWS_ENABLE_GUARDRAILS`: `services/guardrails` samples free heap, the heap low-water mark, the largest free block (fragmentation), and a task's remaining stack, and fires a breach callback when any crosses its `DETWS_GUARDRAIL_*` floor - a proactive fail-safe hook on top of the passive /metrics numbers; evaluator + JSON host-tested, served at `/health` (example 69.Guardrails).
- [ ] Fail-safe safe-state + deadlock-detection WDT + watchdog-protected coroutine lifelines (M).
- [ ] Hardware health (M): power-rail voltage-drop logger, SPI-bus CRC audit + clock backoff, GPIO short-circuit test, capacitor-leakage diag.

## Build / tooling

- [x] Runtime build-flag reporter _(shipped)_ - `server.diag()` / `DETWS_ENABLE_DIAG` serves a build-info JSON (example 42.Diagnostics); the feature enumeration could be extended.
- [ ] Hierarchical build-flag tree (M); virtual protocol-mocking toggles (M).
- [~] **Real-protocol interop test harness** (M, per protocol) - _harness shipped_ in
  [test/servers/](../test/servers/): one CLI,
  `python test/servers/interop.py <protocol>`, drives the device against the _real_
  reference implementation, not just our own round-trip. Peers so far: HTTP (stdlib
  client), WebSocket (`websockets`), SNMP (`net-snmp`, `pysnmp` fallback), Modbus
  client + server (`pymodbus`), CoAP (`aiocoap`), MQTT broker (`mosquitto` + `paho`),
  OPC UA client + server (`asyncua`). Each peer says whether the device is the server
  (the harness probes it, `--host ...`) or the client (the harness serves a reference
  peer the device connects to), reports uniform PASS/FAIL, and exits 0/1/2.
  HW-verified against the board on real third-party stacks: HTTP 4/4, WebSocket 3/3,
  CoAP 2/2, Modbus 6/6, SNMP 3/3, MQTT (device client -> real `mosquitto`), and OPC UA
  (`asyncua`) 3/3 - all seven protocol families. Adding a protocol is one module in
  `peers/` (documented in its README). Remaining: wiring it into CI containers, and a
  peer per new protocol as it lands.
- [x] **Pentesting / adversarial suite** _(shipped)_ - a separately-runnable harness (env `native_pentest` + a nightly `Pentest` CI job, _not_ part of the per-commit unit-test run) that fuzzes the untrusted-input parsers (HTTP request line/headers/body, Modbus ADU, base32) with malformed, oversized, partial slowloris-style, binary/protocol-confusion, and deterministically-random input, asserting the device's safety invariants: fixed footprint (no buffer index past its bound), fail-closed (defined error states only), and liveness (no hang/over-read). Plus a documented on-device stress playbook (slowloris / floods / brute-force vs the throttle / lockout / allowlist defenses). Full guide: [PENTEST.md](PENTEST.md). Extend it to the remaining codecs (CBOR / SNMP / CoAP / WS / multipart) as you go.
- [ ] **Server build configurator (CLI + GUI in `configurator/`)** (L) - a guided front end for the ~120 `DETWS_ENABLE_*` and sizing flags so a user assembles a firmware build without hand-editing `build_flags`. The source of truth is a single **master JSON** holding every library option as `define -> {default, type, help, group, footprint}`, generated from / checked against [DetWebServerConfig.h](../src/DetWebServerConfig.h) so it never drifts. A **CLI backend** reads and writes that JSON and emits a `platformio.ini` `build_flags` fragment; a **GUI front end** drives the same backend, presenting the option groups, a live **build-footprint** estimate (flash + RAM per option, from the FEATURES footprint tables), and **mutual-exclusion** so conflicting options (e.g. the single streaming-body sink, or password vs publickey-only) cannot be co-selected. Guardrails are **advisory, not a straitjacket**: if a choice is unwise (a tiny RX ring with SSH, many heavy protocols on one rig, password auth on an exposed port) it explains _why_ and then lets the user do it anyway. Goal: turn the pile of individual features ("nuts and bolts") into one approachable "build this server" machine, with extra emphasis on being beginner-friendly (docs audience: educators + students of every age).

## Protocol & transport versions

The big-ticket transport/protocol upgrades, sequenced last. Each is large (L) and
gated on the internal-piping straightening (a clean transport read/write API) so a
new framing/record layer slots in behind one owner instead of threading through
every layer. The current HTTP/1.1 core already tracks the modern HTTP specs
(RFC 9110 semantics, RFC 9112 messaging - which obsolete RFC 7230/7231).

- [ ] **TLS 1.2** (L, RFC 5246) - explicit TLS 1.2 record/handshake support with a
      pinned, audited cipher suite set, session resumption, and the static-pool
      mbedTLS integration; make the negotiated version observable and configurable.
- [ ] **TLS 1.3** (L, RFC 8446) - TLS 1.3 handshake (1-RTT, optional 0-RTT early
      data with replay safeguards), modern AEAD-only suites, after TLS 1.2 lands.
- [ ] **HTTP/2** (L, RFC 9113) - HPACK header compression (RFC 7541), stream
      multiplexing + flow control, and `h2` ALPN over the TLS layer above; map
      streams onto the deterministic per-connection model without per-stream heap.
- [ ] **HTTP/3** (L, RFC 9114) - QUIC transport (UDP) + HTTP/3 with QPACK
      (RFC 9204), after HTTP/2; the largest item (a full UDP congestion / loss
      recovery + TLS 1.3 transport).

### Supporting HTTP specs (smaller, fold in alongside the above)

- [x] **Cookies** (M, RFC 6265) _(shipped)_ - emission via `set_cookie()` (name/value
      plus a freeform attributes string for `Secure` / `HttpOnly` / `SameSite` /
      `Max-Age` / `Expires` / `Path` / `Domain`), and inbound reading via
      `http_get_cookie(req, name, out, cap)` - parses the request `Cookie` header
      (case-sensitive names, DQUOTE-stripped values, `=` preserved in values),
      mirroring `http_get_header()`. Host-tested (`test_http_parser`, 4 cookie cases);
      pairs with the session / CSRF / auth features.
- [x] **HTTP caching** (RFC 9111) _(shipped)_ - conditional GET on served files via
      `DETWS_ENABLE_ETAG`: a strong `ETag` + `Last-Modified`, and `304 Not Modified`
      for a matching `If-None-Match` or (when no `If-None-Match`) an `If-Modified-Since`
      not older than the file; plus `Cache-Control` via `set_cache_control()`. Remaining
      (optional): response-side freshness heuristics / `Age`, request `Cache-Control`.
- [~] **Forwarded header** (S, RFC 7239) - _parser shipped._
  `http_forwarded_client()` recovers the original client IPv4 + scheme from `Forwarded`
  (RFC 7239) or the de-facto `X-Forwarded-For` / `X-Forwarded-Proto` (leftmost = the
  client; strips DQUOTE / `:port`; IPv6 / `unknown` not keyed). Host-tested
  (`test_http_parser`, 4 cases). It is a helper (like `http_get_cookie`) the app
  reads in a handler; auto-wiring the recovered IP into the per-IP auth lockout /
  audit, gated on a configured trusted upstream (the header is client-spoofable),
  is the optional remaining step. The IP allowlist stays accept-time (the proxy's
  real TCP source).

## Maintenance

- [x] **API directional symmetry pass** _(audited)_ - swept the paired ingress/egress
      surfaces (codec encode/decode, transport read/send, server/client, HTTP
      request/response, the protocol-service parse/build pairs) against the rule that a
      value at a position on ingress sits at the same position on egress. Result: the
      surfaces are symmetric, with one real fix applied - the egress transport API
      (`det_conn_send` / `det_conn_sndbuf` / `det_conn_flush`, plus the private
      `resp_end`) no longer threads a `tcp_pcb *`; it resolves the slot's pcb
      internally, exactly as the RX read path and the client surface already do, so a
      caller can no longer pass a pcb that disagrees with the slot. The remaining LOW
      findings (CBOR `text` vs MessagePack `str`; builder-returns-length vs
      parser-returns-bool) are spec-faithful, consistent house style - left as is.
- [ ] **Refresh build footprints** (S) - regenerate the per-feature / per-example
      flash + RAM footprint tables (the example READMEs and docs/FEATURES.md) so the
      documented numbers track the current build after the shared-primitive dedup
      (det_mime / det_bytes / base64url / DNS) and any later features. Run after a
      batch of changes lands, not per commit.
- [ ] **Audit the library against its standards** (L) - go through every standard in
      [STANDARDS.md](STANDARDS.md) with the live spec text open and check the
      implementation against its MUST / SHOULD list + conformance vectors, to catch
      anything missed while building the foundation. Pull each spec into the scratchpad
      first; turn each gap into a bug (docs/BUGS.md) + a test (ideally a real-peer
      conformance check, see the interop harness item above). The HTTP request-smuggling
      and CoAP-Observe-clock fixes came out of exactly this kind of review, so expect
      more. Track per-standard pass/fail status as it proceeds.

## Low-level networking (raw Layer 2)

The ESP32 _does_ expose raw Layer-2 access on both interfaces, and the hard-real-time
cyclic timing is reachable too - so the earlier "PHY can't do raw L2 / can't hit the
timing" caveats are corrected here. **Timing model:** build the device as a
_single feature + the base web server_, and run the protocol's cyclic loop as a
hardware-timer / RMT ISR feeding a **high-priority preempting task** (a top-priority,
core-pinned FreeRTOS task that preempts the web server) off a small interrupt queue.
The web server runs underneath at lower priority; the cyclic deadline owns the CPU
when it fires. With the whole device budgeted to one cyclic loop, the sub-millisecond
isochronous deadlines are achievable - the determinism guarantees (fixed buffers, no
heap, bounded work) are exactly what makes a hard real-time loop tractable here.

**Front-end assumption (applies to every "hardware-gated" item below):** the user can
attach an external adapter on **SPI / UART / I2C** to supply any missing PHY, radio,
or line transceiver - so the deliverable is always the protocol + a small adapter
driver, never "impossible." Canonical examples: an **EtherCAT Slave Controller**
(e.g. LAN9252) over SPI, a **HART FSK modem** over UART, a **DSRC / C-V2X radio**
module over SPI/UART, an **IO-Link master IC** over SPI, **RS-485 / CAN / SDLC /
MBP / LON** transceivers, etc. **Analog I/O** is native: a **4-20 mA** current loop
(or 0-10 V) is read straight off the ESP32 **ADC** over a known-range sense resistor
(scale = known full-scale range), and driven out via the DAC / PWM - so analog
instrument variables (incl. HART's 4-20 mA primary value) need no special front end.

- [ ] **Raw L2 frame TX/RX** (M, platform enabler) - a thin, zero-heap raw-frame API
      the raw-L2 protocols build on:
    - **Wi-Fi (802.11):** `esp_wifi_80211_tx()` injects arbitrary management / data
      frames (custom beacons, proprietary MAC headers) - bypasses the Wi-Fi state
      machine. Also the basis for the pentest/observability angle (beacon/mgmt crafting).
    - **Wired (802.3):** `esp_eth_transmit()` + `esp_eth_update_input_path()` detach the
      Ethernet MAC from lwIP so the app reads/writes raw Ethernet frames at L2 directly -
      this is what makes PROFINET-RT / EtherCAT / GOOSE / POWERLINK / SERCOS framing
      reachable - the cyclic deadline is met by the high-priority preempting-ISR/task
      model above (single-feature build), not blocked by the platform.
    - **L2 transparent bridge:** capture raw L2 on Wi-Fi and map onto the wire via the
      Ethernet MAC (and back) so the device acts as an unmanaged Wi-Fi-to-wired L2 switch.
      Fixed BSS frame buffers, no heap; one build flag, off by default (raw injection is
      powerful - keep it opt-in).

## Industrial / standards protocols

- [ ] **MTConnect** (L, ANSI/MTC1.4-2018) - implement the MTConnect agent standard:
      the four HTTP request types (`probe`, `current`, `sample`, `asset`) returning
      the MTConnect XML response documents (`MTConnectDevices`, `MTConnectStreams`,
      `MTConnectAssets`, `MTConnectError`) per the schema, with a fixed-capacity
      in-memory data model (Devices -> Components -> DataItems) and a bounded circular
      sample buffer keyed by `instanceId` + `sequence` (the `from`/`count`/`interval`
      semantics, including HTTP long-poll streaming of `sample`). Zero-heap: the device
      model and sample buffer are fixed BSS, the XML is streamed via the chunked
      response pump. Pairs with the existing data-source services (gpio_map / dashboard)
      as MTConnect DataItems.
- [~] **Industrial Ethernet** (XL, EtherNet/IP, PROFINET, EtherCAT) - _EtherNet/IP + CIP
  messaging shipped._ `DETWS_ENABLE_ENIP` (`services/enip`): the encapsulation layer (TCP/UDP 44818) - `eip_build` / `eip_parse` (the 24-octet header), `eip_build_register_session`, and
  `eip_build_send_rr_data` / `eip_parse_send_rr_data` (wrap/unwrap a CIP message via the Common
  Packet Format). `DETWS_ENABLE_CIP` (`services/cip`): the CIP message inside it -
  `cip_build_epath` (class/instance/attribute logical segments), `cip_build_get_attr_single` /
  `cip_build_request`, and `cip_parse_response`. Together they form a working CIP read path
  (wrap the CIP request with `eip_build_send_rr_data`); both verified against the Wireshark
  ENIP/CIP dissectors, host-tested. Remaining: the broader CIP object dictionary +
  implicit/IO messaging. **PROFINET** (RT, raw L2
  frames) and **EtherCAT** rely on raw L2, which the ESP32 _does_ provide (see
  Low-level networking above - `esp_eth_transmit` / `esp_eth_update_input_path`), and
  the IRT/isochronous cyclic deadline is met by the high-priority preempting-ISR/task
  model in a single-feature build (this protocol + base web server only). So the full
  cyclic RT/IRT stack is on the table, not just a discovery subset; PROFINET `DCP` is
  the easy first milestone, EtherCAT/IRT the hard one. Each: fixed BSS
  object/process-image model, no heap.
- [ ] **Fieldbuses** (L, classic serial/CAN buses) - the pre-Ethernet field protocols
      built on the existing zero-heap codec pattern: **CANopen** (CiA 301: object
      dictionary, PDO/SDO, NMT, heartbeat) over the ESP32 TWAI/CAN controller;
      **PROFIBUS-DP** and **DeviceNet** as scope allows (DeviceNet is CIP-over-CAN, so
      it shares the EtherNet/IP object model). Modbus TCP already ships
      ([modbus](../src/services/modbus/)); these extend the same data-model + ADU
      parse/build approach to the CAN/serial fieldbus families, each behind its own
      build flag.
- [~] **Modbus RTU** (M, serial / RS-485) - _codec shipped._ `DETWS_ENABLE_MODBUS_RTU`
  adds `modbus_rtu_process_adu()`: the `[unit-id][PDU][CRC-16/Modbus]` serial frame
  around the existing data model + function-code dispatch - a CRC mismatch or a
  non-matching unit address is dropped silently, a broadcast (address 0) executes
  with no reply. Host-tested (`test_modbus`, 5 RTU cases incl. the 0xCDC5 CRC vector + read round-trip + bad-CRC/wrong-addr/broadcast). The pure codec is fed a complete
  frame; a UART/RS-485 driver (3.5-char inter-frame idle) + HW-over-RS-485 verify are
  the remaining transport step. (Modbus ASCII is a lower-priority follow-on.)
- [ ] **PROFINET / PROFIBUS** (XL, Siemens automation) - the Siemens factory-automation
      stack as a focused target: **PROFIBUS-DP** (RS-485 master/slave, the cyclic DP-V0
      I/O exchange + a GSD-described slave model) and **PROFINET IO** (the Ethernet
      successor: `DCP` device discovery/naming over raw L2, the IO-Device cyclic data
      exchange, and a GSDML device description). RT/IRT timing classes are gated on PHY + timing the stock ESP32 lacks, so scope to the DP-V0 cyclic subset and the
      PROFINET `DCP`/acyclic-record subset first - fixed BSS process image, no heap,
      one build flag each. (Overlaps the broader Industrial-Ethernet and Fieldbus items
      above; this is the dedicated Siemens-interop slice.)
- [~] **DNP3** (L, IEEE 1815) - _data-link frame codec shipped._ `DETWS_ENABLE_DNP3`
  (`services\dnp3`): a zero-heap builder + CRC-validating parser for the data-link layer - `dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC` header block plus the
  CRC'd 16-octet user-data blocks, and `dnp3_parse_frame` validates the header and every
  block CRC (CRC-16/DNP, verified against the canonical 0xEA82 check value) and de-blocks
  the user data. The remaining layers (transport-function segmentation, the application
  objects with groups/variations, Class 0/1/2/3 polling, unsolicited responses,
  time-sync) layer on the de-blocked user data; fixed BSS point database, no heap.
  Optional Secure Authentication (IEEE 1815 SAv5) later.
- [ ] **HART** (M, FieldComm) - process-instrument protocol: the FSK digital signal
      riding the 4-20 mA loop (and HART-IP over UDP 5094 as the gateway-friendly path).
      Command set (universal + common-practice), the device-variable + status model,
      and burst mode; fixed BSS device model, no heap. Three reachable paths: **HART-IP**
      (pure software, no front end), the **4-20 mA primary value read straight off the
      ADC** (known-range shunt - see Front-end assumption), and the full **FSK digital**
      channel via a HART modem IC over UART. No part is a blocker.
- [ ] **CC-Link / CC-Link IE** (L, CLPA) - Mitsubishi's factory networks: **CC-Link**
      (the RS-485 cyclic remote-I/O / remote-device station model) and **CC-Link IE
      Field** (the Gigabit-Ethernet successor, cyclic + transient messaging). The
      cyclic process-image + station model reuse the existing data-model pattern; IE's
      Gbit timing is PHY-gated, so scope to the protocol/cyclic-frame layer as the
      platform allows. Fixed BSS, one build flag per variant.
- [ ] **PROFIBUS PA** (M, process automation) - the process-automation profile of
      PROFIBUS: the same DP-V0/V1 application as PROFINET/PROFIBUS-DP above but over the
      MBP (Manchester Bus Powered, IEC 61158-2) physical layer for hazardous areas,
      with the PA device profiles (transmitters/valves). Builds on the PROFIBUS-DP work;
      the MBP physical layer is hardware-gated (couples to a DP segment via a
      segment coupler in practice).
- [ ] **CANopen** (M, CiA 301) - first-class CANopen over the ESP32 TWAI/CAN
      controller (also noted under Fieldbuses): the object dictionary, PDO/SDO
      transfer, NMT state machine, and heartbeat/node-guarding; fixed BSS object
      dictionary, no heap. The DS401 generic-I/O device profile as the first profile.
- [ ] **IO-Link** (M, IEC 61131-9) - point-to-point sensor/actuator link: the device
      stack (the SIO/SDCI cyclic process data + on-request ISDU parameter service, and
      the IODD-described device model). The 3-wire UART-style physical layer is
      hardware-gated; scope the protocol/state-machine + ISDU layer first. Fixed BSS
      device model, no heap.
- [ ] **POWERLINK** (XL, EPSG) - Ethernet POWERLINK managed-node stack: the cyclic
      isochronous SoC/PReq/PRes/SoA slot schedule plus asynchronous SDO. Raw L2 +
      the high-priority preempting-task timing model (single-feature build) are both
      reachable (see Low-level networking), so the full isochronous managed-node cycle is
      in scope - object dictionary + the SoC/PReq/PRes slot schedule. Fixed BSS, no heap.
- [ ] **SERCOS III** (XL, motion bus) - the real-time motion/drive bus over Ethernet:
      the IDN parameter model + cyclic MDT/AT telegrams. Raw L2 + the preempting-task
      cyclic timing (single-feature build) are reachable (see Low-level networking), so
      the hard-real-time slot schedule is in scope alongside the IDN service channel.
      Fixed BSS, no heap.
- [ ] **DeviceNet** (L, CIP over CAN) - already noted under Fieldbuses: the CIP object
      model (shared with EtherNet/IP) over the ESP32 TWAI/CAN controller -
      predefined master/slave connection set, explicit + I/O messaging. Fixed BSS
      object model, no heap; pairs with the EtherNet/IP CIP work.
- [ ] **LonWorks / LON** (L, ISO/IEC 14908) - the building-automation network: the
      LonTalk protocol + network-variable (SNVT) model. The native TP/FT-10 physical
      layer needs a Neuron/transceiver, so target **LON/IP** (IEC 14908-4, ISO/IEC
      14908 over UDP) first as the host-reachable path. Fixed BSS NV table, no heap.
- [ ] **Modbus Plus** (L, HDLC token bus) - Schneider's token-passing peer bus: the
      HDLC framing + token-rotation MAC and the path/routing model over the Modbus Plus
      data link. Niche and physical-layer-gated (custom 1 Mbit/s bus); scope the
      framing/token layer as the platform allows, reusing the existing Modbus PDU model.
- [ ] **INTERBUS** (L, Phoenix Contact) - the ring/summation-frame fieldbus: the single
      rotating summation frame (each device a shift-register slice) + the PCP parameter
      channel. Ring physical layer is hardware-gated; scope the summation-frame protocol + process-image model, document the gating. Fixed BSS, no heap.
- [~] **AMQP** (L, OASIS AMQP 1.0 / 0-9-1) - _the 0-9-1 frame codec is shipped._
  `DETWS_ENABLE_AMQP` (`services\amqp`): `amqp_protocol_header` (the `"AMQP" 0 0 9 1`
  preamble), `amqp_build_frame` / `amqp_parse_frame` (type + channel + size + payload +
  the 0xCE frame-end), `amqp_build_method` / `amqp_parse_method` (a METHOD frame's
  class-id / method-id / arguments), and `amqp_build_heartbeat`; host-tested. Remaining:
  the 0-9-1 method-argument field encoding (the connection/channel/exchange/queue/basic
  classes) for RabbitMQ interop, and the **AMQP 1.0** type system + framing (open / begin
  / attach / transfer / disposition) for broker links. Zero-heap: fixed link/session
  state, the payload streamed through the client transport, one build flag. Pairs with the
  MQTT / webhook outbound integrations.
- [~] **DF1 / DH+** (M-L, Allen-Bradley / Rockwell) - _DF1 full-duplex frame codec
  shipped._ `DETWS_ENABLE_DF1` (`services\df1`): a zero-heap framing + DLE byte-stuffing +
  BCC/CRC codec for the serial link layer (pub. 1770-6.5.16) - `df1_build_frame` wraps
  application data in `DLE STX ... DLE ETX` (doubled-DLE escape) with a BCC (2's
  complement of the data sum) or CRC-16/ARC (over the data + ETX, low byte first), and
  `df1_parse_frame` validates and un-stuffs; vectors verified against the manual (BCC
  `0x20`->`0xE0`, CRC `0xBB3D`). Remaining: the **PCCC** command set (PLC-5 / SLC-500
  data-table read/write) inside the application data, the half-duplex master/slave
  framing, and **DH+** (Data Highway Plus token LAN, physical-layer-gated). Fixed BSS,
  no heap.
- [~] **S7comm / S7comm-Plus** (L, Siemens S7) - _ISO-on-TCP framing + the S7comm read
  path shipped._ The TPKT + COTP transport (RFC 1006 / X.224, port 102) is a reusable codec
    - `DETWS_ENABLE_COTP` (`services\cotp`): `tpkt_build` / `tpkt_parse`, `cotp_build_dt`,
      `cotp_build_cr`, `cotp_parse`. **S7comm** itself is shipped - `DETWS_ENABLE_S7COMM`
      (`services\s7comm`): `s7_build_setup` (Setup Communication), `s7_build_read_request` (Read
      Var, S7-ANY items over DB/I/Q/M with the byte-to-bit address encoding), `s7_parse_header`,
      and `s7_read_next_item` (the response data items, honoring the length-in-bits transport
      sizes and the even-item padding); constants verified against the Wireshark dissector,
      host-tested. Remaining: S7comm **write** (function 0x05) + the userdata services, and
      **S7comm-Plus** (the S7-1200/1500 successor with its session/integrity wrapping). Fixed
      BSS data model, no heap.
- [~] **MELSECNET** (L, Mitsubishi) - _MC protocol binary 3E batch-read shipped._
  `DETWS_ENABLE_MELSEC` (`services\melsec`): `melsec_build_read` emits the binary 3E
  batch-read (word) frame (little-endian fields, subheader 0x5000, command 0x0401, the
  device code + 24-bit head device + point count) and `melsec_parse_response` validates
  the 0xD000 response and reports the end code + data; layout + device codes verified
  against a third-party MC impl, host-tested. Completes the major-vendor PLC read set
  (Omron FINS / Host Link, AB DF1, Siemens S7comm). Remaining: MC batch write + the
  1E/4E + ASCII frame variants, and **MELSECNET/H** / **/10** (the cyclic control
  network, PHY/timing-gated). Fixed BSS device model, no heap.
- [~] **FINS** (M-L, Omron) - _FINS/UDP frame codec shipped._ `DETWS_ENABLE_FINS`
  (`services\fins`): a zero-heap command/response builder + parser - `fins_build_command`
  / `fins_build_memory_area_read` emit the 10-octet routing header + MRC/SRC command code + parameters, and `fins_parse_command` / `fins_parse_response` read them back (the
  response MRES/SRES end code included), over the shipped UDP transport. Header layout
  verified against the FINS spec; pure, host-tested. Remaining: the full command set
  (memory-area write / run-stop / clock) and the FINS/TCP framing + the
  FINS/Hostlink-gateway addressing model. Fixed BSS device model, no heap.
- [~] **Host Link** (M, Omron) - _C-mode frame codec shipped._ `DETWS_ENABLE_HOSTLINK`
  (`services\hostlink`): a zero-heap ASCII frame builder + FCS-validating parser for
  Omron's serial C-mode protocol - `hostlink_build` emits `@UU` + header code + text +
  FCS + `*`CR (FCS = the 8-bit XOR from `@` through the text, verified against the
  `@00RD00000010` -> `57` vector), and `hostlink_parse` / `hostlink_end_code` validate and
  split a frame. Pairs with the FINS work (Host Link is the serial sibling); pure,
  host-tested. Remaining: the per-command text encoders (RD/WD/... of the DM/CIO areas)
  and the UART transport. Fixed BSS, no heap.
- [ ] **SNP** (M, GE Fanuc Series Ninety Protocol) - the GE Fanuc Series 90 (90-30 /
      90-70) serial protocol: the SNP / SNP-X master-slave framing over RS-485 for
      register/bit read/write. UART transport, host-testable frame + checksum codec;
      reuses the data-model + ADU parse/build pattern. Fixed BSS, no heap.
- [ ] **DirectNET** (M, AutomationDirect) - the AutomationDirect / Koyo serial protocol
      (the DirectNET HostLink-style framed read/write of V-memory) for DirectLOGIC PLCs.
      UART transport, host-testable framing + checksum codec; pairs with the Modbus-RTU
      and other serial fieldbus work. Fixed BSS, no heap.
- [ ] **IEC 60870-5-101 / -104** (L, power-grid SCADA) - the tele-control telemetry
      protocol: the ASDU information-object model (type IDs, cause-of-transmission,
      single/double points, measured values, commands) shared by **-104** (over TCP
      2404, the APCI/APDU framing with the I/S/U frames + k/w sequence flow control)
      and **-101** (the serial FT 1.2 link layer). -104 first (host-reachable TCP);
      -101 is the serial sibling. Fixed BSS point database, no heap, one build flag.
- [ ] **IEC 61850** (XL, substation automation) - the substation standard: **MMS**
      (Manufacturing Message Specification over ISO-on-TCP 102, the ACSI object model -
      logical devices/nodes, data objects, datasets, reports) as the host-reachable
      client/server core, and **GOOSE** (the raw-L2 multicast event publish for fast
      trips - raw L2 via `esp_eth_*` plus the high-priority preempting-task model (see
      Low-level networking) meet the sub-ms trip deadline in a single-feature build, so
      the APDU encode/decode + dataset model + the fast-retransmit cadence are in scope).
      SCL (the SCD/CID config) drives a fixed BSS model; no heap. Heaviest of the grid
      protocols - sequence behind 60870-5-104.
- [~] **IEEE C37.118** (M-L, synchrophasors) - _frame codec shipped._
  `DETWS_ENABLE_C37118` (`services\c37118`): a zero-heap builder + CRC-validating parser
  for the PMU synchrophasor wire frame (`SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK`,
  CHK = CRC-CCITT) - `c37118_build_frame` frames any payload, `c37118_build_command`
  handles the fixed Command frame, and `c37118_parse_frame` validates the CRC and reports
  the frame type / id / timestamp / payload, with `c37118_parse_command`. CRC verified
  against the canonical CRC-CCITT-FALSE check value; pure, host-tested. The fixed phasor
  configuration / data model (encoding the CFG-2 channel layout and the matching data
  frame, over TCP/UDP 4712/4713, streamed via the chunked / UDP cast path) remains to be
  layered on top. Pairs with the telemetry-math service for on-device PMU analytics.
- [ ] **IEEE 2030.5 (SEP 2.0)** (M-L, DER / smart energy) - the Smart Energy Profile:
      a RESTful resource model (the function-set resources - DER, metering, demand
      response, pricing) over HTTP + TLS with client-cert auth. This is the best fit of
      the grid protocols for this library - it rides the existing HTTP server, routes,
      and TLS; the work is the SEP resource schema (XML, optionally EXI) + the function
      sets, on a fixed BSS model. Strong near-term candidate among the DER protocols.
- [ ] **OpenADR** (L, demand response) - Open Automated Demand Response (OpenADR 2.0a/b,
      and 3.0 which is REST/JSON + OAuth): the VEN (and optionally VTN) roles exchanging
      events / reports. 3.0 rides the existing HTTP client/server + OAuth2 + JSON cleanly
      (the 2.0 XML/EXI profile is heavier); fixed BSS event/report model, no heap.
- [x] **SunSpec Modbus** (M, DER device models) _(shipped)_ - `DETWS_ENABLE_SUNSPEC`
      (`services\sunspec`): a zero-heap codec for the SunSpec Alliance register maps layered
      on the holding-register model. A model-chain walker (`sunspec_check_marker` /
      `sunspec_begin` / `sunspec_next_model` - verify the `SunS` 0x53756E53 marker, then
      iterate each model's id / length / body to the 0xFFFF end model) + typed point readers
      (`sunspec_u16` / `_i16` / `_u32` / `_i32` / `_string`) and a map writer
      (`sunspec_write_marker` / `_model_header` / point writers / `_write_end_model`), so a
      solar inverter / meter / battery is interoperable. Marker + header format verified
      against the SunSpec spec; pure, host-tested. Pairs with the shipped Modbus
      ([modbus](../src/services/modbus/)).
- [ ] **ICCP / TASE.2** (XL, IEC 60870-6) - the Inter-Control-Center Communications
      Protocol: utility control-center-to-control-center data exchange (data sets,
      transfer sets, devices) carried over **MMS** (ISO-on-TCP 102), so it builds on the
      IEC 61850 MMS stack above. Heavy (the full MMS/ACSI object model + bilateral
      tables); sequence after IEC 61850. Fixed BSS object model, no heap.

### Intelligent Transportation Systems (ITS)

- [ ] **NTCIP** (L, US ITS) - National Transportation Communications for ITS Protocol.
      It rides **SNMP** (and STMP), which this library already ships
      ([snmp](../src/services/snmp/)), so the work is the NTCIP object definitions
      (MIBs) on top of the existing agent, on a fixed BSS model. Target the common
      device classes: **1202** (Actuated Traffic Signal Controllers - phase timing,
      signal states, lane/detector triggers), **1203** (Dynamic Message Signs / variable
      message boards - pushing live road messages), and **1211** (Signal Control and
      Prioritization - emergency-vehicle / transit green-light priority requests). One
      build flag per device class over the shared SNMP core; no heap.
- [ ] **UTMC** (L, UK/EU ITS) - Urban Traffic Management and Control: the modular
      data-sharing framework used in the UK and parts of Europe, designed to share data
      across heterogeneous legacy municipal systems. Implement the UTMC datex/HTTP data
      exchange subset on the existing HTTP server + a fixed object model; scope to the
      common-database message set first. Fixed BSS, no heap.
- [ ] **OCIT** (L, DE/AT/CH ITS) - Open Communication Interface for Road Traffic
      Control: the dominant open field-controller interface in Germany, Austria, and
      Switzerland, defining decentralized communication between field controllers,
      vehicle detectors, and central traffic computers. Implement the OCIT-Outstations
      message set over the existing transport; fixed BSS device/detector model, no heap.
- [ ] **V2X / SAE J2735** (XL, connected vehicle) - the Vehicle-to-Everything message
      dictionary that feeds automated routing + AV safety. Implement the J2735 message
      codec (ASN.1 UPER) for the core messages on a fixed BSS model: **BSM** (Basic
      Safety Message - a vehicle's high-rate position / speed / heading / brake status),
      **SPaT** (Signal Phase and Timing - live signal state + countdown pushed to
      vehicles, pairs with the NTCIP signal-controller work above), and **MAP**
      (intersection lane geometry / turn paths). The UPER codec is host-testable and
      zero-heap; the DSRC / C-V2X radio is supplied by an external module over SPI/UART
      (see Front-end assumption) or bridged via an IP gateway - the message + security
      layer is the deliverable either way.
- [ ] **IEEE 1609 (WAVE)** (XL, vehicular radio stack) - Wireless Access in Vehicular
      Environments: the architecture (1609.3 networking / WSMP, 1609.2 security) for
      secure low-latency highway-speed vehicle networks that carries J2735. The DSRC
      radio/MAC comes from an external V2X module (SPI/UART, see Front-end assumption);
      the deliverable is the WSMP message framing + the 1609.2 security envelope codec on
      a fixed BSS model (pairs with the J2735 work above). No heap.
- [ ] **NEMA TS 2** (M, traffic cabinet bus) - the North-American control-cabinet
      internal bus: a synchronous serial (SDLC) topology linking the controller, the
      Malfunction Management Unit / conflict monitor, and the detector/BIU racks (the
      TS 2 frame set - command/status/detector frames). The SDLC frame + cabinet object
      model is host-testable; the synchronous serial PHY (and the SCdetector/BIU timing)
      is hardware-gated, so scope the frame codec + cabinet model. Fixed BSS, no heap.
- [ ] **ATC** (S, platform note) - the Advanced Traffic Controller spec moves cabinets
      from closed microcontrollers to a standard Linux engine API (the ITS Cabinet /
      ATC API for field-device I/O) for local video analytics + sensor fusion. This is
      a host-platform specification more than a wire protocol; the relevant slice for
      this library is interop - exposing NTCIP / NEMA-TS2 data to an ATC engine over the
      existing HTTP/SNMP surface rather than implementing the Linux ATC stack itself.

### IoT device management

- [~] **LwM2M** (L, OMA LwM2M) - _TLV content format shipped._ `DETWS_ENABLE_LWM2M`
  (`services\lwm2m`): a zero-heap writer + cursor reader for the OMA TLV
  (`application/vnd.oma.lwm2m+tlv`) resource encoding - `lwm2m_tlv_write` (+ typed
  `_write_int` shortest-form / `_write_bool` / `_write_string` / `_write_float` helpers),
  `lwm2m_tlv_read`, and `lwm2m_tlv_value_int`, handling 8-/16-bit ids and inline / 8- /
  16- / 24-bit lengths and the Object-Instance / Resource / Multiple-Resource /
  Resource-Instance kinds; type-byte layout verified against the spec, host-tested. Built
  on the shipped CoAP service ([coap](../src/services/coap/)). Remaining: the client
  interfaces (Bootstrap, Registration, Device Management, Information Reporting / Observe)
  and the standard object model (Security/0, Server/1, Device/3, Firmware Update/5, ...)
  on a fixed BSS model. The **SenML-CBOR / SenML-JSON** content formats are now shipped
  separately - `DETWS_ENABLE_SENML` (`services\senml`): `senml_json_build` /
  `senml_cbor_build` emit a SenML (RFC 8428) pack (base name/time, name, unit, one value,
  time per record) over the JSON / CBOR writers, integral numbers kept as integers; verified
  against the RFC example, host-tested. DTLS is gated on the TLS work; scope the NoSec +
  registration/observe core first. No heap, one flag.

### Messaging & RPC

- [x] **STOMP** (M, messaging) _(shipped)_ - `DETWS_ENABLE_STOMP` (`services\stomp`):
      a zero-heap STOMP 1.2 frame codec - `stomp_build_frame()` writes a frame (command +
      escaped `key:value` headers + blank line + NUL-terminated body) and
      `stomp_parse_frame()` is a non-mutating cursor that reports the command, header
      key/value slices, and body (honoring `content-length`, tolerating `\r\n` endings,
      skipping broker heart-beats), with `stomp_header()` lookup and `stomp_unescape()` for
      header escapes (`\r` `\n` `\c` `\\`). Drives `CONNECT` / `SEND` / `SUBSCRIBE` /
      `MESSAGE` / `ACK` ... against ActiveMQ / RabbitMQ / Artemis over the shipped outbound
      client transport (or STOMP-over-WebSocket via the WS client); pure, host-tested. The
      connection / subscription state is the application's.
- [x] **NATS** (M, messaging) _(shipped)_ - `DETWS_ENABLE_NATS` (`services\nats`): a
      zero-heap codec for the text-based NATS pub/sub protocol - builders for
      `CONNECT` / `PUB` (with optional reply-to) / `SUB` (with optional queue group) /
      `UNSUB` / `PING` / `PONG`, and `nats_parse()` which decodes an inbound
      `MSG` / `INFO` / `PING` / `PONG` / `+OK` / `-ERR` (MSG yields subject / sid / reply-to /
      payload). Line-oriented (CRLF), space-delimited, payload only on PUB/MSG; pure,
      host-tested. Rides the outbound client transport; the connection / subscription state is
      the application's.
- [x] **Sparkplug B** (M, industrial IoT) _(shipped)_ - `DETWS_ENABLE_SPARKPLUG`
      (`services\sparkplug`, implies Protobuf): a zero-heap builder for the Eclipse Sparkplug B
      MQTT payload + topic - `spb_build_topic()` (`spBv1.0/group/type/node[/device]`),
      `spb_build_metric()` (a Tahu Metric: name / alias / timestamp / datatype + an int / long
      / float / double / boolean / string value), and `spb_build_payload()` (timestamp +
      metrics + seq) over the Protobuf codec. Field numbers + datatype codes verified against
      the Eclipse Tahu sparkplug_b.proto; pure, host-tested. Publish it with the MQTT client.
- [~] **gRPC / Protocol Buffers** (L) - _Protobuf wire codec shipped._
  `DETWS_ENABLE_PROTOBUF` (`services\protobuf`): a zero-heap streaming writer
  (`pb_uint64` / `pb_sint64` / `pb_fixed32` / `pb_fixed64` / `pb_float` / `pb_double` /
  `pb_bytes` / `pb_string`, embedded messages via a sub-buffer + `pb_bytes`) and a
  cursor reader (`pb_read_field` over varint / ZigZag / I32 / I64 / length-delimited),
  host-tested against the spec vectors (`08 96 01`, the `"testing"` string, ZigZag
  mapping). **gRPC-Web framing also shipped** - `DETWS_ENABLE_GRPC_WEB`
  (`services\grpcweb`): the 5-octet `[flags][len BE32]` message frame
  (`grpcweb_frame_message`), the 0x80 trailers frame (`grpcweb_frame_trailer`,
  `grpc-status` / `grpc-message`), and `grpcweb_parse`, wrapping the Protobuf codec over the
  shipped HTTP/1.1 server/client (host-tested). Full **gRPC** (the same framing but over
  **HTTP/2** with `application/grpc`) remains gated on the HTTP/2 roadmap item above. Fixed
  BSS, no heap.
- [ ] **DDS** (XL, OMG DDS) - Data Distribution Service: a decentralized peer-to-peer
      data-bus standard built on **RTPS** (the Real-Time Publish-Subscribe wire protocol
      over UDP, with SPDP/SEDP discovery). Implement an RTPS participant with the QoS
      subset that fits a fixed footprint (reliability, history depth), the CDR
      serialization, and a bounded reader/writer/topic table - all zero-heap BSS. Large
      (discovery + the reliability/heartbeat protocol); **DDS-XRCE** (the eXtremely
      Resource Constrained Environments agent/client profile over a single agent link)
      is the more MCU-appropriate entry point - target that first.
- [x] **WAMP** (M, web messaging) _(shipped)_ - `DETWS_ENABLE_WAMP` (`services\wamp`): a
      zero-heap codec for the Web Application Messaging Protocol (unified RPC + PubSub over
      WebSocket, subprotocol `wamp.2.json`). Builders for HELLO / SUBSCRIBE / UNSUBSCRIBE /
      PUBLISH / CALL / REGISTER / YIELD / GOODBYE (JSON arrays emitted via the shared
      `JsonWriter`; Options/Details default to `{}`, Arguments / ArgumentsKw passed as JSON
      literals) and a nesting-aware positional parser (`wamp_get_type` / `wamp_get_uint` /
      `wamp_get_uri` / `wamp_element`) that pulls the message type, ids, and URIs out of an
      inbound WELCOME / SUBSCRIBED / EVENT / RESULT / INVOCATION / ERROR. Message codes
      verified against the WAMP spec; pure, host-tested. It rides the shipped WebSocket
      layer; the session / subscription / registration tables are the application's. The
      caller can still serialize payloads with the MessagePack / CBOR codecs.
- [x] **CloudEvents** (S-M, CNCF spec) _(shipped)_ - `DETWS_ENABLE_CLOUDEVENTS`,
      `services/cloudevents`: `cloudevents_build_json()` emits a structured
      `application/cloudevents+json` envelope (required `id` / `source` / `type` +
      `specversion` 1.0, optional `subject` / `datacontenttype` / `data` - `data` either a
      verbatim JSON value or an escaped string) over the existing JSON writer, and
      `cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers.
      Host-tested (`test_cloudevents`, 7 cases). The HTTP body / `ce-*` header bindings
      are emitted with the normal send / `add_response_header` paths; an app reuses the
      same envelope over MQTT / WebHook. Fixed BSS, no heap, one build flag.
- [x] **MQTT-SN** (M, sensor networks) _(shipped)_ - `DETWS_ENABLE_MQTT_SN`
      (`services\mqtt\mqtt_sn`): a zero-heap MQTT-SN v1.2 wire codec for the UDP / non-TCP
      MQTT variant on constrained, lossy links (numeric topic IDs instead of strings,
      gateway discovery, sleeping-client keep-alive). Builders for CONNECT / REGISTER /
      PUBLISH / SUBSCRIBE (by name or pre-defined id) / PINGREQ / DISCONNECT / SEARCHGW and
      a `mqttsn_parse_header()` (both the 1- and 3-octet Length forms, big-endian fields) +
      typed parsers for CONNACK / REGACK / PUBACK / SUBACK / PUBLISH / REGISTER, with a
      `mqttsn_make_flags()` helper (DUP / QoS / retain / will / clean / TopicIdType). Wire
      bytes verified against the spec + the Eclipse Paho reference; pure, host-tested. The
      datagram send (`det_udp_sendto`), topic-ID registry, and sleep/retransmit state are
      the application's. Pairs with the existing MQTT client.

### Network telemetry

- [x] **NetFlow / IPFIX** (M, flow export) _(shipped)_ - `DETWS_ENABLE_FLOW_EXPORT`
      (`services\flow_export`): a zero-heap exporter-side codec for on-device flow
      accounting. **NetFlow v5** is fixed (`flow_v5_write_header` + `flow_v5_write_record`,
      24- and 48-octet layouts); **NetFlow v9** (RFC 3954) and **IPFIX** (RFC 7011) share a
      small cursor (`FlowWriter`) - `flow_ipfix_begin` / `flow_v9_begin`, then
      `flow_export_template()` (a Template Set/FlowSet), `flow_export_data_begin/record/end`
      (matching Data Set), and `flow_export_finish()` which patches the IPFIX message length
      or the v9 record count (and pads each v9 FlowSet to a 4-octet boundary). Field offsets
      verified against RFC 7011 / RFC 3954 / the published v5 layout; pure, host-tested. The
      flow cache (5-tuple + counters) and the UDP send (`det_udp_sendto`) are the app's.

### Building automation

- [~] **BACnet/IP & BACnet/SC** (L, ASHRAE 135) - _the BVLC + NPDU framing is shipped._
  `DETWS_ENABLE_BACNET` (`services\bacnet`): `bvlc_build` / `bvlc_parse` (the BACnet/IP
  virtual-link envelope - type 0x81, function, length) and `npdu_build` / `npdu_parse`
  (the network layer - version + NPCI control + optional DNET/DADR destination addressing + hop count, slicing the APDU); layout per ASHRAE 135 Annex J / Clause 6, host-tested.
  Remaining: the **APDU** application layer (the object model - Device / Analog-Input /
  Binary-Output / ... objects, properties, ReadProperty / WriteProperty / COV) and the
  BBMD foreign-device registration; then **BACnet/SC** (Secure Connect) reuses the shipped
  WebSocket + static-pool TLS for its BVLC-SC framing + the same APDU/object model. Fixed
  BSS object database, no heap.
- [ ] **XMPP (IoT profile)** (L, XSF) - XMPP with the IoT extensions (XEP-0030 service
      discovery, XEP-0060 pub/sub, XEP-0323 sensor data, XEP-0325 control). The XML
      stream protocol over TCP (with the SASL/TLS handshake) is the heavy part; scope a
      bounded streaming XML parser + the core IoT XEPs on a fixed BSS roster/node model,
      no heap. TLS reuses the shipped client TLS.

### Databases & time-series

- [x] **InfluxDB Line Protocol** (S, time-series ingest) _(shipped)_ - the
      `DetwsLine` builder (`services/udp_telemetry`, `DETWS_ENABLE_UDP_TELEMETRY`) emits
      the full `measurement,tag=v field=v timestamp` form: `detws_line_add_tag()` (escaped
      tag set, must precede fields) + `detws_line_set_timestamp()` on top of the existing
      int/uint/float fields. Host-tested (`test_udp_telemetry`: tags+timestamp ordering,
      special-char escaping, tag-after-field fail-closed). Cast over UDP
      (`detws_udp_telemetry_cast`) or POST the same line to InfluxDB `/write` with the
      shipped HTTP client. Pairs with the telemetry-math service. One build flag, no heap.
- [~] **NoSQL / database clients** (M-L, candidate) - _Redis RESP codec shipped._
  `DETWS_ENABLE_REDIS` (`services/redis_resp`): a zero-heap `resp_encode_command()`
  (array of bulk strings, binary-safe via explicit arg lengths - drives any command
  incl. SET/GET/HSET/XADD) + a cursor `resp_parse()` reply decoder (simple / error /
  integer / bulk / array / nil; incomplete + malformed fail closed). Host-tested
  (`test_redis_resp`, 8 cases). Drive it over the shipped outbound client transport.
  Heavier candidates (MongoDB wire protocol, Postgres frontend/backend protocol) are
  larger and lower-priority. Fixed BSS, no heap, one flag per backend.

### Motor / actuator control (ESC protocols)

For drone / robotics use - drive ESCs and ingest their telemetry straight from the
web server, using the ESP32's hardware timing peripherals (RMT / MCPWM) so the
microsecond-precise pulse trains run without locking the CPU. Each behind a build
flag; fixed BSS, no heap.

- [ ] **DShot** (M, RMT) - the digital ESC standard (DShot150 / 300 / 600 / 1200): the
      16-bit packet (11-bit throttle, 1-bit telemetry-request, 4-bit CRC) emitted as
      microsecond-precise pulses via the **RMT** peripheral (no CPU lock). The bit-pack + CRC is host-testable; RMT is the HW timing backend. Expose throttle + arming over
      an HTTP/WS/dashboard control.
- [ ] **Bidirectional / Extended DShot (EDShot)** (M, RMT) - DShot with the return
      channel: read live ESC diagnostics (eRPM, temperature, voltage, current, error
      rate) back over the same signal wire, so the server streams real-time ESC telemetry
      (pairs with the telemetry-math + dashboard services). Builds on DShot.
- [ ] **ProShot** (S, RMT) - the hybrid analog/digital ESC protocol (pulse-position
      carries the digital value) - less timing-critical than high-speed DShot, faster
      than PWM. Same RMT backend + packet model as DShot.
- [ ] **OneShot / Multishot** (S, MCPWM) - the fast legacy analog PWM ESC protocols
      (OneShot125 / OneShot42, Multishot) synced to the control loop, driven by the
      **MCPWM** (motor-control PWM) peripheral. A thin pulse-width mapping over MCPWM;
      pairs with the DShot control surface as the legacy fallback.

### Network Time Security (NTS, RFC 8915)

Authenticated time - the secure successor to plain NTP (the current
`DETWS_ENABLE_NTP` client trusts whatever the network hands it, so a MITM can shift
the device's clock and break TLS validity windows, JWT/OIDC `exp`, TOTP, and log
timestamps). NTS adds cryptographic authentication on top of NTPv4 with no shared
secret. Builds on the existing NTP client + the TLS stack + AEAD primitives; fixed
BSS, no heap, behind a build flag.

- [ ] **NTS-KE (Key Establishment)** (L) - a TLS 1.3 handshake to the NTS-KE server
      that yields the AEAD key material (via RFC 5705 exporter), the NTP server
      address, and the initial cookies. Reuses the static-pool mbedTLS client
      (`det_tls` / the MQTT/WS-client TLS path); ALPN `ntske/1`.
- [ ] **NTS-protected NTP** (L) - NTPv4 with the NTS extension fields: Unique
      Identifier, NTS Cookie, and the NTS Authenticator + Encrypted EFs under
      `AEAD_AES_SIV_CMAC_256` (RFC 5297 AES-SIV). Verify the response authenticator
      and the echoed unique-id (anti-replay), then feed the validated offset to
      `det_clock`; rotate the cookie list per exchange. The packet build / parse +
      AES-SIV are host-testable against the RFC vectors; HW-verify against a public
      NTS server (e.g. time.cloudflare.com).
