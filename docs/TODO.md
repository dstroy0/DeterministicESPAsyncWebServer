# TODO / Known Fixes

Outstanding work and known limitations, roughly highest-impact first. Items are
grouped by area; each names the file(s) involved so the fix is easy to locate.

## Security / correctness (high priority)

- [ ] **Native RSA signing is a `d=1` test stub, not a real signature.**
      `ssh_rsa_sign()` native path (`ssh_rsa.cpp`) truncates the product before
      reducing mod n, so it only works for `d=1`. Implement a correct private-key
      operation by reusing the now-correct `bn_mul_full` / `bn_reduce_full`
      helpers added for `ssh_rsa_verify` (ideally with CRT for speed). Until then
      the native build must never be presented as producing real signatures
      (ESP32/mbedTLS path is real). Tracked in `SECURITY.md` §0 (❌).

- [ ] **No authentication attempt limiting (brute-force).** `ssh_auth_handle_request()`
      accepts unlimited `USERAUTH_REQUEST`s; HTTP Basic Auth has no lockout.
      Add a per-connection failed-attempt counter and disconnect after N
      (RFC 4252 allows a small bounded number). Files: `ssh_auth.cpp` /
      `ssh_server.cpp`; `DeterministicESPAsyncWebServer.cpp` for HTTP.

- [ ] **Software crypto paths are not constant-time.** Native Montgomery
      (`ssh_bignum.cpp`), `bn_reduce_full` (`ssh_rsa.cpp`), and software AES
      (`ssh_aes256ctr.cpp`) have data-dependent timing. They are test-only (ESP32
      uses HW/mbedTLS), but either harden or assert they are never compiled into
      firmware. Tracked in `SECURITY.md` §0 (⚠️).

- [ ] **No connection-flood / rate limiting.** A burst of connections exhausts
      the fixed pool and returns 503; there is no throttling or allow-list.
      Consider per-IP accept throttling at the listener layer (`listener.cpp`).

- [ ] **`base64_decode()` has no output-capacity guard (Basic-auth ingestion).**
      `base64_decode(const char *src, uint8_t *dst)` (`base64.cpp`/`.h`) writes
      until the source string ends with no destination bound. `check_basic_auth()`
      (`DeterministicESPAsyncWebServer.cpp`) decodes the `Authorization` value into
      `decoded[MAX_AUTH_LEN*2+2]`; this is safe **only** by the implicit invariant
      that a `MAX_VAL_LEN`-bounded header value decodes to ≤ that size (true at
      defaults: 48→36 ≤ 66). The `n >= sizeof(decoded)` check is *post-write* and
      cannot prevent an overflow if `MAX_VAL_LEN` is raised relative to
      `MAX_AUTH_LEN`. Fix: add a `dst_cap` parameter to `base64_decode()` (bound
      the write), or add a `static_assert`/`#error` tying the two macros together.
      Note: zero-initializing the destination (`arr[sz] = {0}`) only guarantees
      *termination*, not the capacity bound - the overrunning write must still be
      bounded. (Termination-by-zero-init is already used elsewhere, e.g.
      `http_parser_reset()`'s `*req = {}`, which is why multipart's `strstr` over
      `body` is safe.)
      Note: this is the only unguarded ingestion path found - the HTTP parser
      (indexed bounds + `body[BODY_BUF_SIZE+1]`), multipart (bounded boundary copy
      over a null-terminated body), SSH `read_string()` (capacity-checked), the SSH
      banner (`SSH_VERSION_MAX` + explicit lengths), and the WS handshake
      (`strnlen(client_key, WS_MAX_KEY_LEN+1)`) are all correctly bounded.

## SSH protocol completeness (medium)

- [ ] **`SSH_MSG_UNIMPLEMENTED` not sent for unknown messages.** The dispatcher
      (`ssh_server.cpp` default case) silently ignores unrecognized messages
      because it lacks the packet sequence number RFC 4253 §11.4 requires. Plumb
      the inbound seq number from `ssh_pkt_recv` into the handler and emit
      `SSH_MSG_UNIMPLEMENTED(seq)`.

- [ ] **Single SSH `session` channel only.** No port-forwarding, X11, or
      multiple channels (`ssh_channel.cpp` is a single-channel pool). Add a small
      channel table if multiplexing is needed.

- [ ] **Per-direction NEWKEYS.** A single `ssh_pkt[i].encrypted` flag flips on
      the client's NEWKEYS. Correct for the current send/receive ordering, but a
      strict implementation tracks inbound/outbound cipher activation separately
      (`ssh_packet.*`, `ssh_transport.cpp::ssh_newkeys_complete`).

- [ ] **Key-derivation extension (RFC 4253 §7.2).** `derive_key()` produces a
      single 32-byte block. Fine for AES-256/HMAC-SHA256/IV (≤32 B), but add the
      `K1‖K2‖…` extension loop before introducing any algorithm needing >32 bytes.

## Performance / hardware acceleration (medium)

- [ ] **SSH per-packet HMAC runs in software SHA-256 on ESP32.** `compute_mac()`
      (`ssh_packet.cpp`) MACs every inbound and outbound packet via
      `ssh_hmac_sha256_*`, which is built on the *streaming* `ssh_sha256_init/
      update/final` - and those are software on **both** platforms
      (`ssh_sha256.cpp`: only the one-shot `ssh_sha256()` uses the mbedtls/HW SHA
      accelerator). This makes software SHA-256 the bulk-throughput bottleneck for
      all SSH traffic. Fix: back the streaming context with
      `mbedtls_sha256_context` (`mbedtls_sha256_starts/update/finish`) on Arduino
      so the HW SHA engine accelerates both per-packet HMAC and KEX hashing. This
      also makes the (currently inaccurate) HW-acceleration comment in the
      `ssh_hmac_sha256.cpp` header true.

- [ ] **AES-256-CTR re-acquires the HW engine once per 16-byte block.** The
      Arduino path (`ssh_aes256ctr.cpp`) calls `mbedtls_aes_crypt_ecb()` per block
      inside `ssh_aes256ctr_crypt()`; on ESP32 each ECB call acquires the AES
      peripheral and reloads the key, so the per-block setup dominates for bulk
      data. Switch the Arduino path to `mbedtls_aes_crypt_ctr()` (one acquire for
      the whole buffer, mbedtls manages the counter + keystream offset). Keep the
      native software path as-is.

## HTTP / core (medium)

- [ ] **No TLS (HTTPS).** Plain HTTP only; relies on a trusted LAN, a TLS
      terminator, or the SSH channel. A real fix is large (mbedTLS TLS server).

- [ ] **`Date` response header not emitted.** Acceptable for a clock-less device
      (RFC 7231 §7.1.1.2); revisit if an RTC/NTP time source is available.

- [ ] **Recv scratch on the stack.** `ssh_pkt_recv` uses a
      `SSH_PKT_BUF_SIZE + 32` (~2 KB) stack buffer; size the ESP32 SSH task stack
      accordingly or move it to BSS.

## Optional services / features (toggleable, default off)

Capabilities a small IoT web server commonly needs but the library does not yet
provide. Each should follow the existing feature-flag convention - a
`DETWS_ENABLE_*` macro defaulting to 0, gating its own `.cpp`/pool so it costs
no code, RAM, or flash when disabled (`DetWebServerConfig.h`). Roughly ordered
by how often a deployed device needs it.

- [ ] **mDNS / DNS-SD advertisement (`DETWS_ENABLE_MDNS`).** Reach the device at
      `name.local` instead of a DHCP IP, and advertise `_http._tcp`. Thin wrapper
      over ESP32 `ESPmDNS`; near-essential for headless LAN devices. Hooks in at
      `begin()` after the IP is up.

- [ ] **OTA firmware update (`DETWS_ENABLE_OTA`).** Authenticated
      `POST /update` streaming a firmware image into the ESP32 `Update` API
      (rollback on bad image). Builds on the existing multipart parser and Basic
      auth; the main work is streaming the body to `Update.write()` instead of
      buffering into `BODY_BUF_SIZE`. Essential once a device ships and USB
      reflashing is impractical.

- [ ] **WiFi provisioning / captive portal (`DETWS_ENABLE_PROVISIONING`).**
      First-boot AP mode with a catch-all DNS responder and a credentials form,
      persisting SSID/PSK to NVS - removes the hardcoded `SSID`/`PASSWORD` the
      examples rely on. Needs a softAP path in the physical layer
      (`physical.cpp`) plus a small DNS responder.

- [ ] **Pre-compressed static asset serving (`DETWS_ENABLE_GZIP_STATIC`).**
      When the client sends `Accept-Encoding: gzip` and a `<path>.gz` exists,
      serve it with `Content-Encoding: gzip`. Big win for shipping a web UI from
      flash on a constrained device. Extends `serve_file()`; the examples already
      detect the `Accept-Encoding` header but cannot act on it.

- [ ] **Conditional GET / ETag (`DETWS_ENABLE_ETAG`).** Emit `ETag`/
      `Last-Modified` for served files and answer `If-None-Match`/
      `If-Modified-Since` with `304 Not Modified`. Saves bandwidth and battery on
      repeat fetches of static assets. Extends `serve_file()`.

- [ ] **SNTP time sync (`DETWS_ENABLE_NTP`).** Optional SNTP client so the device
      has wall-clock time. Unblocks the `Date` response header (see HTTP/core
      item above), log timestamps, and future TLS certificate validity checks.
      Thin wrapper over ESP-IDF `esp_sntp`.

(Deliberately omitted as not worth the footprint for this class of device:
HTTP Digest auth, WebSocket permessage-deflate, per-request access logging.)

## Quality-of-life (developer / operator)

Convenience that does not add protocol capability but removes friction. Newbie
items lower the floor for first-time users; operator items help whoever runs a
deployed device.

Newbie / developer experience:

- [ ] **One-call static directory mount.** `serve_file()`
      (`DeterministicESPAsyncWebServer.h`) serves a single file with a
      caller-supplied content-type, so serving a web UI means wiring one route
      per asset. Add `serve_static(url_prefix, fs, fs_root)` that maps a wildcard
      route to a filesystem subtree with an `index.html` fallback. Biggest single
      DX win for newbies shipping a front-end from flash.

- [ ] **MIME type auto-detection by extension.** Pairs with the above: guess
      `Content-Type` from `.html/.css/.js/.json/.png/.svg/.ico/...` so callers
      stop passing it by hand to `serve_file()`. Small static extension→type
      table; falls back to `application/octet-stream`.

- [ ] **Named `begin()` failure codes.** `begin()` returns a bare `-1`
      (`DeterministicESPAsyncWebServer.cpp`), so a newbie can't tell "no listeners
      registered" from "listener pool full" from "lwIP error". Return a small
      negative enum (e.g. `DETWS_ERR_NO_LISTENERS`, `DETWS_ERR_POOL_FULL`,
      `DETWS_ERR_LWIP`). Supersedes / merges with the heap-bytes mismatch item
      below.

- [ ] **`redirect()` helper.** A `server.redirect(slot_id, 301|302, location)`
      convenience (Location header + empty body) - common for `/`→`/index.html`
      or canonical-host redirects, currently hand-rolled via `send_empty()`.

Operator / sysadmin:

- [ ] **Runtime stats endpoint (`DETWS_ENABLE_STATS`).** The existing `DETWS_DIAG_JSON`
      is *compile-time* config only. Add a runtime counters block - uptime,
      requests served, 4xx/5xx counts, active/peak connection-pool usage, min
      free heap - exposed as JSON. The expert example inspects `conn_pool[]` by
      hand; this makes the same data a first-class, monitorable endpoint.

- [ ] **Per-request log callback hook.** `server.on_request_log(cb)` invoked once
      per completed request with method/path/status/bytes, so an operator can
      route access logs to Serial, syslog, or a file. Note: this is the *hook*
      only (one function pointer, no in-library buffering/formatting) - distinct
      from the built-in access-logging that was deliberately omitted above as too
      heavy for this device class.

## Examples (low)

- [ ] **`begin()` heap-bytes contract mismatch.** The header docstring
      (`DeterministicESPAsyncWebServer.h`) says `abs(result)` is the "heap bytes
      needed", but `begin()` returns a literal `-1` on failure, not a negative byte
      count (`DeterministicESPAsyncWebServer.cpp::begin`). Either return the
      shortfall from `heap_needed()`/`heap_available()` or correct the docstring.
      (The reorganized examples no longer print `-result` as a byte count; they
      report the raw error code.)

## Housekeeping (low)

- [ ] **Native `base64_decode()` accepts `=` outside the trailing pad.**
      `b64_val('=')` returns 0 (`base64.cpp`), so a `=` in symbol position 0/1 of
      a quad passes the `< 0` validity check and decodes as a zero sextet instead
      of being rejected (e.g. `"A=BC"`). Device builds use strict mbedtls, so this
      only affects the native/test decoder, but a malformed vector can pass a test
      it should fail. Reject `=` except as 1-2 trailing pad chars.

- [ ] **`test/test_application/` is orphaned** - present but not in any env's
      `test_filter` (`platformio.ini`). Wire it into an env or remove it.

- [ ] **Update `docs/CHANGELOG.md`** for this cycle: HTTP RFC fixes (Host,
      Content-Length, 405/501, HEAD), WebSocket masking/fragmentation, the full
      SSH server (KEX→auth→channel), publickey auth, `DETWS_SSH_ALLOW_PASSWORD`,
      and the docs reorg.

- [ ] **Add an SSH usage example** under `examples/` (host key from NVS,
      `ssh_auth_set_pubkey_cb` / `ssh_auth_set_password_cb`, a data callback).

- [ ] **Publish RSA host-key provisioning docs** - how to generate and store the
      DER key in NVS (`ssh_rsa.cpp` reads `ssh_host_key/priv_der`).
