// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dnp3.h
 * @brief DNP3 (IEEE 1815) data-link frame codec (DWS_ENABLE_DNP3) - zero-heap builder +
 *        CRC-validating parser for the SCADA / utility outstation link layer.
 *
 * A DNP3 data-link frame:
 * @code
 *   0x05 0x64  LEN  CTRL  DEST(2,LE)  SRC(2,LE)  CRC(2)      // 10-octet header block
 *   [<=16 user-data octets][CRC(2)] ...                      // data blocks, each CRC'd
 * @endcode
 *  - LEN counts CTRL + DEST + SRC + user data (the start word, LEN itself, and the CRCs are
 *    excluded), so LEN = 5 + user_data_len; min 5, max 255 (<= 250 user-data octets).
 *  - Addresses are little-endian (LSB first). User data is carried in blocks of up to 16
 *    octets, each followed by its own CRC; the header is its own CRC'd block.
 *  - CRC is CRC-16/DNP (poly 0x3D65, init 0x0000, reflected in/out, final XOR 0xFFFF),
 *    transmitted low octet first.
 *
 * This is the data-link layer (framing + CRC). The transport-function reassembly and the
 * application layer (objects / function codes) are layered on the de-blocked user data.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DNP3_H
#define DETERMINISTICESPASYNCWEBSERVER_DNP3_H

#include "ServerConfig.h"

#if DWS_ENABLE_DNP3

#include <stddef.h>
#include <stdint.h>

#define DNP3_START0 0x05
#define DNP3_START1 0x64
#define DNP3_MAX_USER_DATA 250 ///< LEN max 255 minus the 5 header octets it counts

// Frame geometry (octets). The header is one CRC'd block; user data is carried in
// fixed-size CRC'd blocks. These are wire constants from IEEE 1815, not tunables.
#define DNP3_BLOCK_LEN 16        ///< user-data octets per data block (the last may be shorter)
#define DNP3_CRC_LEN 2           ///< CRC-16/DNP appended after each block, low octet first
#define DNP3_HEADER_LEN 8        ///< header octets the header CRC covers: 0x0564 LEN CTRL DEST SRC
#define DNP3_HEADER_BLOCK_LEN 10 ///< whole header block = DNP3_HEADER_LEN + DNP3_CRC_LEN
#define DNP3_LEN_OVERHEAD 5      ///< octets LEN counts beyond user data: CTRL + DEST + SRC

/** @brief CRC-16/DNP (poly 0x3D65, init 0, reflected, final XOR 0xFFFF). */
uint16_t dnp3_crc(const uint8_t *data, size_t len);

/**
 * @brief Build a complete data-link frame (header block + CRC'd data blocks).
 * @param control link-layer control octet (DIR/PRM/FCB/FCV + function code).
 * @param dest    16-bit destination address (written little-endian).
 * @param src     16-bit source address (written little-endian).
 * @return total octets written, or 0 on overflow / user_data_len > DNP3_MAX_USER_DATA.
 */
size_t dnp3_build_frame(uint8_t *buf, size_t cap, uint8_t control, uint16_t dest, uint16_t src,
                        const uint8_t *user_data, size_t user_data_len);

/** @brief A parsed data-link frame header (the user data is de-blocked separately). */
struct Dnp3Frame
{
    uint8_t length;  ///< the LEN field value
    uint8_t control; ///< link-layer control octet
    uint16_t dest;
    uint16_t src;
};

/**
 * @brief Parse + CRC-validate a frame, de-blocking the user data (per-block CRCs stripped).
 * @param out_user     receives the reassembled user data.
 * @param out_cap      capacity of @p out_user.
 * @param out_user_len receives the user-data length.
 * @return true on a complete, all-CRC-valid frame; false on a bad start word, an invalid
 *         LEN, truncation, a header or block CRC mismatch, or an out_user overflow.
 */
bool dnp3_parse_frame(const uint8_t *buf, size_t len, Dnp3Frame *out, uint8_t *out_user, size_t out_cap,
                      size_t *out_user_len);

#endif // DWS_ENABLE_DNP3

#endif // DETERMINISTICESPASYNCWEBSERVER_DNP3_H
