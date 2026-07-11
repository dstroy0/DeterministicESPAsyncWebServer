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

/** @brief Maximum simultaneous TCP connections. */
#ifndef MAX_CONNS
#define MAX_CONNS 4
#endif

/** @brief Ring-buffer capacity in bytes per connection slot. */
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1024
#define DETWS_RX_BUF_SIZE_DEFAULTED 1 // we chose it; upsized for streaming below (see STREAM_BODY)
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
 * @brief Upper bound (ms) a slot may dwell in CONN_CLOSING after a graceful close
 *        before the idle sweep force-aborts it.
 *
 * On a graceful (local) close the slot stays in CONN_CLOSING - keeping its PCB and
 * callbacks - until the peer ACKs the response (then it frees itself in the sent
 * callback). If the peer never ACKs (dead/black-holed), this bound lets the
 * timeout sweep reclaim the slot so the fixed pool cannot leak.
 */
#ifndef DETWS_CLOSING_TIMEOUT_MS
#define DETWS_CLOSING_TIMEOUT_MS 2000
#endif

// ---------------------------------------------------------------------------
// Worker model (server task concurrency)
// ---------------------------------------------------------------------------
//
// The server pipeline (drain events -> dispatch -> send) runs in one or more
// dedicated FreeRTOS "worker" tasks instead of the user's loop(). Each worker
// owns a disjoint partition of conn_pool slots (slot i -> worker i %
// DETWS_WORKER_COUNT) and its own scratch arena, so no two workers ever touch
// the same slot: shared-nothing, no hot-path locks, latency stays bounded
// (determinism preserved) while cores run disjoint connections in parallel.
//
// DETWS_WORKER_COUNT == 1 (default) is byte-for-byte the single-pipeline model:
// one worker owns every slot, one arena, the existing single event queue. N > 1
// is opt-in. The arena BSS cost is DETWS_SCRATCH_ARENA_SIZE * DETWS_WORKER_COUNT.

/** @brief Number of server worker tasks (slots partitioned i % N). Default 1. */
#ifndef DETWS_WORKER_COUNT
#define DETWS_WORKER_COUNT 1
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
 * (::DETWS_WORKER_STACK_CURVE_MIN for SSH, ::DETWS_WORKER_STACK_RSA_MIN for
 * OIDC) or the first handshake overflows the task stack - a build-time guard
 * (bottom of this file) enforces the floor so a lowered stack is caught at
 * compile time.
 */
#ifndef DETWS_WORKER_TASK_STACK
#if defined(DETWS_ENABLE_SSH) && DETWS_ENABLE_SSH
#define DETWS_WORKER_TASK_STACK 12288
#else
#define DETWS_WORKER_TASK_STACK 8192
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
#ifndef DETWS_WORKER_STACK_RSA_MIN
#define DETWS_WORKER_STACK_RSA_MIN 8192
#endif

/**
 * @brief Minimum worker-task stack (bytes) required once SSH is compiled in.
 *
 * SSH can negotiate curve25519-sha256 + ssh-ed25519, whose software field
 * arithmetic peaks at ~10.5 KB of worker stack; 12 KB leaves ~1.8 KB of margin
 * for the rest of the handshake call chain (comparable to the RSA floor's
 * margin). Raise both this and ::DETWS_WORKER_TASK_STACK together if you extend
 * the handshake, or force RSA/DH only (ssh_kex_set_prefer_rsa) on a very tight
 * build - but the server still advertises the modern suite, so a modern-only
 * client would still exercise it.
 */
#ifndef DETWS_WORKER_STACK_CURVE_MIN
#define DETWS_WORKER_STACK_CURVE_MIN 12288
#endif

/** @brief FreeRTOS priority for each server worker task (ESP32). */
#ifndef DETWS_WORKER_TASK_PRIORITY
#define DETWS_WORKER_TASK_PRIORITY 5
#endif

/**
 * @brief Core that worker 0 pins to (ESP32). Worker k pins to (DETWS_WORKER_CORE
 * + k) % portNUM_PROCESSORS. Default 1 (APP_CPU), keeping Core 0 lean for the
 * WiFi/lwIP stack and offloading the user's loop().
 */
#ifndef DETWS_WORKER_CORE
#define DETWS_WORKER_CORE 1
#endif

/**
 * @brief Depth of each worker's deferred-callback queue.
 *
 * App code on loop() or another task submits work to a slot's owning worker via
 * detws_defer() / detws_defer_slot(); the worker runs it in its own single-thread
 * context, so an async push (ws_send / sse_send from a timer) is race-free. Each
 * worker has one queue of this depth (entries are a {fn, arg} pair, ~8 bytes).
 */
#ifndef DETWS_DEFER_QUEUE_DEPTH
#define DETWS_DEFER_QUEUE_DEPTH 8
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
#ifndef DETWS_WORKER_POLL_TICKS
#define DETWS_WORKER_POLL_TICKS 1
#endif

#if DETWS_WORKER_COUNT < 1
#error "DeterministicESPAsyncWebServer: DETWS_WORKER_COUNT must be >= 1"
#endif
#if DETWS_WORKER_COUNT > MAX_CONNS
#error "DeterministicESPAsyncWebServer: DETWS_WORKER_COUNT must be <= MAX_CONNS"
#endif

// ---------------------------------------------------------------------------
// Preempting work queue (DETWS_ENABLE_PREEMPT_QUEUE) - v5 real-time ingest
// ---------------------------------------------------------------------------
//
// Fixed-capacity queues, each feeding one core-pinned processing task: a producer
// posts a fixed-size item (from a task or an ISR) and the scheduler preempts straight
// to the task. There are named lanes - one USER lane exposed to the app, and internal
// DMA / forwarding / device-access lanes that run at a higher priority so internal
// ingest always preempts user work. Queue storage is static (zero heap), so depth +
// item size are compile-time; a task's stack is created only when its lane starts.
// The no-lane detws_pq_* API drives the USER lane. See preempt_queue.h.

/** @brief Enable the preempting work queue primitive (default off). */
#ifndef DETWS_ENABLE_PREEMPT_QUEUE
#define DETWS_ENABLE_PREEMPT_QUEUE 0
#endif

/** @brief Capacity of the preempting queue in items (static-allocated). */
#ifndef DETWS_PQ_DEPTH
#define DETWS_PQ_DEPTH 16
#endif

/** @brief Bytes per preempting-queue item (the posted item must fit). */
#ifndef DETWS_PQ_ITEM_SIZE
#define DETWS_PQ_ITEM_SIZE 32
#endif

/** @brief Stack (bytes) for each preempting-queue processing task (ESP32). */
#ifndef DETWS_PQ_STACK
#define DETWS_PQ_STACK 4096
#endif

/**
 * @brief Base FreeRTOS priority for the internal preempting lanes (DMA / forwarding /
 *        device access). They run at this and just above, so internal ingest preempts
 *        the user lane; keep it above the user lane's priority and below the lwIP tcpip
 *        (18) / WiFi tasks so networking is never starved. See preempt_queue.h.
 */
#ifndef DETWS_PQ_INTERNAL_PRIORITY
#define DETWS_PQ_INTERNAL_PRIORITY 8
#endif

#if DETWS_ENABLE_PREEMPT_QUEUE && (DETWS_PQ_DEPTH < 1 || DETWS_PQ_ITEM_SIZE < 1)
#error "DeterministicESPAsyncWebServer: DETWS_PQ_DEPTH and DETWS_PQ_ITEM_SIZE must be >= 1"
#endif

// ---------------------------------------------------------------------------
// DMA peripheral ingest / egress (DETWS_ENABLE_DMA) - v5 hardware ingest
// ---------------------------------------------------------------------------
//
// Move peripheral bytes (UART / I2C / SPI) between the wire and a static buffer
// with the CPU free during the transfer; a DMA-complete event carries the bytes
// to a user callback, which typically posts a descriptor into the preempting work
// queue (DETWS_ENABLE_PREEMPT_QUEUE) so the heavy processing runs off the ISR. RX
// is double-buffered (ping-pong): the completed buffer is handed up while the DMA
// engine fills the other. Storage is static (zero heap) - channel count and buffer
// size are compile-time. See services/dma/dma.h.
//
// DETWS_DMA_SIMULATE routes the transfers through an in-memory ingress/egress
// simulator (feed bytes in, capture bytes out, optional TX->RX loopback) so the
// whole pipeline is exercised with no physical loopback wire - on the host test
// bench and, with the flag set, on the device itself. It is the shipped, tested
// engine; a real silicon backend plugs into det_dma_hw_* when DETWS_DMA_SIMULATE=0.

/** @brief Enable the DMA peripheral ingest / egress primitive (default off). */
#ifndef DETWS_ENABLE_DMA
#define DETWS_ENABLE_DMA 0
#endif

/** @brief Number of DMA channels (static-allocated; each is one peripheral link). */
#ifndef DETWS_DMA_CHANNELS
#define DETWS_DMA_CHANNELS 2
#endif

/** @brief Bytes per DMA transfer buffer (RX is double-buffered at this size). */
#ifndef DETWS_DMA_BUF_SIZE
#define DETWS_DMA_BUF_SIZE 256
#endif

/**
 * @brief Route DMA transfers through the ingress/egress simulator (default on).
 *        Set to 0 to drive real silicon via the det_dma_hw_* backend hooks.
 */
#ifndef DETWS_DMA_SIMULATE
#define DETWS_DMA_SIMULATE 1
#endif

#if DETWS_ENABLE_DMA && (DETWS_DMA_CHANNELS < 1 || DETWS_DMA_BUF_SIZE < 1)
#error "DeterministicESPAsyncWebServer: DETWS_DMA_CHANNELS and DETWS_DMA_BUF_SIZE must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Interface forwarding plane (DETWS_ENABLE_FORWARD) - v5 hardware ingest
// ---------------------------------------------------------------------------
//
// A forwarding plane over the ingest pipeline: register interfaces (Wi-Fi STA / AP,
// Ethernet, a peripheral bus, a radio), each with an egress send callback, then add
// per-pair allow / deny rules with an optional rate cap. A frame arriving on one
// interface (det_forward_ingress(), typically from a DMA-complete event posted onto the
// FORWARD lane) is forwarded to every allowed destination, so the device bridges / routes
// between its interfaces instead of only terminating traffic. Default-deny and fail-closed
// (a full destination or an exceeded rate cap drops, never blocks). Static tables (zero
// heap). See services/forward/forward.h.

/** @brief Enable the interface forwarding plane (default off). */
#ifndef DETWS_ENABLE_FORWARD
#define DETWS_ENABLE_FORWARD 0
#endif

/** @brief Max interfaces the forwarding plane tracks (static-allocated). */
#ifndef DETWS_FWD_MAX_IFACES
#define DETWS_FWD_MAX_IFACES 4
#endif

/** @brief Max forwarding rules (src -> dst allow/deny + rate cap; static-allocated). */
#ifndef DETWS_FWD_MAX_RULES
#define DETWS_FWD_MAX_RULES 8
#endif

/** @brief Max ingress access-control entries (byte-pattern permit/deny; static). */
#ifndef DETWS_FWD_MAX_ACL
#define DETWS_FWD_MAX_ACL 8
#endif

/** @brief Bytes an ACL entry can match (its pattern / mask length). */
#ifndef DETWS_FWD_ACL_PATLEN
#define DETWS_FWD_ACL_PATLEN 4
#endif

/** @brief Max policy routes (byte-pattern -> egress interface; static). Policy routes take
 *         precedence over the src->dst rules, so tagged traffic leaves a chosen interface. */
#ifndef DETWS_FWD_MAX_ROUTES
#define DETWS_FWD_MAX_ROUTES 8
#endif

/** @brief Build-time toggle for the forwarding-path inspection hook (default off, for cost +
 *         privacy). When 1, det_forward_set_inspector() installs a runtime callback that observes
 *         / filters each ingress frame before it is forwarded; when 0 the hook is compiled out
 *         entirely (no call site). Runtime toggle: register or clear (null) the inspector. */
#ifndef DETWS_FWD_INSPECT
#define DETWS_FWD_INSPECT 0
#endif

#if DETWS_ENABLE_FORWARD && (DETWS_FWD_MAX_IFACES < 1 || DETWS_FWD_MAX_RULES < 1 || DETWS_FWD_ACL_PATLEN < 1)
#error "DeterministicESPAsyncWebServer: DETWS_FWD_MAX_IFACES / DETWS_FWD_MAX_RULES / DETWS_FWD_ACL_PATLEN must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Radio / wireless gateway (DETWS_ENABLE_GATEWAY) - v5 southbound-to-northbound bridge
// ---------------------------------------------------------------------------
//
// The generic gateway pattern: a southbound radio (LoRa / nRF24 / Zigbee / ... reached
// over SPI / I2C / UART) is a "port"; a frame it receives (data-ready ISR -> DMA -> the
// FORWARD lane -> a per-radio codec) is handed to det_gw_uplink(), which envelopes it with
// its source node address / port / RSSI and publishes it northbound through the uplink
// callback (wire it to MQTT / HTTP / WebSocket / UDP). A northbound command goes the other
// way through det_gw_downlink() to the port's transmit callback. The radio TX + the
// northbound publish are callbacks (the seam a real radio driver / protocol binding plugs
// into), so the bridge is host- and device-testable with no radio. Static tables (zero
// heap). See services/gateway/gateway.h.

/** @brief Enable the radio / wireless gateway bridge (default off). */
#ifndef DETWS_ENABLE_GATEWAY
#define DETWS_ENABLE_GATEWAY 0
#endif

/** @brief Max southbound gateway ports (radios / buses; static-allocated). */
#ifndef DETWS_GW_MAX_PORTS
#define DETWS_GW_MAX_PORTS 4
#endif

/** @brief Default northbound topic prefix (overridable at runtime via det_gw_set_topic_prefix). */
#ifndef DETWS_GW_DEFAULT_PREFIX
#define DETWS_GW_DEFAULT_PREFIX "gw"
#endif

#if DETWS_ENABLE_GATEWAY && (DETWS_GW_MAX_PORTS < 1)
#error "DeterministicESPAsyncWebServer: DETWS_GW_MAX_PORTS must be >= 1"
#endif

// ---------------------------------------------------------------------------
// LoRa radio (DETWS_ENABLE_LORA) - Semtech SX127x / RFM95-96 codec + driver
// ---------------------------------------------------------------------------
//
// A per-radio codec + driver that plugs into the gateway (DETWS_ENABLE_GATEWAY): the
// RadioHead-compatible 4-byte frame header (to / from / id / flags) codec, and an SX127x
// register driver over a caller-supplied register-access bus (so the SPI + chip-select
// wiring is the integration's, and the register protocol is host-testable with a mock
// bus). Bridge received frames northbound with det_gw_uplink(); the actual RF link needs
// the module to verify. See services/lora/lora.h.

/** @brief Enable the LoRa (SX127x) radio codec + driver (default off). */
#ifndef DETWS_ENABLE_LORA
#define DETWS_ENABLE_LORA 0
#endif

/** @brief Max LoRa payload bytes (SX127x FIFO is 256; RadioHead uses 251 + 4 header). */
#ifndef DETWS_LORA_MAX_PAYLOAD
#define DETWS_LORA_MAX_PAYLOAD 251
#endif

#if DETWS_ENABLE_LORA && (DETWS_LORA_MAX_PAYLOAD < 1 || DETWS_LORA_MAX_PAYLOAD > 251)
#error "DeterministicESPAsyncWebServer: DETWS_LORA_MAX_PAYLOAD must be 1..251"
#endif

// ---------------------------------------------------------------------------
// nRF24 radio (DETWS_ENABLE_NRF24) - Nordic nRF24L01+ 2.4 GHz driver
// ---------------------------------------------------------------------------
//
// A radio driver that plugs into the gateway (DETWS_ENABLE_GATEWAY). The nRF24L01+ speaks
// an SPI command protocol (not plain register r/w) and needs a separate CE pin, so the
// driver runs over a caller-supplied SPI transfer + CE bus (nrf_bus). Its hardware pipe
// addressing means the "source address" of a received frame is the pipe number - no
// in-payload header, so there is no separate codec. Bridge received payloads northbound
// with det_gw_uplink(port, pipe, ...); the RF link needs the module to verify.
// See services/nrf24/nrf24.h.

/** @brief Enable the nRF24L01+ radio driver (default off). */
#ifndef DETWS_ENABLE_NRF24
#define DETWS_ENABLE_NRF24 0
#endif

/** @brief nRF24 fixed payload width in bytes (1..32; the chip's static payload size). */
#ifndef DETWS_NRF24_PAYLOAD
#define DETWS_NRF24_PAYLOAD 32
#endif

#if DETWS_ENABLE_NRF24 && (DETWS_NRF24_PAYLOAD < 1 || DETWS_NRF24_PAYLOAD > 32)
#error "DeterministicESPAsyncWebServer: DETWS_NRF24_PAYLOAD must be 1..32"
#endif

// ---------------------------------------------------------------------------
// EnOcean ESP3 (DETWS_ENABLE_ENOCEAN) - energy-harvesting 868 MHz serial codec
// ---------------------------------------------------------------------------
//
// A UART telegram codec for EnOcean's ESP3 (EnOcean Serial Protocol 3), the framing used
// by USB/serial EnOcean gateways (TCM 310 / USB 300): sync 0x55, a 4-byte header (data
// length, optional length, packet type) protected by CRC8, then data + optional data
// protected by a second CRC8. esp3_parse() frames one telegram out of a byte stream and
// verifies both CRCs; esp3_build() assembles one. Pure (no UART code - you feed it the
// serial bytes), so it is fully host-testable. See services/enocean/enocean.h.

/** @brief Enable the EnOcean ESP3 serial codec (default off). */
#ifndef DETWS_ENABLE_ENOCEAN
#define DETWS_ENABLE_ENOCEAN 0
#endif

/** @brief Reject an ESP3 telegram whose declared data length exceeds this (framing sanity). */
#ifndef DETWS_ENOCEAN_MAX_DATA
#define DETWS_ENOCEAN_MAX_DATA 512
#endif

#if DETWS_ENABLE_ENOCEAN && (DETWS_ENOCEAN_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DETWS_ENOCEAN_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// PN532 NFC (DETWS_ENABLE_PN532) - NXP PN532 NFC/RFID controller frame codec
// ---------------------------------------------------------------------------
//
// The NXP PN532 (I2C / SPI / HSU) command-frame protocol - a tag read/write bridged to an
// HTTP / MQTT event. The chip is driven by "normal information frames" (00 00 FF | LEN |
// LCS | TFI | PData | DCS | 00) with a length checksum and a data checksum, plus a 6-byte
// ACK frame. pn532_build_frame() / pn532_parse_frame() assemble and verify those frames
// (the per-command PData is the application's), and pn532_is_ack() detects the ACK. Pure -
// you carry the frame bytes over your I2C / SPI / UART - so it is fully host-testable.
// See services/pn532/pn532.h.

/** @brief Enable the PN532 NFC frame codec (default off). */
#ifndef DETWS_ENABLE_PN532
#define DETWS_ENABLE_PN532 0
#endif

/** @brief Reject a PN532 normal frame whose declared length exceeds this (framing sanity). */
#ifndef DETWS_PN532_MAX_DATA
#define DETWS_PN532_MAX_DATA 254
#endif

#if DETWS_ENABLE_PN532 && (DETWS_PN532_MAX_DATA < 1 || DETWS_PN532_MAX_DATA > 254)
#error "DeterministicESPAsyncWebServer: DETWS_PN532_MAX_DATA must be 1..254"
#endif

// ---------------------------------------------------------------------------
// Sigfox (DETWS_ENABLE_SIGFOX) - Wisol / Murata Sigfox modem AT-command codec
// ---------------------------------------------------------------------------
//
// Tiny low-power uplinks over the Sigfox 0G network. A Wisol / Murata Sigfox modem is
// driven by AT commands over UART: sigfox_build_uplink() formats an `AT$SF=<hex>` frame
// for a <= 12-byte payload, and sigfox_parse_response() classifies the modem's reply
// (OK / ERROR / still pending). Pure text-command codec - you carry it over your UART - so
// it is fully host-testable. See services/sigfox/sigfox.h.

/** @brief Enable the Sigfox AT-command codec (default off). */
#ifndef DETWS_ENABLE_SIGFOX
#define DETWS_ENABLE_SIGFOX 0
#endif

/** @brief Maximum Sigfox uplink payload (the network caps a message at 12 bytes). */
#ifndef DETWS_SIGFOX_MAX_PAYLOAD
#define DETWS_SIGFOX_MAX_PAYLOAD 12
#endif

#if DETWS_ENABLE_SIGFOX && (DETWS_SIGFOX_MAX_PAYLOAD < 1 || DETWS_SIGFOX_MAX_PAYLOAD > 12)
#error "DeterministicESPAsyncWebServer: DETWS_SIGFOX_MAX_PAYLOAD must be 1..12"
#endif

// ---------------------------------------------------------------------------
// Z-Wave (DETWS_ENABLE_ZWAVE) - Silicon Labs Z-Wave Serial API frame codec
// ---------------------------------------------------------------------------
//
// The host-side Serial API of a Silicon Labs 500 / 700-series Z-Wave controller over UART:
// a Z-Wave mesh bridged to the web. Data frames are SOF (0x01) | LEN | Type | Command |
// Data | Checksum, where the checksum is 0xFF XOR-folded over LEN..last-data; single-byte
// ACK (0x06) / NAK (0x15) / CAN (0x18) frames flow-control them. zwave_build_frame() /
// zwave_parse_frame() assemble and verify a data frame; the per-command payload is the
// application's. Pure - you carry the bytes over your UART - so it is fully host-testable.
// See services/zwave/zwave.h.

/** @brief Enable the Z-Wave Serial API frame codec (default off). */
#ifndef DETWS_ENABLE_ZWAVE
#define DETWS_ENABLE_ZWAVE 0
#endif

/** @brief Reject a Z-Wave frame whose declared length exceeds this data cap (sanity). */
#ifndef DETWS_ZWAVE_MAX_DATA
#define DETWS_ZWAVE_MAX_DATA 64
#endif

#if DETWS_ENABLE_ZWAVE && (DETWS_ZWAVE_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DETWS_ZWAVE_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Zigbee (DETWS_ENABLE_ZIGBEE) - Silicon Labs EZSP / ASH serial framing codec
// ---------------------------------------------------------------------------
//
// The ASH (Asynchronous Serial Host) data-link layer that carries EZSP frames to a Silicon
// Labs EmberZNet NCP over UART - a Zigbee network bridged to the web. ASH delimits frames
// with a Flag byte (0x7E), byte-stuffs the reserved control bytes, and protects each frame
// with a CRC-16/CCITT. ash_frame_encode() wraps a control byte + payload into a stuffed,
// CRC'd frame; ash_frame_decode() unstuffs + verifies one. The EZSP command payload the
// frame carries (version, stack status, an incoming APS message, ...) is the application's.
// ash_frame_decode() removes the stuffing and verifies the CRC. Pure - you carry the bytes
// over your UART - so it is fully host-testable. See services/zigbee/zigbee.h.

/** @brief Enable the Zigbee EZSP / ASH framing codec (default off). */
#ifndef DETWS_ENABLE_ZIGBEE
#define DETWS_ENABLE_ZIGBEE 0
#endif

/** @brief Max ASH payload bytes (an EZSP frame; the ASH data field caps near 128). */
#ifndef DETWS_ZIGBEE_MAX_DATA
#define DETWS_ZIGBEE_MAX_DATA 128
#endif

#if DETWS_ENABLE_ZIGBEE && (DETWS_ZIGBEE_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DETWS_ZIGBEE_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Thread (DETWS_ENABLE_THREAD) - OpenThread spinel over HDLC-lite framing codec
// ---------------------------------------------------------------------------
//
// The HDLC-lite framing that carries spinel frames to an OpenThread radio co-processor
// (RCP: an nRF52840 / EFR32) over UART - an 802.15.4 / Thread mesh bridged to IP / the web.
// Each spinel frame is wrapped by HDLC-lite: an FCS (CRC-16/X-25) is appended, the reserved
// bytes are byte-stuffed, and a Flag byte (0x7E) terminates it. spinel_frame_encode() /
// spinel_frame_decode() do the framing + FCS; the spinel command inside (a property
// get/set/insert, a stream frame) is the application's. Pure - you carry the bytes over your
// UART - so it is fully host-testable. See services/thread/thread.h.

/** @brief Enable the Thread spinel / HDLC-lite framing codec (default off). */
#ifndef DETWS_ENABLE_THREAD
#define DETWS_ENABLE_THREAD 0
#endif

/** @brief Max spinel payload bytes carried in one HDLC-lite frame. */
#ifndef DETWS_THREAD_MAX_DATA
#define DETWS_THREAD_MAX_DATA 256
#endif

#if DETWS_ENABLE_THREAD && (DETWS_THREAD_MAX_DATA < 1)
#error "DeterministicESPAsyncWebServer: DETWS_THREAD_MAX_DATA must be >= 1"
#endif

// ---------------------------------------------------------------------------
// Wired Ethernet PHY (DETWS_ENABLE_ETHERNET) - run the server over an RMII PHY
// ---------------------------------------------------------------------------
//
// Bring up a wired Ethernet link (an RMII PHY: LAN8720 / TLK110 / RTL8201 / DP83848) so the
// server runs over Ethernet instead of (or alongside) Wi-Fi. init_eth_physical() is a thin
// wrapper over the Arduino ETH library; the PHY pins / type / clock come from the standard
// ETH_PHY_* build flags for your board (see example 19.Ethernet). The egress reporting
// (det_net_egress -> DetIface::DETIFACE_ETH) and the per-route interface classifier already handle a
// wired route, so once the link has an IP the server accepts on it with no other change.
// Default off (zero cost / the ETH library is not linked). ESP32-only.

/** @brief Enable wired Ethernet bring-up (init_eth_physical / eth_ready). Default off. */
#ifndef DETWS_ENABLE_ETHERNET
#define DETWS_ENABLE_ETHERNET 0
#endif

/**
 * @brief Enable IPv6 on the network interface (dual-stack). Default off.
 *
 * When set, init_ipv6_physical() turns on IPv6 for the Wi-Fi netif (SLAAC link-local plus any
 * router-advertised global address). The TCP and UDP listeners already bind IPADDR_TYPE_ANY, so
 * the server accepts IPv6 connections the moment the interface has a v6 address; the DetIp core
 * (network_drivers/network/ip.h) parses / formats / classifies both families. Requires an
 * lwIP built with LWIP_IPV6=1 (the stock Arduino-ESP32 core ships it).
 */
#ifndef DETWS_ENABLE_IPV6
#define DETWS_ENABLE_IPV6 0
#endif

/**
 * @brief Wi-Fi promiscuous (monitor) capture (DETWS_ENABLE_PROMISC). Default off.
 *
 * Passive 802.11 sniffing: promisc_begin() puts the radio in promiscuous mode on a channel and
 * delivers every frame to a sink (services/promisc). Wire that sink into the forwarding plane
 * (DETWS_ENABLE_FORWARD) to bridge captured Wi-Fi frames to another interface - e.g. stream them
 * to a wired collector over Ethernet. Ships a pure 802.11 header parser and libpcap framing
 * (DLT_IEEE802_11) so a forwarded frame is a valid PCAP a wired Wireshark / tcpdump can read.
 */
#ifndef DETWS_ENABLE_PROMISC
#define DETWS_ENABLE_PROMISC 0
#endif

/**
 * @brief Wired field-bus listen-only capture (DETWS_ENABLE_BUS_CAPTURE). Default off.
 *
 * The wired counterpart to promiscuous Wi-Fi capture: bus_capture_begin() installs the CAN (TWAI)
 * controller in listen-only mode - it decodes every frame on the bus but never ACKs or transmits,
 * so it stays invisible - and delivers each CanFrame to a sink (services/bus_capture). Wire the
 * sink into the forwarding plane (DETWS_ENABLE_FORWARD) to bridge captured CAN frames to another
 * interface. can_to_socketcan() formats a frame as a Linux SocketCAN frame so, with the libpcap
 * DLT_CAN_SOCKETCAN link type, the stream is a capture Wireshark reads.
 */
#ifndef DETWS_ENABLE_BUS_CAPTURE
#define DETWS_ENABLE_BUS_CAPTURE 0
#endif

// Feature / service / codec tuning knobs are consolidated at the END of this file,
// under "Feature tuning knobs (grouped and gated by feature)" - placed there so every
// DETWS_ENABLE_* flag is already resolved and each group can gate on its own feature.

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
 * qop, nc, cnonce) is far longer than MAX_VAL_LEN, so when DETWS_ENABLE_AUTH
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
#ifndef DETWS_DIGEST_NONCE_LIFETIME_MS
#define DETWS_DIGEST_NONCE_LIFETIME_MS (5u * 60u * 1000u)
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
 * DetWebServer::use()). Costs MAX_MIDDLEWARE pointers of BSS; an empty chain
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
 */
#ifndef CHUNK_BUF_SIZE
#define CHUNK_BUF_SIZE 256
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
 * sse_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
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
 * Smaller values reduce peak stack use; larger values improve throughput.
 * Must be <= RX_BUF_SIZE to avoid stalling the TCP send window.
 */
#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 512
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
 * blocking the lwIP thread.
 */
#ifndef EVT_QUEUE_DEPTH
#define EVT_QUEUE_DEPTH 16
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
 * @brief Size of the pre-built CORS header block stored in DetWebServer.
 *
 * Built once by set_cors() and injected into every response.  Must hold
 * Access-Control-Allow-Origin, Access-Control-Allow-Methods, and
 * Access-Control-Allow-Headers lines for the configured origin.
 */
#ifndef CORS_HDR_BUF_SIZE
#define CORS_HDR_BUF_SIZE 192
#endif

/**
 * @brief Size of the optional Cache-Control header line stored in DetWebServer.
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
//   #define DETWS_ENABLE_WEBSOCKET 0
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
//   WEBHOOK   + HTTP_CLIENT : without it, detws_webhook_post() returns -1
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
#ifndef DETWS_ENABLE_WEBSOCKET
#define DETWS_ENABLE_WEBSOCKET 1
#endif

/**
 * @brief WebSocket permessage-deflate (RFC 7692) - bidirectional compression.
 *
 * When set (and DETWS_ENABLE_WEBSOCKET is on), the server negotiates the
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
#ifndef DETWS_ENABLE_WS_DEFLATE
#define DETWS_ENABLE_WS_DEFLATE 0
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
#ifndef DETWS_WS_FRAG_SIZE
#define DETWS_WS_FRAG_SIZE 0
#endif

/** @brief Server-Sent Events push support. */
#ifndef DETWS_ENABLE_SSE
#define DETWS_ENABLE_SSE 1
#endif

/** @brief multipart/form-data body parser. */
#ifndef DETWS_ENABLE_MULTIPART
#define DETWS_ENABLE_MULTIPART 1
#endif

/**
 * @brief Zero-heap CBOR (RFC 8949) encoder for compact binary payloads.
 *
 * Default off. When set, network_drivers/presentation/cbor/cbor.h provides a writer
 * that serializes ints, strings, byte strings, arrays, maps, booleans, null, and
 * float32 into a caller-provided buffer - a compact binary alternative to the JSON
 * writer for telemetry. Pure, no heap, host-tested against the RFC 8949 vectors.
 */
#ifndef DETWS_ENABLE_CBOR
#define DETWS_ENABLE_CBOR 0
#endif

/**
 * @brief Zero-heap MessagePack encoder and decoder for compact binary payloads.
 *
 * Default off. When set, network_drivers/presentation/msgpack/msgpack.h provides a
 * writer that serializes ints, strings, byte strings, arrays, maps, booleans, nil,
 * and float32 into a caller-provided buffer, plus a cursor decoder (msgpack_peek /
 * msgpack_read_*, no-copy strings) over a caller buffer - the MessagePack-format
 * sibling of the CBOR / JSON readers and writers. Pure, no heap, host-tested
 * against the spec encodings and round-trip.
 */
#ifndef DETWS_ENABLE_MSGPACK
#define DETWS_ENABLE_MSGPACK 0
#endif

/** @brief Static file serving via Arduino FS (LittleFS, SPIFFS, SD). */
#ifndef DETWS_ENABLE_FILE_SERVING
#define DETWS_ENABLE_FILE_SERVING 1
#endif

/**
 * @brief WebDAV server (RFC 4918, class 1 + advisory locks) over the file system.
 *
 * Default off. When set (requires DETWS_ENABLE_FILE_SERVING), dav() mounts an FS
 * subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1),
 * PROPPATCH, GET, HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK -
 * so a client (rclone, cadaver, curl, or a mounted network drive) can browse and
 * edit files. PROPFIND returns a 207 Multi-Status document built into a fixed
 * buffer (DETWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at
 * DETWS_WEBDAV_MAX_ENTRIES children. PROPPATCH returns a 207 with each requested
 * property refused 403 Forbidden (the live properties are read-only, no dead-
 * property store) - this keeps Windows Explorer / macOS Finder, which PROPPATCH a
 * timestamp right after a PUT, from erroring on a 405. PUT streams the request
 * body straight to the file (via the shared streaming-body sink), so an upload is
 * not bounded by BODY_BUF_SIZE. Locks are advisory (a synthetic token is issued
 * but not enforced). See docs/SECURITY.md before exposing it.
 */
#ifndef DETWS_ENABLE_WEBDAV
#define DETWS_ENABLE_WEBDAV 0
#endif

/** @brief Buffer (BSS) for a WebDAV 207 Multi-Status response, in bytes (see DETWS_ENABLE_WEBDAV). */
#ifndef DETWS_WEBDAV_BUF_SIZE
#define DETWS_WEBDAV_BUF_SIZE 2048
#endif

/** @brief Maximum children listed in a WebDAV Depth-1 PROPFIND (bounds the response). */
#ifndef DETWS_WEBDAV_MAX_ENTRIES
#define DETWS_WEBDAV_MAX_ENTRIES 32
#endif

/** @brief Maximum properties echoed in a WebDAV PROPPATCH 207 response (bounds the response). */
#ifndef DETWS_WEBDAV_MAX_PROPS
#define DETWS_WEBDAV_MAX_PROPS 16
#endif

/**
 * @brief HTTP method-token buffer size (bytes, including the NUL).
 *
 * Sized for the longest method the server must recognize: 8 normally (OPTIONS),
 * grown to fit the WebDAV methods (PROPPATCH is 9 chars) when WebDAV is enabled.
 */
#ifndef DETWS_METHOD_BUF_SIZE
#if DETWS_ENABLE_WEBDAV
#define DETWS_METHOD_BUF_SIZE 12
#else
#define DETWS_METHOD_BUF_SIZE 8
#endif
#endif

/** @brief HTTP Basic Authentication per-route. */
#ifndef DETWS_ENABLE_AUTH
#define DETWS_ENABLE_AUTH 1
#endif

/** @brief Telnet server support (RFC 854 / IAC option negotiation). */
#ifndef DETWS_ENABLE_TELNET
#define DETWS_ENABLE_TELNET 0
#endif

/** @brief SSH server support (RFC 4253/4252/4254). */
#ifndef DETWS_ENABLE_SSH
#define DETWS_ENABLE_SSH 0
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
 * and is notified of client writes via modbus_on_write(). Modbus has no
 * authentication or encryption - run it only on a trusted control network.
 */
#ifndef DETWS_ENABLE_MODBUS
#define DETWS_ENABLE_MODBUS 0
#endif

/**
 * @brief Modbus RTU framing (serial / RS-485) over the same data model + PDU dispatch.
 *
 * Default off; implies DETWS_ENABLE_MODBUS. Adds the RTU ADU codec
 * `modbus_rtu_process_adu()` - a `[slave addr][PDU][CRC16]` frame (CRC16-Modbus,
 * little-endian) around the existing host-tested PDU dispatch: a CRC mismatch or a
 * non-matching unit address is dropped silently (no reply, per the spec), and a
 * broadcast (address 0) is executed without a reply. The codec is pure and
 * host-tested; feed it from a UART/RS-485 driver (the serial transport is the
 * application's, framed by the 3.5-char inter-frame idle).
 */
#ifndef DETWS_ENABLE_MODBUS_RTU
#define DETWS_ENABLE_MODBUS_RTU 0
#endif
#if DETWS_ENABLE_MODBUS_RTU && !DETWS_ENABLE_MODBUS
#undef DETWS_ENABLE_MODBUS
#define DETWS_ENABLE_MODBUS 1
#endif

/**
 * @brief CloudEvents v1.0 (CNCF) event envelope (structured JSON + binary headers).
 *
 * Default off. Adds `services/cloudevents`: `cloudevents_build_json()` emits a
 * structured `application/cloudevents+json` envelope (required `id`/`source`/`type`
 * + optional `subject`/`datacontenttype`/`data`) via the JSON writer, and
 * `cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers.
 * Makes the device's events interoperable with serverless / event-mesh consumers.
 */
#ifndef DETWS_ENABLE_CLOUDEVENTS
#define DETWS_ENABLE_CLOUDEVENTS 0
#endif

/**
 * @brief Redis RESP2 wire codec (`services/redis_resp`).
 *
 * Default off. A zero-heap command encoder (`resp_encode_command`, array of bulk
 * strings) + a cursor reply parser (`resp_parse`: simple / error / integer / bulk /
 * array / nil) so the device can drive a Redis server over the shipped outbound
 * client transport. Pure codec, host-tested; the connection is the application's.
 */
#ifndef DETWS_ENABLE_REDIS
#define DETWS_ENABLE_REDIS 0
#endif

/**
 * @brief STOMP 1.2 frame codec (`services/stomp`).
 *
 * Default off. A zero-heap frame builder (`stomp_build_frame`, command + escaped
 * headers + NUL-terminated body) + a non-mutating parser (`stomp_parse_frame`,
 * command / header slices / body, honoring `content-length`) so the device can talk
 * to a STOMP broker (ActiveMQ / RabbitMQ / Artemis) over the shipped outbound client
 * transport, or STOMP-over-WebSocket via the WS client. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_STOMP
#define DETWS_ENABLE_STOMP 0
#endif

/** @brief Max header lines parsed per STOMP frame (extras beyond this are ignored). */
#ifndef DETWS_STOMP_MAX_HEADERS
#define DETWS_STOMP_MAX_HEADERS 16
#endif

/**
 * @brief MQTT-SN v1.2 wire codec (`services/mqtt/mqtt_sn`).
 *
 * Default off. A zero-heap message builder + parser for MQTT for Sensor Networks - the
 * UDP / non-TCP MQTT variant for constrained, lossy links (numeric topic IDs instead of
 * strings, gateway discovery, sleeping-client keep-alive). Builds CONNECT / REGISTER /
 * PUBLISH / SUBSCRIBE / PINGREQ / DISCONNECT / SEARCHGW and parses CONNACK / REGACK /
 * PUBACK / SUBACK / PUBLISH / REGISTER, including the 1- and 3-octet Length forms. Pure
 * codec, host-tested; the datagram send (det_udp_sendto) and topic registry are the app's.
 */
#ifndef DETWS_ENABLE_MQTT_SN
#define DETWS_ENABLE_MQTT_SN 0
#endif

/**
 * @brief Flow-record export codec (`services/flow_export`).
 *
 * Default off. A zero-heap exporter-side codec for on-device flow accounting: NetFlow v5
 * (fixed 24-octet header + 48-octet records), NetFlow v9 (RFC 3954), and IPFIX (RFC 7011),
 * the latter two via a small cursor that emits a Template then matching Data records and
 * patches the message length (IPFIX) or record count (v9) on finish. Pure codec,
 * host-tested; the flow cache (5-tuple + counters) and the UDP send (det_udp_sendto) are
 * the application's. Pairs with the telemetry / observability services.
 */
#ifndef DETWS_ENABLE_FLOW_EXPORT
#define DETWS_ENABLE_FLOW_EXPORT 0
#endif

/**
 * @brief Protocol Buffers wire codec (`services/protobuf`).
 *
 * Default off. A zero-heap streaming Protobuf encoder + cursor reader over caller buffers
 * (the same shape as the CBOR / MessagePack codecs): varint / ZigZag / fixed32 / fixed64 /
 * length-delimited fields, with embedded messages built into a sub-buffer and added via
 * `pb_bytes`. Pure codec, host-tested against the spec vectors. This is the standalone
 * Protobuf deliverable; gRPC (framed Protobuf over HTTP/2) is gated on the HTTP/2 item.
 */
#ifndef DETWS_ENABLE_PROTOBUF
#define DETWS_ENABLE_PROTOBUF 0
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
#ifndef DETWS_ENABLE_WAMP
#define DETWS_ENABLE_WAMP 0
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
#ifndef DETWS_ENABLE_SUNSPEC
#define DETWS_ENABLE_SUNSPEC 0
#endif

/**
 * @brief IEEE C37.118.2 synchrophasor frame codec (`services/c37118`).
 *
 * Default off. A zero-heap builder + CRC-validating parser for the PMU / PDC wide-area
 * measurement wire protocol: `c37118_build_frame` / `c37118_build_command` emit a
 * `SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK` frame (CHK = CRC-CCITT) and
 * `c37118_parse_frame` validates the CRC and reports the frame type / ids / timestamp /
 * payload slice. Frames any payload and fully handles the fixed Command frame. Pure codec,
 * host-tested.
 */
#ifndef DETWS_ENABLE_C37118
#define DETWS_ENABLE_C37118 0
#endif

/**
 * @brief DNP3 (IEEE 1815) data-link frame codec (`services/dnp3`).
 *
 * Default off. A zero-heap builder + CRC-validating parser for the SCADA / utility
 * outstation data-link layer: `dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC`
 * header block + the CRC'd 16-octet user-data blocks, and `dnp3_parse_frame` validates the
 * header and every block CRC (CRC-16/DNP) and de-blocks the user data. Pure codec,
 * host-tested; the transport-function reassembly and the application layer are layered on
 * the de-blocked user data.
 */
#ifndef DETWS_ENABLE_DNP3
#define DETWS_ENABLE_DNP3 0
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
#ifndef DETWS_ENABLE_CANOPEN
#define DETWS_ENABLE_CANOPEN 0
#endif

/**
 * @brief SAE J1939 message codec (`services/j1939`).
 *
 * Default off. A zero-heap codec for the heavy-duty-vehicle / agriculture / marine / genset
 * CAN higher-layer protocol over 29-bit extended frames (`shared_primitives/can.h`):
 * `j1939_encode_id` / `j1939_decode_id` pack and unpack the priority / PGN / SA / DA
 * identifier (PDU1 peer + PDU2 broadcast), `j1939_build_message` emits single frames,
 * `j1939_build_request` / `j1939_build_address_claim` (+ `j1939_build_name`) handle the
 * Request PGN and Address Claimed messages, and the Transport Protocol (BAM announce +
 * TP.DT packets) reassembles multi-packet messages up to `DETWS_J1939_TP_MAX` octets. Pure
 * codec, host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI.
 */
#ifndef DETWS_ENABLE_J1939
#define DETWS_ENABLE_J1939 0
#endif

/**
 * @brief DeviceNet link-adaptation codec (`services/devicenet`).
 *
 * Default off. The CAN-specific layer of "CIP over CAN": the 11-bit DeviceNet identifier as a
 * Message Group (1..4) + Message ID + MAC ID (`devicenet_encode_id` / `devicenet_decode_id`),
 * the explicit-message header octet, single-frame explicit messages, and the fragmentation
 * protocol with a reassembler (`devicenet_frag_feed`) for bodies longer than one CAN frame.
 * The CIP application layer (services / EPATH / data) is the same one EtherNet/IP uses, so
 * build the body with the existing `cip_*` functions (`DETWS_ENABLE_CIP`). Pure codec,
 * host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI.
 */
#ifndef DETWS_ENABLE_DEVICENET
#define DETWS_ENABLE_DEVICENET 0
#endif

/**
 * @brief NMEA 2000 codec (`services/nmea2000`).
 *
 * Default off; implies DETWS_ENABLE_J1939 (NMEA 2000 is J1939 at the transport layer). A
 * zero-heap codec for the marine instrumentation network over CAN: it reuses the J1939 29-bit
 * identifier codec and adds the NMEA-specific Fast Packet transport - `n2k_fastpacket_build_frame`
 * splits a 9..223-octet message across frames (a control octet of sequence + frame counter,
 * the first frame carrying the total length) and `n2k_fastpacket_feed` reassembles it;
 * `n2k_build_single` wraps a single-frame message. Pure codec, host-tested. Drive it from the
 * ESP32 TWAI peripheral or an MCP2515 over SPI to bridge an NMEA 2000 backbone onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_NMEA2000
#define DETWS_ENABLE_NMEA2000 0
#endif
#if DETWS_ENABLE_NMEA2000 && !DETWS_ENABLE_J1939
#undef DETWS_ENABLE_J1939
#define DETWS_ENABLE_J1939 1 // NMEA 2000 reuses the J1939 identifier codec
#endif

/**
 * @brief Wired M-Bus (Meter-Bus, EN 13757) frame codec (`services/mbus`).
 *
 * Default off. A zero-heap builder + parser for the M-Bus link-layer frames used by utility
 * meters (water / gas / heat / electricity): the single-character ACK, the short frame
 * (`10 C A CS 16`), and the long / control frame (`68 L L 68 C A CI ... CS 16`, 8-bit sum
 * checksum), plus `mbus_record_next` which walks the EN 13757-3 variable-data records
 * (DIF / VIF, skipping DIFE / VIFE extension chains and decoding the data length). Pure codec,
 * host-tested. Talk to the powered two-wire bus over a UART through an M-Bus level converter
 * (e.g. a TSS721-based master) and bridge meter readings onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_MBUS
#define DETWS_ENABLE_MBUS 0
#endif

/**
 * @brief IEC 60870-5-101 / -104 telecontrol (SCADA) codec (`services/iec60870`).
 *
 * Default off. The utility-SCADA protocol in both transports: the -104 APCI over TCP
 * (`68 LEN` + 4 control octets in I / S / U formats via `iec104_build_i/_s/_u` + `iec104_parse`),
 * the shared ASDU header + 3-octet Information Object Address (`iec_asdu_build_header` /
 * `iec_asdu_parse_header`, `iec_put_ioa` / `iec_get_ioa`), and the -101 FT1.2 serial link
 * frames (fixed + variable, 8-bit sum checksum, via `iec101_build_fixed` / `_variable` +
 * `iec101_parse`). Named type-id / cause-of-transmission constants are provided; the
 * per-type information elements are the application's. Pure codec, host-tested. Run -104 over
 * the shipped TCP stack or -101 over a UART/RS-485 transceiver to bridge an RTU onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_IEC60870
#define DETWS_ENABLE_IEC60870 0
#endif

/**
 * @brief SDI-12 sensor-bus codec (`services/sdi12`).
 *
 * Default off. A zero-heap command / response codec for the 1200-baud single-wire ASCII bus
 * used by environmental / agricultural sensors: builders for the standard commands
 * (`sdi12_build_measure` / `_concurrent` / `_data` / `_identify` / `_change_address` /
 * `_query_address`), a parser for the measurement response (`sdi12_parse_measure`: seconds
 * until ready + value count), a data-value splitter (`sdi12_parse_values`), and the SDI-12
 * CRC (`sdi12_crc16` / `sdi12_crc_encode` / `sdi12_check_crc`) for the CRC-protected `aMC!` /
 * `aCC!` variants. Pure codec, host-tested. Drive the single 1200-baud line over a UART (with
 * a small level / direction circuit) and bridge sensor readings onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_SDI12
#define DETWS_ENABLE_SDI12 0
#endif

/**
 * @brief DMX512 + RDM (ANSI E1.20) lighting codec (`services/dmx`).
 *
 * Default off. A zero-heap codec for stage / architectural lighting over RS-485: `dmx_build` /
 * `dmx_get_channel` assemble and read the positional DMX512 slot packet (a start code + up to
 * 512 channels), and the RDM (Remote Device Management) functions build / parse the addressed
 * management packet that shares the wire - `rdm_build` / `rdm_parse` with 48-bit source /
 * destination UIDs (`rdm_uid`), a command class + parameter id, and the 16-bit additive
 * checksum (`rdm_checksum`). Pure codec, host-tested. Drive a `MAX485`-class transceiver on a
 * UART (250 kbit/s, 8N2; the break is the application's) and bridge a lighting rig onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_DMX
#define DETWS_ENABLE_DMX 0
#endif

/**
 * @brief NMEA 0183 sentence codec (`services/nmea0183`).
 *
 * Default off. A zero-heap codec for the marine / GPS ASCII protocol (`$GPGGA,...*47`):
 * `nmea0183_build` emits a sentence (adding the `$`, XOR checksum, and CR/LF), `nmea0183_parse`
 * validates the `*HH` checksum and splits the comma-separated fields (deriving talker id +
 * sentence type from the address field), and `nmea0183_field_float` / `_int` decode field
 * values. Sentence framing + checksum verified against the NMEA 0183 standard (the canonical
 * GGA example); pure and host-tested. GPS / marine receivers are cheap UART breakouts, so this
 * is a plain HardwareSerial link (4800 / 9600 baud); bridge position / wind / depth onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_NMEA0183
#define DETWS_ENABLE_NMEA0183 0
#endif

/**
 * @brief IO-Link (SDCI, IEC 61131-9) data-link message codec (`services/iolink`).
 *
 * Default off. The point-to-point smart-sensor link's data-link message layer: the M-sequence
 * Control octet (`iol_mc` + decoders), the checksum / type octet of a master message
 * (`iol_ckt`), the checksum / status octet of a device reply (`iol_cks`), and the SDCI message
 * checksum (`iol_checksum6` / `iol_finalize` / `iol_verify`) implemented straight from IO-Link
 * spec v1.1.4 Annex A.1.6 (the 0x52 seed + the 8-to-6-bit compression of equation A.1). Lay
 * the M-sequence / ISDU octets out per your device profile, then finalize / verify with this
 * codec. Pure codec, host-tested. The wire is a UART through an IO-Link transceiver
 * (e.g. MAX14819 / L6360); bridge sensor data onto Wi-Fi.
 */
#ifndef DETWS_ENABLE_IOLINK
#define DETWS_ENABLE_IOLINK 0
#endif

/**
 * @brief gRPC-Web message framing (`services/grpcweb`).
 *
 * Default off. A zero-heap length-prefixed frame builder + parser for gRPC-Web, the
 * HTTP/1.1-reachable subset of gRPC (gRPC proper needs HTTP/2). `grpcweb_frame_message`
 * wraps a Protobuf message in the 5-octet `[flags][len BE32]` prefix, `grpcweb_frame_trailer`
 * emits the 0x80 trailers frame (`grpc-status` / `grpc-message`), and `grpcweb_parse` reads
 * one frame back. Wraps the Protobuf codec (`DETWS_ENABLE_PROTOBUF`) over the shipped
 * HTTP/1.1 server/client. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_GRPC_WEB
#define DETWS_ENABLE_GRPC_WEB 0
#endif

/**
 * @brief OMA LwM2M TLV codec (`services/lwm2m`).
 *
 * Default off. A zero-heap writer + cursor reader for the LwM2M `application/vnd.oma.lwm2m+tlv`
 * resource encoding (Type / Identifier / Length / Value, 8-/16-bit ids, 0-/8-/16-/24-bit
 * lengths), carried over the shipped CoAP service for device management. Value helpers for
 * shortest-form integers, booleans, strings, and floats. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_LWM2M
#define DETWS_ENABLE_LWM2M 0
#endif

/**
 * @brief Omron FINS frame codec (`services/fins`).
 *
 * Default off. A zero-heap command/response builder + parser for the Factory Interface
 * Network Service (FINS/UDP): `fins_build_command` / `fins_build_memory_area_read` emit the
 * 10-octet routing header + command code + parameters, and `fins_parse_command` /
 * `fins_parse_response` read them back (the response end code MRES/SRES included). Talks to
 * an Omron PLC over the shipped UDP transport (det_udp_sendto). Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_FINS
#define DETWS_ENABLE_FINS 0
#endif

/**
 * @brief Omron Host Link (C-mode) frame codec (`services/hostlink`).
 *
 * Default off. A zero-heap ASCII command/response codec for the Omron serial host-link
 * protocol (the RS-232/485 sibling of FINS): `hostlink_build` emits `@UU` + header code +
 * text + FCS + `*`CR, and `hostlink_parse` FCS-validates and splits a frame
 * (`hostlink_end_code` reads a response's end code). FCS is the 8-bit XOR from `@` through
 * the text. Pure codec, host-tested; the serial transport is the application's.
 */
#ifndef DETWS_ENABLE_HOSTLINK
#define DETWS_ENABLE_HOSTLINK 0
#endif

/**
 * @brief SenML (RFC 8428) measurement-pack builder (`services/senml`).
 *
 * Default off; implies DETWS_ENABLE_CBOR (the SenML-CBOR form uses the CBOR writer). A
 * zero-heap SenML-JSON + SenML-CBOR encoder over the shipped JSON / CBOR codecs: the caller
 * fills a `SenmlRecord` array (base name/time, name, unit, one value, time) and
 * `senml_json_build` / `senml_cbor_build` emit the whole pack. Numbers are emitted as
 * integers when integral (so timestamps keep precision), else floats. The standard
 * measurement format for CoAP / LwM2M / HTTP telemetry. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_SENML
#define DETWS_ENABLE_SENML 0
#endif
#if DETWS_ENABLE_SENML && !DETWS_ENABLE_CBOR
#undef DETWS_ENABLE_CBOR
#define DETWS_ENABLE_CBOR 1
#endif

/**
 * @brief Allen-Bradley DF1 full-duplex frame codec (`services/df1`).
 *
 * Default off. A zero-heap framing + DLE byte-stuffing + BCC/CRC codec for the Rockwell
 * serial PLC data-link layer (pub. 1770-6.5.16): `df1_build_frame` wraps application data in
 * `DLE STX ... DLE ETX` with a doubled-DLE escape and a BCC (2's complement of the data sum)
 * or CRC-16 (over the data + ETX, low byte first), and `df1_parse_frame` validates the check
 * and un-stuffs the data. Pure codec, host-tested; the application header is the app's.
 */
#ifndef DETWS_ENABLE_DF1
#define DETWS_ENABLE_DF1 0
#endif

/**
 * @brief TPKT (RFC 1006) + COTP (X.224 class 0) frame codec (`services/cotp`).
 *
 * Default off. A zero-heap "ISO transport on TCP" framing codec - the reusable foundation
 * under S7comm and IEC 61850 MMS. `tpkt_build` / `tpkt_parse` handle the 4-octet TPKT
 * envelope; `cotp_build_dt` wraps user data in a Data TPDU, `cotp_build_cr` builds a
 * Connection Request (with the TPDU-size parameter + caller TSAP params), and `cotp_parse`
 * reports the TPDU type and the DT data / CR-CC refs. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_COTP
#define DETWS_ENABLE_COTP 0
#endif

/**
 * @brief Siemens S7comm PDU codec (`services/s7comm`).
 *
 * Default off. A zero-heap builder + parser for the S7-300/400 communication PDUs carried
 * inside a COTP Data TPDU (DETWS_ENABLE_COTP) over ISO-on-TCP (port 102): `s7_build_setup`
 * (Setup Communication), `s7_build_read_request` (Read Var, S7-ANY items over DB/I/Q/M),
 * `s7_parse_header`, and `s7_read_next_item` (the response data items, honoring the
 * length-in-bits transport sizes + even-item padding). Constants verified against the
 * Wireshark S7comm dissector. Pure codec, host-tested; wrap the PDU with COTP + TPKT.
 */
#ifndef DETWS_ENABLE_S7COMM
#define DETWS_ENABLE_S7COMM 0
#endif

/**
 * @brief Mitsubishi MELSEC MC protocol (binary 3E) codec (`services/melsec`).
 *
 * Default off. A zero-heap batch-read request builder + response parser for MELSEC PLCs over
 * TCP/UDP: `melsec_build_read` emits the binary 3E batch-read (word) frame (little-endian
 * fields, subheader 0x5000, command 0x0401, the device code + 24-bit head device + point
 * count), and `melsec_parse_response` validates the 0xD000 response and reports the end code
 * + the read data. Frame layout + device codes verified against a third-party MC impl. Pure
 * codec, host-tested. Completes the major-vendor PLC read set (FINS / Host Link / DF1 / S7).
 */
#ifndef DETWS_ENABLE_MELSEC
#define DETWS_ENABLE_MELSEC 0
#endif

/**
 * @brief BACnet/IP BVLC + NPDU codec (`services/bacnet`).
 *
 * Default off. A zero-heap framing codec for the ASHRAE 135 building-automation network
 * layer over UDP (47808): `bvlc_build` / `bvlc_parse` handle the BVLC envelope (type 0x81,
 * function, length), and `npdu_build` / `npdu_parse` handle the NPDU (version + NPCI control
 * + optional DNET/DADR destination addressing + hop count) and slice the APDU. The APDU
 * (application-layer services / object model) layers on top. Pure codec, host-tested.
 */
#ifndef DETWS_ENABLE_BACNET
#define DETWS_ENABLE_BACNET 0
#endif

/**
 * @brief EtherNet/IP encapsulation codec (`services/enip`).
 *
 * Default off. A zero-heap builder + parser for the ODVA EtherNet/IP encapsulation layer
 * (TCP/UDP 44818) that carries CIP: `eip_build` / `eip_parse` handle the 24-octet header
 * (little-endian command / length / session handle / status / sender context / options),
 * `eip_build_register_session` opens a session, and `eip_build_send_rr_data` /
 * `eip_parse_send_rr_data` wrap + unwrap a CIP message as an unconnected message (Common
 * Packet Format: Null Address + Unconnected Data items). Commands + CPF item types verified
 * against the Wireshark ENIP dissector. Pure codec, host-tested; the CIP message is the app's.
 */
#ifndef DETWS_ENABLE_ENIP
#define DETWS_ENABLE_ENIP 0
#endif

/**
 * @brief AMQP 0-9-1 frame codec (`services/amqp`).
 *
 * Default off. A zero-heap frame builder + parser for the RabbitMQ wire protocol so a device
 * can be an AMQP client: `amqp_protocol_header` (the `"AMQP" 0 0 9 1` preamble),
 * `amqp_build_frame` / `amqp_parse_frame` (type + channel + size + payload + the 0xCE
 * frame-end), `amqp_build_method` / `amqp_parse_method` (a METHOD frame's class-id /
 * method-id / arguments), and `amqp_build_heartbeat`. Pure codec, host-tested; the method
 * arguments and the connection are the application's. Rides the outbound client transport.
 */
#ifndef DETWS_ENABLE_AMQP
#define DETWS_ENABLE_AMQP 0
#endif

/**
 * @brief CIP (Common Industrial Protocol) message codec (`services/cip`).
 *
 * Default off. A zero-heap CIP request builder + response parser for the message that rides
 * inside an EtherNet/IP Unconnected Data item (DETWS_ENABLE_ENIP): `cip_build_epath` (the
 * class/instance/attribute logical-segment EPATH), `cip_build_request` /
 * `cip_build_get_attr_single`, and `cip_parse_response` (service / general status / data).
 * Service codes + the logical-segment encoding verified against the Wireshark CIP dissector.
 * Pure codec, host-tested; wrap the request with `eip_build_send_rr_data` for a working read.
 */
#ifndef DETWS_ENABLE_CIP
#define DETWS_ENABLE_CIP 0
#endif

/**
 * @brief NATS client protocol codec (`services/nats`).
 *
 * Default off. A zero-heap builder + parser for the text-based NATS pub/sub protocol so a
 * device can be a NATS client: `nats_build_connect` / `nats_build_pub` / `nats_build_sub` /
 * `nats_build_unsub` / `nats_build_ping` / `nats_build_pong`, and `nats_parse` which decodes
 * an inbound MSG / INFO / PING / PONG / +OK / -ERR (MSG yields subject / sid / reply-to /
 * payload). Line-oriented (CRLF), space-delimited; only PUB and MSG carry a payload. Pure
 * codec, host-tested; rides the outbound client transport.
 */
#ifndef DETWS_ENABLE_NATS
#define DETWS_ENABLE_NATS 0
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
#ifndef DETWS_ENABLE_PROXY_PROTOCOL
#define DETWS_ENABLE_PROXY_PROTOCOL 0
#endif

/**
 * @brief Sparkplug B payload + topic codec (`services/sparkplug`).
 *
 * Default off; implies DETWS_ENABLE_PROTOBUF (the payload is a Protobuf message). A zero-heap
 * builder for the Eclipse Sparkplug B industrial-IoT MQTT payload (`spb_build_payload` /
 * `spb_build_metric`, over the protobuf codec) and its topic namespace (`spb_build_topic`,
 * `spBv1.0/group/type/node[/device]`). Field numbers + datatype codes verified against the
 * Eclipse Tahu sparkplug_b.proto. Pure codec, host-tested; publish it with the MQTT client.
 */
#ifndef DETWS_ENABLE_SPARKPLUG
#define DETWS_ENABLE_SPARKPLUG 0
#endif
#if DETWS_ENABLE_SPARKPLUG && !DETWS_ENABLE_PROTOBUF
#undef DETWS_ENABLE_PROTOBUF
#define DETWS_ENABLE_PROTOBUF 1
#endif

/** @brief Max serialized size of one Sparkplug B metric submessage (stack temp, bytes). */
#ifndef DETWS_SPB_METRIC_MAX
#define DETWS_SPB_METRIC_MAX 256
#endif

/**
 * @brief Opt-in Modbus master codec + register scanner (DETWS_ENABLE_MODBUS_MASTER).
 *
 * Default off. services/modbus/modbus_master builds Modbus TCP read-request ADUs
 * and parses the responses (register values or exception), so an app can poll /
 * auto-discover a slave's registers. Pure and host-tested as a full round-trip
 * against the slave codec (modbus_process_adu); the actual send is the app's TCP.
 */
#ifndef DETWS_ENABLE_MODBUS_MASTER
#define DETWS_ENABLE_MODBUS_MASTER 0
#endif

/** @brief Number of Modbus coils (FC 1/5/15), single-bit R/W (BSS, bit-packed). */
#ifndef DETWS_MODBUS_COILS
#define DETWS_MODBUS_COILS 64
#endif

/** @brief Number of Modbus discrete inputs (FC 2), single-bit read-only (BSS, bit-packed). */
#ifndef DETWS_MODBUS_DISCRETE_INPUTS
#define DETWS_MODBUS_DISCRETE_INPUTS 64
#endif

/** @brief Number of Modbus holding registers (FC 3/6/16), 16-bit R/W (BSS). */
#ifndef DETWS_MODBUS_HOLDING_REGS
#define DETWS_MODBUS_HOLDING_REGS 64
#endif

/** @brief Number of Modbus input registers (FC 4), 16-bit read-only (BSS). */
#ifndef DETWS_MODBUS_INPUT_REGS
#define DETWS_MODBUS_INPUT_REGS 64
#endif

/**
 * @brief TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only).
 *
 * When set, the server can accept TLS connections using mbedTLS configured with
 * MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DETWS_TLS_ARENA_SIZE) -
 * no system heap, so the determinism guarantee is preserved. The TLS engine is
 * compiled only on Arduino/ESP32 (mbedTLS is not part of the native build).
 * Default off.
 */
#ifndef DETWS_ENABLE_TLS
#define DETWS_ENABLE_TLS 0
#endif

/** @brief Maximum simultaneous TLS connections (each holds mbedTLS record buffers). */
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 1
#endif

/**
 * @brief TLS session resumption via RFC 5077 session tickets (requires DETWS_ENABLE_TLS).
 *
 * Default off. When set, the TLS 1.2 server issues encrypted session tickets and
 * accepts them on reconnect, so a returning client completes an abbreviated
 * handshake (no certificate or full key exchange) - much faster and far less CPU
 * than the ~RSA/ECDHE full handshake. Resumption is stateless: the session state
 * lives in the client's ticket, sealed with a server-held key, so there is no
 * growing per-session cache (the determinism / zero-heap-growth guarantee holds;
 * only a small fixed ticket key and a little arena headroom are added). The ticket
 * key rotates automatically on the DETWS_TLS_TICKET_LIFETIME_S schedule. Needs the
 * mbedTLS build to provide MBEDTLS_SSL_TICKET_C (stock arduino-esp32 does).
 */
#ifndef DETWS_ENABLE_TLS_RESUMPTION
#define DETWS_ENABLE_TLS_RESUMPTION 0
#endif

/** @brief Session-ticket lifetime / key-rotation period in seconds (see DETWS_ENABLE_TLS_RESUMPTION). */
#ifndef DETWS_TLS_TICKET_LIFETIME_S
#define DETWS_TLS_TICKET_LIFETIME_S 86400
#endif

/**
 * @brief Mutual TLS - require and verify a client certificate (mTLS).
 *
 * Default off. When set (requires DETWS_ENABLE_TLS), the server can be given a
 * trust-anchor CA via DetWebServer::tls_require_client_cert(): the TLS handshake
 * then demands a client certificate chaining to that CA
 * (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents
 * none or an untrusted one. The verified peer's subject DN is available to
 * handlers via DetWebServer::tls_client_subject(). Strong transport-level client
 * authentication with no passwords.
 */
#ifndef DETWS_ENABLE_MTLS
#define DETWS_ENABLE_MTLS 0
#endif

/** @brief Maximum length of a verified mTLS peer subject DN string (incl. NUL). */
#ifndef DETWS_MTLS_SUBJECT_MAX
#define DETWS_MTLS_SUBJECT_MAX 128
#endif

/**
 * @brief SNMP agent (v1/v2c, + v3 USM when DETWS_ENABLE_SNMP_V3) over lwIP UDP.
 *
 * Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER
 * codec itself is gated by this flag and is otherwise unit-tested standalone
 * (env:native_snmp).
 */
#ifndef DETWS_ENABLE_SNMP
#define DETWS_ENABLE_SNMP 0
#endif

/** @brief Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off. */
#ifndef DETWS_ENABLE_SNMP_V3
#define DETWS_ENABLE_SNMP_V3 0
#endif

/**
 * @brief Outbound SNMP notifications - traps and informs (requires DETWS_ENABLE_SNMP).
 *
 * Default off. When set, src/services/snmp/snmp_notify.h sends SNMPv2c (and, with
 * DETWS_ENABLE_SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over
 * UDP - so the agent can push alerts instead of only answering polls. Reuses the
 * BER codec and the transport-layer UDP service; the PDU builder is host-testable.
 */
#ifndef DETWS_ENABLE_SNMP_TRAP
#define DETWS_ENABLE_SNMP_TRAP 0
#endif

/** @brief Maximum extra variable-bindings (beyond sysUpTime/snmpTrapOID) in one notification. */
#ifndef DETWS_SNMP_TRAP_MAX_VARBINDS
#define DETWS_SNMP_TRAP_MAX_VARBINDS 8
#endif

/** @brief Static datagram buffer for an outbound SNMP notification, bytes. */
#ifndef DETWS_SNMP_TRAP_BUF_SIZE
#define DETWS_SNMP_TRAP_BUF_SIZE 1024
#endif

/** @brief Maximum sub-identifiers (arcs) in an SNMP object identifier. */
#ifndef SNMP_MAX_OID_LEN
#define SNMP_MAX_OID_LEN 32
#endif

/**
 * @brief Maximum registered MIB objects (the agent's fixed OID table).
 *
 * Each entry holds its OID, a value descriptor, and optional get/set callbacks
 * (see src/services/snmp/snmp_agent.h). The table lives in BSS; entries are
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

/** @brief Default read-only community (overridable at runtime via snmp_agent_init). Deployments
 *  SHOULD change this from the RFC-1157 well-known "public" for anything but a closed network. */
#ifndef DETWS_SNMP_DEFAULT_RO_COMMUNITY
#define DETWS_SNMP_DEFAULT_RO_COMMUNITY "public"
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
// CoAP server sizing constants  (DETWS_ENABLE_COAP must be 1)
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
#ifndef DETWS_ENABLE_COAP
#define DETWS_ENABLE_COAP 0
#endif

/**
 * @brief CoAP resource observation - RFC 7641 (requires DETWS_ENABLE_COAP).
 *
 * Default off. When set, a client GET with the Observe option registers as an
 * observer of a resource; the application calls coap_notify(path) to push the
 * resource's current representation to every observer (a CoAP notification from
 * the server port with an increasing Observe sequence). Observers are dropped on
 * a deregister GET, a client RST, or send failure.
 */
#ifndef DETWS_ENABLE_COAP_OBSERVE
#define DETWS_ENABLE_COAP_OBSERVE 0
#endif

/** @brief Maximum simultaneous CoAP observers (one slot per observed resource per client). */
#ifndef DETWS_COAP_MAX_OBSERVERS
#define DETWS_COAP_MAX_OBSERVERS 4
#endif

/**
 * @brief CoAP block-wise transfer - RFC 7959 (requires DETWS_ENABLE_COAP).
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
#ifndef DETWS_ENABLE_COAP_BLOCK
#define DETWS_ENABLE_COAP_BLOCK 0
#endif

/** @brief Largest block-size exponent (SZX) the server will use: block size = 2^(SZX+4) bytes, SZX 0..6 (16..1024). */
#ifndef DETWS_COAP_BLOCK_SZX_MAX
#define DETWS_COAP_BLOCK_SZX_MAX 6
#endif

/**
 * @brief Reassembly buffer for a block-wise (Block1) request upload, in bytes.
 *
 * One buffer of this size lives in BSS only when DETWS_ENABLE_COAP_BLOCK is set.
 * It bounds the largest payload a chunked POST/PUT can deliver to a handler.
 */
#ifndef DETWS_COAP_BLOCK1_MAX
#define DETWS_COAP_BLOCK1_MAX 1024
#endif

/**
 * @brief Maximum registered CoAP resources (the server's fixed routing table).
 *
 * Each entry holds a path pointer, an allowed-methods bitmask, and a handler.
 * The table lives in BSS and is scanned linearly (small table).
 */
#ifndef DETWS_COAP_MAX_RESOURCES
#define DETWS_COAP_MAX_RESOURCES 8
#endif

/** @brief Maximum reconstructed Uri-Path length, including separators and the leading '/'. */
#ifndef DETWS_COAP_MAX_PATH
#define DETWS_COAP_MAX_PATH 64
#endif

/** @brief Maximum reconstructed Uri-Query length (segments joined by '&'). */
#ifndef DETWS_COAP_MAX_QUERY
#define DETWS_COAP_MAX_QUERY 64
#endif

/**
 * @brief Maximum CoAP request/response payload in bytes.
 *
 * Sizes the static scratch a handler writes its response body into and bounds
 * the request payload handed to it. One buffer of this size lives in BSS.
 */
#ifndef DETWS_COAP_MAX_PAYLOAD
#define DETWS_COAP_MAX_PAYLOAD 256
#endif

/**
 * @brief Static response-datagram buffer for the CoAP UDP server.
 *
 * One buffer of this size lives in BSS (the request is transport-owned). Must
 * hold a 4-byte header + token (<=8) + the Content-Format option + a 0xFF marker
 * + DETWS_COAP_MAX_PAYLOAD bytes. When block-wise transfer is enabled it must
 * also hold one full block (2^(DETWS_COAP_BLOCK_SZX_MAX+4) bytes) + option
 * overhead, so the default grows accordingly.
 */
#ifndef DETWS_COAP_MSG_BUF_SIZE
#if DETWS_ENABLE_COAP_BLOCK
#define DETWS_COAP_MSG_BUF_SIZE 1152
#else
#define DETWS_COAP_MSG_BUF_SIZE 512
#endif
#endif

/** @brief Default UDP port the CoAP observe transport notifies from (IANA well-known 5683). */
#ifndef DETWS_COAP_OBSERVE_PORT
#define DETWS_COAP_OBSERVE_PORT 5683
#endif

/**
 * @brief Bytes of the static BSS arena mbedTLS allocates from (DETWS_ENABLE_TLS).
 *
 * All mbedTLS allocations (per-connection record buffers, handshake temporaries,
 * cert/key parsing) are served from this fixed arena via a custom allocator
 * installed with mbedtls_platform_set_calloc_free() - never the system heap. Must
 * cover the worst-case handshake peak for MAX_TLS_CONNS; if undersized the
 * handshake fails cleanly (no corruption). Measured peak for ONE ECDSA P-256
 * connection on Arduino-esp32 (16 KB IN + 16 KB OUT records) is ~41.5 KB, so the
 * default leaves a small margin. An RSA cert/larger chain needs more; query the
 * live peak via det_tls_arena_peak(). NOTE: a second concurrent TLS connection
 * roughly doubles the record-buffer cost (~32 KB more), which overflows the
 * static DRAM budget - keep MAX_TLS_CONNS at 1 unless you shrink the IDF record
 * sizes (CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN, needs an ESP-IDF build).
 */
#ifndef DETWS_TLS_ARENA_SIZE
#define DETWS_TLS_ARENA_SIZE 49152
#endif

/**
 * @brief Place the TLS arena in external PSRAM instead of internal DRAM (ESP32).
 *
 * The internal static-DRAM ceiling (`dram0_0_seg`) is only ~122 KB, so a single
 * ~48 KB arena already uses a large slice and a second concurrent connection
 * (MAX_TLS_CONNS > 1) overflows it. On a board with PSRAM, set this to 1 to move
 * the arena to external RAM via `EXT_RAM_BSS_ATTR` / `EXT_RAM_ATTR`, freeing the
 * whole `DETWS_TLS_ARENA_SIZE` back to internal DRAM so many connections fit.
 * Requires `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` (and PSRAM enabled) in the
 * ESP-IDF/PlatformIO config; without it the attribute is a no-op and the arena
 * stays in DRAM (safe fallback). No effect on the native host build.
 */
#ifndef DETWS_TLS_ARENA_IN_PSRAM
#define DETWS_TLS_ARENA_IN_PSRAM 0
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
#ifndef DETWS_TLS_MAX_FRAG_LEN
#define DETWS_TLS_MAX_FRAG_LEN 0
#endif

/**
 * @brief Acknowledge that a MAX_TLS_CONNS > 1 build has been sized to fit.
 *
 * The whole TLS arena is static `.bss` and the internal `dram0_0_seg` ceiling is only
 * ~122 KB, so a second concurrent connection's arena overflows it on a stock build.
 * A validation guard (bottom of this file) therefore rejects MAX_TLS_CONNS > 1 unless
 * you have taken one of the paths in docs/KNOWN_LIMITATIONS.md - move the arena to
 * PSRAM (`DETWS_TLS_ARENA_IN_PSRAM`, which satisfies the guard on its own), shrink the
 * mbedTLS records in a custom ESP-IDF build, or reclaim internal DRAM - and then set
 * this to 1 to confirm the build was sized deliberately.
 */
#ifndef DETWS_TLS_ACK_MULTI_CONN_DRAM
#define DETWS_TLS_ACK_MULTI_CONN_DRAM 0
#endif

// ---------------------------------------------------------------------------
// Optional network services (ESP32-only thin wrappers; each default-off so it
// costs no code/RAM/flash unless explicitly enabled).
// ---------------------------------------------------------------------------

/** @brief mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS. */
#ifndef DETWS_ENABLE_MDNS
#define DETWS_ENABLE_MDNS 0
#endif

/** @brief SNTP wall-clock time sync via the ESP-IDF SNTP client. */
#ifndef DETWS_ENABLE_NTP
#define DETWS_ENABLE_NTP 0
#endif

/**
 * @brief NTP/SNTP time server (RFC 5905 / RFC 4330 server mode) on UDP/123 (services/ntp_server).
 *
 * Turns the device into a local time source: it answers client NTP requests from its own
 * clock (`detws_time_now()` + the `detws_millis()` sub-second fraction), so an offline or
 * air-gapped LAN can keep its devices in sync without reaching the public NTP pool. The
 * 48-byte response codec is pure and host-tested; the wire binding is the transport UDP
 * service. Get the device's own time first (e.g. DETWS_ENABLE_NTP upstream, an RTC, or GPS
 * via a time source) - when it has none, the server stays silent rather than serve bad time.
 */
#ifndef DETWS_ENABLE_NTP_SERVER
#define DETWS_ENABLE_NTP_SERVER 0
#endif

/** @brief Stratum the NTP server advertises (distance from a reference clock; 1-15). */
#ifndef DETWS_NTP_SERVER_STRATUM
#define DETWS_NTP_SERVER_STRATUM 3
#endif

/**
 * @brief Authoritative DNS server (services/dns_server) on UDP/53.
 *
 * Default off. Resolves a small fixed table of `name -> IPv4` A records you register with
 * dns_server_add(), so devices on an offline / air-gapped LAN can use names instead of raw
 * IPs (a companion to the NTP server for offline infrastructure). Answers A/IN queries from
 * the table, returns NXDOMAIN for unknown names, and ignores other query types. The response
 * builder is pure and host-tested; the wire binding is the transport UDP service. This is a
 * general resolver, distinct from the provisioning captive-portal DNS (which answers every
 * query with the softAP IP) - do not enable both (they both bind :53).
 */
#ifndef DETWS_ENABLE_DNS_SERVER
#define DETWS_ENABLE_DNS_SERVER 0
#endif

/** @brief Max A records in the DNS server's fixed table. */
#ifndef DETWS_DNS_SERVER_MAX_RECORDS
#define DETWS_DNS_SERVER_MAX_RECORDS 8
#endif

/** @brief TTL (seconds) the DNS server puts on its answers. */
#ifndef DETWS_DNS_SERVER_TTL
#define DETWS_DNS_SERVER_TTL 60
#endif

/** @brief Max length of a queried/stored DNS name (bytes, incl NUL). */
#ifndef DETWS_DNS_NAME_MAX
#define DETWS_DNS_NAME_MAX 128
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
 * by priority) when DETWS_ENABLE_TIME_SOURCE is set - register your sources with
 * detws_time_source_add() (rtc_time_source, ntp_time_source, ...). Otherwise it comes
 * straight from NTP (detws_ntp_http_date). Needs at least one such time source to emit.
 */
#ifndef DETWS_HTTP_EMIT_DATE
#define DETWS_HTTP_EMIT_DATE 0
#endif

/**
 * @brief Multi-source time fallback (NTP / RTC / GPS / ... by priority).
 *
 * When set, src/services/time_source/time_source.h provides a small registry of
 * user-defined time sources, each a callback returning Unix epoch seconds (0 when
 * that source has no valid time). detws_time_now() queries them in priority order
 * (lowest value first) and returns the first valid result, so the device falls
 * back automatically when its preferred clock is unavailable. Pure and zero-heap
 * (a fixed source table); host-testable. Default off.
 */
#ifndef DETWS_ENABLE_TIME_SOURCE
#define DETWS_ENABLE_TIME_SOURCE 0
#endif

/** @brief Maximum registered time sources (DETWS_ENABLE_TIME_SOURCE). */
#ifndef DETWS_TIME_SOURCE_MAX
#define DETWS_TIME_SOURCE_MAX 4
#endif

/**
 * @brief Shared I2C bus pins for the sensor / peripheral drivers (RTC, SHT3x, MPR121, ADS1115,
 * INA219, PCA9685). All of them share one bus via detws_i2c_begin() (services/i2c.h), so
 * this is the single place to move it. The default -1 uses the platform's default pins (GPIO 21
 * SDA / 22 SCL on the classic ESP32). Set both to free GPIOs when those pins are taken - most
 * importantly a **wired-Ethernet PHY**: the LAN8720 RMII uses GPIO 21 (TX_EN) and GPIO 22
 * (TXD1) on the classic ESP32 (WROOM/WROVER) and the ESP32-P4 (which have the RMII EMAC), so
 * with that Ethernet on, move the I2C bus off them (e.g. 32 / 33). The ESP32-S3/C3 have no RMII
 * MAC and use an SPI Ethernet (W5500) instead - relocate the bus off whatever SPI pins that
 * uses. UART peripherals (LD2410) take their RX/TX pins at ld2410_begin(), so remap those too.
 */
#ifndef DETWS_I2C_SDA_PIN
#define DETWS_I2C_SDA_PIN -1
#endif
#ifndef DETWS_I2C_SCL_PIN
#define DETWS_I2C_SCL_PIN -1
#endif

/**
 * @brief I2C real-time-clock driver (DS1307 / DS3231) - a battery-backed time source.
 *
 * Default off. services/rtc reads and sets a DS1307/DS3231 RTC over I2C (Wire), so the device
 * keeps accurate wall-clock time across reboots and power loss with no network - the ideal
 * fallback below GPS and above upstream NTP in a time-source chain (feeds `detws_time_now()`
 * and the NTP server). The BCD<->epoch conversion (7 time registers, 12/24-hour, leap years,
 * range validation) is pure and host-tested; only the register read/write touches I2C.
 */
#ifndef DETWS_ENABLE_RTC
#define DETWS_ENABLE_RTC 0
#endif

/** @brief I2C address of the RTC (DS1307/DS3231 are fixed at 0x68). */
#ifndef DETWS_RTC_I2C_ADDR
#define DETWS_RTC_I2C_ADDR 0x68
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
#ifndef DETWS_ENABLE_LD2410
#define DETWS_ENABLE_LD2410 0
#endif

/** @brief LD2410 UART baud rate (the module's fixed factory default is 256000). */
#ifndef DETWS_LD2410_BAUD
#define DETWS_LD2410_BAUD 256000
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
#ifndef DETWS_ENABLE_MPR121
#define DETWS_ENABLE_MPR121 0
#endif

/** @brief I2C address of the MPR121 (0x5A default; 0x5B/0x5C/0x5D via the ADDR pin). */
#ifndef DETWS_MPR121_I2C_ADDR
#define DETWS_MPR121_I2C_ADDR 0x5A
#endif

/** @brief MPR121 per-electrode touch threshold (delta counts from baseline; NXP AN3944 suggests ~4..12).
 *         Higher = less sensitive. Keep the release threshold below it for hysteresis. */
#ifndef DETWS_MPR121_TOUCH_THRESHOLD
#define DETWS_MPR121_TOUCH_THRESHOLD 12
#endif

/** @brief MPR121 per-electrode release threshold (delta counts; should be below the touch threshold). */
#ifndef DETWS_MPR121_RELEASE_THRESHOLD
#define DETWS_MPR121_RELEASE_THRESHOLD 6
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
#ifndef DETWS_ENABLE_SHT3X
#define DETWS_ENABLE_SHT3X 0
#endif

/** @brief I2C address of the SHT3x (0x44 with ADDR low; 0x45 with ADDR high). */
#ifndef DETWS_SHT3X_I2C_ADDR
#define DETWS_SHT3X_I2C_ADDR 0x44
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
#ifndef DETWS_ENABLE_PCA9685
#define DETWS_ENABLE_PCA9685 0
#endif

/** @brief I2C address of the PCA9685 (0x40 default; the six address pins select 0x40..0x7F). */
#ifndef DETWS_PCA9685_I2C_ADDR
#define DETWS_PCA9685_I2C_ADDR 0x40
#endif

/** @brief Default PWM output frequency in Hz (50 Hz suits hobby servos). */
#ifndef DETWS_PCA9685_FREQ
#define DETWS_PCA9685_FREQ 50
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
#ifndef DETWS_ENABLE_ADS1115
#define DETWS_ENABLE_ADS1115 0
#endif

/** @brief I2C address of the ADS1115 (0x48 with ADDR to GND; 0x49/0x4A/0x4B for VDD/SDA/SCL). */
#ifndef DETWS_ADS1115_I2C_ADDR
#define DETWS_ADS1115_I2C_ADDR 0x48
#endif

/** @brief Default ADS1115 PGA gain code (ADS1115_GAIN_*): 0=+/-6.144V, 1=+/-4.096V, 2=+/-2.048V (default),
 *         3=+/-1.024V, 4=+/-0.512V, 5=+/-0.256V. Also the fallback when a read passes an invalid gain. */
#ifndef DETWS_ADS1115_GAIN
#define DETWS_ADS1115_GAIN 2 // ADS1115_GAIN_2 (+/- 2.048 V)
#endif

/** @brief Default ADS1115 data-rate code (ADS1115_DR_*): 0=8, 1=16, 2=32, 3=64, 4=128 (default), 5=250,
 *         6=475, 7=860 SPS. The single-shot read waits the matching conversion time. */
#ifndef DETWS_ADS1115_DR
#define DETWS_ADS1115_DR 4 // ADS1115_DR_128 (128 SPS)
#endif

/** @brief ADS1115 input mode: 0 = single-ended (AINx vs GND), 1 = differential. In differential mode the
 *         channel selects the pair: 0=AIN0-AIN1, 1=AIN0-AIN3, 2=AIN1-AIN3, 3=AIN2-AIN3. */
#ifndef DETWS_ADS1115_DIFFERENTIAL
#define DETWS_ADS1115_DIFFERENTIAL 0
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
#ifndef DETWS_ENABLE_INA219
#define DETWS_ENABLE_INA219 0
#endif

/** @brief I2C address of the INA219 (0x40 default; the A0/A1 pins select 0x40..0x4F). */
#ifndef DETWS_INA219_I2C_ADDR
#define DETWS_INA219_I2C_ADDR 0x40
#endif

/** @brief Default INA219 current LSB in microamps per bit (calibration input). The fallback when
 *         ina219_begin() is passed 0. 100 uA/bit with a 100 mohm shunt -> a 2 A full-scale range. */
#ifndef DETWS_INA219_CURRENT_LSB_UA
#define DETWS_INA219_CURRENT_LSB_UA 100
#endif

/** @brief Default INA219 shunt resistance in milliohms (calibration input). The fallback when
 *         ina219_begin() is passed 0. 100 mohm is the common breakout value. */
#ifndef DETWS_INA219_SHUNT_MOHM
#define DETWS_INA219_SHUNT_MOHM 100
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
#ifndef DETWS_ENABLE_CONFIG_STORE
#define DETWS_ENABLE_CONFIG_STORE 0
#endif

/** @brief Max key/value entries in the host (test) config backend. */
#ifndef DETWS_CONFIG_MAX_ENTRIES
#define DETWS_CONFIG_MAX_ENTRIES 16
#endif

/** @brief Max key length incl. null (NVS caps keys at 15 chars). */
#ifndef DETWS_CONFIG_KEY_MAX
#define DETWS_CONFIG_KEY_MAX 16
#endif

/** @brief Max value bytes per entry in the host (test) config backend. */
#ifndef DETWS_CONFIG_VAL_MAX
#define DETWS_CONFIG_VAL_MAX 64
#endif

/**
 * @brief Stable device UUID derived from the chip MAC (RFC 4122 v5).
 *
 * When set, src/services/device_id/device_id.h derives a deterministic v5 UUID
 * from a MAC (via the library's SHA-1) - a storage-free, stable identity for
 * mDNS hostnames, MQTT client IDs, etc. The MAC->UUID core is host-testable;
 * detws_device_uuid() reads the ESP32 factory MAC. Default off.
 */
#ifndef DETWS_ENABLE_DEVICE_ID
#define DETWS_ENABLE_DEVICE_ID 0
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
#ifndef DETWS_ENABLE_TELEMETRY
#define DETWS_ENABLE_TELEMETRY 0
#endif

/**
 * @brief Real-time SVG dashboard (DETWS_ENABLE_DASHBOARD; requires DETWS_ENABLE_SSE).
 *
 * Default off. Serves a self-contained, hand-rolled SVG dashboard page whose
 * widgets are declared in a fixed compile-time DetwsWidget table (zero-heap,
 * deterministic). The page fetches the widget layout as JSON and subscribes to an
 * SSE stream of live values; detws_dashboard_set() + detws_dashboard_publish()
 * push the current readings. The widget-table -> JSON serializers are
 * host-testable; WebSocket controls are a follow-up.
 */
#ifndef DETWS_ENABLE_DASHBOARD
#define DETWS_ENABLE_DASHBOARD 0
#endif

/**
 * @brief Embed the theme stylesheet library as runtime-selectable blobs (default off).
 *
 * Off by default: build-time theme injection (`<!--#theme NAME-->`) costs nothing extra, but
 * embedding the whole library for runtime switching links every theme's CSS into flash (~1 KB each).
 * When set, application/binary_asset_blobs.{h,cpp} exposes `detws_theme_css(name)` + the registry
 * `DETWS_THEME_BLOBS`, so a route (e.g. `/themes/<name>.css`) or a picker can switch themes live.
 * Regenerate with `src/web/wizard/gen_theme_blobs.py` after adding a theme.
 */
#ifndef DETWS_ENABLE_THEMES
#define DETWS_ENABLE_THEMES 0
#endif

/**
 * @brief Include the trademark-named themes in the embedded set (default on / open-source).
 *
 * A few themes are named after a company or product (Darcula, Windows XP, Discord, Spotify, ...). The
 * palette is just colors, but a commercial product should not ship the branded name, so a commercial
 * build sets this to 0 to drop those blobs from the registry (the list is `RESTRICTED` in
 * `src/web/wizard/gen_themes.py`). The open-source (AGPL) build keeps them.
 */
#ifndef DETWS_THEMES_INCLUDE_TRADEMARKED
#define DETWS_THEMES_INCLUDE_TRADEMARKED 1
#endif

/** @brief Maximum widgets in the dashboard table (BSS value array). */
#ifndef DETWS_DASHBOARD_MAX_WIDGETS
#define DETWS_DASHBOARD_MAX_WIDGETS 16
#endif

/** @brief Stack buffer for the dashboard layout / values JSON (bytes). */
#ifndef DETWS_DASHBOARD_JSON_BUF
#define DETWS_DASHBOARD_JSON_BUF 1024
#endif

/**
 * @brief Opt-in flash partition-map monitor endpoint (DETWS_ENABLE_PARTITION_MONITOR).
 *
 * Default off. When set, services/partition_monitor reports the device's flash
 * partition table (label, kind, type / subtype, offset, size, and which app slot
 * is running) as JSON, for diagnostics and OTA dashboards. The partition walk uses
 * esp_partition / esp_ota_ops; the JSON serializer and the kind classifier are
 * pure and host-testable.
 */
#ifndef DETWS_ENABLE_PARTITION_MONITOR
#define DETWS_ENABLE_PARTITION_MONITOR 0
#endif

/** @brief Maximum partitions the monitor reports (BSS table). */
#ifndef DETWS_PARTITION_MAX
#define DETWS_PARTITION_MAX 16
#endif

/** @brief Stack buffer for the partition-map JSON (bytes). */
#ifndef DETWS_PARTITION_JSON_BUF
#define DETWS_PARTITION_JSON_BUF 1024
#endif

/**
 * @brief Opt-in browser GPIO pin-mapper / diagnostics endpoint (DETWS_ENABLE_GPIO_MAP).
 *
 * Default off. When set, services/gpio_map serves a compile-time table of GPIO
 * pins (number, label, direction, live level) as JSON for a browser diag panel,
 * and accepts a control POST (`pin`, `level`) to drive an output. The live read /
 * write uses the Arduino digital API on ESP32; the JSON serializer and the control
 * parser are pure and host-testable.
 */
#ifndef DETWS_ENABLE_GPIO_MAP
#define DETWS_ENABLE_GPIO_MAP 0
#endif

/** @brief Maximum GPIO pins the mapper reports (BSS table). */
#ifndef DETWS_GPIO_MAX
#define DETWS_GPIO_MAX 40
#endif

/** @brief Stack buffer for the GPIO-map JSON (bytes). */
#ifndef DETWS_GPIO_JSON_BUF
#define DETWS_GPIO_JSON_BUF 1024
#endif

/**
 * @brief Opt-in fire-and-forget UDP telemetry cast (DETWS_ENABLE_UDP_TELEMETRY).
 *
 * Default off. When set, services/udp_telemetry casts metric lines (InfluxDB line
 * protocol: `measurement field=val,field2=val2`) to a configured collector over
 * UDP via det_udp_sendto - zero-heap, fire-and-forget (no ACK, no retry), ideal
 * for shipping device metrics to Telegraf/InfluxDB/a log sink. The line builder is
 * pure and host-tested; only the send touches the network.
 */
#ifndef DETWS_ENABLE_UDP_TELEMETRY
#define DETWS_ENABLE_UDP_TELEMETRY 0
#endif

/** @brief Stack buffer for one telemetry line (bytes). */
#ifndef DETWS_UDP_TELEMETRY_BUF
#define DETWS_UDP_TELEMETRY_BUF 256
#endif

/**
 * @brief Opt-in StatsD metrics client (DETWS_ENABLE_STATSD).
 *
 * Default off. When set, services/statsd pushes metrics in the StatsD wire format
 * (`name:value|type`, e.g. `api.hits:1|c`) over UDP to a StatsD-speaking collector -
 * Graphite/StatsD, Telegraf, Datadog, InfluxDB, etc. Counters, gauges (absolute + delta),
 * timings, and sets, with optional sample-rate (`|@0.1`) and DogStatsD tags (`|#env:prod`).
 * This is the push counterpart to the pull-based Prometheus `/metrics`. The line formatter
 * is pure and host-tested; only the send (det_udp_sendto) touches the network. Zero heap.
 */
#ifndef DETWS_ENABLE_STATSD
#define DETWS_ENABLE_STATSD 0
#endif

/** @brief Default StatsD collector UDP port (StatsD/Graphite standard). */
#ifndef DETWS_STATSD_PORT
#define DETWS_STATSD_PORT 8125
#endif

/** @brief Stack buffer for one StatsD line (bytes; caps metric name + value + tags). */
#ifndef DETWS_STATSD_LINE_MAX
#define DETWS_STATSD_LINE_MAX 256
#endif

/**
 * @brief Opt-in runtime heap/stack guardrails (DETWS_ENABLE_GUARDRAILS).
 *
 * Default off. When set, services/guardrails samples free heap, the heap low-water
 * mark, the largest free block (fragmentation), and the calling task's remaining
 * stack, and fires a callback when any crosses its threshold - a proactive
 * fail-safe hook beyond the passive numbers in /metrics. The threshold evaluator
 * and the JSON serializer are pure and host-tested; the sample reads esp_* / the
 * FreeRTOS stack high-water on ESP32.
 */
#ifndef DETWS_ENABLE_GUARDRAILS
#define DETWS_ENABLE_GUARDRAILS 0
#endif

/** @brief Free-heap floor (bytes); below this trips the heap guardrail. */
#ifndef DETWS_GUARDRAIL_HEAP_MIN
#define DETWS_GUARDRAIL_HEAP_MIN 8192
#endif

/** @brief Largest-free-block floor (bytes); below this trips the fragmentation guardrail. */
#ifndef DETWS_GUARDRAIL_FRAG_MIN_BLOCK
#define DETWS_GUARDRAIL_FRAG_MIN_BLOCK 4096
#endif

/** @brief Task remaining-stack floor (bytes); below this trips the stack guardrail. */
#ifndef DETWS_GUARDRAIL_STACK_MIN
#define DETWS_GUARDRAIL_STACK_MIN 512
#endif

/**
 * @brief Opt-in software watchdog: deadlock detection + fail-safe safe-state (DETWS_ENABLE_FAILSAFE).
 *
 * When set, services/failsafe provides a fixed registry of "lifelines" (a task / worker / control loop
 * that must check in within its deadline). detws_failsafe_check() detects one that stopped feeding (a
 * hang / deadlock) and fires a breach callback once per episode so the app can enter a known-safe
 * state. App-defined and per-lifeline, on top of the hardware task watchdog. Pure core, zero heap.
 * Default off.
 */
#ifndef DETWS_ENABLE_FAILSAFE
#define DETWS_ENABLE_FAILSAFE 0
#endif

/** @brief Max monitored lifelines in the fail-safe registry (static, zero-heap). */
#ifndef DETWS_FAILSAFE_MAX_LIFELINES
#define DETWS_FAILSAFE_MAX_LIFELINES 8
#endif

/**
 * @brief Opt-in dynamic sleep-cycle scheduler (DETWS_ENABLE_SLEEP_SCHED).
 *
 * When set, services/sleep_sched provides detws_sleep_next(): from the time since the last activity it
 * returns how long a low-power device should sleep (0 = stay awake), ramping the window from a floor up
 * to a ceiling the longer the idle streak runs. Pure decision core (the app applies the window via
 * light / modem / deep sleep). Complements services/radio_power. Default off.
 */
#ifndef DETWS_ENABLE_SLEEP_SCHED
#define DETWS_ENABLE_SLEEP_SCHED 0
#endif

/**
 * @brief Opt-in flash wear-leveling slot selector (DETWS_ENABLE_WEARLEVEL).
 *
 * When set, services/wearlevel provides detws_wearlevel_pick(): given per-slot write counts it returns
 * the least-worn slot to write next, so repeated flash/NVS writes spread evenly and the region ages
 * together instead of burning out one block. Pure core (the app owns the slots + persisted counts).
 * Default off.
 */
#ifndef DETWS_ENABLE_WEARLEVEL
#define DETWS_ENABLE_WEARLEVEL 0
#endif

/**
 * @brief Opt-in network adaptation decisions (DETWS_ENABLE_NETADAPT).
 *
 * When set, services/netadapt provides two pure decisions: detws_netadapt_window() sizes the TCP
 * receive window from the free heap (bigger when RAM is plentiful, shrinking when tight), and
 * detws_netadapt_dhcp_fallback() decides when to give up on DHCP and use a static IP. The app applies
 * the results (lwIP window / netif config). Default off.
 */
#ifndef DETWS_ENABLE_NETADAPT
#define DETWS_ENABLE_NETADAPT 0
#endif

/**
 * @brief Opt-in DShot ESC throttle protocol codec (DETWS_ENABLE_DSHOT).
 *
 * When set, services/dshot provides detws_dshot_encode() / _decode(): the 16-bit DShot frame
 * (11-bit throttle/command + telemetry bit + 4-bit CRC), the bidirectional/extended inverted-CRC
 * variant, and the per-rate bit timing for an RMT driver. Pure codec (the app clocks it out via RMT).
 * Default off.
 */
#ifndef DETWS_ENABLE_DSHOT
#define DETWS_ENABLE_DSHOT 0
#endif

/**
 * @brief Opt-in HART / HART-IP process-instrument protocol codec (DETWS_ENABLE_HART).
 *
 * When set, services/hart provides the HART command-frame codec (build/parse with the longitudinal XOR
 * checksum, short + long addressing) and the 8-octet HART-IP message header, so a device speaks HART
 * over UDP/TCP 5094 (front-end-free) or, with a HART FSK modem, over the 4-20 mA loop. Pure, host-tested.
 * Default off.
 */
#ifndef DETWS_ENABLE_HART
#define DETWS_ENABLE_HART 0
#endif

/**
 * @brief Opt-in Network Time Security (NTS, RFC 8915) wire codec (DETWS_ENABLE_NTS).
 *
 * When set, services/nts provides the NTS-KE record codec (build/parse the TLV records - next protocol,
 * AEAD, cookies, server/port) and the NTS NTP extension-field framing (Unique Identifier, Cookie,
 * Authenticator). Pure framing (the AES-SIV-CMAC-256 AEAD + TLS-exporter key derivation are the crypto
 * integration on top). Default off.
 */
#ifndef DETWS_ENABLE_NTS
#define DETWS_ENABLE_NTS 0
#endif

/**
 * @brief Opt-in DDS / RTPS wire-protocol codec (DETWS_ENABLE_DDS).
 *
 * When set, services/dds provides the RTPS (DDSI-RTPS) message + submessage framing: the 20-octet
 * header (magic / version / vendor / guidPrefix) and the typed submessages (INFO_TS, DATA, HEARTBEAT,
 * ACKNACK, ...) with the endianness flag, built by detws_rtps_header / _submessage and walked by
 * detws_rtps_parse. Pure framing (CDR payloads + SPDP/SEDP discovery layer on top). Default off.
 */
#ifndef DETWS_ENABLE_DDS
#define DETWS_ENABLE_DDS 0
#endif

/**
 * @brief Opt-in XMPP (RFC 6120) stanza codec (DETWS_ENABLE_XMPP).
 *
 * When set, services/xmpp builds correctly XML-escaped `<stream:stream>` / `<message>` / `<presence>` /
 * `<iq>` stanzas into a caller buffer and reads the stanza element name + an attribute value out of a
 * received stanza, so a device is an IoT XMPP client. Pure text framing (TLS/SASL ride the client TLS
 * path; the IoT XEPs layer inside `<iq>`). Default off.
 */
#ifndef DETWS_ENABLE_XMPP
#define DETWS_ENABLE_XMPP 0
#endif

/**
 * @brief Opt-in raw Layer-2 Ethernet frame codec (DETWS_ENABLE_RAWL2).
 *
 * When set, services/rawl2 builds/parses Ethernet II + 802.1Q VLAN frames (no FCS - the MAC appends it;
 * detws_eth_fcs is provided for the cases that need it), so the app can inject/receive arbitrary L2
 * frames via esp_eth_transmit / esp_wifi_80211_tx - the basis for the raw-L2 industrial protocols
 * (PROFINET DCP, GOOSE, POWERLINK). Pure codec, host-tested. Default off.
 */
#ifndef DETWS_ENABLE_RAWL2
#define DETWS_ENABLE_RAWL2 0
#endif

/**
 * @brief Opt-in single-page-app micro-routing decision (DETWS_ENABLE_SPA_ROUTER).
 *
 * When set, services/spa_router provides detws_spa_route(): given a request path it returns whether to
 * serve a real asset file, serve the SPA shell (index.html) for a client-side route, or pass through to
 * the app's handlers under an API prefix - so a single-page UI's client routing works. Pure decision
 * core (the caller wires the result into serve_static / the router). Default off.
 */
#ifndef DETWS_ENABLE_SPA_ROUTER
#define DETWS_ENABLE_SPA_ROUTER 0
#endif

/**
 * @brief Opt-in IEC 61850 GOOSE publisher codec (DETWS_ENABLE_GOOSE).
 *
 * When set, services/goose builds the BER-encoded IECGoosePdu (gocbRef / timeAllowedToLive / datSet /
 * goID / t / stNum / sqNum / simulation / confRev / ndsCom / numDatSetEntries / allData) and wraps it in
 * the 8-octet GOOSE header + Ethernet frame (ethertype 0x88B8) for the fast raw-L2 substation-event
 * publish. Pure codec (allData is a caller-encoded BER blob; the raw-L2 transmit is the device step).
 * Default off.
 */
#ifndef DETWS_ENABLE_GOOSE
#define DETWS_ENABLE_GOOSE 0
#endif

/**
 * @brief Opt-in MTConnect agent response codec (DETWS_ENABLE_MTCONNECT).
 *
 * When set, services/mtconnect builds the MTConnectStreams (current/sample) and MTConnectError XML
 * response documents (ANSI/MTC1.4) into a caller buffer - header with instanceId + nextSequence, then
 * per-DataItem Samples/Events/Condition observations - so the web server is an MTConnect agent over the
 * existing HTTP stack. Pure text framing (values XML-escaped). Default off.
 */
#ifndef DETWS_ENABLE_MTCONNECT
#define DETWS_ENABLE_MTCONNECT 0
#endif

/**
 * @brief MTConnect rolling sample buffer sizing (DETWS_ENABLE_MTCONNECT).
 *
 * The agent retains the most recent ::DETWS_MTC_SAMPLE_BUFFER observations in a fixed ring so a
 * subscriber can replay them with the `sample` from/count long-poll cursor (MTC1.4 §6.7): a request
 * asks for observations starting at a sequence number, and the response header reports firstSequence /
 * lastSequence / nextSequence so the client knows what it received and where to resume. Each retained
 * observation stores its type / dataItemId / timestamp / value in fixed char fields; when the ring is
 * full the oldest is evicted and firstSequence advances. Zero-heap, compile-time sized; the buffer costs
 * ~DETWS_MTC_SAMPLE_BUFFER * (48 + the four string caps) bytes only where a DetwsMtcSampleBuffer is used.
 */
#ifndef DETWS_MTC_SAMPLE_BUFFER
#define DETWS_MTC_SAMPLE_BUFFER 32 // observations retained for `sample` replay
#endif
#ifndef DETWS_MTC_STR_MAX
#define DETWS_MTC_STR_MAX 24 // max stored type / dataItemId length (excl NUL)
#endif
#ifndef DETWS_MTC_TS_MAX
#define DETWS_MTC_TS_MAX 32 // max stored ISO-8601 timestamp length (excl NUL)
#endif
#ifndef DETWS_MTC_VAL_MAX
#define DETWS_MTC_VAL_MAX 32 // max stored observation value length (excl NUL)
#endif

/**
 * @brief Opt-in write-ahead store for atomic buffer-to-flash storage (DETWS_ENABLE_WAL).
 *
 * services/wal is a power-loss-safe write-ahead log over any fs::FS backend (SD card, LittleFS): records
 * are CRC32-framed, a checkpoint is atomic via an A/B superblock, and a recovery scan on mount replays
 * past the last checkpoint and stops at the first bad CRC (the torn tail), bounding the loss window. Sized
 * from the measured SD envelope (docs/FEATURE_PERFORMANCE.md): append sequentially in ~32 KiB pages,
 * checkpoint every ~128-256 KiB (never scatter small durable writes). The substrate for on-device data
 * stores (dbm / sqlite / nosql). Zero heap. Default off.
 */
#ifndef DETWS_ENABLE_WAL
#define DETWS_ENABLE_WAL 0
#endif
#ifndef DETWS_WAL_PAGE_SIZE
#define DETWS_WAL_PAGE_SIZE 32768 // sequential write unit (the measured durable-throughput knee)
#endif
#ifndef DETWS_WAL_MAX_RECORD
#define DETWS_WAL_MAX_RECORD 4096 // largest single record payload
#endif

/**
 * @brief Opt-in dbm: a log-structured hash key-value store on the WAL (DETWS_ENABLE_DBM, requires WAL).
 *
 * services/dbm is a Bitcask-style key-value store: each put/delete appends one WAL record (so writes are
 * the WAL's fast sequential appends, not slow durable random writes), and an in-RAM open-addressed hash
 * index (fixed BSS, no heap) maps each live key to where its value sits in the log. Mount rebuilds the
 * index by scanning the WAL. Keys are bounded by DETWS_DBM_KEY_MAX, values by DETWS_DBM_VAL_MAX, and the
 * index holds up to DETWS_DBM_SLOTS live keys. Default off.
 */
#ifndef DETWS_ENABLE_DBM
#define DETWS_ENABLE_DBM 0
#endif
#ifndef DETWS_DBM_SLOTS
#define DETWS_DBM_SLOTS 256 // max live keys (in-RAM index capacity; open-addressed, keep load < ~0.7)
#endif
#ifndef DETWS_DBM_KEY_MAX
#define DETWS_DBM_KEY_MAX 32 // largest key in bytes
#endif
#ifndef DETWS_DBM_VAL_MAX
#define DETWS_DBM_VAL_MAX 256 // largest value in bytes
#endif

/**
 * @brief Opt-in local JSON document store on the WAL (DETWS_ENABLE_DOCSTORE, requires DBM + WAL).
 *
 * services/docstore is a small NoSQL document store: JSON documents addressed by an id, kept durably on
 * the write-ahead log. It is a thin layer over dbm (id = key, JSON body = value) and adds the document
 * capability - top-level field queries (find documents whose JSON field equals a value) via the zero-heap
 * JSON reader. Ids are bounded by DETWS_DBM_KEY_MAX, bodies by DETWS_DBM_VAL_MAX. Default off.
 */
#ifndef DETWS_ENABLE_DOCSTORE
#define DETWS_ENABLE_DOCSTORE 0
#endif
#ifndef DETWS_DOCSTORE_FIELD_MAX
#define DETWS_DOCSTORE_FIELD_MAX 128 // largest string field value a find can compare
#endif

/**
 * @brief Opt-in SQLite3 on-disk file-format reader (DETWS_ENABLE_SQLITE).
 *
 * services/sqlite parses the documented SQLite database file structure by hand - the 100-byte database
 * header, the b-tree page header, the record varint, and record serial types - so a device can read a
 * SQLite file (from wal_fs / fs::FS) without the SQLite amalgamation (which needs a heap + stdio and does
 * not fit the no-stdlib zero-heap model). Read-first (a bounded writer is a later step); pure, host-tested
 * against real files from the sqlite3 CLI. Default off.
 */
#ifndef DETWS_ENABLE_SQLITE
#define DETWS_ENABLE_SQLITE 0
#endif

/**
 * @brief Opt-in Redis RESP wire codec (DETWS_ENABLE_REDIS).
 *
 * services/redis is the pure wire layer of a Redis client: a RESP command encoder (a command becomes a
 * RESP array of bulk strings) and a streaming, zero-heap reply decoder that reads one value at a time, so
 * arbitrarily nested replies are walked with only the caller's loop state (no tree allocation). Covers
 * RESP2 and the RESP3 additions (null / boolean / double / big number / bulk error / verbatim / map / set
 * / push). Pure (no I/O; you hand it byte buffers); host-tested against spec vectors and a real
 * redis-server. Default off.
 */
#ifndef DETWS_ENABLE_REDIS
#define DETWS_ENABLE_REDIS 0
#endif

/**
 * @brief Opt-in CNC RS-232 DNC drip-feed codec (DETWS_ENABLE_DNC).
 *
 * services/dnc is the transport-agnostic framing + tape-code layer that streams a G-code program
 * (RS-274 / ISO 6983) to a machine-tool controller over RS-232 or a socket: block framing with a `%`
 * rewind-stop, ISO 7-bit (ASCII, optional even parity) or EIA RS-244 (odd-parity punched-tape code)
 * character translation, a streaming block encoder + reassembling decoder, and XON/XOFF software
 * flow-control state. Pure codec (you own the UART / socket); host-tested. Default off.
 */
#ifndef DETWS_ENABLE_DNC
#define DETWS_ENABLE_DNC 0
#endif

/**
 * @brief Largest G-code block (one line) the DNC decoder reassembles (DETWS_ENABLE_DNC).
 *
 * A block longer than this overflows the decoder's fixed line buffer and is dropped whole
 * (::DNC_EV_OVERFLOW) rather than truncated. Sized for a normal G-code line; raise it only for
 * unusually long blocks (many parameters). Zero heap - this is the static per-decoder buffer.
 */
#ifndef DETWS_DNC_LINE_MAX
#define DETWS_DNC_LINE_MAX 128
#endif

/**
 * @brief Default leader/trailer runout length for the DNC encoder (DETWS_ENABLE_DNC).
 *
 * The number of NUL runout bytes ::dnc_encode_leader emits before the program (and can emit after
 * it). The reader skips them until the first `%`. Traditional tape leaders were a few inches of
 * blank feed; 32 bytes is a serial-link equivalent. Overridable per call via DncCfg::leader_len.
 */
#ifndef DETWS_DNC_LEADER_LEN
#define DETWS_DNC_LEADER_LEN 32
#endif

/**
 * @brief Safety cap on how many times the DNC stream engine polls the reverse channel while paused
 *        by an XOFF, before giving up with an I/O error (DETWS_ENABLE_DNC).
 *
 * `dnc_stream` pauses on XOFF and polls `recv` for the XON that resumes it; a well-behaved transport
 * paces `recv` (blocks briefly when idle) so this cap is only a backstop against a `recv` that spins
 * returning no data forever. Raise it if a slow controller legitimately holds XOFF for a long time.
 */
#ifndef DETWS_DNC_XOFF_MAX_POLLS
#define DETWS_DNC_XOFF_MAX_POLLS 200000
#endif

/**
 * @brief Opt-in TCP relay / DNAT port forwarding (DETWS_ENABLE_RELAY).
 *
 * services/relay is a bidirectional byte pump that publishes an internal `host:port` through the
 * server: an inbound connection is relayed to an origin (an outbound det_client connection), moving
 * bytes both ways with backpressure and independent half-close, so the device fronts a service that
 * lives behind it. The engine is a pure step function over two send/recv seams (host-testable); the
 * app owns the two sockets. Default off.
 */
#ifndef DETWS_ENABLE_RELAY
#define DETWS_ENABLE_RELAY 0
#endif

/**
 * @brief Per-direction relay buffer size (bytes) for services/relay (DETWS_ENABLE_RELAY).
 *
 * Each active relay holds two buffers of this size (one per direction) for bytes read from one peer
 * but not yet accepted by the other (backpressure carry). Larger buffers raise throughput per step
 * (fewer cross-thread det_conn_send marshals per KB) at the cost of RAM per concurrent relay
 * (2 * DETWS_RELAY_BUF * DETWS_RELAY_MAX_CONNS bytes).
 */
#ifndef DETWS_RELAY_BUF
#define DETWS_RELAY_BUF 2048
#endif

/**
 * @brief Max det_relay_step passes per poll for the relay listener (DETWS_ENABLE_RELAY).
 *
 * One poll drains up to this many DETWS_RELAY_BUF chunks per direction, so a single event forwards the
 * whole buffered origin RX ring (DETWS_CLIENT_RX_BUF) instead of one chunk - the difference between a
 * ~0.4 Mbps and a multi-Mbps port-forward. Bounded so one busy bridge cannot starve the others.
 */
#ifndef DETWS_RELAY_DRAIN_MAX
#define DETWS_RELAY_DRAIN_MAX 8
#endif

/**
 * @brief Max published relay ports (bind table size) for the relay listener (DETWS_ENABLE_RELAY).
 *
 * Each det_relay_publish() call binds one listener port to one origin `host:port`. This caps how
 * many distinct ports the device can front at once.
 */
#ifndef DETWS_RELAY_MAX_PUBLISH
#define DETWS_RELAY_MAX_PUBLISH 4
#endif

/**
 * @brief Max concurrent relayed connections (bridge table size) for the relay listener
 *        (DETWS_ENABLE_RELAY). Each holds a DetRelay (two DETWS_RELAY_BUF buffers) + an origin slot.
 */
#ifndef DETWS_RELAY_MAX_CONNS
#define DETWS_RELAY_MAX_CONNS 4
#endif

/** @brief Max origin hostname length (bytes, incl. NUL) stored per published relay port. */
#ifndef DETWS_RELAY_HOST_MAX
#define DETWS_RELAY_HOST_MAX 64
#endif

/** @brief Blocking connect timeout (ms) when the relay listener dials the origin on a new inbound. */
#ifndef DETWS_RELAY_CONNECT_MS
#define DETWS_RELAY_CONNECT_MS 5000
#endif

/**
 * @brief Opt-in FTP client wire codec (DETWS_ENABLE_FTP).
 *
 * services/ftp is the pure protocol layer of an FTP client (RFC 959 + RFC 2428 EPSV/EPRT):
 * `ftp_build_command` / `ftp_build_port` / `ftp_build_eprt` build control-channel commands,
 * `ftp_parse_reply` detects a complete single- or multi-line 3-digit reply, and
 * `ftp_parse_pasv` / `ftp_parse_epsv` decode the data-channel address the server returns. So a
 * device can push/pull files - e.g. drip a `.nc` program to a CNC controller's FTP store. Pure
 * codec (you own the control + data sockets); host-tested. Default off.
 */
#ifndef DETWS_ENABLE_FTP
#define DETWS_ENABLE_FTP 0
#endif

/**
 * @brief Suggested FTP control-command buffer size (DETWS_ENABLE_FTP).
 *
 * A convenience cap for callers sizing the buffer they hand `ftp_build_command`; the builders
 * are all length-checked against the caller's `cap`, so this is only a sensible default. Large
 * enough for a RETR / STOR with a long path.
 */
#ifndef DETWS_FTP_CMD_MAX
#define DETWS_FTP_CMD_MAX 256
#endif

/**
 * @brief Opt-in HTTP Cache-Control directive helpers (DETWS_ENABLE_HTTP_CACHE).
 *
 * services/httpcache is the origin-side of edge caching (RFC 9111 + RFC 8246 + RFC 5861): a
 * structured `Cache-Control` builder (`cache_control_build` + first-class presets like
 * `cache_immutable_asset` / `cache_shared`) so app routes emit correct, edge-cacheable responses
 * (hand the value to DetWebServer::set_cache_control()), a tolerant directive parser
 * (`cache_control_parse`), and the RFC 9111 freshness-lifetime calculation. Pure text, host-tested.
 * Groundwork for the CDN roadmap; the caching tier itself is a separate piece. Default off.
 */
#ifndef DETWS_ENABLE_HTTP_CACHE
#define DETWS_ENABLE_HTTP_CACHE 0
#endif

/**
 * @brief Opt-in SMB2 client (DETWS_ENABLE_SMB).
 *
 * services/smb is an SMB2 client (MS-SMB2) so a device can read/write files on a Windows share -
 * e.g. a CNC controller's program store. The full read/write-a-file path: the Direct-TCP transport
 * frame + SMB2 sync header, NEGOTIATE, the two-round NTLMv2 SESSION_SETUP (NTLM digests MD4/MD5/
 * HMAC-MD5, the NTLMv2 response, the NTLMSSP messages, SPNEGO wrapping), TREE_CONNECT, CREATE, READ,
 * WRITE, and CLOSE. smb_client ties the codecs into the exchange over a send/recv seam (host-tested
 * with a scripted mock server); you own the TCP socket (det_client). All little-endian. Default off.
 */
#ifndef DETWS_ENABLE_SMB
#define DETWS_ENABLE_SMB 0
#endif

/**
 * @brief SMB2 client work-buffer size (bytes) for smb_client's request/response framing.
 *
 * Two buffers of this size live on the stack during a call, plus a few half-size scratch buffers for
 * the NTLM auth tokens, so the engine needs roughly 4x this in stack. 1024 covers the NEGOTIATE ->
 * SESSION_SETUP -> TREE_CONNECT -> CREATE handshake; raise it if a server's SPNEGO/target-info token
 * or your share path is unusually large.
 */
#ifndef DETWS_SMB_BUF
#define DETWS_SMB_BUF 1024
#endif

/**
 * @brief Opt-in SAE J2735 V2X codec (DETWS_ENABLE_J2735).
 *
 * When set, services/j2735 provides the ASN.1 UPER (Unaligned Packed Encoding Rules) bit-level primitive
 * codec (constrained INTEGER / BOOLEAN / bit fields) and, on top of it, the J2735 BSMcore safety-message
 * block (msgCnt / id / secMark / lat / long / elev / speed / heading) encode + decode, for connected-
 * vehicle messaging. Pure codec (the DSRC / C-V2X radio is an external module). Default off.
 */
#ifndef DETWS_ENABLE_J2735
#define DETWS_ENABLE_J2735 0
#endif

/**
 * @brief Opt-in NEMA TS 2 traffic-cabinet SDLC frame codec (DETWS_ENABLE_NEMA_TS2).
 *
 * When set, services/nema_ts2 builds/validates the TS 2 SDLC bus frames ([address][control][frame-type]
 * [data][CRC-16/X-25]) that link a traffic-signal controller to the MMU, BIUs, and detector racks. Pure
 * codec (the synchronous serial PHY + BIU timing are hardware-gated). Default off.
 */
#ifndef DETWS_ENABLE_NEMA_TS2
#define DETWS_ENABLE_NEMA_TS2 0
#endif

/**
 * @brief Opt-in GE Fanuc SNP (Series Ninety Protocol) serial frame codec (DETWS_ENABLE_SNP).
 *
 * When set, services/snp builds/validates the SNP master-slave serial frame ([control][length][data]
 * [arithmetic-sum BCC]) for reading/writing registers on a GE Fanuc Series 90 (90-30/90-70) PLC over
 * RS-485. Pure codec (the UART transport + SNP-X session are the device step). Default off.
 */
#ifndef DETWS_ENABLE_SNP
#define DETWS_ENABLE_SNP 0
#endif

/**
 * @brief Opt-in AutomationDirect / Koyo DirectNET serial frame codec (DETWS_ENABLE_DIRECTNET).
 *
 * When set, services/directnet builds/validates the DirectNET master-slave serial frames - the header
 * (SOH + slave/type/address/blocks ASCII-hex + ETB + LRC) and the data frame (STX + data + ETX + LRC) -
 * for V-memory read/write on an AutomationDirect DirectLOGIC PLC. Pure codec (the UART transport +
 * ACK/NAK handshake are the device step). Default off.
 */
#ifndef DETWS_ENABLE_DIRECTNET
#define DETWS_ENABLE_DIRECTNET 0
#endif

/**
 * @brief Opt-in IEEE 2030.5 (Smart Energy Profile 2.0) resource codec (DETWS_ENABLE_SEP2).
 *
 * When set, services/sep2 builds the core 2030.5 XML resource documents (DeviceCapability, EndDevice,
 * DERControl) in the urn:ieee:std:2030.5:ns namespace, so the web server is a 2030.5 smart-grid
 * server/client over the existing HTTP stack (DER dispatch / curtailment). Pure text framing. Default off.
 */
#ifndef DETWS_ENABLE_SEP2
#define DETWS_ENABLE_SEP2 0
#endif

/**
 * @brief Opt-in PROFINET DCP (Discovery and Configuration Protocol) frame codec (DETWS_ENABLE_PROFINET).
 *
 * When set, services/profinet builds/parses the DCP frames (10-octet header + option/suboption blocks)
 * PROFINET uses to discover and name IO-Devices over raw L2 (ethertype 0x8892) - Identify request/
 * response and Set (assign NameOfStation / IP). Pure codec (the raw-L2 transmit via services/rawl2 +
 * esp_eth is the device step). Default off.
 */
#ifndef DETWS_ENABLE_PROFINET
#define DETWS_ENABLE_PROFINET 0
#endif

/**
 * @brief Opt-in NTCIP transportation-device object identifiers (DETWS_ENABLE_NTCIP).
 *
 * When set, services/ntcip provides the NTCIP (National Transportation Communications for ITS Protocol)
 * object OID definitions for the common device classes - NTCIP 1202 (actuated signal controller: phases,
 * timing, live states) and 1203 (dynamic message sign) - plus an OID builder, so an app exposes them via
 * the shipped SNMP agent (services/snmp). Pure OID data. Default off.
 */
#ifndef DETWS_ENABLE_NTCIP
#define DETWS_ENABLE_NTCIP 0
#endif

/**
 * @brief Opt-in OpenADR 3.0 (Automated Demand Response) JSON codec (DETWS_ENABLE_OPENADR).
 *
 * When set, services/openadr builds the OpenADR 3.0 event (a demand-response signal: programID +
 * eventName + interval payload points) and report (a VEN reading back to the VTN) JSON objects into a
 * caller buffer, over the existing HTTP client/server + OAuth2. Pure JSON framing. Default off.
 */
#ifndef DETWS_ENABLE_OPENADR
#define DETWS_ENABLE_OPENADR 0
#endif

/**
 * @brief Opt-in IEC 61850 MMS PDU codec (DETWS_ENABLE_MMS).
 *
 * When set, services/mms builds/parses the MMS (ISO 9506) confirmed-request/response Read PDUs
 * (BER-encoded, the ACSI client/server core of IEC 61850) - detws_mms_read_request builds a Read of a
 * named Data Object, detws_mms_read_response the data reply. Carried over ISO-on-TCP (TPKT + COTP via
 * the shipped services/cotp) on port 102. Pure BER codec. Default off.
 */
#ifndef DETWS_ENABLE_MMS
#define DETWS_ENABLE_MMS 0
#endif

/**
 * @brief Opt-in CC-Link (CLPA) cyclic fieldbus frame codec (DETWS_ENABLE_CCLINK).
 *
 * When set, services/cclink builds/validates the CC-Link cyclic frame ([station][command][RX/RY bit
 * data][RWr/RWw word data][sum checksum]) a Mitsubishi CC-Link master exchanges with remote stations
 * over RS-485, plus bit/word process-image accessors. Pure codec (the RS-485 timing + CC-Link IE Field
 * PHY are hardware-gated). Default off.
 */
#ifndef DETWS_ENABLE_CCLINK
#define DETWS_ENABLE_CCLINK 0
#endif

/**
 * @brief Opt-in Ethernet POWERLINK (EPSG) basic frame codec (DETWS_ENABLE_POWERLINK).
 *
 * When set, services/powerlink builds/parses the EPL basic frames ([messageType][dest][source][payload])
 * of the isochronous managed-node cycle - SoC (start of cycle), PReq (poll request), PRes (poll
 * response with process data), SoA (start of async) - over raw L2 (ethertype 0x88AB, on the shipped
 * services/rawl2). Pure codec (the raw-L2 transmit + isochronous timing are the device step). Default off.
 */
#ifndef DETWS_ENABLE_POWERLINK
#define DETWS_ENABLE_POWERLINK 0
#endif

/**
 * @brief Opt-in SERCOS III motion-bus telegram codec (DETWS_ENABLE_SERCOS).
 *
 * When set, services/sercos builds/parses the SERCOS III MDT/AT telegrams (type + phase + cycle + cyclic
 * device data) the real-time drive/motion bus exchanges over raw L2 (ethertype 0x88CD, on the shipped
 * services/rawl2), plus the IDN (IDentification Number) encode/decode for drive-parameter addressing.
 * Pure codec (the isochronous timing + ring topology are hardware-gated). Default off.
 */
#ifndef DETWS_ENABLE_SERCOS
#define DETWS_ENABLE_SERCOS 0
#endif

/**
 * @brief Opt-in PROFIBUS-DP FDL telegram codec (DETWS_ENABLE_PROFIBUS).
 *
 * When set, services/profibus builds/validates the PROFIBUS-DP FDL telegrams - SD1 (no-data: SD1 DA SA
 * FC FCS ED) and SD2 (variable-data: SD2 LE LEr SD2 DA SA FC data FCS ED, arithmetic-sum FCS) - a
 * Siemens DP master exchanges with slaves over RS-485 (the DP-V0 cyclic I/O exchange). Pure codec (the
 * RS-485 timing + DP state machine are the device step). Default off.
 */
#ifndef DETWS_ENABLE_PROFIBUS
#define DETWS_ENABLE_PROFIBUS 0
#endif

/**
 * @brief Opt-in LonWorks / LON-IP (ISO/IEC 14908) network-variable codec (DETWS_ENABLE_LONWORKS).
 *
 * When set, services/lonworks builds/parses the LonTalk network-variable PDU ([msg-code][14-bit
 * selector][value]) that a building-automation device exchanges - over LON/IP (14908-4) UDP, so no
 * Neuron chip is needed - plus the common SNVT scalar encodings (SNVT_temp, SNVT_switch). Pure codec
 * (the UDP transport is the shipped UDP layer). Default off.
 */
#ifndef DETWS_ENABLE_LONWORKS
#define DETWS_ENABLE_LONWORKS 0
#endif

/**
 * @brief Opt-in Modbus Plus HDLC token-bus frame codec (DETWS_ENABLE_MBPLUS).
 *
 * When set, services/mbplus builds/validates the Modbus Plus HDLC frame (7E addr ctrl payload CRC-16/X-25
 * 7E) that Schneider's token-passing peer bus exchanges, plus the token-rotation helper (next station in
 * the logical ring). Reuses the shipped Modbus PDU model for the data. Pure codec (the 1 Mbit/s bus is
 * hardware-gated). Default off.
 */
#ifndef DETWS_ENABLE_MBPLUS
#define DETWS_ENABLE_MBPLUS 0
#endif

/**
 * @brief Opt-in INTERBUS summation-frame fieldbus codec (DETWS_ENABLE_INTERBUS).
 *
 * When set, services/interbus assembles/disassembles the INTERBUS summation frame (loopback word +
 * per-device 16-bit process-image slices + CRC-16/CCITT FCS) of the Phoenix Contact ring fieldbus,
 * where every device is a shift-register slice of one circulating frame. Pure codec (the physical ring
 * clocking is hardware-gated). Default off.
 */
#ifndef DETWS_ENABLE_INTERBUS
#define DETWS_ENABLE_INTERBUS 0
#endif

/**
 * @brief Opt-in ICCP / TASE.2 (IEC 60870-6) inter-control-center telemetry codec (DETWS_ENABLE_ICCP).
 *
 * When set, services/iccp builds the TASE.2 Data_Value BER structures - StateQ (a discrete state +
 * quality) and RealQ (a scaled real + quality), each with an optional timestamp - the indication points
 * a control center transfers as MMS Reads (on the shipped services/mms + services/cotp). Pure BER codec.
 * Default off.
 */
#ifndef DETWS_ENABLE_ICCP
#define DETWS_ENABLE_ICCP 0
#endif

/**
 * @brief Opt-in IEEE 1609 WAVE (WSMP + 1609.2 envelope) codec (DETWS_ENABLE_WAVE).
 *
 * When set, services/wave builds/parses the IEEE 1609 vehicular-radio framing that carries J2735: the
 * 1609.3 WSMP header (version + P-encoded PSID + length + payload) and the 1609.2 secured-message
 * envelope header (version + content type). Pairs with services/j2735. Pure codec (the DSRC / C-V2X
 * radio is an external module). Default off.
 */
#ifndef DETWS_ENABLE_WAVE
#define DETWS_ENABLE_WAVE 0
#endif

/**
 * @brief Opt-in UTMC (Urban Traffic Management and Control) common-database codec (DETWS_ENABLE_UTMC).
 *
 * When set, services/utmc builds/parses the UTMC common-database HTTP+XML messages - a UTMCRequest for
 * an object id and a UTMCResponse carrying the object value + a data-quality flag + a timestamp - the UK
 * modular framework for sharing traffic data across municipal systems, over the existing HTTP server.
 * Pure text framing. Default off.
 */
#ifndef DETWS_ENABLE_UTMC
#define DETWS_ENABLE_UTMC 0
#endif

/**
 * @brief Opt-in OCIT-Outstations message codec (DETWS_ENABLE_OCIT).
 *
 * When set, services/ocit builds/parses the OCIT (DE/AT/CH road-traffic-control) object messages
 * ([msg-type][object-type][instance][data-type][value]) between central traffic computers and field
 * controllers / detectors, with typed values (bool / byte / u16 / u32 / octets). Pure codec (the OCIT
 * transport is the shipped transport). Default off.
 */
#ifndef DETWS_ENABLE_OCIT
#define DETWS_ENABLE_OCIT 0
#endif

/**
 * @brief Opt-in ATC (Advanced Traffic Controller) field-I/O interop snapshot (DETWS_ENABLE_ATC).
 *
 * When set, services/atc exposes this device's field-I/O (a fixed table of named input/output points it
 * already gathers via the NTCIP / NEMA-TS2 / gpio services) to an ATC Linux engine over the existing
 * HTTP surface: detws_atc_snapshot_json serializes the FIO map as JSON, and detws_atc_set_output drives
 * an output point from an ATC command. Pure interop codec (ATC is a platform spec, not a wire protocol).
 * Default off.
 */
#ifndef DETWS_ENABLE_ATC
#define DETWS_ENABLE_ATC 0
#endif

/**
 * @brief Opt-in southbound protocol-driver framework (DETWS_ENABLE_SOUTHBOUND).
 *
 * The uniform seam every field-device driver plugs into so the app polls/drives any southbound device
 * (a Modbus slave, a BACnet controller, a raw sensor over SPI/I2C/UART) through one facade: register a
 * SouthboundDriver (a read/write/read_block/write_block vtable + its transport ctx), then address points
 * by driver name via detws_southbound_read / _write / _read_block / _write_block. The block calls are the
 * atomic multi-point (register-matrix) path. Bounded registry (DETWS_SOUTHBOUND_MAX_DRIVERS, default 8),
 * no heap; Modbus master is the one such driver today. Default off.
 */
#ifndef DETWS_ENABLE_SOUTHBOUND
#define DETWS_ENABLE_SOUTHBOUND 0
#endif

/**
 * @brief Opt-in ESP32 panic / exception decoder for a live diagnostics panel (DETWS_ENABLE_EXC_DECODER).
 *
 * When set, services/exc_decoder parses a captured Guru Meditation panic dump (the cause, the register
 * PC + EXCVADDR, and the backtrace PC:SP frames) into a structured ExcInfo and serializes it as JSON for
 * a "/exception" panel; the browser or a build server resolves the PCs to file:line against the firmware
 * ELF (addr2line lives off-device). Pure, no heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_EXC_DECODER
#define DETWS_ENABLE_EXC_DECODER 0
#endif

/**
 * @brief Opt-in HTTP delivery optimizations (DETWS_ENABLE_HTTP_DELIVERY).
 *
 * Three pure cores for cheaper HTTP serving, each a real web standard: RFC 5861 stale-while-revalidate
 * (detws_delivery_swr decision + detws_delivery_cache_control header), RFC 7233 byte-range delta/offset
 * fetch (detws_delivery_range parse of X-Y / X- / -N + detws_delivery_content_range for a 206), and a
 * versioned service-worker precache manifest (detws_delivery_sw_manifest). No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_HTTP_DELIVERY
#define DETWS_ENABLE_HTTP_DELIVERY 0
#endif

/**
 * @brief Opt-in hardware-health diagnostics (DETWS_ENABLE_HW_HEALTH).
 *
 * Four pure decision cores fed with samples the app reads from the hardware: a power-rail voltage-drop
 * logger (detws_hwhealth_rail_sample tracks worst droop + sag/brownout counts), a SPI-bus CRC audit with
 * hysteretic clock backoff (detws_hwhealth_spi_result halves/doubles the clock on fail/ok streaks), a
 * GPIO short-circuit test (detws_hwhealth_gpio_short: driven vs readback), and a capacitor-leakage diag
 * (detws_hwhealth_cap_leak: measured vs expected RC decay). No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_HW_HEALTH
#define DETWS_ENABLE_HW_HEALTH 0
#endif

/**
 * @brief Opt-in adaptive mDNS beacon scheduling (DETWS_ENABLE_MDNS_ADAPTIVE).
 *
 * Pure scheduling decisions on top of the shipped mDNS service: detws_mdns_beacon_adapt backs the
 * announce interval off toward a ceiling under RF contention and recovers it when the air is quiet,
 * detws_mdns_refresh_interval gives the TTL/2 continuous-refresher cadence, detws_mdns_beacon_due says
 * when an announce is due, and detws_mdns_beacon_presleep_due says whether to announce before a sleep
 * window that would otherwise let the record lapse. Wrap-safe time math, no heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_MDNS_ADAPTIVE
#define DETWS_ENABLE_MDNS_ADAPTIVE 0
#endif

/**
 * @brief Opt-in dynamic socket recycling: an LRU connection-slot pool (DETWS_ENABLE_SOCKPOOL).
 *
 * The transport-pool half of the adaptive-networking work: services/sockpool keeps a fixed table of
 * connection slots and, when saturated, recycles the least-recently-used slot for a new peer
 * (detws_sockpool_acquire returns the evicted id so the transport closes it), plus touch / release /
 * find. The app owns the real sockets; this owns which slot a connection lives in and which to reclaim
 * under pressure. No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_SOCKPOOL
#define DETWS_ENABLE_SOCKPOOL 0
#endif

/**
 * @brief Opt-in buffer placement policy (DRAM vs PSRAM) + SPI DMA ping-pong manager (DETWS_ENABLE_PSRAM_POOL).
 *
 * Pure buffer-management decisions for a PSRAM-equipped ESP32: detws_psram_place picks DRAM vs PSRAM for
 * a buffer by size, DMA requirement, and free-heap headroom (large/cold to PSRAM, small/hot + DMA to
 * DRAM, always leaving an internal-DRAM reserve), and detws_pingpong_* keeps the classic SPI DMA
 * double-buffer bookkeeping (CPU fills one buffer while DMA drains the other; swap flips their roles).
 * The actual heap_caps_calloc is the app's. No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_PSRAM_POOL
#define DETWS_ENABLE_PSRAM_POOL 0
#endif

/**
 * @brief Opt-in dual-stack Happy Eyeballs destination selection (DETWS_ENABLE_HAPPY_EYEBALLS).
 *
 * The client-side IPv6/IPv4 fallback decision on top of the shipped DetIp: detws_he_pref scores a
 * destination (RFC 6724 scope + family), detws_he_order sorts a candidate list and interleaves the
 * address families (RFC 8305) so successive connection attempts alternate v6/v4, and
 * detws_he_attempt_due gates the next attempt by the Connection Attempt Delay. Fast IPv6 when it works,
 * quick fallback to IPv4 when it does not. Needs DETWS_ENABLE_IPV6 to matter. No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_HAPPY_EYEBALLS
#define DETWS_ENABLE_HAPPY_EYEBALLS 0
#endif

/**
 * @brief Opt-in 802.11 sniffer / traffic analyzer (DETWS_ENABLE_WIFI_SNIFFER).
 *
 * The decode + decision layer for a promiscuous-mode WiFi sniffer: detws_wifi_parse decodes an 802.11
 * MAC header (frame-control type/subtype + flags and the addresses whose roles depend on ToDS/FromDS),
 * detws_wifi_stats_* tallies frames by type for a traffic panel, and detws_wifi_should_roam decides when
 * a candidate AP is enough stronger (RSSI hysteresis) to justify channel-agility roaming. The
 * promiscuous-mode radio callback stays the app's. No heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_WIFI_SNIFFER
#define DETWS_ENABLE_WIFI_SNIFFER 0
#endif

/**
 * @brief Opt-in multi-interface egress selection / failover policy (DETWS_ENABLE_LINK_MANAGER).
 *
 * The policy that drives which interface carries traffic once a device has more than one (a wired
 * Ethernet PHY alongside WiFi STA / softAP): services/link_manager keeps a small table of interfaces
 * (kind + priority + up/down) and deterministically selects the best link that is up, escalating to a
 * higher-priority interface when it comes up and failing over when it drops, reporting only real
 * transitions so the app reconfigures the netif once. The PHY bring-up (esp_eth) stays the app's. No
 * heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_LINK_MANAGER
#define DETWS_ENABLE_LINK_MANAGER 0
#endif

/**
 * @brief Opt-in CC1101 sub-GHz radio driver (DETWS_ENABLE_CC1101).
 *
 * A gateway radio plugin (DETWS_ENABLE_GATEWAY) for the TI CC1101 300-928 MHz transceiver over SPI:
 * services/cc1101 drives the chip's SPI header protocol (config registers, command strobes, status
 * registers, TX/RX FIFO) - reset + apply a SmartRF register table + set channel + verify VERSION
 * (cc1101_init), send a variable-length packet (cc1101_send), poll TX-done, enter RX, and read a packet
 * with appended RSSI/LQI (cc1101_recv), plus the RSSI-to-dBm decode. The huge modem config is a
 * caller-supplied register table. Host-tested against a mock; the RF link needs the module. Default off.
 */
#ifndef DETWS_ENABLE_CC1101
#define DETWS_ENABLE_CC1101 0
#endif

/**
 * @brief Opt-in FDC2114/2214 capacitance-to-digital field sensor (DETWS_ENABLE_FDC2214).
 *
 * A field-perturbation sensing peripheral: services/fdc2214 decodes the FDC2x14's 28-bit conversion
 * result (a capacitance shift moves the LC-tank frequency, giving contactless proximity / liquid-level /
 * material sensing) - fdc2214_data combines the register pair, fdc2214_error pulls the flags,
 * fdc2214_sensor_freq_hz scales to frequency, and fdc2214_build_config emits a single-channel
 * bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec host-tested. Default off.
 */
#ifndef DETWS_ENABLE_FDC2214
#define DETWS_ENABLE_FDC2214 0
#endif

/**
 * @brief Opt-in LDC1614 inductance-to-digital field sensor (DETWS_ENABLE_LDC1614).
 *
 * A field-perturbation sensing peripheral: services/ldc1614 decodes the LDC1614's 28-bit conversion
 * result (a nearby conductor changes the coil inductance via eddy currents, giving contactless metal
 * proximity / displacement / EM-field sensing) - ldc1614_data combines the register pair, ldc1614_error
 * pulls the flags, ldc1614_sensor_freq_hz scales to frequency, and ldc1614_build_config emits a
 * single-channel bring-up; the ESP32 binding replays it and reads the channel over I2C. Pure codec
 * host-tested. Default off.
 */
#ifndef DETWS_ENABLE_LDC1614
#define DETWS_ENABLE_LDC1614 0
#endif

/**
 * @brief Opt-in VL53L0X optical time-of-flight ranging sensor (DETWS_ENABLE_VL53L0X).
 *
 * A field-perturbation sensing peripheral for contactless distance / gesture: services/vl53l0x decodes
 * the ST VL53L0X ranging registers - vl53l0x_range_mm combines the range byte pair, vl53l0x_data_ready
 * decodes the interrupt-status byte, and vl53l0x_range_valid checks the device range-status field; the
 * ESP32 binding verifies the model id, starts continuous ranging, and reads the distance over I2C.
 * Default-settings ranging (ST's tuning blob is not applied). Pure codec host-tested. Default off.
 */
#ifndef DETWS_ENABLE_VL53L0X
#define DETWS_ENABLE_VL53L0X 0
#endif

/**
 * @brief Opt-in receive-only radio channel sniffer to pcap (DETWS_ENABLE_RADIO_SNIFF).
 *
 * Feeds frames pulled off the air by the RF gateway drivers (CC1101 / LoRa / 802.15.4) in receive-only
 * mode into the capture pipeline: services/radio_sniff wraps each 802.15.4 MAC frame in the Wireshark
 * TAP pseudo-header (carrying per-frame RSSI + channel) and a pcap record so the forwarded stream is a
 * valid .pcap. detws_radiosniff_global writes the DLT-TAP global header and detws_radiosniff_tap_record
 * writes one record. Pure framing (no heap/stdlib); the radio drivers own the receive. Default off.
 */
#ifndef DETWS_ENABLE_RADIO_SNIFF
#define DETWS_ENABLE_RADIO_SNIFF 0
#endif

/**
 * @brief Opt-in Bluetooth ATT protocol codec + GATT characteristic bridge (DETWS_ENABLE_BLE_GATT).
 *
 * The wire protocol under GATT for bridging the on-chip BLE radio to the web: services/ble_gatt builds
 * and parses the common ATT PDUs (read / write / notify / error, Bluetooth Core Vol 3 Part F) and
 * serializes a GATT characteristic table as JSON for the web stack (att_read_req / att_write_req /
 * att_notify / att_error_rsp / att_parse / gatt_char_json). The BLE stack owns the radio; this owns the
 * ATT bytes + the northbound JSON. Pure, no heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_BLE_GATT
#define DETWS_ENABLE_BLE_GATT 0
#endif

/**
 * @brief Opt-in TLS version negotiation + pinned cipher-suite policy (DETWS_ENABLE_TLS_POLICY).
 *
 * A policy layer on top of the mbedTLS-backed transport TLS (which already runs the 1.2 / 1.3 record +
 * handshake): services/tls_policy pins the version to an audited [min,max] and makes the negotiated
 * version observable (detws_tls_negotiate_version / detws_tls_version_name), and pins the cipher suites
 * to an audited allowlist selected by server preference (detws_tls_select_cipher), with an AEAD-only
 * classifier (detws_tls_is_aead) for a hardened profile. Pure, host-tested; the app feeds the results to
 * the mbedTLS config. Default off.
 */
#ifndef DETWS_ENABLE_TLS_POLICY
#define DETWS_ENABLE_TLS_POLICY 0
#endif

/**
 * @brief Opt-in Wi-SUN FAN border-router connector (DETWS_ENABLE_WISUN).
 *
 * Wi-SUN FAN is an IPv6/UDP/CoAP mesh terminated by a border router, so the connector rides the existing
 * IP stack rather than driving a radio: services/wisun keeps a table of FAN nodes (their DetIp addresses +
 * join state) behind the border router and builds the CoAP client requests to their resources
 * (wisun_build_coap frames an RFC 7252 header + Uri-Path options + payload; the CoAP service ships only a
 * server). wisun_nodes_json exposes the mesh to the web. The app sends the built PDU over det_udp; the
 * chosen devboard only sets which border router you point at. Pure, no heap/stdlib. Default off.
 */
#ifndef DETWS_ENABLE_WISUN
#define DETWS_ENABLE_WISUN 0
#endif

/**
 * @brief Opt-in fixed-RAM rotating log buffer with severity traps (DETWS_ENABLE_LOGBUF).
 *
 * Default off. When set, services/logbuf keeps the last DETWS_LOG_LINES log lines
 * in a fixed ring (oldest pruned on overflow - no heap, bounded), dumps them
 * oldest-first for a `/logs` endpoint, and fires a trap callback when a line is
 * logged at/above a severity threshold (forward criticals as an SNMP trap /
 * webhook). The ring + trap logic is pure and host-tested.
 */
#ifndef DETWS_ENABLE_LOGBUF
#define DETWS_ENABLE_LOGBUF 0
#endif

/** @brief Number of log lines retained in the ring. */
#ifndef DETWS_LOG_LINES
#define DETWS_LOG_LINES 32
#endif

/** @brief Maximum length of one stored log line (bytes, including null). */
#ifndef DETWS_LOG_LINE_LEN
#define DETWS_LOG_LINE_LEN 96
#endif

/**
 * @brief Opt-in schema-driven config export / restore (DETWS_ENABLE_CONFIG_IO).
 *
 * Default off. Requires DETWS_ENABLE_CONFIG_STORE. The app declares a fixed schema
 * (key + type); services/config_io serializes the current values to a portable
 * `key=value` text blob (backup / migrate) and parses one back into the store
 * (restore / bulk template). Schema-driven rather than enumerating NVS, so it
 * stays deterministic and zero-heap; the serialize / parse is host-tested.
 */
#ifndef DETWS_ENABLE_CONFIG_IO
#define DETWS_ENABLE_CONFIG_IO 0
#endif

/** @brief Authenticated OTA firmware update (streaming POST to the ESP32 Update API). */
#ifndef DETWS_ENABLE_OTA
#define DETWS_ENABLE_OTA 0
#endif

/**
 * @brief Opt-in OTA rollback protection / soft-brick safeguard (DETWS_ENABLE_OTA_ROLLBACK).
 *
 * Default off. After an OTA update the new image boots in PENDING_VERIFY; this
 * service confirms it (esp_ota_mark_app_valid) once a self-test passes, or rolls
 * back to the previous image if the self-test fails or the confirm window elapses
 * without success - so a bad update self-heals instead of soft-bricking. The
 * decision logic is pure and host-tested; the commit / rollback use esp_ota_ops.
 * Requires the bootloader's app-rollback support (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE).
 */
#ifndef DETWS_ENABLE_OTA_ROLLBACK
#define DETWS_ENABLE_OTA_ROLLBACK 0
#endif

/** @brief Confirm window (ms): a pending image not confirmed within this rolls back. */
#ifndef DETWS_OTA_CONFIRM_WINDOW_MS
#define DETWS_OTA_CONFIRM_WINDOW_MS 30000
#endif

/**
 * @brief Opt-in TOTP two-factor auth (RFC 6238) (DETWS_ENABLE_TOTP).
 *
 * Default off. services/totp computes and verifies time-based one-time passwords
 * (HMAC-SHA1 over the existing SHA-1, Google Authenticator compatible) and decodes
 * base32 shared secrets, for a second factor on top of password / JWT auth. Pure
 * and host-tested against the RFC 6238 vectors; the verifier checks a +/- step
 * window for clock skew.
 */
#ifndef DETWS_ENABLE_TOTP
#define DETWS_ENABLE_TOTP 0
#endif

/**
 * @brief Opt-in outbound webhooks / IFTTT (DETWS_ENABLE_WEBHOOK).
 *
 * Default off. Needs DETWS_ENABLE_HTTP_CLIENT to actually send: the API always
 * compiles, but without the HTTP client detws_webhook_post() returns -1.
 * services/webhook builds an IFTTT Maker URL and a value1/value2/value3 JSON
 * payload (pure, host-tested) and fires them - or any JSON to any URL - via the
 * outbound http_client (POST). Use it to
 * push an event from the device to IFTTT, a Slack/Discord hook, or your own API.
 */
#ifndef DETWS_ENABLE_WEBHOOK
#define DETWS_ENABLE_WEBHOOK 0
#endif

/**
 * @brief Opt-in radio power controls (DETWS_ENABLE_RADIO_POWER).
 *
 * Default off. services/radio_power applies the WiFi modem-sleep mode and an
 * optional max-TX-power cap in one call (esp_wifi_set_ps / esp_wifi_set_max_tx_power)
 * - trade throughput/latency for lower average power on a battery device. The mode
 * names are host-tested; the apply is ESP32-only.
 */
#ifndef DETWS_ENABLE_RADIO_POWER
#define DETWS_ENABLE_RADIO_POWER 0
#endif

/** @brief WiFi modem-sleep mode: 0 = none (max perf), 1 = min modem, 2 = max modem. */
#ifndef DETWS_RADIO_WIFI_PS
#define DETWS_RADIO_WIFI_PS 0
#endif

/** @brief Max TX power cap in dBm (2..20); 0 = leave the platform default. */
#ifndef DETWS_RADIO_MAX_TX_DBM
#define DETWS_RADIO_MAX_TX_DBM 0
#endif

/**
 * @brief Opt-in DNS resolver with answer verification (DETWS_ENABLE_DNS_RESOLVER).
 *
 * Default off. services/dns_resolver resolves a hostname to an IPv4 address (lwIP
 * dns_gethostbyname, marshalled to tcpip_thread like the http_client) and can
 * reject suspicious answers - 0.0.0.0, broadcast, loopback, multicast - which are
 * spoofing / DNS-rebinding indicators for a remote host. The address classifier /
 * verifier is pure and host-tested; the resolve is ESP32-only (blocking, so call
 * it off the request hot path).
 */
#ifndef DETWS_ENABLE_DNS_RESOLVER
#define DETWS_ENABLE_DNS_RESOLVER 0
#endif

/** @brief DNS resolve timeout in milliseconds. */
#ifndef DETWS_DNS_TIMEOUT_MS
#define DETWS_DNS_TIMEOUT_MS 5000
#endif

/**
 * @brief Tamper-evident audit log (DETWS_ENABLE_AUDIT_LOG).
 *
 * Default off. services/audit_log keeps an append-only, hash-chained security
 * log: each record carries SHA-256(prev_hash || fields), so altering, deleting,
 * or reordering any retained record breaks the chain (detws_audit_verify()
 * detects it). Storage is a fixed RAM ring of DETWS_AUDIT_LOG_ENTRIES records
 * (no heap); when it wraps, a moving anchor keeps the retained window verifiable.
 * Install a sink (detws_audit_set_sink) to forward every record at creation time
 * to a durable / remote store - SD-card file, syslog or HTTP log service, serial
 * console - preserving the same chain off-device. Pure and host-tested.
 */
#ifndef DETWS_ENABLE_AUDIT_LOG
#define DETWS_ENABLE_AUDIT_LOG 0
#endif

// Ring depth and per-record message length are tunable in audit_log.h
// (DETWS_AUDIT_LOG_ENTRIES, DETWS_AUDIT_MSG_LEN); define them before include to
// override. The RAM cost is roughly DETWS_AUDIT_LOG_ENTRIES * (DETWS_AUDIT_MSG_LEN
// + 41) bytes.

/**
 * @brief OpenID Connect ID-token verification, RS256 (DETWS_ENABLE_OIDC).
 *
 * Default off. services/oidc verifies an OIDC ID token (JWT) as a relying party:
 * requires alg RS256, selects the issuer key by kid from a JWKS, verifies the
 * RSASSA-PKCS1-v1.5 SHA-256 signature (real RSA modexp via ssh_rsa, mbedTLS-
 * accelerated on ESP32), and checks iss / aud / exp / nbf, extracting sub / email.
 * Pure and host-tested; the caller fetches + caches the JWKS over HTTPS (off the
 * request hot path) and passes the JSON in. Builds on the SSH RSA primitive, not
 * the HS256 JWT module (services/jwt), so the two are independent.
 */
#ifndef DETWS_ENABLE_OIDC
#define DETWS_ENABLE_OIDC 0
#endif

/** @brief Max accepted OIDC ID-token length (also sizes the Authorization buffer). */
#ifndef DETWS_OIDC_MAX_LEN
#define DETWS_OIDC_MAX_LEN 1600
#endif

/**
 * @brief Unified virtual filesystem wrapper (DETWS_ENABLE_VFS).
 *
 * Default off. services/vfs exposes one small file API (open/read/write/close,
 * exists/size/remove/rename, whole-file helpers) over a pluggable backend, so a
 * feature can target storage without knowing the medium. A built-in zero-heap RAM
 * backend (fixed BSS pool - deterministic, host-identical) ships for scratch /
 * tests; an Arduino-FS backend (ESP32) wraps a real fs::FS (LittleFS / SD /
 * SPIFFS) for persistence. Mount one at startup; the API fails closed otherwise.
 * Pool dimensions are tunable in this config (DETWS_VFS_RAM_FILES, _RAM_FILE_SIZE,
 * _MAX_OPEN, _NAME_MAX).
 */
#ifndef DETWS_ENABLE_VFS
#define DETWS_ENABLE_VFS 0
#endif

/**
 * @brief GraphQL query subset (DETWS_ENABLE_GRAPHQL).
 *
 * Default off. services/graphql parses a GraphQL query into a fixed AST node pool
 * (no heap) and emits a `{"data":{...}}` response shaped exactly by the requested
 * selection. Schema-free: a field with a sub-selection is an object (the engine
 * recurses), a leaf field calls your single resolver, and arguments collected
 * along the path are handed to it. Supports nested selections, field arguments,
 * and the anonymous / `query` forms; mutations, subscriptions, fragments, and
 * variables are out of scope. Pure and host-tested; bounds are compile-time
 * (DETWS_GQL_* in this config). Serve it from a POST /graphql route.
 */
#ifndef DETWS_ENABLE_GRAPHQL
#define DETWS_ENABLE_GRAPHQL 0
#endif

/**
 * @brief ESP-NOW peer messaging (DETWS_ENABLE_ESPNOW).
 *
 * Default off. services/espnow wraps ESP-NOW connectionless peer-to-peer radio
 * messaging in a 3-byte typed envelope (magic + type + length) so a receiver can
 * demux by message type and reject a truncated frame, plus a bounded peer
 * registry (DETWS_ESPNOW_MAX_PEERS, no heap). The envelope codec + registry are
 * pure and host-tested; the radio path (begin / add_peer / send / broadcast over
 * esp_now, decoded frames to a callback) is ESP32-only and can bridge to
 * WebSocket/SSE. No stdlib.
 */
#ifndef DETWS_ENABLE_ESPNOW
#define DETWS_ENABLE_ESPNOW 0
#endif

/**
 * @brief OAuth2 token-endpoint client (DETWS_ENABLE_OAUTH2).
 *
 * Default off. services/oauth2 obtains tokens - the counterpart to the OIDC
 * ID-token verifier. It builds the percent-encoded form body for the
 * authorization_code and refresh_token grants (RFC 6749), supporting a
 * confidential client (client_secret) or a public client with PKCE
 * (code_verifier, RFC 7636), and parses the JSON token response (reusing the
 * zero-heap JSON reader). The build + parse core is pure and host-tested; the POST
 * to the token endpoint uses the HTTP(S) client (needs DETWS_ENABLE_HTTP_CLIENT).
 * No heap, no stdlib.
 */
#ifndef DETWS_ENABLE_OAUTH2
#define DETWS_ENABLE_OAUTH2 0
#endif

/**
 * @brief OPC UA Binary server (DETWS_ENABLE_OPCUA).
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
#ifndef DETWS_ENABLE_OPCUA
#define DETWS_ENABLE_OPCUA 0
#endif

/**
 * @brief OPC UA Binary client (DETWS_ENABLE_OPCUA_CLIENT).
 *
 * Default off. Requires DETWS_ENABLE_OPCUA (shares the codec). services/opcua_client
 * provides the client side of the OPC UA Binary protocol: request builders (Hello,
 * OpenSecureChannel, CreateSession, ActivateSession, Read, Browse, CloseSession,
 * CloseSecureChannel) and response parsers, reusing the opcua.h codec. It is
 * transport-agnostic - the app supplies the outbound socket (e.g. an Arduino
 * WiFiClient) and feeds bytes through these pure builders/parsers. No heap, no stdlib.
 */
#ifndef DETWS_ENABLE_OPCUA_CLIENT
#define DETWS_ENABLE_OPCUA_CLIENT 0
#endif

/**
 * @brief Streaming file upload: POST a body straight to a file on the filesystem.
 *
 * Default off. When set, src/services/upload_service.h registers a POST route
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
#ifndef DETWS_ENABLE_UPLOAD
#define DETWS_ENABLE_UPLOAD 0
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
#if DETWS_ENABLE_OTA || DETWS_ENABLE_UPLOAD || DETWS_ENABLE_WEBDAV
#define DETWS_ENABLE_STREAM_BODY 1
#else
#define DETWS_ENABLE_STREAM_BODY 0
#endif

// Streamed uploads need the RX ring to hold a full TCP receive window or the peer
// overruns it and the transfer deadlocks (ack-on-consume reopens the window only as
// the ring drains). If RX_BUF_SIZE was left at its default, upsize it to a value
// that comfortably exceeds the usual TCP_WND (~5.7 KB) when streaming is enabled.
// An explicit RX_BUF_SIZE (build flag) is respected unchanged - set it >= TCP_WND.
#if DETWS_ENABLE_STREAM_BODY && defined(DETWS_RX_BUF_SIZE_DEFAULTED) && RX_BUF_SIZE < 8192
#undef RX_BUF_SIZE
#define RX_BUF_SIZE 8192
#endif

// A modern SSH client's first flight (identification banner + KEXINIT) is ~1.5 KB:
// post-quantum/curve kex names, cert host-key algs, EtM MACs, ext-info-c. The RX
// ring must hold it or the handshake resets at key exchange, so when SSH is enabled
// and RX_BUF_SIZE was left at its default, upsize it to fit a full KEXINIT. An
// explicit RX_BUF_SIZE (build flag) is respected unchanged - keep it >= 2 KB.
#if DETWS_ENABLE_SSH && defined(DETWS_RX_BUF_SIZE_DEFAULTED) && RX_BUF_SIZE < 2048
#undef RX_BUF_SIZE
#define RX_BUF_SIZE 2048
#endif

/** @brief First-boot WiFi provisioning: softAP + captive-portal credentials form. */
#ifndef DETWS_ENABLE_PROVISIONING
#define DETWS_ENABLE_PROVISIONING 0
#endif

/**
 * @brief Syslog client (RFC 5424 over UDP).
 *
 * Default off. When set, the device can ship log lines to a remote syslog server
 * (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the
 * transport-layer UDP service - a zero-heap structured-logging sink for fleets
 * of constrained devices. See src/services/syslog/syslog.h.
 */
#ifndef DETWS_ENABLE_SYSLOG
#define DETWS_ENABLE_SYSLOG 0
#endif

/** @brief Maximum formatted syslog datagram length in bytes (RFC 5424 line). */
#ifndef DETWS_SYSLOG_MSG_MAX
#define DETWS_SYSLOG_MSG_MAX 256
#endif

/** @brief Maximum syslog HOSTNAME / APP-NAME field length (including NUL). */
#ifndef DETWS_SYSLOG_FIELD_MAX
#define DETWS_SYSLOG_FIELD_MAX 32
#endif

/** @brief Default syslog collector UDP port (RFC 5426 well-known 514; overridable at runtime
 *  via syslog_init and here for a non-standard collector). */
#ifndef DETWS_SYSLOG_DEFAULT_PORT
#define DETWS_SYSLOG_DEFAULT_PORT 514
#endif

/**
 * @brief JWT bearer-token authentication (HS256).
 *
 * Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer
 * <jwt>` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can
 * read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a
 * stateless token. Signature verification is constant-time.
 */
#ifndef DETWS_ENABLE_JWT
#define DETWS_ENABLE_JWT 0
#endif

/** @brief Maximum accepted JWT length in bytes (header.payload.signature). */
#ifndef DETWS_JWT_MAX_LEN
#define DETWS_JWT_MAX_LEN 512
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
#ifndef DETWS_ENABLE_HTTP_CLIENT
#define DETWS_ENABLE_HTTP_CLIENT 0
#endif

/** @brief HTTPS client support inside the HTTP client (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_HTTP_CLIENT_TLS
#define DETWS_ENABLE_HTTP_CLIENT_TLS 0
#endif

/** @brief Receive buffer (and max response size) for the outbound HTTP client, bytes. */
#ifndef DETWS_HTTP_CLIENT_BUF_SIZE
#define DETWS_HTTP_CLIENT_BUF_SIZE 2048
#endif

/**
 * @brief Ciphertext receive-ring size for the https:// client, bytes.
 *
 * The lwIP recv callback feeds TLS wire bytes into this draining ring while the
 * TLS engine pulls and decrypts them, so it holds only the in-flight (not yet
 * decrypted) ciphertext: a multi-KB handshake flight fits without loss thanks to
 * the refuse-and-redeliver backpressure. Must exceed one TCP segment (TCP_MSS,
 * ~1460) or a full segment could never fit. Only used when
 * DETWS_ENABLE_HTTP_CLIENT_TLS is set.
 */
#ifndef DETWS_HTTP_CLIENT_CT_BUF_SIZE
#define DETWS_HTTP_CLIENT_CT_BUF_SIZE 4096
#endif

/** @brief Outbound HTTP client connect/response timeout in milliseconds. */
#ifndef DETWS_HTTP_CLIENT_TIMEOUT_MS
#define DETWS_HTTP_CLIENT_TIMEOUT_MS 8000
#endif

/**
 * @brief Outbound SMTP client (RFC 5321) for device email alerts (services/smtp).
 *
 * A blocking one-shot `smtp_send()`: EHLO, optional AUTH LOGIN, MAIL FROM / RCPT TO /
 * DATA over the shared client transport (`det_client`), with implicit TLS (SMTPS, e.g.
 * :465) when the message config sets `tls` and DETWS_ENABLE_TLS is on. Zero heap; the
 * dialogue engine (`smtp_run`) takes a send/recv seam so it is host-tested without lwIP.
 * "SMS fallback" rides on top - most carriers accept an email-to-SMS gateway address.
 */
#ifndef DETWS_ENABLE_SMTP
#define DETWS_ENABLE_SMTP 0
#endif

/** @brief Max length of one SMTP command / address line (bytes, incl. CRLF). */
#ifndef DETWS_SMTP_LINE_MAX
#define DETWS_SMTP_LINE_MAX 256
#endif

/** @brief Max size of the assembled DATA payload (headers + dot-stuffed body), bytes. */
#ifndef DETWS_SMTP_MSG_MAX
#define DETWS_SMTP_MSG_MAX 2048
#endif

/** @brief Max size of one (possibly multi-line) server reply held while parsing, bytes. */
#ifndef DETWS_SMTP_REPLY_MAX
#define DETWS_SMTP_REPLY_MAX 512
#endif

/** @brief SMTP connect / per-reply timeout in milliseconds. */
#ifndef DETWS_SMTP_TIMEOUT_MS
#define DETWS_SMTP_TIMEOUT_MS 10000
#endif

/** @brief Ciphertext receive-ring size for SMTPS, bytes (only used when the message is TLS). */
#ifndef DETWS_SMTP_CT_BUF_SIZE
#define DETWS_SMTP_CT_BUF_SIZE 4096
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
#ifndef DETWS_ENABLE_MQTT
#define DETWS_ENABLE_MQTT 0
#endif

/** @brief MQTTS: run the MQTT client over client-side TLS (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_MQTT_TLS
#define DETWS_ENABLE_MQTT_TLS 0
#endif

/**
 * @brief MQTT packet buffer size in bytes (bounds one outgoing/incoming packet).
 *
 * Two buffers of this size live in BSS (one tx, one rx). Must hold the largest
 * CONNECT/PUBLISH the client sends and the largest incoming PUBLISH it accepts
 * (topic + payload + a few header bytes); larger incoming packets are dropped.
 */
#ifndef DETWS_MQTT_BUF_SIZE
#define DETWS_MQTT_BUF_SIZE 1024
#endif

/** @brief Default MQTT keep-alive interval in seconds (PINGREQ cadence / CONNECT field). */
#ifndef DETWS_MQTT_KEEPALIVE_S
#define DETWS_MQTT_KEEPALIVE_S 30
#endif

/** @brief Ciphertext receive-ring size for MQTTS (draining ring; must exceed one TCP_MSS). */
#ifndef DETWS_MQTT_CT_BUF_SIZE
#define DETWS_MQTT_CT_BUF_SIZE 4096
#endif

/** @brief Maximum inbound MQTT topic length (including NUL) delivered to the callback. */
#ifndef DETWS_MQTT_MAX_TOPIC
#define DETWS_MQTT_MAX_TOPIC 128
#endif

/**
 * @brief Outbound QoS 1/2 in-flight slots (unacknowledged messages held for DUP retransmit).
 *
 * Each slot stores its serialized packet (up to DETWS_MQTT_INFLIGHT_BUF bytes) until
 * the broker acknowledges it; a publish is refused when all slots are busy. The pool
 * costs DETWS_MQTT_MAX_INFLIGHT * (DETWS_MQTT_INFLIGHT_BUF + a few bytes) of BSS.
 */
#ifndef DETWS_MQTT_MAX_INFLIGHT
#define DETWS_MQTT_MAX_INFLIGHT 4
#endif

/** @brief Stored-packet size per in-flight QoS 1/2 slot (caps a retransmittable PUBLISH). */
#ifndef DETWS_MQTT_INFLIGHT_BUF
#define DETWS_MQTT_INFLIGHT_BUF 256
#endif

/** @brief Retransmit timeout (ms) for an unacknowledged in-flight QoS 1/2 message. */
#ifndef DETWS_MQTT_RETRANSMIT_MS
#define DETWS_MQTT_RETRANSMIT_MS 5000
#endif

/** @brief Inbound QoS 2 packet-id de-duplication ring depth (PUBREC-acknowledged, awaiting PUBREL). */
#ifndef DETWS_MQTT_RX_QOS2_SLOTS
#define DETWS_MQTT_RX_QOS2_SLOTS 8
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
#ifndef DETWS_ENABLE_WS_CLIENT
#define DETWS_ENABLE_WS_CLIENT 0
#endif

/** @brief wss://: run the WebSocket client over client-side TLS (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_WS_CLIENT_TLS
#define DETWS_ENABLE_WS_CLIENT_TLS 0
#endif

/** @brief WebSocket client send/receive buffer size in bytes (bounds one frame). */
#ifndef DETWS_WS_CLIENT_BUF_SIZE
#define DETWS_WS_CLIENT_BUF_SIZE 1024
#endif

/** @brief Ciphertext receive-ring size for wss:// (draining ring; must exceed one TCP_MSS). */
#ifndef DETWS_WS_CLIENT_CT_BUF_SIZE
#define DETWS_WS_CLIENT_CT_BUF_SIZE 4096
#endif

/**
 * @brief Internal: client-side TLS engine is compiled (HTTPS client, MQTTS, and/or wss client).
 *
 * The outbound HTTP client (one-shot exchange) and the MQTT / WebSocket clients
 * (persistent sessions) share the same client mbedTLS code in det_tls - the
 * CA/pin trust config, the BIO typedefs, and the session API - gated by this.
 */
#if DETWS_ENABLE_HTTP_CLIENT_TLS || DETWS_ENABLE_MQTT_TLS || DETWS_ENABLE_WS_CLIENT_TLS
#define DETWS_ENABLE_CLIENT_TLS 1
#else
#define DETWS_ENABLE_CLIENT_TLS 0
#endif

// The outbound clients (det_client) resolve hostnames through the shared DNS
// resolver (detws_dns_resolve), so enabling any client implies the resolver - one
// owner of the gethostbyname-marshal pattern instead of a private copy per client.
// DETWS_NEED_DET_CLIENT marks when the client transport is actually used; the
// det_client translation unit compiles its body only then (a server-only Arduino
// build that does not enable a client must not reference the resolver symbols).
// Every feature that drives the outbound client transport must pull it in: the direct callers
// (http_client / mqtt / ws_client / relay / smtp / ssh port-forward) and the seam-based engines
// whose shipped example binds the seam to det_client (smb / dnc). Miss one and its det_client_open
// resolves to the !NEED stub that returns -1, so the feature silently never connects on device.
#if DETWS_ENABLE_HTTP_CLIENT || DETWS_ENABLE_MQTT || DETWS_ENABLE_WS_CLIENT || DETWS_ENABLE_RELAY ||                   \
    DETWS_ENABLE_SMTP || DETWS_SSH_PORT_FORWARD || DETWS_ENABLE_SMB || DETWS_ENABLE_DNC
#undef DETWS_ENABLE_DNS_RESOLVER
#define DETWS_ENABLE_DNS_RESOLVER 1
#define DETWS_NEED_DET_CLIENT 1
#endif
#ifndef DETWS_NEED_DET_CLIENT
#define DETWS_NEED_DET_CLIENT 0
#endif

// ---------------------------------------------------------------------------
// Full Authorization-header capture (internal)
// ---------------------------------------------------------------------------
// Digest auth and JWT bearer tokens both carry an Authorization value far longer
// than MAX_VAL_LEN, so the parser captures the whole header into a dedicated
// per-request buffer (HttpReq::authorization) when either feature is enabled.

/** @brief True when the parser must capture the full Authorization header value. */
#if DETWS_ENABLE_AUTH || DETWS_ENABLE_JWT || DETWS_ENABLE_OIDC
#define DETWS_CAPTURE_AUTH_HEADER 1
#else
#define DETWS_CAPTURE_AUTH_HEADER 0
#endif

/**
 * @brief Capacity of HttpReq::authorization (full Authorization header value).
 *
 * Sized to the largest enabled consumer: a Digest header (DIGEST_AUTH_HDR_MAX), a
 * `Bearer <jwt>` HS256 token (DETWS_JWT_MAX_LEN), or a `Bearer <id_token>` OIDC
 * RS256 token (DETWS_OIDC_MAX_LEN), each plus the scheme.
 */
#if DETWS_ENABLE_OIDC
#define DETWS_AUTH_HDR_CAP_OIDC (DETWS_OIDC_MAX_LEN + 16)
#else
#define DETWS_AUTH_HDR_CAP_OIDC 0
#endif
#if DETWS_ENABLE_JWT
#define DETWS_AUTH_HDR_CAP_JWT (DETWS_JWT_MAX_LEN + 16)
#else
#define DETWS_AUTH_HDR_CAP_JWT 0
#endif
#define DETWS_AUTH_HDR_CAP_M1                                                                                          \
    (DETWS_AUTH_HDR_CAP_JWT > DIGEST_AUTH_HDR_MAX ? DETWS_AUTH_HDR_CAP_JWT : DIGEST_AUTH_HDR_MAX)
#define DETWS_AUTH_HDR_CAP                                                                                             \
    (DETWS_AUTH_HDR_CAP_OIDC > DETWS_AUTH_HDR_CAP_M1 ? DETWS_AUTH_HDR_CAP_OIDC : DETWS_AUTH_HDR_CAP_M1)

/** @brief Runtime stats endpoint (uptime, request/error counts, pool usage, heap). */
#ifndef DETWS_ENABLE_STATS
#define DETWS_ENABLE_STATS 0
#endif

/**
 * @brief Transport-layer observability: connection event hook + counters.
 *
 * Default off (zero cost when unset - the notify points compile to nothing).
 * When set, the transport (L4) fires an application callback on every connection
 * state transition - det_conn_on_event(slot, old_state, new_state, reason) - and
 * maintains lock-free counters (accepts, closes by reason, idle timeouts, RX
 * backpressure events, dropped deferred events, and a live CONN_CLOSING gauge)
 * readable via det_conn_counters(). This is the only state-transition trace the
 * L4/L5 core exposes; pair it with DETWS_ENABLE_STATS for request-level metrics.
 */
#ifndef DETWS_ENABLE_OBSERVABILITY
#define DETWS_ENABLE_OBSERVABILITY 0
#endif

/**
 * @brief Prometheus `/metrics` endpoint (text exposition format 0.0.4).
 *
 * Default off (requires DETWS_ENABLE_STATS for the underlying counters). When
 * set, DetWebServer::metrics() emits the runtime stats as Prometheus metrics
 * (`detws_uptime_seconds`, `detws_http_requests_total`,
 * `detws_http_responses_total{class=...}`, `detws_active_connections`,
 * `detws_free_heap_bytes`, ...) so a Prometheus server can scrape the device.
 */
#ifndef DETWS_ENABLE_METRICS
#define DETWS_ENABLE_METRICS 0
#endif

/**
 * @brief Browser "web serial" terminal over WebSocket (src/services/web_terminal).
 *
 * Serves a self-contained terminal page and a WebSocket endpoint: device output
 * is broadcast to all connected browsers, browser input is delivered to a
 * command callback. Requires DETWS_ENABLE_WEBSOCKET. Default off.
 */
#ifndef DETWS_ENABLE_WEB_TERMINAL
#define DETWS_ENABLE_WEB_TERMINAL 0
#endif

/**
 * @brief Stack scratch for detws_web_terminal_printf()/println() formatting.
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
#ifndef DETWS_ENABLE_ETAG
#define DETWS_ENABLE_ETAG 0
#endif

/**
 * @brief Expose a diagnostic JSON endpoint via server.diag().
 *
 * Disabled by default - enabling it exposes compile-time configuration
 * (buffer sizes, feature flags) which could aid an attacker.  Only
 * enable in development or behind an authenticated route.
 *
 * When enabled, DETWS_DIAG_JSON is a compile-time string constant you can
 * serve from any route handler:
 * @code
 *   server.on("/diag", HTTP_GET, [](uint8_t id, HttpReq *) {
 *       server.diag(id);        // convenience wrapper
 *       // or:
 *       server.send(id, 200, "application/json", DETWS_DIAG_JSON);
 *   });
 * @endcode
 */
#ifndef DETWS_ENABLE_DIAG
#define DETWS_ENABLE_DIAG 0
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
 * and any non-PARSE_COMPLETE path) always close, since the next request boundary
 * is unknown. Idle keep-alive connections are still reclaimed by the existing
 * conn_timeout sweep, and each connection serves at most
 * DETWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.
 */
#ifndef DETWS_ENABLE_KEEPALIVE
#define DETWS_ENABLE_KEEPALIVE 0
#endif

/**
 * @brief Maximum requests served on one keep-alive connection before it is closed.
 *
 * A fairness bound so a single client cannot hold a connection slot
 * indefinitely with a steady request stream. After this many responses the
 * server emits `Connection: close` and drops the link; the client simply
 * reconnects. Only meaningful when DETWS_ENABLE_KEEPALIVE is set.
 */
#ifndef DETWS_KEEPALIVE_MAX_REQUESTS
#define DETWS_KEEPALIVE_MAX_REQUESTS 100
#endif

/**
 * @brief HTTP/2 (RFC 9113) over the version-agnostic request/response core.
 *
 * Default off. When set, the server negotiates HTTP/2 via TLS ALPN ("h2") and speaks the binary
 * framing + HPACK header compression (RFC 7541) on top of the same routes/handlers as HTTP/1.1
 * (the response serializer is version-neutral). The HPACK codec and the frame layer are pure and
 * host-tested; the connection/stream state machine plugs in as a ProtoHandler.
 */
#ifndef DETWS_ENABLE_HTTP2
#define DETWS_ENABLE_HTTP2 0
#endif

/**
 * @brief Per-connection HPACK dynamic-table size in bytes (our decoder; advertised to the peer
 * as SETTINGS_HEADER_TABLE_SIZE). RFC 7541's default is 4096; lower it to save per-connection
 * RAM (each active HTTP/2 connection holds one table).
 */
#ifndef DETWS_HPACK_TABLE_BYTES
#define DETWS_HPACK_TABLE_BYTES 4096
#endif

/** @brief Max HPACK dynamic-table entries (>= DETWS_HPACK_TABLE_BYTES / 32, the min entry size). */
#ifndef DETWS_HPACK_MAX_ENTRIES
#define DETWS_HPACK_MAX_ENTRIES 128
#endif

/**
 * @brief Largest HTTP/2 frame we accept, in bytes (advertised as SETTINGS_MAX_FRAME_SIZE). RFC
 * 9113 requires accepting at least 16384; a whole frame is buffered for reassembly, so this
 * (plus the HPACK table) sets the per-HTTP/2-connection RAM. Range: [16384, 16777215].
 */
#ifndef DETWS_H2_MAX_FRAME
#define DETWS_H2_MAX_FRAME 16384
#endif

/** @brief Max concurrent HTTP/2 streams per connection (advertised as MAX_CONCURRENT_STREAMS). */
#ifndef DETWS_H2_MAX_STREAMS
#define DETWS_H2_MAX_STREAMS 8
#endif

/**
 * @brief Header-block reassembly buffer for HTTP/2 requests that span HEADERS + CONTINUATION
 * frames (a single END_HEADERS frame decodes in place and needs no copy). Caps the compressed
 * request-header size; a larger block is rejected (RFC 9113 sec 6.10).
 */
#ifndef DETWS_H2_HDR_BLOCK
#define DETWS_H2_HDR_BLOCK 4096
#endif

/**
 * @brief Place the HTTP/2 connection-engine pool in external PSRAM (ESP32).
 *
 * Each HTTP/2 connection needs a ~28 KB engine, so the pool (MAX_CONNS of them) does not fit the
 * ~122 KB internal DRAM alongside a TLS server - HTTP/2 therefore requires PSRAM. Set this to 1
 * on a PSRAM board (S3 / P4 / WROVER) to move the pool to external RAM via `EXT_RAM_BSS_ATTR`.
 * Like DETWS_TLS_ARENA_IN_PSRAM it needs a framework built with
 * `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y` (the stock arduino-esp32 core ships it off; see
 * tools/psram/README.md). A compile-time guard rejects DETWS_ENABLE_HTTP2 without this on ARDUINO.
 */
#ifndef DETWS_H2_POOL_IN_PSRAM
#define DETWS_H2_POOL_IN_PSRAM 0
#endif

/**
 * @brief HTTP/3 (RFC 9114) over QUIC (RFC 9000) - in progress, built codec-first.
 *
 * Default off. HTTP/3 runs over QUIC (a reliable transport over UDP) with QPACK (RFC 9204)
 * header compression and its own binary framing. The pure, host-testable pieces land first - the
 * QUIC variable-length integer (RFC 9000 sec 16), the HTTP/3 + QPACK codecs - ahead of the QUIC
 * transport engine. Like HTTP/2 this is a PSRAM-class feature.
 */
#ifndef DETWS_ENABLE_HTTP3
#define DETWS_ENABLE_HTTP3 0
#endif

// Internal request-dispatch slots appended to the connection pool for non-TCP transports.
// HTTP/3 runs over QUIC/UDP and has no accept-time TCP slot, but it reuses the same request
// pipeline (match_and_execute + send), which is indexed by a connection-pool slot. One reserved
// slot at index MAX_CONNS lets an HTTP/3 request run through that pipeline. The TCP accept path only
// ever scans [0, MAX_CONNS), and this slot is driven synchronously by the HTTP/3 poll on the worker
// thread, so there is no accept race. CONN_POOL_SLOTS sizes conn_pool / http_pool / the per-slot
// response-header buffer; every TCP loop still bounds itself with MAX_CONNS.
#if DETWS_ENABLE_HTTP3
#define DETWS_INTERNAL_SLOTS 1
#define DETWS_H3_DISPATCH_SLOT MAX_CONNS ///< reserved conn-pool slot an HTTP/3 request dispatches through
#else
#define DETWS_INTERNAL_SLOTS 0
#endif
#define CONN_POOL_SLOTS (MAX_CONNS + DETWS_INTERNAL_SLOTS)

/** @brief UDP port the HTTP/3 (QUIC) server binds by default (used by DetWebServer::h3_cert). */
#ifndef DETWS_HTTP3_PORT
#define DETWS_HTTP3_PORT 443
#endif

/**
 * @brief Maximum bytes of one QUIC/TLS handshake CRYPTO flight (RFC 9001).
 *
 * The server's second flight - EncryptedExtensions + Certificate + CertificateVerify + Finished -
 * is assembled whole before it is fragmented into CRYPTO frames across Handshake packets. The
 * Certificate (a DER X.509 chain) dominates the size, so this bounds the certificate the server can
 * present. The default fits a single Ed25519 leaf certificate comfortably; raise it for a chain.
 */
#ifndef DETWS_H3_CRYPTO_BUF
#define DETWS_H3_CRYPTO_BUF 2048
#endif

/**
 * @brief Maximum concurrent request streams per HTTP/3 connection.
 *
 * Bounds the per-connection QUIC stream table (client-initiated bidirectional request streams plus
 * the handful of unidirectional control / QPACK streams). Each slot is small; 8 matches the HTTP/2
 * default (DETWS_H2_MAX_STREAMS).
 */
#ifndef DETWS_H3_MAX_STREAMS
#define DETWS_H3_MAX_STREAMS 8
#endif

/**
 * @brief HTTP Range requests / 206 Partial Content for served files.
 *
 * Default off. When set (requires DETWS_ENABLE_FILE_SERVING), serve_file() /
 * serve_static() honor a single-range `Range: bytes=...` request header: they
 * answer `206 Partial Content` with a `Content-Range` header and stream only the
 * requested bytes (seeking the file to the start offset), advertise
 * `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range
 * with `416 Range Not Satisfiable`. This enables resumable downloads and media
 * seeking. Multi-range (multipart/byteranges) requests are not supported - the
 * server falls back to a full 200 response, which is RFC 7233 §3.1 compliant.
 */
#ifndef DETWS_ENABLE_RANGE
#define DETWS_ENABLE_RANGE 0
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
#ifndef DETWS_ENFORCE_HOST_HEADER
#define DETWS_ENFORCE_HOST_HEADER 1
#endif

/**
 * @brief Allow SSH password authentication (default on).
 *
 * Set to 0 to harden the SSH server to publickey-only authentication
 * (RFC 4252 §7): the "password" method is then refused outright and is not
 * advertised in the USERAUTH_FAILURE method list. Publickey auth is always
 * available regardless of this flag.
 */
#ifndef DETWS_SSH_ALLOW_PASSWORD
#define DETWS_SSH_ALLOW_PASSWORD 1
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
#ifndef DETWS_MAX_UDP_LISTENERS
#define DETWS_MAX_UDP_LISTENERS 2
#endif

/**
 * @brief Shared receive-scratch size for the transport-layer UDP service.
 *
 * One static buffer (lwIP delivers a single datagram at a time) into which each
 * incoming datagram is copied before the handler runs. Must hold the largest
 * datagram any UDP service expects (SNMP messages are the largest user).
 */
#ifndef DETWS_UDP_RX_BUF_SIZE
#define DETWS_UDP_RX_BUF_SIZE 1472
#endif

/**
 * @brief Opt-in global accept-rate throttle (connection-flood defense).
 *
 * Default off (zero cost / no behavior change). When set to 1 the accept
 * callback rejects new connections once more than DETWS_ACCEPT_THROTTLE_MAX
 * have been accepted within a DETWS_ACCEPT_THROTTLE_WINDOW_MS fixed window
 * (global across all listeners, two static counters - no per-IP table). This
 * bounds connection churn (e.g. reconnect brute-force) on top of the bounded
 * connection pool and the per-connection auth limits. mitigate finer-grained /
 * per-IP attacks at the network layer.
 */
#ifndef DETWS_ENABLE_ACCEPT_THROTTLE
#define DETWS_ENABLE_ACCEPT_THROTTLE 0
#endif

/** @brief Max accepted connections per throttle window (see DETWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DETWS_ACCEPT_THROTTLE_MAX
#define DETWS_ACCEPT_THROTTLE_MAX 20
#endif

/** @brief Throttle window length in milliseconds (see DETWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DETWS_ACCEPT_THROTTLE_WINDOW_MS
#define DETWS_ACCEPT_THROTTLE_WINDOW_MS 1000
#endif

/**
 * @brief Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4).
 *
 * Default off (zero cost / no behavior change). Complements the global accept
 * throttle: the accept callback rejects a new connection once one source IPv4
 * address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within a
 * DETWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of
 * DETWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source
 * addresses; when a new address arrives and the table is full, an expired or
 * least-recently-started bucket is reused, so memory stays bounded (no heap).
 *
 * This bounds reconnect/brute-force churn from a single host (the gap left by the
 * global throttle, which cannot tell one noisy client from many). It is
 * best-effort: an attacker spreading across many source addresses can still churn
 * the bounded connection pool, so combine it with the global throttle and
 * network-layer filtering.
 */
#ifndef DETWS_ENABLE_PER_IP_THROTTLE
#define DETWS_ENABLE_PER_IP_THROTTLE 0
#endif

/** @brief Number of source IPv4 addresses tracked by the per-IP throttle (BSS bucket table). */
#ifndef DETWS_PER_IP_THROTTLE_SLOTS
#define DETWS_PER_IP_THROTTLE_SLOTS 16
#endif

/** @brief Max accepted connections per window from one source IP (see DETWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DETWS_PER_IP_THROTTLE_MAX
#define DETWS_PER_IP_THROTTLE_MAX 10
#endif

/** @brief Per-IP throttle window length in milliseconds (see DETWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DETWS_PER_IP_THROTTLE_WINDOW_MS
#define DETWS_PER_IP_THROTTLE_WINDOW_MS 10000
#endif

// ---------------------------------------------------------------------------
// Source-IP allowlist  (accept-time firewall; DETWS_ENABLE_IP_ALLOWLIST)
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
 * out. Rules live in a fixed BSS table of DETWS_IP_ALLOWLIST_SLOTS entries (no heap).
 *
 * This is a coarse first-line filter - a spoofed source address can still pass
 * it - so combine it with the accept throttles and network-layer filtering.
 */
#ifndef DETWS_ENABLE_IP_ALLOWLIST
#define DETWS_ENABLE_IP_ALLOWLIST 0
#endif

/** @brief Number of CIDR rules the source-IP allowlist can hold (BSS table). */
#ifndef DETWS_IP_ALLOWLIST_SLOTS
#define DETWS_IP_ALLOWLIST_SLOTS 8
#endif

// ---------------------------------------------------------------------------
// Brute-force auth lockout  (per-source-IP; DETWS_ENABLE_AUTH_LOCKOUT)
// ---------------------------------------------------------------------------

/**
 * @brief Opt-in per-IP brute-force lockout for HTTP auth (requires DETWS_ENABLE_AUTH).
 *
 * Default off (zero cost / no behavior change). When set, the auth gate counts
 * consecutive failed authentications per source address (IPv4 or IPv6, keyed on
 * the full address) in a fixed BSS table; after
 * DETWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for
 * DETWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to
 * DETWS_AUTH_LOCKOUT_MAX_MS. A locked address gets 429 (Retry-After) with no
 * credential check; a successful auth clears it. Bounded memory (no heap); the
 * table evicts idle, then least-recently-used, addresses when full.
 */
#ifndef DETWS_ENABLE_AUTH_LOCKOUT
#define DETWS_ENABLE_AUTH_LOCKOUT 0
#endif

/** @brief Number of source IPs the auth lockout tracks (BSS bucket table). */
#ifndef DETWS_AUTH_LOCKOUT_SLOTS
#define DETWS_AUTH_LOCKOUT_SLOTS 16
#endif

/** @brief Consecutive failed auths from one IP before it is locked out. */
#ifndef DETWS_AUTH_LOCKOUT_THRESHOLD
#define DETWS_AUTH_LOCKOUT_THRESHOLD 5
#endif

/** @brief First lockout duration in ms; doubles on each further failure. */
#ifndef DETWS_AUTH_LOCKOUT_BASE_MS
#define DETWS_AUTH_LOCKOUT_BASE_MS 1000
#endif

/** @brief Maximum lockout duration in ms (the exponential backoff cap). */
#ifndef DETWS_AUTH_LOCKOUT_MAX_MS
#define DETWS_AUTH_LOCKOUT_MAX_MS 300000
#endif

// ---------------------------------------------------------------------------
// CSRF protection  (DETWS_ENABLE_CSRF)
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
 * DETWS_ENABLE_AUTH.
 */
#ifndef DETWS_ENABLE_CSRF
#define DETWS_ENABLE_CSRF 0
#endif

// ---------------------------------------------------------------------------
// Telnet sizing constants  (DETWS_ENABLE_TELNET must be 1)
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
// SSH sizing constants  (DETWS_ENABLE_SSH must be 1)
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
 * (ssh_chan[MAX_SSH_CONNS][DETWS_SSH_MAX_CHANNELS]), no heap.
 */
#ifndef DETWS_SSH_MAX_CHANNELS
#define DETWS_SSH_MAX_CHANNELS 1
#endif
#if DETWS_SSH_MAX_CHANNELS < 1
#error "DeterministicESPAsyncWebServer: DETWS_SSH_MAX_CHANNELS must be >= 1"
#endif

/**
 * @brief SSH TCP port forwarding (`direct-tcpip`, i.e. `ssh -L`). Default off.
 *
 * When set, the SSH server can open an outbound TCP connection to a client-named
 * host:port and bridge bytes between that socket and the SSH channel - the
 * `ssh_forward` owner does the I/O via the outbound client transport (det_client),
 * so it needs `DETWS_CLIENT_CONNS >= DETWS_SSH_FWD_MAX` and a channel pool
 * (`DETWS_SSH_MAX_CHANNELS > 1`) to be useful. Forwarding is still opt-in at
 * runtime: nothing is forwarded until the application calls `ssh_forward_begin()`.
 * Off = the channel codec refuses every `direct-tcpip` open (no open relay).
 */
#ifndef DETWS_SSH_PORT_FORWARD
#define DETWS_SSH_PORT_FORWARD 0
#endif

/** @brief Maximum concurrent forwarded TCP connections (must be <= DETWS_CLIENT_CONNS). */
#ifndef DETWS_SSH_FWD_MAX
#define DETWS_SSH_FWD_MAX 2
#endif

/** @brief Maximum forward target hostname length including null terminator. */
#ifndef DETWS_SSH_FWD_HOST_MAX
#define DETWS_SSH_FWD_HOST_MAX 64
#endif

/** @brief Blocking connect timeout (ms) when opening a forward target. */
#ifndef DETWS_SSH_FWD_CONNECT_MS
#define DETWS_SSH_FWD_CONNECT_MS 3000
#endif

/** @brief Max bytes moved per forward channel per poll, target -> client (<= SSH_PKT_BUF_SIZE). */
#ifndef DETWS_SSH_FWD_CHUNK
#define DETWS_SSH_FWD_CHUNK 1024
#endif

/**
 * @brief Maximum concurrent remote-forward listeners (`ssh -R` / `tcpip-forward`).
 *
 * Each accepted client that requests remote forwarding can bind up to this many
 * ports on the device; each binding consumes one `listener_pool[]` slot, so
 * `MAX_LISTENERS` must have that much headroom above the app's own listeners.
 * Remote forwarding shares `DETWS_SSH_PORT_FORWARD` (compiled in) and is inert
 * until `ssh_forward_begin()`.
 */
#ifndef DETWS_SSH_RFWD_MAX
#define DETWS_SSH_RFWD_MAX 1
#endif

/**
 * @brief Maximum concurrent bridged connections across all remote forwards.
 *
 * Each connection accepted on a forwarded port occupies one transport `conn_pool`
 * slot plus one SSH channel (so it needs `DETWS_SSH_MAX_CHANNELS` headroom) and one
 * entry here while it is bridged back to the client.
 */
#ifndef DETWS_SSH_RFWD_BRIDGE_MAX
#define DETWS_SSH_RFWD_BRIDGE_MAX 2
#endif

/** @brief Packet assembly buffer per SSH connection (bytes). */
#ifndef SSH_PKT_BUF_SIZE
#define SSH_PKT_BUF_SIZE 2048
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
 * guard rejects this on ARDUINO without DETWS_SSH_ZLIB_IN_PSRAM. Requires DETWS_ENABLE_SSH.
 */
#ifndef DETWS_ENABLE_SSH_ZLIB
#define DETWS_ENABLE_SSH_ZLIB 0
#endif

/**
 * @brief Place the per-connection SSH compression state in external PSRAM (ESP32).
 *
 * Like DETWS_H2_POOL_IN_PSRAM / DETWS_TLS_ARENA_IN_PSRAM: moves the compressor pool
 * (MAX_SSH_CONNS of them) to external RAM via `EXT_RAM_BSS_ATTR`. Needs a framework built with
 * `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y` (tools/psram/README.md).
 */
#ifndef DETWS_SSH_ZLIB_IN_PSRAM
#define DETWS_SSH_ZLIB_IN_PSRAM 0
#endif

/**
 * @brief Acknowledge placing the SSH compressor in internal DRAM (no PSRAM).
 *
 * The per-connection compressor is ~48 KB. With MAX_SSH_CONNS=1 and no TLS server it fits internal
 * DRAM on a roomy chip (S3 / P4). Rather than force PSRAM, this mirrors DETWS_TLS_ACK_MULTI_CONN_DRAM:
 * set it to 1 to consciously accept the internal-DRAM cost when DETWS_SSH_ZLIB_IN_PSRAM is off. The
 * build otherwise fails fast on ARDUINO with guidance (below) instead of a raw linker overflow.
 */
#ifndef DETWS_SSH_ZLIB_ACK_DRAM
#define DETWS_SSH_ZLIB_ACK_DRAM 0
#endif

/**
 * @brief SSH s2c DEFLATE sliding-window size in bytes (max back-reference distance). Power of two,
 * 256..32768. Larger = better ratio + more per-connection RAM (the compressor holds a window-sized
 * work buffer + a window-sized hash chain). The client always allocates a 32 KB inflate window, so
 * any value here interoperates; 8 KB is a good ratio/RAM balance for terminal + command output.
 */
#ifndef DETWS_SSH_ZLIB_WINDOW
#define DETWS_SSH_ZLIB_WINDOW 8192
#endif

/**
 * @brief Largest uncompressed payload the s2c compressor accepts in one call (bytes). Outbound SSH
 * payloads are bounded by SSH_PKT_BUF_SIZE; this sizes the compressor's history+input work buffer.
 */
#ifndef DETWS_SSH_ZLIB_MAX_IN
#define DETWS_SSH_ZLIB_MAX_IN 2048
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
#ifndef DETWS_SCRATCH_ARENA_SIZE
#define DETWS_SCRATCH_ARENA_SIZE 8192
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
// │ WEBSOCKET (DETWS_ENABLE_WEBSOCKET=1)                                                        │          │
// │  ws_pool[MAX_WS_CONNS]       │ MAX_WS_CONNS × (WS_FRAME_SIZE + 29)                         │  1 082 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSE (DETWS_ENABLE_SSE=1)     │                                                              │          │
// │  sse_pool[MAX_SSE_CONNS]     │ MAX_SSE_CONNS × (MAX_PATH_LEN + 3)                          │    134 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSH (DETWS_ENABLE_SSH=1)     │                                                              │          │
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
 * Pass a pointer to DetWebServer::begin() or DeterministicAsyncTCP::init().
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
    PROTO_NONE = 0,     ///< Unassigned slot.
    PROTO_HTTP = 1,     ///< HTTP/1.1 with optional WS and SSE upgrades.
    PROTO_TELNET = 2,   ///< Telnet (RFC 854).
    PROTO_SSH = 3,      ///< SSH (RFC 4253/4252/4254).
    PROTO_MODBUS = 4,   ///< Modbus TCP slave (Modbus Application Protocol).
    PROTO_OPCUA = 5,    ///< OPC UA Binary (UA-TCP) server.
    PROTO_SSH_RFWD = 6, ///< SSH remote-forward listener (ssh -R): accepts bridge to a forwarded-tcpip channel.
    PROTO_RELAY = 7,    ///< TCP relay / DNAT (DETWS_ENABLE_RELAY): bridge to an origin det_client connection.
};

/**
 * @brief Network interface a connection arrived on (for per-route filtering).
 *
 * Stamped onto each TcpConn at accept time by comparing the connection's local
 * IP to the softAP IP (see DetWebServer::set_ap_ip()). Used to gate routes to
 * the station or softAP interface only (DetWebServer::on(..., DetIface)).
 */
enum class DetIface : uint8_t
{
    DETIFACE_ANY = 0, ///< Unknown / no filter (matches any interface).
    DETIFACE_STA = 1, ///< Station interface (joined to an AP / your LAN).
    DETIFACE_AP = 2,  ///< softAP interface (clients joined to the device).
    DETIFACE_ETH = 3, ///< Ethernet interface (wired PHY).
};

// ---------------------------------------------------------------------------
// Diagnostic JSON string  (only defined when DETWS_ENABLE_DIAG == 1)
// ---------------------------------------------------------------------------
// DETWS_DIAG_JSON is a compile-time string literal - zero runtime cost.
// Adjacent string literals are concatenated by the compiler; DETWS_STR()
// stringifies an integer macro value without evaluating it twice.

#if DETWS_ENABLE_DIAG

#define _DETWS_STR_(x) #x
#define _DETWS_STR(x) _DETWS_STR_(x)

#if DETWS_ENABLE_WEBSOCKET
#define _DETWS_F_WS "true"
#else
#define _DETWS_F_WS "false"
#endif

#if DETWS_ENABLE_SSE
#define _DETWS_F_SSE "true"
#else
#define _DETWS_F_SSE "false"
#endif

#if DETWS_ENABLE_MULTIPART
#define _DETWS_F_MP "true"
#else
#define _DETWS_F_MP "false"
#endif

#if DETWS_ENABLE_FILE_SERVING
#define _DETWS_F_FS "true"
#else
#define _DETWS_F_FS "false"
#endif

#if DETWS_ENABLE_AUTH
#define _DETWS_F_AUTH "true"
#else
#define _DETWS_F_AUTH "false"
#endif

#define DETWS_DIAG_JSON                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "\"lib\":\"DeterministicESPAsyncWebServer\","                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    "\"features\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"websocket\":" _DETWS_F_WS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"sse\":" _DETWS_F_SSE ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
    "\"multipart\":" _DETWS_F_MP ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"file_serving\":" _DETWS_F_FS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "\"auth\":" _DETWS_F_AUTH "},"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"config\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"MAX_CONNS\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
        MAX_CONNS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                   "\"RX_BUF_SIZE\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                       RX_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
                                    "\"BODY_BUF_SIZE\":" _DETWS_STR(BODY_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                   "\"MAX_ROUTES\":" _DETWS_STR(MAX_ROUTES) ","                                                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                            "\"MAX_"                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                            "HEADERS\""                                                                                                                                                                                                                                                                                                                                                                                 \
                                                                                                                            ":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                MAX_HEADERS) ","                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                             "\"MAX_PATH_"                                                                                                                                                                                                                                                                                                                                                              \
                                                                                                                                             "LEN\""                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                             ":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                 MAX_PATH_LEN) ","                                                                                                                                                                                                                                                                                                                                                      \
                                                                                                                                                               "\"MAX_KEY_LEN\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                           \
                                                                                                                                                                   MAX_KEY_LEN) ","                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                "\"MAX_VAL_LEN\":" _DETWS_STR(MAX_VAL_LEN) ","                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                           "\"MAX_QUERY_LEN\":" _DETWS_STR(MAX_QUERY_LEN) ","                                                                                                                                                                                                                                           \
                                                                                                                                                                                                                                                                          "\"MAX_QUERY_PARAMS\":" _DETWS_STR(MAX_QUERY_PARAMS) ","                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                               "\"CONN_TIMEOUT_MS\":" _DETWS_STR(CONN_TIMEOUT_MS) ","                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                  "\"RESP_HDR_BUF_SIZE\":" _DETWS_STR(                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                      RESP_HDR_BUF_SIZE) ","                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                                                                         "\"WS_HDR_BUF_SIZE\":" _DETWS_STR(                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                             WS_HDR_BUF_SIZE) ","                                                                                       \
                                                                                                                                                                                                                                                                                                                                                                                                                              "\"CORS_HDR_BUF_SIZE\":" _DETWS_STR(CORS_HDR_BUF_SIZE) ","                                \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     "\"EVT_QUEUE_DEPTH\":" _DETWS_STR( \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         EVT_QUEUE_DEPTH) "}"           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          "}"

#endif // DETWS_ENABLE_DIAG

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

#if DETWS_ENABLE_WEBSOCKET && DETWS_ENABLE_SSE
#if MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_WEBSOCKET
#if MAX_WS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_SSE
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

#if DETWS_ENABLE_FILE_SERVING && FILE_CHUNK_SIZE > RX_BUF_SIZE
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
#if DETWS_ENABLE_OIDC && !DETWS_ENABLE_SSH && (DETWS_WORKER_TASK_STACK < DETWS_WORKER_STACK_RSA_MIN)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_WORKER_TASK_STACK is below DETWS_WORKER_STACK_RSA_MIN; RSA-2048 verification (OIDC) needs ~7 KB of worker stack - raise DETWS_WORKER_TASK_STACK (>= 8192) or marshal RSA verifies onto a dedicated larger-stack task"
#endif

// SSH additionally can negotiate curve25519-sha256 + ssh-ed25519, whose software field
// arithmetic peaks at ~10.5 KB of worker stack (deeper than the RSA path). Enforce the
// higher floor so a lowered stack is caught at build time instead of tripping the task
// stack canary on the first modern-crypto handshake.
#if DETWS_ENABLE_SSH && (DETWS_WORKER_TASK_STACK < DETWS_WORKER_STACK_CURVE_MIN)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_WORKER_TASK_STACK is below DETWS_WORKER_STACK_CURVE_MIN; SSH curve25519/ed25519 needs ~10.5 KB of worker stack - raise DETWS_WORKER_TASK_STACK (>= 12288) or marshal the handshake onto a dedicated larger-stack task"
#endif

#if DETWS_ENABLE_TLS
#if MAX_TLS_CONNS < 1 || MAX_TLS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_TLS_CONNS must be between 1 and MAX_CONNS"
#endif
#if DETWS_TLS_ARENA_SIZE < 8192
#error "DeterministicESPAsyncWebServer: DETWS_TLS_ARENA_SIZE is far too small for a TLS handshake"
#endif
// Concurrent TLS guard: the whole arena is static .bss and the ESP32 internal
// dram0_0_seg ceiling is only ~122 KB (ROM-reserved at both ends), so a 2nd
// connection's arena overflows the link. Reject MAX_TLS_CONNS > 1 with a clear
// message unless the arena is offloaded to PSRAM or the build was consciously sized -
// far friendlier than the raw "region `dram0_0_seg' overflowed" linker error.
#if defined(ARDUINO) && (MAX_TLS_CONNS > 1) && !DETWS_TLS_ARENA_IN_PSRAM && !DETWS_TLS_ACK_MULTI_CONN_DRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: MAX_TLS_CONNS > 1 - the static TLS arena will not fit the ~122 KB internal dram0_0_seg. Pick a path (docs/KNOWN_LIMITATIONS.md): set DETWS_TLS_ARENA_IN_PSRAM=1 on a PSRAM board, OR shrink records via a custom ESP-IDF build (CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN + DETWS_TLS_MAX_FRAG_LEN), OR reclaim internal DRAM; then set DETWS_TLS_ACK_MULTI_CONN_DRAM=1 to confirm."
#endif
#endif

// HTTP/2's per-connection engine pool (~MAX_CONNS x 28 KB) cannot fit internal DRAM alongside
// TLS, so it must live in PSRAM. Fail fast with guidance instead of the raw linker overflow.
#if DETWS_ENABLE_HTTP2 && defined(ARDUINO) && !DETWS_H2_POOL_IN_PSRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_ENABLE_HTTP2 needs PSRAM - the HTTP/2 engine pool (~MAX_CONNS x 28 KB) overflows the ~122 KB internal dram0_0_seg alongside TLS. Set DETWS_H2_POOL_IN_PSRAM=1 on a PSRAM board (S3 / P4 / WROVER) built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y (tools/psram/README.md)."
#endif

#if DETWS_ENABLE_SNMP
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

#if DETWS_ENABLE_COAP
#if DETWS_COAP_MAX_RESOURCES < 1
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_RESOURCES must be >= 1"
#endif
#if DETWS_COAP_MAX_PATH < 2
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_PATH must be >= 2 (minimum: \"/\")"
#endif
#if DETWS_COAP_MAX_PAYLOAD < 1
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_PAYLOAD must be >= 1"
#endif
#if DETWS_COAP_MSG_BUF_SIZE < (DETWS_COAP_MAX_PAYLOAD + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_COAP_MSG_BUF_SIZE must be >= DETWS_COAP_MAX_PAYLOAD + 16 (header + token + Content-Format option + payload marker)"
#endif
#endif

#if DETWS_ENABLE_AUTH && MAX_AUTH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_AUTH_LEN must be >= 2 when DETWS_ENABLE_AUTH is set"
#endif

#if DETWS_ENABLE_PER_IP_THROTTLE
#if DETWS_PER_IP_THROTTLE_SLOTS < 1
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_PER_IP_THROTTLE_SLOTS must be >= 1 when DETWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#if DETWS_PER_IP_THROTTLE_MAX < 1
#error "DeterministicESPAsyncWebServer: DETWS_PER_IP_THROTTLE_MAX must be >= 1 when DETWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#endif

#if DETWS_ENABLE_IP_ALLOWLIST && DETWS_IP_ALLOWLIST_SLOTS < 1
#error "DeterministicESPAsyncWebServer: DETWS_IP_ALLOWLIST_SLOTS must be >= 1 when DETWS_ENABLE_IP_ALLOWLIST is set"
#endif

#if DETWS_ENABLE_AUTH_LOCKOUT
#if !DETWS_ENABLE_AUTH
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_AUTH_LOCKOUT requires DETWS_ENABLE_AUTH"
#endif
#if DETWS_AUTH_LOCKOUT_SLOTS < 1
#error "DeterministicESPAsyncWebServer: DETWS_AUTH_LOCKOUT_SLOTS must be >= 1 when DETWS_ENABLE_AUTH_LOCKOUT is set"
#endif
#if DETWS_AUTH_LOCKOUT_THRESHOLD < 1
#error "DeterministicESPAsyncWebServer: DETWS_AUTH_LOCKOUT_THRESHOLD must be >= 1 when DETWS_ENABLE_AUTH_LOCKOUT is set"
#endif
#if DETWS_AUTH_LOCKOUT_BASE_MS < 1 || DETWS_AUTH_LOCKOUT_MAX_MS < DETWS_AUTH_LOCKOUT_BASE_MS
#error "DeterministicESPAsyncWebServer: need 1 <= DETWS_AUTH_LOCKOUT_BASE_MS <= DETWS_AUTH_LOCKOUT_MAX_MS"
#endif
// The backoff doubles a uint32 capped at MAX_MS, so MAX_MS must leave headroom for
// one more shift (cap <= 0x80000000 => cap<<1 fits in uint32 without overflow).
#if DETWS_AUTH_LOCKOUT_MAX_MS > 0x80000000
#error "DeterministicESPAsyncWebServer: DETWS_AUTH_LOCKOUT_MAX_MS must be <= 0x80000000 (2147483648)"
#endif
#endif

#if DETWS_ENABLE_WEBDAV
#if !DETWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WEBDAV requires DETWS_ENABLE_FILE_SERVING"
#endif
#if DETWS_WEBDAV_BUF_SIZE < 256
#error "DeterministicESPAsyncWebServer: DETWS_WEBDAV_BUF_SIZE must be >= 256"
#endif
#if DETWS_METHOD_BUF_SIZE < 10
#error "DeterministicESPAsyncWebServer: DETWS_METHOD_BUF_SIZE must be >= 10 when DETWS_ENABLE_WEBDAV is set (PROPPATCH)"
#endif
#endif

#if DETWS_ENABLE_MODBUS
#if DETWS_MODBUS_COILS < 1 || DETWS_MODBUS_DISCRETE_INPUTS < 1 || DETWS_MODBUS_HOLDING_REGS < 1 ||                     \
    DETWS_MODBUS_INPUT_REGS < 1
#error "DeterministicESPAsyncWebServer: each DETWS_MODBUS_* table size must be >= 1 when DETWS_ENABLE_MODBUS is set"
#endif
#endif

#if DETWS_ENABLE_KEEPALIVE && DETWS_KEEPALIVE_MAX_REQUESTS < 1
#error "DeterministicESPAsyncWebServer: DETWS_KEEPALIVE_MAX_REQUESTS must be >= 1 when DETWS_ENABLE_KEEPALIVE is set"
#endif

#if DETWS_ENABLE_RANGE && !DETWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_RANGE requires DETWS_ENABLE_FILE_SERVING"
#endif

#if DETWS_ENABLE_MTLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_MTLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_TLS_RESUMPTION && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_TLS_RESUMPTION requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_METRICS && !DETWS_ENABLE_STATS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_METRICS requires DETWS_ENABLE_STATS"
#endif

#if DETWS_ENABLE_HTTP_CLIENT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_HTTP_CLIENT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_MQTT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_MQTT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_WS_CLIENT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WS_CLIENT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_SNMP_TRAP && !DETWS_ENABLE_SNMP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_SNMP_TRAP requires DETWS_ENABLE_SNMP"
#endif

#if DETWS_ENABLE_COAP_OBSERVE && !DETWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_COAP_OBSERVE requires DETWS_ENABLE_COAP"
#endif

#if DETWS_ENABLE_COAP_BLOCK
#if !DETWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_COAP_BLOCK requires DETWS_ENABLE_COAP"
#endif
#if DETWS_COAP_BLOCK_SZX_MAX > 6
#error "DeterministicESPAsyncWebServer: DETWS_COAP_BLOCK_SZX_MAX must be <= 6 (block size 2^(SZX+4); SZX 7 is reserved)"
#endif
#if DETWS_COAP_MSG_BUF_SIZE < ((1 << (DETWS_COAP_BLOCK_SZX_MAX + 4)) + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_COAP_MSG_BUF_SIZE must hold one full block (2^(DETWS_COAP_BLOCK_SZX_MAX+4)) + 16 header/option bytes"
#endif
#if DETWS_COAP_BLOCK1_MAX < (1 << (DETWS_COAP_BLOCK_SZX_MAX + 4))
#error "DeterministicESPAsyncWebServer: DETWS_COAP_BLOCK1_MAX must be >= one block (2^(DETWS_COAP_BLOCK_SZX_MAX+4))"
#endif
#endif

// --- feature dependency guards (centralized; see the BUILD-FLAG DEPENDENCY TREE
//     near the top of this file). A child feature requires its parent(s). ---

#if DETWS_ENABLE_WS_DEFLATE && !DETWS_ENABLE_WEBSOCKET
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WS_DEFLATE requires DETWS_ENABLE_WEBSOCKET"
#endif

#if DETWS_ENABLE_SSH_ZLIB && !DETWS_ENABLE_SSH
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_SSH_ZLIB requires DETWS_ENABLE_SSH"
#endif

#if DETWS_ENABLE_SSH_ZLIB
// Window must be a power of two in [256, 32768] (32 KB is zlib's max; the client's inflate window).
#if (DETWS_SSH_ZLIB_WINDOW & (DETWS_SSH_ZLIB_WINDOW - 1)) != 0 || DETWS_SSH_ZLIB_WINDOW < 256 ||                       \
    DETWS_SSH_ZLIB_WINDOW > 32768
#error "DeterministicESPAsyncWebServer: DETWS_SSH_ZLIB_WINDOW must be a power of two in [256, 32768]"
#endif
// Positions index a uint16 hash chain, so window + max-input must stay under 65535.
#if (DETWS_SSH_ZLIB_WINDOW + DETWS_SSH_ZLIB_MAX_IN) > 65534
#error "DeterministicESPAsyncWebServer: DETWS_SSH_ZLIB_WINDOW + DETWS_SSH_ZLIB_MAX_IN must be <= 65534"
#endif
// The compressor must accept a full packet payload, else a max-size outbound packet would fail to
// compress and drop, desyncing the stateful stream.
#if DETWS_SSH_ZLIB_MAX_IN < SSH_PKT_BUF_SIZE
#error "DeterministicESPAsyncWebServer: DETWS_SSH_ZLIB_MAX_IN must be >= SSH_PKT_BUF_SIZE"
#endif
// The per-connection compressor (window work buffer + hash chain) is ~48 KB. On ARDUINO pick a path:
// offload it to PSRAM (DETWS_SSH_ZLIB_IN_PSRAM, a PSRAM board built with the BSS-in-PSRAM core), or
// acknowledge the internal-DRAM cost (DETWS_SSH_ZLIB_ACK_DRAM, fine for MAX_SSH_CONNS=1 without TLS on
// a roomy S3 / P4). Fail fast with guidance instead of a raw linker overflow.
#if defined(ARDUINO) && !DETWS_SSH_ZLIB_IN_PSRAM && !DETWS_SSH_ZLIB_ACK_DRAM
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_ENABLE_SSH_ZLIB - the per-connection compressor is ~48 KB. Set DETWS_SSH_ZLIB_IN_PSRAM=1 on a PSRAM board (S3 / P4 / WROVER, core built with CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y, tools/psram/README.md), OR set DETWS_SSH_ZLIB_ACK_DRAM=1 to accept the internal-DRAM cost (fits MAX_SSH_CONNS=1 without TLS on a roomy chip)."
#endif
#endif

#if DETWS_ENABLE_WEB_TERMINAL && !DETWS_ENABLE_WEBSOCKET
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WEB_TERMINAL requires DETWS_ENABLE_WEBSOCKET"
#endif

#if DETWS_ENABLE_DASHBOARD && !DETWS_ENABLE_SSE
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_DASHBOARD requires DETWS_ENABLE_SSE"
#endif

#if DETWS_ENABLE_SNMP_V3 && !DETWS_ENABLE_SNMP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_SNMP_V3 requires DETWS_ENABLE_SNMP"
#endif

#if DETWS_ENABLE_OPCUA_CLIENT && !DETWS_ENABLE_OPCUA
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_OPCUA_CLIENT requires DETWS_ENABLE_OPCUA (the shared OPC UA codec)"
#endif

#if DETWS_ENABLE_CONFIG_IO && !DETWS_ENABLE_CONFIG_STORE
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_CONFIG_IO requires DETWS_ENABLE_CONFIG_STORE"
#endif

#if DETWS_ENABLE_HTTP_CLIENT_TLS && !DETWS_ENABLE_HTTP_CLIENT
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_HTTP_CLIENT_TLS requires DETWS_ENABLE_HTTP_CLIENT"
#endif

#if DETWS_ENABLE_MQTT_TLS && !DETWS_ENABLE_MQTT
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_MQTT_TLS requires DETWS_ENABLE_MQTT"
#endif

#if DETWS_ENABLE_WS_CLIENT_TLS && !DETWS_ENABLE_WS_CLIENT
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WS_CLIENT_TLS requires DETWS_ENABLE_WS_CLIENT"
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_FRAME_SIZE < 2
#error "DeterministicESPAsyncWebServer: WS_FRAME_SIZE must be >= 2 when DETWS_ENABLE_WEBSOCKET is set"
#endif

#if DETWS_ENABLE_SSE && SSE_BUF_SIZE < 8
#error "DeterministicESPAsyncWebServer: SSE_BUF_SIZE must be >= 8 when DETWS_ENABLE_SSE is set"
#endif

#if DETWS_ENABLE_MULTIPART && MAX_MULTIPART_PARTS < 1
#error "DeterministicESPAsyncWebServer: MAX_MULTIPART_PARTS must be >= 1 when DETWS_ENABLE_MULTIPART is set"
#endif

#if RESP_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= 128 (status line + headers + CORS block)"
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: WS_HDR_BUF_SIZE must be >= 128 when DETWS_ENABLE_WEBSOCKET is set"
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
// value. An optional feature's group is wrapped in that feature's DETWS_ENABLE_* flag
// (resolved above), so a knob only exists when its feature is built.
//
// What is NOT here: protocol- and algorithm-fixed constants (wire opcodes, magic bytes,
// crypto digest / block sizes, spec-mandated PDU / field widths, the deflate/inflate
// scratch sizes a static_assert pins to the table layout). Those are not knobs - changing
// them breaks conformance - so they stay in their feature file next to the code they bind.

// -- Core: protocol dispatch + shared outbound transport (always built) --
/** @brief Largest ConnProto id the protocol-handler dispatch table holds. */
#ifndef DETWS_PROTO_MAX
#define DETWS_PROTO_MAX 8
#endif
/** @brief Number of simultaneous outbound client connections (BSS pool size). */
#ifndef DETWS_CLIENT_CONNS
#define DETWS_CLIENT_CONNS 2
#endif
/**
 * @brief Per-connection wire receive ring size (bytes).
 *
 * Holds plaintext (plain) or ciphertext (TLS). The transport ACKs on consume
 * (det_client_read reopens the window), so for a large inbound transfer to never
 * stall the ring must hold a full TCP receive window: keep DETWS_CLIENT_RX_BUF >=
 * TCP_WND (~5.7 KB). The 8192 default clears that and a multi-KB TLS handshake
 * flight; a ring below TCP_WND can deadlock a sustained download (the peer would be
 * allowed to send more than the ring holds). Must exceed one TCP segment (TCP_MSS).
 */
#ifndef DETWS_CLIENT_RX_BUF
#define DETWS_CLIENT_RX_BUF 8192
#endif

// -- SSH (network_drivers/presentation/ssh; the codec compiles when the SSH sources are
//    built, so its knobs are always defined) --
/** @brief Initial receive window the SSH server advertises (RFC 4254 §5.1). */
#ifndef SSH_CHAN_WINDOW
#define SSH_CHAN_WINDOW 32768u
#endif
/** @brief Maximum SSH channel data payload the server accepts per message. */
#ifndef SSH_CHAN_MAX_PACKET
#define SSH_CHAN_MAX_PACKET 32768u
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
 * only). Measured with the pluggable clock (detws_millis()).
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

#if DETWS_ENABLE_AUDIT_LOG
// -- Audit log (services/audit_log) --
#ifndef DETWS_AUDIT_LOG_ENTRIES
#define DETWS_AUDIT_LOG_ENTRIES 32 ///< RAM ring depth (records retained for query/verify).
#endif
#ifndef DETWS_AUDIT_MSG_LEN
#define DETWS_AUDIT_MSG_LEN 48 ///< Max message bytes per record (truncated to fit).
#endif
#endif // DETWS_ENABLE_AUDIT_LOG

#if DETWS_ENABLE_DEVICENET
// -- DeviceNet (services/devicenet) --
#ifndef DETWS_DEVICENET_MSG_MAX
#define DETWS_DEVICENET_MSG_MAX 256 ///< max reassembled fragmented message
#endif
#endif // DETWS_ENABLE_DEVICENET

#if DETWS_ENABLE_ESPNOW
// -- ESP-NOW (services/espnow) --
#ifndef DETWS_ESPNOW_MAX_PEERS
#define DETWS_ESPNOW_MAX_PEERS 8 ///< Bounded peer registry size.
#endif
#endif // DETWS_ENABLE_ESPNOW

#if DETWS_ENABLE_GRAPHQL
// -- GraphQL (services/graphql) --
#ifndef DETWS_GQL_MAX_NODES
#define DETWS_GQL_MAX_NODES 48 ///< Max fields across the whole query.
#endif
#ifndef DETWS_GQL_MAX_ARGS
#define DETWS_GQL_MAX_ARGS 24 ///< Max arguments across the whole query.
#endif
#ifndef DETWS_GQL_MAX_DEPTH
#define DETWS_GQL_MAX_DEPTH 6 ///< Max selection-set nesting depth.
#endif
#ifndef DETWS_GQL_NAME_MAX
#define DETWS_GQL_NAME_MAX 32 ///< Max field / argument name length.
#endif
#ifndef DETWS_GQL_PATH_MAX
#define DETWS_GQL_PATH_MAX 96 ///< Max dotted path length passed to the resolver.
#endif
#ifndef DETWS_GQL_STRBUF
#define DETWS_GQL_STRBUF 256 ///< Pool for decoded string-argument bytes.
#endif
#endif // DETWS_ENABLE_GRAPHQL

#if DETWS_ENABLE_J1939
// -- J1939 (services/j1939; also built when NMEA 2000 is enabled) --
#ifndef DETWS_J1939_TP_MAX
#define DETWS_J1939_TP_MAX 256 ///< max reassembled TP message (spec allows up to 1785); sized down for RAM
#endif
#endif // DETWS_ENABLE_J1939

#if DETWS_ENABLE_NMEA0183
// -- NMEA 0183 (services/nmea0183) --
#ifndef DETWS_NMEA0183_MAX_FIELDS
#define DETWS_NMEA0183_MAX_FIELDS 26 ///< max comma-separated fields (incl. the address field)
#endif
#endif // DETWS_ENABLE_NMEA0183

#if DETWS_ENABLE_NMEA2000
// -- NMEA 2000 (services/nmea2000) --
#ifndef DETWS_N2K_FP_MAX
#define DETWS_N2K_FP_MAX 223 ///< Fast Packet max payload (6 in frame 0 + 31 x 7)
#endif
#endif // DETWS_ENABLE_NMEA2000

#if DETWS_ENABLE_OAUTH2
// -- OAuth2 (services/oauth2) --
#ifndef DETWS_OAUTH2_TOKEN_LEN
#define DETWS_OAUTH2_TOKEN_LEN 768 ///< access_token / id_token buffer (JWTs are large).
#endif
#ifndef DETWS_OAUTH2_RT_LEN
#define DETWS_OAUTH2_RT_LEN 256 ///< refresh_token buffer.
#endif
#ifndef DETWS_OAUTH2_BODY_BUF
#define DETWS_OAUTH2_BODY_BUF 1024 ///< token-request body buffer.
#endif
#ifndef DETWS_OAUTH2_RESP_BUF
#define DETWS_OAUTH2_RESP_BUF 2048 ///< token-endpoint response buffer.
#endif
#endif // DETWS_ENABLE_OAUTH2

#if DETWS_ENABLE_OIDC
// -- OIDC (services/oidc) --
#ifndef DETWS_OIDC_MAX_LEN
#define DETWS_OIDC_MAX_LEN 1600 ///< Max accepted ID-token length.
#endif
#ifndef DETWS_OIDC_SUB_LEN
#define DETWS_OIDC_SUB_LEN 64 ///< Captured `sub` claim buffer.
#endif
#ifndef DETWS_OIDC_EMAIL_LEN
#define DETWS_OIDC_EMAIL_LEN 96 ///< Captured `email` claim buffer.
#endif
#ifndef DETWS_OIDC_KID_LEN
#define DETWS_OIDC_KID_LEN 80 ///< Max `kid` length.
#endif
#endif // DETWS_ENABLE_OIDC

#if DETWS_ENABLE_OPCUA
// -- OPC UA (services/opcua) --
#ifndef DETWS_OPCUA_BUF
#define DETWS_OPCUA_BUF 8192 ///< Server's advertised buffer / max-message size for the handshake.
#endif
#ifndef DETWS_OPCUA_READ_MAX
#define DETWS_OPCUA_READ_MAX 8 ///< max NodesToRead handled per ReadRequest.
#endif
#ifndef DETWS_OPCUA_BROWSE_MAX
#define DETWS_OPCUA_BROWSE_MAX 4 ///< max NodesToBrowse handled per BrowseRequest.
#endif
#ifndef DETWS_OPCUA_REF_MAX
#define DETWS_OPCUA_REF_MAX 8 ///< max references returned per browsed node.
#endif
#ifndef DETWS_OPCUA_WRITE_MAX
#define DETWS_OPCUA_WRITE_MAX 8 ///< max NodesToWrite handled per WriteRequest.
#endif
// Advertised server identity (endpoint descriptions), overridable per deployment; the app may
// also set these at runtime via opcua_set_endpoint_url() / the OpcUaServerInfo it passes.
#ifndef DETWS_OPCUA_DEFAULT_ENDPOINT
#define DETWS_OPCUA_DEFAULT_ENDPOINT "opc.tcp://localhost:4840" ///< default endpoint URL.
#endif
#ifndef DETWS_OPCUA_DEFAULT_APP_URI
#define DETWS_OPCUA_DEFAULT_APP_URI "urn:det:opcua:server" ///< default ApplicationUri.
#endif
#ifndef DETWS_OPCUA_DEFAULT_APP_NAME
#define DETWS_OPCUA_DEFAULT_APP_NAME "DetOpcUaServer" ///< default ApplicationName.
#endif
#endif // DETWS_ENABLE_OPCUA

#if DETWS_ENABLE_PROVISIONING
// -- Wi-Fi provisioning credential store (services/provisioning_service) --
// The NVS namespace and its keys, overridable per deployment (e.g. to avoid an NVS-namespace
// collision with the application's own store).
#ifndef DETWS_PROV_NVS_NAMESPACE
#define DETWS_PROV_NVS_NAMESPACE "wifi_prov" ///< NVS namespace holding the saved credentials.
#endif
#ifndef DETWS_PROV_KEY_SSID
#define DETWS_PROV_KEY_SSID "ssid" ///< NVS key + HTML form field for the SSID.
#endif
#ifndef DETWS_PROV_KEY_PSK
#define DETWS_PROV_KEY_PSK "psk" ///< NVS key + HTML form field for the pre-shared key.
#endif
#endif // DETWS_ENABLE_PROVISIONING

#if DETWS_ENABLE_VFS
// -- Virtual filesystem (services/vfs) --
#ifndef DETWS_VFS_RAM_FILES
#define DETWS_VFS_RAM_FILES 4 ///< RAM backend: number of files.
#endif
#ifndef DETWS_VFS_RAM_FILE_SIZE
#define DETWS_VFS_RAM_FILE_SIZE 1024 ///< RAM backend: max bytes per file.
#endif
#ifndef DETWS_VFS_MAX_OPEN
#define DETWS_VFS_MAX_OPEN 4 ///< Concurrent open handles (per backend).
#endif
#ifndef DETWS_VFS_NAME_MAX
#define DETWS_VFS_NAME_MAX 48 ///< Max path length (RAM backend).
#endif
#endif // DETWS_ENABLE_VFS

#endif
