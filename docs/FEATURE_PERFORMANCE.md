# Feature performance

Empirical performance of the library's features, to judge **real-world viability** on device - not a
code-quality claim, but measured numbers. This is a living document: rows are filled as features are
benchmarked. The code is written to be efficient by construction (zero heap, fixed buffers, single loop);
this page proves what that costs in time.

## Methodology

Two figures per operation:

- **Host ns/op** - a deterministic relative baseline measured on a fast core (Raspberry Pi 5, `-O2`).
  It is _not_ the device cost; it exists to compare features against each other and to catch
  regressions cheaply. Harness: [`perf/host_microbench.cpp`](../perf/host_microbench.cpp)
  (`std::chrono`, time N iterations, report ns/op + MB/s).
- **ESP32-S3 us/op @ 240 MHz** - the real-world cost, measured on device with the CCOUNT cycle counter
  / `esp_timer`, printed over serial. This is the number that decides viability.

Where an operation moves bytes, throughput (MB/s) is reported too. Whole request paths (HTTP parse, a
JSON response render, a TLS handshake, an SSH KEX) are measured end to end, not only the micro-codecs -
viability is about latency and sustained throughput under load, not a single call in isolation.

## 1. Storage characterization (first priority - pending SD card)

Before the on-device data stores (dbm / sqlite / nosql) and their atomic buffer-to-flash layer are
built, the storage device itself is characterized, so the buffering layer knows **what I/O sizes and
queue depths to batch**. The write path's atomicity design (write-ahead journal vs A/B page swap) and
its batch/queue sizing follow directly from the measured IOPS/latency curve.

Test matrix (to run on the ESP32-S3 with the SD card attached, over SDMMC and/or SPI):

| Dimension   | Sweep                                                         |
| ----------- | ------------------------------------------------------------- |
| Block size  | 512 B, 1 KiB, 4 KiB, 16 KiB, 64 KiB                           |
| Pattern     | sequential vs random                                          |
| Direction   | read vs write (and read-modify-write for the journal case)    |
| Queue depth | 1, then batched (to find the depth where throughput plateaus) |
| Sync        | buffered vs `fsync`/flush-per-op (the durability cost)        |

Metrics captured per cell: **IOPS**, **latency** (avg + p99), **throughput (MB/s)**, and the
flush/`fsync` cost (the price of durability). The resulting curve sizes the atomic layer's write batch
(the block size at the throughput knee) and its queue depth (where more in-flight I/O stops helping).

> Measured 2026-07-09 on the COM4 **ESP32-S3** (SD-over-SPI on HSPI: SCK=12, MISO=13, MOSI=11, CS=10)
> with a **~32 GB SDHC** card, FAT32. Each round trip byte-verified. (SDMMC 4-bit is not wired on this
> board, so these are the SPI-mode numbers - SDMMC would be several times faster.)

### Throughput vs SPI clock (sustained sequential)

| SPI clock | Seq write MB/s | Seq read MB/s | Max write latency | Verify |
| --------- | -------------: | ------------: | ----------------: | ------ |
| 4 MHz     |           0.44 |          0.42 |            168 ms | OK     |
| 8 MHz     |           0.82 |          0.75 |             53 ms | OK     |
| 16 MHz    |           1.26 |          1.22 |            127 ms | OK     |
| 20 MHz    |       **1.55** |          1.39 |            119 ms | OK     |
| 26.7 MHz  |           1.47 |          1.40 |            117 ms | OK     |
| 40 MHz    |           1.56 |          1.40 |            117 ms | OK     |

**Throughput scales with the clock up to ~20 MHz, then plateaus at ~1.5 MB/s write / ~1.4 MB/s read** -
above 20 MHz the SD **card / filesystem is the bottleneck, not the SPI bus**. So ~20-26 MHz is the
efficient operating point; 40 MHz buys almost nothing and only raises EMI / wiring sensitivity.
Sustained is flat (a 139 MB run held ~1.4-1.5 MB/s with no SLC-cache knee).

### Random I/O (durable = flush per op, at 40 MHz)

| Pattern     |      IOPS | Avg latency | Max latency |      MB/s |
| ----------- | --------: | ----------: | ----------: | --------: |
| 4 KiB write |  37-87 \* |       ~8 ms |     ~107 ms | 0.15-0.36 |
| 4 KiB read  |  31-93 \* |       ~3 ms |     ~3.7 ms | 0.13-0.38 |
| 512 B write | 39-105 \* |     ~6-8 ms |     ~106 ms |         - |

\* Durable random-**write** IOPS varied 2-3x run to run (the card's internal state / garbage collection).
Reads are tight and predictable (~3 ms); durable writes carry **100+ ms tail-latency spikes** from SD
internal housekeeping, and those spikes are **clock-independent** (present at every SPI clock).

### Implications for the atomic buffer-to-flash layer

- **Run SPI at ~20-26 MHz** - past that the card, not the bus, is the limit.
- **Batch into large sequential writes.** Sequential ~1.5 MB/s vs durable random ~40-100 IOPS
  (0.15-0.38 MB/s) is a **10-40x** gap. A write-ahead journal (append sequentially, then checkpoint in
  bulk) matches the hardware; scattered small durable writes fight it.
- **Absorb the 100+ ms write tail.** Any single durable write can stall 100+ ms (card GC), so the
  layer's queue must be deep enough to hide the spike or commit asynchronously.
- 16-32 KiB is a good journal block; 512 B durable writes are the worst case.

### Batch / write-size sweep (sizing the journal + queue)

Measured at 20 MHz. **(A)** how larger _durable_ writes (one flush each) amortize the flush cost, and
**(B)** how _batching_ small (4 KiB) writes behind one flush does the same - the two knobs that size the
journal record and the queue depth.

**A. Durable write size (one flush per write)**

| Write size |  MB/s | IOPS | Avg latency | Max latency |
| ---------- | ----: | ---: | ----------: | ----------: |
| 512 B      | 0.112 |  218 |      4.6 ms |      104 ms |
| 1 KiB      | 0.171 |  167 |      6.0 ms |      106 ms |
| 4 KiB      | 0.449 |  110 |      9.1 ms |      110 ms |
| 16 KiB     | 0.785 |   48 |     20.9 ms |      117 ms |
| 32 KiB     | 1.065 |   32 |     30.8 ms |      127 ms |
| 64 KiB     | 1.084 |   17 |     60.5 ms |      150 ms |

**32 KiB is the knee** - 0.11 MB/s at 512 B climbs ~10x to 1.07 MB/s at 32 KiB, and 64 KiB adds only ~2%
(at double the per-op latency). So the journal should write in ~32 KiB units.

**B. Batch depth (4 KiB writes, flush every N)**

| Flush every N |  MB/s | Flushes/s | Avg flush | Max flush |
| ------------- | ----: | --------: | --------: | --------: |
| 1             | 0.447 |       109 |    6.0 ms |    107 ms |
| 2             | 0.530 |        65 |    9.0 ms |    109 ms |
| 4             | 0.591 |        36 |    7.0 ms |    9.8 ms |
| 8             | 0.740 |        23 |    9.9 ms |    100 ms |
| 16            | 0.862 |        13 |   10.5 ms |    107 ms |
| 32            | 0.926 |       7.0 |   13.6 ms |    105 ms |
| 64            | 0.959 |       3.6 |   15.2 ms |     95 ms |
| 128           | 0.986 |       1.9 |   12.7 ms |     91 ms |

Throughput **plateaus around N=32-64** (flushing every 128-256 KiB of accumulated data): N=32 reaches ~94%
of max, N=64 ~97%. Past that, more batching barely helps and only widens the durability window.

**Queue sizing (the answer to "what to queue up"):** write in **~32 KiB units** and force a durable flush
about **every 128-256 KiB** (a batch of 4-8 units). That reaches ~95%+ of the durable throughput ceiling
(~1 MB/s) while bounding the at-risk window to a few hundred KB. Per-op durable writes below ~4 KiB throw
away 60-90% of the throughput. The 90-150 ms write tail is present at every size, so the queue must be deep
enough to keep accepting work across a stalled flush.

## 2. Pure codec host baseline

Host = Raspberry Pi 5 (Cortex-A76, `-O2`), a relative baseline; ESP32-S3 = the real device at 240 MHz.

| Feature   | Operation              | Host ns/op | Host MB/s | ESP32-S3 us/op | ESP32-S3 MB/s |
| --------- | ---------------------- | ---------: | --------: | -------------: | ------------: |
| base64    | encode 1 KiB (sw)      |        944 |      1085 |          47.36 |          21.6 |
| base64    | decode 1 KiB (mbedTLS) |       3274 |       313 |        1814.82 |          0.56 |
| mtconnect | streams doc (20 obs)   |       3291 |       749 |        278.279 |          8.86 |

Notes:

- **base64 was profiled, and it drove a design decision (the numbers above are the shipped hybrid).** The
  first on-device run showed the ESP32 base64 - which delegated to **mbedTLS** for both encode and decode -
  at only ~1.4 MB/s encode and ~0.56 MB/s decode (731 / 1815 us per KiB). mbedTLS is slow here because its
  base64 is **constant-time** (`mbedtls_ct_base64_enc_char` evaluates every alphabet range with branchless
  masks instead of a data-dependent table lookup, so timing / cache access does not leak the bytes). The
  in-house software codec is a plain table lookup - ~20x faster - but not constant-time. Rather than pick
  one globally, the code now splits by what the data is: **encode** only ever handles the _public_
  WebSocket-accept digest, so it uses the fast software codec on every target (~47 us, ~15x faster);
  **decode** is the only path that touches a _secret_ (Basic-auth credentials, RFC 7617), so on the ESP32 it
  keeps mbedTLS's constant-time decoder (it runs once per authenticated request, not in a byte loop).
  JWT / OIDC use `base64url`, which has always been the software codec. Verified on the ESP32-S3: RFC 4648
  vectors both directions, a Basic-auth credential round-trip, a 256-byte round-trip, and fail-closed on
  malformed input all pass.
- MTConnect builds a ~2.5 KB XML document (header + 20 observations) end to end in ~278 us on the device
  (~8.9 MB/s of document assembly) - the zero-heap writer holds up well on hardware.

### On-device CCOUNT microbench (auth / conditional-GET primitives)

Cycle-accurate on the ESP32-S3 @ 240 MHz via the CPU cycle counter (`ESP.getCycleCount()` = CCOUNT),
measured in-firmware by the pentest rig's `/bench` endpoint ([`pentesting/rig_firmware`](../pentesting/rig_firmware),
N=20000 warm iterations; three runs agree to within 0.3%, so the figures are stable). These are the real
device costs of hot pure primitives on the auth and ETag/Digest paths - no network in the measurement.

| Operation                               | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| --------------------------------------- | --------------: | -------------: |
| `det_hex_encode` (16 B -> 32 hex)       |             462 |           1925 |
| `det_hex_decode` (32 hex -> 16 B)       |             689 |           2870 |
| `base64_decode` ("admin:admin", 16 ch)  |            5040 |          21000 |
| `mime_type` (extension -> content-type) |             470 |           1958 |

- The hex codecs are the in-house table-lookup path: ~1.9 us to hex-encode a 16-byte value (an ETag or a
  Digest-nonce MAC), ~2.9 us to decode. Cheap enough that the conditional-GET / Digest machinery is never
  the bottleneck.
- `base64_decode` of an 11-byte Basic-auth credential costs ~21 us - about 11x the hex decode - because on
  the ESP32 the decoder is mbedTLS's **constant-time** base64 (branchless per-character masks so timing does
  not leak the credential; see the base64 note in section 2). At once per authenticated request that is
  invisible, and constant-time is the right trade for a secret. This is the same measurement path that would
  catch a regression if that ever changed to a fast-but-leaky table lookup. A constant-time speedup is
  possible via SWAR (decode 4-8 chars per word with parallel branchless masks) - see the ROADMAP perf item;
  a plain byte-indexed LUT would be faster but is NOT constant-time (data-dependent cache access).
- `mime_type` (path extension -> content-type, run on every file-serving response) is ~1.96 us - cheap; the
  content-type lookup is never the request-path bottleneck.

### Server-Sent Events framing (DETWS_ENABLE_SSE)

`sse_format()` builds one `event:`/`id:`/`data:` record (WHATWG event-stream format) into a buffer; it is
the pure, transport-free hot op behind every `sse_send()` / `sse_broadcast()`. Host figures from
[`perf/bench_sse.cpp`](../perf/bench_sse.cpp); the device figure is the rig `/bench` CCOUNT op (N=20000 warm).

| Operation                      | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------------------ | ---------: | --------: | --------------: | -------------: |
| `sse_format` data-only         |       62.5 |     432.1 |               - |              - |
| `sse_format` event + id + data |      180.2 |     299.7 |            3393 |          14137 |

- A fully-addressed record (named event + resumable id + data) costs ~14.1 us on the S3 - notably more than
  the codec primitives above, because the framing is three `snprintf("%s")` calls and the Xtensa `vsnprintf`
  path dominates. At SSE's push cadence (events, not per-byte) this is invisible, but a hand-rolled
  memcpy-based framer would cut it by an order of magnitude if a high-rate broadcast fan-out ever needs it
  (noted in the ROADMAP perf items). The data-only shape (the common broadcast case) is ~3x cheaper on the
  host, so the device cost scales down similarly.

## 3. Request-path benchmarks

The CPU cost of a request's hot path: the standalone HTTP/1.1 request parser and the zero-heap JSON
writer / reader (`perf/bench_reqpath.cpp` on the host, the same ops in an on-device firmware for the
ESP32-S3 column). Host = Raspberry Pi 5 (Cortex-A76, `-O2`), a relative baseline; ESP32-S3 = the real
device at 240 MHz, timed over in-RAM buffers so this is pure compute, not socket I/O. Byte counts: the GET
request is ~220 bytes over 6 headers, the POST is ~150 bytes with a 50-byte JSON body, the encoded doc is
96 bytes, the decoded body is 50 bytes.

| Feature    | Operation              | Host ns/op | Host MB/s | ESP32-S3 us/op | ESP32-S3 MB/s |
| ---------- | ---------------------- | ---------: | --------: | -------------: | ------------: |
| http_parse | GET (6 headers)        |     1819.6 |     122.6 |         84.969 |           2.6 |
| http_parse | POST + JSON body       |     1110.1 |     138.7 |         56.076 |           2.7 |
| json       | encode (8 fields, 96B) |      675.6 |     142.1 |         49.693 |           1.9 |
| json       | decode (4 fields)      |      263.6 |     189.7 |         19.765 |           2.5 |

Notes:

- **The whole request path is cheap next to the network.** On the device a full browser GET parses in
  ~85 us, a POST with a JSON body in ~56 us, and a typical telemetry response object encodes in ~50 us. So
  a complete parse -> build-JSON round trip is on the order of **~135 us of CPU** - the device can turn over
  ~7000 simple JSON request/responses per second of pure compute, far more than a 1-2-client embedded server
  ever needs. Real request latency is dominated by the TCP round trip and (when enabled) the TLS handshake,
  not by parsing or JSON. **No optimization was warranted here** (unlike the base64 and CRC findings in
  sections 2 and 4), which is itself the useful result: the request path is not the bottleneck.
- **The parser is byte-at-a-time by design.** `http_parser_feed()` is a pure per-byte state machine with no
  look-ahead or buffering, so its cost scales with request length (~0.4 us/byte on the device) and it can
  consume bytes exactly as they arrive off the socket - the parse overlaps the network read instead of
  waiting for a whole request. That streaming design is why a ~220-byte GET costs more than a ~150-byte POST
  here even though the POST carries a body.
- **JSON throughput is ample.** The zero-heap writer emits a 96-byte object in ~50 us (~1.9 MB/s) and each
  top-level field read re-scans the body (~5 us/field on the device); four reads over a 50-byte body is
  ~20 us. Re-scanning per field is O(n) in the body, so for a large body with many reads a single parse pass
  would win - but for the flat IoT shapes this reader targets (a handful of fields in a sub-1-KB body) it is
  already well under any flash or network cost.
- Host is ~35-45x faster than the device across the board, the expected ratio for a 2.4 GHz A76 vs a 240 MHz
  Xtensa LX7, and the two tracked each other with no surprises - nothing on the request path behaved
  differently on hardware than the host predicted.

_Still to add:_ the TLS handshake and SSH KEX wall-clock (one-time per-connection costs, dominated by the
mbedTLS RSA/ECDHE math - the ~7 KB modexp stack cost is already characterized in docs/TODO.md; a full
end-to-end handshake bench needs the PSRAM TLS build) and a chunked / file send-pump pass.

## 4. Embedded data-store stack

The CPU cost of the hot ops in the data-store stack (WAL / dbm / document store / SQLite reader / Redis
RESP), measured over a **RAM-backed device** so this is pure compute, not I/O. Host = Raspberry Pi 5
(Cortex-A76, `-O2`), a relative baseline; ESP32-S3 = the real device at 240 MHz (`perf/bench_datastore.cpp`
on the host, the same benches in an on-device firmware for the ESP32-S3 column).

Two device-motivated optimizations this benchmark drove are already applied (a table-driven CRC-32 and a
hand-rolled RESP length prefix - see the discussion below); numbers below are post-optimization.

| Feature  | Operation               | Host ns/op | ESP32-S3 us/op |
| -------- | ----------------------- | ---------: | -------------: |
| wal      | crc32 (1 KiB)           |     2975.5 |         64.483 |
| wal      | record_encode (128 B)   |      429.9 |         10.974 |
| wal      | store_append (64 B)     |      274.3 |          7.758 |
| wal      | store_checkpoint        |      111.0 |          6.165 |
| dbm      | put (16 B key/64 B val) |      153.9 |         11.298 |
| dbm      | get                     |       32.5 |          2.066 |
| docstore | find_str (scan 100)     |     8962.8 |        443.618 |
| docstore | -> per doc scanned      |       89.6 |          4.436 |
| sqlite   | varint_decode           |        5.0 |          0.301 |
| sqlite   | table scan (40 rows)    |     1500.0 |        127.239 |
| sqlite   | -> per row (+ columns)  |       37.5 |          3.181 |
| resp     | encode_command (3 args) |       65.2 |          3.262 |
| resp     | parse bulk reply        |       13.4 |          1.026 |

**Reads are cheap; the write path is CRC-bound, so the CRC was made table-driven.** A dbm `get` is ~2 us
on the device, a SQLite row (with its columns) ~3.2 us, a RESP reply parse ~1 us, a `docstore` field scan
~4.4 us per document - all comfortably faster than any flash access, so durable reads/queries are
I/O-bound. The write path is `record_encode` / `store_append` / dbm `put`, and these are dominated by the
CRC over the record. The **first on-device run exposed the table-less CRC-32 as the bottleneck at only
~4.4 MB/s** (231 us/KiB) - just ~3x the ~1.5 MB/s durable SD rate, close enough to matter. Switching to a
byte-table CRC (1 KiB of rodata) **cut it ~3.6x to ~15.9 MB/s** (64 us/KiB), which roughly halved
`record_encode`, `store_append`, and dbm `put`; the CRC is now ~10x the SD rate, back to comfortable
I/O-bound territory (the SD card's ~40-100 IOPS and 100+ ms write tail set the real ceiling, which is why
the layer batches and checkpoints in bulk). The host saw the same 3.6x (96 -> 344 MB/s), a nice example of
a fix that only the on-device number motivated - the host had 60x headroom and never showed the problem.

The second finding was `resp_encode_command` at ~20 us on the device - it formatted the RESP length
prefixes with `snprintf`. Replacing that with a hand-rolled decimal writer **cut it ~6x to ~3.3 us**
(and ~5x on the host, 329 -> 65 ns), with byte-identical output. Both fixes came straight out of this
table; neither was visible from the host baseline alone.

## 5. Network transport features (SMB client / Ethernet DNC / port-forward)

Three transport-example features were taken end to end on hardware (ESP32-S3 over WiFi, `q_6`) against
real peers on a Raspberry Pi: a **Samba 4.13** share, a raw-TCP sink, and a Python `http.server` origin.
Each transfer is **byte-verified** (sha256 or FNV-1a compared to the source). HW testing found four real
bugs the host mock-seam tests could not (a stubbed client transport, an `smb_open` stack overflow, a
`listen()` that returned the wrong id, and the relay throughput issue below) - see docs/BUGS.md.

### Port-forward / DNAT relay (DETWS_ENABLE_RELAY)

The board fronts a port and relays every byte to an internal origin (`server.listen(p, PROTO_RELAY)` +
`det_relay_publish()`). Measured by fetching a file **through** the ESP32 (RPi -> ESP32:8080 -> RPi:8000)
and sha256-verifying it:

| Transfer |  Time | Throughput | Byte-exact |
| -------- | ----: | ---------: | ---------- |
| 1 MB     |  4.3s |  1.87 Mbps | OK         |
| 5 MB     | 24.3s |  1.65 Mbps | OK         |
| 10 MB    | 44.3s |  1.81 Mbps | OK         |
| 50 MB    |  209s |  1.91 Mbps | OK         |
| 100 MB   |  449s |  1.78 Mbps | OK         |
| 200 MB   | 1000s |  1.60 Mbps | OK         |

Sustained **~1.6-1.9 Mbps**, flat across three orders of magnitude of transfer size, **byte-exact all the
way to 200 MB** - the relay holds up for large transfers, not just a smoke test. (One first 200 MB attempt
took a single WiFi recv-drop ~15 min in; the retry completed byte-exact. Over a link that flaky, keeping
the radio awake matters - the relay now holds modem sleep off while a bridge is active, see the radio
keep-awake note below.)

**The first on-device run only managed ~0.4 Mbps**, and the fix came straight out of that number (the host
mock never exercises the real send path). Two causes:

- `a_send` forwarded to the inbound socket **all-or-nothing**: a whole `DETWS_RELAY_BUF` chunk rarely fits
  `tcp_sndbuf` in one shot, so a "full or nothing" send forwarded **zero** bytes and stalled. Naively
  raising the buffer to 2 KB made it _worse_ (~0.2 Mbps) for the same reason. The fix sends as much as the
  send window currently allows (`det_conn_sndbuf`), partial.
- `service()` pumped **one 512 B step per poll**. It now drains up to `DETWS_RELAY_DRAIN_MAX` passes, so one
  poll forwards the whole buffered origin RX ring instead of a single chunk.

Together: **~0.4 -> ~1.8 Mbps (~4.5x)**, byte-exact. The remaining ceiling is the classic
bandwidth-delay-product limit - the inbound TCP send window over the double-hop WiFi relay - not the pump;
raising `TCP_SND_BUF` / enabling window scaling in a custom lwIP would lift it further.

**Practicality:** ~1.8 Mbps is the right tool for what a device-side port-forward is actually for -
publishing a control-plane service through the board that bridges two segments: an SSH or HTTP admin
console, a Modbus/OPC-UA endpoint, a config UI on a locked-down PLC network, and moderate file pulls. It
is byte-exact and stable to hundreds of MB, so it will not corrupt a firmware image or a config archive. It
is **not** a bulk media pipe - a 200 MB pull is minutes, and the WiFi double-hop halves the radio's usable
rate. Keep the front port off untrusted networks (the relay authenticates nothing on the inbound side).

### Radio keep-awake during transfers (DETWS_ENABLE_RADIO_POWER)

Modem sleep (the default `WIFI_PS_MIN_MODEM`) parks the radio between DTIM beacons to save power, which is
exactly what dropped a byte mid-transfer on the first 200 MB run. `detws_radio_busy_hold()` /
`detws_radio_busy_release()` are reference-counted: the first hold forces `WIFI_PS_NONE`, the last release
restores the configured `DETWS_RADIO_WIFI_PS`. The relay holds one while any bridge is active, so a
port-forward keeps the radio awake for the whole transfer and lets power saving resume when idle - no
per-app tuning. **Practicality:** on a battery device you still get modem-sleep power savings at rest; you
only pay the awake current while actually moving bytes, which is the right trade for reliability.

### SMB2 client (DETWS_ENABLE_SMB)

Reads a file off a Windows / Samba share with real **NTLMv2** auth (NEGOTIATE -> two-round SESSION_SETUP
-> TREE_CONNECT -> CREATE -> READ). On device it authenticated against Samba 4.13 and read the test file
**byte-exact** (FNV-1a matched the server). The whole open+read+close completes in well under a second on
the LAN. The working buffers (~4 KB) live in an owned static `SmbClientCtx`, not on the stack, because on
the stack they overflow the 8 KB Arduino loopTask (the crash HW testing caught).

**Practicality:** the use case is pulling a **small control file** - a CNC `.nc` program, a recipe, a
config blob - off the shop file server, where correctness and NTLMv2 interop matter far more than MB/s.
The client drives one sequential dialogue at a time (not reentrant across two concurrent SMB connections),
which is exactly how a device fetches a program before a run.

### Ethernet DNC (DETWS_ENABLE_DNC)

Drip-feeds a G-code program to a controller's raw TCP program port with XON/XOFF pacing. On device it
streamed a program to a capture sink and the received bytes were **byte-exact** (FNV-1a matched) with
spec-correct framing: NUL leader, `%`+CRLF start, CR-before-LF end-of-block per line, `%`+CRLF end, NUL
trailer.

**Practicality:** DNC programs are small (KB) and the controller's input buffer is tiny, so the design
point is **correct framing + flow control**, not throughput - a fast sender just overruns the machine. The
XON/XOFF pause path is the feature; the byte-exact framing is what keeps the controller from faulting.
