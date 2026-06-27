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
- [x] Compile-time poll-rate knob _(shipped)_ - `DETWS_WORKER_POLL_TICKS` (default
      1 = the tested 1000 Hz) trades latency for idle CPU/power on a battery build
      without touching the default deterministic cadence.
- [ ] Tuning (S): event-queue-blocking worker drain (lower idle CPU than the
      `vTaskDelay` poll), leaner tcpip callbacks, per-workload worker count/affinity.

## Web / API / UI

- [x] WebSocket permessage-deflate inbound _(shipped)_; outbound compress (Phase 2) open (M).
- [x] REST substrate, AJAX _(shipped)_.
- [x] Real-time **dashboard** _(shipped, phase 1)_ - `DETWS_ENABLE_DASHBOARD`: a compile-time `DetwsWidget` table served as hand-rolled SVG (gauge / value / bar / sparkline, no external JS) via the `web_assets` pipeline, live over SSE; `detws_dashboard_set` / `_publish` (example 62.Dashboard). **Flagship.**
- [x] Dashboard phase 2 _(shipped)_ - WebSocket controls (button / toggle / slider widgets send values to a `detws_dashboard_on_control` callback) and a Canvas chart widget for dense series.
- [x] **Telemetry math** cluster _(shipped)_ - `services/telemetry`: moving-window stats (mean / variance / stddev / min / max), a rate-of-change tracker, and a trapezoidal run-time totalizer (example 61.Telemetry).
- [x] HTTP caching: `Cache-Control` beside ETag _(shipped)_ - `set_cache_control()` injects it into serve_file / serve_static responses.
- [ ] HTTP delivery (S-M): stale-while-revalidate, service-worker cache injection, delta/offset log fetching.
- [x] CBOR encoder + decoder _(shipped)_ - `DETWS_ENABLE_CBOR`: a zero-heap RFC 8949 writer plus a cursor decoder (`cbor_peek` / `cbor_read_*`, no-copy strings) over caller buffers - ints / strings / bytes / arrays / maps / bool / null / float; host-tested against the spec vectors + round-trip (example 65.Cbor).
- [x] MessagePack encoder _(shipped)_ - `DETWS_ENABLE_MSGPACK`: a zero-heap streaming writer over a caller buffer - shortest-form ints (fixint / 8 / 16 / 32 / 64) / str / bin / arrays / maps / bool / nil / float32; overflow tracked, fails closed; host-tested against the spec encodings (example 66.MsgPack). Remaining (M-L): MessagePack decoder, Protobuf / FlatBuffers zero-copy.
- [ ] GraphQL bounded subset (L); feature-dependent schema generation (M).
- [x] Browser diag tools _(shipped, GPIO mapper)_ - `DETWS_ENABLE_GPIO_MAP`: a compile-time pin table (number / label / direction / live level) served at GET `/gpio` as JSON, with a POST control (`pin`, `level`) that drives a mapped output; the serializer + control parser are host-tested, and the example ships a zero-dependency browser panel (example 67.GpioMap). Remaining (M): ping / tracert panel, web logic analyzer.
- [ ] SPA micro-routing + conditional UI streaming (M); local SCADA/HMI fallback (M).
- [ ] WS MTU-aligned chunking / fragmentation control (M).

## Protocols & integrations

- [ ] OPC UA server/client (L); Node-RED integration (M).
- [x] Modbus master codec + register scanner _(shipped)_ - `DETWS_ENABLE_MODBUS_MASTER`: `services/modbus/modbus_master` builds read-request ADUs and parses responses (register values or exception) so an app can poll / auto-discover a slave's registers; pure, host-tested as a full round-trip against the slave codec, HW-verified via self-scan (example 72.ModbusScan).
- [ ] Southbound protocol-driver framework (L; Modbus is one today); Modbus atomic register matrix (M).
- [ ] Webhooks + IFTTT (M); Email + SMS fallbacks (SMTP + gateway) (M).
- [ ] ESP-NOW transport + ESP-NOW<->MQTT bridge (M).

## Networking / connectivity

- [x] Egress-interface reporting _(shipped)_ - `det_net_egress()` / `det_net_egress_ip()` read the live lwIP default route so the app always knows which interface (WiFi STA / softAP / Ethernet) its traffic is leaving on; the stack owns the actual failover, so no manager or polling tick is added (example 63.NetEgress).
- [ ] Ethernet PHY bring-up (L, greenlit) - a wired-PHY init alongside WiFi; failover + the egress report above already work once both links exist. Multi-interface bridge / graceful escalation (L).
- [ ] IPv6 dual-stack + fallback (L); VPN tunneling + reverse-SSH tunnel to a relay (L).
- [ ] WiFi (M): sniffer / traffic analyzer / RF diag, channel-agility roaming.
- [ ] DNS (M): async resolver + response verification, captive-portal DNS-spoof mitigation, captive-portal auto-teardown timer.
- [x] mDNS TXT / `_https._tcp` / extra services _(shipped)_ - `detws_mdns_txt` / `detws_mdns_add_service`.
- [ ] mDNS adaptive / auto-sleep beacons + a continuous refresher for crowded RF (M).
- [x] Raw-UDP telemetry cast _(shipped)_ - `DETWS_ENABLE_UDP_TELEMETRY`: `services/udp_telemetry` builds InfluxDB line-protocol records (`measurement field=val,...`, host-tested) and casts them to a collector over UDP via `det_udp_sendto`, zero-heap fire-and-forget (example 68.UdpTelemetry).
- [ ] Static-IP fallback automation, dynamic socket recycling, TCP window auto-scaling by free RAM (M).

## Power & radio management

- [ ] Radio power (S): disable BT coexistence (`esp_coex_preference_set`) + radio sleep, as build options.
- [ ] Dynamic network sleep modes / sleep-cycle scheduler (M); dynamic power scaling, thermal throttling, brownout recovery, peripheral power gating (M).

## Security & auth

- [x] Source-IP allowlist / firewall in the accept callback _(shipped)_ - `listener_ip_allow_add` / `listener_ip_allowed` (CIDR rules, `DETWS_ENABLE_IP_ALLOWLIST`; example 58.IpAllowlist).
- [x] Brute-force per-IP exponential lockout _(shipped)_ - `DETWS_ENABLE_AUTH_LOCKOUT`; `auth_lockout_*` table issues 429 + Retry-After on the HTTP auth gate (example 59.AuthLockout).
- [x] CSRF token verification _(shipped)_ - `DETWS_ENABLE_CSRF`; global enforcement on POST/PUT/PATCH/DELETE via a stateless HMAC-signed `X-CSRF-Token` (built-in `GET /csrf` issues it; example 60.Csrf).
- [x] Granular API-token scoping _(shipped)_ - `jwt_claim_str()` reads string claims (sub / role / scope) and `jwt_scope_allows()` matches a space-separated OAuth2 scope claim, so a handler can authorize per role/scope on the verified JWT (example 21.JWTAuth).
- [ ] MFA hooks -> external API (S).
- [ ] Enterprise identity handshakes - SAML / OAuth2 / OIDC (L).
- [ ] Secure boot + flash encryption (S, docs/eFuse); encrypted config handshake during onboarding (M).
- [x] MAC-derived UUID _(shipped)_ - `detws_uuid_from_mac` / `detws_device_uuid` (RFC 4122 v5; example 57.DeviceUuid).

## Storage & config

- [x] Typed NVS config store _(shipped)_.
- [x] Partition-map status monitor endpoint _(shipped)_ - `DETWS_ENABLE_PARTITION_MONITOR`: `detws_partition_monitor_begin()` serves the flash partition table (label, kind, type/subtype, offset, size, running app slot) as JSON via `esp_partition` / `esp_ota_ops`; kind classifier + serializer host-tested (example 64.PartitionMonitor).
- [x] Config export / restore _(shipped, schema-driven)_ - `DETWS_ENABLE_CONFIG_IO`: `services/config_io` serializes a declared schema of fields to a portable `key=value` text blob and parses one back into the NVS config store - backup / migrate / bulk-provision, deterministic and zero-heap (host-tested round-trip; example 71.ConfigExport). Remaining (M): full enumeration-based export (needs NVS key iteration), unified VFS wrapper, ZTP multi-stage provisioning.
- [ ] Wear leveling + log offload (server/SD) (M); hot-swap storage safeties (M).
- [x] OTA rollback protection + soft-brick safeguard _(shipped)_ - `DETWS_ENABLE_OTA_ROLLBACK`: `services/ota_rollback` commits a freshly-updated image once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses, so a bad update self-heals; decision logic pure + host-tested, commit/rollback via esp_ota_ops, HW-verified (example 73.OtaRollback). Remaining: modular partition swapping (M).
- [ ] PSRAM web buffers / zero-copy net buffers (`heap_caps_calloc(MALLOC_CAP_SPIRAM)` at begin) + asset offloading + COMPONENT_EMBED_TXTFILES (M); SPI DMA ping-pong buffers (M).

## Observability, diagnostics & reliability

- [ ] Immutable audit logs + store-and-forward + FDA 21 CFR Part 11 (L).
- [x] Rotating log buffer + severity traps _(shipped)_ - `DETWS_ENABLE_LOGBUF`: `services/logbuf` keeps the last `DETWS_LOG_LINES` lines in a fixed RAM ring (oldest pruned, no heap), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback on lines at/above a severity threshold (forward critical lines as an SNMP trap / webhook); pure + host-tested (example 70.LogBuffer).
- [ ] Core dump to SD/FTP + live exception-decoder panel (M); zero-overhead abstract logging (M).
- [x] Runtime heap/stack guardrails _(shipped)_ - `DETWS_ENABLE_GUARDRAILS`: `services/guardrails` samples free heap, the heap low-water mark, the largest free block (fragmentation), and a task's remaining stack, and fires a breach callback when any crosses its `DETWS_GUARDRAIL_*` floor - a proactive fail-safe hook on top of the passive /metrics numbers; evaluator + JSON host-tested, served at `/health` (example 69.Guardrails).
- [ ] Fail-safe safe-state + deadlock-detection WDT + watchdog-protected coroutine lifelines (M).
- [ ] Hardware health (M): power-rail voltage-drop logger, SPI-bus CRC audit + clock backoff, GPIO short-circuit test, capacitor-leakage diag.

## Build / tooling

- [x] Runtime build-flag reporter _(shipped)_ - `server.diag()` / `DETWS_ENABLE_DIAG` serves a build-info JSON (example 42.Diagnostics); the feature enumeration could be extended.
- [ ] Hierarchical build-flag tree (M); virtual protocol-mocking toggles (M).
- [ ] **Pentesting / adversarial suite** (M-L) - a separately-runnable harness (its own runner / CI job, _not_ part of the per-commit unit-test run) that stress-tests live server configurations across the enabled listeners (HTTP / WS / TLS / SSH / SNMP / CoAP / Modbus): malformed, oversized, and slowloris-style requests; header/body and protocol-confusion fuzzing; auth brute-force plus CSRF / IP-allowlist / lockout bypass attempts; and resource exhaustion against the connection / pool / scratch-arena limits. Asserts the server stays within its fixed footprint, never heap-allocates after `begin()`, and fails closed. Run on demand / nightly against a flashed board or the native build.
