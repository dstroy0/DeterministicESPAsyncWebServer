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

### DNS server (DETWS_ENABLE_DNS_SERVER)

An authoritative DNS server on UDP/53 answers A/IN queries from a fixed table (NXDOMAIN otherwise).
`dns_server_build_response` is the per-query hot op: it parses the first question, resolves the name via a
callback, and appends one compressed A answer (or NXDOMAIN / NOTIMP). Pure (no clock, no socket). Host from
[`perf/bench_dns.cpp`](../perf/bench_dns.cpp); device from the rig `/bench` op.

| Operation                           | Host ns/op | Host MB/s | ESP32-S3 cyc/op | ESP32-S3 ns/op |
| ----------------------------------- | ---------: | --------: | --------------: | -------------: |
| `dns_server_build_response` (hit)   |       26.8 |     971.4 |             420 |           1750 |
| `dns_server_build_response` (NXDOM) |       25.4 |    1022.1 |               - |              - |

- A **~1.75 us** query on the device (parse the question labels + emit a 16-octet answer). The security
  shape matters more than the speed: DNS is the #1 reflection/amplification vector, and this server closes
  both doors - it is **authoritative-only** (an unconfigured name gets NXDOMAIN, never resolved, so it is
  **not an open resolver**) and an A answer is only **~1.6x** the query (no amplification, vs the 10-50x of a
  real reflector). It also **rejects compression pointers in the question** (`len & 0xC0` -> drop), so the
  classic compression-pointer parser loop cannot fire. The `dns_server_abuse` attack confirmed
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
