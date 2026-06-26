# Known limitations

Deliberate constraints and caveats of DeterministicESPAsyncWebServer. Most stem
from the zero-heap / fixed-buffer / single-loop design and are intentional; where
a limit is configurable, the relevant `*_SIZE` / `*_MAX` macro is named. Work to
lift some of these is tracked in [ROADMAP.md](ROADMAP.md).

## Platform

- **ESP32 only.** No ESP8266 / RP2040 / RP2350 port - the HW crypto, NVS, and
  lwIP raw-API integration are ESP32-specific. Host builds exist only for tests.
- **IPv4 only.** No IPv6 dual-stack yet, and Wi-Fi STA/AP only (no Ethernet PHY).

## HTTP core

- **Fixed request limits:** `MAX_HEADERS` headers; `MAX_KEY_LEN` / `MAX_VAL_LEN`
  per header (over-long keys/values are truncated, not rejected - except the
  `Authorization` value, which has its own larger buffer); `MAX_PATH_LEN` path;
  `BODY_BUF_SIZE` body (larger gets 413 unless a streaming sink is installed).
- **No `Date` response header by default** (a clock-less device; RFC 7231
  6.5.2). With `DETWS_ENABLE_NTP` an app can add it from a handler.
- **Chunked responses have no `tcp_sndbuf()` backpressure check** - very large
  streams under load may need a per-chunk send-window check.

## WebSocket

- A reassembled message must fit `WS_FRAME_SIZE` (else Close 1009).
- **permessage-deflate is inbound-only** (RFC 7692): the server decompresses
  compressed messages but sends its own uncompressed (outbound compression is on
  the roadmap). No other extensions; RSV2/RSV3 are always rejected.

## JSON

- The reader decodes `\uXXXX` escapes only for code points <= 0xFF; higher code
  points and UTF-16 surrogate pairs become `?` (no multi-byte UTF-8 emission).
- `JsonWriter` is bounded by the caller buffer + `JSON_MAX_DEPTH`; overflow flips
  `ok()` and truncates rather than growing.

## Authentication

- **HTTP Digest:** no `nc` (nonce-count) replay tracking - a single shared server
  nonce suits this 1-2 client device class; the per-`begin()` nonce rotation
  bounds the replay window.

## TLS

- **One TLS connection at a time** (`MAX_TLS_CONNS` = 1): the per-connection
  mbedTLS arena (~41.5 KB) overflows DRAM at 2 on a stock Arduino build (a
  smaller-record ESP-IDF build is needed to raise it).
- **Server-side session resumption only** (RFC 5077 tickets); the outbound client
  does not yet present a ticket on reconnect.

## SSH

- **Single `session` channel** - no port-forwarding, X11, or channel multiplexing.
- **No rekeying** - the connection closes when a sequence number would wrap (the
  safe fallback); there is no data-volume / time-based rekey.
- Per-direction NEWKEYS is a single flag (correct for the strict
  send-then-receive ordering), and the KDF produces a single <= 32-byte block
  (enough for the negotiated AES-256 / HMAC-SHA-256).

## Protocol services

- **CoAP:** piggybacked responses only - no separate (deferred) responses, no CON
  retransmission / de-duplication, no `/.well-known/core` discovery.
- **WebDAV:** `PROPPATCH` is unsupported (405); `PUT` buffers to `BODY_BUF_SIZE`
  (no streaming large uploads); `COPY` handles files only (collection copy -> 501).
- **SNMP:** the v3 *inform* is not implemented (the v3 trap is); the engine ID
  uses a fixed placeholder enterprise OID.
- **Telnet** is plaintext - no auth or encryption; use it only on a trusted LAN
  (prefer SSH or the WebSocket terminal otherwise).
- **Multipart:** at most `MAX_MULTIPART_PARTS` parts; a binary part containing the
  boundary bytes is truncated; only `name` / `filename` are extracted.

## Streaming sinks

- **OTA and streaming upload share the single parser streaming hook** - enable at
  most one of `DETWS_ENABLE_OTA` / `DETWS_ENABLE_UPLOAD` per build.
