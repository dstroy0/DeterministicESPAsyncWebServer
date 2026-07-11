// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ld2410.h
 * @brief HLK-LD2410 24 GHz mmWave presence / motion radar codec (DETWS_ENABLE_LD2410).
 *
 * The LD2410 streams a framed serial report at 256000 baud: header `F4 F3 F2 F1`, a
 * little-endian intra-frame length, the payload, and footer `F8 F7 F6 F5`. The payload carries
 * the target state (none / moving / stationary / both), the moving and stationary target
 * distance (cm) and energy (0-100), the overall detection distance, and - in "engineering
 * mode" - the per-gate energy of all nine range gates. Configuration is a second frame kind
 * (header `FD FC FB FA`, footer `04 03 02 01`) carrying a 2-byte command word.
 *
 * This codec is pure and host-tested: ::ld2410_parse_report decodes one report frame, and
 * ::Ld2410Stream reassembles frames byte-by-byte from a UART with resync on noise (no heap,
 * fixed buffer). The command encoders build the config frames. On an ESP32 the binding pumps a
 * `HardwareSerial` and keeps the latest report; only that read/write touches hardware.
 *
 * A cheap solder-and-bench-test breakout: wire it to a UART, wave a hand, watch presence.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_LD2410_H
#define DETERMINISTICESPASYNCWEBSERVER_LD2410_H

#include <stddef.h>
#include <stdint.h>

/** @brief Range gates the LD2410 reports energy for in engineering mode (gate 0..8). */
#define LD2410_MAX_GATES 9

/** @brief Largest assembled frame: header(4) + len(2) + payload(<=60) + footer(4), rounded up. */
#define LD2410_FRAME_MAX 72

/** @brief Target presence state (report payload byte 2). */
struct Ld2410State
{
    static constexpr uint8_t LD2410_STATE_NONE = 0x00;   ///< no target
    static constexpr uint8_t LD2410_STATE_MOVING = 0x01; ///< moving target only
    static constexpr uint8_t LD2410_STATE_STATIC = 0x02; ///< stationary target only
    static constexpr uint8_t LD2410_STATE_BOTH = 0x03;   ///< both a moving and a stationary target
};

/** @brief A decoded LD2410 target report. Engineering fields are 0 unless @ref engineering. */
struct Ld2410Report
{
    uint8_t engineering;   ///< 1 if this was an engineering-mode frame (per-gate energies valid)
    uint8_t state;         ///< one of LD2410_STATE_*
    uint16_t moving_cm;    ///< moving target distance (cm)
    uint8_t moving_energy; ///< moving target energy (0-100)
    uint16_t static_cm;    ///< stationary target distance (cm)
    uint8_t static_energy; ///< stationary target energy (0-100)
    uint16_t detect_cm;    ///< overall detection distance (cm)
    // Engineering mode only:
    uint8_t max_moving_gate;                      ///< highest configured moving gate
    uint8_t max_static_gate;                      ///< highest configured stationary gate
    uint8_t moving_gate_energy[LD2410_MAX_GATES]; ///< per-gate moving energy (0-100)
    uint8_t static_gate_energy[LD2410_MAX_GATES]; ///< per-gate stationary energy (0-100)
    uint8_t light;                                ///< photosensor level (0-255)
    uint8_t out_pin;                              ///< OUT pin level (0/1)
};

/**
 * @brief Decode one whole LD2410 report frame (header `F4 F3 F2 F1` .. footer `F8 F7 F6 F5`).
 * Pure - no I/O. Validates the header/footer, the intra-frame length, the data-type byte
 * (0x02 basic / 0x01 engineering), the `0xAA` head marker and the `0x55` tail.
 * @return true on a valid frame; false on any mismatch or a short buffer.
 */
bool ld2410_parse_report(const uint8_t *frame, size_t len, Ld2410Report *out);

/** @brief Byte-by-byte report-frame reassembler (fixed buffer, resyncs on noise). */
struct Ld2410Stream
{
    uint8_t buf[LD2410_FRAME_MAX]; ///< frame under construction
    uint16_t pos;                  ///< bytes collected so far
    uint16_t total;                ///< expected full-frame length (known after the length field)
    uint8_t hdr_match;             ///< header bytes matched while syncing (phase = pos<4)
    uint8_t phase;                 ///< 0 sync, 1 length, 2 body
};

/** @brief Reset a stream to the syncing state. */
void ld2410_stream_reset(Ld2410Stream *s);

/**
 * @brief Feed one received byte. When it completes a valid report frame, fills @p out and
 * returns true; otherwise returns false (still syncing / mid-frame / bad frame - it resyncs).
 */
bool ld2410_stream_push(Ld2410Stream *s, uint8_t byte, Ld2410Report *out);

/** @brief true if @p r shows any target (moving or stationary). */
bool ld2410_present(const Ld2410Report *r);

/** @brief Best available target distance (cm): the moving distance if moving, else stationary. */
uint16_t ld2410_distance_cm(const Ld2410Report *r);

// --- Config-command encoders (build a full `FD FC FB FA` .. `04 03 02 01` frame) -------------
// Each returns the frame length written, or 0 if @p cap is too small. Config commands must be
// bracketed by enable/end; engineering + restart take effect inside that bracket.

/** @brief "Enable configuration" (word 0x00FF, value 0x0001). */
size_t ld2410_cmd_config_enable(uint8_t *buf, size_t cap);
/** @brief "End configuration" (word 0x00FE). */
size_t ld2410_cmd_config_end(uint8_t *buf, size_t cap);
/** @brief Enable (0x0062) or disable (0x0063) engineering mode. */
size_t ld2410_cmd_engineering(uint8_t *buf, size_t cap, bool on);
/** @brief Restart the module (word 0x00A3). */
size_t ld2410_cmd_restart(uint8_t *buf, size_t cap);

// --- ESP32 binding (Serial pump; no-ops on a host build) -------------------------------------

/** @brief Open UART2 at DETWS_LD2410_BAUD on @p rx_pin / @p tx_pin. @return true on ESP32. */
bool ld2410_begin(int rx_pin, int tx_pin);

/** @brief Pump the UART through the stream. @return true if a fresh report was decoded. */
bool ld2410_poll();

/** @brief The most recently decoded report, or nullptr before the first one arrives. */
const Ld2410Report *ld2410_last();

/** @brief Enable/disable engineering mode (brackets the command with enable/end). */
bool ld2410_set_engineering(bool on);

/** @brief Restart the module (brackets the command with enable/end). */
bool ld2410_restart();

#endif // DETERMINISTICESPASYNCWEBSERVER_LD2410_H
