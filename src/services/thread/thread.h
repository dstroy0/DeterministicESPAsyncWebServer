// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file thread.h
 * @brief Thread spinel / HDLC-lite framing codec (DWS_ENABLE_THREAD) - OpenThread RCP.
 *
 * The HDLC-lite framing that carries spinel frames to an OpenThread radio co-processor (an
 * nRF52840 / EFR32 RCP) over UART - an 802.15.4 / Thread mesh bridged to IP and the web.
 * HDLC-lite wraps each spinel frame by appending an FCS, byte-stuffing the reserved bytes,
 * and terminating with a Flag:
 *
 *   [spinel payload | FCS(lo,hi)] --byte-stuffed--> ... | 0x7E
 *
 * The FCS is the HDLC frame check sequence, **CRC-16/X-25** (poly 0x1021 reflected, init
 * 0xFFFF, reflected in/out, final XOR 0xFFFF), transmitted low byte first. The reserved
 * bytes stuffed (as 0x7D, byte XOR 0x20) are the Flag 0x7E, the Escape 0x7D, XON 0x11, and
 * XOFF 0x13.
 *
 * dws_spinel_frame_encode() wraps a payload; dws_spinel_frame_decode() finds the flag, removes the
 * stuffing, and verifies the FCS. dws_spinel_fcs() is the shared checksum. The spinel command
 * inside (a property get/set/insert, an 802.15.4 stream) is the application's. Pure - you
 * carry the bytes over your UART - so it is fully host-testable.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_THREAD_H
#define DETERMINISTICESPASYNCWEBSERVER_THREAD_H

#include "ServerConfig.h"

#if DWS_ENABLE_THREAD

#include <stddef.h>
#include <stdint.h>

/** @brief HDLC-lite markers. */
struct ThreadHdlc
{
    static constexpr uint8_t HDLC_FLAG = 0x7E;   ///< frame delimiter
    static constexpr uint8_t HDLC_ESCAPE = 0x7D; ///< byte-stuffing escape
};

/** @brief Common spinel commands (the property accessors a gateway uses). */
struct SpinelCmd
{
    static constexpr uint8_t SPINEL_CMD_NOOP = 0;
    static constexpr uint8_t SPINEL_CMD_RESET = 1;
    static constexpr uint8_t SPINEL_CMD_PROP_VALUE_GET = 2;
    static constexpr uint8_t SPINEL_CMD_PROP_VALUE_SET = 3;
    static constexpr uint8_t SPINEL_CMD_PROP_VALUE_INSERT = 4;
    static constexpr uint8_t SPINEL_CMD_PROP_VALUE_REMOVE = 5;
    static constexpr uint8_t SPINEL_CMD_PROP_VALUE_IS = 6; ///< an async property update from the NCP
};

/** @brief HDLC frame check sequence: CRC-16/X-25 over @p buf. */
uint16_t dws_spinel_fcs(const uint8_t *buf, uint16_t len);

// --- Spinel command layer (rides inside a decoded HDLC frame's payload) ---------------

/**
 * @brief Encode a spinel packed unsigned integer (7 bits/byte, little-endian, high bit =
 *        continuation) into @p out.
 * @return the number of bytes written (1..5), or 0 if it would not fit @p cap.
 */
uint8_t dws_spinel_pack_uint(uint32_t value, uint8_t *out, uint8_t cap);

/**
 * @brief Decode a spinel packed unsigned integer from the front of @p raw.
 * @return the bytes consumed (> 0, value in @p value), 0 if more bytes are needed, or -1 if
 *         the encoding overflows a uint32.
 */
int dws_spinel_unpack_uint(const uint8_t *raw, uint8_t len, uint32_t *value);

/**
 * @brief Build a spinel property-command payload (`header | CMD | PROP | value`) - the
 *        content of an HDLC frame - into @p out. CMD and PROP are packed integers.
 * @return the payload length, or 0 if it would not fit @p cap.
 */
uint16_t dws_spinel_command_build(uint8_t header, uint32_t cmd, uint32_t prop, const uint8_t *value, uint16_t value_len,
                                  uint8_t *out, uint16_t cap);

/**
 * @brief Parse a spinel property-command payload (from a decoded HDLC frame).
 * @param[out] header    the flag/iid/tid header byte.
 * @param[out] cmd       the command (unpacked).
 * @param[out] prop      the property id (unpacked).
 * @param[out] value     the first value byte (points into @p payload).
 * @param[out] value_len the value length.
 * @return the value offset (> 0), or -1 if the header / command / property is malformed.
 */
int dws_spinel_command_parse(const uint8_t *payload, uint16_t len, uint8_t *header, uint32_t *cmd, uint32_t *prop,
                             const uint8_t **value, uint16_t *value_len);

/**
 * @brief Encode an HDLC-lite frame: @p payload + FCS, byte-stuffed, flag-terminated.
 * @return the encoded frame length, or 0 if @p len exceeds DWS_THREAD_MAX_DATA or the
 *         stuffed frame would not fit @p cap.
 */
uint16_t dws_spinel_frame_encode(const uint8_t *payload, uint16_t len, uint8_t *out, uint16_t cap);

/**
 * @brief Decode one HDLC-lite frame from the front of @p raw: find the flag, remove the
 *        stuffing, verify the FCS, and copy the spinel payload to @p payload.
 * @param[out] pay_len set to the payload length.
 * @return the bytes consumed up to and including the flag (> 0), 0 if no flag is present yet
 *         (need more), or -1 if the frame is malformed (too short, bad FCS, dangling escape,
 *         or the payload overflows @p pay_cap) - the caller drops one byte and retries.
 */
int dws_spinel_frame_decode(const uint8_t *raw, uint16_t len, uint8_t *payload, uint16_t pay_cap, uint16_t *pay_len);

#endif // DWS_ENABLE_THREAD

#endif // DETERMINISTICESPASYNCWEBSERVER_THREAD_H
