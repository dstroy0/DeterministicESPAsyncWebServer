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
#define N2K_PGN_COG_SOG_RAPID 129026u  ///< COG & SOG, Rapid Update: course + speed over ground
#define N2K_PGN_ENGINE_RAPID 127488u   ///< Engine Parameters, Rapid Update: speed + boost + tilt/trim
#define N2K_PGN_WIND_DATA 130306u      ///< Wind Data: speed + angle + reference
#define N2K_PGN_SPEED 128259u          ///< Speed: water-referenced + ground-referenced speed
#define N2K_PGN_WATER_DEPTH 128267u    ///< Water Depth: depth below transducer + offset
#define N2K_PGN_VESSEL_HEADING 127250u ///< Vessel Heading: heading + deviation + variation
#define N2K_PGN_ATTITUDE 127257u       ///< Attitude: yaw + pitch + roll
#define N2K_PGN_TEMPERATURE 130312u    ///< Temperature: instance + source + actual / set temperature

// Temperature source (PGN 130312 byte 2).
#define N2K_TEMP_SRC_SEA 0
#define N2K_TEMP_SRC_OUTSIDE 1
#define N2K_TEMP_SRC_INSIDE 2
#define N2K_TEMP_SRC_ENGINE_ROOM 3
#define N2K_TEMP_SRC_MAIN_CABIN 4
#define N2K_TEMP_SRC_LIVE_WELL 5
#define N2K_TEMP_SRC_BAIT_WELL 6
#define N2K_TEMP_SRC_REFRIGERATION 7
#define N2K_TEMP_SRC_HEATING_SYSTEM 8
#define N2K_TEMP_SRC_FREEZER 13
#define N2K_TEMP_SRC_EXHAUST_GAS 14

// COG reference (PGN 129026 byte 1, low 2 bits).
#define N2K_COG_REF_TRUE 0     ///< course over ground referenced to True North
#define N2K_COG_REF_MAGNETIC 1 ///< course over ground referenced to Magnetic North
#define N2K_COG_REF_ERROR 2    ///< reference in error
#define N2K_COG_REF_NULL 3     ///< reference not available / null

// Heading reference (PGN 127250 byte 7, low 2 bits).
#define N2K_HEADING_REF_TRUE 0     ///< true heading
#define N2K_HEADING_REF_MAGNETIC 1 ///< magnetic heading

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

/** @brief Decoded COG & SOG Rapid Update (PGN 129026). */
struct N2kCogSogRapid
{
    uint8_t sid;     ///< sequence id
    uint8_t cog_ref; ///< course reference (@ref N2K_COG_REF_TRUE / _MAGNETIC)
    bool cog_valid;  ///< false if the course over ground is not-available
    float cog_rad;   ///< course over ground (radians, 0.0001 rad per bit)
    bool sog_valid;  ///< false if the speed over ground is not-available
    float sog_mps;   ///< speed over ground (m/s, 0.01 m/s per bit)
};

/** @brief Decoded Engine Parameters, Rapid Update (PGN 127488). */
struct N2kEngineRapid
{
    uint8_t instance; ///< engine instance (0 = single / port, 1 = starboard, ...)
    bool speed_valid; ///< false if the engine speed is not-available
    float speed_rpm;  ///< engine speed (rpm, 0.25 rpm per bit)
    bool boost_valid; ///< false if the boost pressure is not-available
    float boost_pa;   ///< engine boost pressure (Pa, 100 Pa per bit)
    bool tilt_valid;  ///< false if the tilt/trim is not-available
    int8_t tilt_pct;  ///< engine tilt / trim (percent, 1 %/bit, signed)
};

/** @brief Decoded Attitude (PGN 127257): the vessel's orientation. Each angle is signed at 0.0001 rad/bit. */
struct N2kAttitude
{
    uint8_t sid;      ///< sequence id
    bool yaw_valid;   ///< false if yaw is not-available
    float yaw_rad;    ///< yaw (radians); + = bow rotating to starboard
    bool pitch_valid; ///< false if pitch is not-available
    float pitch_rad;  ///< pitch (radians); + = bow up
    bool roll_valid;  ///< false if roll is not-available
    float roll_rad;   ///< roll (radians); + = starboard side down
};

/** @brief Decoded Temperature (PGN 130312). Temperatures are carried in Kelvin (0.01 K/bit) on the wire and
 *  exposed here in Celsius. */
struct N2kTemperature
{
    uint8_t sid;       ///< sequence id
    uint8_t instance;  ///< temperature instance
    uint8_t source;    ///< temperature source (@ref N2K_TEMP_SRC_SEA etc.)
    bool actual_valid; ///< false if the actual temperature is not-available
    float actual_c;    ///< actual temperature (degrees Celsius)
    bool set_valid;    ///< false if the set/target temperature is not-available
    float set_c;       ///< set / target temperature (degrees Celsius)
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

// Speed water-referenced sensor type (PGN 128259 byte 5).
#define N2K_SPEED_TYPE_PADDLE_WHEEL 0
#define N2K_SPEED_TYPE_PITOT_TUBE 1
#define N2K_SPEED_TYPE_DOPPLER 2
#define N2K_SPEED_TYPE_CORRELATION 3     ///< correlation / ultrasound
#define N2K_SPEED_TYPE_ELECTROMAGNETIC 4 ///< electromagnetic

/** @brief Decoded Speed (PGN 128259): through-water and over-ground speed. */
struct N2kSpeed
{
    uint8_t sid;            ///< sequence id
    bool water_valid;       ///< false if the water-referenced speed is not-available
    float water_mps;        ///< speed through water (m/s, 0.01 m/s per bit)
    bool ground_valid;      ///< false if the ground-referenced speed is not-available
    float ground_mps;       ///< speed over ground (m/s, 0.01 m/s per bit)
    uint8_t water_ref_type; ///< water-speed sensor type (@ref N2K_SPEED_TYPE_PADDLE_WHEEL etc.)
};

/** @brief Decoded Water Depth (PGN 128267). */
struct N2kWaterDepth
{
    uint8_t sid;      ///< sequence id
    bool depth_valid; ///< false if the depth is not-available
    float depth_m;    ///< water depth below the transducer (metres, 0.01 m per bit)
    float offset_m;   ///< transducer offset (m); positive = distance to waterline, negative = to keel
};

/** @brief Decoded Vessel Heading (PGN 127250). */
struct N2kVesselHeading
{
    uint8_t sid;         ///< sequence id
    bool heading_valid;  ///< false if the heading is not-available
    float heading_rad;   ///< heading (radians, 0.0001 rad per bit)
    float deviation_rad; ///< magnetic deviation (radians)
    float variation_rad; ///< magnetic variation (radians)
    uint8_t reference;   ///< heading reference (@ref N2K_HEADING_REF_TRUE / _MAGNETIC)
};

/**
 * @brief Decode a Position Rapid Update (PGN 129025) payload into @p out.
 * @return true iff @p len is at least 8 octets; false otherwise.
 */
bool dws_n2k_decode_position_rapid(const uint8_t *payload, size_t len, N2kPositionRapid *out);

/**
 * @brief Decode a COG & SOG Rapid Update (PGN 129026) payload into @p out.
 * @return true iff @p len is at least 6 octets (SID + reference + COG + SOG); false otherwise.
 */
bool dws_n2k_decode_cog_sog_rapid(const uint8_t *payload, size_t len, N2kCogSogRapid *out);

/**
 * @brief Decode an Engine Parameters Rapid Update (PGN 127488) payload into @p out.
 * @return true iff @p len is at least 6 octets (instance + speed + boost + tilt); false otherwise.
 */
bool dws_n2k_decode_engine_rapid(const uint8_t *payload, size_t len, N2kEngineRapid *out);

/**
 * @brief Decode an Attitude (PGN 127257) payload into @p out.
 * @return true iff @p len is at least 7 octets (SID + yaw + pitch + roll); false otherwise.
 */
bool dws_n2k_decode_attitude(const uint8_t *payload, size_t len, N2kAttitude *out);

/**
 * @brief Decode a Temperature (PGN 130312) payload into @p out.
 * @return true iff @p len is at least 7 octets (SID + instance + source + actual + set); false otherwise.
 */
bool dws_n2k_decode_temperature(const uint8_t *payload, size_t len, N2kTemperature *out);

/**
 * @brief Decode a Wind Data (PGN 130306) payload into @p out.
 * @return true iff @p len is at least 6 octets; false otherwise.
 */
bool dws_n2k_decode_wind_data(const uint8_t *payload, size_t len, N2kWindData *out);

/**
 * @brief Decode a Speed (PGN 128259) payload into @p out.
 * @return true iff @p len is at least 6 octets (SID + water speed + ground speed + type); false otherwise.
 */
bool dws_n2k_decode_speed(const uint8_t *payload, size_t len, N2kSpeed *out);

/**
 * @brief Decode a Water Depth (PGN 128267) payload into @p out.
 * @return true iff @p len is at least 7 octets (SID + depth + offset); false otherwise.
 */
bool dws_n2k_decode_water_depth(const uint8_t *payload, size_t len, N2kWaterDepth *out);

/**
 * @brief Decode a Vessel Heading (PGN 127250) payload into @p out.
 * @return true iff @p len is at least 8 octets; false otherwise.
 */
bool dws_n2k_decode_vessel_heading(const uint8_t *payload, size_t len, N2kVesselHeading *out);

#endif // DWS_ENABLE_NMEA2000
#endif // DETERMINISTICESPASYNCWEBSERVER_NMEA2000_H
