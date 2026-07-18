// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ServerConfig.h
 * @brief User-facing configuration for DeterministicESPAsyncWebServer.
 *
 * **Compile-time sizing constants**
 * These govern static array dimensions and must be set before the first
 * library header is included.  Define any of them in your sketch or in a
 * build flag before including this file to override the defaults:
 * @code
 *   // platformio.ini
 *   build_flags = -DMAX_CONNS=8 -DBODY_BUF_SIZE=512
 * @endcode
 *
 * **Runtime parameters - flash or RAM, your choice**
 * `WebServerConfig` holds values that can be changed without a rebuild.
 * On ESP32, `PROGMEM` is a no-op (const data lands in DROM automatically).
 * On AVR it places data in flash and requires `pgm_read_*` accessors - this
 * library targets ESP32 only, so both forms read identically via pointer:
 * @code
 *   // Flash (PROGMEM, no RAM cost at runtime):
 *   const WebServerConfig my_cfg PROGMEM = { .conn_timeout_ms = 10000 };
 *
 *   // RAM (can be changed at runtime):
 *   WebServerConfig my_cfg = { .conn_timeout_ms = 10000 };
 *
 *   server.begin(80, &my_cfg);
 * @endcode
 * Pass `nullptr` (or omit the argument) to use the built-in default
 * (`CONN_TIMEOUT_MS`, 5000 ms idle timeout).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CONFIG_H
#define DETERMINISTICESPASYNCWEBSERVER_CONFIG_H

#include <stdint.h>

// ---------------------------------------------------------------------------
// Compile-time capacity constants (affect static array sizes)
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous TCP connections (fixed static pool; ~3.95 KB of internal RAM per slot).
 *
 * Default 8: a keep-alive/concurrency server needs headroom above its peak concurrent client count,
 * because a connection closed by the keep-alive fairness cap (DWS_KEEPALIVE_MAX_REQUESTS) briefly
 * holds its slot in CONN_CLOSING while it drains, and a reconnecting client needs a free slot mean-
 * while - if concurrency equals the pool size there is none, and the overflow connection is refused
 * (correct backpressure, but it caps clean throughput at concurrency == MAX_CONNS - 1). Set lower
 * (e.g. -DMAX_CONNS=4, ~16 KB less RAM) on a RAM-constrained target, or higher (16/32) for a
 * connection-heavy HTTP server; the event queue tracks it automatically (EVT_QUEUE_DEPTH below).
 */
#ifndef MAX_CONNS
#define MAX_CONNS 8
#endif

/**
 * @brief Disable Nagle's algorithm (set TCP_NODELAY) on every accepted connection.
 *
 * A request/response server is latency-first: the response is buffered whole (`tcp_write`) and pushed with a
 * single `tcp_output`, so Nagle only ever delays the final sub-MSS segment of a multi-segment response (or a
 * streamed chunk) - it waits for the peer's ACK of the prior segment, costing a ~40-200 ms delayed-ACK stall
 * for no bandwidth benefit here. Disabling it lets that tail go out immediately. Set to 0 only if the device
 * mainly streams bulk data and you prefer Nagle's segment coalescing over per-response latency.
 */
#ifndef DWS_TCP_NODELAY
#define DWS_TCP_NODELAY 1
#endif

/** @brief Ring-buffer capacity in bytes per connection slot. */
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1024
#define DWS_RX_BUF_SIZE_DEFAULTED 1 // we chose it; upsized for streaming below (see STREAM_BODY)
#endif

/**
 * @brief Compile-time default for connection idle timeout in milliseconds.
 *
 * The actual runtime value is stored in `WebServerConfig::conn_timeout_ms`
 * and loaded into `DeterministicAsyncTCP::conn_timeout_ms` by init().
 */
#ifndef CONN_TIMEOUT_MS
#define CONN_TIMEOUT_MS 5000
#endif

/**
 * @brief Upper bound (ms) a slot may dwell in ConnState::CONN_CLOSING after a graceful close
 *        before the idle sweep force-aborts it.
 *
 * On a graceful (local) close the slot stays in ConnState::CONN_CLOSING - keeping its PCB and
 * callbacks - until the peer ACKs the response (then it frees itself in the sent
 * callback). If the peer never ACKs (dead/black-holed), this bound lets the
 * timeout sweep reclaim the slot so the fixed pool cannot leak.
 */
#ifndef DWS_CLOSING_TIMEOUT_MS
#define DWS_CLOSING_TIMEOUT_MS 2000
#endif

// ---------------------------------------------------------------------------
// Worker model (server task concurrency)
// ---------------------------------------------------------------------------
//
// The server pipeline (drain events -> dispatch -> send) runs in one or more
// dedicated FreeRTOS "worker" tasks instead of the user's loop(). Each worker
// owns a disjoint partition of conn_pool slots (slot i -> worker i %
// DWS_WORKER_COUNT) and its own scratch arena, so no two workers ever touch
// the same slot: shared-nothing, no hot-path locks, latency stays bounded
// (determinism preserved) while cores run disjoint connections in parallel.
//
// DWS_WORKER_COUNT == 1 (default) is byte-for-byte the single-pipeline model:
// one worker owns every slot, one arena, the existing single event queue. N > 1
// is opt-in. The arena BSS cost is DWS_SCRATCH_ARENA_SIZE * DWS_WORKER_COUNT.

/** @brief Number of server worker tasks (slots partitioned i % N). Default 1. */
#ifndef DWS_WORKER_COUNT
#define DWS_WORKER_COUNT 1
#endif

/**
 * @brief Stack (bytes) for each server worker task (ESP32).
 *
 * Floor note: two heavy computations run on the worker.
 *   - RSA-2048 verification (OIDC / SSH host key / JWKS via the mbedTLS bignum
 *     modexp) uses ~7 KB (measured on a DevKitV1).
 *   - SSH modern crypto (curve25519-sha256 KEX + ssh-ed25519, software field
 *     arithmetic in radix-2^16) peaks at ~10.5 KB (measured on an ESP32-S3): the
 *     deep ssh_gf call chain plus the on-accelerator field inversion nests deeper
 *     than the RSA path.
 * So the default adapts: 12 KB when SSH is enabled (curve/ed25519 can be
 * negotiated), 8 KB otherwise. Do NOT lower it below the matching floor
 * (::DWS_WORKER_STACK_CURVE_MIN for SSH, ::DWS_WORKER_STACK_RSA_MIN for
 * OIDC) or the first handshake overflows the task stack - a build-time guard
 * (bottom of this file) enforces the floor so a lowered stack is caught at
 * compile time.
 */
#ifndef DWS_WORKER_TASK_STACK
// SSH (curve25519 + ssh-ed25519) and HTTP/3 (the QUIC TLS-1.3 handshake reuses the same
// ssh_ed25519 signer for CertificateVerify) both peak at ~10.5 KB on the worker task, so both need
// the higher floor; everything else fits in 8 KB.
#if (defined(DWS_ENABLE_SSH) && DWS_ENABLE_SSH) || (defined(DWS_ENABLE_HTTP3) && DWS_ENABLE_HTTP3)
#define DWS_WORKER_TASK_STACK 12288
#else
#define DWS_WORKER_TASK_STACK 8192
#endif
#endif

/**
 * @brief Minimum worker-task stack (bytes) required once an RSA-2048 verifier is
 *        compiled in (OIDC / SSH).
 *
 * The mbedTLS bignum modexp alone consumes ~7 KB; 8 KB leaves room for the rest
 * of the request call chain. Overridable only for an advanced build that marshals
 * every RSA verify onto a dedicated larger-stack task (then the worker itself never
 * runs one) - otherwise leave it at the default.
 */
#ifndef DWS_WORKER_STACK_RSA_MIN
#define DWS_WORKER_STACK_RSA_MIN 8192
#endif

/**
 * @brief Minimum worker-task stack (bytes) required once SSH is compiled in.
 *
 * SSH can negotiate curve25519-sha256 + ssh-ed25519, whose software field
 * arithmetic peaks at ~10.5 KB of worker stack; 12 KB leaves ~1.8 KB of margin
 * for the rest of the handshake call chain (comparable to the RSA floor's
 * margin). Raise both this and ::DWS_WORKER_TASK_STACK together if you extend
 * the handshake, or force RSA/DH only (ssh_kex_set_prefer_rsa) on a very tight
 * build - but the server still advertises the modern suite, so a modern-only
 * client would still exercise it.
 */
#ifndef DWS_WORKER_STACK_CURVE_MIN
#define DWS_WORKER_STACK_CURVE_MIN 12288
#endif

/** @brief FreeRTOS priority for each server worker task (ESP32). */
#ifndef DWS_WORKER_TASK_PRIORITY
#define DWS_WORKER_TASK_PRIORITY 5
#endif

/**
 * @brief Core that worker 0 pins to (ESP32). Worker k pins to (DWS_WORKER_CORE
 * + k) % portNUM_PROCESSORS. Default 1 (APP_CPU), keeping Core 0 lean for the
 * WiFi/lwIP stack and offloading the user's loop().
 */
#ifndef DWS_WORKER_CORE
#define DWS_WORKER_CORE 1
#endif

/**
 * @brief Depth of each worker's deferred-callback queue.
 *
 * App code on loop() or another task submits work to a slot's owning worker via
 * dws_defer() / dws_defer_slot(); the worker runs it in its own single-thread
 * context, so an async push (ws_send / dws_sse_send from a timer) is race-free. Each
 * worker has one queue of this depth (entries are a {fn, arg} pair, ~8 bytes).
 */
#ifndef DWS_DEFER_QUEUE_DEPTH
#define DWS_DEFER_QUEUE_DEPTH 8
#endif

/**
 * @brief Idle-sweep timeout, in FreeRTOS ticks, that a worker blocks between
 *        service iterations when no events are pending.
 *
 * The worker no longer free-runs a poll: it blocks on a task notification and a
 * producer (a new connection event or a deferred submission) wakes it the moment
 * work arrives, so event latency is independent of this value. The block still
 * times out after this many ticks so the idle timeout sweep (check_timeouts) keeps
 * reaping stale connections when nothing is in flight.
 *
 * Default 1 (1 tick at the Arduino 1 kHz FreeRTOS config) preserves the original
 * idle cadence byte-for-byte. Because events now wake the worker immediately,
 * raising it lowers idle wakeups (CPU/power on a battery device) WITHOUT the
 * latency penalty the old poll-based knob carried - e.g. 100 -> a ~10 Hz idle
 * sweep, still far below any connection timeout. The internal time base stays
 * 1000 Hz regardless (see services/clock.h).
 */
#ifndef DWS_WORKER_POLL_TICKS
#define DWS_WORKER_POLL_TICKS 1
#endif

#if DWS_WORKER_COUNT < 1
#error "DeterministicESPAsyncWebServer: DWS_WORKER_COUNT must be >= 1"
#endif
#if DWS_WORKER_COUNT > MAX_CONNS
#error "DeterministicESPAsyncWebServer: DWS_WORKER_COUNT must be <= MAX_CONNS"
#endif

// ---------------------------------------------------------------------------
// Preempting work queue (DWS_ENABLE_PREEMPT_QUEUE) - v5 real-time ingest
// ---------------------------------------------------------------------------
//
// Fixed-capacity queues, each feeding one core-pinned processing task: a producer
// posts a fixed-size item (from a task or an ISR) and the scheduler preempts straight
// to the task. There are named lanes - one USER lane exposed to the app, and internal
// DMA / forwarding / device-access lanes that run at a higher priority so internal
// ingest always preempts user work. Queue storage is static (zero heap), so depth +
// item size are compile-time; a task's stack is created only when its lane starts.
// The no-lane dws_pq_* API drives the USER lane. See preempt_queue.h.

/** @brief Enable the preempting work queue primitive (default off). */
#ifndef DWS_ENABLE_PREEMPT_QUEUE
#define DWS_ENABLE_PREEMPT_QUEUE 0
#endif

/** @brief Capacity of the preempting queue in items (static-allocated). */
#ifndef DWS_PQ_DEPTH
#define DWS_PQ_DEPTH 16
#endif

/** @brief Bytes per preempting-queue item (the posted item must fit). */
#ifndef DWS_PQ_ITEM_SIZE
#define DWS_PQ_ITEM_SIZE 32
#endif

/** @brief Stack (bytes) for each preempting-queue processing task (ESP32). */
#ifndef DWS_PQ_STACK
#define DWS_PQ_STACK 4096
#endif

/**
 * @brief Base FreeRTOS priority for the internal preempting lanes (DMA / forwarding /
 *        device access). They run at this and just above, so internal ingest preempts
 *        the user lane; keep it above the user lane's priority and below the lwIP tcpip
 *        (18) / WiFi tasks so networking is never starved. See preempt_queue.h.
 */
#ifndef DWS_PQ_INTERNAL_PRIORITY
#define DWS_PQ_INTERNAL_PRIORITY 8
#endif

#if DWS_ENABLE_PREEMPT_QUEUE && (DWS_PQ_DEPTH < 1 || DWS_PQ_ITEM_SIZE < 1)
#error "DeterministicESPAsyncWebServer: DWS_PQ_DEPTH and DWS_PQ_ITEM_SIZE must be >= 1"
#endif

// ---------------------------------------------------------------------------
// DMA peripheral ingest / egress (DWS_ENABLE_DMA) - v5 hardware ingest
// ---------------------------------------------------------------------------
//
// Move peripheral bytes (UART / I2C / SPI) between the wire and a static buffer
// with the CPU free during the transfer; a DMA-complete event carries the bytes
// to a user callback, which typically posts a descriptor into the preempting work
// queue (DWS_ENABLE_PREEMPT_QUEUE) so the heavy processing runs off the ISR. RX
// is double-buffered (ping-pong): the completed buffer is handed up while the DMA
// engine fills the other. Storage is static (zero heap) - channel count and buffer
// size are compile-time. See services/dma/dma.h.
//
// DWS_DMA_SIMULATE routes the transfers through an in-memory ingress/egress
// simulator (feed bytes in, capture bytes out, optional TX->RX loopback) so the
// whole pipeline is exercised with no physical loopback wire - on the host test
// bench and, with the flag set, on the device itself. It is the shipped, tested
// engine; a real silicon backend plugs into dws_dma_hw_* when DWS_DMA_SIMULATE=0.

/** @brief Enable the DMA peripheral ingest / egress primitive (default off). */
#ifndef DWS_ENABLE_DMA
#define DWS_ENABLE_DMA 0
#endif

/** @brief Number of DMA channels (static-allocated; each is one peripheral link). */
#ifndef DWS_DMA_CHANNELS
#define DWS_DMA_CHANNELS 2
#endif

/** @brief Bytes per DMA transfer buffer (RX is double-buffered at this size). */
#ifndef DWS_DMA_BUF_SIZE
#define DWS_DMA_BUF_SIZE 256
#endif

/**
 * @brief Route DMA transfers through the ingress/egress simulator (default on).
 *        Set to 0 to drive real silicon via the dws_dma_hw_* backend hooks.
 */
#ifndef DWS_DMA_SIMULATE
#define DWS_DMA_SIMULATE 1
#endif

#if DWS_ENABLE_DMA && (DWS_DMA_CHANNELS < 1 || DWS_DMA_BUF_SIZE < 1)
#error "DeterministicESPAsyncWebServer: DWS_DMA_CHANNELS and DWS_DMA_BUF_SIZE must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Interface forwarding plane (DWS_ENABLE_FORWARD) - v5 hardware ingest
// ---------------------------------------------------------------------------
//
// A forwarding plane over the ingest pipeline: register interfaces (Wi-Fi STA / AP,
// Ethernet, a peripheral bus, a radio), each with an egress send callback, then add
// per-pair allow / deny rules with an optional rate cap. A frame arriving on one
// interface (dws_forward_ingress(), typically from a DMA-complete event posted onto the
// FORWARD lane) is forwarded to every allowed destination, so the device bridges / routes
// between its interfaces instead of only terminating traffic. Default-deny and fail-closed
// (a full destination or an exceeded rate cap drops, never blocks). Static tables (zero
// heap). See services/forward/forward.h.

/** @brief Enable the interface forwarding plane (default off). */
#ifndef DWS_ENABLE_FORWARD
#define DWS_ENABLE_FORWARD 0
#endif

/** @brief Max interfaces the forwarding plane tracks (static-allocated). */
#ifndef DWS_FWD_MAX_IFACES
#define DWS_FWD_MAX_IFACES 4
#endif

/** @brief Max forwarding rules (src -> dst allow/deny + rate cap; static-allocated). */
#ifndef DWS_FWD_MAX_RULES
#define DWS_FWD_MAX_RULES 8
#endif

/** @brief Max ingress access-control entries (byte-pattern permit/deny; static). */
#ifndef DWS_FWD_MAX_ACL
#define DWS_FWD_MAX_ACL 8
#endif

/** @brief Bytes an ACL entry can match (its pattern / mask length). */
#ifndef DWS_FWD_ACL_PATLEN
#define DWS_FWD_ACL_PATLEN 4
#endif

/** @brief Max policy routes (byte-pattern -> egress interface; static). Policy routes take
 *         precedence over the src->dst rules, so tagged traffic leaves a chosen interface. */
#ifndef DWS_FWD_MAX_ROUTES
#define DWS_FWD_MAX_ROUTES 8
#endif

/** @brief Build-time toggle for the forwarding-path inspection hook (default off, for cost +
 *         privacy). When 1, dws_forward_set_inspector() installs a runtime callback that observes
 *         / filters each ingress frame before it is forwarded; when 0 the hook is compiled out
 *         entirely (no call site). Runtime toggle: register or clear (null) the inspector. */
#ifndef DWS_FWD_INSPECT
#define DWS_FWD_INSPECT 0
#endif

#if DWS_ENABLE_FORWARD && (DWS_FWD_MAX_IFACES < 1 || DWS_FWD_MAX_RULES < 1 || DWS_FWD_ACL_PATLEN < 1)
#error "DeterministicESPAsyncWebServer: DWS_FWD_MAX_IFACES / DWS_FWD_MAX_RULES / DWS_FWD_ACL_PATLEN must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Radio / wireless gateway (DWS_ENABLE_GATEWAY) - v5 southbound-to-northbound bridge
// ---------------------------------------------------------------------------
//
// The generic gateway pattern: a southbound radio (LoRa / nRF24 / Zigbee / ... reached
// over SPI / I2C / UART) is a "port"; a frame it receives (data-ready ISR -> DMA -> the
// FORWARD lane -> a per-radio codec) is handed to dws_gateway_uplink(), which envelopes it with
// its source node address / port / RSSI and publishes it northbound through the uplink
// callback (wire it to MQTT / HTTP / WebSocket / UDP). A northbound command goes the other
// way through dws_gateway_downlink() to the port's transmit callback. The radio TX + the
// northbound publish are callbacks (the seam a real radio driver / protocol binding plugs
// into), so the bridge is host- and device-testable with no radio. Static tables (zero
// heap). See services/gateway/gateway.h.

/** @brief Enable the radio / wireless gateway bridge (default off). */
#ifndef DWS_ENABLE_GATEWAY
#define DWS_ENABLE_GATEWAY 0
#endif

/** @brief Max southbound gateway ports (radios / buses; static-allocated). */
#ifndef DWS_GW_MAX_PORTS
#define DWS_GW_MAX_PORTS 4
#endif

/** @brief Default northbound topic prefix (overridable at runtime via dws_gateway_set_topic_prefix). */
#ifndef DWS_GW_DEFAULT_PREFIX
#define DWS_GW_DEFAULT_PREFIX "gw"
#endif

#if DWS_ENABLE_GATEWAY && (DWS_GW_MAX_PORTS < 1)
#error "DeterministicESPAsyncWebServer: DWS_GW_MAX_PORTS must be >= 1"
#endif

// ---------------------------------------------------------------------------
// LoRa radio (DWS_ENABLE_LORA) - Semtech SX127x / RFM95-96 codec + driver
// ---------------------------------------------------------------------------
//
// A per-radio codec + driver that plugs into the gateway (DWS_ENABLE_GATEWAY): the
// RadioHead-compatible 4-byte frame header (to / from / id / flags) codec, and an SX127x
// register driver over a caller-supplied register-access bus (so the SPI + chip-select
// wiring is the integration's, and the register protocol is host-testable with a mock
// bus). Bridge received frames northbound with dws_gateway_uplink(); the actual RF link needs
// the module to verify. See services/lora/lora.h.

/** @brief Enable the LoRa (SX127x) radio codec + driver (default off). */
#ifndef DWS_ENABLE_LORA
#define DWS_ENABLE_LORA 0
#endif

/** @brief Max LoRa payload bytes (SX127x FIFO is 256; RadioHead uses 251 + 4 header). */
#ifndef DWS_LORA_MAX_PAYLOAD
#define DWS_LORA_MAX_PAYLOAD 251
#endif

#if DWS_ENABLE_LORA && (DWS_LORA_MAX_PAYLOAD < 1 || DWS_LORA_MAX_PAYLOAD > 251)
#error "DeterministicESPAsyncWebServer: DWS_LORA_MAX_PAYLOAD must be 1..251"
#endif

// ---------------------------------------------------------------------------
// nRF24 radio (DWS_ENABLE_NRF24) - Nordic nRF24L01+ 2.4 GHz driver
// ---------------------------------------------------------------------------
//
// A radio driver that plugs into the gateway (DWS_ENABLE_GATEWAY). The nRF24L01+ speaks
// an SPI command protocol (not plain register r/w) and needs a separate CE pin, so the
// driver runs over a caller-supplied SPI transfer + CE bus (nrf_bus). Its hardware pipe
// addressing means the "source address" of a received frame is the pipe number - no
// in-payload header, so there is no separate codec. Bridge received payloads northbound
// with dws_gateway_uplink(port, pipe, ...); the RF link needs the module to verify.
// See services/nrf24/nrf24.h.

/** @brief Enable the nRF24L01+ radio driver (default off). */
#ifndef DWS_ENABLE_NRF24
#define DWS_ENABLE_NRF24 0
#endif

/** @brief nRF24 fixed payload width in bytes (1..32; the chip's static payload size). */
#ifndef DWS_NRF24_PAYLOAD
#define DWS_NRF24_PAYLOAD 32
#endif

#if DWS_ENABLE_NRF24 && (DWS_NRF24_PAYLOAD < 1 || DWS_NRF24_PAYLOAD > 32)
#error "DeterministicESPAsyncWebServer: DWS_NRF24_PAYLOAD must be 1..32"
#endif

// ---------------------------------------------------------------------------
// EnOcean ESP3 (DWS_ENABLE_ENOCEAN) - energy-harvesting 868 MHz serial codec
// ---------------------------------------------------------------------------
//
// A UART telegram codec for EnOcean's ESP3 (EnOcean Serial Protocol 3), the framing used
// by USB/serial EnOcean gateways (TCM 310 / USB 300): sync 0x55, a 4-byte header (data
// length, optional length, packet type) protected by CRC8, then data + optional data
// protected by a second CRC8. dws_esp3_parse() frames one telegram out of a byte stream and
// verifies both CRCs; dws_esp3_build() assembles one. Pure (no UART code - you feed it the
// serial bytes), so it is fully host-testable. See services/enocean/enocean.h.

/** @brief Enable the EnOcean ESP3 serial codec (default off). */
#ifndef DWS_ENABLE_ENOCEAN
#define DWS_ENABLE_ENOCEAN 0
#endif

/** @brief Reject an ESP3 telegram whose declared data length exceeds this (framing sanity). */
#ifndef DWS_ENOCEAN_MAX_DATA
#define DWS_ENOCEAN_MAX_DATA 512
#endif

#if DWS_ENABLE_ENOCEAN && (DWS_ENOCEAN_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DWS_ENOCEAN_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// PN532 NFC (DWS_ENABLE_PN532) - NXP PN532 NFC/RFID controller frame codec
// ---------------------------------------------------------------------------
//
// The NXP PN532 (I2C / SPI / HSU) command-frame protocol - a tag read/write bridged to an
// HTTP / MQTT event. The chip is driven by "normal information frames" (00 00 FF | LEN |
// LCS | TFI | PData | DCS | 00) with a length checksum and a data checksum, plus a 6-byte
// ACK frame. dws_pn532_build_frame() / dws_pn532_parse_frame() assemble and verify those frames
// (the per-command PData is the application's), and dws_pn532_is_ack() detects the ACK. Pure -
// you carry the frame bytes over your I2C / SPI / UART - so it is fully host-testable.
// See services/pn532/pn532.h.

/** @brief Enable the PN532 NFC frame codec (default off). */
#ifndef DWS_ENABLE_PN532
#define DWS_ENABLE_PN532 0
#endif

/** @brief Reject a PN532 normal frame whose declared length exceeds this (framing sanity). */
#ifndef DWS_PN532_MAX_DATA
#define DWS_PN532_MAX_DATA 254
#endif

#if DWS_ENABLE_PN532 && (DWS_PN532_MAX_DATA < 1 || DWS_PN532_MAX_DATA > 254)
#error "DeterministicESPAsyncWebServer: DWS_PN532_MAX_DATA must be 1..254"
#endif

// ---------------------------------------------------------------------------
// Sigfox (DWS_ENABLE_SIGFOX) - Wisol / Murata Sigfox modem AT-command codec
// ---------------------------------------------------------------------------
//
// Tiny low-power uplinks over the Sigfox 0G network. A Wisol / Murata Sigfox modem is
// driven by AT commands over UART: dws_sigfox_build_uplink() formats an `AT$SF=<hex>` frame
// for a <= 12-byte payload, and dws_sigfox_parse_response() classifies the modem's reply
// (OK / ERROR / still pending). Pure text-command codec - you carry it over your UART - so
// it is fully host-testable. See services/sigfox/sigfox.h.

/** @brief Enable the Sigfox AT-command codec (default off). */
#ifndef DWS_ENABLE_SIGFOX
#define DWS_ENABLE_SIGFOX 0
#endif

/** @brief Maximum Sigfox uplink payload (the network caps a message at 12 bytes). */
#ifndef DWS_SIGFOX_MAX_PAYLOAD
#define DWS_SIGFOX_MAX_PAYLOAD 12
#endif

#if DWS_ENABLE_SIGFOX && (DWS_SIGFOX_MAX_PAYLOAD < 1 || DWS_SIGFOX_MAX_PAYLOAD > 12)
#error "DeterministicESPAsyncWebServer: DWS_SIGFOX_MAX_PAYLOAD must be 1..12"
#endif

// ---------------------------------------------------------------------------
// Z-Wave (DWS_ENABLE_ZWAVE) - Silicon Labs Z-Wave Serial API frame codec
// ---------------------------------------------------------------------------
//
// The host-side Serial API of a Silicon Labs 500 / 700-series Z-Wave controller over UART:
// a Z-Wave mesh bridged to the web. Data frames are SOF (0x01) | LEN | Type | Command |
// Data | Checksum, where the checksum is 0xFF XOR-folded over LEN..last-data; single-byte
// ACK (0x06) / NAK (0x15) / CAN (0x18) frames flow-control them. dws_zwave_build_frame() /
// dws_zwave_parse_frame() assemble and verify a data frame; the per-command payload is the
// application's. Pure - you carry the bytes over your UART - so it is fully host-testable.
// See services/zwave/zwave.h.

/** @brief Enable the Z-Wave Serial API frame codec (default off). */
#ifndef DWS_ENABLE_ZWAVE
#define DWS_ENABLE_ZWAVE 0
#endif

/** @brief Reject a Z-Wave frame whose declared length exceeds this data cap (sanity). */
#ifndef DWS_ZWAVE_MAX_DATA
#define DWS_ZWAVE_MAX_DATA 64
#endif

#if DWS_ENABLE_ZWAVE && (DWS_ZWAVE_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DWS_ZWAVE_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Zigbee (DWS_ENABLE_ZIGBEE) - Silicon Labs EZSP / ASH serial framing codec
// ---------------------------------------------------------------------------
//
// The ASH (Asynchronous Serial Host) data-link layer that carries EZSP frames to a Silicon
// Labs EmberZNet NCP over UART - a Zigbee network bridged to the web. ASH delimits frames
// with a Flag byte (0x7E), byte-stuffs the reserved control bytes, and protects each frame
// with a CRC-16/CCITT. dws_ash_frame_encode() wraps a control byte + payload into a stuffed,
// CRC'd frame; dws_ash_frame_decode() unstuffs + verifies one. The EZSP command payload the
// frame carries (version, stack status, an incoming APS message, ...) is the application's.
// dws_ash_frame_decode() removes the stuffing and verifies the CRC. Pure - you carry the bytes
// over your UART - so it is fully host-testable. See services/zigbee/zigbee.h.

/** @brief Enable the Zigbee EZSP / ASH framing codec (default off). */
#ifndef DWS_ENABLE_ZIGBEE
#define DWS_ENABLE_ZIGBEE 0
#endif

/** @brief Max ASH payload bytes (an EZSP frame; the ASH data field caps near 128). */
#ifndef DWS_ZIGBEE_MAX_DATA
#define DWS_ZIGBEE_MAX_DATA 128
#endif

#if DWS_ENABLE_ZIGBEE && (DWS_ZIGBEE_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DWS_ZIGBEE_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Thread (DWS_ENABLE_THREAD) - OpenThread spinel over HDLC-lite framing codec
// ---------------------------------------------------------------------------
//
// The HDLC-lite framing that carries spinel frames to an OpenThread radio co-processor
// (RCP: an nRF52840 / EFR32) over UART - an 802.15.4 / Thread mesh bridged to IP / the web.
// Each spinel frame is wrapped by HDLC-lite: an FCS (CRC-16/X-25) is appended, the reserved
// bytes are byte-stuffed, and a Flag byte (0x7E) terminates it. dws_spinel_frame_encode() /
// dws_spinel_frame_decode() do the framing + FCS; the spinel command inside (a property
// get/set/insert, a stream frame) is the application's. Pure - you carry the bytes over your
// UART - so it is fully host-testable. See services/thread/thread.h.

/** @brief Enable the Thread spinel / HDLC-lite framing codec (default off). */
#ifndef DWS_ENABLE_THREAD
#define DWS_ENABLE_THREAD 0
#endif

/** @brief Max spinel payload bytes carried in one HDLC-lite frame. */
#ifndef DWS_THREAD_MAX_DATA
#define DWS_THREAD_MAX_DATA 256
#endif

#if DWS_ENABLE_THREAD && (DWS_THREAD_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DWS_THREAD_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Wired Ethernet PHY (DWS_ENABLE_ETHERNET) - run the server over an RMII PHY
// ---------------------------------------------------------------------------
//
// Bring up a wired Ethernet link (an RMII PHY: LAN8720 / TLK110 / RTL8201 / DP83848) so the
// server runs over Ethernet instead of (or alongside) Wi-Fi. init_eth_physical() is a thin
// wrapper over the Arduino ETH library; the PHY pins / type / clock come from the standard
// ETH_PHY_* build flags for your board (see example 19.Ethernet). The egress reporting
// (dws_net_egress -> DWSIface::DETIFACE_ETH) and the per-route interface classifier already handle a
// wired route, so once the link has an IP the server accepts on it with no other change.
// Default off (zero cost / the ETH library is not linked). ESP32-only.

/** @brief Enable wired Ethernet bring-up (init_eth_physical / eth_ready). Default off. */
#ifndef DWS_ENABLE_ETHERNET
#define DWS_ENABLE_ETHERNET 0
#endif

// W5500 SPI Ethernet (arduino-esp32 3.x only). Set DWS_ETH_W5500=1 to select the SPI PHY over the RMII
// default; the pins below are the ESP32-S3-DevKitC wiring (HSPI / SPI3). The 2.x ETH library has no W5500,
// so init_eth_physical() falls back to the RMII ETH.begin() when the core is older.
#ifndef DWS_ETH_W5500
#define DWS_ETH_W5500 0
#endif
#ifndef DWS_ETH_W5500_CS
#define DWS_ETH_W5500_CS 7 ///< chip select
#endif
#ifndef DWS_ETH_W5500_RST
#define DWS_ETH_W5500_RST 6 ///< reset
#endif
#ifndef DWS_ETH_W5500_INT
#define DWS_ETH_W5500_INT 5 ///< interrupt
#endif
#ifndef DWS_ETH_W5500_SCK
#define DWS_ETH_W5500_SCK 12 ///< HSPI clock (S3-DevKitC default)
#endif
#ifndef DWS_ETH_W5500_MISO
#define DWS_ETH_W5500_MISO 13 ///< HSPI MISO (S3-DevKitC default)
#endif
#ifndef DWS_ETH_W5500_MOSI
#define DWS_ETH_W5500_MOSI 11 ///< HSPI MOSI (S3-DevKitC default)
#endif
// W5500 SPI clock in MHz. The W5500 datasheet allows up to 33.3 MHz; 20 is the arduino-esp32 default and
// a safe value for breadboard jumper wiring. Higher clocks raise throughput (the link is SPI-bound, not
// PHY-bound) but need clean, short wiring - marginal signal integrity at high MHz corrupts frames.
#ifndef DWS_ETH_W5500_SPI_MHZ
#define DWS_ETH_W5500_SPI_MHZ 20 ///< W5500 SPI clock (MHz); raise for throughput on clean wiring
#endif

/**
 * @brief Enable IPv6 on the network interface (dual-stack). Default off.
 *
 * When set, init_ipv6_physical() turns on IPv6 for the Wi-Fi netif (SLAAC link-local plus any
 * router-advertised global address). The TCP and UDP listeners already bind IPADDR_TYPE_ANY, so
 * the server accepts IPv6 connections the moment the interface has a v6 address; the DWSIp core
 * (network_drivers/network/ip.h) parses / formats / classifies both families. Requires an
 * lwIP built with LWIP_IPV6=1 (the stock Arduino-ESP32 core ships it).
 */
#ifndef DWS_ENABLE_IPV6
#define DWS_ENABLE_IPV6 0
#endif

/**
 * @brief Wi-Fi promiscuous (monitor) capture (DWS_ENABLE_PROMISC). Default off.
 *
 * Passive 802.11 sniffing: dws_promisc_begin() puts the radio in promiscuous mode on a channel and
 * delivers every frame to a sink (services/promisc). Wire that sink into the forwarding plane
 * (DWS_ENABLE_FORWARD) to bridge captured Wi-Fi frames to another interface - e.g. stream them
 * to a wired collector over Ethernet. Ships a pure 802.11 header parser and libpcap framing
 * (DLT_IEEE802_11) so a forwarded frame is a valid PCAP a wired Wireshark / tcpdump can read.
 */
#ifndef DWS_ENABLE_PROMISC
#define DWS_ENABLE_PROMISC 0
#endif

/**
 * @brief Wired field-bus listen-only capture (DWS_ENABLE_BUS_CAPTURE). Default off.
 *
 * The wired counterpart to promiscuous Wi-Fi capture: bus_capture_begin() installs the CAN (TWAI)
 * controller in listen-only mode - it decodes every frame on the bus but never ACKs or transmits,
 * so it stays invisible - and delivers each CanFrame to a sink (services/bus_capture). Wire the
 * sink into the forwarding plane (DWS_ENABLE_FORWARD) to bridge captured CAN frames to another
 * interface. can_to_socketcan() formats a frame as a Linux SocketCAN frame so, with the libpcap
 * DLT_CAN_SOCKETCAN link type, the stream is a capture Wireshark reads.
 */
#ifndef DWS_ENABLE_BUS_CAPTURE
#define DWS_ENABLE_BUS_CAPTURE 0
#endif

// Feature / service / codec tuning knobs are consolidated at the END of this file,
// under "Feature tuning knobs (grouped and gated by feature)" - placed there so every
// DWS_ENABLE_* flag is already resolved and each group can gate on its own feature.

/** @brief Maximum HTTP headers stored per request. */
#ifndef MAX_HEADERS
#define MAX_HEADERS 8
#endif

/** @brief Maximum URL path length (including leading `/`). */
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 64
#endif

/**
 * @brief Maximum header field-name length (e.g. `"Content-Type"`).
 *
 * Must accommodate the longest header name the app needs to read by key.
 * Standard names reach 30+ chars (`Sec-WebSocket-Extensions` = 24,
 * `Access-Control-Request-Headers` = 30), so the default leaves margin; an
 * over-long key is truncated (not rejected) by the parser.
 */
#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 32
#endif

/** @brief Maximum header field-value length. */
#ifndef MAX_VAL_LEN
#define MAX_VAL_LEN 48
#endif

/** @brief Maximum raw query-string length (everything after `?`). */
#ifndef MAX_QUERY_LEN
#define MAX_QUERY_LEN 128
#endif

/** @brief Maximum number of parsed query-string parameters. */
#ifndef MAX_QUERY_PARAMS
#define MAX_QUERY_PARAMS 8
#endif

/** @brief Maximum number of `:name` path parameters captured per route match. */
#ifndef MAX_PATH_PARAMS
#define MAX_PATH_PARAMS 4
#endif

/**
 * @brief Capacity for the full `Authorization` header value (Digest auth).
 *
 * A Digest `Authorization` header (username, realm, nonce, uri, response,
 * qop, nc, cnonce) is far longer than MAX_VAL_LEN, so when DWS_ENABLE_AUTH
 * is set the parser captures it whole into a dedicated per-request buffer.
 */
#ifndef DIGEST_AUTH_HDR_MAX
#define DIGEST_AUTH_HDR_MAX 384
#endif

/**
 * @brief Lifetime of a Digest `nonce`, in milliseconds (default 5 minutes).
 *
 * The server mints a stateless, keyed, timestamped nonce (RFC 7616 3.3) rather
 * than a fixed one: each challenge carries the issue time plus a MAC over the
 * server secret, so no per-nonce table is needed. A client `Authorization` whose
 * nonce is older than this window is treated as @c stale - the credentials are
 * re-checked and, if correct, the server reissues a fresh challenge with
 * `stale=true` so the client retries transparently (no re-prompt). This bounds
 * how long a captured Digest response can be replayed without any server-side
 * state, which the shared-nothing worker model could not hold safely.
 */
#ifndef DWS_DIGEST_NONCE_LIFETIME_MS
#define DWS_DIGEST_NONCE_LIFETIME_MS (5u * 60u * 1000u)
#endif

/** @brief Maximum query-parameter key length. */
#ifndef QUERY_KEY_LEN
#define QUERY_KEY_LEN 24
#endif

/** @brief Maximum query-parameter value length. */
#ifndef QUERY_VAL_LEN
#define QUERY_VAL_LEN 48
#endif

/**
 * @brief Maximum request body bytes stored in `HttpReq::body`.
 *
 * Bodies larger than this trigger a 413 Payload Too Large response -
 * the parser detects the overflow via `Content-Length` before any body
 * bytes arrive, so no data is read or stored for oversized requests.
 */
#ifndef BODY_BUF_SIZE
#define BODY_BUF_SIZE 256
#endif

/** @brief Maximum simultaneously registered routes. */
#ifndef MAX_ROUTES
#define MAX_ROUTES 16
#endif

/**
 * @brief Maximum globally-registered middleware functions.
 *
 * The middleware chain is a fixed array of function pointers run in
 * registration order before a request reaches its route handler (see
 * DWS::use()). Costs MAX_MIDDLEWARE pointers of BSS; an empty chain
 * adds no per-request work.
 */
#ifndef MAX_MIDDLEWARE
#define MAX_MIDDLEWARE 4
#endif

/**
 * @brief Per-chunk staging buffer for send_chunked()'s ChunkSource (max bytes a
 *        source produces per call, hence the largest single chunk on the wire).
 *
 * Allocated on the worker stack only while a chunk is being framed - no persistent
 * RAM cost. The pump asks the source for at most this many bytes (or fewer when the
 * send window is smaller), so it bounds the chunk size, not the total body.
 *
 * Sized to one TCP segment (~MSS): the pump frames + sends each chunk in a single
 * tcpip_thread round-trip (~23 us on-device), so a bigger chunk = fewer round-trips per
 * byte. 1440 keeps the framed chunk within one segment; raise it (up to the send window)
 * to cut the round-trip count further on a fast transport (e.g. Ethernet), at more stack.
 */
#ifndef CHUNK_BUF_SIZE
#define CHUNK_BUF_SIZE 1440
#endif

/**
 * @brief Maximum object/array nesting depth for the JsonWriter (see json.h).
 *
 * Bounds the writer's per-level comma-tracking stack (one bool per level);
 * begin_object()/begin_array() beyond this fail the writer instead of
 * overflowing. No heap; ~JSON_MAX_DEPTH bytes of stack inside the writer object.
 */
#ifndef JSON_MAX_DEPTH
#define JSON_MAX_DEPTH 8
#endif

/**
 * @brief Step budget for the regex route matcher (see on_regex()).
 *
 * The matcher is a bounded backtracker: it counts match steps and fails closed
 * (no match) once this budget is exhausted, so a pathological pattern can never
 * backtrack unboundedly. Keeps regex routing deterministic. Routing patterns hit
 * only a handful of steps; the default leaves wide margin.
 */
#ifndef RE_MAX_STEPS
#define RE_MAX_STEPS 2000
#endif

// ---------------------------------------------------------------------------
// WebSocket sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous WebSocket connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * ws_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_WS_CONNS
#define MAX_WS_CONNS 2
#endif

/**
 * @brief Maximum WebSocket frame payload in bytes.
 *
 * Frames larger than this are rejected with Close code 1009 (Message Too Big).
 * Fragmented messages are not supported; each message must fit in one frame.
 */
#ifndef WS_FRAME_SIZE
#define WS_FRAME_SIZE 512
#endif

// ---------------------------------------------------------------------------
// Server-Sent Events sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous SSE connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * dws_sse_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_SSE_CONNS
#define MAX_SSE_CONNS 2
#endif

/**
 * @brief Output buffer size in bytes for a single SSE event.
 *
 * An event larger than this is silently truncated.  The buffer holds the
 * formatted `data: ...\n\n` line before it is handed to tcp_write().
 */
#ifndef SSE_BUF_SIZE
#define SSE_BUF_SIZE 256
#endif

// ---------------------------------------------------------------------------
// Static file serving sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Bytes read from the filesystem and passed to tcp_write() per loop().
 *
 * Each read+send is one tcpip_thread round-trip (~23 us on-device), so a larger chunk =
 * fewer round-trips per byte (better throughput on a fast transport), at more peak stack.
 * Must be <= RX_BUF_SIZE to avoid stalling the TCP send window; 1024 tracks the default
 * RX_BUF_SIZE. Lower it (e.g. -DFILE_CHUNK_SIZE=512) on a stack-constrained target.
 */
#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 1024
#endif

// ---------------------------------------------------------------------------
// Basic Auth sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum username or password length for HTTP Basic Authentication.
 *
 * Both username and password must fit in this many bytes including the
 * null terminator.  Longer credentials are silently rejected with 401.
 */
#ifndef MAX_AUTH_LEN
#define MAX_AUTH_LEN 32
#endif

// ---------------------------------------------------------------------------
// Multipart form-data sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneously parsed multipart parts per request.
 *
 * Parts beyond this limit are silently ignored.  A typical upload form
 * has 1-4 fields; increase this for forms with more.
 */
#ifndef MAX_MULTIPART_PARTS
#define MAX_MULTIPART_PARTS 4
#endif

/**
 * @brief Maximum MIME boundary length (RFC 2046 allows up to 70 characters).
 */
#ifndef MAX_BOUNDARY_LEN
#define MAX_BOUNDARY_LEN 72
#endif

// ---------------------------------------------------------------------------
// Event queue depth
// ---------------------------------------------------------------------------

/**
 * @brief Depth of the FreeRTOS event queue shared between lwIP callbacks and
 *        the main-loop task.
 *
 * Each slot holds one TcpEvt (8 bytes).  The queue is the only heap
 * allocation the library makes at begin() time:
 *
 *   heap = sizeof(StaticQueue_t) + EVT_QUEUE_DEPTH * sizeof(TcpEvt)
 *
 * Must be large enough to absorb a burst of MAX_CONNS * 4 events without
 * blocking the lwIP thread, so it tracks MAX_CONNS automatically (a raised
 * MAX_CONNS never trips the EVT_QUEUE_DEPTH >= MAX_CONNS * 4 guard below).
 */
#ifndef EVT_QUEUE_DEPTH
#define EVT_QUEUE_DEPTH (MAX_CONNS * 4)
#endif

// ---------------------------------------------------------------------------
// Internal response buffer sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Stack buffer for HTTP response header lines in send() / send_empty() /
 *        send_unauth() / serve_file().
 *
 * Must be large enough to hold the status line, Content-Type, Content-Length,
 * Connection, and any CORS headers.  The CORS block alone can reach
 * CORS_HDR_BUF_SIZE bytes, so this value should be at least
 * CORS_HDR_BUF_SIZE + 96.
 */
#ifndef RESP_HDR_BUF_SIZE
#define RESP_HDR_BUF_SIZE 768
#endif

/**
 * @brief Per-connection buffer for app-supplied custom response headers and
 *        cookies.
 *
 * Filled by add_response_header() / set_cookie() and injected into send() /
 * send_empty() / redirect() the same way the CORS block is. RESP_HDR_BUF_SIZE
 * must be large enough to hold the status line plus the CORS block plus this
 * block (see the assert below).
 */
#ifndef EXTRA_HDR_BUF_SIZE
#define EXTRA_HDR_BUF_SIZE 256
#endif

/**
 * @brief Stack buffer for the HTTP 101 Switching Protocols response sent during
 *        the WebSocket handshake.
 *
 * Must hold: status line + Upgrade + Connection + Sec-WebSocket-Accept (28
 * base64 chars) + CRLF pairs.  Minimum is ~120 bytes; default leaves margin.
 */
#ifndef WS_HDR_BUF_SIZE
#define WS_HDR_BUF_SIZE 256
#endif

/**
 * @brief Size of the pre-built CORS header block stored in DWS.
 *
 * Built once by set_cors() and injected into every response.  Must hold
 * Access-Control-Allow-Origin, Access-Control-Allow-Methods, and
 * Access-Control-Allow-Headers lines for the configured origin.
 */
#ifndef CORS_HDR_BUF_SIZE
#define CORS_HDR_BUF_SIZE 192
#endif

/**
 * @brief Size of the optional Cache-Control header line stored in DWS.
 *
 * Built once by set_cache_control() and injected into static-file responses
 * (serve_file / serve_static) beside the ETag. Holds "Cache-Control: <value>\r\n".
 */
#ifndef CACHE_CONTROL_BUF_SIZE
#define CACHE_CONTROL_BUF_SIZE 64
#endif

// ---------------------------------------------------------------------------
// Feature flags
// ---------------------------------------------------------------------------
// Set any of these to 0 in your sketch BEFORE including this library to strip
// the feature from the build entirely (no code, no RAM, no flash cost).
//
//   #define DWS_ENABLE_WEBSOCKET 0
//   #include <dwserver.h>
//
// ---------------------------------------------------------------------------
// BUILD-FLAG DEPENDENCY TREE
// ---------------------------------------------------------------------------
// Most features are independent. A few build on another feature and cannot
// compile without it; those HARD dependencies are enforced near the bottom of
// this file with a clear #error, so an illegal combination fails fast at
// compile time instead of producing a cryptic linker error. Enable a child
// only together with its parent(s).
//
// Hard dependencies (child requires parent):
//
//   FILE_SERVING
//     |-- WEBDAV
//     `-- RANGE
//   TLS
//     |-- MTLS
//     |-- TLS_RESUMPTION
//     |-- HTTP_CLIENT_TLS   (also requires HTTP_CLIENT)
//     |-- MQTT_TLS          (also requires MQTT)
//     `-- WS_CLIENT_TLS     (also requires WS_CLIENT)
//   WEBSOCKET
//     |-- WS_DEFLATE
//     `-- WEB_TERMINAL
//   SSE
//     `-- DASHBOARD
//   STATS
//     `-- METRICS
//   AUTH
//     `-- AUTH_LOCKOUT
//   SNMP
//     |-- SnmpVersion::SNMP_V3
//     `-- SNMP_TRAP
//   COAP
//     |-- COAP_OBSERVE
//     `-- COAP_BLOCK
//   OPCUA
//     `-- OPCUA_CLIENT
//   CONFIG_STORE
//     `-- CONFIG_IO
//
// Optional integrations (these build fine on their own; the named feature is
// simply inert or reduced until you also enable the other flag):
//
//   WEBHOOK   + HTTP_CLIENT : without it, dws_webhook_post() returns -1
//   OAUTH2    + HTTP_CLIENT : the token-endpoint POST helpers compile only with it
//   DASHBOARD + WEBSOCKET   : adds live control widgets; the SSE value stream works alone
//
// Auto-derived (do NOT set these yourself; the library computes them):
//
//   STREAM_BODY         = OTA || UPLOAD
//   CLIENT_TLS          = HTTP_CLIENT_TLS || MQTT_TLS || WS_CLIENT_TLS
//   CAPTURE_AUTH_HEADER = AUTH || JWT || OIDC
//
// The same tree appears in README.md and examples/Foundation/05.Configuration.
// ---------------------------------------------------------------------------

/** @brief WebSocket support (RFC 6455 framing + SHA-1/base64 handshake). */
#ifndef DWS_ENABLE_WEBSOCKET
#define DWS_ENABLE_WEBSOCKET 1
#endif

/**
 * @brief WebSocket permessage-deflate (RFC 7692) - bidirectional compression.
 *
 * When set (and DWS_ENABLE_WEBSOCKET is on), the server negotiates the
 * `permessage-deflate` extension and both decompresses inbound compressed (RSV1)
 * messages via a bounded INFLATE (network_drivers/presentation/inflate.*) and
 * compresses outbound data frames via a bounded DEFLATE
 * (network_drivers/presentation/deflate.*); both borrow their table scratch from
 * the shared per-dispatch arena. The extension is negotiated with
 * `{client,server}_no_context_takeover` so every message (de)compresses
 * independently - no window is carried between messages. An outbound frame that
 * would not shrink is sent uncompressed (the per-message RSV1 flag permits this).
 * Default off.
 */
#ifndef DWS_ENABLE_WS_DEFLATE
#define DWS_ENABLE_WS_DEFLATE 0
#endif

/**
 * @brief WebSocket outbound fragmentation size (RFC 6455 sec 5.4), in payload bytes. 0 = off.
 *
 * When >0, an outbound data message (text/binary) longer than this many payload bytes is split into
 * that-sized WebSocket frames - the first carrying the opcode (and the RFC 7692 RSV1 bit if the message
 * is compressed), the rest CONTINUATION, the last with FIN - instead of one large frame. Sizing it near
 * the TCP MSS (e.g. 1400) keeps each frame within whole segments (MTU-aligned) and lets a peer with a
 * bounded per-frame reassembly buffer receive an arbitrarily long message. The runtime override is
 * ws_set_frag_size(). Compression applies to the whole message first, then the compressed bytes are
 * split. Default 0 (one frame per message, unchanged).
 */
#ifndef DWS_WS_FRAG_SIZE
#define DWS_WS_FRAG_SIZE 0
#endif

/** @brief Server-Sent Events push support. */
#ifndef DWS_ENABLE_SSE
#define DWS_ENABLE_SSE 1
#endif

/** @brief multipart/form-data body parser. */
#ifndef DWS_ENABLE_MULTIPART
#define DWS_ENABLE_MULTIPART 1
#endif

/**
 * @brief Zero-heap CBOR (RFC 8949) encoder for compact binary payloads.
 *
 * Default off. When set, network_drivers/presentation/cbor/cbor.h provides a writer
 * that serializes ints, strings, byte strings, arrays, maps, booleans, null, and
 * float32 into a caller-provided buffer - a compact binary alternative to the JSON
 * writer for telemetry. Pure, no heap, host-tested against the RFC 8949 vectors.
 */
#ifndef DWS_ENABLE_CBOR
#define DWS_ENABLE_CBOR 0
#endif

/**
 * @brief Zero-heap MessagePack encoder and decoder for compact binary payloads.
 *
 * Default off. When set, network_drivers/presentation/msgpack/msgpack.h provides a
 * writer that serializes ints, strings, byte strings, arrays, maps, booleans, nil,
 * and float32 into a caller-provided buffer, plus a cursor decoder (dws_msgpack_peek /
 * dws_msgpack_read_*, no-copy strings) over a caller buffer - the MessagePack-format
 * sibling of the CBOR / JSON readers and writers. Pure, no heap, host-tested
 * against the spec encodings and round-trip.
 */
#ifndef DWS_ENABLE_MSGPACK
#define DWS_ENABLE_MSGPACK 0
#endif

/** @brief Static file serving via Arduino FS (LittleFS, SPIFFS, SD). */
#ifndef DWS_ENABLE_FILE_SERVING
#define DWS_ENABLE_FILE_SERVING 1
#endif

/**
 * @brief WebDAV server (RFC 4918, class 1 + advisory locks) over the file system.
 *
 * Default off. When set (requires DWS_ENABLE_FILE_SERVING), dav() mounts an FS
 * subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1),
 * PROPPATCH, GET, HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK -
 * so a client (rclone, cadaver, curl, or a mounted network drive) can browse and
 * edit files. PROPFIND returns a 207 Multi-Status document built into a fixed
 * buffer (DWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at
 * DWS_WEBDAV_MAX_ENTRIES children. PROPPATCH returns a 207 with each requested
 * property refused 403 Forbidden (the live properties are read-only, no dead-
 * property store) - this keeps Windows Explorer / macOS Finder, which PROPPATCH a
 * timestamp right after a PUT, from erroring on a 405. PUT streams the request
 * body straight to the file (via the shared streaming-body sink), so an upload is
 * not bounded by BODY_BUF_SIZE. Locks are advisory (a synthetic token is issued
 * but not enforced). See docs/SECURITY.md before exposing it.
 */
#ifndef DWS_ENABLE_WEBDAV
#define DWS_ENABLE_WEBDAV 0
#endif

/** @brief Buffer (BSS) for a WebDAV 207 Multi-Status response, in bytes (see DWS_ENABLE_WEBDAV). */
#ifndef DWS_WEBDAV_BUF_SIZE
#define DWS_WEBDAV_BUF_SIZE 2048
#endif

/** @brief Maximum children listed in a WebDAV Depth-1 PROPFIND (bounds the response). */
#ifndef DWS_WEBDAV_MAX_ENTRIES
#define DWS_WEBDAV_MAX_ENTRIES 32
#endif

/** @brief Maximum properties echoed in a WebDAV PROPPATCH 207 response (bounds the response). */
#ifndef DWS_WEBDAV_MAX_PROPS
#define DWS_WEBDAV_MAX_PROPS 16
#endif

/**
 * @brief HTTP method-token buffer size (bytes, including the NUL).
 *
 * Sized for the longest method the server must recognize: 8 normally (OPTIONS),
 * grown to fit the WebDAV methods (PROPPATCH is 9 chars) when WebDAV is enabled.
 */
#ifndef DWS_METHOD_BUF_SIZE
#if DWS_ENABLE_WEBDAV
#define DWS_METHOD_BUF_SIZE 12
#else
#define DWS_METHOD_BUF_SIZE 8
#endif
#endif

/** @brief HTTP Basic Authentication per-route. */
#ifndef DWS_ENABLE_AUTH
#define DWS_ENABLE_AUTH 1
#endif

/** @brief Telnet server support (RFC 854 / IAC option negotiation). */
#ifndef DWS_ENABLE_TELNET
#define DWS_ENABLE_TELNET 0
#endif

/** @brief SSH server support (RFC 4253/4252/4254). */
#ifndef DWS_ENABLE_SSH
#define DWS_ENABLE_SSH 0
#endif

/**
 * @brief Post-quantum hybrid key exchange: ML-KEM-768 + X25519 (FIPS 203 / RFC 9370 combiner).
 *
 * Adds the mlkem768x25519-sha256 SSH KEX method (draft-ietf-sshm-mlkem-hybrid-kex) and the
 * X25519MLKEM768 TLS 1.3 group (IANA 0x11ec) for HTTP/3, so a PQC-capable peer (OpenSSH 9.x+ and
 * current browsers, which now DEFAULT to hybrid) negotiates a quantum-resistant handshake instead of
 * down-negotiating to classical X25519. The device is always the responder, so only ML-KEM Encaps
 * ships (no KeyGen/Decaps, so none of the constant-time FO-comparison surface). The NTT core is
 * software with Montgomery reduction over q=3329 (the MPI accelerator is for RSA/DH-sized operands,
 * not 12-bit coefficients). Requires DWS_ENABLE_SSH and/or DWS_ENABLE_HTTP3.
 */
#ifndef DWS_ENABLE_PQC_KEX
#define DWS_ENABLE_PQC_KEX 0
#endif

/**
 * @brief Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502.
 *
 * Default off. When set, listen(502, ConnProto::PROTO_MODBUS) serves a fixed data model
 * (coils, discrete inputs, holding + input registers, all in BSS) over Modbus
 * TCP: Read/Write Coils (FC 1/5/15), Read Discrete Inputs (FC 2), Read/Write
 * Holding Registers (FC 3/6/16), and Read Input Registers (FC 4). The codec
 * (MBAP framing + PDU dispatch) is pure and host-tested; the TCP transport is
 * ESP32-only. The application reads/writes the model with the accessor functions
 * and is notified of client writes via dws_modbus_on_write(). Modbus has no
 * authentication or encryption - run it only on a trusted control network.
 */
#ifndef DWS_ENABLE_MODBUS
#define DWS_ENABLE_MODBUS 0
#endif

/**
 * @brief Modbus RTU framing (serial / RS-485) over the same data model + PDU dispatch.
 *
 * Default off; implies DWS_ENABLE_MODBUS. Adds the RTU ADU codec
 * `dws_modbus_rtu_process_adu()` - a `[slave addr][PDU][CRC16]` frame (CRC16-Modbus,
 * little-endian) around the existing host-tested PDU dispatch: a CRC mismatch or a
 * non-matching unit address is dropped silently (no reply, per the spec), and a
 * broadcast (address 0) is executed without a reply. The codec is pure and
 * host-tested; feed it from a UART/RS-485 driver (the serial transport is the
 * application's, framed by the 3.5-char inter-frame idle).
 */
#ifndef DWS_ENABLE_MODBUS_RTU
#define DWS_ENABLE_MODBUS_RTU 0
#endif
#if DWS_ENABLE_MODBUS_RTU && !DWS_ENABLE_MODBUS
#undef DWS_ENABLE_MODBUS
#define DWS_ENABLE_MODBUS 1
#endif

/**
 * @brief CloudEvents v1.0 (CNCF) event envelope (structured JSON + binary headers).
 *
 * Default off. Adds `services/cloudevents`: `dws_cloudevents_build_json()` emits a
 * structured `application/cloudevents+json` envelope (required `id`/`source`/`type`
 * + optional `subject`/`datacontenttype`/`data`) via the JSON writer, and
 * `dws_cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers.
 * Makes the device's events interoperable with serverless / event-mesh consumers.
 */
#ifndef DWS_ENABLE_CLOUDEVENTS
#define DWS_ENABLE_CLOUDEVENTS 0
#endif

/**
 * @brief Redis RESP2 wire codec (`services/redis_resp`).
 *
 * Default off. A zero-heap command encoder (`dws_resp_encode_command`, array of bulk
 * strings) + a cursor reply parser (`dws_resp_parse`: simple / error / integer / bulk /
 * array / nil) so the device can drive a Redis server over the shipped outbound
 * client transport. Pure codec, host-tested; the connection is the application's.
 */
#ifndef DWS_ENABLE_REDIS
#define DWS_ENABLE_REDIS 0
#endif

/**
 * @brief STOMP 1.2 frame codec (`services/stomp`).
 *
 * Default off. A zero-heap frame builder (`dws_stomp_build_frame`, command + escaped
 * headers + NUL-terminated body) + a non-mutating parser (`dws_stomp_parse_frame`,
 * command / header slices / body, honoring `content-length`) so the device can talk
 * to a STOMP broker (ActiveMQ / RabbitMQ / Artemis) over the shipped outbound client
 * transport, or STOMP-over-WebSocket via the WS client. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_STOMP
#define DWS_ENABLE_STOMP 0
#endif

/** @brief Max header lines parsed per STOMP frame (extras beyond this are ignored). */
#ifndef DWS_STOMP_MAX_HEADERS
#define DWS_STOMP_MAX_HEADERS 16
#endif

/**
 * @brief MQTT-SN v1.2 wire codec (`services/mqtt/mqtt_sn`).
 *
 * Default off. A zero-heap message builder + parser for MQTT for Sensor Networks - the
 * UDP / non-TCP MQTT variant for constrained, lossy links (numeric topic IDs instead of
 * strings, gateway discovery, sleeping-client keep-alive). Builds CONNECT / REGISTER /
 * PUBLISH / SUBSCRIBE / PINGREQ / DISCONNECT / SEARCHGW and parses CONNACK / REGACK /
 * PUBACK / SUBACK / PUBLISH / REGISTER, including the 1- and 3-octet Length forms. Pure
 * codec, host-tested; the datagram send (dws_udp_sendto) and topic registry are the app's.
 */
#ifndef DWS_ENABLE_MQTT_SN
#define DWS_ENABLE_MQTT_SN 0
#endif

/**
 * @brief Flow-record export codec (`services/flow_export`).
 *
 * Default off. A zero-heap exporter-side codec for on-device flow accounting: NetFlow v5
 * (fixed 24-octet header + 48-octet records), NetFlow v9 (RFC 3954), and IPFIX (RFC 7011),
 * the latter two via a small cursor that emits a Template then matching Data records and
 * patches the message length (IPFIX) or record count (v9) on finish. Pure codec,
 * host-tested; the flow cache (5-tuple + counters) and the UDP send (dws_udp_sendto) are
 * the application's. Pairs with the telemetry / observability services.
 */
#ifndef DWS_ENABLE_FLOW_EXPORT
#define DWS_ENABLE_FLOW_EXPORT 0
#endif

/**
 * @brief Protocol Buffers wire codec (`services/protobuf`).
 *
 * Default off. A zero-heap streaming Protobuf encoder + cursor reader over caller buffers
 * (the same shape as the CBOR / MessagePack codecs): varint / ZigZag / fixed32 / fixed64 /
 * length-delimited fields, with embedded messages built into a sub-buffer and added via
 * `dws_pb_bytes`. Pure codec, host-tested against the spec vectors. This is the standalone
 * Protobuf deliverable; gRPC (framed Protobuf over HTTP/2) is gated on the HTTP/2 item.
 */
#ifndef DWS_ENABLE_PROTOBUF
#define DWS_ENABLE_PROTOBUF 0
#endif

/**
 * @brief WAMP messaging codec (`services/wamp`).
 *
 * Default off. A zero-heap codec for the Web Application Messaging Protocol (unified RPC +
 * PubSub over WebSocket): builders for HELLO / SUBSCRIBE / PUBLISH / CALL / REGISTER /
 * YIELD / GOODBYE (JSON arrays emitted via the shared JsonWriter) and a positional parser
 * that pulls the message type, ids, and URIs out of an inbound array. Rides the shipped
 * WebSocket layer; the session / subscription / registration tables are the application's.
 * Pure codec, host-tested. Builds on the always-on JSON writer.
 */
#ifndef DWS_ENABLE_WAMP
#define DWS_ENABLE_WAMP 0
#endif

/**
 * @brief SunSpec Modbus device-information-model codec (`services/sunspec`).
 *
 * Default off. A zero-heap codec for the SunSpec Alliance register maps layered on the
 * holding-register model: a model-chain walker (verify the `SunS` marker, then iterate each
 * model's id / length / body) + typed point readers (u16 / i16 / u32 / i32 / string) and a
 * map writer (marker, model headers + points, end model). Makes a solar inverter / meter /
 * battery interoperable. Pure codec, host-tested; pairs with the Modbus service.
 */
#ifndef DWS_ENABLE_SUNSPEC
#define DWS_ENABLE_SUNSPEC 0
#endif

/**
 * @brief IEEE C37.118.2 synchrophasor frame codec (`services/c37118`).
 *
 * Default off. A zero-heap builder + CRC-validating parser for the PMU / PDC wide-area
 * measurement wire protocol: `dws_c37118_build_frame` / `dws_c37118_build_command` emit a
 * `SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK` frame (CHK = CRC-CCITT) and
 * `dws_c37118_parse_frame` validates the CRC and reports the frame type / ids / timestamp /
 * payload slice. Frames any payload and fully handles the fixed Command frame. Pure codec,
 * host-tested.
 */
#ifndef DWS_ENABLE_C37118
#define DWS_ENABLE_C37118 0
#endif

/**
 * @brief DNP3 (IEEE 1815) data-link frame codec (`services/dnp3`).
 *
 * Default off. A zero-heap builder + CRC-validating parser for the SCADA / utility
 * outstation data-link layer: `dws_dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC`
 * header block + the CRC'd 16-octet user-data blocks, and `dws_dnp3_parse_frame` validates the
 * header and every block CRC (CRC-16/DNP) and de-blocks the user data. Pure codec,
 * host-tested; the transport-function reassembly and the application layer are layered on
 * the de-blocked user data.
 */
#ifndef DWS_ENABLE_DNP3
#define DWS_ENABLE_DNP3 0
#endif

/**
 * @brief CANopen (CiA 301) message codec (`services/canopen`).
 *
 * Default off. A zero-heap builder + parser for the CANopen messaging set over classic CAN
 * frames (`shared_primitives/can.h`): NMT node control, SYNC, TIME, heartbeat / boot-up,
 * EMCY, PDO process data, and expedited SDO read / write / abort. The 11-bit COB-ID is a
 * 4-bit function code plus a 7-bit node id; builders compute it, parsers classify it back.
 * The object dictionary is the application's; SDO is expedited only (segmented / block not
 * yet covered). Pure codec, host-tested. Drive it from the ESP32 TWAI peripheral or an
 * MCP2515 over SPI to bridge a CANopen bus onto Wi-Fi.
 */
#ifndef DWS_ENABLE_CANOPEN
#define DWS_ENABLE_CANOPEN 0
#endif

/**
 * @brief CiA 402 / IEC 61800-7-201 drive + motion profile (`services/cia402`).
 *
 * Default off. Requires CANOPEN. The standardised servo / stepper drive profile over CANopen:
 * `dws_cia402_state` decodes the power state machine from the Statusword (the CiA 402 mask/value
 * table), `dws_cia402_controlword` / `dws_cia402_enable_sequence` produce the Controlword commands that
 * walk an axis to Operation Enabled, and the `dws_cia402_sdo_set_*` / `dws_cia402_pack_command` helpers
 * write Controlword / Modes of Operation / target position-velocity-torque via the shipped
 * CANopen SDO / PDO codec. State masks + command values + object indices verified against
 * IEC 61800-7-201. Pure profile, host-tested. Turns the CAN stack into a motion master; close
 * the loop with a `services/control` PID.
 */
#ifndef DWS_ENABLE_CIA402
#define DWS_ENABLE_CIA402 0
#endif

/**
 * @brief Closed-loop control law (`services/control`).
 *
 * Default off. A zero-heap, FPU-accelerated PID controller (single-precision float, FMA-folded,
 * IRAM-placeable with DWS_CONTROL_IRAM=1) with derivative-on-measurement, an optional
 * derivative low-pass, output clamping, and anti-windup by back-calculation plus a hard integral
 * clamp, and a feed-forward term - plus inline control-law primitives (clamp / deadband / slew /
 * low-pass). `pid_update` runs one loop; `pid_update_n` runs a batch of axes off one tick. Pair
 * it with a plant it can command (a `services/cia402` drive, a dshot ESC, a heater) and tune the
 * gains offline with `tools/pid_tune.py`. Pure math, host-tested.
 */
#ifndef DWS_ENABLE_CONTROL
#define DWS_ENABLE_CONTROL 0
#endif

/**
 * @brief SAE J1939 message codec (`services/j1939`).
 *
 * Default off. A zero-heap codec for the heavy-duty-vehicle / agriculture / marine / genset
 * CAN higher-layer protocol over 29-bit extended frames (`shared_primitives/can.h`):
 * `dws_j1939_encode_id` / `dws_j1939_decode_id` pack and unpack the priority / PGN / SA / DA
 * identifier (PDU1 peer + PDU2 broadcast), `dws_j1939_build_message` emits single frames,
 * `dws_j1939_build_request` / `dws_j1939_build_address_claim` (+ `dws_j1939_build_name`) handle the
 * Request PGN and Address Claimed messages, and the Transport Protocol (BAM announce +
 * TP.DT packets) reassembles multi-packet messages up to `DWS_J1939_TP_MAX` octets. Pure
 * codec, host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI.
 */
#ifndef DWS_ENABLE_J1939
#define DWS_ENABLE_J1939 0
#endif

/**
 * @brief DeviceNet link-adaptation codec (`services/devicenet`).
 *
 * Default off. The CAN-specific layer of "CIP over CAN": the 11-bit DeviceNet identifier as a
 * Message Group (1..4) + Message ID + MAC ID (`dws_devicenet_encode_id` / `dws_devicenet_decode_id`),
 * the explicit-message header octet, single-frame explicit messages, and the fragmentation
 * protocol with a reassembler (`dws_devicenet_frag_feed`) for bodies longer than one CAN frame.
 * The CIP application layer (services / EPATH / data) is the same one EtherNet/IP uses, so
 * build the body with the existing `dws_cip_*` functions (`DWS_ENABLE_CIP`). Pure codec,
 * host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI.
 */
#ifndef DWS_ENABLE_DEVICENET
#define DWS_ENABLE_DEVICENET 0
#endif

/**
 * @brief NMEA 2000 codec (`services/nmea2000`).
 *
 * Default off; implies DWS_ENABLE_J1939 (NMEA 2000 is J1939 at the transport layer). A
 * zero-heap codec for the marine instrumentation network over CAN: it reuses the J1939 29-bit
 * identifier codec and adds the NMEA-specific Fast Packet transport - `dws_n2k_fastpacket_build_frame`
 * splits a 9..223-octet message across frames (a control octet of sequence + frame counter,
 * the first frame carrying the total length) and `dws_n2k_fastpacket_feed` reassembles it;
 * `dws_n2k_build_single` wraps a single-frame message. Pure codec, host-tested. Drive it from the
 * ESP32 TWAI peripheral or an MCP2515 over SPI to bridge an NMEA 2000 backbone onto Wi-Fi.
 */
#ifndef DWS_ENABLE_NMEA2000
#define DWS_ENABLE_NMEA2000 0
#endif
#if DWS_ENABLE_NMEA2000 && !DWS_ENABLE_J1939
#undef DWS_ENABLE_J1939
#define DWS_ENABLE_J1939 1 // NMEA 2000 reuses the J1939 identifier codec
#endif

/**
 * @brief Wired M-Bus (Meter-Bus, EN 13757) frame codec (`services/mbus`).
 *
 * Default off. A zero-heap builder + parser for the M-Bus link-layer frames used by utility
 * meters (water / gas / heat / electricity): the single-character ACK, the short frame
 * (`10 C A CS 16`), and the long / control frame (`68 L L 68 C A CI ... CS 16`, 8-bit sum
 * checksum), plus `dws_mbus_record_next` which walks the EN 13757-3 variable-data records
 * (DIF / VIF, skipping DIFE / VIFE extension chains and decoding the data length). Pure codec,
 * host-tested. Talk to the powered two-wire bus over a UART through an M-Bus level converter
 * (e.g. a TSS721-based master) and bridge meter readings onto Wi-Fi.
 */
#ifndef DWS_ENABLE_MBUS
#define DWS_ENABLE_MBUS 0
#endif

/**
 * @brief IEC 60870-5-101 / -104 telecontrol (SCADA) codec (`services/iec60870`).
 *
 * Default off. The utility-SCADA protocol in both transports: the -104 APCI over TCP
 * (`68 LEN` + 4 control octets in I / S / U formats via `dws_iec104_build_i/_s/_u` + `dws_iec104_parse`),
 * the shared ASDU header + 3-octet Information Object Address (`dws_iec_asdu_build_header` /
 * `dws_iec_asdu_parse_header`, `dws_iec_put_ioa` / `dws_iec_get_ioa`), and the -101 FT1.2 serial link
 * frames (fixed + variable, 8-bit sum checksum, via `dws_iec101_build_fixed` / `_variable` +
 * `dws_iec101_parse`). Named type-id / cause-of-transmission constants are provided; the
 * per-type information elements are the application's. Pure codec, host-tested. Run -104 over
 * the shipped TCP stack or -101 over a UART/RS-485 transceiver to bridge an RTU onto Wi-Fi.
 */
#ifndef DWS_ENABLE_IEC60870
#define DWS_ENABLE_IEC60870 0
#endif

/**
 * @brief SDI-12 sensor-bus codec (`services/sdi12`).
 *
 * Default off. A zero-heap command / response codec for the 1200-baud single-wire ASCII bus
 * used by environmental / agricultural sensors: builders for the standard commands
 * (`dws_sdi12_build_measure` / `_concurrent` / `_data` / `_identify` / `_change_address` /
 * `_query_address`), a parser for the measurement response (`dws_sdi12_parse_measure`: seconds
 * until ready + value count), a data-value splitter (`dws_sdi12_parse_values`), and the SDI-12
 * CRC (`dws_sdi12_crc16` / `dws_sdi12_crc_encode` / `dws_sdi12_check_crc`) for the CRC-protected `aMC!` /
 * `aCC!` variants. Pure codec, host-tested. Drive the single 1200-baud line over a UART (with
 * a small level / direction circuit) and bridge sensor readings onto Wi-Fi.
 */
#ifndef DWS_ENABLE_SDI12
#define DWS_ENABLE_SDI12 0
#endif

/**
 * @brief DMX512 + RDM (ANSI E1.20) lighting codec (`services/dmx`).
 *
 * Default off. A zero-heap codec for stage / architectural lighting over RS-485: `dws_dmx_build` /
 * `dws_dmx_get_channel` assemble and read the positional DMX512 slot packet (a start code + up to
 * 512 channels), and the RDM (Remote Device Management) functions build / parse the addressed
 * management packet that shares the wire - `dws_rdm_build` / `dws_rdm_parse` with 48-bit source /
 * destination UIDs (`dws_rdm_uid`), a command class + parameter id, and the 16-bit additive
 * checksum (`dws_rdm_checksum`). Pure codec, host-tested. Drive a `MAX485`-class transceiver on a
 * UART (250 kbit/s, 8N2; the break is the application's) and bridge a lighting rig onto Wi-Fi.
 */
#ifndef DWS_ENABLE_DMX
#define DWS_ENABLE_DMX 0
#endif

/**
 * @brief NMEA 0183 sentence codec (`services/nmea0183`).
 *
 * Default off. A zero-heap codec for the marine / GPS ASCII protocol (`$GPGGA,...*47`):
 * `dws_nmea0183_build` emits a sentence (adding the `$`, XOR checksum, and CR/LF), `dws_nmea0183_parse`
 * validates the `*HH` checksum and splits the comma-separated fields (deriving talker id +
 * sentence type from the address field), and `dws_nmea0183_field_float` / `_int` decode field
 * values. Sentence framing + checksum verified against the NMEA 0183 standard (the canonical
 * GGA example); pure and host-tested. GPS / marine receivers are cheap UART breakouts, so this
 * is a plain HardwareSerial link (4800 / 9600 baud); bridge position / wind / depth onto Wi-Fi.
 */
#ifndef DWS_ENABLE_NMEA0183
#define DWS_ENABLE_NMEA0183 0
#endif

/**
 * @brief IO-Link (SDCI, IEC 61131-9) data-link message codec (`services/iolink`).
 *
 * Default off. The point-to-point smart-sensor link's data-link message layer: the M-sequence
 * Control octet (`dws_iol_mc` + decoders), the checksum / type octet of a master message
 * (`dws_iol_ckt`), the checksum / status octet of a device reply (`dws_iol_cks`), and the SDCI message
 * checksum (`dws_iol_checksum6` / `dws_iol_finalize` / `dws_iol_verify`) implemented straight from IO-Link
 * spec v1.1.4 Annex A.1.6 (the 0x52 seed + the 8-to-6-bit compression of equation A.1). Lay
 * the M-sequence / ISDU octets out per your device profile, then finalize / verify with this
 * codec. Pure codec, host-tested. The wire is a UART through an IO-Link transceiver
 * (e.g. MAX14819 / L6360); bridge sensor data onto Wi-Fi.
 */
#ifndef DWS_ENABLE_IOLINK
#define DWS_ENABLE_IOLINK 0
#endif

/**
 * @brief gRPC-Web message framing (`services/grpcweb`).
 *
 * Default off. A zero-heap length-prefixed frame builder + parser for gRPC-Web, the
 * HTTP/1.1-reachable subset of gRPC (gRPC proper needs HTTP/2). `dws_grpcweb_frame_message`
 * wraps a Protobuf message in the 5-octet `[flags][len BE32]` prefix, `dws_grpcweb_frame_trailer`
 * emits the 0x80 trailers frame (`grpc-status` / `grpc-message`), and `dws_grpcweb_parse` reads
 * one frame back. Wraps the Protobuf codec (`DWS_ENABLE_PROTOBUF`) over the shipped
 * HTTP/1.1 server/client. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_GRPC_WEB
#define DWS_ENABLE_GRPC_WEB 0
#endif

/**
 * @brief OMA LwM2M TLV codec (`services/lwm2m`).
 *
 * Default off. A zero-heap writer + cursor reader for the LwM2M `application/vnd.oma.lwm2m+tlv`
 * resource encoding (Type / Identifier / Length / Value, 8-/16-bit ids, 0-/8-/16-/24-bit
 * lengths), carried over the shipped CoAP service for device management. Value helpers for
 * shortest-form integers, booleans, strings, and floats. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_LWM2M
#define DWS_ENABLE_LWM2M 0
#endif

/**
 * @brief Omron FINS frame codec (`services/fins`).
 *
 * Default off. A zero-heap command/response builder + parser for the Factory Interface
 * Network Service (FINS/UDP): `dws_fins_build_command` / `dws_fins_build_memory_area_read` emit the
 * 10-octet routing header + command code + parameters, and `dws_fins_parse_command` /
 * `dws_fins_parse_response` read them back (the response end code MRES/SRES included). Talks to
 * an Omron PLC over the shipped UDP transport (dws_udp_sendto). Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_FINS
#define DWS_ENABLE_FINS 0
#endif

/**
 * @brief Omron Host Link (C-mode) frame codec (`services/hostlink`).
 *
 * Default off. A zero-heap ASCII command/response codec for the Omron serial host-link
 * protocol (the RS-232/485 sibling of FINS): `dws_hostlink_build` emits `@UU` + header code +
 * text + FCS + `*`CR, and `dws_hostlink_parse` FCS-validates and splits a frame
 * (`dws_hostlink_end_code` reads a response's end code). FCS is the 8-bit XOR from `@` through
 * the text. Pure codec, host-tested; the serial transport is the application's.
 */
#ifndef DWS_ENABLE_HOSTLINK
#define DWS_ENABLE_HOSTLINK 0
#endif

/**
 * @brief SenML (RFC 8428) measurement-pack builder (`services/senml`).
 *
 * Default off; implies DWS_ENABLE_CBOR (the SenML-CBOR form uses the CBOR writer). A
 * zero-heap SenML-JSON + SenML-CBOR encoder over the shipped JSON / CBOR codecs: the caller
 * fills a `SenmlRecord` array (base name/time, name, unit, one value, time) and
 * `dws_senml_json_build` / `dws_senml_cbor_build` emit the whole pack. Numbers are emitted as
 * integers when integral (so timestamps keep precision), else floats. The standard
 * measurement format for CoAP / LwM2M / HTTP telemetry. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_SENML
#define DWS_ENABLE_SENML 0
#endif
#if DWS_ENABLE_SENML && !DWS_ENABLE_CBOR
#undef DWS_ENABLE_CBOR
#define DWS_ENABLE_CBOR 1
#endif

/**
 * @brief Allen-Bradley DF1 full-duplex frame codec (`services/df1`).
 *
 * Default off. A zero-heap framing + DLE byte-stuffing + BCC/CRC codec for the Rockwell
 * serial PLC data-link layer (pub. 1770-6.5.16): `dws_df1_build_frame` wraps application data in
 * `DLE STX ... DLE ETX` with a doubled-DLE escape and a BCC (2's complement of the data sum)
 * or CRC-16 (over the data + ETX, low byte first), and `dws_df1_parse_frame` validates the check
 * and un-stuffs the data. Pure codec, host-tested; the application header is the app's.
 */
#ifndef DWS_ENABLE_DF1
#define DWS_ENABLE_DF1 0
#endif

/**
 * @brief TPKT (RFC 1006) + COTP (X.224 class 0) frame codec (`services/cotp`).
 *
 * Default off. A zero-heap "ISO transport on TCP" framing codec - the reusable foundation
 * under S7comm and IEC 61850 MMS. `dws_tpkt_build` / `dws_tpkt_parse` handle the 4-octet TPKT
 * envelope; `dws_cotp_build_dt` wraps user data in a Data TPDU, `dws_cotp_build_cr` builds a
 * Connection Request (with the TPDU-size parameter + caller TSAP params), and `dws_cotp_parse`
 * reports the TPDU type and the DT data / CR-CC refs. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_COTP
#define DWS_ENABLE_COTP 0
#endif

/**
 * @brief Siemens S7comm PDU codec (`services/s7comm`).
 *
 * Default off. A zero-heap builder + parser for the S7-300/400 communication PDUs carried
 * inside a COTP Data TPDU (DWS_ENABLE_COTP) over ISO-on-TCP (port 102): `dws_s7_build_setup`
 * (Setup Communication), `dws_s7_build_read_request` (Read Var, S7-ANY items over DB/I/Q/M),
 * `dws_s7_parse_header`, and `dws_s7_read_next_item` (the response data items, honoring the
 * length-in-bits transport sizes + even-item padding). Constants verified against the
 * Wireshark S7comm dissector. Pure codec, host-tested; wrap the PDU with COTP + TPKT.
 */
#ifndef DWS_ENABLE_S7COMM
#define DWS_ENABLE_S7COMM 0
#endif

/**
 * @brief Mitsubishi MELSEC MC protocol (binary 3E) codec (`services/melsec`).
 *
 * Default off. A zero-heap batch-read request builder + response parser for MELSEC PLCs over
 * TCP/UDP: `dws_melsec_build_read` emits the binary 3E batch-read (word) frame (little-endian
 * fields, subheader 0x5000, command 0x0401, the device code + 24-bit head device + point
 * count), and `dws_melsec_parse_response` validates the 0xD000 response and reports the end code
 * + the read data. Frame layout + device codes verified against a third-party MC impl. Pure
 * codec, host-tested. Completes the major-vendor PLC read set (FINS / Host Link / DF1 / S7).
 */
#ifndef DWS_ENABLE_MELSEC
#define DWS_ENABLE_MELSEC 0
#endif

/**
 * @brief Beckhoff ADS / AMS protocol codec (`services/ads`).
 *
 * Default off. A zero-heap builder + parser for the TwinCAT PC-based-control protocol over TCP
 * 48898: `dws_ads_build_*` emit complete AMS/TCP + AMS-header frames (little-endian, target-before-
 * source addressing, cmd id + state flags + cbData + invoke id) for ReadDeviceInfo / Read /
 * Write / ReadWrite / ReadState / WriteControl / Add+DeleteNotification, and `dws_ads_parse_*` decode
 * the responses (including the DeviceNotification stamp/sample stream). ReadWrite drives symbol-
 * by-name access (name -> handle via index group 0xF003, value via 0xF005). AMS header layout +
 * command ids verified against the Beckhoff InfoSys spec. Pure codec, host-tested; the caller
 * owns the TCP socket and the AMS route on the target router.
 */
#ifndef DWS_ENABLE_ADS
#define DWS_ENABLE_ADS 0
#endif

/**
 * @brief FANUC FOCAS Ethernet protocol codec (`services/focas`).
 *
 * Default off. A zero-heap builder + parser for the FANUC CNC data protocol over TCP 8193:
 * `dws_focas_build_*` emit the complete on-wire frames (a 10-octet big-endian envelope + payload) for
 * the open/close handshake and the documented read functions (SysInfo, alarm status, CNC
 * parameters, macro variables, position/axis data, actual feed / spindle), and `dws_focas_parse_*`
 * decode the responses (echoed selector + status + data), including the ODBSYS SysInfo layout and
 * the FANUC 8-octet `data / base^exp` value encoding. Frame layout, selector encoding, and value
 * decoding reverse-engineered by and cross-checked against diohpix/pyfanuc. Pure codec, host-
 * tested; the caller owns the TCP socket and drives the open -> command -> close sequence.
 */
#ifndef DWS_ENABLE_FOCAS
#define DWS_ENABLE_FOCAS 0
#endif

/**
 * @brief BACnet/IP BVLC + NPDU codec (`services/bacnet`).
 *
 * Default off. A zero-heap framing codec for the ASHRAE 135 building-automation network
 * layer over UDP (47808): `dws_bvlc_build` / `dws_bvlc_parse` handle the BVLC envelope (type 0x81,
 * function, length), and `dws_npdu_build` / `dws_npdu_parse` handle the NPDU (version + NPCI control
 * + optional DNET/DADR destination addressing + hop count) and slice the APDU. The APDU
 * (application-layer services / object model) layers on top. Pure codec, host-tested.
 */
#ifndef DWS_ENABLE_BACNET
#define DWS_ENABLE_BACNET 0
#endif

/**
 * @brief EtherNet/IP encapsulation codec (`services/enip`).
 *
 * Default off. A zero-heap builder + parser for the ODVA EtherNet/IP encapsulation layer
 * (TCP/UDP 44818) that carries CIP: `dws_eip_build` / `dws_eip_parse` handle the 24-octet header
 * (little-endian command / length / session handle / status / sender context / options),
 * `dws_eip_build_register_session` opens a session, and `dws_eip_build_send_rr_data` /
 * `dws_eip_parse_send_rr_data` wrap + unwrap a CIP message as an unconnected message (Common
 * Packet Format: Null Address + Unconnected Data items). Commands + CPF item types verified
 * against the Wireshark ENIP dissector. Pure codec, host-tested; the CIP message is the app's.
 */
#ifndef DWS_ENABLE_ENIP
#define DWS_ENABLE_ENIP 0
#endif

/**
 * @brief AMQP 0-9-1 frame codec (`services/amqp`).
 *
 * Default off. A zero-heap frame builder + parser for the RabbitMQ wire protocol so a device
 * can be an AMQP client: `dws_amqp_protocol_header` (the `"AMQP" 0 0 9 1` preamble),
 * `dws_amqp_build_frame` / `dws_amqp_parse_frame` (type + channel + size + payload + the 0xCE
 * frame-end), `dws_amqp_build_method` / `dws_amqp_parse_method` (a METHOD frame's class-id /
 * method-id / arguments), and `dws_amqp_build_heartbeat`. Pure codec, host-tested; the method
 * arguments and the connection are the application's. Rides the outbound client transport.
 */
#ifndef DWS_ENABLE_AMQP
#define DWS_ENABLE_AMQP 0
#endif

/**
 * @brief CIP (Common Industrial Protocol) message codec (`services/cip`).
 *
 * Default off. A zero-heap CIP request builder + response parser for the message that rides
 * inside an EtherNet/IP Unconnected Data item (DWS_ENABLE_ENIP): `dws_cip_build_epath` (the
 * class/instance/attribute logical-segment EPATH), `dws_cip_build_request` /
 * `dws_cip_build_get_attr_single`, and `dws_cip_parse_response` (service / general status / data).
 * Service codes + the logical-segment encoding verified against the Wireshark CIP dissector.
 * Pure codec, host-tested; wrap the request with `dws_eip_build_send_rr_data` for a working read.
 */
#ifndef DWS_ENABLE_CIP
#define DWS_ENABLE_CIP 0
#endif

/**
 * @brief NATS client protocol codec (`services/nats`).
 *
 * Default off. A zero-heap builder + parser for the text-based NATS pub/sub protocol so a
 * device can be a NATS client: `dws_nats_build_connect` / `dws_nats_build_pub` / `dws_nats_build_sub` /
 * `dws_nats_build_unsub` / `dws_nats_build_ping` / `dws_nats_build_pong`, and `dws_nats_parse` which decodes
 * an inbound MSG / INFO / PING / PONG / +OK / -ERR (MSG yields subject / sid / reply-to /
 * payload). Line-oriented (CRLF), space-delimited; only PUB and MSG carry a payload. Pure
 * codec, host-tested; rides the outbound client transport.
 */
#ifndef DWS_ENABLE_NATS
#define DWS_ENABLE_NATS 0
#endif

/**
 * @brief HAProxy PROXY protocol codec (`services/proxy_protocol`).
 *
 * Default off. A zero-heap parser + builder for the PROXY protocol header a load balancer /
 * reverse proxy prepends, so the server recovers the real client IPv4 behind one.
 * `proxy_parse` detects + decodes a v1 (text `PROXY TCP4 ...`) or v2 (binary signature +
 * ver_cmd / fam / address block) header and reports the bytes to skip; `proxy_v1_build` /
 * `proxy_v2_build` emit a TCP/IPv4 header. Pure codec, host-tested; the application feeds it
 * the first bytes of an accepted connection.
 */
#ifndef DWS_ENABLE_PROXY_PROTOCOL
#define DWS_ENABLE_PROXY_PROTOCOL 0
#endif

/**
 * @brief Sparkplug B payload + topic codec (`services/sparkplug`).
 *
 * Default off; implies DWS_ENABLE_PROTOBUF (the payload is a Protobuf message). A zero-heap
 * builder for the Eclipse Sparkplug B industrial-IoT MQTT payload (`dws_spb_build_payload` /
 * `dws_spb_build_metric`, over the protobuf codec) and its topic namespace (`dws_spb_build_topic`,
 * `spBv1.0/group/type/node[/device]`). Field numbers + datatype codes verified against the
 * Eclipse Tahu sparkplug_b.proto. Pure codec, host-tested; publish it with the MQTT client.
 */
#ifndef DWS_ENABLE_SPARKPLUG
#define DWS_ENABLE_SPARKPLUG 0
#endif
#if DWS_ENABLE_SPARKPLUG && !DWS_ENABLE_PROTOBUF
#undef DWS_ENABLE_PROTOBUF
#define DWS_ENABLE_PROTOBUF 1
#endif

/** @brief Max serialized size of one Sparkplug B metric submessage (stack temp, bytes). */
#ifndef DWS_SPB_METRIC_MAX
#define DWS_SPB_METRIC_MAX 256
#endif

/**
 * @brief Opt-in Modbus master codec + register scanner (DWS_ENABLE_MODBUS_MASTER).
 *
 * Default off. services/modbus/dws_modbus_master builds Modbus TCP read-request ADUs
 * and parses the responses (register values or exception), so an app can poll /
 * auto-discover a slave's registers. Pure and host-tested as a full round-trip
 * against the slave codec (dws_modbus_process_adu); the actual send is the app's TCP.
 */
#ifndef DWS_ENABLE_MODBUS_MASTER
#define DWS_ENABLE_MODBUS_MASTER 0
#endif

/** @brief Number of Modbus coils (FC 1/5/15), single-bit R/W (BSS, bit-packed). */
#ifndef DWS_MODBUS_COILS
#define DWS_MODBUS_COILS 64
#endif

/** @brief Number of Modbus discrete inputs (FC 2), single-bit read-only (BSS, bit-packed). */
#ifndef DWS_MODBUS_DISCRETE_INPUTS
#define DWS_MODBUS_DISCRETE_INPUTS 64
#endif

/** @brief Number of Modbus holding registers (FC 3/6/16), 16-bit R/W (BSS). */
#ifndef DWS_MODBUS_HOLDING_REGS
#define DWS_MODBUS_HOLDING_REGS 64
#endif

/** @brief Number of Modbus input registers (FC 4), 16-bit read-only (BSS). */
#ifndef DWS_MODBUS_INPUT_REGS
#define DWS_MODBUS_INPUT_REGS 64
#endif

/**
 * @brief TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only).
 *
 * When set, the server can accept TLS connections using mbedTLS configured with
 * MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DWS_TLS_ARENA_SIZE) -
 * no system heap, so the determinism guarantee is preserved. The TLS engine is
 * compiled only on Arduino/ESP32 (mbedTLS is not part of the native build).
 * Default off.
 */
#ifndef DWS_ENABLE_TLS
#define DWS_ENABLE_TLS 0
#endif

/** @brief Maximum simultaneous TLS connections (each holds mbedTLS record buffers). */
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 1
#endif

/**
 * @brief TLS session resumption via RFC 5077 session tickets (requires DWS_ENABLE_TLS).
 *
 * Default off. When set, the TLS 1.2 server issues encrypted session tickets and
 * accepts them on reconnect, so a returning client completes an abbreviated
 * handshake (no certificate or full key exchange) - much faster and far less CPU
 * than the ~RSA/ECDHE full handshake. Resumption is stateless: the session state
 * lives in the client's ticket, sealed with a server-held key, so there is no
 * growing per-session cache (the determinism / zero-heap-growth guarantee holds;
 * only a small fixed ticket key and a little arena headroom are added). The ticket
 * key rotates automatically on the DWS_TLS_TICKET_LIFETIME_S schedule. Needs the
 * mbedTLS build to provide MBEDTLS_SSL_TICKET_C (stock arduino-esp32 does).
 */
#ifndef DWS_ENABLE_TLS_RESUMPTION
#define DWS_ENABLE_TLS_RESUMPTION 0
#endif

/** @brief Session-ticket lifetime / key-rotation period in seconds (see DWS_ENABLE_TLS_RESUMPTION). */
#ifndef DWS_TLS_TICKET_LIFETIME_S
#define DWS_TLS_TICKET_LIFETIME_S 86400
#endif

/**
 * @brief Mutual TLS - require and verify a client certificate (mTLS).
 *
 * Default off. When set (requires DWS_ENABLE_TLS), the server can be given a
 * trust-anchor CA via DWS::tls_require_client_cert(): the TLS handshake
 * then demands a client certificate chaining to that CA
 * (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents
 * none or an untrusted one. The verified peer's subject DN is available to
 * handlers via DWS::tls_client_subject(). Strong transport-level client
 * authentication with no passwords.
 */
#ifndef DWS_ENABLE_MTLS
#define DWS_ENABLE_MTLS 0
#endif

/** @brief Maximum length of a verified mTLS peer subject DN string (incl. NUL). */
#ifndef DWS_MTLS_SUBJECT_MAX
#define DWS_MTLS_SUBJECT_MAX 128
#endif

/**
 * @brief SNMP agent (v1/v2c, + v3 USM when DWS_ENABLE_SNMP_V3) over lwIP UDP.
 *
 * Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER
 * codec itself is gated by this flag and is otherwise unit-tested standalone
 * (env:native_snmp).
 */
#ifndef DWS_ENABLE_SNMP
#define DWS_ENABLE_SNMP 0
#endif

/** @brief Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off. */
#ifndef DWS_ENABLE_SNMP_V3
#define DWS_ENABLE_SNMP_V3 0
#endif

/**
 * @brief Outbound SNMP notifications - traps and informs (requires DWS_ENABLE_SNMP).
 *
 * Default off. When set, src/services/snmp/dws_snmp_notify.h sends SNMPv2c (and, with
 * DWS_ENABLE_SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over
 * UDP - so the agent can push alerts instead of only answering polls. Reuses the
 * BER codec and the transport-layer UDP service; the PDU builder is host-testable.
 */
#ifndef DWS_ENABLE_SNMP_TRAP
#define DWS_ENABLE_SNMP_TRAP 0
#endif

/** @brief Maximum extra variable-bindings (beyond sysUpTime/snmpTrapOID) in one notification. */
#ifndef DWS_SNMP_TRAP_MAX_VARBINDS
#define DWS_SNMP_TRAP_MAX_VARBINDS 8
#endif

/** @brief Static datagram buffer for an outbound SNMP notification, bytes. */
#ifndef DWS_SNMP_TRAP_BUF_SIZE
#define DWS_SNMP_TRAP_BUF_SIZE 1024
#endif

/** @brief Maximum sub-identifiers (arcs) in an SNMP object identifier. */
#ifndef SNMP_MAX_OID_LEN
#define SNMP_MAX_OID_LEN 32
#endif

/**
 * @brief Maximum registered MIB objects (the agent's fixed OID table).
 *
 * Each entry holds its OID, a value descriptor, and optional get/set callbacks
 * (see src/services/snmp/dws_snmp_agent.h). The table lives in BSS; entries are
 * scanned linearly (small table) and need not be registered in OID order.
 */
#ifndef SNMP_MAX_MIB_ENTRIES
#define SNMP_MAX_MIB_ENTRIES 16
#endif

/**
 * @brief Maximum variable bindings the agent will emit in one response.
 *
 * Bounds GetBulk expansion (max-repetitions is clamped so the total response
 * varbind count never exceeds this) and the per-request decode scratch.
 */
#ifndef SNMP_MAX_VARBINDS
#define SNMP_MAX_VARBINDS 16
#endif

/**
 * @brief Static request/response datagram buffers for the SNMP UDP agent.
 *
 * Two buffers of this size live in BSS (one in, one out) - no heap. 484 is the
 * RFC 1157 minimum maximum message size; the default holds a one-frame UDP
 * payload so GetBulk walks fit without IP fragmentation.
 */
#ifndef SNMP_MSG_BUF_SIZE
#define SNMP_MSG_BUF_SIZE 1472
#endif

/** @brief Maximum SNMP community-string length (including null terminator). */
#ifndef SNMP_COMMUNITY_MAX
#define SNMP_COMMUNITY_MAX 32
#endif

/** @brief Default read-only community (overridable at runtime via dws_snmp_agent_init). Deployments
 *  SHOULD change this from the RFC-1157 well-known "public" for anything but a closed network. */
#ifndef DWS_SNMP_DEFAULT_RO_COMMUNITY
#define DWS_SNMP_DEFAULT_RO_COMMUNITY "public"
#endif

/** @brief Maximum SNMPv3 USM user-name length (including null terminator). */
#ifndef SNMP_V3_USER_MAX
#define SNMP_V3_USER_MAX 32
#endif

/** @brief Maximum SNMPv3 authoritative engine-ID length in bytes (RFC 3411 allows 5..32). */
#ifndef SNMP_V3_ENGINEID_MAX
#define SNMP_V3_ENGINEID_MAX 32
#endif

// ---------------------------------------------------------------------------
// CoAP server sizing constants  (DWS_ENABLE_COAP must be 1)
// ---------------------------------------------------------------------------

/**
 * @brief CoAP server (RFC 7252) over UDP/5683.
 *
 * A zero-heap Constrained Application Protocol endpoint: a fixed resource table
 * dispatched against the request's Uri-Path, with a pure host-testable message
 * codec (parse/build) and an ESP32 UDP binding via the transport-layer UDP
 * service. Default off; the codec is otherwise unit-tested standalone
 * (env:native_coap).
 */
#ifndef DWS_ENABLE_COAP
#define DWS_ENABLE_COAP 0
#endif

/**
 * @brief CoAP resource observation - RFC 7641 (requires DWS_ENABLE_COAP).
 *
 * Default off. When set, a client GET with the Observe option registers as an
 * observer of a resource; the application calls dws_coap_notify(path) to push the
 * resource's current representation to every observer (a CoAP notification from
 * the server port with an increasing Observe sequence). Observers are dropped on
 * a deregister GET, a client RST, or send failure.
 */
#ifndef DWS_ENABLE_COAP_OBSERVE
#define DWS_ENABLE_COAP_OBSERVE 0
#endif

/** @brief Maximum simultaneous CoAP observers (one slot per observed resource per client). */
#ifndef DWS_COAP_MAX_OBSERVERS
#define DWS_COAP_MAX_OBSERVERS 4
#endif

/**
 * @brief CoAP block-wise transfer - RFC 7959 (requires DWS_ENABLE_COAP).
 *
 * Default off. When set, the server understands the Block2 (descriptive,
 * responses) and Block1 (control, request uploads) options:
 *  - Block2: a representation larger than one block, or any GET that carries a
 *    Block2 option, is served one block at a time. A constrained client requests
 *    a small block size (SZX) and pages through with ascending block numbers; the
 *    server re-renders the (idempotent) resource and slices out the asked-for
 *    block, setting the More bit until the last.
 *  - Block1: a POST/PUT payload larger than one block is reassembled into a
 *    single BSS buffer. Each non-final block is acknowledged 2.31 Continue; the
 *    final block dispatches the handler with the whole reassembled payload.
 *
 * One block-wise transfer is reassembled at a time (deterministic, single
 * buffer); an out-of-order or oversized block yields 4.08 / 4.13. Size1/Size2
 * options and the /.well-known/core listing are out of scope.
 */
#ifndef DWS_ENABLE_COAP_BLOCK
#define DWS_ENABLE_COAP_BLOCK 0
#endif

/** @brief Largest block-size exponent (SZX) the server will use: block size = 2^(SZX+4) bytes, SZX 0..6 (16..1024). */
#ifndef DWS_COAP_BLOCK_SZX_MAX
#define DWS_COAP_BLOCK_SZX_MAX 6
#endif

/**
 * @brief Reassembly buffer for a block-wise (Block1) request upload, in bytes.
 *
 * One buffer of this size lives in BSS only when DWS_ENABLE_COAP_BLOCK is set.
 * It bounds the largest payload a chunked POST/PUT can deliver to a handler.
 */
#ifndef DWS_COAP_BLOCK1_MAX
#define DWS_COAP_BLOCK1_MAX 1024
#endif

/**
 * @brief Maximum registered CoAP resources (the server's fixed routing table).
 *
 * Each entry holds a path pointer, an allowed-methods bitmask, and a handler.
 * The table lives in BSS and is scanned linearly (small table).
 */
#ifndef DWS_COAP_MAX_RESOURCES
#define DWS_COAP_MAX_RESOURCES 8
#endif

/** @brief Maximum reconstructed Uri-Path length, including separators and the leading '/'. */
#ifndef DWS_COAP_MAX_PATH
#define DWS_COAP_MAX_PATH 64
#endif

/** @brief Maximum reconstructed Uri-Query length (segments joined by '&'). */
#ifndef DWS_COAP_MAX_QUERY
#define DWS_COAP_MAX_QUERY 64
#endif

/**
 * @brief Maximum CoAP request/response payload in bytes.
 *
 * Sizes the static scratch a handler writes its response body into and bounds
 * the request payload handed to it. One buffer of this size lives in BSS.
 */
#ifndef DWS_COAP_MAX_PAYLOAD
#define DWS_COAP_MAX_PAYLOAD 256
#endif

/**
 * @brief Static response-datagram buffer for the CoAP UDP server.
 *
 * One buffer of this size lives in BSS (the request is transport-owned). Must
 * hold a 4-byte header + token (<=8) + the Content-Format option + a 0xFF marker
 * + DWS_COAP_MAX_PAYLOAD bytes. When block-wise transfer is enabled it must
 * also hold one full block (2^(DWS_COAP_BLOCK_SZX_MAX+4) bytes) + option
 * overhead, so the default grows accordingly.
 */
#ifndef DWS_COAP_MSG_BUF_SIZE
#if DWS_ENABLE_COAP_BLOCK
#define DWS_COAP_MSG_BUF_SIZE 1152
#else
#define DWS_COAP_MSG_BUF_SIZE 512
#endif
#endif

/** @brief Default UDP port the CoAP observe transport notifies from (IANA well-known 5683). */
#ifndef DWS_COAP_OBSERVE_PORT
#define DWS_COAP_OBSERVE_PORT 5683
#endif

/**
 * @brief Bytes of the static BSS arena mbedTLS allocates from (DWS_ENABLE_TLS).
 *
 * All mbedTLS allocations (per-connection record buffers, handshake temporaries,
 * cert/key parsing) are served from this fixed arena via a custom allocator
 * installed with mbedtls_platform_set_calloc_free() - never the system heap. Must
 * cover the worst-case handshake peak for MAX_TLS_CONNS; if undersized the
 * handshake fails cleanly (no corruption). Measured peak for ONE ECDSA P-256
 * connection on Arduino-esp32 (16 KB IN + 16 KB OUT records) is ~41.5 KB, so the
 * default leaves a small margin. An RSA cert/larger chain needs more; query the
 * live peak via dws_tls_arena_peak(). NOTE: a second concurrent TLS connection
 * roughly doubles the record-buffer cost (~32 KB more), which overflows the
 * static DRAM budget - keep MAX_TLS_CONNS at 1 unless you shrink the IDF record
 * sizes (CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN, needs an ESP-IDF build).
 */
#ifndef DWS_TLS_ARENA_SIZE
#define DWS_TLS_ARENA_SIZE 49152
#endif

/**
 * @brief Place the TLS arena in external PSRAM instead of internal DRAM (ESP32).
 *
 * The internal static-DRAM ceiling (`dram0_0_seg`) is only ~122 KB, so a single
 * ~48 KB arena already uses a large slice and a second concurrent connection
 * (MAX_TLS_CONNS > 1) overflows it. On a board with PSRAM, set this to 1 to move
 * the arena to external RAM via `EXT_RAM_BSS_ATTR` / `EXT_RAM_ATTR`, freeing the
 * whole `DWS_TLS_ARENA_SIZE` back to internal DRAM so many connections fit.
 * Requires `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` (and PSRAM enabled) in the
 * ESP-IDF/PlatformIO config; without it the attribute is a no-op and the arena
 * stays in DRAM (safe fallback). No effect on the native host build.
 */
#ifndef DWS_TLS_ARENA_IN_PSRAM
#define DWS_TLS_ARENA_IN_PSRAM 0
#endif

/**
 * @brief Cap TLS records via the Maximum Fragment Length extension (RFC 6066).
 *
 * 0 (default) leaves the 16 KB TLS record ceiling. Set to 512, 1024, 2048, or 4096
 * to negotiate a smaller maximum record. On a mbedTLS build with variable-length
 * record buffers this shrinks the per-connection arena footprint (so more concurrent
 * connections fit); on a fixed-buffer build it still bounds the on-wire record size
 * (bandwidth / latency on a constrained link) and honors a client's MFL request.
 * Applied to both the server and the outbound client config. Needs an mbedTLS build
 * with `MBEDTLS_SSL_MAX_FRAGMENT_LENGTH` (else it is a no-op).
 */
#ifndef DWS_TLS_MAX_FRAG_LEN
#define DWS_TLS_MAX_FRAG_LEN 0
#endif

/**
 * @brief Acknowledge that a MAX_TLS_CONNS > 1 build has been sized to fit.
 *
 * The whole TLS arena is static `.bss` and the internal `dram0_0_seg` ceiling is only
 * ~122 KB, so a second concurrent connection's arena overflows it on a stock build.
 * A validation guard (bottom of this file) therefore rejects MAX_TLS_CONNS > 1 unless
 * you have taken one of the paths in docs/KNOWN_LIMITATIONS.md - move the arena to
 * PSRAM (`DWS_TLS_ARENA_IN_PSRAM`, which satisfies the guard on its own), shrink the
 * mbedTLS records in a custom ESP-IDF build, or reclaim internal DRAM - and then set
 * this to 1 to confirm the build was sized deliberately.
 */
#ifndef DWS_TLS_ACK_MULTI_CONN_DRAM
#define DWS_TLS_ACK_MULTI_CONN_DRAM 0
#endif

// ---------------------------------------------------------------------------
// Optional network services (ESP32-only thin wrappers; each default-off so it
// costs no code/RAM/flash unless explicitly enabled).
// ---------------------------------------------------------------------------

/** @brief mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS. */
#ifndef DWS_ENABLE_MDNS
#define DWS_ENABLE_MDNS 0
#endif

/** @brief SNTP wall-clock time sync via the ESP-IDF SNTP client. */
#ifndef DWS_ENABLE_NTP
#define DWS_ENABLE_NTP 0
#endif

/**
 * @brief NTP/SNTP time server (RFC 5905 / RFC 4330 server mode) on UDP/123 (services/dws_ntp_server).
 *
 * Turns the device into a local time source: it answers client NTP requests from its own
 * clock (`dws_time_now()` + the `dws_millis()` sub-second fraction), so an offline or
 * air-gapped LAN can keep its devices in sync without reaching the public NTP pool. The
 * 48-byte response codec is pure and host-tested; the wire binding is the transport UDP
 * service. Get the device's own time first (e.g. DWS_ENABLE_NTP upstream, an RTC, or GPS
 * via a time source) - when it has none, the server stays silent rather than serve bad time.
 */
#ifndef DWS_ENABLE_NTP_SERVER
#define DWS_ENABLE_NTP_SERVER 0
#endif

/** @brief Stratum the NTP server advertises (distance from a reference clock; 1-15). */
#ifndef DWS_NTP_SERVER_STRATUM
#define DWS_NTP_SERVER_STRATUM 3
#endif

/**
 * @brief Authoritative DNS server (services/dns_server) on UDP/53.
 *
 * Default off. Resolves a small fixed table of `name -> IPv4` A records you register with
 * dws_dns_server_add(), so devices on an offline / air-gapped LAN can use names instead of raw
 * IPs (a companion to the NTP server for offline infrastructure). Answers A/IN queries from
 * the table, returns NXDOMAIN for unknown names, and ignores other query types. The response
 * builder is pure and host-tested; the wire binding is the transport UDP service. This is a
 * general resolver, distinct from the provisioning captive-portal DNS (which answers every
 * query with the softAP IP) - do not enable both (they both bind :53).
 */
#ifndef DWS_ENABLE_DNS_SERVER
#define DWS_ENABLE_DNS_SERVER 0
#endif

/** @brief Max A records in the DNS server's fixed table. */
#ifndef DWS_DNS_SERVER_MAX_RECORDS
#define DWS_DNS_SERVER_MAX_RECORDS 8
#endif

/** @brief TTL (seconds) the DNS server puts on its answers. */
#ifndef DWS_DNS_SERVER_TTL
#define DWS_DNS_SERVER_TTL 60
#endif

/** @brief Max length of a queried/stored DNS name (bytes, incl NUL). */
#ifndef DWS_DNS_NAME_MAX
#define DWS_DNS_NAME_MAX 128
#endif

/**
 * @brief Auto-inject a `Date` response header (RFC 7231 7.1.1.2) when a wall-clock
 *        time is available.
 *
 * Default off: a clock-less device must not emit a wrong `Date`, and most embedded
 * responses do not need one, so it stays off the hot path. When set, every dynamic
 * response carries `Date: <IMF-fixdate>` - but only once a real time exists; before a
 * source has valid time it is silently omitted (still correct for a clock-less boot).
 *
 * The time is taken from the multi-source registry (any enabled NTP / GPS / RTC / ...
 * by priority) when DWS_ENABLE_TIME_SOURCE is set - register your sources with
 * dws_time_source_add() (dws_rtc_time_source, dws_ntp_time_source, ...). Otherwise it comes
 * straight from NTP (dws_ntp_http_date). Needs at least one such time source to emit.
 */
#ifndef DWS_HTTP_EMIT_DATE
#define DWS_HTTP_EMIT_DATE 0
#endif

/**
 * @brief Multi-source time fallback (NTP / RTC / GPS / ... by priority).
 *
 * When set, src/services/time_source/time_source.h provides a small registry of
 * user-defined time sources, each a callback returning Unix epoch seconds (0 when
 * that source has no valid time). dws_time_now() queries them in priority order
 * (lowest value first) and returns the first valid result, so the device falls
 * back automatically when its preferred clock is unavailable. Pure and zero-heap
 * (a fixed source table); host-testable. Default off.
 */
#ifndef DWS_ENABLE_TIME_SOURCE
#define DWS_ENABLE_TIME_SOURCE 0
#endif

/** @brief Maximum registered time sources (DWS_ENABLE_TIME_SOURCE). */
#ifndef DWS_TIME_SOURCE_MAX
#define DWS_TIME_SOURCE_MAX 4
#endif

/**
 * @brief Shared I2C bus pins for the sensor / peripheral drivers (RTC, SHT3x, MPR121, ADS1115,
 * INA219, PCA9685). All of them share one bus via dws_i2c_begin() (services/i2c.h), so
 * this is the single place to move it. The default -1 uses the platform's default pins (GPIO 21
 * SDA / 22 SCL on the classic ESP32). Set both to free GPIOs when those pins are taken - most
 * importantly a **wired-Ethernet PHY**: the LAN8720 RMII uses GPIO 21 (TX_EN) and GPIO 22
 * (TXD1) on the classic ESP32 (WROOM/WROVER) and the ESP32-P4 (which have the RMII EMAC), so
 * with that Ethernet on, move the I2C bus off them (e.g. 32 / 33). The ESP32-S3/C3 have no RMII
 * MAC and use an SPI Ethernet (W5500) instead - relocate the bus off whatever SPI pins that
 * uses. UART peripherals (LD2410) take their RX/TX pins at dws_ld2410_begin(), so remap those too.
 */
#ifndef DWS_I2C_SDA_PIN
#define DWS_I2C_SDA_PIN -1
#endif
#ifndef DWS_I2C_SCL_PIN
#define DWS_I2C_SCL_PIN -1
#endif

/**
 * @brief I2C real-time-clock driver (DS1307 / DS3231) - a battery-backed time source.
 *
 * Default off. services/rtc reads and sets a DS1307/DS3231 RTC over I2C (Wire), so the device
 * keeps accurate wall-clock time across reboots and power loss with no network - the ideal
 * fallback below GPS and above upstream NTP in a time-source chain (feeds `dws_time_now()`
 * and the NTP server). The BCD<->epoch conversion (7 time registers, 12/24-hour, leap years,
 * range validation) is pure and host-tested; only the register read/write touches I2C.
 */
#ifndef DWS_ENABLE_RTC
#define DWS_ENABLE_RTC 0
#endif

/** @brief I2C address of the RTC (DS1307/DS3231 are fixed at 0x68). */
#ifndef DWS_RTC_I2C_ADDR
#define DWS_RTC_I2C_ADDR 0x68
#endif

/**
 * @brief HLK-LD2410 24 GHz mmWave presence / motion radar (UART).
 *
 * Default off. services/ld2410 syncs to the LD2410's framed serial output (256000 baud) and
 * decodes the target report - presence state (none / moving / stationary / both), the moving
 * and stationary target distance (cm) and energy (0-100), and, in engineering mode, the
 * per-gate energies - plus encodes the config commands (enter / exit config, enable / disable
 * engineering mode, restart). The frame sync + decode is pure and host-tested; only the UART
 * read/write touches hardware. A cheap solder-and-test breakout: wave a hand, watch presence.
 */
#ifndef DWS_ENABLE_LD2410
#define DWS_ENABLE_LD2410 0
#endif

/** @brief LD2410 UART baud rate (the module's fixed factory default is 256000). */
#ifndef DWS_LD2410_BAUD
#define DWS_LD2410_BAUD 256000
#endif

/**
 * @brief DFRobot SEN0192 10.525 GHz microwave Doppler motion sensor (single digital OUT line).
 *
 * Default off. services/sen0192 tracks the module's OUT line as a debounced presence signal: it asserts
 * presence on an active sample and holds it for DWS_SEN0192_HOLD_MS after the last active sample, so
 * brief gaps between Doppler returns don't make presence flap. The presence state machine is pure and
 * host-tested; only the GPIO read touches hardware. Unlike a PIR it senses motion through thin non-metal
 * enclosures and is unaffected by ambient light / temperature.
 */
#ifndef DWS_ENABLE_SEN0192
#define DWS_ENABLE_SEN0192 0
#endif

/** @brief GPIO the SEN0192 OUT line is wired to. */
#ifndef DWS_SEN0192_PIN
#define DWS_SEN0192_PIN 4
#endif

/** @brief Presence is held this many ms after the last active (motion) sample before it clears. */
#ifndef DWS_SEN0192_HOLD_MS
#define DWS_SEN0192_HOLD_MS 2000
#endif

/** @brief SEN0192 OUT polarity: 1 = the OUT line reads HIGH on motion, 0 = active-LOW. */
#ifndef DWS_SEN0192_ACTIVE_HIGH
#define DWS_SEN0192_ACTIVE_HIGH 1
#endif

/**
 * @brief NXP MPR121 12-channel capacitive-touch controller (I2C).
 *
 * Default off. services/mpr121 decodes the touch-status word (12 electrode bits + proximity +
 * over-current) and the 10-bit filtered / baseline per-electrode data, and builds the register
 * init sequence (soft reset, the NXP filter/AFE defaults, per-electrode touch/release
 * thresholds, and the electrode-configuration start). The decode + init-sequence builder are
 * pure and host-tested; only the register read/write touches I2C. A cheap solder-and-test
 * breakout for touch buttons / sliders: wire it up, touch a pad, watch the bit set.
 */
#ifndef DWS_ENABLE_MPR121
#define DWS_ENABLE_MPR121 0
#endif

/** @brief I2C address of the MPR121 (0x5A default; 0x5B/0x5C/0x5D via the ADDR pin). */
#ifndef DWS_MPR121_I2C_ADDR
#define DWS_MPR121_I2C_ADDR 0x5A
#endif

/** @brief MPR121 per-electrode touch threshold (delta counts from baseline; NXP AN3944 suggests ~4..12).
 *         Higher = less sensitive. Keep the release threshold below it for hysteresis. */
#ifndef DWS_MPR121_TOUCH_THRESHOLD
#define DWS_MPR121_TOUCH_THRESHOLD 12
#endif

/** @brief MPR121 per-electrode release threshold (delta counts; should be below the touch threshold). */
#ifndef DWS_MPR121_RELEASE_THRESHOLD
#define DWS_MPR121_RELEASE_THRESHOLD 6
#endif

/**
 * @brief Sensirion SHT3x temperature / humidity sensor (I2C).
 *
 * Default off. services/sht3x issues the single-shot measurement command, checks the CRC-8 on
 * each returned word (polynomial 0x31, init 0xFF - the Sensirion check value 0xBEEF -> 0x92),
 * and converts the raw 16-bit ticks to temperature and relative humidity in integer milli-units
 * (no float printf needed). The CRC + conversion are pure and host-tested; only the command
 * write / data read touches I2C. A cheap solder-and-test breakout (GY-SHT31 etc.) for
 * environmental telemetry: read it, bridge it onto the network.
 */
#ifndef DWS_ENABLE_SHT3X
#define DWS_ENABLE_SHT3X 0
#endif

/** @brief I2C address of the SHT3x (0x44 with ADDR low; 0x45 with ADDR high). */
#ifndef DWS_SHT3X_I2C_ADDR
#define DWS_SHT3X_I2C_ADDR 0x44
#endif

/**
 * @brief NXP PCA9685 16-channel 12-bit PWM / servo driver (I2C).
 *
 * Default off. services/pca9685 computes the PRESCALE value for a PWM frequency from the 25 MHz
 * oscillator, the per-channel register address, the 12-bit ON/OFF pulse counts, and a servo
 * pulse-width (microseconds) -> count conversion; it also emits the 5-byte channel PWM write.
 * The prescale / count math + the register encoder are pure and host-tested; only the register
 * writes touch I2C. A cheap solder-and-test breakout for driving up to 16 servos or LEDs.
 */
#ifndef DWS_ENABLE_PCA9685
#define DWS_ENABLE_PCA9685 0
#endif

/** @brief I2C address of the PCA9685 (0x40 default; the six address pins select 0x40..0x7F). */
#ifndef DWS_PCA9685_I2C_ADDR
#define DWS_PCA9685_I2C_ADDR 0x40
#endif

/** @brief Default PWM output frequency in Hz (50 Hz suits hobby servos). */
#ifndef DWS_PCA9685_FREQ
#define DWS_PCA9685_FREQ 50
#endif

/**
 * @brief TI ADS1115 16-bit ADC (I2C) - a precise external analog input.
 *
 * Default off. services/ads1115 builds the 16-bit config register (OS start, single-ended
 * channel MUX, programmable gain, single-shot mode, data rate, comparator disabled) for a
 * single-shot reading, and converts the signed 16-bit result to microvolts for the selected
 * gain's full-scale range. The config encoder + conversion are pure and host-tested; only the
 * register write / conversion read touches I2C. A cheap solder-and-test breakout for reading
 * batteries, potentiometers, and analog sensors with far more resolution than the ESP32 ADC.
 */
#ifndef DWS_ENABLE_ADS1115
#define DWS_ENABLE_ADS1115 0
#endif

/** @brief I2C address of the ADS1115 (0x48 with ADDR to GND; 0x49/0x4A/0x4B for VDD/SDA/SCL). */
#ifndef DWS_ADS1115_I2C_ADDR
#define DWS_ADS1115_I2C_ADDR 0x48
#endif

/** @brief Default ADS1115 PGA gain code (ADS1115_GAIN_*): 0=+/-6.144V, 1=+/-4.096V, 2=+/-2.048V (default),
 *         3=+/-1.024V, 4=+/-0.512V, 5=+/-0.256V. Also the fallback when a read passes an invalid gain. */
#ifndef DWS_ADS1115_GAIN
#define DWS_ADS1115_GAIN 2 // ADS1115_GAIN_2 (+/- 2.048 V)
#endif

/** @brief Default ADS1115 data-rate code (ADS1115_DR_*): 0=8, 1=16, 2=32, 3=64, 4=128 (default), 5=250,
 *         6=475, 7=860 SPS. The single-shot read waits the matching conversion time. */
#ifndef DWS_ADS1115_DR
#define DWS_ADS1115_DR 4 // ADS1115_DR_128 (128 SPS)
#endif

/** @brief ADS1115 input mode: 0 = single-ended (AINx vs GND), 1 = differential. In differential mode the
 *         channel selects the pair: 0=AIN0-AIN1, 1=AIN0-AIN3, 2=AIN1-AIN3, 3=AIN2-AIN3. */
#ifndef DWS_ADS1115_DIFFERENTIAL
#define DWS_ADS1115_DIFFERENTIAL 0
#endif

/**
 * @brief TI INA219 high-side current / power monitor (I2C).
 *
 * Default off. services/ina219 decodes the bus-voltage register (LSB 4 mV) and the shunt-voltage
 * register (LSB 10 uV), computes the calibration register from the shunt resistance and the
 * chosen current LSB, and scales the raw current / power registers to microamps / microwatts.
 * The decode + calibration + scaling math are pure and host-tested; only the register read /
 * write touches I2C. A cheap solder-and-test breakout for measuring how much current and power a
 * circuit actually draws.
 */
#ifndef DWS_ENABLE_INA219
#define DWS_ENABLE_INA219 0
#endif

/** @brief I2C address of the INA219 (0x40 default; the A0/A1 pins select 0x40..0x4F). */
#ifndef DWS_INA219_I2C_ADDR
#define DWS_INA219_I2C_ADDR 0x40
#endif

/** @brief Default INA219 current LSB in microamps per bit (calibration input). The fallback when
 *         dws_ina219_begin() is passed 0. 100 uA/bit with a 100 mohm shunt -> a 2 A full-scale range. */
#ifndef DWS_INA219_CURRENT_LSB_UA
#define DWS_INA219_CURRENT_LSB_UA 100
#endif

/** @brief Default INA219 shunt resistance in milliohms (calibration input). The fallback when
 *         dws_ina219_begin() is passed 0. 100 mohm is the common breakout value. */
#ifndef DWS_INA219_SHUNT_MOHM
#define DWS_INA219_SHUNT_MOHM 100
#endif

/**
 * @brief Typed NVS configuration store (WiFi creds, IP config, ... as blobs).
 *
 * When set, src/services/config_store/config_store.h provides a typed key/value
 * API (string / u32 / blob) that routes core settings into the ESP32's native
 * NVS partition (via `Preferences`) instead of a JSON file on the filesystem -
 * which survives FS corruption and is the corruption-resistant home for
 * credentials. On host builds it is backed by a fixed in-memory table so the
 * typed contract is unit-testable. Default off.
 */
#ifndef DWS_ENABLE_CONFIG_STORE
#define DWS_ENABLE_CONFIG_STORE 0
#endif

/** @brief Max key/value entries in the host (test) config backend. */
#ifndef DWS_CONFIG_MAX_ENTRIES
#define DWS_CONFIG_MAX_ENTRIES 16
#endif

/** @brief Max key length incl. null (NVS caps keys at 15 chars). */
#ifndef DWS_CONFIG_KEY_MAX
#define DWS_CONFIG_KEY_MAX 16
#endif

/** @brief Max value bytes per entry in the host (test) config backend. */
#ifndef DWS_CONFIG_VAL_MAX
#define DWS_CONFIG_VAL_MAX 64
#endif

/**
 * @brief Stable device UUID derived from the chip MAC (RFC 4122 v5).
 *
 * When set, src/services/device_id/device_id.h derives a deterministic v5 UUID
 * from a MAC (via the library's SHA-1) - a storage-free, stable identity for
 * mDNS hostnames, MQTT client IDs, etc. The MAC->UUID core is host-testable;
 * dws_device_uuid() reads the ESP32 factory MAC. Default off.
 */
#ifndef DWS_ENABLE_DEVICE_ID
#define DWS_ENABLE_DEVICE_ID 0
#endif

/**
 * @brief Telemetry math helpers (moving-window stats, rate-of-change, totalizer).
 *
 * Default off. When set, src/services/telemetry/telemetry.h provides zero-heap
 * pure-computation helpers over caller-supplied storage: a moving-window stats
 * accumulator (mean / variance / stddev / min / max), a derivative / rate-of-
 * change tracker, and a trapezoidal run-time totalizer. No ESP32 dependency, so
 * the whole cluster is host-testable; it feeds dashboards, alert triggers, and
 * odometer-style counters.
 */
#ifndef DWS_ENABLE_TELEMETRY
#define DWS_ENABLE_TELEMETRY 0
#endif

/**
 * @brief Real-time SVG dashboard (DWS_ENABLE_DASHBOARD; requires DWS_ENABLE_SSE).
 *
 * Default off. Serves a self-contained, hand-rolled SVG dashboard page whose
 * widgets are declared in a fixed compile-time DWSWidget table (zero-heap,
 * deterministic). The page fetches the widget layout as JSON and subscribes to an
 * SSE stream of live values; dws_dashboard_set() + dws_dashboard_publish()
 * push the current readings. The widget-table -> JSON serializers are
 * host-testable; WebSocket controls are a follow-up.
 */
#ifndef DWS_ENABLE_DASHBOARD
#define DWS_ENABLE_DASHBOARD 0
#endif

/**
 * @brief Embed the theme stylesheet library as runtime-selectable blobs (default off).
 *
 * Off by default: build-time theme injection (`<!--#theme NAME-->`) costs nothing extra, but
 * embedding the whole library for runtime switching links every theme's CSS into flash (~1 KB each).
 * When set, application/binary_asset_blobs.{h,cpp} exposes `dws_theme_css(name)` + the registry
 * `DWS_THEME_BLOBS`, so a route (e.g. `/themes/<name>.css`) or a picker can switch themes live.
 * Regenerate with `src/web/wizard/gen_theme_blobs.py` after adding a theme.
 */
#ifndef DWS_ENABLE_THEMES
#define DWS_ENABLE_THEMES 0
#endif

/**
 * @brief Include the trademark-named themes in the embedded set (default on / open-source).
 *
 * A few themes are named after a company or product (Darcula, Windows XP, Discord, Spotify, ...). The
 * palette is just colors, but a commercial product should not ship the branded name, so a commercial
 * build sets this to 0 to drop those blobs from the registry (the list is `RESTRICTED` in
 * `src/web/wizard/gen_themes.py`). The open-source (AGPL) build keeps them.
 */
#ifndef DWS_THEMES_INCLUDE_TRADEMARKED
#define DWS_THEMES_INCLUDE_TRADEMARKED 1
#endif

/** @brief Maximum widgets in the dashboard table (BSS value array). */
#ifndef DWS_DASHBOARD_MAX_WIDGETS
#define DWS_DASHBOARD_MAX_WIDGETS 16
#endif

/** @brief Stack buffer for the dashboard layout / values JSON (bytes). */
#ifndef DWS_DASHBOARD_JSON_BUF
#define DWS_DASHBOARD_JSON_BUF 1024
#endif

/**
 * @brief Opt-in flash partition-map monitor endpoint (DWS_ENABLE_PARTITION_MONITOR).
 *
 * Default off. When set, services/partition_monitor reports the device's flash
 * partition table (label, kind, type / subtype, offset, size, and which app slot
 * is running) as JSON, for diagnostics and OTA dashboards. The partition walk uses
 * esp_partition / esp_ota_ops; the JSON serializer and the kind classifier are
 * pure and host-testable.
 */
#ifndef DWS_ENABLE_PARTITION_MONITOR
#define DWS_ENABLE_PARTITION_MONITOR 0
#endif

/** @brief Maximum partitions the monitor reports (BSS table). */
#ifndef DWS_PARTITION_MAX
#define DWS_PARTITION_MAX 16
#endif

/** @brief Stack buffer for the partition-map JSON (bytes). */
#ifndef DWS_PARTITION_JSON_BUF
#define DWS_PARTITION_JSON_BUF 1024
#endif

/**
 * @brief Opt-in browser GPIO pin-mapper / diagnostics endpoint (DWS_ENABLE_GPIO_MAP).
 *
 * Default off. When set, services/gpio_map serves a compile-time table of GPIO
 * pins (number, label, direction, live level) as JSON for a browser diag panel,
 * and accepts a control POST (`pin`, `level`) to drive an output. The live read /
 * write uses the Arduino digital API on ESP32; the JSON serializer and the control
 * parser are pure and host-testable.
 */
#ifndef DWS_ENABLE_GPIO_MAP
#define DWS_ENABLE_GPIO_MAP 0
#endif

/** @brief Maximum GPIO pins the mapper reports (BSS table). */
#ifndef DWS_GPIO_MAX
#define DWS_GPIO_MAX 40
#endif

/** @brief Stack buffer for the GPIO-map JSON (bytes). */
#ifndef DWS_GPIO_JSON_BUF
#define DWS_GPIO_JSON_BUF 1024
#endif

/**
 * @brief Opt-in fire-and-forget UDP telemetry cast (DWS_ENABLE_UDP_TELEMETRY).
 *
 * Default off. When set, services/udp_telemetry casts metric lines (InfluxDB line
 * protocol: `measurement field=val,field2=val2`) to a configured collector over
 * UDP via dws_udp_sendto - zero-heap, fire-and-forget (no ACK, no retry), ideal
 * for shipping device metrics to Telegraf/InfluxDB/a log sink. The line builder is
 * pure and host-tested; only the send touches the network.
 */
#ifndef DWS_ENABLE_UDP_TELEMETRY
#define DWS_ENABLE_UDP_TELEMETRY 0
#endif

/** @brief Stack buffer for one telemetry line (bytes). */
#ifndef DWS_UDP_TELEMETRY_BUF
#define DWS_UDP_TELEMETRY_BUF 256
#endif

/**
 * @brief Opt-in StatsD metrics client (DWS_ENABLE_STATSD).
 *
 * Default off. When set, services/statsd pushes metrics in the StatsD wire format
 * (`name:value|type`, e.g. `api.hits:1|c`) over UDP to a StatsD-speaking collector -
 * Graphite/StatsD, Telegraf, Datadog, InfluxDB, etc. Counters, gauges (absolute + delta),
 * timings, and sets, with optional sample-rate (`|@0.1`) and DogStatsD tags (`|#env:prod`).
 * This is the push counterpart to the pull-based Prometheus `/metrics`. The line formatter
 * is pure and host-tested; only the send (dws_udp_sendto) touches the network. Zero heap.
 */
#ifndef DWS_ENABLE_STATSD
#define DWS_ENABLE_STATSD 0
#endif

/** @brief Default StatsD collector UDP port (StatsD/Graphite standard). */
#ifndef DWS_STATSD_PORT
#define DWS_STATSD_PORT 8125
#endif

/** @brief Stack buffer for one StatsD line (bytes; caps metric name + value + tags). */
#ifndef DWS_STATSD_LINE_MAX
#define DWS_STATSD_LINE_MAX 256
#endif

/**
 * @brief Opt-in runtime heap/stack guardrails (DWS_ENABLE_GUARDRAILS).
 *
 * Default off. When set, services/guardrails samples free heap, the heap low-water
 * mark, the largest free block (fragmentation), and the calling task's remaining
 * stack, and fires a callback when any crosses its threshold - a proactive
 * fail-safe hook beyond the passive numbers in /metrics. The threshold evaluator
 * and the JSON serializer are pure and host-tested; the sample reads esp_* / the
 * FreeRTOS stack high-water on ESP32.
 */
#ifndef DWS_ENABLE_GUARDRAILS
#define DWS_ENABLE_GUARDRAILS 0
#endif

/** @brief Free-heap floor (bytes); below this trips the heap guardrail. */
#ifndef DWS_GUARDRAIL_HEAP_MIN
#define DWS_GUARDRAIL_HEAP_MIN 8192
#endif

/** @brief Largest-free-block floor (bytes); below this trips the fragmentation guardrail. */
#ifndef DWS_GUARDRAIL_FRAG_MIN_BLOCK
#define DWS_GUARDRAIL_FRAG_MIN_BLOCK 4096
#endif

/** @brief Task remaining-stack floor (bytes); below this trips the stack guardrail. */
#ifndef DWS_GUARDRAIL_STACK_MIN
#define DWS_GUARDRAIL_STACK_MIN 512
#endif

/**
 * @brief Opt-in software watchdog: deadlock detection + fail-safe safe-state (DWS_ENABLE_FAILSAFE).
 *
 * When set, services/failsafe provides a fixed registry of "lifelines" (a task / worker / control loop
 * that must check in within its deadline). dws_failsafe_check() detects one that stopped feeding (a
 * hang / deadlock) and fires a breach callback once per episode so the app can enter a known-safe
 * state. App-defined and per-lifeline, on top of the hardware task watchdog. Pure core, zero heap.
 * Default off.
 */
#ifndef DWS_ENABLE_FAILSAFE
#define DWS_ENABLE_FAILSAFE 0
#endif

/** @brief Max monitored lifelines in the fail-safe registry (static, zero-heap). */
#ifndef DWS_FAILSAFE_MAX_LIFELINES
#define DWS_FAILSAFE_MAX_LIFELINES 8
#endif

/**
 * @brief Opt-in dynamic sleep-cycle scheduler (DWS_ENABLE_SLEEP_SCHED).
 *
 * When set, services/sleep_sched provides dws_sleep_next(): from the time since the last activity it
 * returns how long a low-power device should sleep (0 = stay awake), ramping the window from a floor up
 * to a ceiling the longer the idle streak runs. Pure decision core (the app applies the window via
 * light / modem / deep sleep). Complements services/radio_power. Default off.
 */
#ifndef DWS_ENABLE_SLEEP_SCHED
#define DWS_ENABLE_SLEEP_SCHED 0
#endif

/**
 * @brief Opt-in flash wear-leveling slot selector (DWS_ENABLE_WEARLEVEL).
 *
 * When set, services/wearlevel provides dws_wearlevel_pick(): given per-slot write counts it returns
 * the least-worn slot to write next, so repeated flash/NVS writes spread evenly and the region ages
 * together instead of burning out one block. Pure core (the app owns the slots + persisted counts).
 * Default off.
 */
#ifndef DWS_ENABLE_WEARLEVEL
#define DWS_ENABLE_WEARLEVEL 0
#endif

/**
 * @brief Opt-in network adaptation decisions (DWS_ENABLE_NETADAPT).
 *
 * When set, services/netadapt provides two pure decisions: dws_netadapt_window() sizes the TCP
 * receive window from the free heap (bigger when RAM is plentiful, shrinking when tight), and
 * dws_netadapt_dhcp_fallback() decides when to give up on DHCP and use a static IP. The app applies
 * the results (lwIP window / netif config). Default off.
 */
#ifndef DWS_ENABLE_NETADAPT
#define DWS_ENABLE_NETADAPT 0
#endif

/**
 * @brief Opt-in DShot ESC throttle protocol codec (DWS_ENABLE_DSHOT).
 *
 * When set, services/dshot provides dws_dshot_encode() / _decode(): the 16-bit DShot frame
 * (11-bit throttle/command + telemetry bit + 4-bit CRC), the bidirectional/extended inverted-CRC
 * variant, and the per-rate bit timing for an RMT driver. Pure codec (the app clocks it out via RMT).
 * Default off.
 */
#ifndef DWS_ENABLE_DSHOT
#define DWS_ENABLE_DSHOT 0
#endif

/**
 * @brief Opt-in HART / HART-IP process-instrument protocol codec (DWS_ENABLE_HART).
 *
 * When set, services/hart provides the HART command-frame codec (build/parse with the longitudinal XOR
 * checksum, short + long addressing) and the 8-octet HART-IP message header, so a device speaks HART
 * over UDP/TCP 5094 (front-end-free) or, with a HART FSK modem, over the 4-20 mA loop. Pure, host-tested.
 * Default off.
 */
#ifndef DWS_ENABLE_HART
#define DWS_ENABLE_HART 0
#endif

/**
 * @brief Opt-in Network Time Security (NTS, RFC 8915) wire codec (DWS_ENABLE_NTS).
 *
 * When set, services/nts provides the NTS-KE record codec (build/parse the TLV records - next protocol,
 * AEAD, cookies, server/port) and the NTS NTP extension-field framing (Unique Identifier, Cookie,
 * Authenticator). Pure framing (the AES-SIV-CMAC-256 AEAD + TLS-exporter key derivation are the crypto
 * integration on top). Default off.
 */
#ifndef DWS_ENABLE_NTS
#define DWS_ENABLE_NTS 0
#endif

/**
 * @brief Opt-in DDS / RTPS wire-protocol codec (DWS_ENABLE_DDS).
 *
 * When set, services/dds provides the RTPS (DDSI-RTPS) message + submessage framing: the 20-octet
 * header (magic / version / vendor / guidPrefix) and the typed submessages (INFO_TS, DATA, HEARTBEAT,
 * ACKNACK, ...) with the endianness flag, built by dws_rtps_header / _submessage and walked by
 * dws_rtps_parse. Pure framing (CDR payloads + SPDP/SEDP discovery layer on top). Default off.
 */
#ifndef DWS_ENABLE_DDS
#define DWS_ENABLE_DDS 0
#endif

/**
 * @brief Opt-in XMPP (RFC 6120) stanza codec (DWS_ENABLE_XMPP).
 *
 * When set, services/xmpp builds correctly XML-escaped `<stream:stream>` / `<message>` / `<presence>` /
 * `<iq>` stanzas into a caller buffer and reads the stanza element name + an attribute value out of a
 * received stanza, so a device is an IoT XMPP client. Pure text framing (TLS/SASL ride the client TLS
 * path; the IoT XEPs layer inside `<iq>`). Default off.
 */
#ifndef DWS_ENABLE_XMPP
#define DWS_ENABLE_XMPP 0
#endif

/**
 * @brief Opt-in raw Layer-2 Ethernet frame codec (DWS_ENABLE_RAWL2).
 *
 * When set, services/rawl2 builds/parses Ethernet II + 802.1Q VLAN frames (no FCS - the MAC appends it;
 * dws_eth_fcs is provided for the cases that need it), so the app can inject/receive arbitrary L2
 * frames via esp_eth_transmit / esp_wifi_80211_tx - the basis for the raw-L2 industrial protocols
 * (PROFINET DCP, GOOSE, POWERLINK). Pure codec, host-tested. Default off.
 */
#ifndef DWS_ENABLE_RAWL2
#define DWS_ENABLE_RAWL2 0
#endif

/**
 * @brief Opt-in single-page-app micro-routing decision (DWS_ENABLE_SPA_ROUTER).
 *
 * When set, services/spa_router provides dws_spa_route(): given a request path it returns whether to
 * serve a real asset file, serve the SPA shell (index.html) for a client-side route, or pass through to
 * the app's handlers under an API prefix - so a single-page UI's client routing works. Pure decision
 * core (the caller wires the result into serve_static / the router). Default off.
 */
#ifndef DWS_ENABLE_SPA_ROUTER
#define DWS_ENABLE_SPA_ROUTER 0
#endif

/**
 * @brief Opt-in IEC 61850 GOOSE publisher codec (DWS_ENABLE_GOOSE).
 *
 * When set, services/goose builds the BER-encoded IECGoosePdu (gocbRef / timeAllowedToLive / datSet /
 * goID / t / stNum / sqNum / simulation / confRev / ndsCom / numDatSetEntries / allData) and wraps it in
 * the 8-octet GOOSE header + Ethernet frame (ethertype 0x88B8) for the fast raw-L2 substation-event
 * publish. Pure codec (allData is a caller-encoded BER blob; the raw-L2 transmit is the device step).
 * Default off.
 */
#ifndef DWS_ENABLE_GOOSE
#define DWS_ENABLE_GOOSE 0
#endif

/**
 * @brief Opt-in MTConnect agent response codec (DWS_ENABLE_MTCONNECT).
 *
 * When set, services/mtconnect builds the MTConnectStreams (current/sample) and MTConnectError XML
 * response documents (ANSI/MTC1.4) into a caller buffer - header with instanceId + nextSequence, then
 * per-DataItem Samples/Events/Condition observations - so the web server is an MTConnect agent over the
 * existing HTTP stack. Pure text framing (values XML-escaped). Default off.
 */
#ifndef DWS_ENABLE_MTCONNECT
#define DWS_ENABLE_MTCONNECT 0
#endif

/**
 * @brief MTConnect rolling sample buffer sizing (DWS_ENABLE_MTCONNECT).
 *
 * The agent retains the most recent ::DWS_MTC_SAMPLE_BUFFER observations in a fixed ring so a
 * subscriber can replay them with the `sample` from/count long-poll cursor (MTC1.4 §6.7): a request
 * asks for observations starting at a sequence number, and the response header reports firstSequence /
 * lastSequence / nextSequence so the client knows what it received and where to resume. Each retained
 * observation stores its type / dataItemId / timestamp / value in fixed char fields; when the ring is
 * full the oldest is evicted and firstSequence advances. Zero-heap, compile-time sized; the buffer costs
 * ~DWS_MTC_SAMPLE_BUFFER * (48 + the four string caps) bytes only where a DWSMtcSampleBuffer is used.
 */
#ifndef DWS_MTC_SAMPLE_BUFFER
#define DWS_MTC_SAMPLE_BUFFER 32 // observations retained for `sample` replay
#endif
#ifndef DWS_MTC_STR_MAX
#define DWS_MTC_STR_MAX 24 // max stored type / dataItemId length (excl NUL)
#endif
#ifndef DWS_MTC_TS_MAX
#define DWS_MTC_TS_MAX 32 // max stored ISO-8601 timestamp length (excl NUL)
#endif
#ifndef DWS_MTC_VAL_MAX
#define DWS_MTC_VAL_MAX 32 // max stored observation value length (excl NUL)
#endif

/**
 * @brief Opt-in write-ahead store for atomic buffer-to-flash storage (DWS_ENABLE_WAL).
 *
 * services/wal is a power-loss-safe write-ahead log over any fs::FS backend (SD card, LittleFS): records
 * are CRC32-framed, a checkpoint is atomic via an A/B superblock, and a recovery scan on mount replays
 * past the last checkpoint and stops at the first bad CRC (the torn tail), bounding the loss window. Sized
 * from the measured SD envelope (docs/FEATURE_PERFORMANCE.md): append sequentially in ~32 KiB pages,
 * checkpoint every ~128-256 KiB (never scatter small durable writes). The substrate for on-device data
 * stores (dbm / sqlite / nosql). Zero heap. Default off.
 */
#ifndef DWS_ENABLE_WAL
#define DWS_ENABLE_WAL 0
#endif
#ifndef DWS_WAL_PAGE_SIZE
#define DWS_WAL_PAGE_SIZE 32768 // sequential write unit (the measured durable-throughput knee)
#endif
#ifndef DWS_WAL_MAX_RECORD
#define DWS_WAL_MAX_RECORD 4096 // largest single record payload
#endif

/**
 * @brief Opt-in dbm: a log-structured hash key-value store on the WAL (DWS_ENABLE_DBM, requires WAL).
 *
 * services/dbm is a Bitcask-style key-value store: each put/delete appends one WAL record (so writes are
 * the WAL's fast sequential appends, not slow durable random writes), and an in-RAM open-addressed hash
 * index (fixed BSS, no heap) maps each live key to where its value sits in the log. Mount rebuilds the
 * index by scanning the WAL. Keys are bounded by DWS_DBM_KEY_MAX, values by DWS_DBM_VAL_MAX, and the
 * index holds up to DWS_DBM_SLOTS live keys. Default off.
 */
#ifndef DWS_ENABLE_DBM
#define DWS_ENABLE_DBM 0
#endif
#ifndef DWS_DBM_SLOTS
#define DWS_DBM_SLOTS 256 // max live keys (in-RAM index capacity; open-addressed, keep load < ~0.7)
#endif
#ifndef DWS_DBM_KEY_MAX
#define DWS_DBM_KEY_MAX 32 // largest key in bytes
#endif
#ifndef DWS_DBM_VAL_MAX
#define DWS_DBM_VAL_MAX 256 // largest value in bytes
#endif

/**
 * @brief Opt-in local JSON document store on the WAL (DWS_ENABLE_DOCSTORE, requires DBM + WAL).
 *
 * services/docstore is a small NoSQL document store: JSON documents addressed by an id, kept durably on
 * the write-ahead log. It is a thin layer over dbm (id = key, JSON body = value) and adds the document
 * capability - top-level field queries (find documents whose JSON field equals a value) via the zero-heap
 * JSON reader. Ids are bounded by DWS_DBM_KEY_MAX, bodies by DWS_DBM_VAL_MAX. Default off.
 */
#ifndef DWS_ENABLE_DOCSTORE
#define DWS_ENABLE_DOCSTORE 0
#endif
#ifndef DWS_DOCSTORE_FIELD_MAX
#define DWS_DOCSTORE_FIELD_MAX 128 // largest string field value a find can compare
#endif

/**
 * @brief Opt-in SQLite3 on-disk file-format reader (DWS_ENABLE_SQLITE).
 *
 * services/sqlite parses the documented SQLite database file structure by hand - the 100-byte database
 * header, the b-tree page header, the record varint, and record serial types - so a device can read a
 * SQLite file (from dws_wal_fs / fs::FS) without the SQLite amalgamation (which needs a heap + stdio and does
 * not fit the no-stdlib zero-heap model). Read-first (a bounded writer is a later step); pure, host-tested
 * against real files from the sqlite3 CLI. Default off.
 */
#ifndef DWS_ENABLE_SQLITE
#define DWS_ENABLE_SQLITE 0
#endif

/**
 * @brief Opt-in Redis RESP wire codec (DWS_ENABLE_REDIS).
 *
 * services/redis is the pure wire layer of a Redis client: a RESP command encoder (a command becomes a
 * RESP array of bulk strings) and a streaming, zero-heap reply decoder that reads one value at a time, so
 * arbitrarily nested replies are walked with only the caller's loop state (no tree allocation). Covers
 * RESP2 and the RESP3 additions (null / boolean / double / big number / bulk error / verbatim / map / set
 * / push). Pure (no I/O; you hand it byte buffers); host-tested against spec vectors and a real
 * redis-server. Default off.
 */
#ifndef DWS_ENABLE_REDIS
#define DWS_ENABLE_REDIS 0
#endif

/**
 * @brief Opt-in CNC RS-232 DNC drip-feed codec (DWS_ENABLE_DNC).
 *
 * services/dnc is the transport-agnostic framing + tape-code layer that streams a G-code program
 * (RS-274 / ISO 6983) to a machine-tool controller over RS-232 or a socket: block framing with a `%`
 * rewind-stop, ISO 7-bit (ASCII, optional even parity) or EIA RS-244 (odd-parity punched-tape code)
 * character translation, a streaming block encoder + reassembling decoder, and XON/XOFF software
 * flow-control state. Pure codec (you own the UART / socket); host-tested. Default off.
 */
#ifndef DWS_ENABLE_DNC
#define DWS_ENABLE_DNC 0
#endif

/**
 * @brief Largest G-code block (one line) the DNC decoder reassembles (DWS_ENABLE_DNC).
 *
 * A block longer than this overflows the decoder's fixed line buffer and is dropped whole
 * (::DNC_EV_OVERFLOW) rather than truncated. Sized for a normal G-code line; raise it only for
 * unusually long blocks (many parameters). Zero heap - this is the static per-decoder buffer.
 */
#ifndef DWS_DNC_LINE_MAX
#define DWS_DNC_LINE_MAX 128
#endif

/**
 * @brief Default leader/trailer runout length for the DNC encoder (DWS_ENABLE_DNC).
 *
 * The number of NUL runout bytes ::dws_dnc_encode_leader emits before the program (and can emit after
 * it). The reader skips them until the first `%`. Traditional tape leaders were a few inches of
 * blank feed; 32 bytes is a serial-link equivalent. Overridable per call via DncCfg::leader_len.
 */
#ifndef DWS_DNC_LEADER_LEN
#define DWS_DNC_LEADER_LEN 32
#endif

/**
 * @brief Safety cap on how many times the DNC stream engine polls the reverse channel while paused
 *        by an XOFF, before giving up with an I/O error (DWS_ENABLE_DNC).
 *
 * `dnc_stream` pauses on XOFF and polls `recv` for the XON that resumes it; a well-behaved transport
 * paces `recv` (blocks briefly when idle) so this cap is only a backstop against a `recv` that spins
 * returning no data forever. Raise it if a slow controller legitimately holds XOFF for a long time.
 */
#ifndef DWS_DNC_XOFF_MAX_POLLS
#define DWS_DNC_XOFF_MAX_POLLS 200000
#endif

/**
 * @brief Opt-in TCP relay / DNAT port forwarding (DWS_ENABLE_RELAY).
 *
 * services/relay is a bidirectional byte pump that publishes an internal `host:port` through the
 * server: an inbound connection is relayed to an origin (an outbound dws_client connection), moving
 * bytes both ways with backpressure and independent half-close, so the device fronts a service that
 * lives behind it. The engine is a pure step function over two send/recv seams (host-testable); the
 * app owns the two sockets. Default off.
 */
#ifndef DWS_ENABLE_RELAY
#define DWS_ENABLE_RELAY 0
#endif

/**
 * @brief User-defined address:port -> hardware-bus bridge (services/iface_bridge).
 *
 * A configurable "device server": the app registers rules mapping a listen `x.x.x.x:nnnn` (TCP/UDP) to a
 * UART, SPI chip-select, or I2C address. A network client talking to the port is bridged to that bus -
 * raw stream passthrough for UART, or framed write-then-read transactions (uint16 write_len || uint16
 * read_len || write_bytes) for SPI/I2C. The rule table + transaction frame codec are a pure, zero-heap,
 * host-tested core; the bus I/O (Serial/SPI/Wire) and the PROTO_BRIDGE listener are the ESP32 step.
 * Default off.
 */
#ifndef DWS_ENABLE_IFACE_BRIDGE
#define DWS_ENABLE_IFACE_BRIDGE 0
#endif

/** @brief Max concurrent address:port -> bus rules (services/iface_bridge). */
#ifndef DWS_BRIDGE_MAX_RULES
#define DWS_BRIDGE_MAX_RULES 8
#endif

/**
 * @brief Max write / read payload (bytes) per TRANSACTION frame (services/iface_bridge).
 *
 * Bounds the per-transaction stack scratch used to clock an SPI/I2C write-then-read, and rejects a frame
 * whose write_len or read_len exceeds it. Device-server transactions are small register accesses, so the
 * default is modest; a frame over the cap closes the connection (protocol error). Keep it comfortably
 * under the transport RX ring so a whole frame can buffer before it is parsed.
 */
#ifndef DWS_BRIDGE_TXN_MAX
#define DWS_BRIDGE_TXN_MAX 256
#endif

/** @brief STREAM (UART) pipe chunk size (bytes) for services/iface_bridge - one socket<->UART hop. */
#ifndef DWS_BRIDGE_STREAM_CHUNK
#define DWS_BRIDGE_STREAM_CHUNK 256
#endif

/** @brief UART TRANSACTION read window (ms): how long a write-then-read waits for the read_len reply. */
#ifndef DWS_BRIDGE_UART_TXN_MS
#define DWS_BRIDGE_UART_TXN_MS 50
#endif

/**
 * @brief GNSS RTK base station + NTRIP caster (services/gnss).
 *
 * Turns the device into a differential-GNSS correction source: it surveys in a fixed antenna position and
 * serves RTCM 3.x corrections to rovers over the network as an NTRIP caster, so a rover applies them for
 * RTK / DGPS accuracy. The RTCM3 frame codec (0xD3 preamble, 10-bit length, CRC-24Q, message-type parse,
 * MSB-first bit I/O, station-reference 1005/1006 encode/decode) is a pure, zero-heap, host-tested core;
 * the caster server (rover connections + sourcetable) and the receiver bring-up (UBX / NMEA over UART) are
 * the ESP32 step. Generating RTCM3 *observation* (MSM) messages needs a receiver that outputs raw
 * measurements (u-blox RXM-RAWX: F9P / M8T class); a raw-less module (NEO-6/7, GT-U7) can still serve the
 * surveyed reference point + sourcetable. Default off.
 */
#ifndef DWS_ENABLE_NTRIP_CASTER
#define DWS_ENABLE_NTRIP_CASTER 0
#endif

/** @brief Max concurrent rover connections a caster serves corrections to (services/gnss). */
#ifndef DWS_NTRIP_MAX_ROVERS
#define DWS_NTRIP_MAX_ROVERS 4
#endif

// The base surveys in from the receiver's GGA fixes, so the NTRIP caster implies the NMEA 0183 codec.
#if DWS_ENABLE_NTRIP_CASTER && !DWS_ENABLE_NMEA0183
#undef DWS_ENABLE_NMEA0183
#define DWS_ENABLE_NMEA0183 1
#endif

/** @brief Max length (incl. NUL) of an NTRIP mountpoint name the caster serves. */
#ifndef DWS_NTRIP_MOUNT_MAX
#define DWS_NTRIP_MOUNT_MAX 32
#endif

/** @brief Max NTRIP client request size (bytes) the caster buffers while reading the request headers. */
#ifndef DWS_NTRIP_REQ_MAX
#define DWS_NTRIP_REQ_MAX 512
#endif

/** @brief Max distinct mountpoints a single caster serves (each = one RTCM stream). */
#ifndef DWS_NTRIP_MAX_MOUNTS
#define DWS_NTRIP_MAX_MOUNTS 2
#endif

/**
 * @brief Per-direction relay buffer size (bytes) for services/relay (DWS_ENABLE_RELAY).
 *
 * Each active relay holds two buffers of this size (one per direction) for bytes read from one peer
 * but not yet accepted by the other (backpressure carry). Larger buffers raise throughput per step
 * (fewer cross-thread dws_conn_send marshals per KB) at the cost of RAM per concurrent relay
 * (2 * DWS_RELAY_BUF * DWS_RELAY_MAX_CONNS bytes).
 */
#ifndef DWS_RELAY_BUF
#define DWS_RELAY_BUF 2048
#endif

/**
 * @brief Max dws_relay_step passes per poll for the relay listener (DWS_ENABLE_RELAY).
 *
 * One poll drains up to this many DWS_RELAY_BUF chunks per direction, so a single event forwards the
 * whole buffered origin RX ring (DWS_CLIENT_RX_BUF) instead of one chunk - the difference between a
 * ~0.4 Mbps and a multi-Mbps port-forward. Bounded so one busy bridge cannot starve the others.
 */
#ifndef DWS_RELAY_DRAIN_MAX
#define DWS_RELAY_DRAIN_MAX 8
#endif

/**
 * @brief Max published relay ports (bind table size) for the relay listener (DWS_ENABLE_RELAY).
 *
 * Each dws_relay_publish() call binds one listener port to one origin `host:port`. This caps how
 * many distinct ports the device can front at once.
 */
#ifndef DWS_RELAY_MAX_PUBLISH
#define DWS_RELAY_MAX_PUBLISH 4
#endif

/**
 * @brief Max concurrent relayed connections (bridge table size) for the relay listener
 *        (DWS_ENABLE_RELAY). Each holds a DWSRelay (two DWS_RELAY_BUF buffers) + an origin slot.
 */
#ifndef DWS_RELAY_MAX_CONNS
#define DWS_RELAY_MAX_CONNS 4
#endif

/** @brief Max origin hostname length (bytes, incl. NUL) stored per published relay port. */
#ifndef DWS_RELAY_HOST_MAX
#define DWS_RELAY_HOST_MAX 64
#endif

/** @brief Blocking connect timeout (ms) when the relay listener dials the origin on a new inbound. */
#ifndef DWS_RELAY_CONNECT_MS
#define DWS_RELAY_CONNECT_MS 5000
#endif

/**
 * @brief Opt-in FTP client wire codec (DWS_ENABLE_FTP).
 *
 * services/ftp is the pure protocol layer of an FTP client (RFC 959 + RFC 2428 EPSV/EPRT):
 * `dws_ftp_build_command` / `dws_ftp_build_port` / `dws_ftp_build_eprt` build control-channel commands,
 * `dws_ftp_parse_reply` detects a complete single- or multi-line 3-digit reply, and
 * `dws_ftp_parse_pasv` / `dws_ftp_parse_epsv` decode the data-channel address the server returns. So a
 * device can push/pull files - e.g. drip a `.nc` program to a CNC controller's FTP store. Pure
 * codec (you own the control + data sockets); host-tested. Default off.
 */
#ifndef DWS_ENABLE_FTP
#define DWS_ENABLE_FTP 0
#endif

/**
 * @brief Suggested FTP control-command buffer size (DWS_ENABLE_FTP).
 *
 * A convenience cap for callers sizing the buffer they hand `dws_ftp_build_command`; the builders
 * are all length-checked against the caller's `cap`, so this is only a sensible default. Large
 * enough for a RETR / STOR with a long path.
 */
#ifndef DWS_FTP_CMD_MAX
#define DWS_FTP_CMD_MAX 256
#endif

/**
 * @brief Opt-in HTTP Cache-Control directive helpers (DWS_ENABLE_HTTP_CACHE).
 *
 * services/httpcache is the origin-side of edge caching (RFC 9111 + RFC 8246 + RFC 5861): a
 * structured `Cache-Control` builder (`cache_control_build` + first-class presets like
 * `cache_immutable_asset` / `cache_shared`) so app routes emit correct, edge-cacheable responses
 * (hand the value to DWS::set_cache_control()), a tolerant directive parser
 * (`cache_control_parse`), and the RFC 9111 freshness-lifetime calculation. Pure text, host-tested.
 * Groundwork for the CDN roadmap; the caching tier itself is a separate piece. Default off.
 */
#ifndef DWS_ENABLE_HTTP_CACHE
#define DWS_ENABLE_HTTP_CACHE 0
#endif

/**
 * @brief Opt-in CDN edge-cache tier (DWS_ENABLE_EDGE_CACHE, requires HTTP_CACHE).
 *
 * services/edge_cache is the caching reverse-proxy edge that services/httpcache is the origin-side
 * groundwork for: a device sits in front of a remote upstream origin, fetches a response once, and
 * serves subsequent hits from a bounded local store - honoring `Cache-Control` / `Expires` / `ETag` /
 * `Last-Modified`, revalidating stale entries with conditional requests (`If-None-Match` /
 * `If-Modified-Since` -> 304), and serving `Range` / `206` straight from the cache. A two-tier store:
 * bounded RAM (L1, hot) plus an optional dbm/WAL-backed SD tier (L2, persistent, when DWS_ENABLE_DBM
 * is set). Misses/revalidations fetch the origin asynchronously (the client request is suspended and
 * resumed from the poll loop, never stalling the worker) and always fail open. Zero heap. Default off.
 */
#ifndef DWS_ENABLE_EDGE_CACHE
#define DWS_ENABLE_EDGE_CACHE 0
#endif
#if DWS_ENABLE_EDGE_CACHE && !DWS_ENABLE_HTTP_CACHE
#error "DWS_ENABLE_EDGE_CACHE requires DWS_ENABLE_HTTP_CACHE"
#endif
#if DWS_ENABLE_EDGE_CACHE && !DWS_ENABLE_HTTP_CLIENT
#error "DWS_ENABLE_EDGE_CACHE requires DWS_ENABLE_HTTP_CLIENT (it fetches the upstream origin)"
#endif
// Opt-in TLS upstream origins: when set, a mapped `https://` origin is fetched over the shared client-TLS
// session (dws_tls_csess) instead of being rejected. One outbound TLS origin fetch at a time (the session is
// a singleton, shared with MQTTS/wss); the handshake blocks the worker briefly at connect (like the MQTT/WS
// clients). Needs the TLS engine + the ~48 KB arena - an S3 / PSRAM board is recommended.
#ifndef DWS_ENABLE_EDGE_ORIGIN_TLS
#define DWS_ENABLE_EDGE_ORIGIN_TLS 0
#endif
#if DWS_ENABLE_EDGE_ORIGIN_TLS && !DWS_ENABLE_EDGE_CACHE
#error "DWS_ENABLE_EDGE_ORIGIN_TLS requires DWS_ENABLE_EDGE_CACHE"
#endif
#if DWS_ENABLE_EDGE_ORIGIN_TLS && !DWS_ENABLE_TLS
#error "DWS_ENABLE_EDGE_ORIGIN_TLS requires DWS_ENABLE_TLS (the client-TLS engine)"
#endif
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 4 // L1 RAM entries (each holds one cached object; bump up on PSRAM)
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 2048 // largest cacheable body in bytes (per L1 entry; bump up on PSRAM)
#endif
#ifndef DWS_EDGE_KEY_MAX
#define DWS_EDGE_KEY_MAX 128 // largest canonical cache key (method\nhost\npath[\nquery])
#endif
#ifndef DWS_EDGE_VARY_MAX
#define DWS_EDGE_VARY_MAX 64 // stored Vary field-name list / captured request values (each)
#endif
#ifndef DWS_EDGE_MAP_MAX
#define DWS_EDGE_MAP_MAX 4 // path-prefix -> origin route mappings
#endif
#ifndef DWS_EDGE_ORIGIN_URL_MAX
#define DWS_EDGE_ORIGIN_URL_MAX 128 // largest origin base URL in a route mapping
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 2 // concurrent in-flight origin fetches (<= DWS_CLIENT_CONNS)
#endif
#ifndef DWS_EDGE_FETCH_BUF
#define DWS_EDGE_FETCH_BUF 2560 // per-fetch origin-response accumulation buffer (>= body max + headers)
#endif
#ifndef DWS_EDGE_FETCH_TIMEOUT_MS
#define DWS_EDGE_FETCH_TIMEOUT_MS 8000 // origin fetch deadline before fail-open
#endif
#ifndef DWS_EDGE_DEFAULT_TTL_S
#define DWS_EDGE_DEFAULT_TTL_S 60 // fallback freshness when no directive and no wall clock
#endif
// L2 (SD) tier: when DWS_ENABLE_DBM is also set, the edge cache spills evicted entries to a dbm store
// (edge_cache_sd) so the cached set survives a reboot. Each entry serializes to ~ its body plus ~470 B of
// response metadata; for a full-body spill, DWS_DBM_VAL_MAX must be >= that size (>= DWS_EDGE_BODY_MAX
// + ~470). Entries that do not fit simply stay L1-only, so a small DWS_DBM_VAL_MAX is safe but persists
// less. The L2 key is the 32-byte cache-key digest, so DWS_DBM_KEY_MAX must be >= 32 (its default).

/**
 * @brief Opt-in mesh (sibling-cache) distribution for the edge cache (DWS_ENABLE_EDGE_MESH).
 *
 * Lets a fleet of edge nodes share one warm cache: on a full local miss, a node queries its configured
 * sibling peers (over a plaintext ConnProto::PROTO_MESH TCP link) before hitting the origin, and pulls a
 * fresh copy from whichever peer has it - so the origin is fetched once per fleet, not once per node. Pull
 * (read-through) only: no push, no invalidation protocol, no consistency window - a stale sibling copy
 * self-expires by its own TTL and the requester re-checks freshness on arrival. The transfer carries the
 * object plus its freshness/age (RFC 9111 age propagation), so a sibling-fresh object serves for its
 * remaining lifetime with zero origin contact. A serving node answers only from its local store (one hop,
 * never re-querying its own origin/peers, so the fleet cannot loop). Peers are a static list
 * (dws_edge_cache_add_peer); auto-discovery is a follow-up. Zero heap. Default off.
 */
#ifndef DWS_ENABLE_EDGE_MESH
#define DWS_ENABLE_EDGE_MESH 0
#endif
#if DWS_ENABLE_EDGE_MESH && !DWS_ENABLE_EDGE_CACHE
#error "DWS_ENABLE_EDGE_MESH requires DWS_ENABLE_EDGE_CACHE"
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 4 // sibling peers queried on a local miss (tried in series, first hit wins)
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 1 // concurrent inbound peer-serve connections (a small fleet queries serially)
#endif
#ifndef DWS_MESH_QUERY_MS
#define DWS_MESH_QUERY_MS 300 // per-peer query deadline before moving on (miss) / to the origin
#endif
#ifndef DWS_MESH_HOST_MAX
#define DWS_MESH_HOST_MAX 64 // largest sibling peer host string
#endif
#ifndef DWS_MESH_HDRS_MAX
#define DWS_MESH_HDRS_MAX                                                                                              \
    384 // request-header snapshot carried to a peer so it can match Vary variants
        // (headers past the cap are dropped -> at worst a safe mesh miss, never wrong content)
#endif

/**
 * @brief Opt-in SMB2 client (DWS_ENABLE_SMB).
 *
 * services/smb is an SMB2 client (MS-SMB2) so a device can read/write files on a Windows share -
 * e.g. a CNC controller's program store. The full read/write-a-file path: the Direct-TCP transport
 * frame + SMB2 sync header, NEGOTIATE, the two-round NTLMv2 SESSION_SETUP (NTLM digests MD4/MD5/
 * HMAC-MD5, the NTLMv2 response, the NTLMSSP messages, SPNEGO wrapping), TREE_CONNECT, CREATE, READ,
 * WRITE, and CLOSE. smb_client ties the codecs into the exchange over a send/recv seam (host-tested
 * with a scripted mock server); you own the TCP socket (dws_client). All little-endian. Default off.
 */
#ifndef DWS_ENABLE_SMB
#define DWS_ENABLE_SMB 0
#endif

/**
 * @brief SMB2 client work-buffer size (bytes) for smb_client's request/response framing.
 *
 * Two buffers of this size live on the stack during a call, plus a few half-size scratch buffers for
 * the NTLM auth tokens, so the engine needs roughly 4x this in stack. 1024 covers the NEGOTIATE ->
 * SESSION_SETUP -> TREE_CONNECT -> CREATE handshake; raise it if a server's SPNEGO/target-info token
 * or your share path is unusually large.
 */
#ifndef DWS_SMB_BUF
#define DWS_SMB_BUF 1024
#endif

/**
 * @brief Opt-in SAE J2735 V2X codec (DWS_ENABLE_J2735).
 *
 * When set, services/j2735 provides the ASN.1 UPER (Unaligned Packed Encoding Rules) bit-level primitive
 * codec (constrained INTEGER / BOOLEAN / bit fields) and, on top of it, the J2735 BSMcore safety-message
 * block (msgCnt / id / secMark / lat / long / elev / speed / heading) encode + decode, for connected-
 * vehicle messaging. Pure codec (the DSRC / C-V2X radio is an external module). Default off.
 */
#ifndef DWS_ENABLE_J2735
#define DWS_ENABLE_J2735 0
#endif

/**
 * @brief Opt-in NEMA TS 2 traffic-cabinet SDLC frame codec (DWS_ENABLE_NEMA_TS2).
 *
 * When set, services/nema_ts2 builds/validates the TS 2 SDLC bus frames ([address][control][frame-type]
 * [data][CRC-16/X-25]) that link a traffic-signal controller to the MMU, BIUs, and detector racks. Pure
 * codec (the synchronous serial PHY + BIU timing are hardware-gated). Default off.
 */
#ifndef DWS_ENABLE_NEMA_TS2
#define DWS_ENABLE_NEMA_TS2 0
#endif

/**
 * @brief Opt-in GE Fanuc SNP (Series Ninety Protocol) serial frame codec (DWS_ENABLE_SNP).
 *
 * When set, services/snp builds/validates the SNP master-slave serial frame ([control][length][data]
 * [arithmetic-sum BCC]) for reading/writing registers on a GE Fanuc Series 90 (90-30/90-70) PLC over
 * RS-485. Pure codec (the UART transport + SNP-X session are the device step). Default off.
 */
#ifndef DWS_ENABLE_SNP
#define DWS_ENABLE_SNP 0
#endif

/**
 * @brief Opt-in AutomationDirect / Koyo DirectNET serial frame codec (DWS_ENABLE_DIRECTNET).
 *
 * When set, services/directnet builds/validates the DirectNET master-slave serial frames - the header
 * (SOH + slave/type/address/blocks ASCII-hex + ETB + LRC) and the data frame (STX + data + ETX + LRC) -
 * for V-memory read/write on an AutomationDirect DirectLOGIC PLC. Pure codec (the UART transport +
 * ACK/NAK handshake are the device step). Default off.
 */
#ifndef DWS_ENABLE_DIRECTNET
#define DWS_ENABLE_DIRECTNET 0
#endif

/**
 * @brief Opt-in IEEE 2030.5 (Smart Energy Profile 2.0) resource codec (DWS_ENABLE_SEP2).
 *
 * When set, services/sep2 builds the core 2030.5 XML resource documents (DeviceCapability, EndDevice,
 * DERControl) in the urn:ieee:std:2030.5:ns namespace, so the web server is a 2030.5 smart-grid
 * server/client over the existing HTTP stack (DER dispatch / curtailment). Pure text framing. Default off.
 */
#ifndef DWS_ENABLE_SEP2
#define DWS_ENABLE_SEP2 0
#endif

/**
 * @brief Opt-in PROFINET DCP (Discovery and Configuration Protocol) frame codec (DWS_ENABLE_PROFINET).
 *
 * When set, services/profinet builds/parses the DCP frames (10-octet header + option/suboption blocks)
 * PROFINET uses to discover and name IO-Devices over raw L2 (ethertype 0x8892) - Identify request/
 * response and Set (assign NameOfStation / IP). Pure codec (the raw-L2 transmit via services/rawl2 +
 * esp_eth is the device step). Default off.
 */
#ifndef DWS_ENABLE_PROFINET
#define DWS_ENABLE_PROFINET 0
#endif

/**
 * @brief Opt-in NTCIP transportation-device object identifiers (DWS_ENABLE_NTCIP).
 *
 * When set, services/ntcip provides the NTCIP (National Transportation Communications for ITS Protocol)
 * object OID definitions for the common device classes - NTCIP 1202 (actuated signal controller: phases,
 * timing, live states) and 1203 (dynamic message sign) - plus an OID builder, so an app exposes them via
 * the shipped SNMP agent (services/snmp). Pure OID data. Default off.
 */
#ifndef DWS_ENABLE_NTCIP
#define DWS_ENABLE_NTCIP 0
#endif

/**
 * @brief Opt-in OpenADR 3.0 (Automated Demand Response) JSON codec (DWS_ENABLE_OPENADR).
 *
 * When set, services/openadr builds the OpenADR 3.0 event (a demand-response signal: programID +
 * eventName + interval payload points) and report (a VEN reading back to the VTN) JSON objects into a
 * caller buffer, over the existing HTTP client/server + OAuth2. Pure JSON framing. Default off.
 */
#ifndef DWS_ENABLE_OPENADR
#define DWS_ENABLE_OPENADR 0
#endif

/**
 * @brief Opt-in IEC 61850 MMS PDU codec (DWS_ENABLE_MMS).
 *
 * When set, services/mms builds/parses the MMS (ISO 9506) confirmed-request/response Read PDUs
 * (BER-encoded, the ACSI client/server core of IEC 61850) - dws_mms_read_request builds a Read of a
 * named Data Object, dws_mms_read_response the data reply. Carried over ISO-on-TCP (TPKT + COTP via
 * the shipped services/cotp) on port 102. Pure BER codec. Default off.
 */
#ifndef DWS_ENABLE_MMS
#define DWS_ENABLE_MMS 0
#endif

/**
 * @brief Opt-in CC-Link (CLPA) cyclic fieldbus frame codec (DWS_ENABLE_CCLINK).
 *
 * When set, services/cclink builds/validates the CC-Link cyclic frame ([station][command][RX/RY bit
 * data][RWr/RWw word data][sum checksum]) a Mitsubishi CC-Link master exchanges with remote stations
 * over RS-485, plus bit/word process-image accessors. Pure codec (the RS-485 timing + CC-Link IE Field
 * PHY are hardware-gated). Default off.
 */
#ifndef DWS_ENABLE_CCLINK
#define DWS_ENABLE_CCLINK 0
#endif

/**
 * @brief Opt-in Ethernet POWERLINK (EPSG) basic frame codec (DWS_ENABLE_POWERLINK).
 *
 * When set, services/powerlink builds/parses the EPL basic frames ([messageType][dest][source][payload])
 * of the isochronous managed-node cycle - SoC (start of cycle), PReq (poll request), PRes (poll
 * response with process data), SoA (start of async) - over raw L2 (ethertype 0x88AB, on the shipped
 * services/rawl2). Pure codec (the raw-L2 transmit + isochronous timing are the device step). Default off.
 */
#ifndef DWS_ENABLE_POWERLINK
#define DWS_ENABLE_POWERLINK 0
#endif

/**
 * @brief Opt-in SERCOS III motion-bus telegram codec (DWS_ENABLE_SERCOS).
 *
 * When set, services/sercos builds/parses the SERCOS III MDT/AT telegrams (type + phase + cycle + cyclic
 * device data) the real-time drive/motion bus exchanges over raw L2 (ethertype 0x88CD, on the shipped
 * services/rawl2), plus the IDN (IDentification Number) encode/decode for drive-parameter addressing.
 * Pure codec (the isochronous timing + ring topology are hardware-gated). Default off.
 */
#ifndef DWS_ENABLE_SERCOS
#define DWS_ENABLE_SERCOS 0
#endif

/**
 * @brief Opt-in PROFIBUS-DP FDL telegram codec (DWS_ENABLE_PROFIBUS).
 *
 * When set, services/profibus builds/validates the PROFIBUS-DP FDL telegrams - SD1 (no-data: SD1 DA SA
 * FC FCS ED) and SD2 (variable-data: SD2 LE LEr SD2 DA SA FC data FCS ED, arithmetic-sum FCS) - a
 * Siemens DP master exchanges with slaves over RS-485 (the DP-V0 cyclic I/O exchange). Pure codec (the
 * RS-485 timing + DP state machine are the device step). Default off.
 */
#ifndef DWS_ENABLE_PROFIBUS
#define DWS_ENABLE_PROFIBUS 0
#endif

/**
 * @brief Opt-in LonWorks / LON-IP (ISO/IEC 14908) network-variable codec (DWS_ENABLE_LONWORKS).
 *
 * When set, services/lonworks builds/parses the LonTalk network-variable PDU ([msg-code][14-bit
 * selector][value]) that a building-automation device exchanges - over LON/IP (14908-4) UDP, so no
 * Neuron chip is needed - plus the common SNVT scalar encodings (SNVT_temp, SNVT_switch). Pure codec
 * (the UDP transport is the shipped UDP layer). Default off.
 */
#ifndef DWS_ENABLE_LONWORKS
#define DWS_ENABLE_LONWORKS 0
#endif

/**
 * @brief Opt-in Modbus Plus HDLC token-bus frame codec (DWS_ENABLE_MBPLUS).
 *
 * When set, services/mbplus builds/validates the Modbus Plus HDLC frame (7E addr ctrl payload CRC-16/X-25
 * 7E) that Schneider's token-passing peer bus exchanges, plus the token-rotation helper (next station in
 * the logical ring). Reuses the shipped Modbus PDU model for the data. Pure codec (the 1 Mbit/s bus is
 * hardware-gated). Default off.
 */
#ifndef DWS_ENABLE_MBPLUS
#define DWS_ENABLE_MBPLUS 0
#endif

/**
 * @brief Opt-in INTERBUS summation-frame fieldbus codec (DWS_ENABLE_INTERBUS).
 *
 * When set, services/interbus assembles/disassembles the INTERBUS summation frame (loopback word +
 * per-device 16-bit process-image slices + CRC-16/CCITT FCS) of the Phoenix Contact ring fieldbus,
 * where every device is a shift-register slice of one circulating frame. Pure codec (the physical ring
 * clocking is hardware-gated). Default off.
 */
#ifndef DWS_ENABLE_INTERBUS
#define DWS_ENABLE_INTERBUS 0
#endif

/**
 * @brief Opt-in ICCP / TASE.2 (IEC 60870-6) inter-control-center telemetry codec (DWS_ENABLE_ICCP).
 *
 * When set, services/iccp builds the TASE.2 Data_Value BER structures - StateQ (a discrete state +
 * quality) and RealQ (a scaled real + quality), each with an optional timestamp - the indication points
 * a control center transfers as MMS Reads (on the shipped services/mms + services/cotp). Pure BER codec.
 * Default off.
 */
#ifndef DWS_ENABLE_ICCP
#define DWS_ENABLE_ICCP 0
#endif

/**
 * @brief Opt-in IEEE 1609 WAVE (WSMP + 1609.2 envelope) codec (DWS_ENABLE_WAVE).
 *
 * When set, services/wave builds/parses the IEEE 1609 vehicular-radio framing that carries J2735: the
 * 1609.3 WSMP header (version + P-encoded PSID + length + payload) and the 1609.2 secured-message
 * envelope header (version + content type). Pairs with services/j2735. Pure codec (the DSRC / C-V2X
 * radio is an external module). Default off.
 */
#ifndef DWS_ENABLE_WAVE
#define DWS_ENABLE_WAVE 0
#endif

/**
 * @brief Opt-in UTMC (Urban Traffic Management and Control) common-database codec (DWS_ENABLE_UTMC).
 *
 * When set, services/utmc builds/parses the UTMC common-database HTTP+XML messages - a UTMCRequest for
 * an object id and a UTMCResponse carrying the object value + a data-quality flag + a timestamp - the UK
 * modular framework for sharing traffic data across municipal systems, over the existing HTTP server.
 * Pure text framing. Default off.
 */
#ifndef DWS_ENABLE_UTMC
#define DWS_ENABLE_UTMC 0
#endif

/**
 * @brief Opt-in OCIT-Outstations message codec (DWS_ENABLE_OCIT).
 *
 * When set, services/ocit builds/parses the OCIT (DE/AT/CH road-traffic-control) object messages
 * ([msg-type][object-type][instance][data-type][value]) between central traffic computers and field
 * controllers / detectors, with typed values (bool / byte / u16 / u32 / octets). Pure codec (the OCIT
 * transport is the shipped transport). Default off.
 */
#ifndef DWS_ENABLE_OCIT
#define DWS_ENABLE_OCIT 0
#endif

/**
 * @brief Opt-in ATC (Advanced Traffic Controller) field-I/O interop snapshot (DWS_ENABLE_ATC).
 *
 * When set, services/atc exposes this device's field-I/O (a fixed table of named input/output points it
 * already gathers via the NTCIP / NEMA-TS2 / gpio services) to an ATC Linux engine over the existing
 * HTTP surface: dws_atc_snapshot_json serializes the FIO map as JSON, and dws_atc_set_output drives
 * an output point from an ATC command. Pure interop codec (ATC is a platform spec, not a wire protocol).
 * Default off.
 */
#ifndef DWS_ENABLE_ATC
#define DWS_ENABLE_ATC 0
#endif

/**
 * @brief Opt-in southbound protocol-driver framework (DWS_ENABLE_SOUTHBOUND).
 *
 * The uniform seam every field-device driver plugs into so the app polls/drives any southbound device
 * (a Modbus slave, a BACnet controller, a raw sensor over SPI/I2C/UART) through one facade: register a
 * SouthboundDriver (a read/write/read_block/write_block vtable + its transport ctx), then address points
 * by driver name via dws_southbound_read / _write / _read_block / _write_block. The block calls are the
 * atomic multi-point (register-matrix) path. Bounded registry (DWS_SOUTHBOUND_MAX_DRIVERS, default 8),
 * no heap; Modbus master is the one such driver today. Default off.
 */
#ifndef DWS_ENABLE_SOUTHBOUND
#define DWS_ENABLE_SOUTHBOUND 0
#endif

/**
 * @brief Opt-in ESP32 panic / exception decoder for a live diagnostics panel (DWS_ENABLE_EXC_DECODER).
 *
 * When set, services/exc_decoder parses a captured Guru Meditation panic dump (the cause, the register
 * PC + EXCVADDR, and the backtrace PC:SP frames) into a structured ExcInfo and serializes it as JSON for
 * a "/exception" panel; the browser or a build server resolves the PCs to file:line against the firmware
 * ELF (addr2line lives off-device). Pure, no heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_EXC_DECODER
#define DWS_ENABLE_EXC_DECODER 0
#endif

/**
 * @brief Opt-in HTTP delivery optimizations (DWS_ENABLE_HTTP_DELIVERY).
 *
 * Three pure cores for cheaper HTTP serving, each a real web standard: RFC 5861 stale-while-revalidate
 * (dws_delivery_swr decision + dws_delivery_cache_control header), RFC 7233 byte-range delta/offset
 * fetch (dws_delivery_range parse of X-Y / X- / -N + dws_delivery_content_range for a 206), and a
 * versioned service-worker precache manifest (dws_delivery_sw_manifest). No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_HTTP_DELIVERY
#define DWS_ENABLE_HTTP_DELIVERY 0
#endif

/**
 * @brief Opt-in hardware-health diagnostics (DWS_ENABLE_HW_HEALTH).
 *
 * Four pure decision cores fed with samples the app reads from the hardware: a power-rail voltage-drop
 * logger (dws_hwhealth_rail_sample tracks worst droop + sag/brownout counts), a SPI-bus CRC audit with
 * hysteretic clock backoff (dws_hwhealth_spi_result halves/doubles the clock on fail/ok streaks), a
 * GPIO short-circuit test (dws_hwhealth_gpio_short: driven vs readback), and a capacitor-leakage diag
 * (dws_hwhealth_cap_leak: measured vs expected RC decay). No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_HW_HEALTH
#define DWS_ENABLE_HW_HEALTH 0
#endif

/**
 * @brief Opt-in adaptive mDNS beacon scheduling (DWS_ENABLE_MDNS_ADAPTIVE).
 *
 * Pure scheduling decisions on top of the shipped mDNS service: dws_mdns_beacon_adapt backs the
 * announce interval off toward a ceiling under RF contention and recovers it when the air is quiet,
 * dws_mdns_refresh_interval gives the TTL/2 continuous-refresher cadence, dws_mdns_beacon_due says
 * when an announce is due, and dws_mdns_beacon_presleep_due says whether to announce before a sleep
 * window that would otherwise let the record lapse. Wrap-safe time math, no heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_MDNS_ADAPTIVE
#define DWS_ENABLE_MDNS_ADAPTIVE 0
#endif

/**
 * @brief Opt-in dynamic socket recycling: an LRU connection-slot pool (DWS_ENABLE_SOCKPOOL).
 *
 * The transport-pool half of the adaptive-networking work: services/sockpool keeps a fixed table of
 * connection slots and, when saturated, recycles the least-recently-used slot for a new peer
 * (dws_sockpool_acquire returns the evicted id so the transport closes it), plus touch / release /
 * find. The app owns the real sockets; this owns which slot a connection lives in and which to reclaim
 * under pressure. No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_SOCKPOOL
#define DWS_ENABLE_SOCKPOOL 0
#endif

/**
 * @brief Opt-in buffer placement policy (DRAM vs PSRAM) + SPI DMA ping-pong manager (DWS_ENABLE_PSRAM_POOL).
 *
 * Pure buffer-management decisions for a PSRAM-equipped ESP32: dws_psram_place picks DRAM vs PSRAM for
 * a buffer by size, DMA requirement, and free-heap headroom (large/cold to PSRAM, small/hot + DMA to
 * DRAM, always leaving an internal-DRAM reserve), and dws_pingpong_* keeps the classic SPI DMA
 * double-buffer bookkeeping (CPU fills one buffer while DMA drains the other; swap flips their roles).
 * The actual heap_caps_calloc is the app's. No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_PSRAM_POOL
#define DWS_ENABLE_PSRAM_POOL 0
#endif

/**
 * @brief Opt-in dual-stack Happy Eyeballs destination selection (DWS_ENABLE_HAPPY_EYEBALLS).
 *
 * The client-side IPv6/IPv4 fallback decision on top of the shipped DWSIp: dws_he_pref scores a
 * destination (RFC 6724 scope + family), dws_he_order sorts a candidate list and interleaves the
 * address families (RFC 8305) so successive connection attempts alternate v6/v4, and
 * dws_he_attempt_due gates the next attempt by the Connection Attempt Delay. Fast IPv6 when it works,
 * quick fallback to IPv4 when it does not. Needs DWS_ENABLE_IPV6 to matter. No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_HAPPY_EYEBALLS
#define DWS_ENABLE_HAPPY_EYEBALLS 0
#endif

/**
 * @brief Opt-in 802.11 sniffer / traffic analyzer (DWS_ENABLE_WIFI_SNIFFER).
 *
 * The decode + decision layer for a promiscuous-mode WiFi sniffer: dws_wifi_parse decodes an 802.11
 * MAC header (frame-control type/subtype + flags and the addresses whose roles depend on ToDS/FromDS),
 * dws_wifi_stats_* tallies frames by type for a traffic panel, and dws_wifi_should_roam decides when
 * a candidate AP is enough stronger (RSSI hysteresis) to justify channel-agility roaming. The
 * promiscuous-mode radio callback stays the app's. No heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_WIFI_SNIFFER
#define DWS_ENABLE_WIFI_SNIFFER 0
#endif

/**
 * @brief Opt-in multi-interface egress selection / failover policy (DWS_ENABLE_LINK_MANAGER).
 *
 * The policy that drives which interface carries traffic once a device has more than one (a wired
 * Ethernet PHY alongside WiFi STA / softAP): services/link_manager keeps a small table of interfaces
 * (kind + priority + up/down) and deterministically selects the best link that is up, escalating to a
 * higher-priority interface when it comes up and failing over when it drops, reporting only real
 * transitions so the app reconfigures the netif once. The PHY bring-up (esp_eth) stays the app's. No
 * heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_LINK_MANAGER
#define DWS_ENABLE_LINK_MANAGER 0
#endif

/**
 * @brief Opt-in CC1101 sub-GHz radio driver (DWS_ENABLE_CC1101).
 *
 * A gateway radio plugin (DWS_ENABLE_GATEWAY) for the TI CC1101 300-928 MHz transceiver over SPI:
 * services/cc1101 drives the chip's SPI header protocol (config registers, command strobes, status
 * registers, TX/RX FIFO) - reset + apply a SmartRF register table + set channel + verify VERSION
 * (dws_cc1101_init), send a variable-length packet (dws_cc1101_send), poll TX-done, enter RX, and read a packet
 * with appended RSSI/LQI (dws_cc1101_recv), plus the RSSI-to-dBm decode. The huge modem config is a
 * caller-supplied register table. Host-tested against a mock; the RF link needs the module. Default off.
 */
#ifndef DWS_ENABLE_CC1101
#define DWS_ENABLE_CC1101 0
#endif

/**
 * @brief Opt-in FDC2114/2214 capacitance-to-digital field sensor (DWS_ENABLE_FDC2214).
 *
 * A field-perturbation sensing peripheral: services/fdc2214 decodes the FDC2x14's 28-bit conversion
 * result (a capacitance shift moves the LC-tank frequency, giving contactless proximity / liquid-level /
 * material sensing) - dws_fdc2214_data combines the register pair, dws_fdc2214_error pulls the flags,
 * dws_fdc2214_sensor_freq_hz scales to frequency, and dws_fdc2214_build_config emits a single-channel
 * bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec host-tested. Default off.
 */
#ifndef DWS_ENABLE_FDC2214
#define DWS_ENABLE_FDC2214 0
#endif

/**
 * @brief Opt-in LDC1614 inductance-to-digital field sensor (DWS_ENABLE_LDC1614).
 *
 * A field-perturbation sensing peripheral: services/ldc1614 decodes the LDC1614's 28-bit conversion
 * result (a nearby conductor changes the coil inductance via eddy currents, giving contactless metal
 * proximity / displacement / EM-field sensing) - dws_ldc1614_data combines the register pair, dws_ldc1614_error
 * pulls the flags, dws_ldc1614_sensor_freq_hz scales to frequency, and dws_ldc1614_build_config emits a
 * single-channel bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec
 * host-tested. Default off.
 */
#ifndef DWS_ENABLE_LDC1614
#define DWS_ENABLE_LDC1614 0
#endif

/**
 * @brief Opt-in VL53L0X optical time-of-flight ranging sensor (DWS_ENABLE_VL53L0X).
 *
 * A field-perturbation sensing peripheral for contactless distance / gesture: services/vl53l0x decodes
 * the ST VL53L0X ranging registers - dws_vl53l0x_range_mm combines the range byte pair, dws_vl53l0x_data_ready
 * decodes the interrupt-status byte, and dws_vl53l0x_range_valid checks the device range-status field; the
 * ESP32 binding verifies the model id, starts continuous ranging, and reads the distance over I2C.
 * Default-settings ranging (ST's tuning blob is not applied). Pure codec host-tested. Default off.
 */
#ifndef DWS_ENABLE_VL53L0X
#define DWS_ENABLE_VL53L0X 0
#endif

/**
 * @brief Opt-in receive-only radio channel sniffer to pcap (DWS_ENABLE_RADIO_SNIFF).
 *
 * Feeds frames pulled off the air by the RF gateway drivers (CC1101 / LoRa / 802.15.4) in receive-only
 * mode into the capture pipeline: services/radio_sniff wraps each 802.15.4 MAC frame in the Wireshark
 * TAP pseudo-header (carrying per-frame RSSI + channel) and a pcap record so the forwarded stream is a
 * valid .pcap. dws_radiosniff_global writes the DLT-TAP global header and dws_radiosniff_tap_record
 * writes one record. Pure framing (no heap/stdlib); the radio drivers own the receive. Default off.
 */
#ifndef DWS_ENABLE_RADIO_SNIFF
#define DWS_ENABLE_RADIO_SNIFF 0
#endif

/**
 * @brief Opt-in Bluetooth ATT protocol codec + GATT characteristic bridge (DWS_ENABLE_BLE_GATT).
 *
 * The wire protocol under GATT for bridging the on-chip BLE radio to the web: services/ble_gatt builds
 * and parses the common ATT PDUs (read / write / notify / error, Bluetooth Core Vol 3 Part F) and
 * serializes a GATT characteristic table as JSON for the web stack (att_read_req / att_write_req /
 * att_notify / att_error_rsp / att_parse / dws_gatt_char_json). The BLE stack owns the radio; this owns the
 * ATT bytes + the northbound JSON. Pure, no heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_BLE_GATT
#define DWS_ENABLE_BLE_GATT 0
#endif

/**
 * @brief Opt-in TLS version negotiation + pinned cipher-suite policy (DWS_ENABLE_TLS_POLICY).
 *
 * A policy layer on top of the mbedTLS-backed transport TLS (which already runs the 1.2 / 1.3 record +
 * handshake): services/tls_policy pins the version to an audited [min,max] and makes the negotiated
 * version observable (dws_tls_negotiate_version / dws_tls_version_name), and pins the cipher suites
 * to an audited allowlist selected by server preference (dws_tls_select_cipher), with an AEAD-only
 * classifier (dws_tls_is_aead) for a hardened profile. Pure, host-tested; the app feeds the results to
 * the mbedTLS config. Default off.
 */
#ifndef DWS_ENABLE_TLS_POLICY
#define DWS_ENABLE_TLS_POLICY 0
#endif

/**
 * @brief Opt-in Wi-SUN FAN border-router connector (DWS_ENABLE_WISUN).
 *
 * Wi-SUN FAN is an IPv6/UDP/CoAP mesh terminated by a border router, so the connector rides the existing
 * IP stack rather than driving a radio: services/wisun keeps a table of FAN nodes (their DWSIp addresses +
 * join state) behind the border router and builds the CoAP client requests to their resources
 * (dws_wisun_build_coap frames an RFC 7252 header + Uri-Path options + payload; the CoAP service ships only a
 * server). dws_wisun_nodes_json exposes the mesh to the web. The app sends the built PDU over dws_udp; the
 * chosen devboard only sets which border router you point at. Pure, no heap/stdlib. Default off.
 */
#ifndef DWS_ENABLE_WISUN
#define DWS_ENABLE_WISUN 0
#endif

/**
 * @brief Opt-in fixed-RAM rotating log buffer with severity traps (DWS_ENABLE_LOGBUF).
 *
 * Default off. When set, services/logbuf keeps the last DWS_LOG_LINES log lines
 * in a fixed ring (oldest pruned on overflow - no heap, bounded), dumps them
 * oldest-first for a `/logs` endpoint, and fires a trap callback when a line is
 * logged at/above a severity threshold (forward criticals as an SNMP trap /
 * webhook). The ring + trap logic is pure and host-tested.
 */
#ifndef DWS_ENABLE_LOGBUF
#define DWS_ENABLE_LOGBUF 0
#endif

/** @brief Number of log lines retained in the ring. */
#ifndef DWS_LOG_LINES
#define DWS_LOG_LINES 32
#endif

/** @brief Maximum length of one stored log line (bytes, including null). */
#ifndef DWS_LOG_LINE_LEN
#define DWS_LOG_LINE_LEN 96
#endif

/**
 * @brief Opt-in schema-driven config export / restore (DWS_ENABLE_CONFIG_IO).
 *
 * Default off. Requires DWS_ENABLE_CONFIG_STORE. The app declares a fixed schema
 * (key + type); services/config_io serializes the current values to a portable
 * `key=value` text blob (backup / migrate) and parses one back into the store
 * (restore / bulk template). Schema-driven rather than enumerating NVS, so it
 * stays deterministic and zero-heap; the serialize / parse is host-tested.
 */
#ifndef DWS_ENABLE_CONFIG_IO
#define DWS_ENABLE_CONFIG_IO 0
#endif

/** @brief Authenticated OTA firmware update (streaming POST to the ESP32 Update API). */
#ifndef DWS_ENABLE_OTA
#define DWS_ENABLE_OTA 0
#endif

/**
 * @brief Opt-in OTA rollback protection / soft-brick safeguard (DWS_ENABLE_OTA_ROLLBACK).
 *
 * Default off. After an OTA update the new image boots in PENDING_VERIFY; this
 * service confirms it (esp_ota_mark_app_valid) once a self-test passes, or rolls
 * back to the previous image if the self-test fails or the confirm window elapses
 * without success - so a bad update self-heals instead of soft-bricking. The
 * decision logic is pure and host-tested; the commit / rollback use esp_ota_ops.
 * Requires the bootloader's app-rollback support (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE).
 */
#ifndef DWS_ENABLE_OTA_ROLLBACK
#define DWS_ENABLE_OTA_ROLLBACK 0
#endif

/** @brief Confirm window (ms): a pending image not confirmed within this rolls back. */
#ifndef DWS_OTA_CONFIRM_WINDOW_MS
#define DWS_OTA_CONFIRM_WINDOW_MS 30000
#endif

/**
 * @brief Opt-in TOTP two-factor auth (RFC 6238) (DWS_ENABLE_TOTP).
 *
 * Default off. services/totp computes and verifies time-based one-time passwords
 * (HMAC-SHA1 over the existing SHA-1, Google Authenticator compatible) and decodes
 * base32 shared secrets, for a second factor on top of password / JWT auth. Pure
 * and host-tested against the RFC 6238 vectors; the verifier checks a +/- step
 * window for clock skew.
 */
#ifndef DWS_ENABLE_TOTP
#define DWS_ENABLE_TOTP 0
#endif

/**
 * @brief Opt-in outbound webhooks / IFTTT (DWS_ENABLE_WEBHOOK).
 *
 * Default off. Needs DWS_ENABLE_HTTP_CLIENT to actually send: the API always
 * compiles, but without the HTTP client dws_webhook_post() returns -1.
 * services/webhook builds an IFTTT Maker URL and a value1/value2/value3 JSON
 * payload (pure, host-tested) and fires them - or any JSON to any URL - via the
 * outbound http_client (POST). Use it to
 * push an event from the device to IFTTT, a Slack/Discord hook, or your own API.
 */
#ifndef DWS_ENABLE_WEBHOOK
#define DWS_ENABLE_WEBHOOK 0
#endif

/**
 * @brief Opt-in radio power controls (DWS_ENABLE_RADIO_POWER).
 *
 * Default off. services/radio_power applies the WiFi modem-sleep mode and an
 * optional max-TX-power cap in one call (esp_wifi_set_ps / esp_wifi_set_max_tx_power)
 * - trade throughput/latency for lower average power on a battery device. The mode
 * names are host-tested; the apply is ESP32-only.
 */
#ifndef DWS_ENABLE_RADIO_POWER
#define DWS_ENABLE_RADIO_POWER 0
#endif

/** @brief WiFi modem-sleep mode: 0 = none (max perf), 1 = min modem, 2 = max modem. */
#ifndef DWS_RADIO_WIFI_PS
#define DWS_RADIO_WIFI_PS 0
#endif

/** @brief Max TX power cap in dBm (2..20); 0 = leave the platform default. */
#ifndef DWS_RADIO_MAX_TX_DBM
#define DWS_RADIO_MAX_TX_DBM 0
#endif

/**
 * @brief Opt-in DNS resolver with answer verification (DWS_ENABLE_DNS_RESOLVER).
 *
 * Default off. services/dns_resolver resolves a hostname to an IPv4 address (lwIP
 * dns_gethostbyname, marshalled to tcpip_thread like the http_client) and can
 * reject suspicious answers - 0.0.0.0, broadcast, loopback, multicast - which are
 * spoofing / DNS-rebinding indicators for a remote host. The address classifier /
 * verifier is pure and host-tested; the resolve is ESP32-only (blocking, so call
 * it off the request hot path).
 */
#ifndef DWS_ENABLE_DNS_RESOLVER
#define DWS_ENABLE_DNS_RESOLVER 0
#endif

/** @brief DNS resolve timeout in milliseconds. */
#ifndef DWS_DNS_TIMEOUT_MS
#define DWS_DNS_TIMEOUT_MS 5000
#endif

/**
 * @brief Tamper-evident audit log (DWS_ENABLE_AUDIT_LOG).
 *
 * Default off. services/audit_log keeps an append-only, hash-chained security
 * log: each record carries SHA-256(prev_hash || fields), so altering, deleting,
 * or reordering any retained record breaks the chain (dws_audit_verify()
 * detects it). Storage is a fixed RAM ring of DWS_AUDIT_LOG_ENTRIES records
 * (no heap); when it wraps, a moving anchor keeps the retained window verifiable.
 * Install a sink (dws_audit_set_sink) to forward every record at creation time
 * to a durable / remote store - SD-card file, syslog or HTTP log service, serial
 * console - preserving the same chain off-device. Pure and host-tested.
 */
#ifndef DWS_ENABLE_AUDIT_LOG
#define DWS_ENABLE_AUDIT_LOG 0
#endif

// Ring depth and per-record message length are tunable in audit_log.h
// (DWS_AUDIT_LOG_ENTRIES, DWS_AUDIT_MSG_LEN); define them before include to
// override. The RAM cost is roughly DWS_AUDIT_LOG_ENTRIES * (DWS_AUDIT_MSG_LEN
// + 41) bytes.

/**
 * @brief OpenID Connect ID-token verification, RS256 (DWS_ENABLE_OIDC).
 *
 * Default off. services/oidc verifies an OIDC ID token (JWT) as a relying party:
 * requires alg RS256, selects the issuer key by kid from a JWKS, verifies the
 * RSASSA-PKCS1-v1.5 SHA-256 signature (real RSA modexp via ssh_rsa, mbedTLS-
 * accelerated on ESP32), and checks iss / aud / exp / nbf, extracting sub / email.
 * Pure and host-tested; the caller fetches + caches the JWKS over HTTPS (off the
 * request hot path) and passes the JSON in. Builds on the SSH RSA primitive, not
 * the HS256 JWT module (services/jwt), so the two are independent.
 */
#ifndef DWS_ENABLE_OIDC
#define DWS_ENABLE_OIDC 0
#endif

/** @brief Max accepted OIDC ID-token length (also sizes the Authorization buffer). */
#ifndef DWS_OIDC_MAX_LEN
#define DWS_OIDC_MAX_LEN 1600
#endif

/**
 * @brief Unified virtual filesystem wrapper (DWS_ENABLE_VFS).
 *
 * Default off. services/vfs exposes one small file API (open/read/write/close,
 * exists/size/remove/rename, whole-file helpers) over a pluggable backend, so a
 * feature can target storage without knowing the medium. A built-in zero-heap RAM
 * backend (fixed BSS pool - deterministic, host-identical) ships for scratch /
 * tests; an Arduino-FS backend (ESP32) wraps a real fs::FS (LittleFS / SD /
 * SPIFFS) for persistence. Mount one at startup; the API fails closed otherwise.
 * Pool dimensions are tunable in this config (DWS_VFS_RAM_FILES, _RAM_FILE_SIZE,
 * _MAX_OPEN, _NAME_MAX).
 */
#ifndef DWS_ENABLE_VFS
#define DWS_ENABLE_VFS 0
#endif

/**
 * @brief GraphQL query subset (DWS_ENABLE_GRAPHQL).
 *
 * Default off. services/graphql parses a GraphQL query into a fixed AST node pool
 * (no heap) and emits a `{"data":{...}}` response shaped exactly by the requested
 * selection. Schema-free: a field with a sub-selection is an object (the engine
 * recurses), a leaf field calls your single resolver, and arguments collected
 * along the path are handed to it. Supports nested selections, field arguments,
 * and the anonymous / `query` forms; mutations, subscriptions, fragments, and
 * variables are out of scope. Pure and host-tested; bounds are compile-time
 * (DWS_GQL_* in this config). Serve it from a POST /graphql route.
 */
#ifndef DWS_ENABLE_GRAPHQL
#define DWS_ENABLE_GRAPHQL 0
#endif

/**
 * @brief ESP-NOW peer messaging (DWS_ENABLE_ESPNOW).
 *
 * Default off. services/espnow wraps ESP-NOW connectionless peer-to-peer radio
 * messaging in a 3-byte typed envelope (magic + type + length) so a receiver can
 * demux by message type and reject a truncated frame, plus a bounded peer
 * registry (DWS_ESPNOW_MAX_PEERS, no heap). The envelope codec + registry are
 * pure and host-tested; the radio path (begin / add_peer / send / broadcast over
 * esp_now, decoded frames to a callback) is ESP32-only and can bridge to
 * WebSocket/SSE. No stdlib.
 */
#ifndef DWS_ENABLE_ESPNOW
#define DWS_ENABLE_ESPNOW 0
#endif

/**
 * @brief OAuth2 token-endpoint client (DWS_ENABLE_OAUTH2).
 *
 * Default off. services/oauth2 obtains tokens - the counterpart to the OIDC
 * ID-token verifier. It builds the percent-encoded form body for the
 * authorization_code and refresh_token grants (RFC 6749), supporting a
 * confidential client (client_secret) or a public client with PKCE
 * (code_verifier, RFC 7636), and parses the JSON token response (reusing the
 * zero-heap JSON reader). The build + parse core is pure and host-tested; the POST
 * to the token endpoint uses the HTTP(S) client (needs DWS_ENABLE_HTTP_CLIENT).
 * No heap, no stdlib.
 */
#ifndef DWS_ENABLE_OAUTH2
#define DWS_ENABLE_OAUTH2 0
#endif

/**
 * @brief OPC UA Binary server (DWS_ENABLE_OPCUA).
 *
 * Default off. services/opcua provides an OPC UA (IEC 62541) Binary server: the
 * little-endian built-in-type codec (incl. NodeId / ExtensionObject / DateTime /
 * Variant / DataValue / ReferenceDescription), UA-TCP (UACP) message framing, the
 * Hello/Acknowledge handshake, the SecureChannel (OpenSecureChannel, SecurityPolicy
 * None), the Session (CreateSession + ActivateSession), GetEndpoints, the Read, Write
 * and Browse services (registered resolvers map a NodeId to a value / accept a written
 * value / list child references), plus CloseSession + CloseSecureChannel and a
 * ServiceFault for unsupported services, served on TCP via ConnProto::PROTO_OPCUA
 * (`listen(4840, ConnProto::PROTO_OPCUA)`). The MSG framing is spec-faithful (incl.
 * SecureChannelId), so standard clients interoperate (verified with python asyncua:
 * connect + browse + read + write/read-back). All pure and host-tested. No heap, no stdlib.
 */
#ifndef DWS_ENABLE_OPCUA
#define DWS_ENABLE_OPCUA 0
#endif

/**
 * @brief OPC UA Binary client (DWS_ENABLE_OPCUA_CLIENT).
 *
 * Default off. Requires DWS_ENABLE_OPCUA (shares the codec). services/dws_opcua_client
 * provides the client side of the OPC UA Binary protocol: request builders (Hello,
 * OpenSecureChannel, CreateSession, ActivateSession, Read, Browse, CloseSession,
 * CloseSecureChannel) and response parsers, reusing the opcua.h codec. It is
 * transport-agnostic - the app supplies the outbound socket (e.g. an Arduino
 * WiFiClient) and feeds bytes through these pure builders/parsers. No heap, no stdlib.
 */
#ifndef DWS_ENABLE_OPCUA_CLIENT
#define DWS_ENABLE_OPCUA_CLIENT 0
#endif

/**
 * @brief umati - OPC UA for Machine Tools information model (DWS_ENABLE_UMATI).
 *
 * Default off. Requires DWS_ENABLE_OPCUA (builds on the OPC UA Binary server). services/umati
 * exposes the umati / OPC UA for Machine Tools companion model (OPC 40501-1, namespace
 * `http://opcfoundation.org/UA/MachineTool/`): a fixed MachineTool node hierarchy -
 * Identification, Monitoring (MachineTool / Channel / Spindle / Axis_X..Z), Production, and
 * Notification - served through the OPC UA Browse + Read resolvers out of a caller-owned
 * UmatiMachineTool struct you refresh each loop. Faithful BrowseNames per OPC 40501-1; a monitoring
 * (read-only) model any umati / OPC UA client browses and reads by BrowseName. No heap, no stdlib.
 */
#ifndef DWS_ENABLE_UMATI
#define DWS_ENABLE_UMATI 0
#endif

/** @brief NamespaceIndex the umati MachineTool nodes live at (default 1). */
#ifndef DWS_UMATI_NS
#define DWS_UMATI_NS 1
#endif

/**
 * @brief Streaming file upload: POST a body straight to a file on the filesystem.
 *
 * Default off. When set, src/services/upload_service/upload_service.h registers a POST route
 * that streams the request body directly into an Arduino FS file (LittleFS /
 * SPIFFS / SD) - the upload never has to fit in RAM. Reuses the same parser
 * streaming-body hook as OTA.
 *
 * For reliable streamed uploads the RX ring must hold at least one full TCP
 * receive window (RX_BUF_SIZE >= TCP_WND, ~5.7 KB by default): the transport
 * reopens the window only as the consumer drains the ring (ack-on-consume), so a
 * ring smaller than the window lets the peer overrun it and the transfer
 * deadlocks - you cannot advertise a window larger than your buffer. When a
 * streaming feature is enabled and RX_BUF_SIZE was left at its default, it is
 * automatically upsized below; an explicit RX_BUF_SIZE is honored as-is (set it
 * >= TCP_WND yourself). The 1024 default suits ordinary requests, not uploads.
 */
#ifndef DWS_ENABLE_UPLOAD
#define DWS_ENABLE_UPLOAD 0
#endif

/**
 * @brief Internal: the parser's streaming-body machinery (OTA, file upload, WebDAV PUT).
 *
 * Each streams the request body to a sink instead of buffering it into body[]; the
 * parser support is shared and compiled when any of these features is enabled. The
 * sink is a single global hook, so only one streaming consumer is active per build
 * (the last to register wins) - do not combine OTA / upload / WebDAV streaming in
 * the same firmware.
 */
#if DWS_ENABLE_OTA || DWS_ENABLE_UPLOAD || DWS_ENABLE_WEBDAV
#define DWS_ENABLE_STREAM_BODY 1
#else
#define DWS_ENABLE_STREAM_BODY 0
#endif

// Streamed uploads need the RX ring to hold a full TCP receive window or the peer
// overruns it and the transfer deadlocks (ack-on-consume reopens the window only as
// the ring drains). If RX_BUF_SIZE was left at its default, upsize it to a value
// that comfortably exceeds the usual TCP_WND (~5.7 KB) when streaming is enabled.
// An explicit RX_BUF_SIZE (build flag) is respected unchanged - set it >= TCP_WND.
#if DWS_ENABLE_STREAM_BODY && defined(DWS_RX_BUF_SIZE_DEFAULTED) && RX_BUF_SIZE < 8192
#undef RX_BUF_SIZE
#define RX_BUF_SIZE 8192
#endif

// A modern SSH client's first flight (identification banner + KEXINIT) is ~1.5 KB:
// post-quantum/curve kex names, cert host-key algs, EtM MACs, ext-info-c. The RX
// ring must hold it or the handshake resets at key exchange, so when SSH is enabled
// and RX_BUF_SIZE was left at its default, upsize it to fit a full KEXINIT. An
// explicit RX_BUF_SIZE (build flag) is respected unchanged - keep it >= 2 KB.
#if DWS_ENABLE_SSH && defined(DWS_RX_BUF_SIZE_DEFAULTED) && RX_BUF_SIZE < 2048
#undef RX_BUF_SIZE
#define RX_BUF_SIZE 2048
#endif

// A modern TLS ClientHello (TLS 1.3 key shares + cipher/sig-alg lists + the RFC 7685 padding real
// clients send) is ~1.5 KB and arrives as one TCP segment - larger than the 1024 default ring. The
// recv callback refuses a whole segment that will not fit the ring (ERR_MEM, lossless backpressure),
// so a ClientHello bigger than the ring is refused forever and the handshake stalls to an idle-timeout
// RST: every 1.3-leading client (curl, browsers, Python) then fails to connect while a 1.2-only client
// squeaks in. The ring must hold a full segment, so when TLS is enabled and RX_BUF_SIZE was left at its
// default, upsize it to fit a full ClientHello. An explicit RX_BUF_SIZE (build flag) is respected
// unchanged - keep it >= 2 KB.
#if DWS_ENABLE_TLS && defined(DWS_RX_BUF_SIZE_DEFAULTED) && RX_BUF_SIZE < 2048
#undef RX_BUF_SIZE
#define RX_BUF_SIZE 2048
#endif

/** @brief First-boot WiFi provisioning: softAP + captive-portal credentials form. */
#ifndef DWS_ENABLE_PROVISIONING
#define DWS_ENABLE_PROVISIONING 0
#endif

/**
 * @brief Syslog client (RFC 5424 over UDP).
 *
 * Default off. When set, the device can ship log lines to a remote syslog server
 * (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the
 * transport-layer UDP service - a zero-heap structured-logging sink for fleets
 * of constrained devices. See src/services/syslog/syslog.h.
 */
#ifndef DWS_ENABLE_SYSLOG
#define DWS_ENABLE_SYSLOG 0
#endif

/** @brief Maximum formatted syslog datagram length in bytes (RFC 5424 line). */
#ifndef DWS_SYSLOG_MSG_MAX
#define DWS_SYSLOG_MSG_MAX 256
#endif

/** @brief Maximum syslog HOSTNAME / APP-NAME field length (including NUL). */
#ifndef DWS_SYSLOG_FIELD_MAX
#define DWS_SYSLOG_FIELD_MAX 32
#endif

/** @brief Default syslog collector UDP port (RFC 5426 well-known 514; overridable at runtime
 *  via dws_syslog_init and here for a non-standard collector). */
#ifndef DWS_SYSLOG_DEFAULT_PORT
#define DWS_SYSLOG_DEFAULT_PORT 514
#endif

/**
 * @brief JWT bearer-token authentication (HS256).
 *
 * Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer
 * <jwt>` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can
 * read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a
 * stateless token. Signature verification is constant-time.
 */
#ifndef DWS_ENABLE_JWT
#define DWS_ENABLE_JWT 0
#endif

/** @brief Maximum accepted JWT length in bytes (header.payload.signature). */
#ifndef DWS_JWT_MAX_LEN
#define DWS_JWT_MAX_LEN 512
#endif

/**
 * @brief Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS).
 *
 * Default off. When set, src/services/http_client/http_client.h can issue a
 * blocking GET/POST to a remote server: it resolves the host (DNS), opens a raw
 * lwIP TCP connection (https:// goes through client-side mbedTLS over the same
 * static arena as the server TLS), sends the request, and returns the status +
 * body in caller buffers. For webhooks, telemetry push, REST calls from the
 * device. The request builder + response parser are host-testable; the transport
 * is ESP32-only.
 */
#ifndef DWS_ENABLE_HTTP_CLIENT
#define DWS_ENABLE_HTTP_CLIENT 0
#endif

/** @brief HTTPS client support inside the HTTP client (needs DWS_ENABLE_TLS). */
#ifndef DWS_ENABLE_HTTP_CLIENT_TLS
#define DWS_ENABLE_HTTP_CLIENT_TLS 0
#endif

/** @brief Receive buffer (and max response size) for the outbound HTTP client, bytes. */
#ifndef DWS_HTTP_CLIENT_BUF_SIZE
#define DWS_HTTP_CLIENT_BUF_SIZE 2048
#endif

/**
 * @brief Ciphertext receive-ring size for the https:// client, bytes.
 *
 * The lwIP recv callback feeds TLS wire bytes into this draining ring while the
 * TLS engine pulls and decrypts them, so it holds only the in-flight (not yet
 * decrypted) ciphertext: a multi-KB handshake flight fits without loss thanks to
 * the refuse-and-redeliver backpressure. Must exceed one TCP segment (TCP_MSS,
 * ~1460) or a full segment could never fit. Only used when
 * DWS_ENABLE_HTTP_CLIENT_TLS is set.
 */
#ifndef DWS_HTTP_CLIENT_CT_BUF_SIZE
#define DWS_HTTP_CLIENT_CT_BUF_SIZE 4096
#endif

/** @brief Outbound HTTP client connect/response timeout in milliseconds. */
#ifndef DWS_HTTP_CLIENT_TIMEOUT_MS
#define DWS_HTTP_CLIENT_TIMEOUT_MS 8000
#endif

/**
 * @brief Outbound SMTP client (RFC 5321) for device email alerts (services/smtp).
 *
 * A blocking one-shot `smtp_send()`: EHLO, optional AUTH LOGIN, MAIL FROM / RCPT TO /
 * DATA over the shared client transport (`dws_client`), with implicit TLS (SMTPS, e.g.
 * :465) when the message config sets `tls` and DWS_ENABLE_TLS is on. Zero heap; the
 * dialogue engine (`smtp_run`) takes a send/recv seam so it is host-tested without lwIP.
 * "SMS fallback" rides on top - most carriers accept an email-to-SMS gateway address.
 */
#ifndef DWS_ENABLE_SMTP
#define DWS_ENABLE_SMTP 0
#endif

/** @brief Max length of one SMTP command / address line (bytes, incl. CRLF). */
#ifndef DWS_SMTP_LINE_MAX
#define DWS_SMTP_LINE_MAX 256
#endif

/** @brief Max size of the assembled DATA payload (headers + dot-stuffed body), bytes. */
#ifndef DWS_SMTP_MSG_MAX
#define DWS_SMTP_MSG_MAX 2048
#endif

/** @brief Max size of one (possibly multi-line) server reply held while parsing, bytes. */
#ifndef DWS_SMTP_REPLY_MAX
#define DWS_SMTP_REPLY_MAX 512
#endif

/** @brief SMTP connect / per-reply timeout in milliseconds. */
#ifndef DWS_SMTP_TIMEOUT_MS
#define DWS_SMTP_TIMEOUT_MS 10000
#endif

/** @brief Ciphertext receive-ring size for SMTPS, bytes (only used when the message is TLS). */
#ifndef DWS_SMTP_CT_BUF_SIZE
#define DWS_SMTP_CT_BUF_SIZE 4096
#endif

/**
 * @brief MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS).
 *
 * Default off. When set, src/services/mqtt/mqtt.h provides a persistent outbound
 * client: connect to a broker, PUBLISH (QoS 0/1/2) and SUBSCRIBE to topics, receive
 * incoming messages via a callback, with keep-alive pings - the dominant IoT
 * messaging pattern, for telemetry push and remote command. The packet codec is
 * host-testable; the transport (DNS + raw lwIP TCP, MQTTS via client-side mbedTLS)
 * is ESP32-only. Full QoS 0/1/2 (outbound DUP retransmit, inbound QoS-2
 * de-duplication by packet id) and Last-Will are supported.
 */
#ifndef DWS_ENABLE_MQTT
#define DWS_ENABLE_MQTT 0
#endif

/** @brief MQTTS: run the MQTT client over client-side TLS (needs DWS_ENABLE_TLS). */
#ifndef DWS_ENABLE_MQTT_TLS
#define DWS_ENABLE_MQTT_TLS 0
#endif

/**
 * @brief MQTT packet buffer size in bytes (bounds one outgoing/incoming packet).
 *
 * Two buffers of this size live in BSS (one tx, one rx). Must hold the largest
 * CONNECT/PUBLISH the client sends and the largest incoming PUBLISH it accepts
 * (topic + payload + a few header bytes); larger incoming packets are dropped.
 */
#ifndef DWS_MQTT_BUF_SIZE
#define DWS_MQTT_BUF_SIZE 1024
#endif

/** @brief Default MQTT keep-alive interval in seconds (PINGREQ cadence / CONNECT field). */
#ifndef DWS_MQTT_KEEPALIVE_S
#define DWS_MQTT_KEEPALIVE_S 30
#endif

/** @brief Ciphertext receive-ring size for MQTTS (draining ring; must exceed one TCP_MSS). */
#ifndef DWS_MQTT_CT_BUF_SIZE
#define DWS_MQTT_CT_BUF_SIZE 4096
#endif

/** @brief Maximum inbound MQTT topic length (including NUL) delivered to the callback. */
#ifndef DWS_MQTT_MAX_TOPIC
#define DWS_MQTT_MAX_TOPIC 128
#endif

/**
 * @brief Outbound QoS 1/2 in-flight slots (unacknowledged messages held for DUP retransmit).
 *
 * Each slot stores its serialized packet (up to DWS_MQTT_INFLIGHT_BUF bytes) until
 * the broker acknowledges it; a publish is refused when all slots are busy. The pool
 * costs DWS_MQTT_MAX_INFLIGHT * (DWS_MQTT_INFLIGHT_BUF + a few bytes) of BSS.
 */
#ifndef DWS_MQTT_MAX_INFLIGHT
#define DWS_MQTT_MAX_INFLIGHT 4
#endif

/** @brief Stored-packet size per in-flight QoS 1/2 slot (caps a retransmittable PUBLISH). */
#ifndef DWS_MQTT_INFLIGHT_BUF
#define DWS_MQTT_INFLIGHT_BUF 256
#endif

/** @brief Retransmit timeout (ms) for an unacknowledged in-flight QoS 1/2 message. */
#ifndef DWS_MQTT_RETRANSMIT_MS
#define DWS_MQTT_RETRANSMIT_MS 5000
#endif

/** @brief Inbound QoS 2 packet-id de-duplication ring depth (PUBREC-acknowledged, awaiting PUBREL). */
#ifndef DWS_MQTT_RX_QOS2_SLOTS
#define DWS_MQTT_RX_QOS2_SLOTS 8
#endif

/**
 * @brief Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS).
 *
 * Default off. When set, src/services/ws_client/ws_client.h connects to a remote
 * WebSocket endpoint (ws://, or wss:// over client-side mbedTLS), performs the
 * RFC 6455 client handshake (Sec-WebSocket-Key/Accept), and sends masked text /
 * binary frames + receives server frames via a callback - for streaming to cloud
 * dashboards or bidirectional control. The frame/handshake codec is host-testable.
 */
#ifndef DWS_ENABLE_WS_CLIENT
#define DWS_ENABLE_WS_CLIENT 0
#endif

/** @brief wss://: run the WebSocket client over client-side TLS (needs DWS_ENABLE_TLS). */
#ifndef DWS_ENABLE_WS_CLIENT_TLS
#define DWS_ENABLE_WS_CLIENT_TLS 0
#endif

/** @brief WebSocket client send/receive buffer size in bytes (bounds one frame). */
#ifndef DWS_WS_CLIENT_BUF_SIZE
#define DWS_WS_CLIENT_BUF_SIZE 1024
#endif

/** @brief Ciphertext receive-ring size for wss:// (draining ring; must exceed one TCP_MSS). */
#ifndef DWS_WS_CLIENT_CT_BUF_SIZE
#define DWS_WS_CLIENT_CT_BUF_SIZE 4096
#endif

/**
 * @brief Internal: client-side TLS engine is compiled (HTTPS client, MQTTS, wss client, and/or a TLS edge-cache
 * origin).
 *
 * The outbound HTTP client (one-shot exchange) and the MQTT / WebSocket clients
 * and the edge cache's TLS origin fetch (persistent sessions) share the same
 * client mbedTLS code in dws_tls - the CA/pin trust config, the BIO typedefs,
 * and the session API - gated by this.
 */
#if DWS_ENABLE_HTTP_CLIENT_TLS || DWS_ENABLE_MQTT_TLS || DWS_ENABLE_WS_CLIENT_TLS || DWS_ENABLE_EDGE_ORIGIN_TLS
#define DWS_ENABLE_CLIENT_TLS 1
#else
#define DWS_ENABLE_CLIENT_TLS 0
#endif

// The outbound clients (dws_client) resolve hostnames through the shared DNS
// resolver (dws_dns_resolver_resolve), so enabling any client implies the resolver - one
// owner of the gethostbyname-marshal pattern instead of a private copy per client.
// DWS_NEED_DET_CLIENT marks when the client transport is actually used; the
// dws_client translation unit compiles its body only then (a server-only Arduino
// build that does not enable a client must not reference the resolver symbols).
// Every feature that drives the outbound client transport must pull it in: the direct callers
// (http_client / mqtt / ws_client / relay / smtp / ssh port-forward) and the seam-based engines
// whose shipped example binds the seam to dws_client (smb / dnc). Miss one and its dws_client_open
// resolves to the !NEED stub that returns -1, so the feature silently never connects on device.
#if DWS_ENABLE_HTTP_CLIENT || DWS_ENABLE_MQTT || DWS_ENABLE_WS_CLIENT || DWS_ENABLE_RELAY || DWS_ENABLE_SMTP ||        \
    DWS_SSH_PORT_FORWARD || DWS_ENABLE_SMB || DWS_ENABLE_DNC
#undef DWS_ENABLE_DNS_RESOLVER
#define DWS_ENABLE_DNS_RESOLVER 1
#define DWS_NEED_DET_CLIENT 1
#endif
#ifndef DWS_NEED_DET_CLIENT
#define DWS_NEED_DET_CLIENT 0
#endif

// ---------------------------------------------------------------------------
// Full Authorization-header capture (internal)
// ---------------------------------------------------------------------------
// Digest auth and JWT bearer tokens both carry an Authorization value far longer
// than MAX_VAL_LEN, so the parser captures the whole header into a dedicated
// per-request buffer (HttpReq::authorization) when either feature is enabled.

/** @brief True when the parser must capture the full Authorization header value. */
#if DWS_ENABLE_AUTH || DWS_ENABLE_JWT || DWS_ENABLE_OIDC
#define DWS_CAPTURE_AUTH_HEADER 1
#else
#define DWS_CAPTURE_AUTH_HEADER 0
#endif

/**
 * @brief Capacity of HttpReq::authorization (full Authorization header value).
 *
 * Sized to the largest enabled consumer: a Digest header (DIGEST_AUTH_HDR_MAX), a
 * `Bearer <jwt>` HS256 token (DWS_JWT_MAX_LEN), or a `Bearer <id_token>` OIDC
 * RS256 token (DWS_OIDC_MAX_LEN), each plus the scheme.
 */
#if DWS_ENABLE_OIDC
#define DWS_AUTH_HDR_CAP_OIDC (DWS_OIDC_MAX_LEN + 16)
#else
#define DWS_AUTH_HDR_CAP_OIDC 0
#endif
#if DWS_ENABLE_JWT
#define DWS_AUTH_HDR_CAP_JWT (DWS_JWT_MAX_LEN + 16)
#else
#define DWS_AUTH_HDR_CAP_JWT 0
#endif
#define DWS_AUTH_HDR_CAP_M1 (DWS_AUTH_HDR_CAP_JWT > DIGEST_AUTH_HDR_MAX ? DWS_AUTH_HDR_CAP_JWT : DIGEST_AUTH_HDR_MAX)
#define DWS_AUTH_HDR_CAP (DWS_AUTH_HDR_CAP_OIDC > DWS_AUTH_HDR_CAP_M1 ? DWS_AUTH_HDR_CAP_OIDC : DWS_AUTH_HDR_CAP_M1)

/** @brief Runtime stats endpoint (uptime, request/error counts, pool usage, heap). */
#ifndef DWS_ENABLE_STATS
#define DWS_ENABLE_STATS 0
#endif

/**
 * @brief Transport-layer observability: connection event hook + counters.
 *
 * Default off (zero cost when unset - the notify points compile to nothing).
 * When set, the transport (L4) fires an application callback on every connection
 * state transition - dws_conn_on_event(slot, old_state, new_state, reason) - and
 * maintains lock-free counters (accepts, closes by reason, idle timeouts, RX
 * backpressure events, dropped deferred events, and a live ConnState::CONN_CLOSING gauge)
 * readable via dws_conn_counters(). This is the only state-transition trace the
 * L4/L5 core exposes; pair it with DWS_ENABLE_STATS for request-level metrics.
 */
#ifndef DWS_ENABLE_OBSERVABILITY
#define DWS_ENABLE_OBSERVABILITY 0
#endif

/**
 * @brief Prometheus `/metrics` endpoint (text exposition format 0.0.4).
 *
 * Default off (requires DWS_ENABLE_STATS for the underlying counters). When
 * set, DWS::metrics() emits the runtime stats as Prometheus metrics
 * (`dws_uptime_seconds`, `dws_http_requests_total`,
 * `dws_http_responses_total{class=...}`, `dws_active_connections`,
 * `dws_free_heap_bytes`, ...) so a Prometheus server can scrape the device.
 */
#ifndef DWS_ENABLE_METRICS
#define DWS_ENABLE_METRICS 0
#endif

/**
 * @brief Browser "web serial" terminal over WebSocket (src/services/web_terminal).
 *
 * Serves a self-contained terminal page and a WebSocket endpoint: device output
 * is broadcast to all connected browsers, browser input is delivered to a
 * command callback. Requires DWS_ENABLE_WEBSOCKET. Default off.
 */
#ifndef DWS_ENABLE_WEB_TERMINAL
#define DWS_ENABLE_WEB_TERMINAL 0
#endif

/**
 * @brief Stack scratch for dws_web_terminal_printf()/println() formatting.
 *
 * One formatted terminal line must fit in this many bytes (longer is truncated).
 * Allocated on the stack only during the call - no persistent RAM cost.
 */
#ifndef TERM_TX_BUF_SIZE
#define TERM_TX_BUF_SIZE 256
#endif

/**
 * @brief Conditional GET (ETag + Last-Modified) for served files.
 *
 * When set, serve_file()/serve_static() emit a strong `ETag` (from file size +
 * mtime) and a `Last-Modified` date, and answer a conditional request with
 * `304 Not Modified` when either the client's `If-None-Match` matches the ETag or
 * - per RFC 9110, only if no `If-None-Match` is present - its `If-Modified-Since`
 * is not older than the file. Saves bandwidth on repeat fetches of static assets.
 * (If-Modified-Since needs a real wall clock for the file mtime; with no clock the
 * date validator is skipped and the ETag validator still works.)
 */
#ifndef DWS_ENABLE_ETAG
#define DWS_ENABLE_ETAG 0
#endif

/**
 * @brief Expose a diagnostic JSON endpoint via server.diag().
 *
 * Disabled by default - enabling it exposes compile-time configuration
 * (buffer sizes, feature flags) which could aid an attacker.  Only
 * enable in development or behind an authenticated route.
 *
 * When enabled, DWS_DIAG_JSON is a compile-time string constant you can
 * serve from any route handler:
 * @code
 *   server.on("/diag", HttpMethod::HTTP_GET, [](uint8_t id, HttpReq *) {
 *       server.diag(id);        // convenience wrapper
 *       // or:
 *       server.send(id, 200, "application/json", DWS_DIAG_JSON);
 *   });
 * @endcode
 */
#ifndef DWS_ENABLE_DIAG
#define DWS_ENABLE_DIAG 0
#endif

/**
 * @brief HTTP/1.1 persistent connections (keep-alive).
 *
 * Default off (every response carries `Connection: close` and the connection is
 * closed after one request - the long-standing behavior). When set to 1, a
 * cleanly-parsed request is answered with `Connection: keep-alive` and the slot
 * is recycled for the next request on the same socket: HTTP/1.1 keeps the
 * connection open unless the client sends `Connection: close`; HTTP/1.0 closes
 * unless the client sends `Connection: keep-alive`. Error responses (400/413/414
 * and any non-ParseState::PARSE_COMPLETE path) always close, since the next request boundary
 * is unknown. Idle keep-alive connections are still reclaimed by the existing
 * conn_timeout sweep, and each connection serves at most
 * DWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.
 */
#ifndef DWS_ENABLE_KEEPALIVE
#define DWS_ENABLE_KEEPALIVE 1
#endif

/**
 * @brief Maximum requests served on one keep-alive connection before it is closed.
 *
 * A fairness bound so a single client cannot hold a connection slot
 * indefinitely with a steady request stream. After this many responses the
 * server emits `Connection: close` and drops the link; the client simply
 * reconnects. Only meaningful when DWS_ENABLE_KEEPALIVE is set.
 */
#ifndef DWS_KEEPALIVE_MAX_REQUESTS
#define DWS_KEEPALIVE_MAX_REQUESTS 100
#endif

/**
 * @brief HTTP/2 (RFC 9113) over the version-agnostic request/response core.
 *
 * Default off. When set, the server negotiates HTTP/2 via TLS ALPN ("h2") and speaks the binary
 * framing + HPACK header compression (RFC 7541) on top of the same routes/handlers as HTTP/1.1
 * (the response serializer is version-neutral). The HPACK codec and the frame layer are pure and
 * host-tested; the connection/stream state machine plugs in as a ProtoHandler.
 */
#ifndef DWS_ENABLE_HTTP2
#define DWS_ENABLE_HTTP2 0
#endif

/**
 * @brief Per-connection HPACK dynamic-table size in bytes (our decoder; advertised to the peer
 * as SETTINGS_HEADER_TABLE_SIZE). RFC 7541's default is 4096; lower it to save per-connection
 * RAM (each active HTTP/2 connection holds one table).
 */
#ifndef DWS_HPACK_TABLE_BYTES
#define DWS_HPACK_TABLE_BYTES 4096
#endif

/** @brief Max HPACK dynamic-table entries (>= DWS_HPACK_TABLE_BYTES / 32, the min entry size). */
#ifndef DWS_HPACK_MAX_ENTRIES
#define DWS_HPACK_MAX_ENTRIES 128
#endif

/**
 * @brief Largest HTTP/2 frame we accept, in bytes (advertised as SETTINGS_MAX_FRAME_SIZE). RFC
 * 9113 requires accepting at least 16384; a whole frame is buffered for reassembly, so this
 * (plus the HPACK table) sets the per-HTTP/2-connection RAM. Range: [16384, 16777215].
 */
#ifndef DWS_H2_MAX_FRAME
#define DWS_H2_MAX_FRAME 16384
#endif

/** @brief Max concurrent HTTP/2 streams per connection (advertised as MAX_CONCURRENT_STREAMS). */
#ifndef DWS_H2_MAX_STREAMS
#define DWS_H2_MAX_STREAMS 8
#endif

/**
 * @brief Header-block reassembly buffer for HTTP/2 requests that span HEADERS + CONTINUATION
 * frames (a single END_HEADERS frame decodes in place and needs no copy). Caps the compressed
 * request-header size; a larger block is rejected (RFC 9113 sec 6.10).
 */
#ifndef DWS_H2_HDR_BLOCK
#define DWS_H2_HDR_BLOCK 4096
#endif

/**
 * @brief Place the HTTP/2 connection-engine pool in external PSRAM (ESP32).
 *
 * Each HTTP/2 connection needs a ~28 KB engine, so the pool (MAX_CONNS of them) does not fit the
 * ~122 KB internal DRAM alongside a TLS server - HTTP/2 therefore requires PSRAM. Set this to 1
 * on a PSRAM board (S3 / P4 / WROVER) to move the pool to external RAM via `EXT_RAM_BSS_ATTR`.
 * Like DWS_TLS_ARENA_IN_PSRAM it needs a framework built with
 * `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y` (the stock arduino-esp32 core ships it off; see
 * tools/psram/README.md). A compile-time guard rejects DWS_ENABLE_HTTP2 without this on ARDUINO.
 */
#ifndef DWS_H2_POOL_IN_PSRAM
#define DWS_H2_POOL_IN_PSRAM 0
#endif

/**
 * @brief HTTP/3 (RFC 9114) over QUIC (RFC 9000) - implemented, host-tested end-to-end (HW verification pending).
 *
 * Default off. HTTP/3 runs over QUIC (a reliable transport over UDP) with QPACK (RFC 9204)
 * header compression and its own binary framing. The full stack is in place and exercised by a
 * host end-to-end test - the QUIC variable-length integer (RFC 9000 sec 16), packet protection +
 * framing, the TLS 1.3-in-QUIC handshake, the transport connection engine, and the HTTP/3 + QPACK
 * codecs. On-device (ESP32) HW verification and an example are still pending. Like HTTP/2 this is a
 * PSRAM-class feature.
 */
#ifndef DWS_ENABLE_HTTP3
#define DWS_ENABLE_HTTP3 0
#endif

/**
 * @brief DTLS 1.3 datagram security (RFC 9147) - the record layer.
 *
 * DTLS 1.3 secures datagram (UDP) transports - CoAP-over-DTLS and other constrained-device
 * telemetry - reusing the hand-rolled TLS 1.3 handshake crypto that already backs HTTP/3. This
 * flag gates the DTLS 1.3 **record layer** (dws_dtls_record): the DTLSCiphertext unified header,
 * per-record AEAD protection (AEAD_AES_128_GCM), the RFC 9147 sequence-number encryption,
 * sequence-number reconstruction, and the anti-replay window; the **handshake framing and
 * reliability** layer (dws_dtls_handshake, RFC 9147 §5 + §7): the 12-byte handshake header,
 * overlap-tolerant message reassembly, the ACK message, and the stateless HelloRetryRequest
 * cookie; and the **server handshake state machine** (dws_dtls_conn, RFC 9147 §5-6): the
 * one-round-trip full handshake (TLS_AES_128_GCM_SHA256 / X25519 / Ed25519), epoch 0->2->3
 * transitions, reusing the TLS 1.3 messages + key schedule (dws_tls13_msg / dws_tls13_kdf). The
 * HelloRetryRequest cookie round-trip and ACK/timeout retransmission, plus a CoAPs front-end, are
 * the following phases. Enabling this also compiles the shared dws_quic_hkdf / dws_quic_aead / dws_tls13_*
 * primitives (otherwise gated behind HTTP/3). Default off.
 */
#ifndef DWS_ENABLE_DTLS
#define DWS_ENABLE_DTLS 0
#endif

// Internal request-dispatch slots appended to the connection pool for non-TCP transports.
// HTTP/3 runs over QUIC/UDP and has no accept-time TCP slot, but it reuses the same request
// pipeline (match_and_execute + send), which is indexed by a connection-pool slot. One reserved
// slot at index MAX_CONNS lets an HTTP/3 request run through that pipeline. The TCP accept path only
// ever scans [0, MAX_CONNS), and this slot is driven synchronously by the HTTP/3 poll on the worker
// thread, so there is no accept race. CONN_POOL_SLOTS sizes conn_pool / http_pool / the per-slot
// response-header buffer; every TCP loop still bounds itself with MAX_CONNS.
#if DWS_ENABLE_HTTP3
#define DWS_INTERNAL_SLOTS 1
#define DWS_H3_DISPATCH_SLOT MAX_CONNS ///< reserved conn-pool slot an HTTP/3 request dispatches through
#else
#define DWS_INTERNAL_SLOTS 0
#endif
#define CONN_POOL_SLOTS (MAX_CONNS + DWS_INTERNAL_SLOTS)

/** @brief UDP port the HTTP/3 (QUIC) server binds by default (used by DWS::dws_h3_cert). */
#ifndef DWS_HTTP3_PORT
#define DWS_HTTP3_PORT 443
#endif

/**
 * @brief Maximum bytes of one QUIC/TLS handshake CRYPTO flight (RFC 9001).
 *
 * The server's second flight - EncryptedExtensions + Certificate + CertificateVerify + Finished -
 * is assembled whole before it is fragmented into CRYPTO frames across Handshake packets. The
 * Certificate (a DER X.509 chain) dominates the size, so this bounds the certificate the server can
 * present. The default fits a single Ed25519 leaf certificate comfortably; raise it for a chain.
 */
#ifndef DWS_H3_CRYPTO_BUF
#define DWS_H3_CRYPTO_BUF 2048
#endif

/**
 * @brief Maximum concurrent request streams per HTTP/3 connection.
 *
 * Bounds the per-connection QUIC stream table (client-initiated bidirectional request streams plus
 * the handful of unidirectional control / QPACK streams). Each slot is small; 8 matches the HTTP/2
 * default (DWS_H2_MAX_STREAMS).
 */
#ifndef DWS_H3_MAX_STREAMS
#define DWS_H3_MAX_STREAMS 8
#endif

/**
 * @brief HTTP Range requests / 206 Partial Content (requires DWS_ENABLE_FILE_SERVING or
 *        DWS_ENABLE_EDGE_CACHE).
 *
 * Default off. When set, serve_file() / serve_static() and the CDN edge cache honor a single-range
 * `Range: bytes=...` request header: they answer `206 Partial Content` with a `Content-Range` header
 * and stream only the requested bytes (file serving seeks the file; the edge cache windows the cached
 * body), advertise `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range with
 * `416 Range Not Satisfiable`. This enables resumable downloads and media seeking. Multi-range
 * (multipart/byteranges) requests are not supported - the server falls back to a full 200 response,
 * which is RFC 7233 §3.1 compliant. The parser is shared (server/http_range.h).
 */
#ifndef DWS_ENABLE_RANGE
#define DWS_ENABLE_RANGE 0
#endif

/**
 * @brief Enforce the RFC 7230 §5.4 Host-header requirement (default on).
 *
 * When 1, an HTTP/1.1 request that lacks a Host header - or carries more than
 * one - is rejected with 400 Bad Request. When 0, the Host header is not
 * required (useful for constrained clients or test harnesses that feed bare
 * request lines). The multiple-Host rule and Content-Length validation are
 * always active regardless of this flag.
 */
#ifndef DWS_ENFORCE_HOST_HEADER
#define DWS_ENFORCE_HOST_HEADER 1
#endif

/**
 * @brief Allow SSH password authentication (default on).
 *
 * Set to 0 to harden the SSH server to publickey-only authentication
 * (RFC 4252 §7): the "password" method is then refused outright and is not
 * advertised in the USERAUTH_FAILURE method list. Publickey auth is always
 * available regardless of this flag.
 */
#ifndef DWS_SSH_ALLOW_PASSWORD
#define DWS_SSH_ALLOW_PASSWORD 1
#endif

/**
 * @brief Maximum failed SSH authentication attempts per connection.
 *
 * RFC 4252 §4 permits the server to disconnect after a small bounded number of
 * failed USERAUTH_REQUESTs. After this many SSH_MSG_USERAUTH_FAILURE responses
 * on one connection the server sends SSH_MSG_DISCONNECT and drops the link.
 * (The publickey "would-be-accepted" probe and a SUCCESS do not count.)
 */
#ifndef SSH_MAX_AUTH_ATTEMPTS
#define SSH_MAX_AUTH_ATTEMPTS 6
#endif

// ---------------------------------------------------------------------------
// Listener pool
// ---------------------------------------------------------------------------

/** @brief Maximum number of simultaneously active listener ports. */
#ifndef MAX_LISTENERS
#define MAX_LISTENERS 3
#endif

/**
 * @brief Maximum simultaneously bound UDP ports (transport-layer UDP service).
 *
 * Sizes the fixed pool in udp.cpp. One slot per bound port, e.g. SNMP
 * (:161) and the captive-portal DNS responder (:53). Costs only a few pointers
 * of BSS each.
 */
#ifndef DWS_MAX_UDP_LISTENERS
#define DWS_MAX_UDP_LISTENERS 2
#endif

/**
 * @brief Shared receive-scratch size for the transport-layer UDP service.
 *
 * One static buffer (lwIP delivers a single datagram at a time) into which each
 * incoming datagram is copied before the handler runs. Must hold the largest
 * datagram any UDP service expects (SNMP messages are the largest user).
 */
#ifndef DWS_UDP_RX_BUF_SIZE
#define DWS_UDP_RX_BUF_SIZE 1472
#endif

/**
 * @brief Opt-in global accept-rate throttle (connection-flood defense).
 *
 * Default off (zero cost / no behavior change). When set to 1 the accept
 * callback rejects new connections once more than DWS_ACCEPT_THROTTLE_MAX
 * have been accepted within a DWS_ACCEPT_THROTTLE_WINDOW_MS fixed window
 * (global across all listeners, two static counters - no per-IP table). This
 * bounds connection churn (e.g. reconnect brute-force) on top of the bounded
 * connection pool and the per-connection auth limits. mitigate finer-grained /
 * per-IP attacks at the network layer.
 */
#ifndef DWS_ENABLE_ACCEPT_THROTTLE
#define DWS_ENABLE_ACCEPT_THROTTLE 0
#endif

/** @brief Max accepted connections per throttle window (see DWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DWS_ACCEPT_THROTTLE_MAX
#define DWS_ACCEPT_THROTTLE_MAX 20
#endif

/** @brief Throttle window length in milliseconds (see DWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DWS_ACCEPT_THROTTLE_WINDOW_MS
#define DWS_ACCEPT_THROTTLE_WINDOW_MS 1000
#endif

/**
 * @brief Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4).
 *
 * Default off (zero cost / no behavior change). Complements the global accept
 * throttle: the accept callback rejects a new connection once one source IPv4
 * address has opened more than DWS_PER_IP_THROTTLE_MAX connections within a
 * DWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of
 * DWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source
 * addresses; when a new address arrives and the table is full, an expired or
 * least-recently-started bucket is reused, so memory stays bounded (no heap).
 *
 * This bounds reconnect/brute-force churn from a single host (the gap left by the
 * global throttle, which cannot tell one noisy client from many). It is
 * best-effort: an attacker spreading across many source addresses can still churn
 * the bounded connection pool, so combine it with the global throttle and
 * network-layer filtering.
 */
#ifndef DWS_ENABLE_PER_IP_THROTTLE
#define DWS_ENABLE_PER_IP_THROTTLE 0
#endif

/** @brief Number of source IPv4 addresses tracked by the per-IP throttle (BSS bucket table). */
#ifndef DWS_PER_IP_THROTTLE_SLOTS
#define DWS_PER_IP_THROTTLE_SLOTS 16
#endif

/** @brief Max accepted connections per window from one source IP (see DWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DWS_PER_IP_THROTTLE_MAX
#define DWS_PER_IP_THROTTLE_MAX 10
#endif

/** @brief Per-IP throttle window length in milliseconds (see DWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DWS_PER_IP_THROTTLE_WINDOW_MS
#define DWS_PER_IP_THROTTLE_WINDOW_MS 10000
#endif

// ---------------------------------------------------------------------------
// Source-IP allowlist  (accept-time firewall; DWS_ENABLE_IP_ALLOWLIST)
// ---------------------------------------------------------------------------

/**
 * @brief Opt-in source-IP allowlist (accept-time firewall, IPv4 and IPv6).
 *
 * Default off (zero cost / no behavior change). When set, the accept callback
 * drops any connection whose source address is not contained in a configured
 * CIDR rule (add rules with listener_ip_allow_add_cidr("192.168.1.0/24") /
 * "2001:db8::/32"). Matching is a full-address prefix compare per family, so a v4
 * peer never matches a v6 rule and vice versa. An empty allowlist allows
 * everything, so enabling the feature before adding rules never locks the device
 * out. Rules live in a fixed BSS table of DWS_IP_ALLOWLIST_SLOTS entries (no heap).
 *
 * This is a coarse first-line filter - a spoofed source address can still pass
 * it - so combine it with the accept throttles and network-layer filtering.
 */
#ifndef DWS_ENABLE_IP_ALLOWLIST
#define DWS_ENABLE_IP_ALLOWLIST 0
#endif

/** @brief Number of CIDR rules the source-IP allowlist can hold (BSS table). */
#ifndef DWS_IP_ALLOWLIST_SLOTS
#define DWS_IP_ALLOWLIST_SLOTS 8
#endif

// ---------------------------------------------------------------------------
// Brute-force auth lockout  (per-source-IP; DWS_ENABLE_AUTH_LOCKOUT)
// ---------------------------------------------------------------------------

/**
 * @brief Opt-in per-IP brute-force lockout for HTTP auth (requires DWS_ENABLE_AUTH).
 *
 * Default off (zero cost / no behavior change). When set, the auth gate counts
 * consecutive failed authentications per source address (IPv4 or IPv6, keyed on
 * the full address) in a fixed BSS table; after
 * DWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for
 * DWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to
 * DWS_AUTH_LOCKOUT_MAX_MS. A locked address gets 429 (Retry-After) with no
 * credential check; a successful auth clears it. Bounded memory (no heap); the
 * table evicts idle, then least-recently-used, addresses when full.
 */
#ifndef DWS_ENABLE_AUTH_LOCKOUT
#define DWS_ENABLE_AUTH_LOCKOUT 0
#endif

/** @brief Number of source IPs the auth lockout tracks (BSS bucket table). */
#ifndef DWS_AUTH_LOCKOUT_SLOTS
#define DWS_AUTH_LOCKOUT_SLOTS 16
#endif

/** @brief Consecutive failed auths from one IP before it is locked out. */
#ifndef DWS_AUTH_LOCKOUT_THRESHOLD
#define DWS_AUTH_LOCKOUT_THRESHOLD 5
#endif

/** @brief First lockout duration in ms; doubles on each further failure. */
#ifndef DWS_AUTH_LOCKOUT_BASE_MS
#define DWS_AUTH_LOCKOUT_BASE_MS 1000
#endif

/** @brief Maximum lockout duration in ms (the exponential backoff cap). */
#ifndef DWS_AUTH_LOCKOUT_MAX_MS
#define DWS_AUTH_LOCKOUT_MAX_MS 300000
#endif

// ---------------------------------------------------------------------------
// CSRF protection  (DWS_ENABLE_CSRF)
// ---------------------------------------------------------------------------

/**
 * @brief Opt-in CSRF protection for state-changing HTTP requests.
 *
 * Default off (zero cost / no behavior change). When set, every POST / PUT /
 * PATCH / DELETE must carry a valid `X-CSRF-Token` header (a stateless,
 * HMAC-signed token); requests without one get 403 Forbidden. GET / HEAD /
 * OPTIONS are exempt (they are not state-changing). Clients fetch a token from
 * the built-in `GET /csrf` endpoint, which also sets it as the `csrf` cookie.
 * No server-side session storage - the token self-validates against an HMAC
 * secret seeded from the hardware RNG at begin(); it is independent of
 * DWS_ENABLE_AUTH.
 */
#ifndef DWS_ENABLE_CSRF
#define DWS_ENABLE_CSRF 0
#endif

// ---------------------------------------------------------------------------
// Telnet sizing constants  (DWS_ENABLE_TELNET must be 1)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous Telnet connections. */
#ifndef MAX_TELNET_CONNS
#define MAX_TELNET_CONNS 2
#endif

/** @brief Stack buffer for one Telnet I/O chunk. */
#ifndef TELNET_BUF_SIZE
#define TELNET_BUF_SIZE 256
#endif

// ---------------------------------------------------------------------------
// SSH sizing constants  (DWS_ENABLE_SSH must be 1)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous SSH connections. */
#ifndef MAX_SSH_CONNS
#define MAX_SSH_CONNS 1
#endif

/**
 * @brief Maximum concurrent SSH channels per connection (RFC 4254 multiplexing).
 *
 * Default 1 - one "session" channel per connection, byte-for-byte the original
 * single-channel behavior. Raise it to multiplex several channels (e.g. several
 * concurrent shells/exec, or - with the forwarding build flags - tunnels) over one
 * SSH connection; each channel gets its own id, window, and peer state. Fixed BSS
 * (ssh_chan[MAX_SSH_CONNS][DWS_SSH_MAX_CHANNELS]), no heap.
 */
#ifndef DWS_SSH_MAX_CHANNELS
#define DWS_SSH_MAX_CHANNELS 1
#endif
#if DWS_SSH_MAX_CHANNELS < 1
#error "DeterministicESPAsyncWebServer: DWS_SSH_MAX_CHANNELS must be >= 1"
#endif

/**
 * @brief SSH TCP port forwarding (`direct-tcpip`, i.e. `ssh -L`). Default off.
 *
 * When set, the SSH server can open an outbound TCP connection to a client-named
 * host:port and bridge bytes between that socket and the SSH channel - the
 * `ssh_forward` owner does the I/O via the outbound client transport (dws_client),
 * so it needs `DWS_CLIENT_CONNS >= DWS_SSH_FWD_MAX` and a channel pool
 * (`DWS_SSH_MAX_CHANNELS > 1`) to be useful. Forwarding is still opt-in at
 * runtime: nothing is forwarded until the application calls `dws_ssh_forward_begin()`.
 * Off = the channel codec refuses every `direct-tcpip` open (no open relay).
 */
#ifndef DWS_SSH_PORT_FORWARD
#define DWS_SSH_PORT_FORWARD 0
#endif

/** @brief Maximum concurrent forwarded TCP connections (must be <= DWS_CLIENT_CONNS). */
#ifndef DWS_SSH_FWD_MAX
#define DWS_SSH_FWD_MAX 2
#endif

/** @brief Maximum forward target hostname length including null terminator. */
#ifndef DWS_SSH_FWD_HOST_MAX
#define DWS_SSH_FWD_HOST_MAX 64
#endif

/** @brief Blocking connect timeout (ms) when opening a forward target. */
#ifndef DWS_SSH_FWD_CONNECT_MS
#define DWS_SSH_FWD_CONNECT_MS 3000
#endif

/** @brief Max bytes moved per forward channel per poll, target -> client (<= SSH_PKT_BUF_SIZE). */
#ifndef DWS_SSH_FWD_CHUNK
#define DWS_SSH_FWD_CHUNK 1024
#endif

/**
 * @brief Maximum concurrent remote-forward listeners (`ssh -R` / `tcpip-forward`).
 *
 * Each accepted client that requests remote forwarding can bind up to this many
 * ports on the device; each binding consumes one `listener_pool[]` slot, so
 * `MAX_LISTENERS` must have that much headroom above the app's own listeners.
 * Remote forwarding shares `DWS_SSH_PORT_FORWARD` (compiled in) and is inert
 * until `dws_ssh_forward_begin()`.
 */
#ifndef DWS_SSH_RFWD_MAX
#define DWS_SSH_RFWD_MAX 1
#endif

/**
 * @brief Maximum concurrent bridged connections across all remote forwards.
 *
 * Each connection accepted on a forwarded port occupies one transport `conn_pool`
 * slot plus one SSH channel (so it needs `DWS_SSH_MAX_CHANNELS` headroom) and one
 * entry here while it is bridged back to the client.
 */
#ifndef DWS_SSH_RFWD_BRIDGE_MAX
#define DWS_SSH_RFWD_BRIDGE_MAX 2
#endif

/** @brief Packet assembly buffer per SSH connection (bytes). */
#ifndef SSH_PKT_BUF_SIZE
#define SSH_PKT_BUF_SIZE 2048
#endif

/**
 * @brief SFTP server subsystem over SSH (SSH_FXP_* v3, draft-ietf-secsh-filexfer-02). Default off.
 *
 * When set, an SSH client's `subsystem` request for "sftp" opens an SFTP session over the channel and the
 * device serves files from an fs::FS mount (SD / LittleFS) bound via dws_ssh_sftp_begin(fs, root):
 * open/read/write/opendir/readdir/stat/mkdir/rmdir/remove/rename/realpath, with a fixed handle table and
 * streamed reads/writes (zero heap). The standards-track southbound path for secure file / G-code push over
 * the one authenticated SSH port. Requires DWS_ENABLE_SSH + DWS_ENABLE_FILE_SERVING (the fs::FS seam).
 */
#ifndef DWS_ENABLE_SSH_SFTP
#define DWS_ENABLE_SSH_SFTP 0
#endif

/**
 * @brief SCP server over SSH (the legacy RCP protocol via `exec "scp -t/-f"`). Default off.
 *
 * When set, `scp file board:/path` (sink) and `scp board:/path file` (source) transfer a file to/from the
 * fs::FS mount bound by dws_ssh_sftp_begin. Reuses the SFTP fs binding + path-traversal guard. v1 carries its
 * error/ack bytes inline on the channel (no CHANNEL_EXTENDED_DATA). Requires SSH + FILE_SERVING.
 */
#ifndef DWS_ENABLE_SSH_SCP
#define DWS_ENABLE_SSH_SCP 0
#endif

/** @brief Max concurrent open SFTP handles (files + dirs) per SSH connection. */
#ifndef DWS_SFTP_MAX_HANDLES
#define DWS_SFTP_MAX_HANDLES 4
#endif

/** @brief SFTP packet-assembly buffer per SFTP channel (bytes); bounds one non-streamed request/response. */
#ifndef DWS_SFTP_PKT_BUF
#define DWS_SFTP_PKT_BUF 2048
#endif

/**
 * @brief Largest SSH_FXP_DATA payload returned for one READ (a short read - the client re-requests). Kept
 *        within one SSH packet (SSH_PKT_BUF_SIZE minus framing), so bump SSH_PKT_BUF_SIZE too for throughput.
 */
#ifndef DWS_SFTP_MAX_READ
#define DWS_SFTP_MAX_READ 1024
#endif

/** @brief Largest absolute path the SFTP/SCP server resolves (mount root + request path). */
#ifndef DWS_SFTP_PATH_MAX
#define DWS_SFTP_PATH_MAX 256
#endif

/**
 * @brief SSH server-to-client compression (`zlib@openssh.com` / `zlib`, RFC 4253 sec 6.2). Default off.
 *
 * When set, the server advertises `zlib@openssh.com` (delayed, OpenSSH's default) and `zlib` for the
 * SERVER->CLIENT direction and, once active, compresses every outbound packet payload with a
 * context-takeover DEFLATE stream (a persistent sliding window carried across packets, sync-flushed
 * per packet - RFC 1951 / RFC 1950). Client->server stays `none`: SSH negotiates each direction
 * independently, and the inbound direction (keystrokes / uploads to the device) is tiny and, because
 * OpenSSH compresses outbound with Z_PARTIAL_FLUSH, would need a far larger resumable inflate engine
 * for little gain. `ssh -o Compression=yes` still gets real compression on the high-volume direction.
 *
 * PSRAM-class: each connection holds a compressor (~window + hash tables, tens of KB). A compile-time
 * guard rejects this on ARDUINO without DWS_SSH_ZLIB_IN_PSRAM. Requires DWS_ENABLE_SSH.
 */
#ifndef DWS_ENABLE_SSH_ZLIB
#define DWS_ENABLE_SSH_ZLIB 0
#endif

/**
 * @brief Place the per-connection SSH compression state in external PSRAM (ESP32).
 *
 * Like DWS_H2_POOL_IN_PSRAM / DWS_TLS_ARENA_IN_PSRAM: moves the compressor pool
 * (MAX_SSH_CONNS of them) to external RAM via `EXT_RAM_BSS_ATTR`. Needs a framework built with
 * `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y` (tools/psram/README.md).
 */
#ifndef DWS_SSH_ZLIB_IN_PSRAM
#define DWS_SSH_ZLIB_IN_PSRAM 0
#endif

/**
 * @brief Acknowledge placing the SSH compressor in internal DRAM (no PSRAM).
 *
 * The per-connection compressor is ~48 KB. With MAX_SSH_CONNS=1 and no TLS server it fits internal
 * DRAM on a roomy chip (S3 / P4). Rather than force PSRAM, this mirrors DWS_TLS_ACK_MULTI_CONN_DRAM:
 * set it to 1 to consciously accept the internal-DRAM cost when DWS_SSH_ZLIB_IN_PSRAM is off. The
 * build otherwise fails fast on ARDUINO with guidance (below) instead of a raw linker overflow.
 */
#ifndef DWS_SSH_ZLIB_ACK_DRAM
#define DWS_SSH_ZLIB_ACK_DRAM 0
#endif

/**
 * @brief SSH s2c DEFLATE sliding-window size in bytes (max back-reference distance). Power of two,
 * 256..32768. Larger = better ratio + more per-connection RAM (the compressor holds a window-sized
 * work buffer + a window-sized hash chain). The client always allocates a 32 KB inflate window, so
 * any value here interoperates; 8 KB is a good ratio/RAM balance for terminal + command output.
 */
#ifndef DWS_SSH_ZLIB_WINDOW
#define DWS_SSH_ZLIB_WINDOW 8192
#endif

/**
 * @brief Largest uncompressed payload the s2c compressor accepts in one call (bytes). Outbound SSH
 * payloads are bounded by SSH_PKT_BUF_SIZE; this sizes the compressor's history+input work buffer.
 */
#ifndef DWS_SSH_ZLIB_MAX_IN
#define DWS_SSH_ZLIB_MAX_IN 2048
#endif

/** @brief Maximum SSH username length including null terminator. */
#ifndef SSH_MAX_USERNAME_LEN
#define SSH_MAX_USERNAME_LEN 32
#endif

/** @brief Maximum SSH password length including null terminator. */
#ifndef SSH_MAX_PASSWORD_LEN
#define SSH_MAX_PASSWORD_LEN 64
#endif

/**
 * @brief Shared scratch buffer for SSH big-number operations.
 *
 * Holds Montgomery multiplication temporaries and RSA padding workspace.
 * Must be large enough for the biggest single crypto operation:
 *
 *   DH expmod:
 *     base_mont  (SshBigNum = 256 B)
 *     result     (SshBigNum = 256 B)
 *     tmp        (SshBigNum = 256 B)
 *     mont_t     (uint32_t[129] = 516 B)
 *                                ─────
 *                                1284 B  → round up with margin → 1536 B
 *
 * The buffer is zeroed via volatile memset immediately after each operation.
 * Only one SSH KEX may run at a time (guaranteed by the single Arduino loop
 * task and MAX_SSH_CONNS synchronous handshake model).
 */
#ifndef SSH_CRYPTO_WORK_SIZE
#define SSH_CRYPTO_WORK_SIZE 1536
#endif

/**
 * @brief Size in bytes of the shared per-dispatch scratch arena.
 *
 * Codec / protocol handlers borrow transient working memory from this single BSS
 * arena (see network_drivers/session/scratch.h) instead of each feature owning a
 * dedicated buffer. The session layer empties it before every event dispatch, so
 * it only needs to hold the *peak concurrent* scratch of any one dispatch, not
 * the sum across features. Tune from the scratch_high_water() reading on a real
 * workload; an over-budget borrow fails closed (scratch_alloc returns nullptr).
 */
#ifndef DWS_SCRATCH_ARENA_SIZE
#define DWS_SCRATCH_ARENA_SIZE 8192
#endif

// ---------------------------------------------------------------------------
// Static RAM (BSS) usage table
// ---------------------------------------------------------------------------
//
// All library memory is in BSS - allocated at link time, zero-initialized by
// the C runtime, never heap-allocated after begin().  The table below shows
// the contribution of every feature at its default constant values.
//
// Sizes are for ESP32 (32-bit pointers, int = 4 B).  Where a size depends on
// a macro the formula is given so you can compute the impact of any change.
//
// ┌──────────────────────────────┬──────────────────────────────────────────────────────────────┬──────────┐
// │ Symbol / pool                │ Size formula                                                 │ Default  │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ TRANSPORT LAYER (always on)  │                                                              │          │
// │  conn_pool[MAX_CONNS]        │ MAX_CONNS × (RX_BUF_SIZE + 22)                              │  4 168 B │
// │  listener_pool[MAX_LISTENERS]│ MAX_LISTENERS × (StaticQueue_t≈48 + EVT_QUEUE_DEPTH×12 + 18)│    654 B │
// │  conn_timeout_ms             │ 4 B                                                          │      4 B │
// │  TRANSPORT SUBTOTAL          │                                                              │  4 826 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ HTTP PRESENTATION (always on)│                                                              │          │
// │  http_pool[MAX_CONNS]        │ MAX_CONNS × (MAX_PATH_LEN + MAX_QUERY_LEN                   │          │
// │                              │   + MAX_HEADERS×(MAX_KEY_LEN+MAX_VAL_LEN)                   │          │
// │                              │   + MAX_QUERY_PARAMS×(QUERY_KEY_LEN+QUERY_VAL_LEN)          │          │
// │                              │   + BODY_BUF_SIZE + 50)                                     │  6 668 B │
// │  HTTP SUBTOTAL               │                                                              │  6 668 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ WEBSOCKET (DWS_ENABLE_WEBSOCKET=1)                                                        │          │
// │  ws_pool[MAX_WS_CONNS]       │ MAX_WS_CONNS × (WS_FRAME_SIZE + 29)                         │  1 082 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSE (DWS_ENABLE_SSE=1)     │                                                              │          │
// │  dws_sse_pool[MAX_SSE_CONNS]     │ MAX_SSE_CONNS × (MAX_PATH_LEN + 3)                          │    134 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSH (DWS_ENABLE_SSH=1)     │                                                              │          │
// │  ssh_pool[MAX_SSH_CONNS]     │ MAX_SSH_CONNS × (SSH_PKT_BUF_SIZE + 22)                     │  2 070 B │
// │  ssh_keys[MAX_SSH_CONNS]     │ MAX_SSH_CONNS × (2×SshAesCtrCtx + 64)                       │          │
// │   └─ SshAesCtrCtx (native)   │   rk[60]=240 + counter[16] + keystream[16] + pos[1] = 273 B │    610 B │
// │   └─ SshAesCtrCtx (ARDUINO)  │   mbedtls_aes_context (≈284 B) + 33 B = ≈317 B per ctx     │    698 B │
// │  ssh_dh[MAX_SSH_CONNS]       │ MAX_SSH_CONNS × (3×SshBigNum[256] + H[32] + 1)              │    801 B │
// │  crypto_work[]               │ SSH_CRYPTO_WORK_SIZE (scratch, wiped after each use)         │  1 536 B │
// │  SSH SUBTOTAL                │                                                              │  5 017 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ GRAND TOTAL (all features)   │                                                              │ ≈18 KB   │
// └──────────────────────────────┴──────────────────────────────────────────────────────────────┴──────────┘
//
// ESP32 has 320 KB of SRAM; the library uses ~5–18 KB depending on features.
// Stack usage is separate; the largest frame is during SSH DH key exchange
// (~256 B for the SshBigNum private scalar on the call stack before it is
// zeroed by ssh_dh_finish()).
//
// SSH KEY MATERIAL IS NOT IN THE TABLE ABOVE intentionally:
//   - The RSA host private key is NEVER stored in any static array.  It is
//     loaded from NVS into a local stack frame at sign time, used once, then
//     explicitly zeroed (volatile memset) before the function returns.
//   - AES session keys and HMAC keys live in ssh_keys[] (above), which is a
//     separate BSS symbol from ssh_pool[].  Physical separation means a
//     buffer overflow in the packet receive path (ssh_pool[].pkt_buf) cannot
//     reach the key material without crossing a distinct linker symbol - a
//     significant barrier against heap/BSS spray attacks.
//   - The DH ephemeral private scalar y lives in ssh_dh[].y and is zeroed
//     immediately after the shared secret K is derived.
//   - crypto_work[] is zeroed via volatile memset after every use so that
//     bignum intermediates (including partial products that contain key
//     material) do not persist in memory.

// ---------------------------------------------------------------------------
// Runtime configuration struct
// ---------------------------------------------------------------------------

/**
 * @brief Runtime-tunable server parameters.
 *
 * Can be declared as `const PROGMEM` (flash) or as a mutable variable (RAM).
 * Pass a pointer to DWS::begin() or DeterministicAsyncTCP::init().
 */
struct WebServerConfig
{
    /** Milliseconds of inactivity before a connection is force-closed. */
    uint32_t conn_timeout_ms;
};

// ---------------------------------------------------------------------------
// Protocol identifier
// ---------------------------------------------------------------------------

/**
 * @brief Application protocol spoken on a listener port or connection slot.
 *
 * Stored in both Listener::proto and TcpConn::proto.  The session layer uses
 * this to route events to the correct protocol handler without branching on
 * port numbers.
 *
 * All values are always present regardless of feature flags - the enum is
 * part of the listener API.  Feature flags gate the implementation, not the
 * identifier.
 */
enum class ConnProto : uint8_t
{
    PROTO_NONE = 0,         ///< Unassigned slot.
    PROTO_HTTP = 1,         ///< HTTP/1.1 with optional WS and SSE upgrades.
    PROTO_TELNET = 2,       ///< Telnet (RFC 854).
    PROTO_SSH = 3,          ///< SSH (RFC 4253/4252/4254).
    PROTO_MODBUS = 4,       ///< Modbus TCP slave (Modbus Application Protocol).
    PROTO_OPCUA = 5,        ///< OPC UA Binary (UA-TCP) server.
    PROTO_SSH_RFWD = 6,     ///< SSH remote-forward listener (ssh -R): accepts bridge to a forwarded-tcpip channel.
    PROTO_RELAY = 7,        ///< TCP relay / DNAT (DWS_ENABLE_RELAY): bridge to an origin dws_client connection.
    PROTO_BRIDGE = 8,       ///< address:port -> hardware bus (DWS_ENABLE_IFACE_BRIDGE): UART/SPI/I2C device server.
    PROTO_NTRIP_CASTER = 9, ///< NTRIP caster (DWS_ENABLE_NTRIP_CASTER): serves RTCM3 corrections to rovers.
    PROTO_MESH = 10, ///< Edge-cache sibling link (DWS_ENABLE_EDGE_MESH): answers a peer's content-addressed query.
};

/**
 * @brief Network interface a connection arrived on (for per-route filtering).
 *
 * Stamped onto each TcpConn at accept time by comparing the connection's local
 * IP to the softAP IP (see DWS::set_ap_ip()). Used to gate routes to
 * the station or softAP interface only (DWS::on(..., DWSIface)).
 */
enum class DWSIface : uint8_t
{
    DETIFACE_ANY = 0, ///< Unknown / no filter (matches any interface).
    DETIFACE_STA = 1, ///< Station interface (joined to an AP / your LAN).
    DETIFACE_AP = 2,  ///< softAP interface (clients joined to the device).
    DETIFACE_ETH = 3, ///< Ethernet interface (wired PHY).
};

// ---------------------------------------------------------------------------
// Diagnostic JSON string  (only defined when DWS_ENABLE_DIAG == 1)
// ---------------------------------------------------------------------------
// DWS_DIAG_JSON is a compile-time string literal - zero runtime cost.
// Adjacent string literals are concatenated by the compiler; DWS_STR()
// stringifies an integer macro value without evaluating it twice.

#if DWS_ENABLE_DIAG

#define _DWS_STR_(x) #x
#define _DWS_STR(x) _DWS_STR_(x)

#if DWS_ENABLE_WEBSOCKET
#define _DWS_F_WS "true"
#else
#define _DWS_F_WS "false"
#endif

#if DWS_ENABLE_SSE
#define _DWS_F_SSE "true"
#else
#define _DWS_F_SSE "false"
#endif

#if DWS_ENABLE_MULTIPART
#define _DWS_F_MP "true"
#else
#define _DWS_F_MP "false"
#endif

#if DWS_ENABLE_FILE_SERVING
#define _DWS_F_FS "true"
#else
#define _DWS_F_FS "false"
#endif

#if DWS_ENABLE_AUTH
#define _DWS_F_AUTH "true"
#else
#define _DWS_F_AUTH "false"
#endif

// Every DWS_ENABLE_* defaults to 0/1, so a two-level token-paste stringifies each to "true"/"false" and
// the protocol keys below stay one line each (no 5-line #if per flag). Lets tooling (pentesting/dws_pentest.py)
// auto-detect which protocols are built and select the applicable attacks without a manual --all.
#define _DWS_BOOL_0 "false"
#define _DWS_BOOL_1 "true"
#define _DWS_FB2(v) _DWS_BOOL_##v
#define _DWS_FB(v) _DWS_FB2(v)

#define DWS_DIAG_JSON                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
    "{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "\"lib\":\"DeterministicESPAsyncWebServer\","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    "\"features\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"websocket\":" _DWS_F_WS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"sse\":" _DWS_F_SSE ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           \
    "\"multipart\":" _DWS_F_MP ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"file_serving\":" _DWS_F_FS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
    "\"auth\":" _DWS_F_AUTH ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
    "\"webdav\":" _DWS_FB(DWS_ENABLE_WEBDAV) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                             "\"coap\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                                                 DWS_ENABLE_COAP) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                  "\"snmp\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                      DWS_ENABLE_SNMP) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
                                                                                       "\"opcua\":" _DWS_FB(DWS_ENABLE_OPCUA) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                                                                                                                              "\"umati\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                  DWS_ENABLE_UMATI) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
                                                                                                                                                    "\"modbus\":" _DWS_FB(DWS_ENABLE_MODBUS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                             "\"mqtt\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                 DWS_ENABLE_MQTT) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                  "\"mtconnect\":" _DWS_FB(DWS_ENABLE_MTCONNECT) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                 "\"redis\":" _DWS_FB(DWS_ENABLE_REDIS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                                                                                                                                                                                        "\"ftp\":" _DWS_FB(DWS_ENABLE_FTP) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                           "\"smtp\":" _DWS_FB(DWS_ENABLE_SMTP) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                                                                                                                                "\"smb\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                                                                                                                                    DWS_ENABLE_SMB) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                                                                                                                                                                                    "\"syslog\":" _DWS_FB(DWS_ENABLE_SYSLOG) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                                                                                                             "\"dws_ntp_server\":" _DWS_FB(DWS_ENABLE_NTP_SERVER) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "\"dns_server\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      DWS_ENABLE_DNS_SERVER) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             "\"nats\":" _DWS_FB(DWS_ENABLE_NATS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "\"stomp\":" _DWS_FB(DWS_ENABLE_STOMP) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         "\"statsd\":" _DWS_FB(DWS_ENABLE_STATSD) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "\"jwt\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      DWS_ENABLE_JWT) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      "\"tls\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          DWS_ENABLE_TLS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          "\"http2\":" _DWS_FB(DWS_ENABLE_HTTP2) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 "\"http3\":" _DWS_FB(DWS_ENABLE_HTTP3) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        "\"ssh\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            DWS_ENABLE_SSH) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            "\"ws_deflate\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                DWS_ENABLE_WS_DEFLATE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       "\"range\":" _DWS_FB(DWS_ENABLE_RANGE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              "\"csrf\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  DWS_ENABLE_CSRF) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   "\"accept_throttle\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       DWS_ENABLE_ACCEPT_THROTTLE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   "\"per_ip_throttle\":" _DWS_FB(DWS_ENABLE_PER_IP_THROTTLE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              "\"auth_lockout\":" _DWS_FB(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  DWS_ENABLE_AUTH_LOCKOUT) "},"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           "\"config\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           "\"MAX_CONNS\":" _DWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               MAX_CONNS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          "\"RX_BUF_SIZE\":" _DWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              RX_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           "\"BODY_BUF_SIZE\":" _DWS_STR(BODY_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        "\"MAX_ROUTES\":" _DWS_STR(                                                                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            MAX_ROUTES) ","                                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        "\"MAX_"                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        "HEADERS\""                                                                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ":" _DWS_STR(MAX_HEADERS) ","                                                                                                                                                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "\"MAX_PATH_"                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "LEN\""                                                                                                                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ":" _DWS_STR(MAX_PATH_LEN) ","                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             "\"MAX_KEY_LEN\":" _DWS_STR(MAX_KEY_LEN) ","                                                                                                                                                                                                                                                                                               \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      "\"MAX_VAL_LEN\":" _DWS_STR(MAX_VAL_LEN) ","                                                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               "\"MAX_QUERY_LEN\":" _DWS_STR(                                                                                                                                                                                                                           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   MAX_QUERY_LEN) ","                                                                                                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  "\"MAX_QUERY_PARAMS\":" _DWS_STR(MAX_QUERY_PARAMS) ","                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     "\"CONN_TIMEOUT_MS\":" _DWS_STR(                                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         CONN_TIMEOUT_MS) ","                                                                                                                                                           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          "\"RESP_HDR_BUF_SIZE\":" _DWS_STR(RESP_HDR_BUF_SIZE) ","                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               "\"WS_HDR_BUF_SIZE\":" _DWS_STR(WS_HDR_BUF_SIZE) ","                                                     \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                "\"CORS_HDR_BUF_SIZE\":" _DWS_STR(                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    CORS_HDR_BUF_SIZE) ","                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       "\"EVT_QUEUE_DEPTH\":" _DWS_STR( \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           EVT_QUEUE_DEPTH) "}"         \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            "}"

#endif // DWS_ENABLE_DIAG

// ---------------------------------------------------------------------------
// Compile-time sanity checks
// ---------------------------------------------------------------------------
// These produce a clear #error message in the compiler output rather than a
// cryptic linker failure or silent misbehavior.

#if EVT_QUEUE_DEPTH < MAX_CONNS * 4
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: EVT_QUEUE_DEPTH must be >= MAX_CONNS * 4 to absorb event bursts without blocking lwIP"
#endif

#if MAX_CONNS < 1
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be >= 1"
#endif

#if MAX_CONNS > 255
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be <= 255 (slot IDs are uint8_t)"
#endif

#if DWS_ENABLE_WEBSOCKET && DWS_ENABLE_SSE
#if MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#elif DWS_ENABLE_WEBSOCKET
#if MAX_WS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS must not exceed MAX_CONNS"
#endif
#elif DWS_ENABLE_SSE
#if MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#endif

#if BODY_BUF_SIZE < 1
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must be >= 1"
#endif

#if BODY_BUF_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must not exceed RX_BUF_SIZE (parser reads from the ring buffer)"
#endif

#if DWS_ENABLE_FILE_SERVING && FILE_CHUNK_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: FILE_CHUNK_SIZE must not exceed RX_BUF_SIZE"
#endif

#if MAX_KEY_LEN < 4
#error "DeterministicESPAsyncWebServer: MAX_KEY_LEN must be >= 4 (minimum valid HTTP header name length)"
#endif

#if MAX_VAL_LEN < 1
#error "DeterministicESPAsyncWebServer: MAX_VAL_LEN must be >= 1"
#endif

#if MAX_PATH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_PATH_LEN must be >= 2 (minimum: \"/\")"
#endif

#if MAX_ROUTES < 1
#error "DeterministicESPAsyncWebServer: MAX_ROUTES must be >= 1"
#endif

#if MAX_MIDDLEWARE < 1
#error "DeterministicESPAsyncWebServer: MAX_MIDDLEWARE must be >= 1"
#endif

#if CHUNK_BUF_SIZE < 16
#error "DeterministicESPAsyncWebServer: CHUNK_BUF_SIZE must be >= 16"
#endif

#if JSON_MAX_DEPTH < 1
#error "DeterministicESPAsyncWebServer: JSON_MAX_DEPTH must be >= 1"
#endif

#if RE_MAX_STEPS < 64
#error "DeterministicESPAsyncWebServer: RE_MAX_STEPS must be >= 64"
#endif

// RSA-2048 verification (OIDC / SSH host key / JWKS) runs on a worker task and consumes
// ~7 KB of stack via the mbedTLS bignum modexp. Enforce the documented floor so a lowered
// worker stack is caught at build time instead of overflowing on the first verify.
#if DWS_ENABLE_OIDC && !DWS_ENABLE_SSH && (DWS_WORKER_TASK_STACK < DWS_WORKER_STACK_RSA_MIN)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_WORKER_TASK_STACK is below DWS_WORKER_STACK_RSA_MIN; RSA-2048 verification (OIDC) needs ~7 KB of worker stack - raise DWS_WORKER_TASK_STACK (>= 8192) or marshal RSA verifies onto a dedicated larger-stack task"
#endif

// SSH additionally can negotiate curve25519-sha256 + ssh-ed25519, whose software field
// arithmetic peaks at ~10.5 KB of worker stack (deeper than the RSA path). Enforce the
// higher floor so a lowered stack is caught at build time instead of tripping the task
// stack canary on the first modern-crypto handshake.
#if (DWS_ENABLE_SSH || DWS_ENABLE_HTTP3) && (DWS_WORKER_TASK_STACK < DWS_WORKER_STACK_CURVE_MIN)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_WORKER_TASK_STACK is below DWS_WORKER_STACK_CURVE_MIN; SSH and HTTP/3 (QUIC TLS-1.3) curve25519/ed25519 need ~10.5 KB of worker stack - raise DWS_WORKER_TASK_STACK (>= 12288) or marshal the handshake onto a dedicated larger-stack task"
#endif

// The PQ/T hybrid KEX (DWS_ENABLE_PQC_KEX) runs ML-KEM-768 Encaps in the handshake path, whose NTT
// + sampling peak at ~7 KB of worker stack on top of the classical curve/ed25519 work. Enforce a
// higher floor so it is caught at build time rather than overflowing on the first hybrid handshake.
#ifndef DWS_WORKER_STACK_PQC_MIN
#define DWS_WORKER_STACK_PQC_MIN 16384
#endif
#if DWS_ENABLE_PQC_KEX && (DWS_ENABLE_SSH || DWS_ENABLE_HTTP3) && (DWS_WORKER_TASK_STACK < DWS_WORKER_STACK_PQC_MIN)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_WORKER_TASK_STACK is below DWS_WORKER_STACK_PQC_MIN; the ML-KEM-768 hybrid KEX (DWS_ENABLE_PQC_KEX) needs ~7 KB more worker stack - raise DWS_WORKER_TASK_STACK (>= 16384) or marshal the handshake onto a dedicated larger-stack task"
#endif

#if DWS_ENABLE_TLS
#if MAX_TLS_CONNS < 1 || MAX_TLS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_TLS_CONNS must be between 1 and MAX_CONNS"
#endif
#if DWS_TLS_ARENA_SIZE < 8192
#error "DeterministicESPAsyncWebServer: DWS_TLS_ARENA_SIZE is far too small for a TLS handshake"
#endif
// Concurrent TLS guard: the whole arena is static .bss and the ESP32 internal
// dram0_0_seg ceiling is only ~122 KB (ROM-reserved at both ends), so a 2nd
// connection's arena overflows the link. Reject MAX_TLS_CONNS > 1 with a clear
// message unless the arena is offloaded to PSRAM or the build was consciously sized -
// far friendlier than the raw "region `dram0_0_seg' overflowed" linker error.
#if defined(ARDUINO) && (MAX_TLS_CONNS > 1) && !DWS_TLS_ARENA_IN_PSRAM && !DWS_TLS_ACK_MULTI_CONN_DRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: MAX_TLS_CONNS > 1 - the static TLS arena will not fit the ~122 KB internal dram0_0_seg. Pick a path (docs/KNOWN_LIMITATIONS.md): set DWS_TLS_ARENA_IN_PSRAM=1 on a PSRAM board, OR shrink records via a custom ESP-IDF build (CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN + DWS_TLS_MAX_FRAG_LEN), OR reclaim internal DRAM; then set DWS_TLS_ACK_MULTI_CONN_DRAM=1 to confirm."
#endif
#endif

// HTTP/2's per-connection engine pool (~MAX_CONNS x 28 KB) cannot fit internal DRAM alongside
// TLS, so it must live in PSRAM. Fail fast with guidance instead of the raw linker overflow.
#if DWS_ENABLE_HTTP2 && defined(ARDUINO) && !DWS_H2_POOL_IN_PSRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_ENABLE_HTTP2 needs PSRAM - the HTTP/2 engine pool (~MAX_CONNS x 28 KB) overflows the ~122 KB internal dram0_0_seg alongside TLS. Set DWS_H2_POOL_IN_PSRAM=1 on a PSRAM board (S3 / P4 / WROVER) built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y (tools/psram/README.md)."
#endif

#if DWS_ENABLE_SNMP
#if SNMP_MAX_OID_LEN < 4
#error "DeterministicESPAsyncWebServer: SNMP_MAX_OID_LEN must be >= 4"
#endif
#if SNMP_MAX_MIB_ENTRIES < 1
#error "DeterministicESPAsyncWebServer: SNMP_MAX_MIB_ENTRIES must be >= 1"
#endif
#if SNMP_MAX_VARBINDS < 1
#error "DeterministicESPAsyncWebServer: SNMP_MAX_VARBINDS must be >= 1"
#endif
#if SNMP_MSG_BUF_SIZE < 484
#error "DeterministicESPAsyncWebServer: SNMP_MSG_BUF_SIZE must be >= 484 (RFC 1157 minimum)"
#endif
#endif

#if DWS_ENABLE_COAP
#if DWS_COAP_MAX_RESOURCES < 1
#error "DeterministicESPAsyncWebServer: DWS_COAP_MAX_RESOURCES must be >= 1"
#endif
#if DWS_COAP_MAX_PATH < 2
#error "DeterministicESPAsyncWebServer: DWS_COAP_MAX_PATH must be >= 2 (minimum: \"/\")"
#endif
#if DWS_COAP_MAX_PAYLOAD < 1
#error "DeterministicESPAsyncWebServer: DWS_COAP_MAX_PAYLOAD must be >= 1"
#endif
#if DWS_COAP_MSG_BUF_SIZE < (DWS_COAP_MAX_PAYLOAD + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_COAP_MSG_BUF_SIZE must be >= DWS_COAP_MAX_PAYLOAD + 16 (header + token + Content-Format option + payload marker)"
#endif
#endif

#if DWS_ENABLE_AUTH && MAX_AUTH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_AUTH_LEN must be >= 2 when DWS_ENABLE_AUTH is set"
#endif

#if DWS_ENABLE_PER_IP_THROTTLE
#if DWS_PER_IP_THROTTLE_SLOTS < 1
#error "DeterministicESPAsyncWebServer: DWS_PER_IP_THROTTLE_SLOTS must be >= 1 when DWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#if DWS_PER_IP_THROTTLE_MAX < 1
#error "DeterministicESPAsyncWebServer: DWS_PER_IP_THROTTLE_MAX must be >= 1 when DWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#endif

#if DWS_ENABLE_IP_ALLOWLIST && DWS_IP_ALLOWLIST_SLOTS < 1
#error "DeterministicESPAsyncWebServer: DWS_IP_ALLOWLIST_SLOTS must be >= 1 when DWS_ENABLE_IP_ALLOWLIST is set"
#endif

#if DWS_ENABLE_AUTH_LOCKOUT
#if !DWS_ENABLE_AUTH
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_AUTH_LOCKOUT requires DWS_ENABLE_AUTH"
#endif
#if DWS_AUTH_LOCKOUT_SLOTS < 1
#error "DeterministicESPAsyncWebServer: DWS_AUTH_LOCKOUT_SLOTS must be >= 1 when DWS_ENABLE_AUTH_LOCKOUT is set"
#endif
#if DWS_AUTH_LOCKOUT_THRESHOLD < 1
#error "DeterministicESPAsyncWebServer: DWS_AUTH_LOCKOUT_THRESHOLD must be >= 1 when DWS_ENABLE_AUTH_LOCKOUT is set"
#endif
#if DWS_AUTH_LOCKOUT_BASE_MS < 1 || DWS_AUTH_LOCKOUT_MAX_MS < DWS_AUTH_LOCKOUT_BASE_MS
#error "DeterministicESPAsyncWebServer: need 1 <= DWS_AUTH_LOCKOUT_BASE_MS <= DWS_AUTH_LOCKOUT_MAX_MS"
#endif
// The backoff doubles a uint32 capped at MAX_MS, so MAX_MS must leave headroom for
// one more shift (cap <= 0x80000000 => cap<<1 fits in uint32 without overflow).
#if DWS_AUTH_LOCKOUT_MAX_MS > 0x80000000
#error "DeterministicESPAsyncWebServer: DWS_AUTH_LOCKOUT_MAX_MS must be <= 0x80000000 (2147483648)"
#endif
#endif

#if DWS_ENABLE_WEBDAV
#if !DWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_WEBDAV requires DWS_ENABLE_FILE_SERVING"
#endif
#if DWS_WEBDAV_BUF_SIZE < 256
#error "DeterministicESPAsyncWebServer: DWS_WEBDAV_BUF_SIZE must be >= 256"
#endif
#if DWS_METHOD_BUF_SIZE < 10
#error "DeterministicESPAsyncWebServer: DWS_METHOD_BUF_SIZE must be >= 10 when DWS_ENABLE_WEBDAV is set (PROPPATCH)"
#endif
#endif

#if DWS_ENABLE_MODBUS
#if DWS_MODBUS_COILS < 1 || DWS_MODBUS_DISCRETE_INPUTS < 1 || DWS_MODBUS_HOLDING_REGS < 1 || DWS_MODBUS_INPUT_REGS < 1
#error "DeterministicESPAsyncWebServer: each DWS_MODBUS_* table size must be >= 1 when DWS_ENABLE_MODBUS is set"
#endif
#endif

#if DWS_ENABLE_KEEPALIVE && DWS_KEEPALIVE_MAX_REQUESTS < 1
#error "DeterministicESPAsyncWebServer: DWS_KEEPALIVE_MAX_REQUESTS must be >= 1 when DWS_ENABLE_KEEPALIVE is set"
#endif

#if DWS_ENABLE_RANGE && !DWS_ENABLE_FILE_SERVING && !DWS_ENABLE_EDGE_CACHE
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_RANGE requires DWS_ENABLE_FILE_SERVING or DWS_ENABLE_EDGE_CACHE"
#endif

#if DWS_ENABLE_MTLS && !DWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_MTLS requires DWS_ENABLE_TLS"
#endif

#if DWS_ENABLE_TLS_RESUMPTION && !DWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_TLS_RESUMPTION requires DWS_ENABLE_TLS"
#endif

#if DWS_ENABLE_METRICS && !DWS_ENABLE_STATS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_METRICS requires DWS_ENABLE_STATS"
#endif

#if DWS_ENABLE_HTTP_CLIENT_TLS && !DWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_HTTP_CLIENT_TLS requires DWS_ENABLE_TLS"
#endif

#if DWS_ENABLE_MQTT_TLS && !DWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_MQTT_TLS requires DWS_ENABLE_TLS"
#endif

#if DWS_ENABLE_WS_CLIENT_TLS && !DWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_WS_CLIENT_TLS requires DWS_ENABLE_TLS"
#endif

#if DWS_ENABLE_SNMP_TRAP && !DWS_ENABLE_SNMP
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SNMP_TRAP requires DWS_ENABLE_SNMP"
#endif

#if DWS_ENABLE_COAP_OBSERVE && !DWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_COAP_OBSERVE requires DWS_ENABLE_COAP"
#endif

#if DWS_ENABLE_COAP_BLOCK
#if !DWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_COAP_BLOCK requires DWS_ENABLE_COAP"
#endif
#if DWS_COAP_BLOCK_SZX_MAX > 6
#error "DeterministicESPAsyncWebServer: DWS_COAP_BLOCK_SZX_MAX must be <= 6 (block size 2^(SZX+4); SZX 7 is reserved)"
#endif
#if DWS_COAP_MSG_BUF_SIZE < ((1 << (DWS_COAP_BLOCK_SZX_MAX + 4)) + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_COAP_MSG_BUF_SIZE must hold one full block (2^(DWS_COAP_BLOCK_SZX_MAX+4)) + 16 header/option bytes"
#endif
#if DWS_COAP_BLOCK1_MAX < (1 << (DWS_COAP_BLOCK_SZX_MAX + 4))
#error "DeterministicESPAsyncWebServer: DWS_COAP_BLOCK1_MAX must be >= one block (2^(DWS_COAP_BLOCK_SZX_MAX+4))"
#endif
#endif

// --- feature dependency guards (centralized; see the BUILD-FLAG DEPENDENCY TREE
//     near the top of this file). A child feature requires its parent(s). ---

#if DWS_ENABLE_WS_DEFLATE && !DWS_ENABLE_WEBSOCKET
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_WS_DEFLATE requires DWS_ENABLE_WEBSOCKET"
#endif

#if DWS_ENABLE_CIA402 && !DWS_ENABLE_CANOPEN
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_CIA402 requires DWS_ENABLE_CANOPEN"
#endif

#if DWS_ENABLE_SSH_ZLIB && !DWS_ENABLE_SSH
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_ZLIB requires DWS_ENABLE_SSH"
#endif

#if DWS_ENABLE_SSH_SFTP && !DWS_ENABLE_SSH
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_SFTP requires DWS_ENABLE_SSH"
#endif
#if DWS_ENABLE_SSH_SFTP && !DWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_SFTP requires DWS_ENABLE_FILE_SERVING (the fs::FS seam)"
#endif
#if DWS_ENABLE_SSH_SCP && !DWS_ENABLE_SSH
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_SCP requires DWS_ENABLE_SSH"
#endif
#if DWS_ENABLE_SSH_SCP && !DWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_SCP requires DWS_ENABLE_FILE_SERVING (the fs::FS seam)"
#endif

#if DWS_ENABLE_SSH_ZLIB
// Window must be a power of two in [256, 32768] (32 KB is zlib's max; the client's inflate window).
#if (DWS_SSH_ZLIB_WINDOW & (DWS_SSH_ZLIB_WINDOW - 1)) != 0 || DWS_SSH_ZLIB_WINDOW < 256 || DWS_SSH_ZLIB_WINDOW > 32768
#error "DeterministicESPAsyncWebServer: DWS_SSH_ZLIB_WINDOW must be a power of two in [256, 32768]"
#endif
// Positions index a uint16 hash chain, so window + max-input must stay under 65535.
#if (DWS_SSH_ZLIB_WINDOW + DWS_SSH_ZLIB_MAX_IN) > 65534
#error "DeterministicESPAsyncWebServer: DWS_SSH_ZLIB_WINDOW + DWS_SSH_ZLIB_MAX_IN must be <= 65534"
#endif
// The compressor must accept a full packet payload, else a max-size outbound packet would fail to
// compress and drop, desyncing the stateful stream.
#if DWS_SSH_ZLIB_MAX_IN < SSH_PKT_BUF_SIZE
#error "DeterministicESPAsyncWebServer: DWS_SSH_ZLIB_MAX_IN must be >= SSH_PKT_BUF_SIZE"
#endif
// The per-connection compressor (window work buffer + hash chain) is ~48 KB. On ARDUINO pick a path:
// offload it to PSRAM (DWS_SSH_ZLIB_IN_PSRAM, a PSRAM board built with the BSS-in-PSRAM core), or
// acknowledge the internal-DRAM cost (DWS_SSH_ZLIB_ACK_DRAM, fine for MAX_SSH_CONNS=1 without TLS on
// a roomy S3 / P4). Fail fast with guidance instead of a raw linker overflow.
#if defined(ARDUINO) && !DWS_SSH_ZLIB_IN_PSRAM && !DWS_SSH_ZLIB_ACK_DRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DWS_ENABLE_SSH_ZLIB - the per-connection compressor is ~48 KB. Set DWS_SSH_ZLIB_IN_PSRAM=1 on a PSRAM board (S3 / P4 / WROVER, core built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y, tools/psram/README.md), OR set DWS_SSH_ZLIB_ACK_DRAM=1 to accept the internal-DRAM cost (fits MAX_SSH_CONNS=1 without TLS on a roomy chip)."
#endif
#endif

#if DWS_ENABLE_WEB_TERMINAL && !DWS_ENABLE_WEBSOCKET
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_WEB_TERMINAL requires DWS_ENABLE_WEBSOCKET"
#endif

#if DWS_ENABLE_DASHBOARD && !DWS_ENABLE_SSE
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_DASHBOARD requires DWS_ENABLE_SSE"
#endif

#if DWS_ENABLE_SNMP_V3 && !DWS_ENABLE_SNMP
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_SNMP_V3 requires DWS_ENABLE_SNMP"
#endif

#if DWS_ENABLE_OPCUA_CLIENT && !DWS_ENABLE_OPCUA
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_OPCUA_CLIENT requires DWS_ENABLE_OPCUA (the shared OPC UA codec)"
#endif

#if DWS_ENABLE_UMATI && !DWS_ENABLE_OPCUA
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_UMATI requires DWS_ENABLE_OPCUA (it builds on the OPC UA server)"
#endif

#if DWS_ENABLE_CONFIG_IO && !DWS_ENABLE_CONFIG_STORE
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_CONFIG_IO requires DWS_ENABLE_CONFIG_STORE"
#endif

#if DWS_ENABLE_HTTP_CLIENT_TLS && !DWS_ENABLE_HTTP_CLIENT
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_HTTP_CLIENT_TLS requires DWS_ENABLE_HTTP_CLIENT"
#endif

#if DWS_ENABLE_MQTT_TLS && !DWS_ENABLE_MQTT
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_MQTT_TLS requires DWS_ENABLE_MQTT"
#endif

#if DWS_ENABLE_WS_CLIENT_TLS && !DWS_ENABLE_WS_CLIENT
#error "DeterministicESPAsyncWebServer: DWS_ENABLE_WS_CLIENT_TLS requires DWS_ENABLE_WS_CLIENT"
#endif

#if DWS_ENABLE_WEBSOCKET && WS_FRAME_SIZE < 2
#error "DeterministicESPAsyncWebServer: WS_FRAME_SIZE must be >= 2 when DWS_ENABLE_WEBSOCKET is set"
#endif

#if DWS_ENABLE_SSE && SSE_BUF_SIZE < 8
#error "DeterministicESPAsyncWebServer: SSE_BUF_SIZE must be >= 8 when DWS_ENABLE_SSE is set"
#endif

#if DWS_ENABLE_MULTIPART && MAX_MULTIPART_PARTS < 1
#error "DeterministicESPAsyncWebServer: MAX_MULTIPART_PARTS must be >= 1 when DWS_ENABLE_MULTIPART is set"
#endif

#if RESP_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= 128 (status line + headers + CORS block)"
#endif

#if DWS_ENABLE_WEBSOCKET && WS_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: WS_HDR_BUF_SIZE must be >= 128 when DWS_ENABLE_WEBSOCKET is set"
#endif

#if CORS_HDR_BUF_SIZE < 64
#error "DeterministicESPAsyncWebServer: CORS_HDR_BUF_SIZE must be >= 64"
#endif

#if RESP_HDR_BUF_SIZE < (CORS_HDR_BUF_SIZE + EXTRA_HDR_BUF_SIZE + 96)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= CORS_HDR_BUF_SIZE + EXTRA_HDR_BUF_SIZE + 96 (status line + CORS block + custom-header block are injected into response headers)"
#endif

// ===========================================================================
// Feature tuning knobs (grouped and gated by feature)
// ===========================================================================
//
// One place to turn every tunable knob - buffer sizes, table depths, limits, thresholds -
// so you never have to open a feature header. Each is an override-able default (set it in
// your build flags to change it); the owning module includes this header and uses the
// value. An optional feature's group is wrapped in that feature's DWS_ENABLE_* flag
// (resolved above), so a knob only exists when its feature is built.
//
// What is NOT here: protocol- and algorithm-fixed constants (wire opcodes, magic bytes,
// crypto digest / block sizes, spec-mandated PDU / field widths, the deflate/inflate
// scratch sizes a static_assert pins to the table layout). Those are not knobs - changing
// them breaks conformance - so they stay in their feature file next to the code they bind.

// -- Core: protocol dispatch + shared outbound transport (always built) --
/** @brief Size of the protocol-handler dispatch table; must exceed the largest ConnProto id. */
#ifndef DWS_PROTO_MAX
#define DWS_PROTO_MAX 11
#endif
// proto_register / proto_get index this table by ConnProto id, so it must be wide enough for every id.
static_assert((unsigned)ConnProto::PROTO_MESH < DWS_PROTO_MAX, "DWS_PROTO_MAX must exceed the largest ConnProto id");
/** @brief Number of simultaneous outbound client connections (BSS pool size). */
#ifndef DWS_CLIENT_CONNS
#define DWS_CLIENT_CONNS 2
#endif
/**
 * @brief Per-connection wire receive ring size (bytes).
 *
 * Holds plaintext (plain) or ciphertext (TLS). The transport ACKs on consume
 * (dws_client_read reopens the window), so for a large inbound transfer to never
 * stall the ring must hold a full TCP receive window: keep DWS_CLIENT_RX_BUF >=
 * TCP_WND (~5.7 KB). The 8192 default clears that and a multi-KB TLS handshake
 * flight; a ring below TCP_WND can deadlock a sustained download (the peer would be
 * allowed to send more than the ring holds). Must exceed one TCP segment (TCP_MSS).
 */
#ifndef DWS_CLIENT_RX_BUF
#define DWS_CLIENT_RX_BUF 8192
#endif

// -- SSH (network_drivers/presentation/ssh; the codec compiles when the SSH sources are
//    built, so its knobs are always defined) --
/** @brief Initial receive window the SSH server advertises (RFC 4254 §5.1). */
#ifndef SSH_CHAN_WINDOW
#define SSH_CHAN_WINDOW 32768u
#endif
/**
 * @brief Maximum SSH channel data payload the server advertises it can receive per message.
 *
 * This is what a peer may put in one SSH_MSG_CHANNEL_DATA, so it MUST fit one inbound SSH packet: the
 * transport rejects any packet larger than SSH_PKT_BUF_SIZE, so advertising more than that (minus the
 * channel-data + packet framing + MAC + padding) makes a peer that sends a bigger message - e.g. an SFTP
 * WRITE - trip the packet-too-large check and drop the connection. Derived from SSH_PKT_BUF_SIZE so it scales
 * when that buffer is raised (e.g. for higher SFTP throughput). Interactive shells never approach it.
 */
#ifndef SSH_CHAN_MAX_PACKET
#define SSH_CHAN_MAX_PACKET (SSH_PKT_BUF_SIZE - 64u)
#endif
/**
 * @brief Re-key when either packet sequence number reaches this value.
 *
 * RFC 4253 §9 recommends re-keying after ~1 GB or one hour. A packet-count proxy is used
 * here; the default is well below SSH_SEQ_CLOSE_THRESHOLD so a re-key always happens before
 * the sequence number can wrap.
 */
#ifndef SSH_REKEY_PACKET_THRESHOLD
#define SSH_REKEY_PACKET_THRESHOLD 0x40000000u
#endif
/**
 * @brief Elapsed-time re-key trigger in milliseconds (RFC 4253 §9: "after each hour"). Default 1 hour.
 *
 * A server-initiated re-key fires when either this much time or SSH_REKEY_PACKET_THRESHOLD packets have
 * passed since the last KEX, whichever comes first. Set to 0 to disable the time trigger (packet-count
 * only). Measured with the pluggable clock (dws_millis()).
 */
#ifndef SSH_REKEY_TIME_MS
#define SSH_REKEY_TIME_MS 3600000u
#endif
/** @brief Max stored user name (RFC 4252 imposes no limit; we cap for BSS). */
#ifndef SSH_AUTH_USER_MAX
#define SSH_AUTH_USER_MAX 32
#endif
/** @brief Max stored password length. */
#ifndef SSH_AUTH_PASS_MAX
#define SSH_AUTH_PASS_MAX 64
#endif
/**
 * @brief Max stored size of the CLIENT KEXINIT payload (I_C, for the exchange hash).
 *
 * A modern OpenSSH client's KEXINIT (post-quantum KEX names + cert host-key algs + EtM
 * MACs + ext-info-c) runs well past 1 KB, so this must be large enough to hold it - a
 * smaller bound silently rejects real clients at key exchange. The packet layer already
 * caps any single packet at SSH_PKT_BUF_SIZE.
 */
#ifndef SSH_KEXINIT_MAX
#define SSH_KEXINIT_MAX 2048
#endif

#if DWS_ENABLE_AUDIT_LOG
// -- Audit log (services/audit_log) --
#ifndef DWS_AUDIT_LOG_ENTRIES
#define DWS_AUDIT_LOG_ENTRIES 32 ///< RAM ring depth (records retained for query/verify).
#endif
#ifndef DWS_AUDIT_MSG_LEN
#define DWS_AUDIT_MSG_LEN 48 ///< Max message bytes per record (truncated to fit).
#endif
#endif // DWS_ENABLE_AUDIT_LOG

#if DWS_ENABLE_DEVICENET
// -- DeviceNet (services/devicenet) --
#ifndef DWS_DEVICENET_MSG_MAX
#define DWS_DEVICENET_MSG_MAX 256 ///< max reassembled fragmented message
#endif
#endif // DWS_ENABLE_DEVICENET

#if DWS_ENABLE_ESPNOW
// -- ESP-NOW (services/espnow) --
#ifndef DWS_ESPNOW_MAX_PEERS
#define DWS_ESPNOW_MAX_PEERS 8 ///< Bounded peer registry size.
#endif
#endif // DWS_ENABLE_ESPNOW

#if DWS_ENABLE_GRAPHQL
// -- GraphQL (services/graphql) --
#ifndef DWS_GQL_MAX_NODES
#define DWS_GQL_MAX_NODES 48 ///< Max fields across the whole query.
#endif
#ifndef DWS_GQL_MAX_ARGS
#define DWS_GQL_MAX_ARGS 24 ///< Max arguments across the whole query.
#endif
#ifndef DWS_GQL_MAX_DEPTH
#define DWS_GQL_MAX_DEPTH 6 ///< Max selection-set nesting depth.
#endif
#ifndef DWS_GQL_NAME_MAX
#define DWS_GQL_NAME_MAX 32 ///< Max field / argument name length.
#endif
#ifndef DWS_GQL_PATH_MAX
#define DWS_GQL_PATH_MAX 96 ///< Max dotted path length passed to the resolver.
#endif
#ifndef DWS_GQL_STRBUF
#define DWS_GQL_STRBUF 256 ///< Pool for decoded string-argument bytes.
#endif
#endif // DWS_ENABLE_GRAPHQL

#if DWS_ENABLE_J1939
// -- J1939 (services/j1939; also built when NMEA 2000 is enabled) --
#ifndef DWS_J1939_TP_MAX
#define DWS_J1939_TP_MAX 256 ///< max reassembled TP message (spec allows up to 1785); sized down for RAM
#endif
#endif // DWS_ENABLE_J1939

#if DWS_ENABLE_NMEA0183
// -- NMEA 0183 (services/nmea0183) --
#ifndef DWS_NMEA0183_MAX_FIELDS
#define DWS_NMEA0183_MAX_FIELDS 26 ///< max comma-separated fields (incl. the address field)
#endif
#endif // DWS_ENABLE_NMEA0183

#if DWS_ENABLE_NMEA2000
// -- NMEA 2000 (services/nmea2000) --
#ifndef DWS_N2K_FP_MAX
#define DWS_N2K_FP_MAX 223 ///< Fast Packet max payload (6 in frame 0 + 31 x 7)
#endif
#endif // DWS_ENABLE_NMEA2000

#if DWS_ENABLE_OAUTH2
// -- OAuth2 (services/oauth2) --
#ifndef DWS_OAUTH2_TOKEN_LEN
#define DWS_OAUTH2_TOKEN_LEN 768 ///< access_token / id_token buffer (JWTs are large).
#endif
#ifndef DWS_OAUTH2_RT_LEN
#define DWS_OAUTH2_RT_LEN 256 ///< refresh_token buffer.
#endif
#ifndef DWS_OAUTH2_BODY_BUF
#define DWS_OAUTH2_BODY_BUF 1024 ///< token-request body buffer.
#endif
#ifndef DWS_OAUTH2_RESP_BUF
#define DWS_OAUTH2_RESP_BUF 2048 ///< token-endpoint response buffer.
#endif
#endif // DWS_ENABLE_OAUTH2

#if DWS_ENABLE_OIDC
// -- OIDC (services/oidc) --
#ifndef DWS_OIDC_MAX_LEN
#define DWS_OIDC_MAX_LEN 1600 ///< Max accepted ID-token length.
#endif
#ifndef DWS_OIDC_SUB_LEN
#define DWS_OIDC_SUB_LEN 64 ///< Captured `sub` claim buffer.
#endif
#ifndef DWS_OIDC_EMAIL_LEN
#define DWS_OIDC_EMAIL_LEN 96 ///< Captured `email` claim buffer.
#endif
#ifndef DWS_OIDC_KID_LEN
#define DWS_OIDC_KID_LEN 80 ///< Max `kid` length.
#endif
#ifndef DWS_OIDC_JWKS_MAX
#define DWS_OIDC_JWKS_MAX 16384 ///< Max JWKS document scanned; exceeds any real multi-key set, bounds the parse.
#endif
#endif // DWS_ENABLE_OIDC

#if DWS_ENABLE_OPCUA
// -- OPC UA (services/opcua) --
#ifndef DWS_OPCUA_BUF
#define DWS_OPCUA_BUF 8192 ///< Server's advertised buffer / max-message size for the handshake.
#endif
#ifndef DWS_OPCUA_READ_MAX
#define DWS_OPCUA_READ_MAX 8 ///< max NodesToRead handled per ReadRequest.
#endif
#ifndef DWS_OPCUA_BROWSE_MAX
#define DWS_OPCUA_BROWSE_MAX 4 ///< max NodesToBrowse handled per BrowseRequest.
#endif
#ifndef DWS_OPCUA_REF_MAX
#define DWS_OPCUA_REF_MAX 8 ///< max references returned per browsed node.
#endif
#ifndef DWS_OPCUA_WRITE_MAX
#define DWS_OPCUA_WRITE_MAX 8 ///< max NodesToWrite handled per WriteRequest.
#endif
// Advertised server identity (endpoint descriptions), overridable per deployment; the app may
// also set these at runtime via dws_opcua_set_endpoint_url() / the OpcUaServerInfo it passes.
#ifndef DWS_OPCUA_DEFAULT_ENDPOINT
#define DWS_OPCUA_DEFAULT_ENDPOINT "opc.tcp://localhost:4840" ///< default endpoint URL.
#endif
#ifndef DWS_OPCUA_DEFAULT_APP_URI
#define DWS_OPCUA_DEFAULT_APP_URI "urn:det:opcua:server" ///< default ApplicationUri.
#endif
#ifndef DWS_OPCUA_DEFAULT_APP_NAME
#define DWS_OPCUA_DEFAULT_APP_NAME "DWSOpcUaServer" ///< default ApplicationName.
#endif
#endif // DWS_ENABLE_OPCUA

#if DWS_ENABLE_PROVISIONING
// -- Wi-Fi provisioning credential store (services/provisioning_service) --
// The NVS namespace and its keys, overridable per deployment (e.g. to avoid an NVS-namespace
// collision with the application's own store).
#ifndef DWS_PROV_NVS_NAMESPACE
#define DWS_PROV_NVS_NAMESPACE "wifi_prov" ///< NVS namespace holding the saved credentials.
#endif
#ifndef DWS_PROV_KEY_SSID
#define DWS_PROV_KEY_SSID "ssid" ///< NVS key + HTML form field for the SSID.
#endif
#ifndef DWS_PROV_KEY_PSK
#define DWS_PROV_KEY_PSK "psk" ///< NVS key + HTML form field for the pre-shared key.
#endif
#endif // DWS_ENABLE_PROVISIONING

#if DWS_ENABLE_VFS
// -- Virtual filesystem (services/vfs) --
#ifndef DWS_VFS_RAM_FILES
#define DWS_VFS_RAM_FILES 4 ///< RAM backend: number of files.
#endif
#ifndef DWS_VFS_RAM_FILE_SIZE
#define DWS_VFS_RAM_FILE_SIZE 1024 ///< RAM backend: max bytes per file.
#endif
#ifndef DWS_VFS_MAX_OPEN
#define DWS_VFS_MAX_OPEN 4 ///< Concurrent open handles (per backend).
#endif
#ifndef DWS_VFS_NAME_MAX
#define DWS_VFS_NAME_MAX 48 ///< Max path length (RAM backend).
#endif
#endif // DWS_ENABLE_VFS

#endif
