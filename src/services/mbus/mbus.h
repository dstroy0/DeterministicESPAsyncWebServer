// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mbus.h
 * @brief Wired M-Bus (Meter-Bus, EN 13757-2/-3) frame codec (DWS_ENABLE_MBUS).
 *
 * A pure, zero-heap builder + parser for the M-Bus link-layer frames used by utility meters
 * (water / gas / heat / electricity), plus a walker for the EN 13757-3 variable-data records
 * (DIF / VIF). M-Bus has three frame formats:
 * @code
 *   single char : E5                                                  (ACK)
 *   short frame : 10 C A CS 16
 *   long frame  : 68 L L 68 C A CI [user data] CS 16   (L = 3 + data; CS = sum(C..data) mod 256)
 * @endcode
 * The control frame is just a long frame with no user data (L = 3). The checksum is the
 * 8-bit sum of every octet from C through the end of the user data.
 *
 * The wired bus is a powered two-wire pair: the ESP32 talks to it over a UART through an
 * M-Bus level converter (e.g. a TSS721-based master module). This codec is the framing +
 * record layer; the UART transport is the application's. Bridge meters onto Wi-Fi by polling
 * REQ_UD2 and publishing the decoded records.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MBUS_H
#define DETERMINISTICESPASYNCWEBSERVER_MBUS_H

#include "ServerConfig.h"

#if DWS_ENABLE_MBUS

#include <stddef.h>
#include <stdint.h>

#define MBUS_START_SHORT 0x10u ///< short-frame start octet
#define MBUS_START_LONG 0x68u  ///< long / control-frame start octet
#define MBUS_STOP 0x16u        ///< frame stop octet
#define MBUS_ACK 0xE5u         ///< single-character acknowledge

// Common control-field (C) values.
#define MBUS_C_SND_NKE 0x40u ///< initialize slave (link reset)
#define MBUS_C_REQ_UD2 0x5Bu ///< request class-2 user data (FCB=0); 0x7B with FCB=1
#define MBUS_C_REQ_UD1 0x5Au ///< request class-1 user data
#define MBUS_C_SND_UD 0x53u  ///< send user data to slave (FCB=0); 0x73 with FCB=1
#define MBUS_C_RSP_UD 0x08u  ///< response with user data (+ ACD/DFC bits)

// Common control-information (CI) values.
#define MBUS_CI_DATA_SEND 0x51u    ///< data send (master -> slave)
#define MBUS_CI_SELECT 0x52u       ///< selection of slaves
#define MBUS_CI_RSP_VARIABLE 0x72u ///< variable data response, long header (LSB first)
#define MBUS_CI_RSP_FIXED 0x73u    ///< fixed data response

#define MBUS_MAX_DATA 252u ///< max user-data octets (L is one octet; 255 - 3)

/** @brief M-Bus frame kinds. */
enum class MbusFrameType : uint8_t
{
    MBUS_FRAME_NONE = 0,
    MBUS_FRAME_ACK,   ///< single 0xE5
    MBUS_FRAME_SHORT, ///< 10 C A CS 16
    MBUS_FRAME_LONG,  ///< 68 L L 68 C A CI ... CS 16 (control frame = long with no data)
};

/** @brief A parsed M-Bus frame (data points into the caller's buffer). */
struct MbusFrame
{
    MbusFrameType type;
    uint8_t c;           ///< control field (short / long)
    uint8_t a;           ///< address field (short / long)
    uint8_t ci;          ///< control-information field (long only)
    const uint8_t *data; ///< user data (long only), or nullptr
    uint8_t data_len;    ///< user-data length (long only)
};

// DIF data-field coding (low nibble of the DIF). The decoded fixed lengths are in octets.
enum class MbusDifCoding : uint8_t
{
    MBUS_DIF_NONE = 0x0,     ///< no data
    MBUS_DIF_INT8 = 0x1,     ///< 8-bit integer
    MBUS_DIF_INT16 = 0x2,    ///< 16-bit integer
    MBUS_DIF_INT24 = 0x3,    ///< 24-bit integer
    MBUS_DIF_INT32 = 0x4,    ///< 32-bit integer
    MBUS_DIF_REAL32 = 0x5,   ///< 32-bit IEEE-754 real
    MBUS_DIF_INT48 = 0x6,    ///< 48-bit integer
    MBUS_DIF_INT64 = 0x7,    ///< 64-bit integer
    MBUS_DIF_READOUT = 0x8,  ///< selection for readout (no data)
    MBUS_DIF_BCD2 = 0x9,     ///< 2-digit BCD (1 octet)
    MBUS_DIF_BCD4 = 0xA,     ///< 4-digit BCD (2 octets)
    MBUS_DIF_BCD6 = 0xB,     ///< 6-digit BCD (3 octets)
    MBUS_DIF_BCD8 = 0xC,     ///< 8-digit BCD (4 octets)
    MBUS_DIF_VARIABLE = 0xD, ///< variable length (LVAR octet precedes the data)
    MBUS_DIF_BCD12 = 0xE,    ///< 12-digit BCD (6 octets)
    MBUS_DIF_SPECIAL = 0xF,  ///< special functions (no data)
};

/** @brief One decoded EN 13757-3 data record. */
struct MbusRecord
{
    uint8_t dif;         ///< first data-information octet
    uint8_t coding;      ///< DIF low nibble (see MbusDifCoding)
    uint8_t vif;         ///< first value-information octet (0 if none)
    const uint8_t *data; ///< value octets (points into the caller buffer)
    uint8_t data_len;    ///< value length in octets
};

// --- builders: write into @p buf (cap), return frame length or 0 on overflow ---

/** @brief Single-character acknowledge (0xE5). */
size_t dws_mbus_build_ack(uint8_t *buf, size_t cap);

/** @brief Short frame: 10 C A CS 16. */
size_t dws_mbus_build_short(uint8_t *buf, size_t cap, uint8_t c, uint8_t a);

/** @brief Long frame: 68 L L 68 C A CI [data] CS 16. @p data_len 0 builds a control frame. */
size_t dws_mbus_build_long(uint8_t *buf, size_t cap, uint8_t c, uint8_t a, uint8_t ci, const uint8_t *data,
                           uint8_t data_len);

/** @brief Convenience: a SND_NKE (link reset) short frame to address @p a. */
size_t dws_mbus_build_snd_nke(uint8_t *buf, size_t cap, uint8_t a);

/** @brief Convenience: a REQ_UD2 short frame to address @p a (@p fcb toggles the FCB bit). */
size_t dws_mbus_build_req_ud2(uint8_t *buf, size_t cap, uint8_t a, bool fcb);

// --- parser ---

/**
 * @brief Parse one M-Bus frame from @p buf. Validates the start/stop octets, the doubled
 * length, and the checksum. On success fills @p out and sets @p consumed to the frame length.
 */
bool dws_mbus_parse(const uint8_t *buf, size_t len, MbusFrame *out, size_t *consumed);

// --- variable-data records (DIF / VIF) ---

/** @brief Map a DIF low-nibble coding to its fixed data length (0 for variable / none). */
uint8_t dws_mbus_dif_data_len(uint8_t coding);

/**
 * @brief Walk one data record at @p *pos within a long-frame body (the octets after CI).
 * Skips DIFE / VIFE extension chains, decodes the data length (incl. the LVAR variable form),
 * and advances @p *pos past the record. Returns false at the end of data or on overflow.
 */
bool dws_mbus_record_next(const uint8_t *body, size_t len, size_t *pos, MbusRecord *out);

#endif // DWS_ENABLE_MBUS
#endif // DETERMINISTICESPASYNCWEBSERVER_MBUS_H
