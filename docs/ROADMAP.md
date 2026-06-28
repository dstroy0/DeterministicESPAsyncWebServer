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

- [~] OPC UA server _(in progress)_ - `DETWS_ENABLE_OPCUA`: increment 1 ships the OPC UA Binary built-in-type codec, UA-TCP (UACP) framing, and the Hello/Acknowledge handshake served on PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`; example 84.OpcUa), host-tested + HW-verified. SecureChannel (OPN), Session, and the Read service are later increments; SecurityPolicy None. Node-RED integration (M) and an OPC UA client remain open (L).
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
- [x] **Pentesting / adversarial suite** _(shipped)_ - a separately-runnable harness (env `native_pentest` + a nightly `Pentest` CI job, _not_ part of the per-commit unit-test run) that fuzzes the untrusted-input parsers (HTTP request line/headers/body, Modbus ADU, base32) with malformed, oversized, partial slowloris-style, binary/protocol-confusion, and deterministically-random input, asserting the device's safety invariants: fixed footprint (no buffer index past its bound), fail-closed (defined error states only), and liveness (no hang/over-read). Plus a documented on-device stress playbook (slowloris / floods / brute-force vs the throttle / lockout / allowlist defenses). Full guide: [PENTEST.md](PENTEST.md). Extend it to the remaining codecs (CBOR / SNMP / CoAP / WS / multipart) as you go.

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

- [ ] **Cookies** (M, RFC 6265) - `Set-Cookie` / `Cookie` parsing + emission with
      the security attributes (`Secure`, `HttpOnly`, `SameSite`, `Max-Age`/`Expires`,
      `Path`, `Domain`); pairs with the existing session/CSRF/auth features.
- [x] **HTTP caching** (RFC 9111) _(shipped)_ - conditional GET on served files via
      `DETWS_ENABLE_ETAG`: a strong `ETag` + `Last-Modified`, and `304 Not Modified`
      for a matching `If-None-Match` or (when no `If-None-Match`) an `If-Modified-Since`
      not older than the file; plus `Cache-Control` via `set_cache_control()`. Remaining
      (optional): response-side freshness heuristics / `Age`, request `Cache-Control`.
- [ ] **Forwarded header** (S, RFC 7239) - _optional, off by default, trust-proxy
      gated._ Parse `Forwarded` (and de-facto `X-Forwarded-For`/`-Proto`) to recover
      the real client IP + scheme when behind a reverse proxy, so the IP allowlist,
      per-IP auth lockout, audit-by-IP and absolute-URL/scheme logic stay correct.
      Only honored from a configured trusted upstream (the header is client-spoofable;
      trusting it blindly would defeat the allowlist/lockout). Needed only for
      behind-a-proxy deployments - hence optional.

## Maintenance

- [ ] **Refresh build footprints** (S) - regenerate the per-feature / per-example
      flash + RAM footprint tables (the example READMEs and docs/FEATURES.md) so the
      documented numbers track the current build after the shared-primitive dedup
      (det_mime / det_bytes / base64url / DNS) and any later features. Run after a
      batch of changes lands, not per commit.
