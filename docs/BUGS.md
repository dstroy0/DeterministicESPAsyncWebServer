# Bug log

A running record of every bug found in this library: what broke, the root cause,
the fix, and status. Newest first. A bug is logged here the moment it is found
(even before it is fixed) so nothing slips.

Status key: **OPEN** (found, not fixed) - **FIXED** (fixed, validated) - **SHIPPED** (released).

---

## HTTP/3 TLS flight append ignored its buffer cap (potential `flight_hs` overflow)

- **Status:** FIXED (native_quic_tls / native_tls13_msg / native_h3_e2e green; flagged by SonarCloud
  cpp:S3519 as two BLOCKER "memory copy overflows the destination buffer" findings).
- **Found:** 2026-07-06, reviewing the SonarCloud quality gate on the HTTP/3 handshake code.
- **Symptom:** the TLS 1.3 server flight is built message-by-message into a fixed `qt->flight_hs`
  buffer, each builder called with `sizeof(flight_hs) - flight_hs_len` as its capacity. `emit()` was
  handed that same capacity but ignored it (`(void)cap;`) and did `*plen += written` unconditionally.
- **Root cause:** nothing bounded `flight_hs_len` against `sizeof(flight_hs)`. In the correct flow each
  builder returns `<= cap`, so the sum stayed in bounds, but the invariant was never enforced: if a
  builder ever returned more than the remaining room, a later builder's `sizeof(flight_hs) -
flight_hs_len` would underflow to a huge `cap` and `w_bytes()` would `memcpy` past the fixed array.
- **Fix:** `emit()` now honors the `cap` contract it was given - it refuses an append that would run
  past the buffer (`written > cap - *plen`), maintaining the `*plen <= cap` invariant so the next
  builder's capacity subtraction can never underflow. `w_bytes()` also now checks `w->pos > w->cap`
  explicitly before the `w->cap - w->pos` bound so that subtraction provably cannot underflow even for
  a malformed `Writer`. Separately, the HTTP/3 DATA-frame coalescing in `dispatch_request()` now clamps
  with a room subtraction (`sizeof(body) - body_len`) instead of a `body_len + take` sum that could
  wrap. All three are the codebase's own overflow-safe idiom.

---

## QUIC anti-amplification checked after building, desyncing the flight under loss

- **Status:** FIXED (RPi netem loss interop + native_quic_conn; found while adding PTO loss recovery).
- **Found:** 2026-07-06, driving the HTTP/3 interop harness under 10-20% netem packet loss - a
  timer-polled server got _worse_ with loss, not better, and connections stalled.
- **Symptom:** under loss, `quic_conn_send()` advanced packet-number / CRYPTO-offset / stream-send
  state for a datagram it then discarded, so the retransmitted flight no longer matched what the peer
  had (or had not) received. Loss recovery made the stall worse instead of curing it.
- **Root cause:** the 3x anti-amplification check (RFC 9000 sec 8.1) ran _after_ `build_packet()` had
  already bumped `next_pn` / `crypto_tx_off` / `tx_sent` / `last_ae_pn`. When the send was then
  amplification-blocked and dropped, that state stayed advanced - a build-then-discard desync.
- **Fix:** move the amplification check to the top of `quic_conn_send()`, before any packet is built,
  so a blocked send advances no packet state. Also reset the PTO backoff on acknowledged progress
  (RFC 9002 sec 6.2) so a recovering connection does not keep doubling its probe interval.

---

## SSH outbound wire buffer under-sized for a near-max payload + long MAC

- **Status:** FIXED (native SSH suites green; found while adding s2c compression, which made the
  overflow reachable in the general case).
- **Found:** 2026-07-05, sizing the wire buffer for compression - the existing sizing did not cover
  the worst case even without compression.
- **Root cause:** `ssh_conn.cpp` sized every outbound wire buffer as `SSH_PKT_BUF_SIZE +
SSH_HMAC_SHA256_LEN` (= 2080). A payload approaching `SSH_PKT_BUF_SIZE` with the hmac-sha2-512 MAC
  (64-byte tag) needs `4 + (1 + payload + pad) + 64` ≈ 2128 bytes, so `ssh_pkt_send()` would hit its
  `wire_len > out_cap` guard and return -1, dropping the packet. Latent because real payloads
  (channel-data chunks bounded by the peer window) never approached the max. With s2c compression the
  effective payload can also expand slightly (fixed-Huffman on incompressible data), and a dropped
  packet mid-stream desyncs the stateful cipher / compression stream (session corruption), so the
  under-size had to be fixed properly rather than relied upon to never trigger.
- **Fix:** a single `SSH_WIRE_CAP` in `ssh_packet.h` sized for the true worst case - `4 + 1 +
SSH_MAX_EFFECTIVE_PAYLOAD + SSH_MAX_PAD(32) + SSH_MAX_MAC(64)`, where the effective payload grows to
  `ssh_deflate_bound(SSH_PKT_BUF_SIZE)` when `DETWS_ENABLE_SSH_ZLIB` is set. All four wire buffers in
  `ssh_conn.cpp` use it. Correct for every cipher/MAC mode, compressed or not.

---

## UDP transport called raw lwIP off tcpip_thread (sibling of the listener bug)

- **Status:** FIXED (HW-validated on an ESP32-S3: the SNMP agent binds UDP/161 and runs;
  before, the same `udp_bind` from the app task would have asserted like the listener did).
- **Found:** 2026-07-04, immediately after the listener fix below - auditing for the same
  root cause in the UDP path, since UDP services bind at `begin()` from the app task too.
- **Root cause:** `udp_transport.cpp` (`det_udp_*`, the single place lwIP UDP is touched)
  called raw `udp_new` / `udp_bind` / `udp_recv` / `udp_remove` / `udp_sendto` **directly from
  the app task**, never marshaled. On arduino-esp32 3.x (lwIP core-locking) that trips
  `LWIP_ASSERT_CORE_LOCKED`, so every UDP service - SNMP agent (`:161`), CoAP, the captive-portal
  DNS responder (`:53`), syslog, UDP telemetry, flow-export - would crash at `begin()` / on send.
  Same latency story as the listener: harmless on the IDF-4.x board all runtime HW used, and CI
  only compiles the 3.x core. (The TCP transport `det_tcp_do` and the outbound client `cc_do_*`
  were already marshaled; UDP was the remaining raw path.)
- **Fix:** marshal the UDP ops onto tcpip_thread via `tcpip_api_call()` (a `udp_do` dispatcher for
  listen / send / send-out), mirroring the TCP transport - including the `s_in_tcpip_thread` guard
  so a handler replying from the `udp_recv` trampoline (already in-thread) sends directly instead of
  re-marshaling (which would deadlock). All `det_udp_*` callers are unchanged; the native host stubs
  are untouched. Native UDP-service suites green (`native_coap` / `native_snmp` / `native_dns_resolver`
  / `native_syslog`); HW: SNMP agent binds `:161` and the device runs on the modern core.
- **Prevention:** every raw lwIP `tcp_*` / `udp_*` from outside a lwIP callback now goes through
  `tcpip_api_call()`; the transport layer is the one owner of that rule (listener, TCP conn, UDP,
  client all marshal). See the listener entry below for the runtime-test-the-3.x-core lesson.

---

## Listener bring-up called raw lwIP off tcpip_thread, asserting on arduino-esp32 3.x

- **Status:** FIXED (HW-validated on an ESP32-S3: `begin_tls()` now starts the HTTPS
  listener and the server runs, where before it crash-looped at boot).
- **Found:** 2026-07-04, first on-hardware run of any listener on the arduino-esp32 **3.x**
  core (IDF 5.x) - surfaced while HW-proving the PSRAM TLS arena. The board booted, joined
  WiFi, then panicked in `begin_tls()`:
  `assert failed: tcp_alloc ...lwip/src/core/tcp.c:1854 (Required to lock TCPIP core functionality!)`,
  rebooting in a loop.
- **Root cause:** `listener_add()` / `listener_stop()` (the path `begin()` and `begin_tls()`
  use) called raw lwIP `tcp_new_ip_type` / `tcp_bind` / `tcp_listen_with_backlog` / `tcp_close`
  **directly from the app (setup/loop) task**, which is not `tcpip_thread` and does not hold the
  TCPIP core lock. arduino-esp32 3.x ships `CONFIG_LWIP_TCPIP_CORE_LOCKING=1` +
  `CONFIG_LWIP_CHECK_THREAD_SAFETY=1`, so `LWIP_ASSERT_CORE_LOCKED` fires. It was latent because
  (1) all prior runtime HW testing used the classic ESP32 via PlatformIO (arduino 2.x / IDF 4.x),
  where the assert is compiled out and the unlocked call merely raced, and (2) the "Arduino Build"
  CI only **compiles** the 3.x core, never runs it. The _dynamic_ listener (SSH `ssh -R`) already
  marshaled correctly via `tcpip_api_call()`; the primary listener was the one unmarshaled path,
  written under a stale "this build has core-locking off" assumption.
- **Fix:** `listener_add()` / `listener_stop()` now route their create/close through the same
  `listener_lwip_marshal()` (`tcpip_api_call`) the dynamic listener uses, on ARDUINO. The raw path
  stays only for the native host build (no lwIP thread). This is correct on **both** cores: with
  core-locking it satisfies the lock; without it, it still removes the off-thread race. The stale
  comment was corrected.
- **Prevention:** HW test the modern (arduino-esp32 3.x) core at runtime, not only compile it -
  the core-locking assert only fires when the code actually runs. Any new raw lwIP `tcp_*` from a
  non-callback context must go through `tcpip_api_call()` (transport `det_tcp_do` and the client
  `cc_do_*` were already correct; the listener now matches).

---

## SSH curve25519/ed25519 handshake overflowed the 8 KB worker task stack

- **Status:** FIXED (HW-validated on an ESP32-S3; the modern-crypto handshake completes with
  the raised default and ~1.8 KB of stack margin).
- **Found:** 2026-07-03, first on-hardware connect to the new curve25519-sha256 +
  ssh-ed25519 suite: the client reached `expecting SSH2_MSG_KEX_ECDH_REPLY` then
  `Connection reset by peer`, and the serial log showed
  `Guru Meditation Error: ... Stack canary watchpoint triggered (detws_worker)`.
- **Root cause:** the software field arithmetic for curve25519 (`ssh_x25519`) and ed25519
  (`ssh_ed25519_sign`), in radix-2^16 with many `ssh_gf` temporaries per frame, nests deeper
  than the finite-field DH/RSA path - plus the field inversion calls the mbedTLS bignum
  modexp at the bottom of that chain. Measured peak worker-task stack use is ~10.5 KB
  (`uxTaskGetStackHighWaterMark`: a 16 KB stack left 5928 B free at the deepest point), which
  overflows the historical 8 KB worker default sized for the ~7 KB RSA path.
- **Fix:** `DETWS_WORKER_TASK_STACK` now defaults to 12288 when `DETWS_ENABLE_SSH` is set
  (8192 otherwise), a new `DETWS_WORKER_STACK_CURVE_MIN` (12288) documents the floor, and the
  compile-time guard enforces it for SSH builds (the RSA-only floor still applies to
  OIDC-only builds). Re-flashed at the shipped 12288 default: the curve25519 + ssh-ed25519 +
  ed25519-client-auth session runs to a data round-trip with 1832 B of stack free at peak, no
  canary trip; the DH-group14 + rsa-sha2-256 path is unchanged (both echo on hardware).
- **Prevention:** the worker stack is now sized from a measured high-water mark, not a guess,
  and a build-time `#error` catches any future lowering below the curve floor before it can
  reach the canary at runtime.

---

## SSH answered SSH_MSG_GLOBAL_REQUEST with UNIMPLEMENTED instead of REQUEST_FAILURE

- **Status:** FIXED (`native_ssh` 127/127; the dispatcher now routes GLOBAL_REQUEST to a
  dedicated handler).
- **Found:** 2026-07-03, implementing `ssh -R` remote forwarding (which arrives as a
  `tcpip-forward` global request).
- **Root cause:** the SSH message dispatcher had no case for `SSH_MSG_GLOBAL_REQUEST` (80),
  so every connection-wide request fell through to the default arm and got
  `SSH_MSG_UNIMPLEMENTED` (RFC 4253 §11.4). That is wrong: GLOBAL_REQUEST is a **known**
  message type - only the request _name_ may be unknown - and RFC 4254 §4 says an
  unrecognized request is answered with `SSH_MSG_REQUEST_FAILURE` when `want_reply` is set,
  or ignored otherwise. In practice this broke OpenSSH client keepalives
  (`keepalive@openssh.com`, `want_reply=true`): the client saw an UNIMPLEMENTED rather than a
  SUCCESS/FAILURE reply and could treat the keepalive as unanswered, and benign
  `want_reply=false` requests (`hostkeys-00@openssh.com`, `no-more-sessions@openssh.com`) drew
  a spurious UNIMPLEMENTED.
- **Fix:** `ssh_global_request_handle()` (ssh_channel) now parses the request name +
  `want_reply`, replies `REQUEST_FAILURE` (or stays silent) per §4, and routes
  `tcpip-forward` / `cancel-tcpip-forward` to an opt-in remote-forward seam (accepted only
  when an owner is installed; a port-0 bind echoes its allocated port per §7.1). Host-tested
  (`test_ssh_channel`: accept / refuse / port-0 echo / no-reply / cancel / unknown-request /
  malformed).
- **Prevention:** the seam is the plug-in point for the `ssh -R` listener + byte-bridge owner
  (the next phase); until it is installed the server correctly declines forwards rather than
  making a promise it cannot keep.

---

## Abuse-prevention state keyed on a 32-bit hash of the IPv6 source address (security)

- **Status:** FIXED (transport now carries the full family-tagged address; all IP-keyed
  features match the whole IPv4/IPv6 address; `native_det_ip` / `native_accept_gate` /
  `native_auth_lockout` / `native` green with added v6 coverage).
- **Found:** 2026-07-03, reviewing the IPv6 phase-2 work. Shipped in v4.87.0 (auth lockout)
  and v4.88.0 (per-IP throttle) - both superseded by this fix.
- **Root cause:** when IPv6 arrived, the per-IP throttle, the auth-lockout table, and the
  accept-time source key all reduced the peer address to a 32-bit value: a raw v4 word, and
  for v6 a **32-bit FNV-1a hash** (`det_ip_key` / `det_conn_remote_key` / `det_lwip_ip_key`).
  In a security-forward library that is a real vulnerability, not a shortcut:
    - **Targeted lockout / throttle poisoning.** A 32-bit hash is trivially collidable, so an
      attacker can pick a v6 source that hashes to the same bucket as a victim and drive the
      victim into lockout or throttle denial (a remote DoS of a specific peer).
    - **Per-IP cap evasion.** A v6 attacker owns a whole `/64` (2^64 addresses); collisions let
      many real addresses share one bucket (reset/share another's budget) or dodge their own cap.
    - **Cross-family confusion.** Flattening a v6 peer toward a v4-shaped key risks it colliding
      with a real v4 host's bucket.
- **Fix:** the transport pipe was made protocol-agnostic. `DetIp` (a family tag + 16 address
  bytes, the library's `sockaddr_storage`/lwIP `ip_addr_t` equivalent) is now carried end to
  end: `det_conn_remote_addr()` / `det_lwip_to_detip()` produce it, and **every** IP-keyed
  feature stores and matches the **full** address - bucket lookups by `det_ip_equal` (throttle,
  lockout), allowlist by `det_ip_prefix_match` (real v4 `/0-32` and v6 `/0-128` CIDR
  containment). The hash/word keys (`det_ip_key`, `det_conn_remote_key`, `det_lwip_ip_key`) and
  the host-order `DETWS_IPV4` allowlist macro are deleted; the public allowlist entry point is
  now CIDR text (`listener_ip_allow_add_cidr("2001:db8::/32")`). No hashing, no flattening.
- **Prevention:** the native suites now assert the security property directly - distinct v6
  peers get distinct throttle/lockout buckets, a v6 peer never shares a v4 peer's state, and a
  v4 allowlist rule never admits a v6 peer (and vice versa). One owner (the transport) resolves
  the peer address; features consume `DetIp` and never re-key it.

---

## 06.PreemptQueue example used the pre-3.0 Arduino-ESP32 timer API

- **Status:** FIXED (compiles on esp32:esp32 3.3.10; still builds on the 2.x core the
  PlatformIO CI pins).
- **Found:** 2026-07-02, the all-example compile sweep against the 3.x core (the second
  break it caught, after ESP-NOW).
- **Root cause:** Arduino-ESP32 **3.0** reworked the timer API - `timerBegin(freq)` is now
  frequency-based (was `timerBegin(num, divider, countUp)`), `timerAttachInterrupt()` dropped
  its edge argument, and `timerAlarm()` replaced `timerAlarmWrite()` + `timerAlarmEnable()`.
  The example's ISR setup used the 2.x forms, so it failed to compile under the core an
  Arduino IDE user installs.
- **Fix:** the timer block in `06.PreemptQueue.ino` is guarded with
  `#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)` and uses the 3.x calls on the
  new core, the 2.x calls otherwise. It is the only example using that API (grep-verified).
- **Prevention:** a new **Arduino Build** CI (`.github/workflows/arduino-build.yml`) now
  compiles every example against the Arduino-ESP32 3.x core with arduino-cli - using each
  example's shipped `build_opt.h`, exactly as the IDE builds it - so 3.x / IDF-5 API drift is
  caught on every push, not just in a manual local sweep.

---

## ESP-NOW would not compile on the Arduino-ESP32 3.x core (ESP-IDF 5 recv-callback ABI)

- **Status:** FIXED (compiles clean on esp32:esp32 3.3.10 via arduino-cli; `native_espnow`
  host suite still 7/7).
- **Found:** 2026-07-02, first compile of `53.EspNow` in the Arduino IDE toolchain
  (esp32 core 3.x). The PlatformIO CI pins `espressif32 @ ^6.0.0` (Arduino-ESP32 **2.x**,
  ESP-IDF 4.4), so it never exercised the 3.x API and reported green - the break only
  showed up under the core an Arduino IDE user actually installs.
- **Root cause:** ESP-IDF 5.0 changed the `esp_now_recv_cb_t` signature. The receive
  callback used the 4.x shape `(const uint8_t *mac, const uint8_t *data, int len)`; under
  IDF 5 the source MAC moved into a struct and the type is now
  `(const esp_now_recv_info_t *info, const uint8_t *data, int len)`, so registering the old
  callback was an invalid conversion (`-fpermissive` error).
- **Fix:** `services/espnow/espnow.cpp` selects the callback signature with an
  `#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)` guard (mirrors how `ssh_rsa` handles
  mbedTLS v2/v3) and reads the MAC from `info->src_addr` on 5.x, the `mac` argument on 4.x.
  Both cores now build. The whole binding is under `#ifdef ARDUINO`, so host tests are
  unaffected.
- **Prevention:** all examples now build unmodified in the Arduino IDE (each ships a
  `build_opt.h`) and were compile-swept against the 3.x core; a broad multi-feature 3.x
  compile found no other IDF-5 API breaks.

---

## DMA frames corrupted (~8%) when the completion was posted to the work queue as a pointer

- **Status:** FIXED (HW-verified on an ESP32 DevKitV1: 2.2M+ frames ingested with zero
  integrity errors under concurrent HTTP stress, no heap growth).
- **Found:** 2026-07-02, first HW stress of the new DMA ingest path (`services/dma`). A
  combined webserver + continuous-DMA rig reported `dma_errors: 8824` out of ~95946 frames
  (~8%) via an HTTP counter - while all 11 host tests passed. Textbook "a small happy-path
  smoke hides the bug; stress on hardware surfaces it."
- **Root cause:** the DMA-complete callback posted the `det_dma_event` whose `data` is a
  **pointer into the 2-deep ping-pong RX buffer** into the 16-deep preempting work queue.
  `detws_pq_post_from_isr()` calls `portYIELD_FROM_ISR()`, but the simulator drives the
  callback from a **task** context (not a real ISR), where the yield is not an immediate
  context switch - so under load `loop()` ran several more completions before the
  high-priority task drained the queue, the two ping-pong buffers wrapped, and the older
  queued pointers read half-overwritten data. A descriptor consumed by another task must
  own its bytes, not point into a buffer that is reused a transfer or two later.
- **Fix:** the callback now **copies the frame bytes into the queue item** (a self-contained
  message) instead of posting the pointer. Eliminates the dangling reference entirely
  (error rate 8% -> 0%). The example `07.DmaIngest` uses the same copy pattern, and
  `det_dma.h` now documents that a deferred (queued) consumer must copy the `len` bytes
  rather than keep the RX pointer. The pointer API itself is unchanged and correct for an
  in-callback consumer (the standard DMA-HAL shape).
- **Tests:** the host suite (`native_dma`) covers the ping-pong flip, byte-exact loopback
  round trip, and fail-closed paths; the deferred-consumer lifetime is a HW-load property,
  now covered by the on-device stress rig.

---

## SSH server could not interoperate with a stock OpenSSH client

- **Status:** FIXED (host-tested + HW-verified on an ESP32 against OpenSSH 9.5: `ssh`, pubkey
  auth, and `ssh -L` port forwarding all work with no client-side algorithm overrides).
- **Found:** 2026-06-30, first live `ssh` against the server once a host key was provisioned.
  A stock `ssh user@board` reset during key exchange; even forcing the KEX through, RSA pubkey
  auth was refused. The earlier "handshake HW-verified" had only ever used a hand-built
  minimal client (the native test) and a tiny forced-algorithm client.
- **Two independent root causes, both fixed:**
    - **The RX ring and the KEXINIT store were smaller than a real client's first flight.** A
      modern OpenSSH KEXINIT (post-quantum + curve KEX names, cert host-key algs, EtM MACs,
      `ext-info-c`) is ~1.5 KB. `SSH_KEXINIT_MAX` was 512, so `ssh_kexinit_parse()` rejected
      the payload outright (`len > SSH_KEXINIT_MAX` -> dispatch returns -1 -> RST), and the
      default 1024-byte `RX_BUF_SIZE` ring could not hold the banner + KEXINIT, resetting the
      handshake at key exchange. Fixed by raising `SSH_KEXINIT_MAX` to 2048 (client I_C; the
      server I_S stays small at `SSH_KEXINIT_S_MAX`) and auto-upsizing `RX_BUF_SIZE` to >= 2048
      when SSH is enabled and the ring was left at its default (same idiom as the streaming-body
      upsize; an explicit `RX_BUF_SIZE` is honored).
    - **No RFC 8308 `ext-info` / `server-sig-algs`.** Without it a modern OpenSSH client will
      not sign an RSA key (`send_pubkey_test: no mutual signature algorithm`) and falls back to
      password. Fixed by advertising `ext-info-s` in the server KEXINIT, detecting the client's
      `ext-info-c`, and sending `SSH_MSG_EXT_INFO` with `server-sig-algs = rsa-sha2-256` as the
      first message after NEWKEYS. An inbound `EXT_INFO` is now accepted (ignored) rather than
      answered with UNIMPLEMENTED.
- **Tests:** `test_ssh_server` gains `test_extinfo_build_advertises_server_sig_algs` and
  `test_large_client_kexinit_accepted`; the full-handshake test now sends `ext-info-c` and
  asserts the server replies with EXT_INFO. (Curve25519 KEX + Ed25519 keys, which would let the
  client use its _preferred_ algorithms instead of falling back to group14/rsa, are tracked
  separately on the roadmap.)

---

## SSH channel close packed CHANNEL_EOF + CHANNEL_CLOSE into one binary packet

- **Status:** FIXED (host-tested; `test_ssh_server`).
- **Found:** 2026-06-30, while building the SSH port-forwarding (`direct-tcpip`) close path on top
  of the channel layer.
- **Root cause:** `build_close_chan()` (`ssh_channel.cpp`) frames the channel-close sequence as
  ten bytes - `CHANNEL_EOF` (type + recipient, bytes 0..4) immediately followed by
  `CHANNEL_CLOSE` (type + recipient, bytes 5..9) - in one output buffer, and the only emit site
  (the `SSH_MSG_CHANNEL_CLOSE` case in `ssh_server.cpp`) sent the whole buffer through a single
  `ssh_pkt_send()`, i.e. **two SSH messages in one binary packet**. RFC 4253 6 says a binary
  packet carries exactly one message; a strict client (openssh runs `packet_check_eom()` after
  each message handler) sees five trailing bytes after `CHANNEL_EOF` and disconnects with a
  packet-integrity error, so a channel could not be closed cleanly against openssh. It slipped
  earlier HW checks because those exercised the handshake + channel data, not a strict close.
- **Fix:** emit the two halves as two packets - `emit(buf, 5)` then `emit(buf + 5, 5)` - so each
  is framed, encrypted, and sequence-numbered on its own. The builder is unchanged (its 10-byte
  layout is still the contract); the fix is at the emit boundary, with a comment so it is not
  re-bundled. Regression test `test_inbound_close_emits_eof_then_close_separately` asserts the
  dispatcher emits two packets (EOF then CLOSE).

---

## SonarCloud static-analysis sweep: response-header over-read, crypto zeroization, and friends

- **Status:** FIXED (first SonarCloud C/C++ scan; host-tested).
- **Found:** 2026-06-30, the initial SonarCloud analysis (8 BLOCKER, 7 bug, 7 vulnerability
  findings; the ~2169 "code smell" results are mostly modern-C++ style rules that clash with
  this deliberately terse C++11 / zero-heap embedded style and are not defects).
- **Real issues fixed:**
    - **Dynamic-response header over-read on truncation (`cpp:S3519`,
      `DeterministicESPAsyncWebServer.cpp`).** `append_resp_trailer()` returned `snprintf`'s
      _would-be_ length, which can exceed the header buffer when the trailer (Date + CORS +
      user `_extra_hdr` cookies/headers + Connection) does not fit. The caller then sent
      `(u16_t)hlen` bytes from a fixed `header[RESP_HDR_BUF_SIZE]` - reading past the stack
      buffer. Fixed by clamping `append_resp_trailer()`'s return into `[0, cap-1]` so no send
      path (send / send_empty / redirect / send_template / send_chunked) can ever read past
      the buffer; an over-long header is now sent truncated-but-in-bounds.
    - **Crypto key material not securely zeroed (`cpp:S5798` x5, `services/snmp/snmp_crypto.cpp`).**
      The `memset()`s that wipe the localized key, AES round keys, key stream, and CFB feedback
      at function exit can be elided by the optimizer (dead store on a buffer that is never read
      again), leaving secrets on the stack. Replaced with `snmp_wipe()`, a volatile-loop clear
      the compiler cannot remove (same idiom as `ssh_wipe`).
    - **Uninitialized byte fed to the parser if the ring drains mid-read (`cpp:S836` x2,
      `presentation.cpp`, `websocket.cpp`).** The drain loops ignored `det_conn_read_byte()`'s
      return and fed `byte` to the parser even though a `false` return means nothing was read.
      Now they `break` when the read fails.
    - **`conn_pool[slot_id]` indexed without a bound (`cpp:S3519`).** The public response
      emitters dereferenced `conn_pool[slot_id]` on a caller-supplied id; added a
      `slot_id >= MAX_CONNS` guard at each entry (send / send_empty / redirect / send_template /
      send_chunked).
    - **Misleading constructs:** a `(cond) ? 4 : 4` ternary (`cpp:S3923`, `nats.cpp`) reduced to
      `4`; a `while` that always broke on the first iteration (`cpp:S1751`,
      `http_parser.cpp` Forwarded `proto=`) rewritten as the `if` it actually is; and a
      redundant `if (!app(...)) return len; return len;` (`cpp:S3923`, `webdav.cpp`) simplified
      to `app(...); return len;` (the helper is atomic - it leaves `len` unchanged on no-room).
- **Assessed and intentionally kept (false positives, marked `// NOSONAR` with a reason):**
  `cpp:S5332` "using HTTP is insecure" on `http_client.cpp` (a URL parser in an HTTP **client**
  must accept `http://`; TLS is the caller's choice) and `opcua.cpp` (the OPC UA spec
  transport-profile URI `http://opcfoundation.org/...`, an identifier string that is never
  dereferenced as a URL). The 3 remaining `cpp:S134`/long-switch style notes are correct
  protocol-dispatch switches, left as-is.

---

## WebSocket / SSE upgrade-alloc-fail detached the pcb but never closed it (leak)

- **Status:** FIXED (during the L7 teardown-ownership refactor; host-tested + HW-soaked).
- **Found:** 2026-06-30, auditing the hand-rolled connection teardown sites while routing
  them through one transport-owned API.
- **When a WebSocket or SSE upgrade had already been promoted but the per-protocol slot
  pool was full (`ws_alloc` / `sse_alloc` returned null), the cleanup path called
  `det_conn_detach(pcb)` and reset the slot but never closed or aborted the pcb.** The
  `struct tcp_pcb` was left open with no lwIP callbacks attached: a leaked pcb that lingered
  until lwIP's own timeout, with the application slot already freed. Under upgrade churn
  against an exhausted pool this slowly bled pcbs.
- **Fix:** the teardown is now owned by the transport. `det_conn_abort_slot(slot)` frees the
  TLS context (abrupt), detaches the pcb, resets the slot, and sends a RST; every drop path
  (WS/SSE close + upgrade-fail, `session.cpp` `tls_abort`, SSH/telnet/modbus/opcua) routes
  through it (or `det_conn_close(slot)` for graceful), so a pcb can no longer be detached
  without also being closed. Verified on hardware (COM3): a WS-pool-exhaustion churn drove
  12 `det_conn_abort_slot` RSTs with every slot reclaimed and no pcb/slot leak (the device
  kept accepting throughout). Tracked-debt item closed in [TODO.md](TODO.md).

---

## Test-report generator spewed "tail: write error: Broken pipe" into the CI log

- **Status:** FIXED (host-verified against all 1906 test functions; behavior identical).
- **Found:** 2026-06-30, reading the CI Test Report run log: the report was written fine but
  the job output was flooded with dozens of `tail: write error: Broken pipe` lines.
- **`get_test_comment` in `test/run_tests.sh` extracted a test's doc comment with**
  `tail -n "+${fn_line}" "$abs_file" | head -20 | awk '...'`**.** The `awk` exits at the first
  match (`print; exit`) and `head -20` closes its input after 20 lines, so once the reader is
  gone GNU `tail` gets `EPIPE` on its next write and prints `tail: write error: Broken pipe` to
  stderr. The helper runs once per test function (~1900 calls in a full run), so the CI log
  filled with the message. Not a correctness bug (the report still generated), but noise that
  hides real warnings.
- **Fix:** fold the slice + scan into a single `awk -v start="$fn_line"` pass that reads the
  file directly (`NR < start` skips, `NR >= start + 20` exits), so the early `exit` no longer
  closes a pipe and nothing writes into a dead reader. Verified behavior-identical to the old
  pipeline across every `void test_*()` in `test/test_*/` (1906 functions, 0 diffs, 0 stderr).

---

## Host Link frame builder did not NUL-terminate, so callers read uninitialized memory

- **Status:** FIXED (found by the header-shim migration verification; host-tested).
- **Found:** 2026-06-29, running the full native suite on WSL after migrating the source tree
  to the new `shared_primitives/shim.h` include. `native_hostlink` was the lone failure out of
  103 envs.
- **`hostlink_build` (`services/hostlink/hostlink.cpp`) wrote exactly the wire frame
  (`@UU XX <text> FF * CR`) and returned its length, but never wrote a NUL terminator, while the
  sibling ASCII builder `sdi12_build` does (it reserves `n + 1` and writes `buf[n] = '\0'`).**
  NUL-terminating the ASCII frame is the house convention, so a caller (and the test) that treats
  the returned buffer as a C-string reads off the end into uninitialized stack. It stayed latent
  because the stack byte after the frame happened to be zero; the shim migration shifted the
  binary layout and the byte was now garbage, so `TEST_ASSERT_EQUAL_STRING` saw
  `@00RD0000001057*\r` followed by junk and the test crashed (SIGHUP/ERRORED).
- **Fix:** make `hostlink_build` consistent with `sdi12_build` - reserve room for the terminator
  (`if (total >= cap) return 0`, wrap-safe: no addition) and write `buf[p] = '\0'` after the
  frame. The return value is still the frame length (not counting the NUL), so callers may treat
  `buf` as either a sized buffer or a C-string. The existing `native_hostlink` tests now pass
  7/7; the overflow test (`char small[8]`) still fails closed.

---

## Two more 32-bit length-wrap bounds bypasses: SNMP BER decoder + HTTP chunked decode

- **Status:** FIXED (follow-up bug-hunt; host-tested, incl. a regression that crashes the
  pre-fix code even on a 64-bit host).
- **Found:** 2026-06-29, a second bug-hunt sweeping the wrap class across the parsers the first
  pass did not cover.
- **The same 32-bit `size_t` wrap as the codec entry below, in two network-facing parsers the
  first pass missed:**
    - **SNMP BER decoder (`ber_read_header`, `services/snmp/snmp_ber.cpp`).** A long-form ASN.1
      length is read as a full 4-octet value (up to `0xFFFFFFFF`), then checked with
      `d->pos + length_val > d->len`. On the 32-bit ESP32 an attacker-supplied length of
      `0xFFFFFFFF` makes `d->pos + length_val` wrap to `d->pos - 1` (`< d->len`), so the bound
      is bypassed and a bogus huge length is returned; downstream `ber_read_oid`'s `i < len`
      loop then reads out of bounds. SNMP is an attacker-reachable UDP agent. `ber_skip` had the
      same pattern. Fix: compare against the remaining capacity (`length_val > d->len - d->pos`,
      valid because `d->pos <= d->len` holds after the count-byte check).
    - **HTTP client chunked decode (`http_client_parse_response`, `services/http_client/http_client.cpp`).**
      The hex chunk size `csz` is accumulated unbounded from the response, then clamped with
      `if (in + csz > len)`. A malicious / MITM'd server sending an oversized chunk size makes
      `in + csz` wrap below `len`, skipping the clamp and leaving a multi-gigabyte `memmove`. Fix:
      `if (csz > len - in)` (wrap-safe; `in <= len` here).
- The 64-bit host never wrapped on the 4-octet BER case, so it stayed latent; the HTTP
  regression uses a 16-hex-digit size (`0xFFFFFFFFFFFFFFFF`) that wraps a 64-bit `size_t` too,
  so it crashes the old code on the host and proves the fix. Tests added to test_snmp_ber and
  test_http_client.
- **Related hardening (same sweep):** audited the rest of the wrap class clean - `opcua`
  (`r_skip`/`ua_r_string` lengths are `int32`, capped at `0x7FFFFFFF`, and all callers guard
  `> 0`), `protobuf` (`PB_WT_LEN` compares in `uint64_t`; varints reject `> 10` bytes), `lwm2m`
  (24-bit length bounded), and `wamp_get_uri` (`body + 1 > out_cap`) are all correctly bounded.

---

## Codec length fields could wrap the bounds check on a 32-bit target

- **Status:** FIXED (multi-agent codebase audit; host-tested).
- **Found:** 2026-06-29, the codec bug-hunt audit of the v4.x protocol codecs.
- **Three text/binary parsers computed `overhead + declared_length` before comparing to the
  buffer length, which wraps on a 32-bit `size_t` (the ESP32 target) for an attacker-controlled
  length field — so the `> len` guard could falsely pass and hand the caller an out-of-bounds
  slice.** Affected: `amqp_parse_frame` (AMQP 0-9-1 32-bit size, `services/amqp/amqp.cpp`),
  `nats_parse` MSG byte count (`services/nats/nats.cpp`), and `resp_parse` `$` bulk length
  (`services/redis_resp.cpp`). The 64-bit host tests never exercised the wrap, so it was latent.
  Fix: in each, compare the declared length against the _remaining capacity_ (`len - overhead`)
  without adding, so no addition can wrap. Added oversized-length rejection tests to
  test_amqp / test_nats / test_redis_resp.
- **Related hardening in the same pass:** `stomp` `content-length` parse now rejects on overflow
  AND a present-but-invalid `content-length` is a malformed frame (previously it silently
  fell back to NUL-delimited body parsing - a request-smuggling-style differential); `parse_len`
  caps at `SIZE_MAX`. `flow_export` IPFIX `finish` now fails closed when the message exceeds the
  16-bit length field instead of truncating it. Tests added/updated accordingly.

---

## det_client.cpp failed to compile on a server-only Arduino build

- **Status:** FIXED (found while building the interop rig; HW build verified on COM3).
- **Found:** 2026-06-29, bringing up the real-protocol interop harness (test/servers).
- **A server-only Arduino build (no HTTP client / MQTT / WS client) did not compile
  (build break).** `det_client.cpp` guarded its body with only `#if defined(ARDUINO)` yet
  calls `detws_dns_resolve()`, whose declaration lives behind `DETWS_ENABLE_DNS_RESOLVER`.
  That flag is force-enabled only by a client transport (`HTTP_CLIENT || MQTT || WS_CLIENT`,
  DetWebServerConfig.h), so a build that enabled, e.g., WebSocket + SNMP + CoAP + Modbus but
  no client left the symbol undeclared: `error: 'detws_dns_resolve' was not declared in this
scope`. Host builds were unaffected (the body is already `#if defined(ARDUINO)`), so the
  native suites never caught it. Fix: add `DETWS_NEED_DET_CLIENT` (set alongside the
  DNS-resolver force-enable) and gate the translation unit on
  `#if defined(ARDUINO) && DETWS_NEED_DET_CLIENT`, so det_client compiles exactly when a
  client transport needs it. Verified: a WebSocket+SNMP+CoAP+Modbus rig now builds and
  flashes, and the interop harness drives all four against real peers.

## Standards-conformance audit, batch 2e (LOW/SHOULD closeout)

- **Status:** FIXED (standards-conformance audit; host + HW-verified on COM3)
- **Found:** 2026-06-29, multi-agent conformance audit (the batch-2b "Still OPEN" list).
- **Closes the remaining LOW/SHOULD items so the audit is 100% addressed.**
- **WebSocket handshake accepted a malformed upgrade (LOW, RFC 6455 4.2.1).** The upgrade
  test did not require an `upgrade` token in the `Connection` header, nor that
  `Sec-WebSocket-Key` base64-decode to 16 bytes. Both are now required; a bad handshake
  gets `400` and no upgrade. Tests: `test_web_terminal`
  `test_ws_upgrade_requires_connection_token`, `test_ws_upgrade_rejects_bad_key_length`.
- **MQTT topic validation (LOW, MQTT-3.3.2-2 / 1.5.3).** `mqtt_build_publish` now rejects a
  Topic Name containing wildcards (`+`/`#`); `mqtt_parse_publish` rejects a Topic Name that
  is not well-formed UTF-8 or contains U+0000. The UTF-8 validator was extracted to a shared
  primitive (`shared_primitives/det_utf8.h`) and reused by WebSocket (no duplicate copy).
  Tests: `test_mqtt` `test_publish_wildcard_topic_rejected`,
  `test_publish_topic_nul_or_bad_utf8_rejected`.
- **base64url decoder was lenient (LOW, RFC 4648 5 / RFC 7515).** `base64url_decode` also
  accepted the standard `+`/`/` alphabet, contradicting its name and the JWS base64url
  contract. Now strict (URL alphabet only); no caller fed it standard base64 (OIDC `n`/`e`
  are Base64urlUInt, not `x5c`). Not a signature-bypass vector (JWS signs the literal
  transmitted ASCII), hence LOW. Test: `test_jwt` `test_base64url_strict_alphabet`.
- **Digest nonce never rotated; no replay window bound (SHOULD, RFC 7616 3.3).** The server
  used a single fixed nonce regenerated only at `begin()`, so a captured Digest response
  could be replayed indefinitely. Replaced with a stateless, keyed, timestamped nonce
  (`<issue_ms_hex>.<SHA-256(secret||issue) truncated>`): no per-nonce table (compatible with
  the shared-nothing worker model), bounded lifetime (`DETWS_DIGEST_NONCE_LIFETIME_MS`,
  default 5 min), and an expired-but-valid response is answered `401 stale=true` for a
  transparent retry (not counted against the lockout). Full per-nonce `nc` replay tracking
  remains intentionally out of scope: it requires shared mutable per-nonce state the
  deterministic worker model cannot hold safely; the bounded-lifetime nonce is the standard
  stateless mitigation. Tests: `test_digest_auth` `test_nonce_is_stateless_timestamped`,
  `test_stale_nonce_triggers_transparent_retry`.
- **SNMP v2c GET returned noSuchObject for a missing instance (LOW, RFC 3416 4.2.1).** A
  Get for an unbound name always reported `noSuchObject`. Now distinguishes
  `noSuchInstance` (the name's object-type prefix matches a registered object, only the
  instance is absent) from `noSuchObject` (no such object at all). Test: `test_snmp_agent`
  `test_get_bad_instance_v2c_nosuchinstance`.
- **HTTP chunked was sent to HTTP/1.0 clients (MED-niche, RFC 7230 3.3.1).** `send_chunked`
  always emitted `Transfer-Encoding: chunked`, which is invalid for a 1.0 client. It now
  falls back to a close-delimited body (no Transfer-Encoding, `Connection: close`, raw bytes
  paged across loops, end signalled by the connection close) when the request is not
  HTTP/1.1. Tests: `test_chunked` `test_http10_falls_back_to_close_delimited`,
  `test_http10_large_body_not_truncated`. HW: `--http1.0 /stream` -> `HTTP/1.0 200` +
  `Connection: close`, body intact; `--http1.1` -> chunked.

## Standards-conformance audit, batch 2d (WebDAV PROPFIND Depth: infinity)

- **Status:** FIXED (standards-conformance audit; HW-verified)
- **Found:** 2026-06-29, multi-agent conformance audit (WebDAV).
- **PROPFIND `Depth: infinity` was silently truncated to one level (MED, RFC 4918
  9.1.1).** A collection PROPFIND with `Depth: infinity` returned a one-level 207 the
  client would read as a complete tree. The server lists at most one level, so it now
  rejects infinity with `403` + the `<D:propfind-finite-depth/>` precondition body;
  `Depth: 0`/`1` are unchanged. HW: `Depth: infinity` -> 403 (precondition body present),
  `Depth: 1` -> 207.

## Standards-conformance audit, batch 2c (HTTP If-None-Match comparison)

- **Status:** FIXED (standards-conformance audit)
- **Found:** 2026-06-29, multi-agent conformance audit (HTTP semantics).
- **If-None-Match used exact strong `strcmp` (MED, RFC 9110 13.1.2).** Conditional GET
  only matched a single, byte-identical strong tag, so it ignored `*`, a comma-separated
  tag list, and the mandated weak comparison (an inbound `W/"x"` for our strong `"x"`).
  A standards-compliant cache revalidating with `W/` tags, a list, or `*` got a full 200
  instead of 304. Fix: `inm_matches()` handles `*` (matches the current representation),
  splits a list, and weak-compares (ignores a `W/` prefix). Test: `test_application`
  `test_serve_static_inm_star_list_weak` (`*` / `W/"x"` / list-with-tag -> 304;
  list-without-tag -> 200).

## Standards-conformance audit, batch 2b (WS UTF-8, SSH padding, syslog PRI, BER OID)

- **Status:** FIXED (standards-conformance audit)
- **Found:** 2026-06-29, multi-agent conformance audit.
- **WebSocket: TEXT messages were not UTF-8-validated (MED, RFC 6455 8.1).** An invalid-
  UTF-8 text message was delivered to the app instead of failing the connection. Fix:
  validate the fully reassembled + decompressed TEXT message and close 1007
  (WS_CLOSE_INVALID_PAYLOAD) on invalid UTF-8 (strict: rejects overlong / surrogate /
  out-of-range / truncated). BINARY frames are not validated. Tests: test_websocket.
- **SSH: padding_length < 4 was not rejected (LOW, RFC 4253 6).** The receive path only
  checked padding < packet length. Now also rejects padding < 4 (gated behind MAC
  verification, so a robustness gap, not a vulnerability).
- **syslog: PRI not range-bounded (LOW, RFC 5424 6.2.1).** An out-of-range caller
  facility/severity could emit a malformed PRI. Now clamped to 0..191.
- **SNMP/BER: OID decoder mishandled a first subidentifier >= 128 (LOW, X.690 8.19.4).**
  The first subidentifier (40*arc0 + arc1) is base-128 and may span octets; the decoder
  read only one octet (encoder was already correct). Fixed; common OIDs (1.3.6.1...,
  first subid 43) decode identically. Test: test_oid_large_first_subidentifier_roundtrip.
- **These LOW/SHOULD items are now CLOSED in batch 2e (above):** HTTP chunked to non-1.1
  clients, SNMP v2c noSuchInstance vs noSuchObject, Digest nonce rotation (stateless
  timestamped nonce; full nc-replay tracking intentionally out of scope), stricter
  base64url, WS handshake Connection:Upgrade + key-length check, MQTT topic UTF-8/wildcard.

## Standards-conformance audit, batch 2a (auth: JWT alg, Digest uri)

- **Status:** FIXED (found by the auth/crypto conformance audit)
- **Found:** 2026-06-29, multi-agent conformance audit (auth module).
- **JWT: the `alg` header was never validated (RFC 7515 5.2, MED).** `jwt_verify_hs256`
  computed HMAC-SHA256 and compared without checking the token's declared algorithm, so
  a token whose header said `none` / `RS256` / `HS384` was accepted as long as its
  signature equaled base64url(HMAC-SHA256(secret, signing_input)) - an algorithm-
  substitution hazard (not directly exploitable since the HMAC is still enforced and
  empty-sig `none` fails the length gate, hence MED). Fix: decode the header and require
  `alg":"HS256"` before verifying. Test: `test_jwt` `test_alg_not_hs256_rejected` (a
  valid-HMAC token with alg `none` is rejected).
- **Digest: the `uri` parameter was not matched to the request target (RFC 7616 3.4,
  MED).** `check_digest_auth` folded the client-supplied `uri` into HA2 but never
  compared it to the actual request target, so a Digest response captured for route A
  was structurally valid against any route under the same realm/nonce. Fix: reconstruct
  the request target (`path[?query]`) and require it equals `uri`. Test:
  `test_digest_auth` `test_uri_mismatch_rejected`.
- Audited clean: Basic auth (first-colon split), OIDC RS256 (strict alg/exp/nbf/iss/aud
    - constant-time RSA block compare), TOTP/HOTP (RFC 4226 truncation), OAuth2 client,
      constant-time HMAC compare. Noted for later (SHOULD/LOW): Digest nonce rotation + `nc`
      replay tracking; stricter base64url; constant-time Digest response compare.

## Standards-conformance audit, batch 1 (WS / MQTT / CoAP / Telnet)

- **Status:** FIXED (found by the parallel standards-conformance audit; specs read from
  the downloaded RFC texts, mapped in docs/STANDARDS.md)
- **Found:** 2026-06-29, multi-agent conformance audit against the live specs.
- Five real conformance gaps, fixed with tests:
    - **WebSocket close left the TCP socket open (HIGH, RFC 6455 5.5.1).** The plaintext
      WS close/error path (DeterministicESPAsyncWebServer.cpp) only freed the WS slot and
      `http_reset`'d it - it never closed the TCP connection (the TLS path did). The slot
      stayed CONN_ACTIVE and re-armed as an HTTP parser, so bytes after the Close frame
      were re-interpreted as a new HTTP request (state confusion). Fix: `det_conn_begin_close`
      on the slot so it leaves CONN_ACTIVE (the queued Close frame still flushes).
    - **MQTT PUBLISH QoS=3 accepted (HIGH, MQTT-3.3.1-4 / 4.8.0-1).** A PUBLISH with both
      QoS bits set was treated as QoS 2; the spec says it is malformed and the receiver
      MUST close the connection. Fix: `mqtt_parse_publish` rejects qos==3; the handler
      `mq_close()`s on a malformed PUBLISH.
    - **CoAP unsupported method returned 5.01 (MED, RFC 7252 5.8).** Must be 4.05 Method
      Not Allowed. Fixed.
    - **CoAP unrecognized critical option silently ignored (MED, RFC 7252 5.4.1).** An
      unknown odd-numbered (critical) option must yield 4.02 Bad Option (so e.g. Accept,
      or Block when COAP_BLOCK is off, is rejected, not ignored). Fixed.
    - **Telnet literal IAC (0xFF) in output not doubled (MED, RFC 854).** Echoed/printed
      0xFF bytes were sent un-doubled, desyncing the client's command stream. Fix: a
      `send_escaped` data path doubles IAC for echo + app output (protocol commands still
      use the raw path).
    - SNMP/BER, SSH, WebDAV, syslog, OIDC/TOTP/Basic-auth/OAuth2 audited clean.

## CoAP Observe used millis() (would not build on host + pluggable-clock violation)

- **Status:** FIXED (found by the test-gap hardening pass)
- **Found:** 2026-06-29, adding a CI env that compiles the Observe-gated code.
- **Symptom:** `coap_notify()` (under `DETWS_ENABLE_COAP_OBSERVE`) built the notification
  message-id with `millis()` and pulled `<Arduino.h>` for it. The flag was enabled by no
  test env, so the code had never been compiled on host - it failed to build there
  (`'millis' was not declared`), and even on ESP32 it violated the pluggable-clock rule
  that `detws_millis()` is the single monotonic source (same class as the dns_resolver
  bug above). Latent because the whole Observe path was never compiled in CI.
- **Fix:** use `detws_millis()` (include `services/det_clock.h`), drop the Arduino.h
  include; added a `native_coap_observe` env so the Observe-gated code is compiled + the
  CoAP suite runs under the flag in CI (no longer bit-rots).

## HTTP request smuggling: Transfer-Encoding ignored on inbound requests

- **Status:** FIXED (found by the test-gap hardening pass)
- **Found:** 2026-06-29, request-parser test-gap review.
- **Symptom:** the HTTP parser only framed request bodies by `Content-Length` and
  ignored `Transfer-Encoding` entirely. A `Transfer-Encoding: chunked` request with no
  `Content-Length` was treated as body-length-0, so the chunk octets
  (`5\r\nhello\r\n0\r\n\r\n`) were left in the RX buffer and re-parsed as the next request
    - a classic TE request-smuggling / desync vector on a keep-alive connection (and the
      CL+TE combination is the canonical CL.TE desync).
- **Fix:** the server does not decode chunked request bodies, so reject any request
  bearing `Transfer-Encoding` -> `PARSE_ERROR` (400), fail-closed, matching the existing
  conflicting-`Content-Length` rejection (RFC 9112 §6.1/§6.3). Tests:
  `test_compliance` `test_transfer_encoding_chunked_rejected` /
  `_with_content_length_rejected` / `_case_insensitive_rejected`.

## Byte-range parser integer overflow on a huge Range value

- **Status:** FIXED (v4.9.1; found by the edge-case audit)
- **Found:** 2026-06-28, agent edge-case test-gap audit.
- **Symptom (latent):** `parse_byte_range` accumulated `start/end = *10 + digit` with no
  overflow guard, so a `Range: bytes=99999999999999999999999-` wraps `size_t` to a small
  value that can pass the `start >= size` check and yield a wrong `206` window.
- **Fix:** saturate the accumulator at `SIZE_MAX` on overflow, so a huge start is treated
  as past-EOF (416) and a huge end clamps to the last byte - never a corrupt window.

## If-Modified-Since month token could mis-parse (off-by-alignment)

- **Status:** FIXED (v4.9.1; in v4.9.0's just-shipped conditional-GET code)
- **Found:** 2026-06-28, agent edge-case test-gap audit.
- **Symptom (latent):** `http_not_modified_since` matched the month via
  `strstr(MONTHS, mon)` without checking the match offset is a multiple of 3, so a
  malformed token like `ebM` (which appears inside "FebMar") parsed as a valid month ->
  a wrong 304/200 cache decision on malformed input. No memory-safety impact.
- **Fix:** reject the match unless `(mp - MONTHS) % 3 == 0`.

## DNS resolver ignored the pluggable clock

- **Status:** FIXED (v4.9.1; found by the duplicate-code audit)
- **Found:** 2026-06-28, agent services duplicate-code audit.
- **Symptom:** `services/dns_resolver` polled its resolve deadline with `millis()` instead
  of `detws_millis()`, so it ignored a custom clock (`detws_set_clock`) - violating the
  pluggable-clock rule that `detws_millis()` is the single monotonic source.
- **Fix:** use `detws_millis()`. (The bigger dedup, det_client reusing this one resolver,
  shipped later as the shared-primitive DNS-owner change.)

## Client ring used `volatile` indices (weak cross-core ordering)

- **Status:** FIXED (v4.8.1; found by analysis while unifying the ring primitive)
- **Found:** 2026-06-28, merging the server/client ring implementations.
- **Symptom (latent):** the outbound client ring (`det_client`) used `volatile size_t`
  head/tail. `volatile` blocks compiler reordering but provides no cross-core
  acquire/release; on the dual-core ESP32 (producer = tcpip_thread, consumer = caller
  loop, often different cores) the consumer could observe an advanced `head` before
  the buffer bytes it published were visible -> a rare stale read. The server ring
  already used `DetAtomic` (correct); the client was inconsistent.
- **Fix:** both transports now share `det_ring.h` (the `DetAtomic` SPSC index +
  the drain math); the client ring's indices are `DetAtomic`, matching the server's
  acquire/release ordering. One ring primitive, no hand-rolled wrap/ordering.

## Client transport could deadlock on a large inbound transfer

- **Status:** FIXED (v4.8.0; same fix as the server, found by analysis during the
  unified-client-transport pass)
- **Found:** 2026-06-28, reviewing `det_client` against the server's ack-on-consume fix.
- **Symptom (latent):** an outbound client (http_client / mqtt / ws_client) reading a
  response larger than the client ring could stall - the same class as the server
  deadlock below, in the other direction.
- **Root cause:** `det_client`'s recv callback ACKed on copy (`tcp_recved` in
  `cc_recv`) with a 4 KB ring (< TCP_WND), so a sustained inbound transfer could fill
  the ring, get refused (lwIP `refused_data`), and race.
- **Fix:** ack-on-consume on the client transport too - `cc_recv` no longer ACKs;
  `det_client_read()` marshals `tcp_recved()` for the bytes it drained. Default
  `DETWS_CLIENT_RX_BUF` raised 4096 -> 8192 (>= TCP_WND). Client and server transports
  now share one flow-control model.
- **HW proof:** device `http_get()` of a 12 KB body (> the 8 KB client ring, so it
  wraps the ring and exercises the ack-on-consume read path) returned the full
  `len:12000`, 5/5 - no truncation or deadlock.

## RX flow-control deadlock on streamed uploads (WebDAV PUT)

- **Status:** FIXED (v4.6.0; host + HW validated)
- **Found:** 2026-06-28, stress-testing WebDAV streaming PUT on hardware.
- **Symptom:** large PUTs intermittently hung ~20 s (curl timeout), stored 0 bytes,
  and repeated hangs eventually wedged every slot (device pings but HTTP is dead).
- **Root cause:** `recv_cb` ACKed received data on **copy** (`tcp_recved` at copy
  time), decoupling the advertised TCP window from how full the ring actually was.
  A slow consumer (flash writes) let the ring fill to `RX_BUF_SIZE`; the next
  segment was refused (`ERR_MEM` -> lwIP `refused_data`). When `RX_BUF_SIZE <
TCP_WND` the ring can never hold a full receive window, so refusals were constant
  and lwIP's refused-data redelivery raced fatally. Serial trace nailed it: failing
  PUTs stalled at exactly `bytes_written == RX_BUF_SIZE`. This was an **interlayer**
  bug: the receive-window invariant was smeared across transport (ACK on copy),
  presentation (drains the ring) and session (worker loop), with no single owner.
- **Fix:** ack-on-consume, owned entirely by transport. `recv_cb` no longer ACKs;
  the worker calls `det_conn_ack_consumed(slot)` once per loop and transport
  reopens the window by exactly the bytes drained since the last ACK
  (`tcp_recved` marshaled to tcpip_thread). The window now tracks ring occupancy,
  so a slow sink cannot overflow the ring. **TCP-level requirement:** the ring must
  hold at least one receive window (`RX_BUF_SIZE >= TCP_WND`); a smaller ring is a
  configuration error (you cannot advertise a window larger than your buffer).
- **HW proof:** RX=8192 (>= TCP_WND) + ack-on-consume -> 10/10 50 KB byte-exact,
  `backpressure=0`. Pre-fix: RX=2048 -> ~40% hang + permanent wedge.

## Boot stack-overflow when RX_BUF_SIZE is large (pool_init)

- **Status:** FIXED (v4.6.0; host validated)
- **Found:** 2026-06-28, while testing large rings for the deadlock above.
- **Symptom:** "Stack canary watchpoint triggered (loopTask)" at boot
  (`begin()` -> `pool_init` -> `memset`) once `RX_BUF_SIZE` was set large (e.g. 8192).
- **Root cause:** `conn_pool[i] = {}` materializes a full `sizeof(TcpConn)`
  temporary - the entire `rx_buffer[RX_BUF_SIZE]` - on the loopTask stack.
- **Fix:** reset from a single `static const TcpConn blank = {}` in BSS via
  copy-assign (uses `DetAtomic::operator=`, no atomic-memset UB); no large stack
  temporary.

## WebDAV streamed PUT leaks the file handle on abort

- **Status:** FIXED (v4.6.0)
- **Found:** 2026-06-28, investigating the deadlock cascade to a permanent wedge.
- **Symptom:** a PUT torn down before completion (peer reset / timeout / the
  deadlock above) never closed `g_dav_put_file`; after a few, LittleFS ran out of
  open-file slots and `open()` failed ("no permits for creation").
- **Root cause:** the file was closed only on the PARSE_COMPLETE handler path; no
  cleanup hook for an aborted stream.
- **Fix:** added `HttpStreamAbortCb` to the parser stream hooks, fired from
  `http_parser_reset` when `body_streaming && parse_state != PARSE_COMPLETE`.
  WebDAV registers it and closes the half-written file. The completion path now
  also clears `g_dav_put_active` so the abort hook cannot double-close.

## WebDAV concurrent streamed PUTs clobber each other

- **Status:** FIXED (v4.7.0; host + HW validated)
- **Found:** 2026-06-28, concurrent (4x) 50 KB PUT stress test.
- **Symptom:** 4 simultaneous PUTs -> one 201, the rest 409/timeout (the file was
  not actually written for the losers).
- **Root cause:** the WebDAV streaming-PUT state (`g_dav_put_file/active/error/...`)
  was a single global, and `HttpStreamDataCb` carried no slot/connection, so the
  data hook could not route bytes to per-connection state. Overlapping PUTs shared
  and clobbered the one global transfer.
- **Fix:** made the streaming-body `data` hook slot-aware (`HttpStreamDataCb(HttpReq*,
...)`; `begin`/`abort` already had it) and replaced the global state with per-slot
  `g_dav_put[MAX_CONNS]`, so each connection streams to its own file. Aligns with the
  no-spaghetti directive (the data hook lacking connection context was the interlayer
  gap). HW: 4 concurrent PUTs with distinct payloads, all 4 byte-exact (was 0/4).

## TX truncation on large responses (serve_file, send_chunked)

- **Status:** SHIPPED (serve_file v4.4.1; send_chunked v4.5.0)
- **Found:** 2026-06-28, stress-testing file/chunked GET on hardware.
- **Symptom:** any file/WebDAV GET or chunked/SSE body larger than ~`TCP_SND_BUF`
  (~5.7 KB) was truncated.
- **Root cause:** the senders called `det_conn_send()` and ignored the return; once
  the TCP send buffer filled, the remainder was silently dropped. Hidden on host
  because the mock `tcp_sndbuf` is constant and `tcp_write` never returns `ERR_MEM`.
- **Fix:** per-slot send continuations (`file_send_pump` / `chunk_send_pump`) that
  page out one send-window per worker loop and resume on the sent callback; the
  chunked API became a pull generator (`ChunkSource`) so it can resume across loops.
