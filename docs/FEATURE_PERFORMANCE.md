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

### Second-board confirmation - XIAO ESP32-S3 Sense (2026-07-13)

Repeated on a **XIAO ESP32-S3 Sense** (its onboard microSD, SPI CS=GPIO21, bus SCK7/MISO8/MOSI9) with a
**32 GB SDHC** card, 16 KiB sequential blocks, content byte-verified (FNV-1a round-trip) at every point:

| SPI clock       | Seq write MB/s | Seq read MB/s | Max write latency |
| --------------- | -------------: | ------------: | ----------------: |
| 4 MHz (default) |          0.424 |         0.424 |            133 ms |
| 8 MHz           |          0.751 |         0.753 |            115 ms |
| 16 MHz          |          1.394 |         1.242 |            105 ms |
| **20 MHz**      |      **1.688** |         1.423 |         **16 ms** |
| 25 / 40 MHz     |          1.688 |         1.423 |             16 ms |

Same shape as the DevKitC: **scales to 20 MHz, then plateaus** (~1.69 MB/s write / ~1.42 MB/s read) - the ESP
SD-SPI driver clamps the effective clock, so 25/40 MHz are byte-identical to 20. Max latency also collapses to
~16 ms at 20 MHz, so **20 MHz is the sweet spot** (best throughput and lowest per-op stall). The Arduino
`SD.begin(cs)` default is **4 MHz** - 4x slower - so an app must pass the clock: `SD.begin(cs, spi, 20000000)`.
The ~1.5-1.7 MB/s SD-over-SPI ceiling holds across two boards and two cards, confirming it is card/protocol
bound, not board-specific.

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

### WebDAV 207 Multi-Status builder (DETWS_ENABLE_WEBDAV)

`webdav_ms_entry()` builds one `<response>` element (RFC 4918 Multi-Status) for a resource; it runs
once per directory child on every PROPFIND, and internally XML-escapes the href. Pure (no filesystem),
so it benches standalone. Host figures from [`perf/bench_webdav.cpp`](../perf/bench_webdav.cpp); the
device figure is the rig `/bench` CCOUNT op (N=20000 warm).

| Operation                             | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------------------------- | ---------: | --------: | --------------: | -------------: |
| `webdav_ms_entry` (one file response) |      128.4 |    3161.2 |            4598 |          19158 |
| PROPFIND Depth-1 body (8 children)    |     1223.2 |    2992.3 |               - |              - |
| `webdav_xml_escape` (5 escapables)    |       67.0 |     582.3 |               - |              - |

- One `<response>` costs ~19.2 us on the S3 - the heaviest of the presentation-layer builders (it
  escapes the href and does a hand-rolled itoa for the content length into a 512 B temp), but PROPFIND
  runs it only once per directory child, so a Depth-1 listing of a small directory is a handful of these
  plus the flush. The host shows the whole Depth-1 body (prolog + 8 children + epilog) at ~1.2 us; on
  device that scales to roughly `8 x 19 us` plus the filesystem `readdir` + `stat` per child, so for a
  large directory the **filesystem walk, not the XML build, is the cost** - the builder is not the bottleneck.
- The real device PROPFIND latency is dominated by LittleFS directory enumeration; the XML build is cheap
  by comparison. A directory-listing cache (invalidated on PUT/DELETE/MKCOL/MOVE) would help a hot share.

### CoAP server codec (DETWS_ENABLE_COAP)

`coap_server_process()` is the whole CoAP request→response path (RFC 7252): parse the 4-byte header +
options, reconstruct the Uri-Path, dispatch against the resource table, and encode the piggybacked
reply. Pure (no sockets, no heap). Host figures from [`perf/bench_coap.cpp`](../perf/bench_coap.cpp);
the device figure is the rig `/bench` CCOUNT op (N=20000 warm), including the handler that renders the
`/info` JSON.

| Operation                               | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| --------------------------------------- | ---------: | --------: | --------------: | -------------: |
| `coap_server_process` GET /info         |       58.8 |     221.0 |            5331 |          22212 |
| `coap_server_process` GET /a/b/c (3seg) |       29.6 |     338.3 |               - |              - |

- A full CoAP GET round trip (parse + dispatch + encode, plus the handler's `snprintf` of the JSON body)
  is ~22 us on the S3. That is a complete datagram exchange, not a micro-codec, so it is the honest
  request-path number for CoAP - comparable to a small HTTP request but over a single UDP datagram with
  no connection setup. UDP + the fixed resource table make CoAP the cheapest of the request paths per
  transaction on this device.

### SNMP agent codec (DETWS_ENABLE_SNMP)

`snmp_agent_process()` is the whole SNMP v1/v2c path (RFC 1157/3416): BER-decode the message + PDU,
walk the MIB against the varbind OIDs, BER-encode the reply. Pure (no sockets, no heap). Host figures
from [`perf/bench_snmp.cpp`](../perf/bench_snmp.cpp); the device figure is the rig `/bench` CCOUNT op.

| Operation                           | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ----------------------------------- | ---------: | --------: | --------------: | -------------: |
| `snmp_agent_process` GET sysDescr.0 |      673.9 |      72.7 |            7005 |          29187 |
| `snmp_agent_process` GETNEXT (walk) |      799.2 |      58.8 |               - |              - |

- A GET round trip is ~29 us on the S3 - the heaviest of the request-path codecs, because BER is a
  tag-length-value format decoded and re-encoded field by field (vs CoAP's simpler byte-oriented options
  and HTTP's text parse). GETNEXT (the snmpwalk step) costs a bit more again for the lexicographic MIB
  successor search. Still well under the datagram inter-arrival time of any realistic poll, so BER is not
  a bottleneck for a monitored device.
- **Amplification note (security):** the pentest `snmp_getbulk_amplification` measured a GETBULK with
  max-repetitions=10000 producing only ~9.7x (40 B request -> 386 B reply) - the constrained agent caps
  the reply at its small MIB + fixed tx buffer, so it is **not a usable reflection/amplification vector**
  (unlike a full SNMP daemon over a large MIB). That bound is a determinism property, not a config knob.

### OPC UA Binary codec (DETWS_ENABLE_OPCUA)

The OPC UA server's hot ops (IEC 62541 / OPC UA Part 6): the UACP Hello/Acknowledge handshake
(`opcua_parse_hello` + `opcua_build_ack`, run once per connection) and the per-node DataValue Variant
encode (`ua_w_datavalue`, the Read-service hot op). All pure little-endian codecs. Host figures from
[`perf/bench_opcua.cpp`](../perf/bench_opcua.cpp); the device figure is the rig `/bench` CCOUNT op
(parse HELLO + build ACK together).

| Operation                           | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ----------------------------------- | ---------: | --------: | --------------: | -------------: |
| `opcua_parse_hello`                 |       15.0 |    3934.4 |               - |              - |
| `opcua_build_ack`                   |       25.1 |    1115.1 |               - |              - |
| HELLO parse + ACK build (handshake) |          - |         - |            1563 |           6512 |
| `ua_w_datavalue` (scalar Double)    |       14.2 |     705.0 |               - |              - |

- The whole handshake is ~6.5 us on the S3 - the cheapest connection-setup of the TCP protocols, because
  OPC UA Binary is a fixed little-endian struct format (no text parse, no TLV). The built-in-type codec
  (`ua_w_*` / `ua_r_*`) is a few ns per field, so the Read/Browse response cost scales with the number of
  nodes + references, not the encoding.
- **Buffer-negotiation note (security):** the pentest `opcua_hello_buffer_abuse` sent a HELLO advertising
  4 GB Receive/Send/MaxMessage buffers; the ACK negotiated them **down to the server's fixed 8192 B**
  (`DETWS_OPCUA_BUF`) with MaxChunkCount 1 - the server never honors the client's huge sizes, so a Hello
  cannot induce an over-allocation. That bound is structural (fixed buffers), not a tunable.

### Modbus TCP slave codec (DETWS_ENABLE_MODBUS)

`modbus_process_adu()` is the whole Modbus TCP slave path (Modbus Application Protocol): parse the MBAP
header, dispatch the function code against the coil/register data model, build the response ADU. Pure
(no sockets, no heap). Host figures from [`perf/bench_modbus.cpp`](../perf/bench_modbus.cpp); the device
figure is the rig `/bench` CCOUNT op (Read Holding Registers x8).

| Operation                            | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------------------------ | ---------: | --------: | --------------: | -------------: |
| `modbus_process_adu` read holding x8 |       17.6 |     683.3 |             477 |           1987 |
| `modbus_process_adu` write multi x2  |       12.6 |    1351.2 |               - |              - |

- A Read Holding Registers round trip is ~2.0 us on the S3 - the **cheapest of all the protocol request
  paths** (a fixed binary MBAP header + a direct index into the register array, no text/TLV/negotiation).
  Modbus is the lightest per-transaction protocol on the device, fitting its role as a high-rate PLC/SCADA
  poll target - a single connection can be polled at hundreds of Hz without the codec being the limit.

### WebSocket permessage-deflate (RFC 7692)

The compress/decompress hot ops on every WebSocket data frame when permessage-deflate is negotiated:
`deflate_raw()` (TX) and `inflate_raw()` (RX). Both are pure (a caller scratch, no heap). Host figures
from [`perf/bench_deflate.cpp`](../perf/bench_deflate.cpp); the device figure is the rig `/bench` CCOUNT
op (inflate of a ~70 B JSON message).

| Operation                    | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ---------------------------- | ---------: | --------: | --------------: | -------------: |
| `deflate_raw` (JSON message) |     5366.3 |      37.5 |               - |              - |
| `inflate_raw` (JSON message) |     5133.5 |      39.2 |           47131 |         196379 |

- These are **by far the most expensive per-message ops** on the device: `inflate_raw` is ~196 us to
  decompress one small WebSocket frame (real LZ77 back-references + Huffman decode), ~10x the SNMP BER
  path and ~100x the Modbus codec. permessage-deflate trades ~200 us of CPU per message for the bandwidth
  saving - worth it on a slow/metered link, a poor trade on a fast LAN. Enable it deliberately, not by default.
- **Decompression-bomb note (security):** the pentest `ws_deflate_bomb` sent a **986 B** compressed frame
  that inflates to **1 MB (a 1014:1 ratio)**; the device bounded `inflate_raw` to `WS_FRAME_SIZE` and
  **refused it without allocating the expanded payload** - a WebSocket zip bomb cannot exhaust memory.
  The output cap is structural (a fixed message buffer), so the defense holds regardless of the ratio.

### SSH server crypto (DETWS_ENABLE_SSH)

The SSH-2 server's hot cryptographic ops: the curve25519 KEX (an X25519 scalar multiplication for the
ephemeral key and again for the shared secret), the ssh-ed25519 host-key signature over the exchange
hash (a fixed-base scalar multiplication), and the chacha20-poly1305@openssh.com record layer (the
per-packet steady-state op). Host figures from [`perf/bench_ssh.cpp`](../perf/bench_ssh.cpp); the device
figures are an on-device CCOUNT microbench (`ESP.getCycleCount()` @ 240 MHz, N warm iterations, run on
a high-priority core-1 task in the `rig_s3_ssh` firmware; the two X25519 measurements agree to 0.03%).

| Operation                               | Host ns/op | Host MB/s | ESP32-S3 us/op | ESP32-S3 MB/s |
| --------------------------------------- | ---------: | --------: | -------------: | ------------: |
| `ssh_x25519` scalarmult (KEX)           |  1,588,291 |         - |     23,219 [1] |             - |
| `ssh_ed25519_sign` (host-key signature) |  5,448,039 |         - |     85,638 [1] |             - |
| `ssh_chachapoly_encrypt` (1 KiB packet) |      6,103 |     167.8 |            705 |           1.5 |
| `ssh_gf_mul` (radix-2^16 field mul)     |        510 |         - |          55.45 |             - |

[1] Both the X25519 KEX and the Ed25519 host-key signature run their field arithmetic on the RSA/MPI hardware
accelerator on the S3 (see the MODMULT bullet); the host figures are the software radix-2^16 ladder (the native
/ non-S3 fallback).

- **The SSH handshake crypto is ~0.13 s on the device** (two X25519 at ~23 ms each + one ed25519 sign at
  ~86 ms) - down from ~0.85 s of pure software crypto. Both the curve25519 KEX and the ed25519 host-key
  signature now run their GF(2^255-19) field arithmetic on the **RSA/MPI hardware accelerator** (the MODMULT
  bullet below); only the record-layer chacha20-poly1305 stays software. That is a fixed sub-0.2 s connection
  setup - comfortable for an admin/config channel or a tunnel, and much closer to viable for shorter
  connections. The chacha-poly record layer keeps the ~95-115x host/device ratio (a software op); the two
  HW-offloaded field-arithmetic ops break it.
- The **chacha20-poly1305 record layer runs at ~1.5 MB/s** on the S3 (705 us to encrypt+authenticate a 1 KiB
  packet: two ChaCha20 keystreams + a Poly1305 tag, all software). That is the steady-state throughput
  ceiling once the session is up - ample for a shell / control channel or metered telemetry, a bottleneck
  for bulk file transfer (`scp` of large files). AES-256-CTR is offered as a fallback (HW-accelerated AES),
  but chacha is negotiated first and is the security-preferred choice.
- **SIMD field multiply SHIPPED (ESP32-S3).** `ssh_gf_mul` now runs on the S3 vector unit
  (`ee.vmulas.s16.accx`): balance the limbs into signed-16-bit, run the 31-output convolution on the 40-bit
  ACCX, fold in C. The isolated field multiply is **8,583 vs 13,308 cycles = 1.55x** (device-measured,
  byte-exact vs scalar across 3000 operands; guarded `#if CONFIG_IDF_TARGET_ESP32S3`, scalar fallback
  elsewhere). Measured in the ladder: **X25519 150.8 -> 97.5 ms (1.55x), ed25519_sign 547.9 -> 380.3 ms (1.44x)**, so the
  handshake crypto (2 X25519 + 1 ed25519 sign) falls from ~0.85 s to ~0.58 s. Getting there took three
  levers past the raw MAC: (1) a dedicated vector `ssh_gf_sq` that balances the operand **once** (squarings
  are ~2/3 of the Montgomery ladder; `mul(a,a)` would balance twice); (2) **`gf_balance_s16` in `int32`
  instead of `int64`** - the balance runs per operand and its 48-step carry propagation was emulated 64-bit
  math, which dominated the field op (limbs stay ~+-2^18 so `int32` is byte-exact); (3) confirming the
  window loads were **not** the bottleneck - an in-register sliding window that removes every per-output
  `ee.ld.128.usar` is byte-exact but only ~1.4% faster (7,955 vs 8,067 cycles), so it was not shipped. X25519
  gains more than ed25519 because ed25519's Edwards scalar-mult is multiply-dominated (fewer squarings). The
  paragraph below is the pre-SIMD baseline.
- **The GF(2^255-19) field layer runs on the RSA/MPI hardware MODMULT (SHIPPED, ESP32-S3) - X25519 97.5 -> 22.65
  ms (4.31x) AND ed25519_sign 380.3 -> 85.6 ms (4.44x).** Field elements are canonical `uint32[8] mod p` and each
  field multiply is one **256-bit modular multiply** `Z = X*Y mod p` on the RSA accelerator, driven
  **register-direct with static buffers (zero heap)**: load `M,X,Y` + `r=R^2 mod p` into the result block + `M'`,
  trigger `MOD_MULT`, read `Z` (add/sub are native 32-bit carry + conditional-subtract-p; the inversions are
  tweetnacl MODMULT chains; no per-op pack/unpack because everything stays canonical - MODMULT output is provably
  `< p`, verified 0 / 5000). One modmul is **1,386 cycles vs the SIMD `gf_mul`'s 7,955 (5.8x), 9.6x vs scalar**,
  data-independent (constant-time). The X25519 Montgomery ladder and the Ed25519 extended-twisted-Edwards point
  arithmetic share this `fe` layer (`ssh_fe25519.h`, `DETWS_FE25519_MPI_HW`); end to end **X25519 97.5 -> 22.65
  ms, ed25519_sign 380 -> 85.6 ms**, dropping the handshake crypto from ~0.58 s (SIMD) to **~0.13 s**. Byte-exact
  vs the software radix-2^16 ladder (RFC 7748 §5.2 / RFC 8032 §7.1 + Wycheproof, native tests) and **HW-verified
  by a live `curve25519-sha256` KEX with an `ssh-ed25519` host key against OpenSSH** on the rig (a wrong X25519
  fails NEWKEYS; a wrong ed25519 signature is rejected before NEWKEYS - both reached NEWKEYS + auth). The layer
  shares the accelerator (and its lock) with mbedTLS RSA/DH, so each scalar-mult brackets itself with
  `esp_mpi_{enable,disable}_hardware_hw_op()` - the same lock+power bring-up mbedTLS uses - and holds the lock for
  its run (a handshake is infrequent; per-multiply toggling would cost more than it saves). Guarded
  `#if defined(ARDUINO) && CONFIG_IDF_TARGET_ESP32S3`, with the SIMD/scalar `ssh_gf` ladder as the fallback.
  **HTTP/3 shares the same `ssh_x25519` + `ssh_ed25519`, so the QUIC handshake gets the win too.** The
  reproducible probe lives in `pentesting/rig_firmware/src/main_ssh.cpp` under `DETWS_SSH_BENCH`.
- **`-O2` does not speed up the crypto (measured).** Rebuilt `rig_s3_ssh` at `-O2` (pre-MODMULT SIMD build):
  X25519 **97.3 ms**, ed25519_sign **380 ms** - identical to the `-Og` numbers. The ladder is hand-written
  vector assembly plus already-`int32` C glue, neither of which the optimizer can improve, so the shipped `-Og`
  figures **are** the production crypto numbers (unlike the pure-C protocol codecs, which gain ~15-30% at `-O2`).
  The MODMULT X25519 is likewise hardware-bound (the accelerator, not the C glue, is the floor), so `-O2` is a
  no-op there too. The pure-C ciphers that _do_ benefit (ChaCha20, Poly1305 - ~2x) force a higher level per
  translation unit via `DETWS_CRYPTO_HOT` ([`shared_primitives/crypto_opt.h`](../src/shared_primitives/crypto_opt.h),
  configurable `DETWS_CRYPTO_OPT_LEVEL` = 2 default / 3 / 0), applied **only** to code that is constant-time by
  structure; the mask-select scalar-mult paths are deliberately left at `-Os` (they are HW-dominated and cranking
  the optimizer risks defeating their constant-time property for no speedup).
- **Where the handshake time goes - and the SIMD acceleration target.** The radix-2^16 field multiply
  `ssh_gf_mul` is **13,308 cycles / 55.4 us** on the S3 in scalar form (a 16x16 schoolbook = 256 multiply-accumulates). At
  ~2,600 field multiplies per X25519 (255 ladder steps x ~10 mul/sq + the reduction) it is essentially the
  **entire** scalar-multiply cost - so cutting it cuts the whole handshake. A first, host-validatable scalar
  optimization already landed: casting the limbs to `int32` in the inner product makes gcc emit a hardware
  widening `mull`/`mulsh` (32x32->64) instead of the emulated `__muldi3` (64x64), which shaved ~3% (13,762 ->
  13,308 cyc). That the win is only ~3% is the important finding: **the 256 `int64` accumulations dominate, not
  the multiply** (products reach ~38 bits, so the accumulator cannot be narrowed in scalar code), which is
  exactly what a vector unit fixes. The S3 has 128-bit **integer SIMD** (the "PIE" vector unit;
  `ee.vmulas.s16.qacc` = eight signed-16-bit MACs into a wide hardware accumulator - the same unit esp-dsp's
  `dotprod_s16_aes3` kernel uses), which maps directly onto the 16-bit limbs and accumulates all eight lanes in
  one instruction: the 256 scalar MACs + their 256 `int64` adds become ~32 vector-MAC instructions. That is the
  acceleration target (a plausible 5-10x handshake win), tracked as the ed25519/curve25519 SIMD work. The FPU is
  **not** an option here: the S3 FPU is single-precision only (`__FP_FAST_FMAF32`, no double), and a
  floating-point curve25519 needs ~51-bit limb products, so the integer path is the only viable one.

### All self-implemented crypto primitives (device CCOUNT sweep)

Every cryptographic primitive the library implements itself (not the mbedtls TLS record path), timed on an
**ESP32-S3 @ 240 MHz** with the Xtensa cycle counter (`ESP.getCycleCount()` reads CCOUNT). Each op is warmed
once then averaged over N iterations on a high-priority core-1 task; N is sized per op so the total stays
under the 32-bit CCOUNT wrap (2^32 cyc ≈ 17.9 s) - the one trap here (a too-large N silently wraps and
reports a fraction of the true cost). Reproducible firmware: [`pentesting/rig_firmware/src/main_cryptobench.cpp`](../pentesting/rig_firmware/src/main_cryptobench.cpp),
env `rig_s3_cryptobench` (stock `espressif32@6.13.0`, arduino 2.x / mbedtls v2, `-Og`). The KEX/signature
figures agree with the independently measured numbers in the SSH section above to ~1% (x25519 23.1 vs 23.2 ms,
ed25519_sign 84.6 vs 85.6 ms, `fe_mul` 1377 vs 1386 cyc), which cross-validates the harness.

**Bulk primitives (per 1 KiB, sorted by throughput):**

| Primitive                       | Backend           | S3 cyc / KiB | ns / byte | MB/s |
| ------------------------------- | ----------------- | -----------: | --------: | ---: |
| `ssh_aes256ctr`                 | HW AES            |       10,909 |      44.4 | 22.5 |
| `ssh_sha256`                    | HW SHA            |       12,859 |      52.3 | 19.1 |
| `ssh_sha512`                    | HW SHA            |       17,149 |      69.8 | 14.3 |
| `ssh_poly1305`                  | SW (-O2 TU)       |       24,904 |     101.3 |  9.9 |
| `ssh_hmac_sha256`               | HW SHA            |       31,994 |     130.2 |  7.7 |
| `ssh_hmac_sha512`               | HW SHA            |       43,628 |     177.5 |  5.6 |
| `ssh_chacha20`                  | SW (-O2 TU)       |       46,434 |     188.9 |  5.3 |
| `ssh_chachapoly` encrypt (AEAD) | SW (-O2 TU)       |       77,433 |     315.1 |  3.2 |
| `ssh_aesgcm` seal (AES-256-GCM) | HW AES + SW GHASH |      555,421 |     2,260 | 0.44 |
| `quic_aes128_gcm` seal          | HW AES + SW GHASH |      562,292 |     2,288 | 0.44 |
| `dtls_record` protect (DTLS1.3) | HW AES + SW GHASH |      578,730 |     2,355 | 0.42 |

**One-shot primitives (KEX, KDF, signatures; sorted by cost):**

| Primitive                          | Backend             | S3 cyc / op | time / op |
| ---------------------------------- | ------------------- | ----------: | --------: |
| `fe_mul` (256-bit field multiply)  | HW MODMULT          |       1,377 |   5.74 us |
| `ssh_gf_mul` (field mul, fallback) | SW radix-2^16       |       9,212 |   38.4 us |
| `quic_hkdf_extract`                | HW SHA              |      25,044 |    104 us |
| `quic_hkdf_expand_label`(16)       | HW SHA              |      25,946 |    108 us |
| `tls13_kdf_expand_label`(16)       | HW SHA              |      25,910 |    108 us |
| `ssh_rsa_2048_verify` (SHA-256)    | HW MPI              |   3,959,764 |   16.5 ms |
| `ssh_ed25519_sign`                 | HW MODMULT + HW SHA |   4,651,281 |   19.4 ms |
| `ssh_x25519` scalarmult (KEX)      | HW MODMULT          |   5,547,625 |   23.1 ms |
| `mlkem768_encaps` (ML-KEM-768)     | SW NTT              |   5,645,995 |   23.5 ms |
| `ssh_ecdsa_p256_ecdh` (KEX)        | HW MODMULT          |  12,269,174 |   51.1 ms |
| `ssh_ed25519_verify`               | HW MODMULT + HW SHA |  12,427,688 |   51.8 ms |
| `ssh_ecdsa_p256_sign`              | HW MODMULT          |  13,064,925 |   54.4 ms |
| `ssh_ecdsa_p256_verify`            | HW MODMULT          |  24,774,465 |  103.2 ms |
| `bn_expmod_group14` (DH-2048)      | HW MPI              |  43,543,754 |  181.4 ms |
| `ssh_rsa_2048_sign` (SHA-256)      | HW MPI (CRT+cached) |  64,699,824 |  269.6 ms |

- **AES-GCM is GHASH-bound, and GHASH is software (now 4-bit-table, ~7x faster).** AES-256-CTR runs at
  **22.5 MB/s** on the HW AES block, but AES-256-GCM authenticates (GHASH) in software, so it is far slower.
  The original table-less bitwise GF(2^128) multiply (128 iterations/block, ~3,700 cyc/byte) sealed 1 KiB in
  ~3.83 M cyc (0.064 MB/s). A shared **4-bit (Shoup) table GHASH** ([`shared_primitives/ghash.h`](../src/shared_primitives/ghash.h),
  built once per key) replaced it: **555,421 cyc (0.44 MB/s) - a 6.9x speedup**, byte-exact vs the bitwise
  reference (NIST/McGrew GCM KATs + a direct fuzz cross-check). The QUIC and DTLS 1.3 record layers
  (AES-128-GCM) share the same GHASH and gain the same ~6.8x. Unlike curve25519 (which offloads its field
  multiply to the RSA/MPI MODMULT), **this chip has no hardware GF(2^128) multiplier** (no `SOC_AES_SUPPORT_GCM`
  on the arduino-2.x toolchain), so the lever was algorithmic, not a HW offload. The platform mbedtls GCM is
  still ~5x faster again (104 K cyc, 2.4 MB/s) - it is precompiled at `-O2` (the library ships at the arduino
  framework's `-Os`, which does not speed up the table GHASH but ~2x's the other pure-C crypto) and/or uses a
  larger table; routing the ESP32 path through `mbedtls_gcm` would reach 2.4 MB/s but adds a host-untestable
  code path, so it is left as a documented option. For an SSH bulk transfer, `chacha20-poly1305` (1.6 MB/s
  software) is still faster than aes256-gcm and is negotiated first; AES-256-CTR (22.5 MB/s) is faster still
  where an AEAD is not required.
- **Ed25519 sign is ~4.4x faster with a fixed-base comb.** An `ssh-ed25519` host-key signature dropped from
  **84.6 ms to 19.4 ms** by replacing the variable-base ladder for the base point B with a constant-time
  signed 4-bit fixed-base comb (ref10 layout): a table of `256^i * B` multiples in flash
  ([`ssh_ed25519_comb_table.h`](../src/network_drivers/presentation/ssh/crypto/ssh_ed25519_comb_table.h),
  generated by [`tools/gen_ed25519_comb.py`](../tools/gen_ed25519_comb.py), verified vs an affine reference)
  bakes the doublings into the table, so a base-point scalar mult is ~64 additions + 4 doublings instead of
  the 255-add / 255-double ladder. Sign gains ~4.4x because it does **two** fixed-base mults (`A = a*B` for
  the public key, `R = r*B` for the nonce); Ed25519 _verify_ gains ~1.6x (**84.3 -> 51.8 ms**, only the
  `S*B` half is fixed-base - `h*A` stays a variable-base ladder). Byte-exact vs RFC 8032 sec 7.1 on-device
  (KAT: pubkey + sign + verify) and the `ssh_gf` path (native suite) is unchanged. The table is S3-only
  (flash, ~24 KB, zero RAM).
- **Prefer Ed25519 host keys over RSA.** An `ssh-ed25519` host-key signature is **19.4 ms**; an RSA-2048
  signature is **270 ms** (~14x slower). RSA _verify_ is cheap (16.5 ms, public exponent 65537), but the
  server pays the _sign_ cost on every handshake. The RSA sign was **440 ms** until the host key was cached:
  `ssh_rsa_sign` used to re-read and re-parse the NVS key on every call, and re-parsing threw away mbedTLS's
  per-context blinding state so it re-ran the ~167 ms first-use blinding init (`r^-1 mod n`) every sign. On a
  device-CCOUNT breakdown the 440 ms split as ~3 ms NVS read + 0.2 ms parse + 270 ms CRT modexp + **167 ms
  wasted blinding re-init**. Parsing the key once at startup and reusing the context ([`ssh_rsa.cpp`](../src/network_drivers/presentation/ssh/crypto/ssh_rsa.cpp),
  `SshRsaCtx`, mutex-guarded) drops the sign to **270 ms** with blinding fully preserved. The residual 270 ms
  is already CRT (two 1024-bit half-exponentiations) on the RSA/MPI hardware accelerator (Montgomery in
  silicon), so it is not reducible in software - the modexp is not the software big-integer multiply that
  Xtensa-assembly / CIOS tricks target, it is a hardware peripheral.
- **ECDSA P-256 now rides the same RSA/MPI MODMULT as curve25519** (sign **54 ms**, verify **103 ms**, ECDH
  **51 ms** - **~2.7-2.9x** the old mbedtls ECP path, which measured 146 / 291 / 140 ms). A self-contained
  P-256 ([`ssh_ecdsa.cpp`](../src/network_drivers/presentation/ssh/crypto/ssh_ecdsa.cpp)) does every field
  and scalar multiply as one 256-bit MODMULT - the accelerator is modulus-generic, so the same engine serves
  the field (mod p) and the scalar ring (mod n) by swapping the `{M, m', R^2}` constants. Point math uses
  exception-free complete (Renes-Costello-Batina) formulas under a constant-time 4-bit-window ladder, the
  curve `a = -3` multiply is folded to `-3x` (two adds, no MODMULT), and signing is RFC 6979 deterministic
  so the on-device output is byte-exact to the published KATs. It is still ~2x curve25519 (23 ms KEX / 85 ms
  sign) because a short-Weierstrass complete addition needs ~17 field muls vs the Montgomery ladder's
  handful, so curve25519 stays the first-offered, cheapest option; P-256 is offered for interop.
- **classic DH is expensive:** `diffie-hellman-group14` is a 2048-bit modexp at **181 ms** - another reason
  `curve25519-sha256` (23 ms) is the preferred, first-offered KEX.
- **PQC is affordable:** ML-KEM-768 encapsulation is **23.5 ms** in pure software (SW NTT), about the cost of
  one X25519. The `mlkem768x25519` hybrid KEX therefore roughly doubles the KEX field-crypto time (~46 ms) -
  a small fixed add on top of a handshake already dominated by the host-key signature.
- **The HKDF/KDF layer is cheap** (~104-108 us each): three HMAC-SHA256 calls on the HW SHA engine. TLS 1.3 /
  QUIC / DTLS all key-schedule through the same `expand_label` at this cost.
- **HW SHA is ~19 MB/s** (SHA-256); HMAC halves it (two hash passes plus key blocks).
- **ChaCha20 / Poly1305 (the default SSH AEAD) sped up ~2x by compiling those two TUs at `-O2`.** These are
  pure-integer software ciphers. SIMD is **not** available: the ESP32-S3 PIE vector unit has only a
  _saturating_ 32-bit add (`ee.vadds.s32`) and no wrapping `ee.vadd.s32` (assembler-verified), and ChaCha's
  quarter-round is modular uint32 arithmetic, so it cannot be vectorized (nor via 16-bit halves - those adds
  saturate too). The real lever is that the whole library ships at the arduino framework's `-Os`; ChaCha runs
  **108,919 -> 46,434 cyc (2.35x, 2.3 -> 5.3 MB/s)** and Poly1305 **31,452 -> 24,904 cyc (1.26x)** at `-O2`,
  so `chacha20-poly1305` seal goes **154,194 -> 77,433 cyc (1.99x, 1.6 -> 3.2 MB/s)**. A per-translation-unit
  `#pragma GCC optimize("O2")` on `ssh_chacha20.cpp` / `ssh_poly1305.cpp` forces `-O2` for just those hot
  functions (byte-exact, RFC 8439 KATs unchanged) so the cipher is fast on any consumer's size-optimized
  build. AES-256-CTR (22.5 MB/s, HW) is still faster where an AEAD is not required.

### MQTT 3.1.1 client codec (DETWS_ENABLE_MQTT)

The device is an MQTT client; these are its pure packet build/parse hot ops - `mqtt_build_connect` /
`mqtt_build_publish` (TX) and `mqtt_parse_publish` (the inbound-message decode). Host figures from
[`perf/bench_mqtt.cpp`](../perf/bench_mqtt.cpp); the device figure is the rig `/bench` CCOUNT op
(`mqtt_build_publish`, QoS 1 to a telemetry topic).

| Operation                    | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ---------------------------- | ---------: | --------: | --------------: | -------------: |
| `mqtt_build_connect`         |       24.6 |    1055.1 |               - |              - |
| `mqtt_build_publish` (QoS 1) |       44.9 |    1446.9 |            1008 |           4200 |
| `mqtt_parse_publish`         |       60.6 |    1073.4 |               - |              - |

- Building a PUBLISH is ~4.2 us on the S3 (a length-prefixed topic + the variable-length Remaining Length
  field + the payload copy) - cheap; an MQTT client can publish at a high rate without the encode being
  the limit. The network round trip and the broker, not the codec, set the publish throughput ceiling.

### Redis RESP2/RESP3 codec (DETWS_ENABLE_REDIS)

The Redis wire codec a device uses to talk to a Redis server: `resp_encode_command` (build an outbound
command) and `resp_parse` (decode one value of a server reply - the untrusted-input cursor). Both pure.
Host figures from [`perf/bench_redis.cpp`](../perf/bench_redis.cpp); the device figure is the rig
`/bench` CCOUNT op (`resp_parse` of an array header).

| Operation                             | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------------------------- | ---------: | --------: | --------------: | -------------: |
| `resp_encode_command` (SET)           |       66.4 |     708.1 |               - |              - |
| `resp_parse` (array reply, per value) |       64.7 |     432.5 |             244 |           1016 |

- `resp_parse` decodes one value per call at ~1.0 us on the device (it is a **cursor**, not a recursive
  descent - an aggregate reports its child count and the caller loops, so there is no per-depth stack cost
  and a deeply-nested reply cannot blow the stack). Encoding a command is comparably cheap. RESP is the
  lightest of the client codecs; the Redis round-trip cost is the network, not the parse.

### FTP client wire codec (DETWS_ENABLE_FTP)

The FTP control-channel codec a device uses to push/pull files (RFC 959 + RFC 2428): `ftp_build_command`
(emit a `VERB<SP>ARG` line), `ftp_parse_reply` (scan a single- or multi-line 3-digit reply over a fixed
buffer - the untrusted-input hot op), and `ftp_parse_pasv` (decode the `227 (h1,h2,h3,h4,p1,p2)` data
address). All pure (the two sockets are the application's). Host figures from
[`perf/bench_ftp.cpp`](../perf/bench_ftp.cpp); the device figure is the rig `/bench` CCOUNT op
(`ftp_parse_reply` of a 3-line FEAT block).

| Operation                           | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ----------------------------------- | ---------: | --------: | --------------: | -------------: |
| `ftp_build_command` (STOR)          |       22.4 |     893.1 |               - |              - |
| `ftp_parse_reply` (multiline FEAT)  |       46.8 |     961.2 |             700 |           2916 |
| `ftp_parse_pasv` (227 data address) |       67.1 |     759.6 |               - |              - |

- `ftp_parse_reply` walks a possibly-multiline reply to the `NNN<SP>` terminator over a **fixed 512 B**
  control buffer (no heap), so an oversized or never-terminated multiline cannot over-read or over-buffer -
  it just returns "need more" until the caller's deadline (validated by the `ftp_malicious_server` attack).
  At ~2.9 us on the device the parse is free relative to the control round trip; FTP throughput is the data
  channel, not the codec. HW-verified device-as-FTP-client against a real `pyftpdlib` server (11/11 interop
  checks, the STOR confirmed server-side).

### SMTP client dialogue (DETWS_ENABLE_SMTP)

The outbound SMTP client that sends a device alert email (RFC 5321): `smtp_run` drives the whole
exchange - read the greeting, `EHLO`, optional `AUTH LOGIN`, `MAIL FROM` / `RCPT TO` / `DATA`, build the
message (CRLF-normalize + RFC 5321 sec 4.5.2 dot-stuffing) and stream it, then `QUIT`. It is pure (a
send/recv seam), so the whole dialogue is benched end to end over a **scripted in-memory transport** (canned
server replies, sink send) - the reply parser (`reply_complete`, over a fixed 512 B buffer) plus the message
builder together, one alert email's worth of work. Host from [`perf/bench_smtp.cpp`](../perf/bench_smtp.cpp);
device from the rig `/bench` `smtp_run` op.

| Operation                          | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 us/op |
| ---------------------------------- | ---------: | --------: | --------------: | -------------: |
| `smtp_run` (full 7-reply dialogue) |      936.2 |      42.7 |           13564 |           56.5 |

- The full dialogue - seven reply parses, the `EHLO`/`MAIL`/`RCPT` command builds, and the message
  build with dot-stuffing - costs **~56 us** on the device (pure compute; the real send-alert latency is the
  ~7 network round trips). This is the heaviest single "client op" benched so far because it is an entire
  protocol exchange, not one codec call, yet it is still trivial next to the network. Every buffer is a
  compile-time size (`DETWS_SMTP_REPLY_MAX` 512, `DETWS_SMTP_MSG_MAX` 2048), so a malicious server cannot
  grow the client's footprint (validated by the `smtp_malicious_server` attack). HW-verified
  device-as-SMTP-client against a real `aiosmtpd` server (6/6 interop, the message confirmed server-side).

### syslog client formatter (DETWS_ENABLE_SYSLOG)

The RFC 5424 syslog client formats one `<PRI>1 - HOSTNAME APP-NAME - - - MSG` line per log call and ships
it as a UDP datagram (`det_udp_sendto`). `syslog_format` is the pure per-line hot op (no socket, no heap).
Host from [`perf/bench_syslog.cpp`](../perf/bench_syslog.cpp); device from the rig `/bench` `syslog_format`
op.

| Operation                  | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| -------------------------- | ---------: | --------: | --------------: | -------------: |
| `syslog_format` (RFC 5424) |      159.1 |     452.5 |            3686 |          15358 |

- At **~15 us** on the device the format is dominated by `snprintf` composing the header + four field
  substitutions (newlib `snprintf` is not cheap on Xtensa - it is ~10x the leaner hand-rolled codecs). Still
  trivial for a log line, and the datagram is fire-and-forget over UDP. The line is bounded to
  `DETWS_SYSLOG_MSG_MAX` (256 B): an oversized message makes `syslog_format` return 0 and `syslog_log`
  refuse (no overflow, no giant datagram - validated by the `syslog_injection` attack, which held the bound
  at an 80 B datagram for a 2 KB input). HW-verified device-as-syslog-client against a UDP collector (7/7
  interop; PRI/VERSION/HOSTNAME/APP-NAME/MSG validated as RFC 5424).

### NTP server (DETWS_ENABLE_NTP_SERVER)

The device answers NTP requests on UDP/123 from its own clock (RFC 5905 server mode).
`ntp_server_build_response` is the per-query hot op: it validates the 48-octet request, echoes the client's
version + transmit stamp into the origin field, and stamps the reference/receive/transmit timestamps. Pure
(no clock, no socket). Host from [`perf/bench_ntp.cpp`](../perf/bench_ntp.cpp); device from the rig `/bench`
op.

| Operation                        | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| -------------------------------- | ---------: | --------: | --------------: | -------------: |
| `ntp_server_build_response` (48) |       10.5 |    9108.3 |             328 |           1366 |

- The **cheapest server op benched** - a fixed 48-octet build at **~1.4 us** on the device (memset + a few
  field writes; no parse loop). More important than the speed is the **shape**: the reply is always exactly
  48 octets and a request shorter than 48 gets none, so **reply size never exceeds request size** - the
  server has **no amplification factor** (unlike an ntpd MONLIST reflector). The `ntp_server_abuse` attack
  confirmed max reply/request = **1.00x** across 1..2048-octet requests, plus all 64 mode/version combos and
  malformed/oversized packets handled without a crash. HW-verified against the real `ntplib` client (6/6
  interop: mode 4, stratum, origin echo, LOCL ref-id, a plausible epoch).

### NTS framing, RFC 8915 (DETWS_ENABLE_NTS)

Network Time Security wraps NTP with a TLS-1.3 key-establishment exchange (NTS-KE on :4460) and per-packet
authenticated extension fields. These are the pure framing hot ops - no TLS, no AEAD, no socket - from
[`perf/bench_nts.cpp`](../perf/bench_nts.cpp): the client builds a KE request and parses the server's record
stream once per key establishment, and stamps a Unique-Identifier + Cookie EF on every protected NTP
request. The AES-SIV-CMAC-256 AEAD + the TLS-exporter key derivation sit on top and are not part of these ops.

| Operation                    | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ---------------------------- | ---------: | --------: | --------------: | -------------: |
| `ke_request` build           |        3.3 |    4779.6 |               - |              - |
| `ke_parse` (server response) |       18.9 |    2755.6 |             270 |           1125 |
| `ef_unique_id` build         |        7.9 |    2525.0 |               - |              - |
| `ef_cookie` build            |        7.9 |    4541.9 |               - |              - |

- Framing is trivially cheap: the real cost of NTS is the TLS-1.3 handshake + the AES-SIV AEAD, not the
  record/EF assembly. The `ke_parse` walk (validate each `[critical|type][len][body]` TLV to the
  End-of-Message record) is the heaviest - **~1.1 us (270 cyc) on the ESP32-S3** for a 4-record response
  (host ~19 ns), from the rig `/bench` op. So key establishment parses in a microsecond; interop against a
  real NTS client and a malformed-record attack remain the open NTS coverage.

### DNS server (DETWS_ENABLE_DNS_SERVER)

An authoritative DNS server on UDP/53 answers A/IN queries from a fixed table (NXDOMAIN otherwise).
`det_dns_server_build_response` is the per-query hot op: it parses the first question, resolves the name via a
callback, and appends one compressed A answer (or NXDOMAIN / NOTIMP). Pure (no clock, no socket). Host from
[`perf/bench_dns.cpp`](../perf/bench_dns.cpp); device from the rig `/bench` op.

| Operation                               | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| --------------------------------------- | ---------: | --------: | --------------: | -------------: |
| `det_dns_server_build_response` (hit)   |       26.8 |     971.4 |             420 |           1750 |
| `det_dns_server_build_response` (NXDOM) |       25.4 |    1022.1 |               - |              - |

- A **~1.75 us** query on the device (parse the question labels + emit a 16-octet answer). The security
  shape matters more than the speed: DNS is the #1 reflection/amplification vector, and this server closes
  both doors - it is **authoritative-only** (an unconfigured name gets NXDOMAIN, never resolved, so it is
  **not an open resolver**) and an A answer is only **~1.6x** the query (no amplification, vs the 10-50x of a
  real reflector). It also **rejects compression pointers in the question** (`len & 0xC0` -> drop), so the
  classic compression-pointer parser loop cannot fire. The `det_dns_server_abuse` attack confirmed
  `open_resolver=False`, `max_amp=1.64x`, and 14 malformed/pointer/opcode cases handled with the server up.
  HW-verified against the real `dnspython` client (11/11 interop: three A records + AA flag + NXDOMAIN).

### NATS client codec (DETWS_ENABLE_NATS)

The text pub/sub codec a device uses to talk to a NATS server: `nats_build_pub` (publish) and `nats_parse`
(decode one inbound server frame - INFO/MSG/PING/+OK/-ERR, the untrusted-input hot op). Both pure. Host from
[`perf/bench_nats.cpp`](../perf/bench_nats.cpp); device from the rig `/bench` `nats_parse` op.

| Operation          | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------ | ---------: | --------: | --------------: | -------------: |
| `nats_build_pub`   |       33.8 |    1863.9 |               - |              - |
| `nats_parse` (MSG) |      139.4 |     466.2 |            1573 |           6554 |

- `nats_parse` decodes one frame per call at **~6.5 us** on the device (the MSG path tokenizes
  subject/sid/reply/size, then bounds the payload byte-count against the buffer - `size > len - after_line -
2` returns "need more", so a byte-count lie can never over-read or over-allocate). Building a PUB is a
  cheap line emit. NATS is a light line-oriented protocol; the round-trip cost is the network, not the codec.
  HW-verified device-as-NATS-client against a real `nats-server` (7/7 interop: INFO/CONNECT/SUB/PUB and the
  PUB delivered to an independent subscriber through the broker); the `nats_malicious_server` attack held all
  10 malformed-frame personalities.

### STOMP 1.2 frame codec (DETWS_ENABLE_STOMP)

The STOMP 1.2 frame codec a device uses to talk to a message broker: `stomp_build_frame` (emit a
SEND/SUBSCRIBE) and `stomp_parse_frame` (decode one inbound frame - command + headers + content-length body,
the untrusted-input hot op). Both pure. Host from [`perf/bench_stomp.cpp`](../perf/bench_stomp.cpp); device
from the rig `/bench` `stomp_parse_frame` op.

| Operation           | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------- | ---------: | --------: | --------------: | -------------: |
| `stomp_build_frame` |      117.1 |     597.8 |               - |              - |
| `stomp_parse_frame` |      136.3 |     755.5 |            2314 |           9641 |

- `stomp_parse_frame` decodes one frame per call at **~9.6 us** on the device (scan the command + header
  lines, then take the body by `content-length`, which is bounded against the buffer - a declared length past
  the buffered bytes returns "need more" and a length that does not land on the terminating NUL is rejected,
  so a content-length lie can never over-read; the header count is capped at `DETWS_STOMP_MAX_HEADERS`).
  Building a frame escapes the header octets. HW-verified device-as-STOMP-client against an independent STOMP
  1.2 broker (7/7 interop: CONNECT/SUBSCRIBE/SEND and the SEND captured server-side); the
  `stomp_malicious_broker` attack held all 10 malformed-frame personalities.

### StatsD metrics client (DETWS_ENABLE_STATSD)

The StatsD line client the device uses to push metrics: `statsd_format` builds one `name:value|type[|@rate]
[|#tags]` line and the emit helpers `det_udp_sendto` it (fire-and-forget UDP). `statsd_format` is the pure
per-metric hot op. Host from [`perf/bench_statsd.cpp`](../perf/bench_statsd.cpp); device from the rig
`/bench` `statsd_format` op.

| Operation                        | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| -------------------------------- | ---------: | --------: | --------------: | -------------: |
| `statsd_format` (counter + tags) |       47.4 |     971.4 |            1052 |           4383 |

- A **~4.4 us** metric on the device (hand-rolled integer/rate rendering, then a bounded assemble - no
  `printf`). The line is capped at `DETWS_STATSD_LINE_MAX` (256 B): an oversized name/value/tags makes
  `statsd_format` return 0 and the metric is dropped (no overflow, no giant datagram - validated by the
  `statsd_injection` attack, which held the bound at a 51 B datagram for a 2 KB name). HW-verified
  device-as-StatsD-client against a UDP collector (5/5 interop; name/value/type validated).

### JWT HS256 bearer-auth verify (DETWS_ENABLE_JWT)

The per-request bearer-token check the device runs to authenticate a caller: `jwt_verify_hs256` splits the
compact JWT, enforces `alg == HS256` (rejecting `alg=none` / RS256 / HS384 before any HMAC), HMAC-SHA256s the
signing input, base64url-encodes the MAC, and constant-time compares it. Host from
[`perf/bench_jwt.cpp`](../perf/bench_jwt.cpp); device from the rig `/bench` op.

| Operation          | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 us/op |
| ------------------ | ---------: | --------: | --------------: | -------------: |
| `jwt_verify_hs256` |     3243.0 |      43.5 |           33838 |          140.9 |

- At **~141 us** this is the **heaviest per-request op** in the library after WS inflate - HMAC-SHA256 is two
  SHA-256 compressions plus the base64url. That is fine for a login / occasional bearer check, but a route
  hit at high request rate should cache the decision rather than re-verify every request. The important
  property is the **security shape**: the `alg` header is enforced before the MAC (RFC 7515 §5.2), the
  signature must be exactly 43 base64url chars, and the compare is constant-time - so the whole
  alg-confusion / forgery class is closed. The `jwt_forgery` attack confirmed **14/14 forgeries rejected**
  (alg=none, a valid HMAC under alg=HS384/RS256/None, wrong-secret, stripped/bit-flipped/truncated signature,
  extra dots, oversized) with a genuinely valid token as the positive control. HW-verified against the real
  `PyJWT` library (6/6 interop: valid accepted, wrong-secret rejected, expired rejected by the `exp` check).

### TLS handshake (DETWS_ENABLE_TLS)

Unlike the codecs above, TLS is not a pure host op - it is an mbedTLS handshake terminated on the device from
its 48 KB static BSS arena (zero heap, no `malloc`), so the meaningful number is the **device-side wall time**
of a full handshake, measured end to end from a real client (Python `ssl` on the RPi) against the slim HTTPS
rig on an ESP32-S3. Cipher suite `ECDHE-ECDSA-AES256-GCM-SHA384` (P-256), self-signed ECDSA leaf.

| Operation (ECDHE curve)                          | ESP32-S3 min | avg     | Rate (full req) |
| ------------------------------------------------ | -----------: | ------- | --------------: |
| TLS 1.2 handshake - **before** (secp521r1)       |       870 ms | 1075 ms |      ~1.0 req/s |
| TLS 1.2 handshake - **after** (x25519/secp256r1) |   **487 ms** | 501 ms  |      ~2.0 req/s |

- **Curve preference cut the handshake 2.05x (1000 ms -> 487 ms), no downside.** The cost of a full
  handshake is dominated by one asymmetric op: the ECDHE **variable-base scalar multiply** in software
  mbedTLS (no ECC accelerator on the S3). mbedTLS, given no server preference, negotiates the _first_ curve
  in its own list that the client offers - which on the esp-idf build is **secp521r1, the most expensive**
  (its scalar mult is ~2.4x a P-256/x25519 one). The library now pins a fast, modern preference order
  (`mbedtls_ssl_conf_curves`/`_groups`: x25519, secp256r1 ahead of secp384r1/secp521r1) on both the server
  and outbound-client config, so the negotiated curve is a cheap 128-bit one. Every curve stays enabled -
  this only reorders preference, so a peer that supports just one still connects.
- Decomposition (CCOUNT `us/op` on the S3, `/bench/tls`): ECDHE shared-secret (variable base) **~142 ms**
  (x25519) / **~142 ms** (P-256) / **~333 ms** (P-521); ECDHE ephemeral gen (fixed base) ~57-142 ms; the
  fixed **ECDSA-P256 server signature is only ~63 ms**. So the _curve_ is what moves the handshake, not the
  signature - which is why the curve preference is the whole win and a P-256 field-math rewrite would not
  help (mbedTLS already uses the HW MPI for the NIST reduction, and it is precompiled in the core anyway).
- Still ~0.5 s per _full_ handshake, so the practical guidance stands: **keep connections alive** and/or
  enable `DETWS_ENABLE_TLS_RESUMPTION` (RFC 5077 tickets) so a returning client skips the ECDHE+ECDSA cost
  entirely. **Measured resumed handshake: ~54 ms (full 509 ms -> resumed 48-57 ms, ~10x)**, HW-verified with
  OpenSSL reporting `Reused` (session saved with `-sess_out`, resumed with `-sess_in`, against the rig built
  with resumption on). Stacked with the curve preference, a returning client goes from the original ~1000 ms
  (unconfigured, secp521r1, no resumption) to ~54 ms - about 18x. The device work in a resumed handshake is
  only symmetric (ticket AES-256-GCM decrypt + key schedule, both cheap); the ~54 ms wall time is mostly the
  client/TCP round trip. Resumption is **off by default** (a deliberate attack-surface choice - the server
  holds a ticket-sealing key; note TLS 1.2 has no 0-RTT so there is no early-data replay, and a replayed
  ticket still cannot complete the handshake without the master secret). Bulk AES-256-GCM record encryption
  after the handshake is comparatively free.
- HW-verified: `curl`/browsers/OpenSSL/Python (all 1.3-leading) negotiate down to TLS 1.2 and get `200`;
  `tls_server_abuse` held every invariant (downgrade + weak-cipher refused, 7 malformed handshakes survived).
  Bringing this up found and fixed two hardware-only bugs (a tcpip_thread self-deadlock and an RX ring smaller
  than a modern ClientHello) - see [BUGS.md](BUGS.md).

### HTTP/2 over TLS (DETWS_ENABLE_HTTP2, PSRAM)

HTTP/2 rides the TLS handshake (ALPN `h2`) and adds the binary framing + HPACK + a per-connection stream
engine whose ~28 KB-per-conn pool lives in PSRAM. Measured device-as-server on the ESP32-S3 from a real h2
client (Python `httpx`, backed by the `h2` library) against the PSRAM h2 rig.

| Operation                                     | ESP32-S3            | Notes                                             |
| --------------------------------------------- | ------------------- | ------------------------------------------------- |
| Cold connect (TLS + ALPN h2 + first GET /)    | ~452 ms median      | TLS handshake dominates (see above)               |
| Warm request (established connection, GET /)  | ~51 ms/req (19.5/s) | HEADERS/DATA + HPACK per stream, h2 pool in PSRAM |
| Effective concurrent streams (one connection) | ~1-2                | excess streams are reset; device stays up         |

- On a warm connection a request is **~51 ms** - the h2 framing (HPACK encode/decode, frame assembly) plus the
  PSRAM-resident stream pool (external RAM is slower than internal DRAM) cost more than a plaintext HTTP/1.1
  hit. The engine serves **sequential** streams reliably but **serializes** concurrent ones: a burst of 4-8
  simultaneous streams completes only ~1-2 and resets the rest (the client sees a stream error), which is the
  bounded-resource, deterministic trade-off - it never grows memory to absorb the fan-out, and the device
  stays alive. Practicality: HTTP/2 here buys ALPN interop + header compression + one long-lived connection,
  not high stream parallelism; drive it with modest concurrency.
- HW-verified: the `http2` interop peer is 7/7 (ALPN h2, multiplexed streams, JSON/text bodies) and the
  `h2_abuse` attack is held (rapid-reset / CONTINUATION-flood / HPACK-bomb / PING-flood, 0 findings). Bring-up
  fixed a core-locking `tcp_write` assert on the IDF-5.5 PSRAM core - see [BUGS.md](BUGS.md).

### HTTP/3 over QUIC (DETWS_ENABLE_HTTP3, PSRAM)

HTTP/3 runs the whole QUIC + TLS-1.3 stack in the library (no mbedTLS): X25519 key exchange, an Ed25519
CertificateVerify, AES-128-GCM packet protection, and QPACK, with the QuicConn/H3Conn pool + ingest ring in
PSRAM. Measured device-as-server on the ESP32-S3 from a real QUIC client (Python `aioquic`) against the PSRAM
h3 rig.

| Operation                                   | ESP32-S3    | Notes                                                 |
| ------------------------------------------- | ----------- | ----------------------------------------------------- |
| Cold connect (QUIC handshake + first GET /) | ~940-994 ms | measured pre-accel; X25519 + Ed25519 now both HW [S3] |
| Request streams served per QUIC connection  | ~2          | static stream budget; no MAX_STREAMS credit renewal   |

- The **~0.95 s** cold connect is the heaviest of the three transports (TLS 1.2 ~0.9 s, h2 cold ~0.45 s): the
  QUIC handshake adds an Ed25519 signature over the transcript on top of the X25519 exchange (the same
  `ssh_x25519` / `ssh_ed25519` path, ~10.5 KB of stack). Both now run their field arithmetic on the RSA/MPI
  hardware accelerator on the S3 (`DETWS_FE25519_MPI_HW`: X25519 97.5 -> 22.65 ms, ed25519_sign 380 -> 85.6 ms;
  see the SSH crypto section), which should cut the QUIC handshake's crypto (one X25519 + one Ed25519 sign) from
  ~0.46 s to ~0.11 s - re-measuring the cold-connect figure on a rebuilt h3 rig is a follow-up. Once up, a
  connection serves about **two**
  request streams - the static per-connection stream table's initial budget - and does not renew stream credit
  (it never emits MAX_STREAMS), so a third request stalls; a client that needs more opens a fresh connection.
  This is the bounded-resource, zero-growth trade-off, not high request parallelism. Practicality: HTTP/3 here
  is a standards-complete QUIC/h3 endpoint for a handful of requests (config, status, a secure POST), not a
  high-throughput API surface.
- HW-verified: the `http3` interop peer is 5/5 via aioquic (QUIC handshake, GET / -> 200 over QPACK + DATA,
  multiplexed second stream). Bring-up fixed two hardware-only bugs - a worker-stack overflow in the QUIC
  Ed25519 signer and a frame parser that rejected a real client's post-handshake frames - see
  [BUGS.md](BUGS.md).
- Re-confirmed 2026-07-13 over a **W5500 wired link** (not just WiFi) and with a **second independent client**:
  `curl --http3` (OpenSSL QUIC + nghttp3) completes the exchange at HTTP/3, `:status` 200 on both `/` and
  `/status`, and reports `Certificate level 0: ED25519, signed using ED25519` - so a real client **validates
  the Ed25519 CertificateVerify signature**, not only the aioquic path (which disables verification). Two
  mature stacks now agree on the wire against the on-device server.

## 3. Request-path benchmarks

The CPU cost of a request's hot path: the standalone HTTP/1.1 request parser and the zero-heap JSON
writer / reader (`perf/bench_reqpath.cpp` on the host, the same ops in an on-device firmware for the
ESP32-S3 column). Host = Raspberry Pi 5 (Cortex-A76, `-O2`), a relative baseline; ESP32-S3 = the real
device at 240 MHz, timed over in-RAM buffers so this is pure compute, not socket I/O. Byte counts: the GET
request is ~220 bytes over 6 headers, the POST is ~150 bytes with a 50-byte JSON body, the encoded doc is
96 bytes, the decoded body is 50 bytes.

| Feature    | Operation              | Host ns/op | Host MB/s | ESP32-S3 us/op | ESP32-S3 MB/s |
| ---------- | ---------------------- | ---------: | --------: | -------------: | ------------: |
| http_parse | GET (6 headers)        |     1692.0 |     131.8 |         83.304 |           2.7 |
| http_parse | POST + JSON body       |     1104.0 |     139.5 |         54.620 |           2.8 |
| json       | encode (8 fields, 96B) |      675.6 |     142.1 |         49.693 |           1.9 |
| json       | decode (4 fields)      |      263.6 |     189.7 |         19.765 |           2.5 |

The GET/POST device figures are direct CCOUNT reads from the rig `/bench/reqparse` endpoint (reset + a full
`http_parser_feed` of the request), at 240 MHz over in-RAM buffers.

Notes:

- **Char-class table optimization (2026-07-12).** The per-byte parser classifies every request octet
  (method/field-name `tchar`, path/query `vchar`, header-value bytes). Folding the three branch-heavy
  classifiers into one 256-entry const table (flash `.rodata`, one load + a mask bit) and dropping a redundant
  terminal-state guard switch cut the parser measurably - **device `http_parser_feed` -9.7% on the GET
  (21146 -> 19088 cyc, ~94.8 -> ~85.6 cyc/byte) and -9.1% on the POST**, host `-O2` GET -12% - with the
  parser's 93 unit tests unchanged. So the request path _was_ worth an optimization after all (the earlier
  "not warranted" note predated a direct CCOUNT measurement of the feed loop).
- **`http_parser_reset` is the cheap part.** Zeroing the whole `HttpReq` (2624 bytes) once per request costs
  **905 cyc / ~3.8 us** on the device - only ~4% of a GET. The dominant per-request core cost is `feed()`, not
  the reset, so the reset's simple full-struct clear (no stale-data risk) is kept.
- **The parser is byte-at-a-time by design.** `http_parser_feed()` is a pure per-byte state machine with no
  look-ahead or buffering, so its cost scales with request length (~0.36 us/byte on the device) and it can
  consume bytes exactly as they arrive off the socket - the parse overlaps the network read instead of
  waiting for a whole request. That streaming design is why a ~220-byte GET costs more than a ~150-byte POST
  here even though the POST carries a body. Real request latency is still dominated by the TCP round trip and
  (when enabled) the TLS handshake, not by parsing.
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

### DNP3 data-link codec, IEEE 1815 (DETWS_ENABLE_DNP3)

The SCADA / utility-outstation link layer: a CRC-16/DNP (poly 0x3D65, reflected) over the header block and
every 16-octet data block, a zero-heap frame builder, and a CRC-validating de-blocking parser. Pure (no
socket). Host from [`perf/bench_dnp3.cpp`](../perf/bench_dnp3.cpp).

| Operation                     | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ----------------------------- | ---------: | --------: | --------------: | -------------: |
| `crc16` (16-octet block)      |      215.4 |      74.3 |               - |              - |
| `build_frame` (32 B user)     |      548.3 |      83.9 |               - |              - |
| `parse_frame` (validate CRCs) |      553.1 |      83.2 |            3753 |          15637 |

- The **CRC-16/DNP is the hot inner op** (~215 ns/block host): build + parse each run one header CRC + one
  CRC per data block, so a 32-byte frame (header + 2 blocks) is ~3 CRCs and lands at ~550 ns host / **~15.6
  us (3753 cyc) on the ESP32-S3** (parse, from the rig `/bench` op). The **table-less bit-reflected CRC is
  the whole cost** on-device (~28x the host); a 256-entry lookup table would trade ~512 B flash for a large
  speedup if a DNP3 outstation ever needed line-rate framing. First of the industrial/SCADA family (dnp3, iec60870, mms, goose, s7comm, enip, profinet,
  ...) to get a bench - **all are implemented codecs**; their device us/op + interop + attack are the real
  remaining coverage work.

### BACnet/IP BVLC + NPDU codec, ASHRAE 135 (DETWS_ENABLE_BACNET)

The building-automation network layer over UDP/47808: the BVLC envelope (Annex J - type/function/length) and
the NPDU (Clause 6 - version/NPCI-control + optional DNET/DLEN/DADR + SNET/SLEN/SADR + hop count), build +
validate/slice. Pure (no socket). Host from [`perf/bench_bacnet.cpp`](../perf/bench_bacnet.cpp).

| Operation    | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------ | ---------: | --------: | --------------: | -------------: |
| `bvlc_parse` |        4.2 |    4785.6 |               - |              - |
| `npdu_parse` |        5.8 |    2736.7 |             101 |            420 |
| `npdu_build` |       15.8 |    1009.8 |               - |              - |

- Trivially cheap: BVLC/NPDU are fixed-field framing with no CRC (BACnet relies on the UDP checksum), so
  parse is a handful of bounds checks + slices - **~0.42 us (101 cyc) on the ESP32-S3** for `npdu_parse`
  (from the rig `/bench` op), ~37x lighter than DNP3's per-block-CRC parse (15.6 us) despite both being
  ~30-octet frames. `npdu_build` costs more (writes the addressing fields). The `bacnet_frame_fuzz` parser
  attack HELD on HW (12 malformed BVLC/NPDU datagrams - bad type/version, length lies, DLEN/SLEN over-runs,
  truncation - all rejected, a valid one still parses).

### S7comm codec, Siemens S7 / ISO-on-TCP (DETWS_ENABLE_S7COMM)

The Siemens S7 application layer (over ISO-on-TCP / RFC 1006, TCP/102): the client Setup-Communication +
Read-Var (S7-ANY pointer) request builders and the response-header parser (protocol id 0x32 / ROSCTR /
param + data lengths). Pure (no socket). Host from [`perf/bench_s7comm.cpp`](../perf/bench_s7comm.cpp).

| Operation                | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ------------------------ | ---------: | --------: | --------------: | -------------: |
| `build_setup`            |        4.2 |    5270.0 |               - |              - |
| `build_read_request` (3) |       16.3 |    2951.6 |               - |              - |
| `parse_header`           |        5.0 |    9584.9 |             129 |            537 |

- Fixed-field framing, no CRC (S7 rides ISO-on-TCP + the TCP checksum) - a few ns to build/parse host,
  **~0.54 us (129 cyc) on the ESP32-S3** for `parse_header` (rig `/bench` op). The 3-item Read Var build
  costs most (writes three 12-octet S7-ANY item pointers). The `s7comm_frame_fuzz` parser attack HELD on HW
  (11 malformed PDUs - bad protocol id / ROSCTR, param/data length lies, truncated + Ack_Data-truncated
  headers - all rejected, a valid PDU still parses).

### IEC 60870-5-104 codec, SCADA telecontrol (DETWS_ENABLE_IEC60870)

The utility telecontrol protocol over TCP: the -104 APCI (`68 LEN` + 4 control octets, I/S/U formats) and
the ASDU header (type id / SQ / count / cause-of-transmission / common address). Pure (no socket). Host
from [`perf/bench_iec60870.cpp`](../perf/bench_iec60870.cpp).

| Operation           | Host ns/op | Host MB/s | Device (S3) cyc | Device us/op |
| ------------------- | ---------: | --------: | --------------: | -----------: |
| `build_i` (I-frame) |        8.3 |    2156.7 |               - |            - |
| `parse` (APCI)      |        5.8 |    3083.4 |             112 |         0.47 |
| `asdu_parse_header` |        5.0 |    2398.6 |               - |            - |

- Fixed-field framing, no CRC on the -104 path (it rides the TCP checksum; only the serial -101 FT1.2 frame
  carries a sum check) - a few ns to build/parse host, **~0.47 us (112 cyc) on the ESP32-S3** for `iec104_parse`
  (APCI start/length validate + I/S/U decode + ASDU slice, rig `/bench` op) - sits between BACnet's `npdu_parse`
  (0.42 us) and S7's `parse_header` (0.54 us): all three are the same fixed-field-bounds-check-and-slice class,
  ~33x lighter than DNP3's per-block-CRC parse (15.6 us). The `iec104_frame_fuzz` parser attack **HELD on HW**
  (rig `/iec104/parse`: 14/15 malformed APDUs handled - bad start octet (!=0x68), length under/over-run,
  truncated APCI, degenerate I/S/U control, a short chained ASDU header, all-0xFF - each rejected or sliced
  without over-reading, a valid I-frame still parsed to its type-9 ASDU; the 15th, an oversized 2 KB blob, is
  refused by the 256 B body cap; free heap flat across repeat runs after the warm-up first-touch). Interop still
  needs the IEC-60870 **app/role layer** (interrogation + spontaneous reporting) built on the codec.

### IEC 61850 MMS codec (DETWS_ENABLE_MMS)

The client/server core of IEC 61850 (MMS / ISO 9506 over ISO-on-TCP/102): the confirmed-request Read builder
(a nested BER encoding of the ACSI ObjectName for a Data Object reference), the confirmed-response Read-data
builder, and the confirmed-PDU header parser. Pure (no socket, no TPKT/COTP). Host from
[`perf/bench_mms.cpp`](../perf/bench_mms.cpp).

| Operation               | Host ns/op | Host MB/s |
| ----------------------- | ---------: | --------: |
| `read_request` (build)  |      101.3 |     404.8 |
| `read_response` (build) |       52.8 |     246.2 |
| `parse` (confirmed PDU) |        8.5 |    4807.9 |

- The **parse is trivially cheap (~8.5 ns)** - it only walks the outer confirmed-PDU BER TLVs (tag + `invokeID`
  INTEGER + service tag) and slices the service body, no recursion. The **request build (~101 ns) is the
  heaviest** because it lays down the deeply nested `confirmed-request > read > variableAccessSpecification >
listOfVariable > objectName` BER structure around the ObjectName VisibleString (~7 nested length-prefixed
  TLVs), computing each length back-to-front. Still a fixed-shape encode, no allocation. Device us/op via the
  rig `/bench` op and an `mms_frame_fuzz` parser attack are the next MMS increments (same shape as the rest of
  the SCADA family); interop needs the IEC-61850 **ACSI app layer** (the object model + report control blocks)
  on top of the codec.

### IEC 61850 GOOSE publisher codec (DETWS_ENABLE_GOOSE)

GOOSE (Generic Object Oriented Substation Event) is the fast raw-L2 multicast IEC 61850 uses for protection
trips: an Ethernet frame (ethertype 0x88B8) + an 8-octet GOOSE header + the BER `IECGoosePdu` (11 control
fields - gocbRef / stNum / sqNum / allData / ...). Pure (no socket). Host from
[`perf/bench_goose.cpp`](../perf/bench_goose.cpp).

| Operation             | Host ns/op | Host MB/s |
| --------------------- | ---------: | --------: |
| `goose_pdu` (build)   |      131.3 |     761.8 |
| `goose_frame` (build) |      134.5 |     907.3 |

- The **BER `IECGoosePdu` encode is the whole cost (~131 ns)** - 11 length-prefixed control fields plus the
  `allData` blob, same nested-TLV-back-to-front class as MMS's request builder. Wrapping it in the Ethernet +
  GOOSE header (`goose_frame`) adds only **~3 ns** (a 22-octet header memcpy), so a publish is ~135 ns of pure
  encode. GOOSE is **publish-only** here (no subscriber/parser), so unlike the rest of the SCADA family it has
  **no parser-fuzz attack surface** - it is a bench-only codec (like NTS). A device µs/op via the rig `/bench`
  op (build-into-buffer, no transmit) is the remaining increment; full interop needs a raw-L2 multicast
  subscriber and an Ethernet PHY (hardware the rig does not yet have).

### EtherNet/IP codec (DETWS_ENABLE_ENIP)

EtherNet/IP (CIP encapsulation over TCP/44818): the 24-octet encapsulation header (command / length /
session-handle / status / sender-context / options) + command data, plus the RegisterSession handshake and
SendRRData (which carries the CIP message). Pure (no socket). Host from
[`perf/bench_enip.cpp`](../perf/bench_enip.cpp).

| Operation                  | Host ns/op | Host MB/s |
| -------------------------- | ---------: | --------: |
| `eip_build` (encap)        |        7.5 |    4250.7 |
| `eip_parse` (encap)        |        4.9 |    6493.8 |
| `register_session` (build) |        2.9 |    9587.6 |

- **Fixed little-endian header, no BER, no CRC** - so it lands in the fast fixed-field class (~5-8 ns, like
  IEC-104's APCI), ~20x cheaper than the nested-BER MMS/GOOSE builders. `eip_parse` (~4.9 ns) validates the
  command/length/status and slices the command data - the receive op, and the surface an `enip_frame_fuzz`
  parser attack targets (a length lie must not over-read past the 24-octet header). Device µs/op via the rig
  `/bench` op + that attack are the next ENIP increments; interop needs a CIP object model + a peer (e.g.
  cpppo / an OpENer target) on top of the encapsulation codec.

### PROFINET DCP codec (DETWS_ENABLE_PROFINET)

PROFINET DCP (Discovery and Configuration Protocol, the raw-L2 device-discovery/naming layer): the 10-octet
DCP header (frameID / service / xid / dataLength), the header parser, and the block walker over
`[option][suboption][blockLength][value]` TLVs. Pure (no socket). Host from
[`perf/bench_profinet.cpp`](../perf/bench_profinet.cpp).

| Operation          | Host ns/op | Host MB/s |
| ------------------ | ---------: | --------: |
| `dcp_header` build |        4.9 |    2056.3 |
| `dcp_parse_header` |        4.0 |    2518.8 |
| `dcp_walk` blocks  |        7.5 |    1595.2 |

- Fixed 10-octet header + even-padded TLV blocks, no BER/CRC - the fast fixed-field class (~4-8 ns), same as
  EtherNet/IP. **`dcp_walk` (~7.5 ns) is the fuzz-target parser**: it iterates the option/suboption/blockLength
  TLVs, and a block-length lie or a missing even-pad must not walk past the buffer. Device µs/op via the rig
  `/bench` op + a `profinet_dcp_fuzz` attack are the next increments; full interop needs a raw-L2 DCP peer
  (e.g. a PROFINET discovery tool) + an Ethernet PHY.

### PID control law (DETWS_ENABLE_CONTROL)

The single-precision-float PID: derivative-on-measurement + optional low-pass, output clamp,
anti-windup by conditional integration, feed-forward. Measured on a real **ESP32-S3 @ 240 MHz** by
reading the Xtensa `CCOUNT` cycle register around a 20 000-iteration loop and subtracting an
empty-loop baseline, so each figure is the net cost of one call, warm cache.

First, the bare arithmetic ops on the LX7 (same CCOUNT method), because they explain everything
below:

| bare op | int32 | float |
| ------- | ----: | ----: |
| `mul`   |   3.8 |   4.9 |
| `add`   |     - |   5.1 |
| `1/x`   |     - |  56.2 |
| `div`   |   7.0 |  58.3 |

A float **divide is ~58 cyc - ~12x a float multiply** - and it is fully FPU-accelerated: `mul`/`add`
are single hardware instructions (`mul.s`/`add.s`), but the LX7 FPU has **no divide instruction**,
so GCC compiles `a / b` to a `call8 __divsf3` (a libgcc helper that does an FPU reciprocal +
Newton-Raphson). Confirmed by disassembly (`fmul` -> `mul.s`; `fdiv` -> `call __divsf3`).

The PID's only divide is the derivative's `/dt`, so eliminating it is the whole game:

| Operation                                      | ESP32-S3 cyc/op | ns/op | updates/s |
| ---------------------------------------------- | --------------: | ----: | --------: |
| `pid_update` (runtime dt - one `__divsf3`)     |           165.7 |   690 |     1.45M |
| `pid_update` (compile-time-constant dt)        |          ~ 98.4 |   410 |     2.44M |
| `pid_update_fixed` (dt cached by pid_set_rate) |            97.4 |   406 |     2.46M |

- The `/dt` divide is ~68 of the 166 cyc. `pid_update` is `inline`, so if the caller passes a
  **constant** `dt` the compiler folds `1/dt` to a literal and the `__divsf3` call vanishes (166 ->
  98 cyc for free). For a **runtime** `dt`, call `pid_set_rate(p, dt)` once and then
  `pid_update_fixed()` - it multiplies by a cached `1/dt` so the hot path is all `mul.s`/`madd.s`,
  **97.4 cyc, ~41% faster** than paying the per-tick `__divsf3`. Everything is single-precision, so
  it never falls onto the soft-float `double` path. At 97 cyc a 1 kHz loop is ~0.04% of one core,
  and ~100 axes at 1 kHz is ~4%. For deterministic latency free of flash-cache stalls, place the
  control loop itself in IRAM. Host-tested for correctness (`native_control`); tune the gains
  offline with [`tools/pid_tune.py`](../tools/pid_tune.py).

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

**Over a W5500 wired link (2026-07-13, ESP32-S3):** a 1 MB pull through the board (`8080 -> origin :8000`)
was **byte-exact** (SHA256 matched) but ran at only **~44 KB/s (~0.35 Mbps)** - ~5x slower than the WiFi
relay. The cause is the W5500's **single SPI bus**: both relay hops (inbound RX + origin RX/TX) serialize
over one bus into the chip's small 2 KB socket buffers, so the doubly-traversed bytes cannot overlap the
way they do on WiFi. A single-direction W5500 download is ~7 Mbps, but a relay crosses it twice and the two
directions contend, collapsing the rate. Correctness is unaffected; wired relay is for control-plane
publishing, not bulk transfer.

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

### W5500 SPI Ethernet throughput (DETWS_ETH_W5500)

Wired Ethernet over a W5500 SPI PHY (arduino-esp32 3.x `esp_eth`). A large body is streamed with
`send_chunked` (constant memory, one `tcp_write` per chunk within the send window). **Throughput is
SPI-bound, not PHY-bound**: the W5500's 100 Mbit PHY is fed over SPI, and the per-frame register protocol
(read RX size -> read socket buffer -> advance pointer -> RECV command) plus small on-chip socket buffers
cap real throughput near ~8 Mbit/s regardless of the 100 Mbit line.

> Measured 2026-07-13 on the COM4 **ESP32-S3** (W5500 on HSPI/SPI3: SCK=12, MISO=13, MOSI=11, CS=7,
> RST=6, INT=5), breadboard jumper wiring. `curl` GET of a `send_chunked` body; heap integrity
> (`heap_caps_check_integrity_all`) checked once per second throughout - it stayed intact on every run,
> including the truncated ones (the failures are SPI signal-integrity connection resets, not heap crashes).

#### Throughput vs SPI clock (10 MB GET burst)

| SPI clock | Mbit/s |  MB/s | Result                           |
| --------- | -----: | ----: | -------------------------------- |
| 8 MHz     |    4.7 | 0.588 | clean                            |
| 12 MHz    |    5.9 | 0.736 | clean                            |
| 16 MHz    |    7.0 | 0.877 | clean                            |
| 20 MHz    |    7.2 | 0.902 | clean (default)                  |
| 24 MHz    |    8.2 | 1.025 | clean                            |
| 30 MHz    |    8.3 | 1.037 | clean (burst); truncates > 15 MB |
| 33 MHz    |    8.2 | 1.021 | clean (burst)                    |
| 40 MHz    |      - |     - | truncated (SI, exit 56)          |
| 60 MHz    |      - |     - | truncated (SI, exit 56)          |
| 80 MHz    |      - |     - | link down (chip ID mis-read)     |

**Throughput scales with the clock to ~24 MHz, then plateaus at the W5500's internal ~8.3 Mbit/s ceiling
around 30 MHz** - faster SPI buys no more throughput, only less margin. `DETWS_ETH_W5500_SPI_MHZ` sets the
clock (default 20).

#### Sustained reliability (longer streams need more margin than bursts)

| SPI clock | Test   | Result                       | Mbit/s |
| --------- | ------ | ---------------------------- | -----: |
| 20 MHz    | 200 MB | clean, byte-exact, flat heap |    7.0 |
| 24 MHz    | 50 MB  | clean                        |    8.2 |
| 30 MHz    | 50 MB  | truncated at ~15 MB (SI)     |      - |

**Practicality:** wired Ethernet removes the Wi-Fi RTT, so it is the transport for **large, reliable
transfers** (firmware images, data logs, file serving). On clean/short PCB wiring the reliable-sustained
clock is higher; on breadboard jumpers keep it at the 20 MHz default (proven at 200 MB) or 24 MHz for
~14% more. For near-100-Mbit speed use an **RMII PHY (LAN8720)** instead - the W5500 trades speed for
needing no built-in MAC (works on the S3 / C3, which have none).

### Camera MJPEG streaming over send_chunked (real producer load)

The streaming TX path (`send_chunked`, constant memory, paged across worker loops) driven by a **live
OV2640 camera** instead of a synthetic source - the same path a firmware image or a file download uses,
now fed by a real, variable-size, back-to-back producer. Measured on a **XIAO ESP32-S3 Sense** (onboard
OV2640, 8 MB PSRAM frame buffers) over Wi-Fi.

| Endpoint   | Result                                                                                       |
| ---------- | -------------------------------------------------------------------------------------------- |
| `/capture` | one VGA JPEG, HTTP 200 `image/jpeg`, 15.7 KB, valid SOI..EOI (`FFD8`..`FFD9`)                |
| `/stream`  | `multipart/x-mixed-replace` MJPEG: **105 complete frames in 5 s (~21 fps VGA), ~5.2 Mbit/s** |

The `/stream` handler is a one-line `send_chunked(slot, 200, "multipart/x-mixed-replace; boundary=...",
source, ctx)`; the `source` is a small per-frame state machine (boundary+headers -> JPEG body ->
`esp_camera_fb_return`, then the next frame) that **never returns 0**, so the stream runs until the client
disconnects. Over a 5 s pull the wire carried 106 multipart parts (boundary + `Content-Type: image/jpeg` +
JPEG each); 105 JPEGs closed with `FFD9` and the 106th was cut mid-frame by the client timeout - exactly a
live stream truncated at an arbitrary instant.

**Practicality:** this proves the chunked streaming path holds up under a **continuous, unbounded producer**
(a webcam) in constant memory, not just a fixed-size body. Throughput here is producer- and Wi-Fi-bound
(~21 fps VGA), not a limit of the send path - the camera's JPEG rate and the radio set the pace. The
library owns the HTTP framing and flow control; the camera driver (`esp_camera`) and pin map are the
integration's (XIAO ESP32-S3 Sense: XCLK10, SIOD40/SIOC39, VSYNC38/HREF47/PCLK13, data Y2-Y9 =
15/17/18/16/14/12/11/48).

### PDM microphone WAV streaming over send_chunked

The same streaming path fed by the XIAO's **PDM digital microphone** (I2S PDM RX, CLK42/DATA41, 16 kHz /
16-bit / mono) as a WAV: `/capture.wav` returns a complete 2 s recording (44-byte RIFF/WAVE header + 64000 B
PCM), and `/stream.wav` streams a live WAV whose `source` emits the header once then continuous mic PCM
forever. `/stream.wav` runs at **31.4 KB/s ≈ 32000 B/s exactly** - the real-time byte rate for 16 kHz mono
16-bit - so the send path is paced by the microphone's sample production (1x realtime), which is correct for
audio. The mic is a live acoustic sensor, not a stuck value: an ambient capture reads ~442 peak-to-peak, and
capturing while a sound is made near the board widens it to ~1573 (min 653 / max 2226) - the range tracks the
acoustic input. (PDM output carries a DC bias, ~1418 here; a playback path high-passes it.) Together with the
camera, this shows `send_chunked` handles two very different real producers - a bursty variable-size video
frame source and a fixed-rate audio sample stream - in constant memory.
