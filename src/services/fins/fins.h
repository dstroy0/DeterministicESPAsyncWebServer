// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file fins.h
 * @brief Omron FINS frame codec (DETWS_ENABLE_FINS) - zero-heap command/response builder +
 *        parser for the Factory Interface Network Service (FINS/UDP), so a device can talk
 *        to an Omron PLC over the shipped UDP transport.
 *
 * A FINS message is a 10-octet header then the command code and data:
 * @code
 *   ICF RSV GCT  DNA DA1 DA2  SNA SA1 SA2  SID   MRC SRC  [params / data...]
 * @endcode
 *  - ICF: bit 6 = command(0)/response(1), bit 0 = response required(0)/not(1), bit 7 = use
 *    gateway. RSV = 0, GCT = 0x02. DNA/DA1/DA2 = destination net/node/unit; SNA/SA1/SA2 =
 *    source; SID = service id (echoed in the response).
 *  - MRC/SRC are the main/sub command code. A response inserts a 2-octet end code
 *    (MRES/SRES) before its data; MRES = SRES = 0 means normal completion.
 *  - Multi-octet command parameters (addresses, counts) are big-endian.
 *
 * FINS/UDP carries this frame directly (UDP provides integrity, so there is no checksum);
 * FINS/TCP would prepend its own header. This is the message codec; the send is the app's.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_FINS_H
#define DETERMINISTICESPASYNCWEBSERVER_FINS_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_FINS

#include <stddef.h>
#include <stdint.h>

#define FINS_HEADER_SIZE 10

#define FINS_ICF_COMMAND 0x80     ///< command, response required, gateway
#define FINS_ICF_RESPONSE 0xC0    ///< response
#define FINS_ICF_NO_RESPONSE 0x01 ///< OR into ICF: response not required

// Common command codes (MRC, SRC).
#define FINS_MRC_MEMORY_AREA 0x01
#define FINS_SRC_MEMORY_AREA_READ 0x01
#define FINS_SRC_MEMORY_AREA_WRITE 0x02

/** @brief The 10-octet FINS routing header. */
struct FinsHeader
{
    uint8_t icf, rsv, gct;
    uint8_t dna, da1, da2; ///< destination network / node / unit
    uint8_t sna, sa1, sa2; ///< source network / node / unit
    uint8_t sid;           ///< service id
};

/** @brief Build a command frame: header + MRC + SRC + params. Returns total octets, or 0. */
size_t fins_build_command(uint8_t *buf, size_t cap, const FinsHeader *h, uint8_t mrc, uint8_t src,
                          const uint8_t *params, size_t params_len);

/**
 * @brief Build a Memory Area Read command (0101): area code, 2-octet word address + bit,
 *        2-octet item count. The number of items is big-endian.
 */
size_t fins_build_memory_area_read(uint8_t *buf, size_t cap, const FinsHeader *h, uint8_t area, uint16_t address,
                                   uint8_t bit, uint16_t count);

/** @brief A parsed command (request side). @ref params points INTO the source buffer. */
struct FinsCommand
{
    FinsHeader header;
    uint8_t mrc, src;
    const uint8_t *params;
    size_t params_len;
};

/** @brief Parse a command frame (header + MRC + SRC + params). */
bool fins_parse_command(const uint8_t *buf, size_t len, FinsCommand *out);

/** @brief A parsed response. @ref data points INTO the source buffer. */
struct FinsResponse
{
    FinsHeader header;
    uint8_t mrc, src;   ///< echoed command code
    uint8_t mres, sres; ///< end code (0/0 = normal completion)
    const uint8_t *data;
    size_t data_len;
};

/** @brief Parse a response frame (header + MRC + SRC + MRES + SRES + data). */
bool fins_parse_response(const uint8_t *buf, size_t len, FinsResponse *out);

#endif // DETWS_ENABLE_FINS

#endif // DETERMINISTICESPASYNCWEBSERVER_FINS_H
