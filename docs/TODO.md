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

## HTTP / core (medium)

- [ ] **No TLS (HTTPS).** Plain HTTP only; relies on a trusted LAN, a TLS
      terminator, or the SSH channel. A real fix is large (mbedTLS TLS server).

- [ ] **`Date` response header not emitted.** Acceptable for a clock-less device
      (RFC 7231 §7.1.1.2); revisit if an RTC/NTP time source is available.

- [ ] **Recv scratch on the stack.** `ssh_pkt_recv` uses a
      `SSH_PKT_BUF_SIZE + 32` (~2 KB) stack buffer; size the ESP32 SSH task stack
      accordingly or move it to BSS.

## Housekeeping (low)

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
