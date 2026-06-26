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

1. **Quick wins (mostly already there):** Cache-Control beside ETag (done),
   runtime build-flag endpoint (done, `server.diag()`), MAC-derived UUID (done);
   remaining: raw-UDP telemetry helper (S).
2. **Security hardening cluster:** IP allowlist + brute-force lockout + CSRF
   tokens - all shipped.
3. **Partition-map monitor** (\*requested) + config export / restore - rounds out
   the new NVS config store.
4. **Dashboard configurator** - done: telemetry math, the SVG dashboard over SSE,
   and the WebSocket controls + Canvas chart.
5. **Architectural (deliberate):** Ethernet PHY + failover, GraphQL, OPC UA.

## Web / API / UI

- [x] WebSocket permessage-deflate inbound _(shipped)_; outbound compress (Phase 2) open (M).
- [x] REST substrate, AJAX _(shipped)_.
- [x] Real-time **dashboard** _(shipped, phase 1)_ - `DETWS_ENABLE_DASHBOARD`: a compile-time `DetwsWidget` table served as hand-rolled SVG (gauge / value / bar / sparkline, no external JS) via the `web_assets` pipeline, live over SSE; `detws_dashboard_set` / `_publish` (example 62.Dashboard). **Flagship.**
- [x] Dashboard phase 2 _(shipped)_ - WebSocket controls (button / toggle / slider widgets send values to a `detws_dashboard_on_control` callback) and a Canvas chart widget for dense series.
- [x] **Telemetry math** cluster _(shipped)_ - `services/telemetry`: moving-window stats (mean / variance / stddev / min / max), a rate-of-change tracker, and a trapezoidal run-time totalizer (example 61.Telemetry).
- [x] HTTP caching: `Cache-Control` beside ETag _(shipped)_ - `set_cache_control()` injects it into serve_file / serve_static responses.
- [ ] HTTP delivery (S-M): stale-while-revalidate, service-worker cache injection, delta/offset log fetching.
- [ ] Binary streaming (M-L): CBOR / MessagePack, or Protobuf / FlatBuffers zero-copy instead of JSON.
- [ ] GraphQL bounded subset (L); feature-dependent schema generation (M).
- [ ] Browser diag tools (M): GPIO pin mapper, ping/tracert panel, web logic analyzer.
- [ ] SPA micro-routing + conditional UI streaming (M); local SCADA/HMI fallback (M).
- [ ] WS MTU-aligned chunking / fragmentation control (M).

## Protocols & integrations

- [ ] OPC UA server/client (L); Node-RED integration (M).
- [ ] Southbound protocol-driver framework (L; Modbus is one today); Modbus register scanner/auto-discovery (M); atomic register matrix (M).
- [ ] Webhooks + IFTTT (M); Email + SMS fallbacks (SMTP + gateway) (M).
- [ ] ESP-NOW transport + ESP-NOW<->MQTT bridge (M).

## Networking / connectivity

- [ ] Ethernet PHY abstraction (L, greenlit) + network failover / multi-interface bridge + graceful interface escalation (L).
- [ ] IPv6 dual-stack + fallback (L); VPN tunneling + reverse-SSH tunnel to a relay (L).
- [ ] WiFi (M): sniffer / traffic analyzer / RF diag, channel-agility roaming.
- [ ] DNS (M): async resolver + response verification, captive-portal DNS-spoof mitigation, captive-portal auto-teardown timer.
- [x] mDNS TXT / `_https._tcp` / extra services _(shipped)_ - `detws_mdns_txt` / `detws_mdns_add_service`.
- [ ] mDNS adaptive / auto-sleep beacons + a continuous refresher for crowded RF (M).
- [ ] Static-IP fallback automation, dynamic socket recycling, TCP window auto-scaling by free RAM (M); raw-UDP telemetry cast (S).

## Power & radio management

- [ ] Radio power (S): disable BT coexistence (`esp_coex_preference_set`) + radio sleep, as build options.
- [ ] Dynamic network sleep modes / sleep-cycle scheduler (M); dynamic power scaling, thermal throttling, brownout recovery, peripheral power gating (M).

## Security & auth

- [x] Source-IP allowlist / firewall in the accept callback _(shipped)_ - `listener_ip_allow_add` / `listener_ip_allowed` (CIDR rules, `DETWS_ENABLE_IP_ALLOWLIST`; example 58.IpAllowlist).
- [x] Brute-force per-IP exponential lockout _(shipped)_ - `DETWS_ENABLE_AUTH_LOCKOUT`; `auth_lockout_*` table issues 429 + Retry-After on the HTTP auth gate (example 59.AuthLockout).
- [x] CSRF token verification _(shipped)_ - `DETWS_ENABLE_CSRF`; global enforcement on POST/PUT/PATCH/DELETE via a stateless HMAC-signed `X-CSRF-Token` (built-in `GET /csrf` issues it; example 60.Csrf).
- [ ] MFA hooks -> external API (S); granular API-token scoping (JWT/bearer) (M; JWT shipped).
- [ ] Enterprise identity handshakes - SAML / OAuth2 / OIDC (L).
- [ ] Secure boot + flash encryption (S, docs/eFuse); encrypted config handshake during onboarding (M).
- [x] MAC-derived UUID _(shipped)_ - `detws_uuid_from_mac` / `detws_device_uuid` (RFC 4122 v5; example 57.DeviceUuid).

## Storage & config

- [x] Typed NVS config store _(shipped)_.
- [ ] \*Partition-map status monitor endpoint (M, requested) - `esp_partition` / `esp_ota_get_*` usage breakdown for app/OTA/FS.
- [ ] Unified virtual-filesystem wrapper (M); bulk config templates + full export/restore (M); ZTP + graceful multi-stage provisioning (M).
- [ ] Wear leveling + log offload (server/SD) (M); hot-swap storage safeties (M).
- [ ] OTA: modular partition swapping + rollback protection + soft-brick safeguards (M).
- [ ] PSRAM web buffers / zero-copy net buffers (`heap_caps_calloc(MALLOC_CAP_SPIRAM)` at begin) + asset offloading + COMPONENT_EMBED_TXTFILES (M); SPI DMA ping-pong buffers (M).

## Observability, diagnostics & reliability

- [ ] Immutable audit logs + store-and-forward + FDA 21 CFR Part 11 (L).
- [ ] Core dump to SD/FTP + live exception-decoder panel (M); zero-overhead abstract logging (M); log rotation/pruning + traps (S).
- [ ] Heap high-water tracker (in /metrics already) + runtime stack/heap guardrails (S-M).
- [ ] Fail-safe safe-state + deadlock-detection WDT + watchdog-protected coroutine lifelines (M).
- [ ] Hardware health (M): power-rail voltage-drop logger, SPI-bus CRC audit + clock backoff, GPIO short-circuit test, capacitor-leakage diag.

## Build / tooling

- [x] Runtime build-flag reporter _(shipped)_ - `server.diag()` / `DETWS_ENABLE_DIAG` serves a build-info JSON (example 42.Diagnostics); the feature enumeration could be extended.
- [ ] Hierarchical build-flag tree (M); virtual protocol-mocking toggles (M).
- [ ] **Pentesting / adversarial suite** (M-L) - a separately-runnable harness (its own runner / CI job, _not_ part of the per-commit unit-test run) that stress-tests live server configurations across the enabled listeners (HTTP / WS / TLS / SSH / SNMP / CoAP / Modbus): malformed, oversized, and slowloris-style requests; header/body and protocol-confusion fuzzing; auth brute-force plus CSRF / IP-allowlist / lockout bypass attempts; and resource exhaustion against the connection / pool / scratch-arena limits. Asserts the server stays within its fixed footprint, never heap-allocates after `begin()`, and fails closed. Run on demand / nightly against a flashed board or the native build.
