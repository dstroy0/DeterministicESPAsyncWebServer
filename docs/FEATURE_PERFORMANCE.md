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

| Dimension        | Sweep                                                        |
| ---------------- | ------------------------------------------------------------ |
| Block size       | 512 B, 1 KiB, 4 KiB, 16 KiB, 64 KiB                          |
| Pattern          | sequential vs random                                         |
| Direction        | read vs write (and read-modify-write for the journal case)   |
| Queue depth      | 1, then batched (to find the depth where throughput plateaus)|
| Sync             | buffered vs `fsync`/flush-per-op (the durability cost)       |

Metrics captured per cell: **IOPS**, **latency** (avg + p99), **throughput (MB/s)**, and the
flush/`fsync` cost (the price of durability). The resulting curve sizes the atomic layer's write batch
(the block size at the throughput knee) and its queue depth (where more in-flight I/O stops helping).

> Status: **blocked on the SD card being attached.** The characterization harness is specified here and
> is the next build once the hardware is on. Results land in the table below.

| Interface | Block | Pattern | Dir | IOPS | Latency avg / p99 | MB/s |
| --------- | ----- | ------- | --- | ---- | ----------------- | ---- |
| _pending_ |       |         |     |      |                   |      |

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
