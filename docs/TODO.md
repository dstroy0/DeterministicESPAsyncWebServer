# TODO / Known Fixes

Outstanding work and known limitations, roughly highest-impact first. Items are
grouped by area; each names the file(s) involved so the fix is easy to locate.

> **Status:** Security/correctness, the ESP32 build blocker, SSH
> `UNIMPLEMENTED`, housekeeping, the DX feature set (`serve_static` + MIME + gzip,
> [`redirect()`](@ref DetWebServer::redirect), named [`begin()`](@ref DetWebServer::begin) codes), the **HW-crypto performance** items
> (streaming SHA-256 + AES-CTR whole-buffer, **verified on a DevKitV1**), and the
> **optional services** (mDNS, OTA, WiFi provisioning/captive portal, SNTP, ETag,
> runtime stats, access-log hook) are all **done** - host-tested where possible
> and ESP32-firmware-linked. Items marked `[x]` carry a _(done)_ note.
>
> **Since v2.0.0** (all opt-in, default off, host-tested where possible +
> HW-verified): HTTPS/**TLS** server, **mTLS**, and **TLS session resumption**
> (RFC 5077 tickets); outbound **MQTT** (3.1.1) and **WebSocket** clients, each
> also over TLS; **SNMP** traps/informs (v2c + v3); **CoAP** resource Observe
> (RFC 7641) and block-wise transfer (RFC 7959); a **per-IP accept throttle**;
> **WebDAV** (RFC 4918); and a **Modbus TCP** slave. Plus an architecture pass
> (pluggable protocol-handler dispatch, flow-control primitives) and the
> `src/web` asset-generator pipeline.
>
> Optional services use only the base SDK + lwIP + mbedTLS (no add-on Arduino
> libraries): mDNS via the ESP-IDF `mdns` component, the captive-portal DNS via a
> raw lwIP UDP socket. Each is gated by a `DETWS_ENABLE_*` flag (default off).
>
> **Still deferred (YAGNI / large):** IPv6 dual-stack and an Ethernet PHY
> abstraction (the two architectural tracks); concurrent TLS connections
> (`MAX_TLS_CONNS` > 1 needs a smaller-record ESP-IDF build); SSH
> multiplexing, per-direction
> NEWKEYS, and the KDF `K1‖K2…` extension (no current use case); moving
> `ssh_pkt_recv`'s ~2 KB scratch off the stack. Full runtime verification of the
> WiFi-dependent services (mDNS resolve, NTP sync, OTA upload, portal join) needs
> WiFi credentials / a phone.

## From the working thread

Ideas and intentions captured from the [Working Thread discussion](https://github.com/dstroy0/DeterministicESPAsyncWebServer/discussions/15) (the maintainer's thought-stream). Unpolished on purpose; refine as they are picked up.

### Refactor / security hardening

- [ ] Owner-refactor endgame: remove the remaining globals now that the core is integrated, and switch to known security patterns for owners. The mutable global pointers that supported a speedy implementation are no longer necessary.
- [ ] Remediate SonarQube code smells and CodeQL gripes.

### Test coverage

- [ ] Keep extending test coverage. Provably-dead lines get `GCOVR_EXCL` with rationale inline; be conservative with exclusions (even a pointless test teaches) - guarded dead branches / unreachable guards are the main exclusion targets.
- [ ] Raise condition (branch) coverage - line coverage is good, conditional lags. Test both sides of each condition.

### Codecs / drivers / features

- [ ] Write more niche pure codecs from old manuals - the ones wanted a while ago but never put down on paper.
- [ ] W5500 SPI Ethernet driver, to support non-RMII ESP32s like the S3.
- [ ] Radio-as-a-plugin: strip an ESP to bare metal and do (very legal) radio things as a server plugin.
- [ ] `Industrial_ESPIDF/`: create the directory and write the CMake for it.

### Embedded data stores (SD card)

An on-device data-store stack. The user is attaching an **SD card** and wants an atomic buffer-to-flash
layer built first, then the store codecs on top. Substrate before stores.

- [x] **Atomic buffer-to-flash layer (the substrate).** _(done)_ A power-loss-safe write path over the
      FS / SD: a write-ahead log with a CRC per record and an A/B superblock commit marker, so an
      interrupted write is discarded (never half-applied) on the next mount. Zero-heap, bounded, static
      buffers. `DETWS_ENABLE_WAL` = the pure record codec (`services/wal/wal.h`) + the durable store
      (`wal_store.h`: A/B superblock + `wal_store_checkpoint` + mount/tail-replay over a `WalDev`
      block-device seam) + the `fs::FS` binding (`wal_fs.h`, preallocated file, random-access over
      `File::seek`/`flush`). Host-tested (13 cases: CRC vector, torn/truncated-tail recovery, checkpoint
      durability, superblock fallback) **and hardware-verified on an SD card over SPI** (checkpoint
      recovery, torn-tail drop, byte-level payload persistence, and survival across a chip reset all
      pass). _Follow-up:_ a ring-wrap (currently a linear log; wrapping needs a per-record epoch to
      disambiguate stale records); a WAL write-path throughput line in Performance.
- [ ] **dbm**: a classic on-disk hash key-value store (ndbm / gdbm-style) - a bounded, host-testable
      file-format codec on top of the flash layer.
- [ ] **sqlite**: SQLite3 **on-disk file-format** access (the documented page / b-tree / record
      encoding) - read first, bounded writer later. Not the full SQLite amalgamation (heap + stdio,
      incompatible with the no-stdlib zero-heap model).
- [ ] **nosql**: a NoSQL store - target TBD (a Redis RESP or MongoDB OP_MSG/BSON **wire client**, or a
      local on-flash document / KV store). Scope with the user before building.

### Performance

- [ ] **Feature performance measurement -> `docs/FEATURE_PERFORMANCE.md`.** Benchmark each feature's hot
      operation(s) to judge real-world viability: a host **ns/op** deterministic baseline plus the
      on-device **ESP32-S3 us/op @ 240 MHz** and throughput (the number that actually matters). A
      reusable bench harness (a shared timing header; a native runner on the RPi + an on-device example
      that prints a table over serial). Living table: feature, operation, host ns/op, ESP32 us/op,
      throughput, notes. Measure whole paths (HTTP parse, TLS handshake, JSON render), not just
      micro-codecs.

### CNC / machine-tool connectivity

Two link layers reach a CNC controller: legacy **RS-232** (drip-feed / DNC) and modern **Ethernet** (vendor APIs, file transfer, open telemetry). The read/telemetry side already ships as `services/mtconnect` (an ANSI/MTC1.4 agent over the existing HTTP stack); the rest below is open.

- [ ] RS-232 DNC codec: G-code (RS-274 / ISO 6983) line framing to drip-feed a program to a controller, with software flow control (XON/XOFF = DC1 `0x11` / DC3 `0x13`), `%` program start/end markers, and EIA RS-244 vs ISO 7-bit tape handling. Transport-agnostic so the same framing rides RS-232 or a socket.
- [ ] Ethernet DNC: stream the same G-code framing over a plain TCP socket (network drip-feed) for controllers that expose a raw program port.
- [ ] Fanuc FOCAS: client codec for the Fanuc Open CNC API (FOCAS1/2 over Ethernet) - read position/status/alarms and program up/download. Proprietary wire format; reverse from manuals plus a real controller.
- [ ] FTP client: many controllers (Fanuc, Haas, Mazak, Heidenhain) expose program storage over FTP - a small RFC 959 client (control + passive data channel) to push/pull `.nc` files.
- [ ] SMB/CIFS client: Windows-share program storage is the other common file path - a minimal SMB2 client (negotiate / session-setup / tree-connect / create / read / write) to read and write programs on a share.
- [x] MTConnect follow-ups _(done)_: the `probe` (device model) document - `detws_mtc_devices_begin/add_item/end` build an MTConnectDevices doc (a `<Device>` with its `<DataItems>`, optional `name`/`units`); the `asset` document - `detws_mtc_assets_begin` + `detws_mtc_assets_cutting_tool_begin`/`_tool_life`/`_cutting_tool_end` + `detws_mtc_assets_end` build an MTConnectAssets doc (a `<CuttingTool>` with its `<CuttingToolLifeCycle>`/`<ToolLife>`, optional `serialNumber`/`toolId`/`deviceUuid`/`timestamp`/`limit`); and the streaming `sample` sequence cursor - `DetwsMtcSampleBuffer` (a fixed ring, `detws_mtc_sample_buffer_init`/`_add`) with `detws_mtc_sample_query` replaying a from/count window as an MTConnectStreams document whose header carries firstSequence/lastSequence/nextSequence (MTC1.4 §6.7, oldest evicted + firstSequence advances when full). All tested in `test_mtconnect` (12 cases). The MTConnect agent's read documents (current/sample/probe/asset) are now complete.

### Routing / forwarding / inspection

Building on the existing forwarder (`native_forward` / `native_gateway` / `native_southbound`) toward the v5 "interface forwarding" milestone.

- [ ] Route-by-tag to interface: let a rule tag a flow (by source, destination, port, protocol, or a match expression) and bind that tag to an egress interface, so tagged traffic leaves a chosen NIC / radio - policy routing layered on the forwarder.
- [ ] Port forwarding: DNAT-style forward of an inbound port to an internal `host:port` (and the return path), so the server can publish a service that lives behind it.
- [ ] Optional packet inspection: an opt-in inspection hook on the forwarding path (parse / observe / filter before forward) for logging, metrics, or drop rules. Off by default (cost + privacy); a build-time + runtime toggle.

### Pentesting

- [ ] Extend the pentesting suite to cover more cases. Get creative; try to break the server.

### Docs

- [ ] Chart how the docs are scraped and built; keep refining and implementing automation in the gaps to increase maintainability and catch outliers.
- [ ] Full docs audit (not creative grep + diff): close the Grand-Canyon-sized gaps and the stale sections.
- [ ] Docs theming: the color palette is close but a little off, and there are many alignment issues. Refine "squirty's house" (bubbles, rocks, sponge conceptually right) and his behavior.
- [ ] Add a switch to the sandboxed docs so teachers can turn it off completely.
- [ ] Let users preview the (silly) themes on the live docs. Maybe.

## Feature parity with ESPAsyncWebServer (ESP32Async)

Conceptual features [ESP32Async/ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer)
offers that this library does not yet, ordered by value-vs-fit within the
zero-heap / fixed-buffer model. Implement top-down, one at a time, each with
native Unity tests before moving on. Each must keep the "no heap after
`begin()`" guarantee (fixed-size buffers, compile-time caps).

<details>
<summary><b>Expand feature-parity items</b></summary>

- [x] **1. Custom response headers + cookies (high / easy).** _(done)_
      Per-connection `_extra_hdr[MAX_CONNS][EXTRA_HDR_BUF_SIZE]` buffer injected
      into [`send()`](@ref DetWebServer::send) /
      [`send_empty()`](@ref DetWebServer::send_empty) /
      [`redirect()`](@ref DetWebServer::redirect), the same way the CORS block is
      injected. New API: [`add_response_header()`](@ref DetWebServer::add_response_header),
      [`set_cookie()`](@ref DetWebServer::set_cookie),
      [`clear_response_headers()`](@ref DetWebServer::clear_response_headers).
      Oversized headers are dropped whole; the buffer is cleared at the start of
      each dispatch. Tested by `test_response_headers` (9 cases).

- [x] **2. Request-data convenience (high / easy).** _(done)_ By-name request
      header lookup already existed via [`http_get_header()`](@ref http_get_header);
      added [`http_get_form()`](@ref http_get_form) for urlencoded POST body
      fields (parsed on demand into a caller buffer, gated on the
      `application/x-www-form-urlencoded` Content-Type, raw values to match
      `http_get_query()`). Tested by `test_form_params` (5 cases).

- [x] **3. Path parameters in routing (high / medium).** _(done)_ `/users/:id`
      style capture segments stored in a fixed `HttpReq::path_params[MAX_PATH_PARAMS]`
      array, exposed via [`http_get_param()`](@ref http_get_param). Routes are
      flagged `is_param` at registration; `match_path_params()` does a
      segment-by-segment match (literal segments exact, `:name` segments
      captured) alongside the existing exact + trailing-`*` matcher. Tested by
      `test_path_params` (8 cases).

- [x] **4. Digest authentication (medium / medium).** _(done)_ HTTP Digest
      (RFC 7616, SHA-256, `qop=auth`) via the existing `ssh_sha256`, selected by
      the new `digest` flag on
      [`on(..., realm, user, pass, digest=true)`](@ref DetWebServer::on).
      Server nonce regenerated per `begin()`; challenge emitted by
      `send_unauth()`; verified by `check_digest_auth()`. The parser now
      captures the full `Authorization` value into a dedicated
      `HttpReq::authorization[DIGEST_AUTH_HDR_MAX]` buffer (a Digest header far
      exceeds `MAX_VAL_LEN`). Tested by `test_digest_auth` (5 cases: challenge,
      valid handshake, wrong password, forged nonce, 128-bit hex nonce) and
      independently grounded against `openssl`/FIPS vectors by `test_digest_vectors`
      (4 cases). The nonce is now seeded from the hardware CSPRNG
      (`esp_random()`) folded through SHA-256 with a counter + `millis()`, and
      regenerated per `begin()`.
      _Follow-up:_ `nc` (nonce-count) replay tracking is still not implemented:
      it needs per-client state, which conflicts with the single shared server
      nonce and this device class's 1–2 client model (global `nc` tracking would
      reject legitimate concurrent clients). The per-`begin()` nonce rotation
      bounds the replay window in the meantime.

- [x] **5. Response templating (medium / medium).** _(done)_ `{{name}}`
      substitution via [`send_template()`](@ref DetWebServer::send_template) with
      a `TemplateVar` resolver callback. The body is walked twice (size, then
      write) so it is never buffered whole - constant memory regardless of body
      size. Unterminated/over-long placeholders are emitted literally; HEAD
      sends headers only. Tested by `test_template` (6 cases).
      _Follow-up:_ apply the same resolver path to static-file serving.

- [x] **6. Middleware pipeline (high value / large).** _(done)_ Fixed-size,
      composable global middleware chain (cap `MAX_MIDDLEWARE`, default 4) run in
      registration order before route matching via
      [`use()`](@ref DetWebServer::use). A [`Middleware`](@ref Middleware) returns
      [`MW_NEXT`](@ref MwResult) to fall through or [`MW_HALT`](@ref MwResult) to
      short-circuit (after sending its own response); middlewares can also inject
      response headers / log every request (incl. unmatched 404s). Added a
      built-in fixed-window rate limiter,
      [`enable_rate_limit(max, window_ms)`](@ref DetWebServer::enable_rate_limit),
      that answers over-budget requests with `429` + `Retry-After` before the
      chain (cheapest rejection under flood); rollover-safe, per-server state, no
      per-IP table. Tested by `test_middleware` (9 cases).
      _Design note:_ CORS + Basic/Digest auth were **left as-is** (tested/green)
      rather than re-expressed as middlewares - the chain is additive and
      composes alongside them; "logging middleware" is the existing
      [`on_request_log()`](@ref DetWebServer::on_request_log) hook plus any
      user `use()` middleware. _Follow-up:_ per-route middleware attachment (a
      middleware can already gate on `req->path` in user code).

- [x] **7. Chunked / streaming app responses (medium / medium).** _(done; pull
      generator since v4.5.0)_
      [`send_chunked(slot, code, type, source, ctx)`](@ref DetWebServer::send_chunked)
      writes the status + headers (`Transfer-Encoding: chunked`, plus CORS /
      queued custom headers), then pulls the body from a
      [`ChunkSource`](@ref ChunkSource) generator one piece at a time, frames each
      as an RFC 7230 §4.1 chunk, and emits the terminating `0\r\n\r\n`. The body is
      never buffered whole AND the send paces with the TCP window, paging across
      worker loops (`chunk_send_pump`, resumed by the sent callback) - so output is
      unbounded in constant memory and a body past the send window is never
      truncated (the old one-shot `ChunkedResponse` writer silently truncated
      there). The source returns 0 to end and tracks its position in `ctx` (which
      must outlive the response). HEAD sends headers only;
      [`on_request_log()`](@ref DetWebServer::on_request_log) reports the total body
      length. Tested by `test_chunked` (10 cases, incl. a 16 KB body).
      _Follow-up (done):_ `chunk_send_pump` sizes each chunk to `det_conn_sndbuf()`
      (reserving the frame overhead), flushes and resumes on the next loop when the
      window is full, and clamps a misbehaving source to the window - the same
      per-loop send-window backpressure `file_send_pump()` uses.

- [x] **8. Stretch / lower priority.** _(resolved: the sub-items are shipped, except multi-MCU
      portability which is a closed won't-do per the user's standing request.)_
  - [x] Regex routes _(done)_: [`on_regex()`](@ref DetWebServer::on_regex):
        whole-path match via a bounded, allocation-free backtracker (`.`, `* + ?`,
        `[...]`/`[^...]` ranges, `\d \w \s`, `\` escapes; non-capturing). A
        `RE_MAX_STEPS` budget keeps it deterministic (fails closed). Tested by
        `test_regex` (9 cases); example `15.RegexRoutes`.
  - [x] Static JSON request/response helper _(done)_: zero-heap `JsonWriter`
        (formats into a caller buffer, auto comma/escape, `JSON_MAX_DEPTH` cap)
        plus `json_get_str()`/`json_get_int()`/`json_get_bool()` top-level object
        readers (`src/network_drivers/presentation/json.*`). ArduinoJson stays optional (it heap-allocates).
        Tested by `test_json` (17 cases); example `10.Json`.
  - [x] Interface filters _(done)_: per-route STA/AP gate via
        [`on(..., DetIface)`](@ref DetWebServer::on) + [`set_ap_ip()`](@ref DetWebServer::set_ap_ip).
        Each connection is tagged `DETIFACE_STA`/`DETIFACE_AP` at accept time by
        comparing its local IP to the softAP IP. Tested by `test_iface` (7 cases);
        example `09.InterfaceFilter`.
  - [x] Portability beyond ESP32 (ESP8266 / RP2040 / RP2350). **Won't do (closed):** deferred at the
        user's explicit request and confirmed not pursued - a deliberate scope decision, not an open gap.
        The library targets the ESP32 family (Arduino core + ESP-IDF); porting the WiFi/BLE radio,
        lwIP/FreeRTOS bindings, and the mbedTLS/MPI hardware-acceleration seams to the ESP8266 (no
        FreeRTOS SMP, far less RAM) or the RP2040/RP2350 (no on-chip WiFi/BLE) is out of scope. Reopen only
        if the ESP32-only assumption ever needs to change.

</details>

## Round 2 - post-v2.0.0 subsystems

<details>
<summary><b>Expand Round 2 items</b></summary>

All opt-in (`DETWS_ENABLE_*`, default off), host-tested where a pure codec exists
and HW-verified on an ESP32 DevKit. Per-feature footprints are in the README.

- [x] **Architecture pass.** Pluggable per-protocol handler dispatch
      (`network_drivers/session/proto_handler.h` - a `ProtoHandler` table, so a new
      TCP protocol registers a handler instead of editing the dispatchers),
      flow-control primitives ([`det_conn_send`](@ref det_conn_send) returns bool,
      `det_conn_sndbuf`, context-safe `det_conn_raw_send`), response header+body
      write coalescing, and a TLS-BIO unification that fixed a latent handshake
      cross-thread race.
- [x] **Client TLS hardening** (extends [`DETWS_ENABLE_HTTP_CLIENT_TLS`](@ref DETWS_ENABLE_HTTP_CLIENT_TLS)).
      Optional CA-chain + hostname verification and SHA-256 cert pinning for
      outbound TLS; encrypt-only by default. `native_http_client`; HW-verified.
- [x] **MQTT 3.1.1 client** ([`DETWS_ENABLE_MQTT`](@ref DETWS_ENABLE_MQTT)) + MQTTS.
      Full QoS 0/1/2 (DUP retransmit + inbound QoS-2 duplicate suppression),
      Last-Will, keepalive.
      Host-tested codec (`native_mqtt`); example `46.MqttClient`.
- [x] **WebSocket client** ([`DETWS_ENABLE_WS_CLIENT`](@ref DETWS_ENABLE_WS_CLIENT))
      + `wss://`. Masked frames, fragment reassembly, ping/pong. Host-tested codec
      (`native_ws_client`); example `47.WebSocketClient`.
- [x] **SNMP notifications** ([`DETWS_ENABLE_SNMP_TRAP`](@ref DETWS_ENABLE_SNMP_TRAP)).
      Outbound Traps + InformRequests (v2c) and SNMPv3 USM authPriv traps. Host
      -tested PDU builder (`native_snmp_trap`); example `48.SnmpTrap`.
- [x] **CoAP server** ([`DETWS_ENABLE_COAP`](@ref DETWS_ENABLE_COAP), RFC 7252) with
      resource **Observe** (RFC 7641, [`DETWS_ENABLE_COAP_OBSERVE`](@ref DETWS_ENABLE_COAP_OBSERVE))
      and **block-wise transfer** (RFC 7959, [`DETWS_ENABLE_COAP_BLOCK`](@ref DETWS_ENABLE_COAP_BLOCK)).
      Host-tested core (`native_coap`); examples `49.CoapObserve`, `50.CoapBlock`.
- [x] **Per-IP accept throttle** ([`DETWS_ENABLE_PER_IP_THROTTLE`](@ref DETWS_ENABLE_PER_IP_THROTTLE)).
      Closes the cross-connection flood gap left by the global throttle. Example
      `51.PerIpThrottle`.
- [x] **WebDAV** ([`DETWS_ENABLE_WEBDAV`](@ref DETWS_ENABLE_WEBDAV), RFC 4918 class 1
      + advisory locks): OPTIONS/PROPFIND/GET/HEAD/PUT/DELETE/MKCOL/COPY/MOVE/LOCK/
      UNLOCK over the FS. Host-tested 207 builder (`native_webdav`); example
      `52.WebDav`.
- [x] **Modbus TCP slave** ([`DETWS_ENABLE_MODBUS`](@ref DETWS_ENABLE_MODBUS)). Fixed
      data model + MBAP/PDU codec, FC 1/2/3/4/5/6/15/16, via a `PROTO_MODBUS`
      handler. Host-tested (`native_modbus`); example `53.ModbusTcp`.
- [x] **TLS session resumption** (see the TLS item) and the **web-asset generator**
      (`src/web/input` -> `web_assets.{h,cpp}` via `build_assets.py`; `/metrics` and
      `/stats` are editable `{{name}}` templates).

Open follow-ups discovered during the above:

- [x] **WebDAV: collection `COPY`** _(done, HW-verified)_ - recursive collection copy
      (RFC 4918 9.8) via `dav_copy_recursive` (bounded depth 8): honors `Depth: 0`
      (collection only) vs `infinity`/absent (full tree), and `Overwrite` (clears the
      target first, 204 vs 201, `Overwrite: F` -> 412). HW-tested on LittleFS (nested
      subcollection + files copied byte-exact) and host-tested: `test/mocks/FS.h` gained
      an opt-in directory tree (`mock_fs_tree_enable()`), and the new
      `native_webdav_handler` env (`test_webdav_handler`) drives the real handler through
      recursive COPY / MOVE / DELETE against it.
      _(PROPPATCH done: 207 with each property refused 403. Streaming PUT done: the
      body is written to the file as it arrives, no longer bounded by
      [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE).)_
- [x] **Client-side TLS resumption** _(done, ESP32 compile-verified)_ - the persistent
      client session (`csess`, e.g. MQTTS/WSS) now enables client session tickets
      ([`DETWS_ENABLE_TLS_RESUMPTION`](@ref DETWS_ENABLE_TLS_RESUMPTION)), saves the
      established session with `mbedtls_ssl_get_session()` after each successful
      handshake, and presents it with `mbedtls_ssl_set_session()` on the next
      `det_tls_csess_begin()` for an abbreviated handshake;
      `det_tls_csess_forget_session()` forces a fresh full handshake. Compiles on the
      ESP32 toolchain. _Full abbreviated-handshake HW proof is blocked by the same
      stock-Arduino DRAM limit as concurrent TLS (the ~48 KB `DETWS_TLS_ARENA_SIZE`
      plus MQTT + transport overflows DRAM; needs a smaller-record ESP-IDF build)._
- [x] **SNMPv3 _inform_** _(done)_ - `snmp_inform_v3()` (symmetric with
      `snmp_inform_v2c()` / `snmp_trap_v3()`) builds + sends an authenticated (authPriv
      when a privacy password is set) USM `InformRequest`; the caller owns the
      `request_id` the receiver echoes in its Response and retransmits for confirmed
      delivery. Host-tested via a new opt-in UDP capture seam (`test_snmp_v3`
      `test_inform_v3_builds_informrequest`: a v3 message carrying the InformRequest PDU
      + request-id).
- [~] **CoAP server scope** - `/.well-known/core` resource discovery (RFC 6690) is
      now served: GET returns the registered resources in Link Format
      (`application/link-format`, CF 40), paged with Block2 if large; non-GET -> 4.05.
      Host-tested (`test_coap` `test_well_known_core_discovery` /
      `_rejects_post`) and HW-verified against `aiocoap` (interop `coap` peer).
      Still out of scope (add only if needed): separate (deferred) responses, and CON
      retransmission + message de-duplication - the model stays piggybacked-only
      (CON -> piggybacked ACK, NON -> NON).
- [~] **Concurrent TLS** (`MAX_TLS_CONNS` > 1). _(library side landed; a `MAX_TLS_CONNS=2`
      PSRAM build was HW-verified to link, boot, and run on an ESP32-S3 - the only unproven
      piece is a live 2-clients-at-once soak, blocked by the lab network, not the code.)_ The whole mbedTLS
      working set is served from one static
      `.bss` arena, and the real internal ceiling is the ESP32 `dram0_0_seg` region
      (~122 KB, ROM-reserved both ends - NOT the 320 KB PlatformIO prints), so a 2nd
      connection overflows the link (measured: `overflowed by 34048 bytes` at an 88 KB
      arena). Three library-side paths now exist + a build guard that turns the cryptic
      linker error into a clear message ([`DETWS_TLS_ACK_MULTI_CONN_DRAM`](@ref DETWS_TLS_ACK_MULTI_CONN_DRAM)):
      (1) [`DETWS_TLS_ARENA_IN_PSRAM`](@ref DETWS_TLS_ARENA_IN_PSRAM) places the arena in
      external RAM via `EXT_RAM_BSS_ATTR` (needs `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY`;
      the stock precompiled arduino-esp32 2.0.x has it off, so a PSRAM/IDF build is
      required - now HW-verified end to end on an ESP32-S3 N16R8 with a rebuilt flag-enabled
      core: the arena lands at `0x3C0xxxxx` external RAM, the board boots octal with no
      watchdog loop, internal DRAM use drops sharply, and it stays zero-heap; the rebuild
      recipe is `tools/psram/README.md`);
      (2) [`DETWS_TLS_MAX_FRAG_LEN`](@ref DETWS_TLS_MAX_FRAG_LEN) (RFC 6066 MFL, applied to
      server + client) caps records, pairing with a custom-IDF `CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN`
      shrink; (3) a `memory.ld` DRAM reclaim (advanced - the `0xdb5c` is ROM-reserved,
      so risky). Full 3-prong decision tree in docs/KNOWN_LIMITATIONS.md. **HW-verified on an
      ESP32-S3 N16R8** (arduino-esp32 3.x, PSRAM arena): `MAX_TLS_CONNS=2` with a 128 KB arena
      links using only **56 KB internal DRAM** (17%; the same build with the arena in DRAM uses
      187 KB), boots octal PSRAM, and `begin_tls()` starts the HTTPS listener with a flat heap and
      8.2 MB PSRAM free - the arena is static `.ext_ram.bss`, zero heap. Note the S3's data segment
      is far larger than the classic ESP32's ~122 KB, so on the S3 both connections fit in DRAM too;
      PSRAM's win there is headroom, and it is _required_ only on the tight classic-ESP32 segment.
      This run also uncovered + fixed a core-locking crash in the listener bring-up on arduino-esp32
      3.x (see docs/BUGS.md). **Remaining:** a live 2-clients-at-once handshake soak; not run here
      because the test host cannot reach the device (AP client isolation on the lab WiFi + no admin
      to add a route - the same topology the interop harness works around with a device-out broker),
      not a code limitation.
- [~] **Ethernet PHY abstraction** - _(bring-up shipped)_ `DETWS_ENABLE_ETHERNET`:
      `init_eth_physical()` / `eth_ready()` in `network_drivers/physical` wrap the Arduino
      ETH library for an RMII PHY (LAN8720 / ...), configured by the standard `ETH_PHY_*`
      build flags. The egress reporting + per-route interface classifier already handle a
      wired route (DETIFACE_ETH, host-tested), so the server serves over Ethernet - or
      dual-homed with Wi-Fi - once the link has an IP. Example 19.Ethernet; ESP32-compiled.
      Remaining: verify against a PHY board.
- [~] **IPv6 dual-stack** - _phase 1 landed (v4.83.0); phase 2 landed (v4.89.0)._ `DETWS_ENABLE_IPV6`
      enables IPv6 on the netif (`init_ipv6_physical` / `net_global_ipv6` / `ipv6_ready`); the
      listeners already bind `IPADDR_TYPE_ANY`, so the server accepts v6 once an address is up. The
      `DetIp` address core (`network_drivers/network/ip.h`) parses / formats / classifies both
      families (`native_det_ip`; RFC 4291 + 5952). Example 20.IPv6; both cores compiled. **Phase 2
      (done):** the transport carries the peer as a protocol-agnostic family-tagged `DetIp`
      (`det_conn_remote_addr()` / `det_lwip_to_detip()`), and every IP-keyed abuse-prevention feature
      stores and matches the FULL address - the per-IP throttle + auth lockout by
      [`det_ip_equal`](@ref det_ip_equal), the IP allowlist by
      [`det_ip_prefix_match`](@ref det_ip_prefix_match) (v4 /0-32 + v6 /0-128 CIDR via
      [`listener_ip_allow_add_cidr`](@ref listener_ip_allow_add_cidr)). This replaced the interim v6
      32-bit hash key, which was collidable (see docs/BUGS.md); no abuse-prevention state is keyed on
      a hash or a uint32 flattening any more (the audit log has no client-IP field). **Remaining:**
      HW-verify SLAAC on a real v6 network.
- [x] **Shared scratch-buffer pool (decided: build before permessage-deflate).** _(done)_
      Several features carry their own fixed _transient_ scratch (SSH `crypto_work`
      and the ~2 KB `ssh_pkt_recv` stack buffer, header formatting, the upcoming
      deflate window). These are mutually exclusive in time, so one shared arena
      cuts peak DRAM. **Model - region-reset-per-dispatch:** one compile-time-sized
      BSS arena (`DETWS_SCRATCH_ARENA_SIZE`); `scratch_alloc(n, align)`
      bump-allocates; the arena is reset to empty at the top of every event
      dispatch in `server_tick()`, before the protocol handler runs.
      **Race-safety (verified):** all codec/protocol logic runs only in the single
      loop task (`server_tick` / `handle`); the lwIP callbacks (tcpip_thread, maybe
      a different core) only fill the rx ring + enqueue events and never touch
      scratch - so the arena has exactly one accessor and needs no lock. Add a debug
      owner-task assert (`xTaskGetCurrentTaskHandle`) that fails loud if any foreign
      context ever borrows. **Exhaustion-safety:** borrows live only within one
      dispatch and are auto-reclaimed at the reset, so leaks (creeping exhaustion)
      are impossible; an over-budget `scratch_alloc` returns nullptr and every
      caller has a defined fail-closed path (WS close 1011, 503, or skip the
      optimization) - never UB, never block. Sizing = worst-case concurrent borrows
      in any single dispatch. This generalizes the existing single-loop-confined
      `crypto_work` pattern (only one SSH KEX runs at a time).
      _Status (done):_ arena core + LIFO mark/release + RAII `ScratchScope` landed
      (`test_scratch`; exhaustion + no-accumulate verified); the single-owner debug
      assert (`assert_single_owner` / `xTaskGetCurrentTaskHandle`, per worker) guards
      against a foreign-task borrow; `scratch_reset()` wired into `server_tick()`.
      Tenants migrated: `ssh_pkt_recv` (its ~2 KB stack buffer removed), `ssh_conn`,
      the OIDC verifier's ~2.6 KB decode buffers, and - the planned final tenant - the
      **permessage-deflate window** (both the outbound `deflate_raw` and inbound
      `inflate_raw` scratch in `websocket.cpp` are `scratch_alloc`'d, fail-closed on
      exhaustion). Host tests green and esp32dev links.

</details>

## Roadmap & known limitations

Forward-looking feature ideas and the future-work backlog have moved to their own
files so this one stays focused on bugfixes, maintenance, and the record of
shipped work:

- **Future features / backlog:** [ROADMAP.md](ROADMAP.md)
- **Deliberate constraints / caveats:** [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md)

## Build / toolchain

<details>
<summary><b>Expand Build / toolchain items</b></summary>

- [x] **`esp32dev` build failed on the official platform (mbedtls v2).** _(done)_
      `ssh_rsa.cpp`'s ARDUINO path now compiles on **both** mbedtls v2 (official
      `espressif32`, Arduino core 2.0.x) and v3 (core 3.x) via
      `MBEDTLS_VERSION_MAJOR` guards around `mbedtls_rsa_init`, `mbedtls_pk_sign`
      (with an `esp_fill_random`-backed `f_rng`), and `mbedtls_rsa_pkcs1_verify`.
      Two further fixes: (1) a missing `intelhex` Python module broke
      `bootloader.bin` (installed into the PlatformIO Python); (2) **latent bug** -
      the ARDUINO [`ssh_rsa_sign()`](@ref ssh_rsa_sign) passed the raw exchange hash `H` to
      `mbedtls_pk_sign()`, which does not re-hash, so it signed `DigestInfo||H`
      instead of `DigestInfo||SHA256(H)` (RFC 8332) and any client would reject
      the host signature; now hashes `H` first to match the native path.
      Verified: `pio run -e esp32dev` compiles all `src/` (incl. SSH) and a full
      firmware links (`pio ci examples/01.Basic --board esp32dev`: RAM 18.4%,
      Flash 56.3%). `platformio.ini` pins `espressif32 @ ^6.0.0` for
      reproducibility.

</details>

## Security / correctness (high priority)

<details>
<summary><b>Expand Security / correctness (high priority) items</b></summary>

- [x] **Native RSA signing is a `d=1` test stub, not a real signature.** _(done)_
      `ssh_rsa_sign()` native path now performs a full-width `s = em^d mod n`
      via `bn_modexp_full()` (square-and-multiply over every bit of d, reusing
      the correct `bn_mul_full` / `bn_reduce_full` helpers). Validated by
      `test_rsa_sign_verify_roundtrip` with a real 2048-bit private exponent.
      Still software / not constant-time (test-only path; ESP32/mbedTLS is real) - covered by the constant-time item below. CRT was deliberately skipped
      (YAGNI: the native path is test-only, speed is adequate).

- [x] **No authentication attempt limiting (brute-force).** _(done)_
      SSH now bounds failed `USERAUTH_REQUEST`s per connection: the dispatcher
      (`ssh_server.cpp`) counts [`SSH_MSG_USERAUTH_FAILURE`](@ref SSH_MSG_USERAUTH_FAILURE) responses in
      `SshSession.auth_failures` and, after [`SSH_MAX_AUTH_ATTEMPTS`](@ref SSH_MAX_AUTH_ATTEMPTS)
      (`ServerConfig.h`, default 6), emits [`SSH_MSG_DISCONNECT`](@ref SSH_MSG_DISCONNECT)
      (reason 14) and closes (RFC 4252 §4). The publickey probe (PK_OK) and a
      SUCCESS do not count. Tested by `test_auth_bruteforce_disconnect` /
      `test_auth_success_after_failures`.
      HTTP Basic needs no separate per-connection counter: `send_unauth()`
      already sends `Connection: close` and tears down the socket on every 401,
      so a client gets exactly one guess per TCP connection. Cross-connection
      (per-IP) throttling is the connection-flood item below.

- [x] **Software crypto paths are not constant-time.** _(done - asserted out of
      firmware)_ The native Montgomery cluster (`ssh_bignum.cpp`: `bn_init`,
      `bn_monpro`, `bn_shl1`, `bn_sub_inplace`, `g14_R1/R2`) is now under
      `#ifndef ARDUINO`, so it is not compiled into firmware at all; the software
      AES (`ssh_aes256ctr.cpp`) and native RSA modexp (`ssh_rsa.cpp`,
      `bn_reduce_full`/`bn_modexp_*`) already live in the `#else` of an
      `#ifdef ARDUINO`. On ESP32 only the HW/mbedTLS paths compile and run.
      Hardening the software paths to constant-time was deliberately skipped
      (YAGNI: they are host-test-only and now provably absent from firmware).
      Documented in `SECURITY.md` (⚠️ timing row).

- [x] **No connection-flood / rate limiting.** _(done - opt-in global throttle)_
      `listener.cpp` now has a fixed-window accept-rate gate
      ([`listener_accept_allowed()`](@ref listener_accept_allowed)): when [`DETWS_ENABLE_ACCEPT_THROTTLE`](@ref DETWS_ENABLE_ACCEPT_THROTTLE) is set,
      the accept callback drops connections beyond
      [`DETWS_ACCEPT_THROTTLE_MAX`](@ref DETWS_ACCEPT_THROTTLE_MAX) per [`DETWS_ACCEPT_THROTTLE_WINDOW_MS`](@ref DETWS_ACCEPT_THROTTLE_WINDOW_MS)
      (`ServerConfig.h`) before claiming a pool slot. Default off (zero
      cost / no behavior change). Two static counters, global across listeners -
      a per-IP table was deliberately not added (YAGNI; the mock PCB carries no
      remote IP and a 1-3 connection device gains little from per-IP state).
      Rollover-safe; tested by `test_accept_throttle*\*`in`test_transport`.
      A per-IP accept throttle was **since added** (round 2,
      [`DETWS_ENABLE_PER_IP_THROTTLE`](@ref DETWS_ENABLE_PER_IP_THROTTLE)): a fixed
      BSS bucket table keyed by source IPv4 with a per-address fixed window,
      host-tested in `test_transport`.

- [x] **[`base64_decode()`](@ref base64_decode) has no output-capacity guard (Basic-auth ingestion).**
      _(done)_ `base64_decode()` now takes a `dst_cap` parameter
      (`base64.cpp`/`.h`, both platforms) and bounds every write; an over-capacity
      decode returns 0 instead of overrunning. `check_basic_auth()`
      (`dwserver.cpp`) passes `sizeof(decoded) - 1`, leaving
      room for the null terminator regardless of how [`MAX_VAL_LEN`](@ref MAX_VAL_LEN)/[`MAX_AUTH_LEN`](@ref MAX_AUTH_LEN)
      are set. Tested by `test_base64_decode_respects_capacity`; all callers
      (WS handshake tests) updated to the new signature.
      Note: this was the only unguarded ingestion path - the HTTP parser
      (indexed bounds + `body[BODY_BUF_SIZE+1]`), multipart (bounded boundary copy
      over a null-terminated body), SSH `read_string()` (capacity-checked), the SSH
      banner ([`SSH_VERSION_MAX`](@ref SSH_VERSION_MAX) + explicit lengths), and the WS handshake
      (`strnlen(client_key, WS_MAX_KEY_LEN+1)`) are all correctly bounded.

- [x] **Connection close/abort is driven from L7 holding the raw `tcp_pcb`.** _(done)_
      The transport now owns the whole teardown: `det_conn_close(slot)` (graceful, was
      `det_conn_close(slot, pcb)`) and the new `det_conn_abort_slot(slot)` (hard RST)
      each detach the pcb, free the per-connection TLS context, reset the slot, and
      then FIN/RST - on a captured pcb pointer, so a late lwIP callback finds a freed
      slot. Every hand-rolled teardown now passes only the slot: the WS/SSE close +
      upgrade-fail sites in `dwserver.cpp`, `session.cpp`
      `tls_abort`, and the SSH (x2) / telnet / modbus / opcua drop paths. This also
      fixed a latent pcb leak (the WS/SSE upgrade-alloc-fail paths detached but never
      aborted). Host-tested (`test_observability`: local-close frees the slot,
      abort-slot counts + frees, abort-slot no-ops on a free slot), full native suite
      green, and **HW-soaked on COM3**: HTTP close-path + WS churn (12 `abort_slot`
      RSTs on WS-pool exhaustion) reclaim every slot with no leak and the device keeps
      accepting throughout.

- [x] **`detws_oidc_verify_with_key()` decode buffers moved off the stack.** _(done)_
      The verifier's `hdr[512]` + `sig[DETWS_OIDC_RSA_BYTES]` + `pl[DETWS_OIDC_MAX_LEN]`
      + `iss[256]` (~2.6 KB) are now borrowed from the per-dispatch scratch arena under a
      `ScratchScope` (fail-closed if the arena is exhausted), not stacked. This is
      single-worker-race-safe (the arena has one accessor) where a `static` buffer would
      race concurrent workers. HW-soaked on COM3: a real RS256 verify returns OK with
      `scratch_high_water == 2624` (exactly the four buffers, now in BSS) and the verify
      compiles + runs under ARDUINO (mbedTLS RSA). `native_oidc` links
      `session/scratch.cpp` + `worker.cpp`; 13/13 OIDC tests still pass.
      _Follow-up:_ the HW soak showed the verify still consumes ~7 KB of **stack** during
      the call - that residual is the **mbedTLS RSA-2048 modexp** itself, not the decode
      buffers. A worker task that runs OIDC verification must be sized for it (or the
      verify marshaled onto a larger-stack task); tracked as the stack-budget item below.

- [x] **RSA-2048 modexp uses ~7 KB of stack (mbedTLS).** _(done - enforced minimum)_
      Measured on COM3: an OIDC RS256 verify (and any `ssh_rsa_verify` path: OIDC, the
      SSH server host key, JWKS) drops the task stack high-water by ~7 KB, dominated by
      the mbedTLS bignum modexp. The "documented minimum worker-stack" option is now
      **enforced at build time**: [`DETWS_WORKER_STACK_RSA_MIN`](@ref DETWS_WORKER_STACK_RSA_MIN)
      (default 8192, `ServerConfig.h`) is the floor, and a validation `#error`
      fires when `DETWS_ENABLE_OIDC` or `DETWS_ENABLE_SSH` is set while
      [`DETWS_WORKER_TASK_STACK`](@ref DETWS_WORKER_TASK_STACK) is below it - so a lowered
      worker stack is caught at compile time instead of overflowing on the first verify.
      An advanced build that marshals every RSA verify onto a dedicated larger-stack task
      (the worker itself never runs one) can override `DETWS_WORKER_STACK_RSA_MIN`. The
      other two options (lower `MBEDTLS_MPI_MAX_SIZE`, or the dedicated task) remain
      available but are not required for the default architecture. Verified: the default
      config and OIDC-on-with-8192 compile; OIDC-on-with-4096 fails with the guard message.

</details>

## SSH protocol completeness (medium)

<details>
<summary><b>Expand SSH protocol completeness (medium) items</b></summary>

- [x] **[`SSH_MSG_UNIMPLEMENTED`](@ref SSH_MSG_UNIMPLEMENTED) not sent for unknown messages.** _(done)_ The
      dispatcher's default case (`ssh_server.cpp`) now emits
      `SSH_MSG_UNIMPLEMENTED` with the rejected packet's sequence number
      (`ssh_pkt[i].seq_no_recv - 1`, since `ssh_pkt_recv` has already advanced the
      counter) per RFC 4253 §11.4 - no handler-signature change needed. Tested by
      `test_unimplemented_reply_for_unknown_message`.

- [~] **SSH channel multiplexing + port-forwarding.** _(channels + direct-tcpip
      `ssh -L` done; forwarded-tcpip + X11 pending)_
      `ssh_channel.cpp` is a per-connection channel table (`DETWS_SSH_MAX_CHANNELS`,
      default 1 = the original single channel): up to N concurrent channels per
      connection, each with its own id / window / peer state, every inbound
      `CHANNEL_*` routed to its channel by the recipient id, and the data callback /
      `ssh_conn_send` tagged with the channel id. **direct-tcpip** (`ssh -L`) channels
      now parse + route through a normalized forwarding seam: a channel carries a
      `type` (session / direct-tcpip), `CHANNEL_OPEN` "direct-tcpip" extracts the
      target host:port and consults `ssh_channel_set_forward_open_cb` (opt-in; absent
      = administratively prohibited, refused = connect-failed, accepted = confirmed),
      and forward-channel data routes to `ssh_channel_set_forward_data_cb` instead of
      the session callback. Host-tested (`test_ssh_channel`: independent routing,
      pool-full -> resource shortage, unknown-type, forward open accept/refuse,
      forward-data routing). The **forward owner** (`ssh_forward`, behind
      `DETWS_SSH_PORT_FORWARD`) does the actual outbound TCP via the `det_client`
      transport and bridges bytes both ways - no I/O in the codec - with an optional
      target policy, a per-poll target->client pump bounded by the channel window,
      and EOF+CLOSE propagation; `ssh_conn_close_channel` sends a server-initiated
      close as two packets. ESP32 build + link verified.
      **Global requests** (RFC 4254 §4) now have a real handler
      ([`ssh_global_request_handle`](@ref ssh_global_request_handle)): an unrecognized
      request answers `REQUEST_FAILURE` when `want_reply` is set (never `UNIMPLEMENTED` -
      that was a client-keepalive interop bug, see docs/BUGS.md), and **`tcpip-forward` /
      `cancel-tcpip-forward`** (`ssh -R`) route to an opt-in remote-forward seam
      (`ssh_channel_set_rforward_open_cb` / `_cancel_cb`) that replies `REQUEST_SUCCESS`
      (echoing the allocated port for a port-0 bind, §7.1) when an owner accepts. Host-tested
      (`test_ssh_channel`, +7). **`forwarded-tcpip` (`ssh -R`) now fully works**: the owner
      (`ssh_forward`, behind `DETWS_SSH_PORT_FORWARD`) allocates a real listener via the new
      **tcpip_thread-marshaled** `listener_add_dynamic` / `listener_stop_dynamic` (a dynamic
      listener created from the SSH worker task must marshal its raw lwIP `tcp_bind`/`tcp_listen`
      onto tcpip_thread), routes each accepted connection through a `PROTO_SSH_RFWD` handler that
      opens a server-initiated `forwarded-tcpip` channel (`ssh_channel_open_forwarded` +
      CONFIRMATION/FAILURE handling) and bridges bytes both ways with per-poll window-bounded
      pumps; listeners + bridges are torn down on cancel and on SSH disconnect. Host-tested
      (`test_ssh_channel`: open/confirm/failure/inbound-routing) and **HW-verified end-to-end on
      an ESP32-S3** (`ssh -R 8080:localhost:9000` -> a connection to the device's :8080 tunnels
      to the client's :9000 and back, both directions + close propagation). _Remaining:_
      **X11 forwarding** (and a bind-port policy hook for remote forwards, currently any free port).

- [x] **Per-direction NEWKEYS.** _(done)_ The single `ssh_pkt[i].encrypted` flag is
      split into `enc_out` (outbound) and `enc_in` (inbound), tracked independently per
      RFC 4253 sec 7.3. `ssh_newkeys_sent()` turns on the outbound cipher/MAC (+ the s2c
      compression stream) the moment the server emits its NEWKEYS; `ssh_newkeys_complete()`
      turns on the inbound cipher/MAC when the peer's NEWKEYS arrives. The send path (pack)
      reads `enc_out`, the receive path (unpack) reads `enc_in`, so a strict peer that
      activates its send direction before we activate ours is handled. Wire-equivalent on
      the happy path; the 145 `native_ssh` cases (incl. the full KEXINIT->NEWKEYS->SERVICE
      handshake with real crypto + rekey) pass, and it compiles for the ESP32 target.

- [x] **Key-derivation extension (RFC 4253 §7.2).** _(done)_ `ssh_kdf_derive()`
      produces any length up to `SSH_KDF_MAX` (4 blocks) via the `K1‖K2‖…` chain
      (Ki+1 = HASH(mpint(K) ‖ H ‖ K1..Ki)); `derive_key()` is now the 32-byte wrapper,
      so the existing KEX is byte-identical (all negotiated algorithms still use one
      block). Host-tested: `test_ssh_crypto` `test_ssh_kdf_extension_chain` verifies K1
      equals the single-block derive and K2 chains correctly.

- [x] **Session rekeying (RFC 4253 §9).** _(done)_ A server-initiated re-key now fires from
      `ssh_conn_poll()` when either the volume budget (`SSH_REKEY_PACKET_THRESHOLD`, a packet-count
      proxy for ~1 GB) or the time budget (`SSH_REKEY_TIME_MS`, default 1 h) since the last KEX is
      spent, on an authenticated channel that is not already re-keying: it emits a fresh KEXINIT via the
      existing `ssh_transport_begin_rekey()`, and the KEXINIT dispatch carries it to completion (session
      + auth preserved). The decision is a pure, host-tested helper `ssh_rekey_due()`
      (`test_rekey_due_volume_and_time`), the timer resets in `ssh_newkeys_complete()` off the pluggable
      clock, and the sequence-number-wrap close remains the last-resort fallback. So a long-lived /
      high-throughput session re-keys in place instead of being dropped.

- [~] **Compression (RFC 4253 §6.2).** _(server->client done; client->server deferred)_
      `DETWS_ENABLE_SSH_ZLIB` adds `zlib@openssh.com` / `zlib` for the **s2c** direction: a
      context-takeover DEFLATE stream (`ssh_zlib`, persistent sliding window + per-packet
      sync-flush + zlib wrapper) owned by `ssh_comp`, negotiated per direction, started after
      USERAUTH_SUCCESS (delayed) or NEWKEYS (plain). HW-verified against OpenSSH 10.3 (42 KB
      byte-perfect). **c2s stays `none`:** OpenSSH compresses outbound with `Z_PARTIAL_FLUSH`,
      whose deflate blocks straddle packet byte-boundaries, so decompressing it needs a
      resumable inflate state machine (bit-state + mid-block decode carried across packets) -
      a much larger engine for a direction (keystrokes / uploads to the device) that barely
      benefits. Add the resumable inflate if a bulk c2s (SFTP-to-device) use case appears.

</details>

## Performance / hardware acceleration (medium)

<details>
<summary><b>Expand Performance / hardware acceleration (medium) items</b></summary>

- [x] **SSH per-packet HMAC ran in software SHA-256 on ESP32.** _(done; on-device
      verification pending a board connection)_ The streaming SHA-256 context is
      now backed by `mbedtls_sha256_context` on Arduino
      (`mbedtls_sha256_starts/update/finish`, v2/v3-guarded), so the HW SHA engine
      accelerates per-packet HMAC **and** KEX hashing. The software FIPS-180-4 path
      is now compiled only on native (`#ifndef ARDUINO`). The `ssh_hmac_sha256.cpp`
      HW-acceleration comment is now accurate. Native software KATs still pass;
      `examples/35.SSHCryptoSelfTest` validates the HW path on-device.

- [x] **AES-256-CTR re-acquired the HW engine once per 16-byte block.** _(done)_
      The Arduino [`ssh_aes256ctr_crypt()`](@ref ssh_aes256ctr_crypt) now makes a single
      `mbedtls_aes_crypt_ctr()` call for the whole buffer (our `counter` /
      `keystream` / `pos` fields map 1:1 to mbedtls's `nonce_counter` /
      `stream_block` / `nc_off`), replacing the per-block
      `mbedtls_aes_crypt_ecb()` loop. Native software path unchanged. Validated by
      the native AES-CTR KATs and `examples/35.SSHCryptoSelfTest` on-device.

- [x] **DMA UART / I2C / SPI transfer (v5 milestone).** DONE - `services/dma`
      (`DETWS_ENABLE_DMA`). Channels move peripheral bytes to a static ping-pong (RX) /
      staging (TX) buffer; a DMA-complete event carries the bytes to a callback that
      posts them into the preempting task queue. Zero heap, fail-closed. The
      ingress/egress **simulator** (`DETWS_DMA_SIMULATE`, default on) exercises the whole
      pipeline with no physical loopback - on the host bench and on-device; a real silicon
      driver plugs into the `det_dma_hw_*` hooks. Host-tested (`native_dma`, 11 cases) +
      HW-verified (example `07.DmaIngest`; and a combined webserver + continuous-DMA rig
      ingested 2.2M+ frames with zero integrity errors under HTTP stress, no heap growth).
      Remaining: the real UHCI-UART / `spi_master`-DMA silicon backend (needs peripheral
      hardware to verify; the seam is in place).

- [x] **User-configurable preempting task queue (v5 milestone).** DONE -
      `services/preempt_queue` (`DETWS_ENABLE_PREEMPT_QUEUE`). Producers post from a task
      (`xQueueSendToBack` / `-Front`, wait timeout) or an ISR (`xQueueSendFromISR` +
      `portYIELD_FROM_ISR`); the scheduler preempts to the processing task immediately.
      Task priority + core are user-settable at `detws_pq_start[_lane]()`, depth is
      compile-time. **Named lanes**: one USER lane exposed to the app (no-arg `detws_pq_*`)
      plus internal DMA / FORWARD / DEVICE lanes that run above it (DMA highest, below
      tcpip / WiFi), so internal ingest preempts user work. Host-tested
      (`native_preempt_queue`, 11 cases) + HW-verified (DMA + USER lanes ran continuously
      with zero errors under an HTTP flood; examples 06.PreemptQueue + 08.PreemptLanes).

- [x] **Interface forwarding (v5 milestone), DMA-driven.** DONE - `services/forward`
      (`DETWS_ENABLE_FORWARD`). A forwarding plane: register interfaces (each with an
      egress send callback), add per-pair rules (allow / deny + rate cap); a frame on one
      interface (`det_forward_ingress()`, wired from a DMA-complete event on the FORWARD
      lane) is forwarded to every allowed destination, so the device bridges / routes
      instead of only terminating traffic. Default-deny, never reflects to the source,
      fail-closed (exceeded cap / refused send drops and is counted), multi-destination
      fan-out. Zero-heap static tables. Host-tested (`native_forward`, 10 cases) +
      HW-verified (600k+ frames ingested over DMA and forwarded to a second interface with
      zero loss / zero integrity errors under an HTTP flood; example 09.InterfaceForward).
      This is the generic data path the post-v5 wireless gateway bridges sit on top of.

- [~] **Post-v5 southbound bridges + sensing (backlog).** The **generic gateway
      framework is DONE** - `services/gateway` (`DETWS_ENABLE_GATEWAY`): ports,
      address-aware northbound enveloping + topic, bidirectional up/down-link, per-port
      rate cap, stats; fail-closed, zero-heap, HW-verified end to end over DMA + the
      FORWARD lane (example 10.RadioGateway). The per-module **codec + driver** plugins
      on top of it are now nearly all shipped: RF / wireless **gateway bridges** (LoRa,
      nRF24, CC1101, Thread over SPI; Zigbee, Z-Wave, EnOcean, Sigfox over UART; NFC over
      I2C/SPI/UART; BLE ATT/GATT) - all codec-shipped + host-tested; **promiscuous /
      monitor capture** (Wi-Fi raw 802.11, CAN bus listen-only, radio channel sniff to
      802.15.4-TAP pcap) shipped; and **field-perturbation sensing** (LD2410 mmWave radar,
      FDC2214 capacitive, LDC1614 inductive, VL53L0X ToF) shipped. _Remaining:_ Wi-SUN FAN
      (blocked on a devboard choice), analog Doppler + MR60BHA 60 GHz radar variants, and
      the real-module HW verification of each. See
      [ROADMAP.md](ROADMAP.md#post-v5-rf--wireless-gateway-bridges).

</details>

## HTTP / core (medium)

<details>
<summary><b>Expand HTTP / core (medium) items</b></summary>

- [x] **TLS (HTTPS).** _(done)_ Opt-in mbedTLS server on a fixed static arena
      ([`DETWS_ENABLE_TLS`](@ref DETWS_ENABLE_TLS)) - see the HTTPS / TLS item under
      Optional services, plus mTLS, `wss://` / TLS-SSE, and RFC 5077 session
      resumption.

- [x] **`Date` response header** _(done, opt-in)_ - [`DETWS_HTTP_EMIT_DATE`](@ref DETWS_HTTP_EMIT_DATE)
      (default off, so the hot path is unchanged unless enabled) auto-injects
      `Date: <IMF-fixdate>` into every dynamic response once a wall-clock time exists
      ([`detws_ntp_http_date()`](@ref detws_ntp_http_date) non-empty); a clock-less / pre-sync device omits it
      (RFC 7231 §7.1.1.2). Host-tested via a time-injection seam
      (`test_response_headers`: emitted-when-set / omitted-when-clockless) and HW-verified
      with NTP (`Date: Mon, 29 Jun 2026 ... GMT`). Apps can still add it from a handler.

- [x] **Recv scratch off the stack.** _(done)_ `ssh_pkt_recv`'s per-packet
      plaintext buffer (`SSH_PKT_BUF_SIZE + SSH_HMAC_SHA256_LEN`, ~2 KB) moved from
      the stack into the shared per-dispatch scratch arena
      (`network_drivers/session/scratch.*`), borrowed under an RAII `ScratchScope`
      so it is reclaimed on every exit path and reused (not accumulated) across
      packets in one call. See the shared scratch-pool item under Round 2.

</details>

## Optional services / features (toggleable, default off)

<details>
<summary><b>Expand Optional services / features (toggleable, default off) items</b></summary>

Capabilities a small IoT web server commonly needs but the library does not yet
provide. Each should follow the existing feature-flag convention - a
`DETWS_ENABLE_*` macro defaulting to 0, gating its own `.cpp`/pool so it costs
no code, RAM, or flash when disabled (`ServerConfig.h`). Roughly ordered
by how often a deployed device needs it.

- [x] **mDNS / DNS-SD advertisement ([`DETWS_ENABLE_MDNS`](@ref DETWS_ENABLE_MDNS)).** _(done)_
      `detws_mdns_begin(hostname, port)` (`src/services/mdns_service.*`) makes the
      device reachable at `<hostname>.local` and advertises `_http._tcp`. Uses the
      **ESP-IDF `mdns` component directly** (not the `ESPmDNS` add-on) to keep the
      dependency set to base-SDK + mbedTLS. Firmware links (`examples/37.mDNS`).

- [x] **OTA firmware update ([`DETWS_ENABLE_OTA`](@ref DETWS_ENABLE_OTA)).** _(done)_ Authenticated
      streaming `POST /update` into the ESP32 `Update` API
      (`src/services/ota_service.*`). The HTTP parser gained a `#if DETWS_ENABLE_OTA`
      streaming-body hook (`http_parser_set_stream_hooks`) that feeds the image to
      `Update.write()` in [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE) chunks instead of buffering it - so the
      `BODY_BUF_SIZE`/413 cap is bypassed and multi-MB images never live in RAM.
      The matching route handler replies + reboots. Parser hook native-tested
      (`test_http_ota`, env `native_ota`) with **no regression** to the 80 parser
      tests (fully gated); `examples/38.OTA` firmware links.

- [x] **WiFi provisioning / captive portal ([`DETWS_ENABLE_PROVISIONING`](@ref DETWS_ENABLE_PROVISIONING)).** _(done)_
      `src/services/provisioning_service.*`: first-boot softAP + a catch-all DNS
      responder + a credentials form, persisting SSID/PSK to NVS
      (`detws_provisioning_load`/`_begin`/`_clear`, `examples/39.Provisioning`).
      The DNS responder is a **raw lwIP UDP socket** (no `DNSServer` add-on) -
      callback-driven, so no per-loop polling. The form-field/URL-decode parser is
      native-tested (`test_provisioning`, env `native_prov`); firmware links.

- [x] **Pre-compressed static asset serving.** _(done)_ `serve_static()` serves
      `<path>.gz` with `Content-Encoding: gzip` when the client sends
      `Accept-Encoding: gzip` and the `.gz` exists (original Content-Type
      preserved). No separate flag needed - it is zero-cost when no `.gz` is
      present. Tested by `test_serve_static_gzip_when_accepted` /
      `test_serve_static_no_gzip_when_not_accepted`.

- [x] **Conditional GET / ETag ([`DETWS_ENABLE_ETAG`](@ref DETWS_ENABLE_ETAG)).** _(done)_ `serve_file()` /
      `serve_static()` emit a strong `ETag` (`"<hexsize>-<hexmtime>"` from
      `f.size()` + `f.getLastWrite()`) and answer a matching `If-None-Match` with
      `304 Not Modified` (no body). The FS test mock gained `getLastWrite()` so it
      is host-tested (`test_serve_static_etag_conditional_get`); ESP32 path
      compile-verified against the real `fs::File`. Now also emits a `Last-Modified`
      date and honors `If-Modified-Since` (per RFC 9110, only when no `If-None-Match`
      is present); with no wall clock the date validator is skipped and the ETag
      validator still works.

- [x] **SNTP time sync (`DETWS_ENABLE_NTP`).** _(done)_ [`detws_ntp_begin()`](@ref detws_ntp_begin)/
      `_synced()`/`_epoch()`/`_http_date()` (`src/services/ntp_service.*`) wrap
      `configTzTime` (ESP-IDF SNTP) and format an RFC 7231 `Date`. `examples/40.SNTP`
      exposes `GET /time`; firmware links. (Auto-emitting the `Date` response
      header is left to the app via the helper - kept off the hot path.)

- [x] **Multi-source time fallback ([`DETWS_ENABLE_TIME_SOURCE`](@ref DETWS_ENABLE_TIME_SOURCE)).**
      _(done)_ A zero-heap registry of user-defined time sources
      (`src/services/time_source/time_source.*`): each source is a callback
      returning Unix epoch seconds (0 = no valid time), registered with a priority.
      `detws_time_now()` queries them in ascending priority and returns the first
      valid result (stopping early so a costly lower-priority read is skipped), so
      the device falls back automatically (e.g. GPS fix lost -> RTC -> NTP);
      `detws_time_source_active()` reports which source answered. Host-tested
      (`native_time_source`, 9 cases) with mock sources; example
      `56.TimeSourceFallback` (NTP preferred, RTC fallback); esp32dev links.

- [x] **Zero-copy template slicing.** _(addressed by design)_
      [`send_template()`](@ref DetWebServer::send_template) never buffers the
      expanded body: it walks the template twice (size, then stream each literal
      run and resolved `{{name}}` value straight to the socket), and placeholder
      names over 32 chars are emitted literally - so there is no fixed expansion
      slot to overflow. _Follow-up:_ apply the same resolver path to static-file
      serving.

- [x] **JSON request/response helper.** _(done)_ Zero-heap `JsonWriter`
      (formats into a caller buffer with automatic comma/escaping and a
      `JSON_MAX_DEPTH` nesting cap; overflow flips `ok()` and truncates safely)
      plus top-level object readers `json_get_str()`/`json_get_int()`/
      `json_get_bool()` (`src/network_drivers/presentation/json.*`). ArduinoJson stays optional (it
      heap-allocates). Tested by `test_json` (28); example `10.Json`.
      _Follow-up (done):_ string unescaping now decodes `\uXXXX` to UTF-8 (1-4
      bytes) and joins UTF-16 surrogate pairs into astral code points; an unpaired
      surrogate becomes U+FFFD and malformed/short hex becomes `?`, with a clean
      truncation when a code point's UTF-8 sequence would not fit (`json.cpp`).

- [x] **Web "serial" terminal ([`DETWS_ENABLE_WEB_TERMINAL`](@ref DETWS_ENABLE_WEB_TERMINAL)).**
      _(done)_ A WebSerial-style browser terminal over the existing WebSocket
      layer (`src/services/web_terminal.*`): serves a self-contained CRT-themed
      page + a WebSocket endpoint, broadcasts device output to all browsers, and
      delivers typed lines to a command callback - all zero-heap. Tested by
      `test_web_terminal` (7); example `27.WebTerminal`.

- [x] **HTTPS / TLS ([`DETWS_ENABLE_TLS`](@ref DETWS_ENABLE_TLS)).** _(done)_
      Opt-in mbedTLS on a static memory pool (`src/network_drivers/tls/det_tls.*`):
      all mbedTLS allocations come from a fixed BSS arena
      (`DETWS_TLS_ARENA_SIZE`, default 48 KB) via
      `mbedtls_platform_set_calloc_free()`, so the zero-heap guarantee holds. HW
      CSPRNG RNG; BIO bridged to the raw `tcp_pcb` + rx ring; handshake pumped in
      the session loop. `begin_tls(port, cert, …)` / [`listen_tls()`](@ref DetWebServer::listen_tls).
      HW-verified: `ECDHE-ECDSA-AES256-GCM-SHA384`, TLS 1.2+. See SECURITY.md §6.
      Example `22.HTTPS`. `wss://` + TLS-SSE now run over the same record layer,
      and **session resumption** shipped (RFC 5077 tickets,
      [`DETWS_ENABLE_TLS_RESUMPTION`](@ref DETWS_ENABLE_TLS_RESUMPTION), example
      `54.TlsResumption`). _Still open:_ `MAX_TLS_CONNS` > 1 (needs smaller IDF
      record buffers) and client-side resumption.

- **SNMP agent v1 / v2c / v3.** Zero-heap ASN.1 BER codec + a fixed MIB
      (OID table) over a raw lwIP UDP socket, GET / GETNEXT / GETBULK / SET.
      Shared base (codec + PDU + MIB) is native-testable.
  - [x] BER codec (RFC indefinite-free definite-length TLV): INTEGER, OCTET
        STRING, NULL, OID (base-128), SEQUENCE, and the SNMP application types.
        `src/services/snmp/snmp_ber.*`, KAT-tested (`env:native_snmp`).
  - [x] v1/v2c agent (community-string access, RFC 1157 / 3416): GET / GETNEXT /
        GETBULK / SET dispatch over a fixed MIB-II-style table, per-varbind v2c
        exceptions (`noSuchObject`/`endOfMibView`) and v1 error-status/-index,
        SET gated by a separate read-write community. `snmp_agent_process()` is a
        pure, host-testable core (13 tests); the transport-layer UDP service
        (`det_udp_listen`) on :161 carries datagrams (the same service the
        provisioning DNS responder uses). `snmp_agent_*` API, example `33.SNMP`.
        **HW-verified** with a UDP client: `snmpget`/walk of the system group in
        OID order, GetBulk, dynamic Gauge32, SET authorization (RO→noAccess,
        RW→success), v1 `noSuchName`, and unknown-community drop all behave per
        net-snmp.
  - [x] v3 (USM, RFC 3414): gated behind `DETWS_ENABLE_SNMP_V3` (default off).
        Auth = `usmHMAC192SHA256` (HMAC-SHA-256, 24-byte; RFC 7860, reusing the
        SSH SHA-256/HMAC), privacy = `usmAesCfb128` (AES-128-CFB, RFC 3826 - a
        compact portable AES added in `snmp_crypto`). Implements the v3 message
        framing (msgGlobalData + msgSecurityParameters + scopedPDU), engine
        discovery (Report `usmStatsUnknownEngineIDs`), the timeliness window
        (engineBoots/engineTime; boots persists via `snmp_v3_set_boots()` from
        NVS), USM error Reports (unknownUserNames / wrongDigests /
        notInTimeWindows / decryptionErrors), and key localization (RFC 3414
        §2.6). `snmp_v3_*` API; `snmp_v3_process()` reuses the shared
        [`snmp_dispatch_pdu()`](@ref snmp_dispatch_pdu) MIB core. Native tests
        (`env:native_snmp_v3`): SHA-256 localization KAT (hashlib-grounded),
        AES-128 FIPS-197 KAT, and the full discovery -> authNoPriv -> authPriv
        flow. **HW-verified** against an independent manager (pycryptodome AES +
        Python hashlib/hmac): authNoPriv + authPriv GET/SET and the error Reports
        interoperate byte-for-byte over real UDP. Example `33.SNMP` (set the
        flag to enable the user). _Follow-up:_ derive the engine ID from the chip
        MAC; persist engineBoots across reboots.

- [x] **Telnet console ([`DETWS_ENABLE_TELNET`](@ref DETWS_ENABLE_TELNET)).**
      _(done)_ Minimal RFC 854 line-oriented Telnet server dispatched from the
      session layer's `PROTO_TELNET` arm
      (`src/network_drivers/presentation/telnet.*`): negotiates server echo +
      suppress-go-ahead (character mode), accumulates a line with backspace
      handling, hands each completed line to a command callback, and can push
      output to all connected clients. Plaintext - no auth or encryption, so use
      it only on a trusted LAN (prefer SSH or the WebSocket terminal otherwise).
      Example `36.Telnet`.

- [x] **JWT bearer auth ([`DETWS_ENABLE_JWT`](@ref DETWS_ENABLE_JWT)).** _(done)_
      Stateless `Authorization: Bearer <jwt>` verification, HS256
      (HMAC-SHA-256, reusing the SSH crypto layer), constant-time signature
      compare, all in fixed stack/BSS - no sessions, no heap
      (`src/services/jwt/*`). Host-tested (`native_jwt`); example `21.JWTAuth`.
      **Time claims now enforced (opt-in via the caller's clock):** the `*_at`
      variants ([`jwt_time_valid`](@ref jwt_time_valid),
      [`jwt_verify_hs256_at`](@ref jwt_verify_hs256_at),
      [`jwt_bearer_valid_at`](@ref jwt_bearer_valid_at)) reject on `exp` (RFC 7519
      §4.1.4) and `nbf` (§4.1.5) with a skew leeway, given `now = (long)detws_time_now()`
      (`DETWS_ENABLE_NTP` / any time source); passing `now = 0` on a clockless device
      skips the time check so the signature still gates. `iat` is informational
      (read via `jwt_claim_int`). The base signature-only functions are unchanged.
      _Out of scope:_ RS256/ES256 (asymmetric, allocation-heavy).

- [x] **Remote syslog ([`DETWS_ENABLE_SYSLOG`](@ref DETWS_ENABLE_SYSLOG)).**
      _(done)_ RFC 5424 log lines shipped as UDP datagrams via the transport UDP
      service (`src/services/syslog/*`): a pure host-testable `syslog_format()`
      builds one line into a caller buffer, an ESP32-only `syslog_log()` sends it.
      Host-tested (`native_syslog`); example `41.Syslog`.

- [x] **Streaming file upload ([`DETWS_ENABLE_UPLOAD`](@ref DETWS_ENABLE_UPLOAD)).**
      _(done)_ A `POST` route streams its body straight into a file on an Arduino
      FS (LittleFS/SPIFFS/SD) in `FILE_CHUNK_SIZE` pieces - the upload never has to
      fit in RAM (`src/services/upload_service.*`). Reuses the parser's
      streaming-body hook. Example `30.FileUpload`. _Constraint:_ only one streaming
      sink exists, so `DETWS_ENABLE_UPLOAD` and [`DETWS_ENABLE_OTA`](@ref DETWS_ENABLE_OTA)
      share it - enable at most one per build.

- [x] **WebSocket permessage-deflate - Phase 1 (inbound)**
      (`DETWS_ENABLE_WS_DEFLATE`, RFC 7692). _(done)_ The handshake negotiates
      `permessage-deflate` with `client_no_context_takeover; server_no_context_takeover`,
      so each message decompresses independently. A compressed message (RSV1 on its
      first frame) is INFLATEd before delivery by a hand-rolled bounded RFC 1951
      decompressor (`network_drivers/presentation/inflate.*`) whose Huffman tables
      are borrowed from the shared per-dispatch scratch arena - no per-connection
      buffer, and the output buffer doubles as the LZ77 window (no separate 32 KB
      window). Both the compressed input and the decompressed output must fit
      `WS_FRAME_SIZE`; a malformed stream closes 1002. Outbound frames stay
      uncompressed (§6 permits). Host-tested: `native_inflate` (12 cases, vectors
      grounded against zlib) + `native_ws_deflate` (handshake / RSV1 / delivery);
      esp32dev links; example `55.WebSocketCompression`. _HW test pending a board._
  - [x] **Phase 2 - outbound compress _(shipped)_.** A bounded fixed-Huffman DEFLATE
        encoder compresses outbound data frames (RSV1) under `DETWS_ENABLE_WS_DEFLATE`,
        with an uncompressed fallback when the result would not shrink. permessage-deflate
        is now bidirectional (see ROADMAP "WebSocket permessage-deflate, inbound and
        outbound"); host-tested via `native_deflate` + `native_ws_deflate`.

(Deliberately omitted as not worth the footprint for this class of device: none
currently. WebSocket permessage-deflate - previously omitted - now ships its
inbound half; see above.)

</details>

## Quality-of-life (developer / operator)

<details>
<summary><b>Expand Quality-of-life (developer / operator) items</b></summary>

Convenience that does not add protocol capability but removes friction. Newbie
items lower the floor for first-time users; operator items help whoever runs a
deployed device.

Newbie / developer experience:

- [x] **One-call static directory mount.** _(done)_
      `serve_static(url_prefix, fs, fs_root)` (`dwserver.h`)
      mounts a filesystem subtree at a URL prefix via a wildcard `ROUTE_STATIC`:
      `index.html` fallback for `/` or directory requests, MIME auto-detection,
      gzip-static, path-traversal rejection, GET/HEAD only (else 405). Tested by
      the `test_serve_static_*` suite.

- [x] **MIME type auto-detection by extension.** _(done)_ `DetWebServer::mime_type(path)` - a static, case-insensitive extension→type table (html/css/js/json/svg/
      png/jpg/gif/ico/webp/wasm/woff2/… → falls back to
      `application/octet-stream`). Used automatically by `serve_static()` and
      callable directly with `serve_file()`. Tested by `test_mime_type_detection`.

- [x] **Named `begin()` failure codes.** _(done)_ `begin()`/[`listen()`](@ref DetWebServer::listen)/[`restart()`](@ref DetWebServer::restart)
      now return a [`DetWebServerResult`](@ref DetWebServerResult) enum: [`DETWS_OK`](@ref DETWS_OK), [`DETWS_ERR_NO_LISTENERS`](@ref DETWS_ERR_NO_LISTENERS),
      [`DETWS_ERR_LISTENER_FULL`](@ref DETWS_ERR_LISTENER_FULL), [`DETWS_ERR_LISTEN_FAILED`](@ref DETWS_ERR_LISTEN_FAILED)
      (`dwserver.h`/`.cpp`). Subsumes the heap-bytes mismatch
      item below (docstring corrected).

- [x] **`redirect()` helper.** _(done)_ `server.redirect(slot_id, code, location)`
      sends a `Location` header + empty body and closes; accepts 301/302/303/307/308
      (any other code → 302). Tested by `test_redirect_*`.

Operator / sysadmin:

- [x] **Runtime stats endpoint ([`DETWS_ENABLE_STATS`](@ref DETWS_ENABLE_STATS)).** _(done)_ `server.stats(slot)`
      emits a JSON snapshot - uptime, total requests, 2xx/4xx/5xx counts, active
      connection-pool slots, and free heap. Counters are maintained centrally in
      `note_response()` (the single funnel through which `send`/`send_empty`/
      `redirect`/`serve_file` report each response). Tested by
      `test_stats_endpoint_emits_json`.

- [x] **Per-request log callback hook.** _(done)_ `server.on_request_log(cb)`
      (`RequestLogCb`) fires once per response with method/path/status/body-length,
      via the same `note_response()` funnel. Pure hook - one function pointer, no
      in-library buffering. Always compiled (no flag). Tested by
      `test_request_log_hook_fires`.

</details>

## Examples (low)

<details>
<summary><b>Expand Examples (low) items</b></summary>

- [x] **`begin()` heap-bytes contract mismatch.** _(done)_ The misleading
      "abs(result) == heap bytes needed" docstring/example was corrected; `begin()`
      now returns a `DetWebServerResult` code (see the named-failure-codes item
      above). The `heap_needed()`/`heap_available()` no-op shims were removed in
      v4.0.0 (the library makes no heap allocations).

</details>

## Housekeeping (low)

<details>
<summary><b>Expand Housekeeping (low) items</b></summary>

- [x] **Native `base64_decode()` accepts `=` outside the trailing pad.** _(done)_
      `b64_val()` no longer treats `=` as a value; the decoder validates padding
      positionally - full 4-char quads only, `=` permitted only as 1-2 trailing
      chars of the final quad (`base64.cpp`). Misplaced padding and non-multiple-
      of-4 input now return 0. Tested by `test_base64_decode_rejects_misplaced_padding`.

- [x] **`test/test_application/` is orphaned** _(done)_ - wired into the
      `native_app` env's `test_filter` (`platformio.ini`) and de-bit-rotted (it
      called the removed `DeterministicAsyncTCP::init(80)`; now [`pool_init()`](@ref DeterministicAsyncTCP::pool_init)).
      All 35 cases pass.

- [x] **`docs/CHANGELOG.md` upkeep.** _(done - automated)_ Generated and
      committed by the `changelog.yml` workflow (`chore: update CHANGELOG.md
      [skip ci]`), so it tracks each cycle without a manual pass.

- [x] **Add an SSH usage example** _(done)_ - `examples/34.SSH/34.SSH.ino`:
      enables SSH, loads the host key from NVS ([`ssh_rsa_load_pubkey()`](@ref ssh_rsa_load_pubkey)), installs
      password + publickey auth callbacks and a channel data callback that echoes
      via the new [`ssh_conn_send()`](@ref ssh_conn_send) helper, listens on [`PROTO_SSH`](@ref PROTO_SSH). Required a
      small public outbound API (`ssh_conn_send()`, `ssh_conn.*`) since the
      dispatcher's emit path was internal-only.

- [x] **Publish RSA host-key provisioning docs** _(done)_ - `docs/SSH.md`
      now has a "Host key provisioning" section: `openssl genrsa` →
      `pkcs8 -topk8 -outform DER`, embed + write to NVS (`ssh_host_key/priv_der`)
      with `Preferences`, and `ssh_rsa_load_pubkey()` at boot.

</details>
