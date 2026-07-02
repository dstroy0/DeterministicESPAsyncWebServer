// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file thread.h
 * @brief Thread spinel / HDLC-lite framing codec (DETWS_ENABLE_THREAD) - OpenThread RCP.
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
 * spinel_frame_encode() wraps a payload; spinel_frame_decode() finds the flag, removes the
 * stuffing, and verifies the FCS. spinel_fcs() is the shared checksum. The spinel command
 * inside (a property get/set/insert, an 802.15.4 stream) is the application's. Pure - you
 * carry the bytes over your UART - so it is fully host-testable.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_THREAD_H
#define DETERMINISTICESPASYNCWEBSERVER_THREAD_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_THREAD

#include <stddef.h>
#include <stdint.h>

/** @brief HDLC-lite markers. */
enum
{
    HDLC_FLAG = 0x7E,   ///< frame delimiter
    HDLC_ESCAPE = 0x7D, ///< byte-stuffing escape
};

/** @brief HDLC frame check sequence: CRC-16/X-25 over @p buf. */
uint16_t spinel_fcs(const uint8_t *buf, uint16_t len);

/**
 * @brief Encode an HDLC-lite frame: @p payload + FCS, byte-stuffed, flag-terminated.
 * @return the encoded frame length, or 0 if @p len exceeds DETWS_THREAD_MAX_DATA or the
 *         stuffed frame would not fit @p cap.
 */
uint16_t spinel_frame_encode(const uint8_t *payload, uint16_t len, uint8_t *out, uint16_t cap);

/**
 * @brief Decode one HDLC-lite frame from the front of @p raw: find the flag, remove the
 *        stuffing, verify the FCS, and copy the spinel payload to @p payload.
 * @param[out] pay_len set to the payload length.
 * @return the bytes consumed up to and including the flag (> 0), 0 if no flag is present yet
 *         (need more), or -1 if the frame is malformed (too short, bad FCS, dangling escape,
 *         or the payload overflows @p pay_cap) - the caller drops one byte and retries.
 */
int spinel_frame_decode(const uint8_t *raw, uint16_t len, uint8_t *payload, uint16_t pay_cap, uint16_t *pay_len);

#endif // DETWS_ENABLE_THREAD

#endif // DETERMINISTICESPASYNCWEBSERVER_THREAD_H
