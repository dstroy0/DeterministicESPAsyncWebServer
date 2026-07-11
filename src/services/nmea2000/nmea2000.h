// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea2000.h
 * @brief NMEA 2000 codec (DETWS_ENABLE_NMEA2000) - the marine instrumentation network, built on
 *        J1939 over CAN.
 *
 * NMEA 2000 is J1939 at the transport layer (the same 29-bit priority / PGN / source /
 * destination identifier), so this codec reuses the J1939 id encode / decode
 * (`DETWS_ENABLE_NMEA2000` force-enables `DETWS_ENABLE_J1939`). What it adds is the
 * NMEA-specific **Fast Packet** transport: messages of 9..223 octets are split across CAN
 * frames using a per-frame control octet (sequence counter + frame counter) instead of the
 * J1939 BAM/CMDT protocol. The first frame carries the total length; continuations carry 7
 * data octets each.
 *
 * Pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to
 * bridge an NMEA 2000 backbone (GPS, wind, depth, engine PGNs) onto Wi-Fi.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_NMEA2000_H
#define DETERMINISTICESPASYNCWEBSERVER_NMEA2000_H

#include "ServerConfig.h"

#if DETWS_ENABLE_NMEA2000

#include "services/j1939/j1939.h" // reuses the J1939 29-bit identifier codec
#include "shared_primitives/can.h"
#include <stddef.h>
#include <stdint.h>

#define N2K_FP_SEQ_SHIFT 5      ///< control octet: sequence counter in bits 7..5
#define N2K_FP_FRAME_MASK 0x1Fu ///< control octet: frame counter in bits 4..0
#define N2K_FP_F0_DATA 6u       ///< data octets in the first frame (after control + length octets)
#define N2K_FP_FN_DATA 7u       ///< data octets in a continuation frame

/** @brief Result of feeding a frame to the Fast Packet reassembler. */
enum class N2kFpResult : uint8_t
{
    N2K_FP_IGNORED = 0, ///< not part of the active sequence
    N2K_FP_STARTED,     ///< first frame opened a sequence
    N2K_FP_PROGRESS,    ///< a continuation frame was accepted
    N2K_FP_COMPLETE,    ///< the message is fully reassembled
    N2K_FP_ERR,         ///< out-of-order / too large
};

/** @brief Fast Packet reassembly context (one in-flight message). */
struct N2kFastPacketRx
{
    bool active;
    uint8_t seq;        ///< sequence counter of the in-progress message
    uint8_t sa;         ///< source address
    uint32_t pgn;       ///< the message PGN
    uint16_t total_len; ///< announced total length
    uint16_t received;  ///< octets stored so far
    uint8_t next_frame; ///< next expected frame counter
    uint8_t buf[DETWS_N2K_FP_MAX];
};

/** @brief Number of Fast Packet frames needed for @p total_len octets. */
uint8_t n2k_fastpacket_num_frames(uint16_t total_len);

/**
 * @brief Build Fast Packet frame @p frame_idx (0-based) of a message.
 * @p seq is the 0..7 sequence counter for this message; @p total_len is the whole payload.
 */
bool n2k_fastpacket_build_frame(CanFrame *out, uint8_t seq, uint8_t frame_idx, uint8_t priority, uint32_t pgn,
                                uint8_t sa, uint8_t da, const uint8_t *data, uint16_t total_len);

/** @brief Reset a Fast Packet reassembly context to idle. */
void n2k_fastpacket_reset(N2kFastPacketRx *rx);

/** @brief Feed a received frame to the Fast Packet reassembler; see @ref N2kFpResult. */
N2kFpResult n2k_fastpacket_feed(N2kFastPacketRx *rx, const CanFrame *f);

/** @brief Build a single-frame (<= 8 octet) NMEA 2000 message (a thin wrap of J1939). */
bool n2k_build_single(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                      uint8_t len);

#endif // DETWS_ENABLE_NMEA2000
#endif // DETERMINISTICESPASYNCWEBSERVER_NMEA2000_H
