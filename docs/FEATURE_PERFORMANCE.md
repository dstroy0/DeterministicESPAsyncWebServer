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

Measured on a Raspberry Pi 5 (Cortex-A76, `-O2`); a relative baseline only. The ESP32-S3 column is
filled from the on-device harness (pending a board run).

| Feature   | Operation            | Host ns/op | Host MB/s | ESP32-S3 us/op | ESP32-S3 MB/s |
| --------- | -------------------- | ---------: | --------: | -------------: | ------------: |
| base64    | encode 1 KiB         |        944 |      1085 |        pending |       pending |
| base64    | decode 1 KiB         |       3274 |       313 |        pending |       pending |
| mtconnect | streams doc (20 obs) |       3291 |       749 |        pending |       pending |

Notes:

- base64 **decode is ~3.5x the cost of encode** (per-character alphabet lookup + validation vs a
  straight 3-byte to 4-char map) - worth knowing where a hot path decodes large Basic-auth / payload
  blobs.
- The MTConnect row builds a ~2.5 KB XML document (header + 20 observations) end to end, so ~750 MB/s
  of document assembly on the host baseline.

## 3. Request-path and protocol benchmarks

_To be added: HTTP request parse, JSON encode/decode throughput, a chunked/file send-pump pass, an SSH
KEX, a TLS handshake time, and per-protocol codec encode/decode rates - each host-baselined here and
then measured on the ESP32-S3._

## 4. Embedded data-store stack (host baseline)

The CPU cost of the hot ops in the data-store stack (WAL / dbm / document store / SQLite reader / Redis
RESP), measured over a **RAM-backed device** so this is pure compute, not I/O. Raspberry Pi 5
(Cortex-A76, `-O2`); relative baseline. Harness: `perf/bench_datastore.cpp`.

| Feature  | Operation               | Host ns/op | Host MB/s |
| -------- | ----------------------- | ---------: | --------: |
| wal      | crc32 (1 KiB)           |    10672.3 |      95.9 |
| wal      | record_encode (128 B)   |     1510.0 |      84.8 |
| wal      | store_append (64 B)     |      871.1 |      73.5 |
| wal      | store_checkpoint        |      325.4 |         - |
| dbm      | put (16 B key/64 B val) |      392.7 |         - |
| dbm      | get                     |       32.4 |         - |
| docstore | find_str (scan 100)     |     8914.3 |         - |
| docstore | -> per doc scanned      |       89.1 |         - |
| sqlite   | varint_decode           |        5.0 |         - |
| sqlite   | table scan (40 rows)    |     1486.0 |         - |
| sqlite   | -> per row (+ columns)  |       37.1 |         - |
| resp     | encode_command (3 args) |      327.7 |      91.5 |
| resp     | parse bulk reply        |       13.3 |         - |

**The takeaway: the stores are I/O-bound, not CPU-bound.** The compute per op is tiny - a dbm `get` is
~32 ns, a SQLite row (with its columns) ~37 ns, a RESP reply parse ~13 ns. The WAL write path is the
heaviest, and it is dominated by the **table-less CRC-32** (~96 MB/s, ~10 ns/byte): `record_encode` and
`store_append` are essentially their CRC cost. Even so, 96 MB/s is ~60x the measured durable SD write
rate (~1.5 MB/s, section 1), so the CRC is nowhere near the bottleneck - the SD card's ~40-100 durable
IOPS and 100+ ms write tail set the real ceiling, which is exactly why the layer batches into the WAL and
checkpoints in bulk. A `docstore` field query is a linear scan of ~89 ns per document (a dbm `get` plus a
top-level JSON parse), so an in-RAM 100-document `find` is ~9 us; at scale it is again the per-document
flash read, not the compare, that dominates.

_Follow-up: a table-driven or hardware (ESP32 ROM `crc32_le`) CRC would cut the WAL write CPU several-fold
if a faster backing store (PSRAM / LittleFS) ever makes it CPU-bound; and the on-device ESP32-S3 us/op
column for this table (the numbers above are the host baseline)._
