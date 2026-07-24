// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea2000.h
 * @brief NMEA 2000 codec (DWS_ENABLE_NMEA2000) - the marine instrumentation network, built on
 *        J1939 over CAN.
 *
 * NMEA 2000 is J1939 at the transport layer (the same 29-bit priority / PGN / source /
 * destination identifier), so this codec reuses the J1939 id encode / decode
 * (`DWS_ENABLE_NMEA2000` force-enables `DWS_ENABLE_J1939`). What it adds is the
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

#if DWS_ENABLE_NMEA2000

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
    uint8_t buf[DWS_N2K_FP_MAX];
};

/** @brief Number of Fast Packet frames needed for @p total_len octets. */
uint8_t dws_n2k_fastpacket_num_frames(uint16_t total_len);

/**
 * @brief Build Fast Packet frame @p frame_idx (0-based) of a message.
 * @p seq is the 0..7 sequence counter for this message; @p total_len is the whole payload.
 */
bool dws_n2k_fastpacket_build_frame(CanFrame *out, uint8_t seq, uint8_t frame_idx, uint8_t priority, uint32_t pgn,
                                    uint8_t sa, uint8_t da, const uint8_t *data, uint16_t total_len);

/** @brief Reset a Fast Packet reassembly context to idle. */
void dws_n2k_fastpacket_reset(N2kFastPacketRx *rx);

/** @brief Feed a received frame to the Fast Packet reassembler; see @ref N2kFpResult. */
N2kFpResult dws_n2k_fastpacket_feed(N2kFastPacketRx *rx, const CanFrame *f);

/** @brief Build a single-frame (<= 8 octet) NMEA 2000 message (a thin wrap of J1939). */
bool dws_n2k_build_single(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                          uint8_t len);

// --- typed decoders for common single-frame PGNs ---
//
// These decode a raw PGN payload (a single frame's data[] or a reassembled Fast Packet buffer) into
// engineering units. The caller matches the PGN off the CAN id first, then calls the matching decoder.
// NMEA 2000 marks a field "not available" with an all-ones raw (0xFFFF for a U2, 0x7FFFFFFF for the
// signed lat/lon), which clears the field's validity flag.

#define N2K_PGN_POSITION_RAPID 129025u ///< Position, Rapid Update: latitude + longitude
#define N2K_PGN_WIND_DATA 130306u      ///< Wind Data: speed + angle + reference

// Wind reference (PGN 130306 byte 5, low 3 bits).
#define N2K_WIND_REF_TRUE_NORTH 0 ///< true, referenced to North
#define N2K_WIND_REF_MAGNETIC 1   ///< magnetic, referenced to North
#define N2K_WIND_REF_APPARENT 2   ///< apparent
#define N2K_WIND_REF_TRUE_BOAT 3  ///< true, referenced to the vessel (boat)
#define N2K_WIND_REF_TRUE_WATER 4 ///< true, referenced to the water

/** @brief Decoded Position Rapid Update (PGN 129025). */
struct N2kPositionRapid
{
    bool valid;     ///< false if either coordinate is not-available
    double lat_deg; ///< latitude in decimal degrees (1e-7 deg/bit)
    double lon_deg; ///< longitude in decimal degrees
};

/** @brief Decoded Wind Data (PGN 130306). */
struct N2kWindData
{
    uint8_t sid;       ///< sequence id
    bool speed_valid;  ///< false if the wind speed is not-available
    float speed_mps;   ///< wind speed (m/s, 0.01 m/s per bit)
    bool angle_valid;  ///< false if the wind angle is not-available
    float angle_rad;   ///< wind angle (radians, 0.0001 rad per bit)
    uint8_t reference; ///< wind reference (@ref N2K_WIND_REF_TRUE_NORTH etc.)
};

/**
 * @brief Decode a Position Rapid Update (PGN 129025) payload into @p out.
 * @return true iff @p len is at least 8 octets; false otherwise.
 */
bool dws_n2k_decode_position_rapid(const uint8_t *payload, size_t len, N2kPositionRapid *out);

/**
 * @brief Decode a Wind Data (PGN 130306) payload into @p out.
 * @return true iff @p len is at least 6 octets; false otherwise.
 */
bool dws_n2k_decode_wind_data(const uint8_t *payload, size_t len, N2kWindData *out);

#endif // DWS_ENABLE_NMEA2000
#endif // DETERMINISTICESPASYNCWEBSERVER_NMEA2000_H
