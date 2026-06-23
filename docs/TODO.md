# TODO / Known Fixes

Outstanding work and known limitations, roughly highest-impact first. Items are
grouped by area; each names the file(s) involved so the fix is easy to locate.

> **Status:** Security/correctness, the ESP32 build blocker, SSH
> `UNIMPLEMENTED`, housekeeping, the DX feature set (`serve_static` + MIME + gzip,
> `redirect()`, named `begin()` codes), the **HW-crypto performance** items
> (streaming SHA-256 + AES-CTR whole-buffer, **verified on a DevKitV1**), and the
> **optional services** (mDNS, OTA, WiFi provisioning/captive portal, SNTP, ETag,
> runtime stats, access-log hook) are all **done** - host-tested where possible
> and ESP32-firmware-linked. Items marked `[x]` carry a _(done)_ note.
>
> Optional services use only the base SDK + lwIP + mbedTLS (no add-on Arduino
> libraries): mDNS via the ESP-IDF `mdns` component, the captive-portal DNS via a
> raw lwIP UDP socket. Each is gated by a `DETWS_ENABLE_*` flag (default off).
>
> **Still deferred (YAGNI / large):** TLS (HTTPS) server; SSH multiplexing,
> per-direction NEWKEYS, and the KDF `K1‖K2…` extension (no current use case);
> moving `ssh_pkt_recv`'s ~2 KB scratch off the stack. Full runtime verification
> of the WiFi-dependent services (mDNS resolve, NTP sync, OTA upload, portal
> join) needs WiFi credentials / a phone - only compile+link verified here.

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
      the ARDUINO `ssh_rsa_sign()` passed the raw exchange hash `H` to
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
      (`ssh_server.cpp`) counts `SSH_MSG_USERAUTH_FAILURE` responses in
      `SshSession.auth_failures` and, after `SSH_MAX_AUTH_ATTEMPTS`
      (`DetWebServerConfig.h`, default 6), emits `SSH_MSG_DISCONNECT`
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
      (`listener_accept_allowed()`): when `DETWS_ENABLE_ACCEPT_THROTTLE` is set,
      the accept callback drops connections beyond
      `DETWS_ACCEPT_THROTTLE_MAX` per `DETWS_ACCEPT_THROTTLE_WINDOW_MS`
      (`DetWebServerConfig.h`) before claiming a pool slot. Default off (zero
      cost / no behavior change). Two static counters, global across listeners -
      a per-IP table was deliberately not added (YAGNI; the mock PCB carries no
      remote IP and a 1-3 connection device gains little from per-IP state).
      Rollover-safe; tested by `test_accept_throttle_*` in `test_transport`.
      Finer-grained / per-IP throttling remains a network-layer concern.

- [x] **`base64_decode()` has no output-capacity guard (Basic-auth ingestion).**
      _(done)_ `base64_decode()` now takes a `dst_cap` parameter
      (`base64.cpp`/`.h`, both platforms) and bounds every write; an over-capacity
      decode returns 0 instead of overrunning. `check_basic_auth()`
      (`DeterministicESPAsyncWebServer.cpp`) passes `sizeof(decoded) - 1`, leaving
      room for the null terminator regardless of how `MAX_VAL_LEN`/`MAX_AUTH_LEN`
      are set. Tested by `test_base64_decode_respects_capacity`; all callers
      (WS handshake tests) updated to the new signature.
      Note: this was the only unguarded ingestion path - the HTTP parser
      (indexed bounds + `body[BODY_BUF_SIZE+1]`), multipart (bounded boundary copy
      over a null-terminated body), SSH `read_string()` (capacity-checked), the SSH
      banner (`SSH_VERSION_MAX` + explicit lengths), and the WS handshake
      (`strnlen(client_key, WS_MAX_KEY_LEN+1)`) are all correctly bounded.

</details>

## SSH protocol completeness (medium)

<details>
<summary><b>Expand SSH protocol completeness (medium) items</b></summary>

- [x] **`SSH_MSG_UNIMPLEMENTED` not sent for unknown messages.** _(done)_ The
      dispatcher's default case (`ssh_server.cpp`) now emits
      `SSH_MSG_UNIMPLEMENTED` with the rejected packet's sequence number
      (`ssh_pkt[i].seq_no_recv - 1`, since `ssh_pkt_recv` has already advanced the
      counter) per RFC 4253 §11.4 - no handler-signature change needed. Tested by
      `test_unimplemented_reply_for_unknown_message`.

- [ ] **Single SSH `session` channel only.** No port-forwarding, X11, or
      multiple channels (`ssh_channel.cpp` is a single-channel pool). Add a small
      channel table if multiplexing is needed. _(Deferred - YAGNI: a headless IoT
      device exposes one shell/exec session; multiplexing adds a channel table +
      window bookkeeping for no current use case.)_

- [ ] **Per-direction NEWKEYS.** A single `ssh_pkt[i].encrypted` flag flips on
      the client's NEWKEYS. Correct for the current send/receive ordering, but a
      strict implementation tracks inbound/outbound cipher activation separately
      (`ssh_packet.*`, `ssh_transport.cpp::ssh_newkeys_complete`). _(Deferred -
      YAGNI: the current strict send-then-receive ordering makes a single flag
      correct; splitting it is churn with no behavioral change until an
      out-of-order activation path exists.)_

- [ ] **Key-derivation extension (RFC 4253 §7.2).** `derive_key()` produces a
      single 32-byte block. Fine for AES-256/HMAC-SHA256/IV (≤32 B), but add the
      `K1‖K2‖…` extension loop before introducing any algorithm needing >32 bytes.
      _(Deferred - YAGNI: every negotiated algorithm needs ≤32 B; add the loop
      only when an algorithm that needs more is introduced.)_

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
      `examples/07.SSHCryptoSelfTest` validates the HW path on-device.

- [x] **AES-256-CTR re-acquired the HW engine once per 16-byte block.** _(done)_
      The Arduino `ssh_aes256ctr_crypt()` now makes a single
      `mbedtls_aes_crypt_ctr()` call for the whole buffer (our `counter` /
      `keystream` / `pos` fields map 1:1 to mbedtls's `nonce_counter` /
      `stream_block` / `nc_off`), replacing the per-block
      `mbedtls_aes_crypt_ecb()` loop. Native software path unchanged. Validated by
      the native AES-CTR KATs and `examples/07.SSHCryptoSelfTest` on-device.

</details>

## HTTP / core (medium)

<details>
<summary><b>Expand HTTP / core (medium) items</b></summary>

- [ ] **No TLS (HTTPS).** Plain HTTP only; relies on a trusted LAN, a TLS
      terminator, or the SSH channel. A real fix is large (mbedTLS TLS server).

- [ ] **`Date` response header not emitted.** Acceptable for a clock-less device
      (RFC 7231 §7.1.1.2). A time source now exists (`DETWS_ENABLE_NTP` +
      `detws_ntp_http_date()`); auto-injecting `Date` into every response is left
      off the hot path - apps that want it can add the header from a handler.

- [ ] **Recv scratch on the stack.** `ssh_pkt_recv` uses a
      `SSH_PKT_BUF_SIZE + 32` (~2 KB) stack buffer; size the ESP32 SSH task stack
      accordingly or move it to BSS.

</details>

## Optional services / features (toggleable, default off)

<details>
<summary><b>Expand Optional services / features (toggleable, default off) items</b></summary>

Capabilities a small IoT web server commonly needs but the library does not yet
provide. Each should follow the existing feature-flag convention - a
`DETWS_ENABLE_*` macro defaulting to 0, gating its own `.cpp`/pool so it costs
no code, RAM, or flash when disabled (`DetWebServerConfig.h`). Roughly ordered
by how often a deployed device needs it.

- [x] **mDNS / DNS-SD advertisement (`DETWS_ENABLE_MDNS`).** _(done)_
      `detws_mdns_begin(hostname, port)` (`src/services/mdns_service.*`) makes the
      device reachable at `<hostname>.local` and advertises `_http._tcp`. Uses the
      **ESP-IDF `mdns` component directly** (not the `ESPmDNS` add-on) to keep the
      dependency set to base-SDK + mbedTLS. Firmware links (`examples/08.Services`).

- [x] **OTA firmware update (`DETWS_ENABLE_OTA`).** _(done)_ Authenticated
      streaming `POST /update` into the ESP32 `Update` API
      (`src/services/ota_service.*`). The HTTP parser gained a `#if DETWS_ENABLE_OTA`
      streaming-body hook (`http_parser_set_stream_hooks`) that feeds the image to
      `Update.write()` in `BODY_BUF_SIZE` chunks instead of buffering it - so the
      `BODY_BUF_SIZE`/413 cap is bypassed and multi-MB images never live in RAM.
      The matching route handler replies + reboots. Parser hook native-tested
      (`test_http_ota`, env `native_ota`) with **no regression** to the 80 parser
      tests (fully gated); `examples/09.OTA` firmware links.

- [x] **WiFi provisioning / captive portal (`DETWS_ENABLE_PROVISIONING`).** _(done)_
      `src/services/provisioning_service.*`: first-boot softAP + a catch-all DNS
      responder + a credentials form, persisting SSID/PSK to NVS
      (`detws_provisioning_load`/`_begin`/`_clear`, `examples/10.Provisioning`).
      The DNS responder is a **raw lwIP UDP socket** (no `DNSServer` add-on) -
      callback-driven, so no per-loop polling. The form-field/URL-decode parser is
      native-tested (`test_provisioning`, env `native_prov`); firmware links.

- [x] **Pre-compressed static asset serving.** _(done)_ `serve_static()` serves
      `<path>.gz` with `Content-Encoding: gzip` when the client sends
      `Accept-Encoding: gzip` and the `.gz` exists (original Content-Type
      preserved). No separate flag needed - it is zero-cost when no `.gz` is
      present. Tested by `test_serve_static_gzip_when_accepted` /
      `test_serve_static_no_gzip_when_not_accepted`.

- [x] **Conditional GET / ETag (`DETWS_ENABLE_ETAG`).** _(done)_ `serve_file()` /
      `serve_static()` emit a strong `ETag` (`"<hexsize>-<hexmtime>"` from
      `f.size()` + `f.getLastWrite()`) and answer a matching `If-None-Match` with
      `304 Not Modified` (no body). The FS test mock gained `getLastWrite()` so it
      is host-tested (`test_serve_static_etag_conditional_get`); ESP32 path
      compile-verified against the real `fs::File`. (`Last-Modified`/
      `If-Modified-Since` intentionally skipped - ETag alone is sufficient and
      avoids HTTP-date parsing.)

- [x] **SNTP time sync (`DETWS_ENABLE_NTP`).** _(done)_ `detws_ntp_begin()`/
      `_synced()`/`_epoch()`/`_http_date()` (`src/services/ntp_service.*`) wrap
      `configTzTime` (ESP-IDF SNTP) and format an RFC 7231 `Date`. `examples/08.Services`
      exposes `GET /time`; firmware links. (Auto-emitting the `Date` response
      header is left to the app via the helper - kept off the hot path.)

- [ ] **Implement zero-copy template slicing.** The templating engine must operate within 
      strict boundaries. If a user tries to expand a template placeholder into a string 
      that exceeds the pre-allocated channel or work buffer slot, reject/truncate the token 
      or enforce an immediate boundary guard.

- [ ] **Implement JSON serialization** JSON parsing or serialization must be constrained. If 
      the serialized JSON payload attempts to cross the pre-allocated buffer ceiling, it hits 
      an ingestion or serialization boundary guard. That means strict pre-defined caps on the 
      depth and size of the JSON payloads.

(Deliberately omitted as not worth the footprint for this class of device:
HTTP Digest auth, WebSocket permessage-deflate, per-request access logging.)

</details>

## Quality-of-life (developer / operator)

<details>
<summary><b>Expand Quality-of-life (developer / operator) items</b></summary>

Convenience that does not add protocol capability but removes friction. Newbie
items lower the floor for first-time users; operator items help whoever runs a
deployed device.

Newbie / developer experience:

- [x] **One-call static directory mount.** _(done)_
      `serve_static(url_prefix, fs, fs_root)` (`DeterministicESPAsyncWebServer.h`)
      mounts a filesystem subtree at a URL prefix via a wildcard `ROUTE_STATIC`:
      `index.html` fallback for `/` or directory requests, MIME auto-detection,
      gzip-static, path-traversal rejection, GET/HEAD only (else 405). Tested by
      the `test_serve_static_*` suite.

- [x] **MIME type auto-detection by extension.** _(done)_ `DetWebServer::mime_type(path)` - a static, case-insensitive extension→type table (html/css/js/json/svg/
      png/jpg/gif/ico/webp/wasm/woff2/… → falls back to
      `application/octet-stream`). Used automatically by `serve_static()` and
      callable directly with `serve_file()`. Tested by `test_mime_type_detection`.

- [x] **Named `begin()` failure codes.** _(done)_ `begin()`/`listen()`/`restart()`
      now return a `DetWebServerResult` enum: `DETWS_OK`, `DETWS_ERR_NO_LISTENERS`,
      `DETWS_ERR_LISTENER_FULL`, `DETWS_ERR_LISTEN_FAILED`
      (`DeterministicESPAsyncWebServer.h`/`.cpp`). Subsumes the heap-bytes mismatch
      item below (docstring corrected).

- [x] **`redirect()` helper.** _(done)_ `server.redirect(slot_id, code, location)`
      sends a `Location` header + empty body and closes; accepts 301/302/303/307/308
      (any other code → 302). Tested by `test_redirect_*`.

Operator / sysadmin:

- [x] **Runtime stats endpoint (`DETWS_ENABLE_STATS`).** _(done)_ `server.stats(slot)`
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
      above), and `heap_needed()`/`heap_available()` remain the way to check heap.

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
      called the removed `DeterministicAsyncTCP::init(80)`; now `pool_init()`).
      All 35 cases pass.

- [ ] **Update `docs/CHANGELOG.md`** for this cycle: HTTP RFC fixes (Host,
      Content-Length, 405/501, HEAD), WebSocket masking/fragmentation, the full
      SSH server (KEX→auth→channel), publickey auth, `DETWS_SSH_ALLOW_PASSWORD`,
      and the docs reorg.

- [x] **Add an SSH usage example** _(done)_ - `examples/06.SSH/06.SSH.ino`:
      enables SSH, loads the host key from NVS (`ssh_rsa_load_pubkey()`), installs
      password + publickey auth callbacks and a channel data callback that echoes
      via the new `ssh_conn_send()` helper, listens on `PROTO_SSH`. Required a
      small public outbound API (`ssh_conn_send()`, `ssh_conn.*`) since the
      dispatcher's emit path was internal-only.

- [x] **Publish RSA host-key provisioning docs** _(done)_ - `docs/SSH.md`
      now has a "Host key provisioning" section: `openssl genrsa` →
      `pkcs8 -topk8 -outform DER`, embed + write to NVS (`ssh_host_key/priv_der`)
      with `Preferences`, and `ssh_rsa_load_pubkey()` at boot.

</details>
