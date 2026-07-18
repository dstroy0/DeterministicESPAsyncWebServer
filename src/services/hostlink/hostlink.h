// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hostlink.h
 * @brief Omron Host Link (C-mode) frame codec (DWS_ENABLE_HOSTLINK) - zero-heap ASCII
 *        command/response framing for the Omron serial host-link protocol, the RS-232/485
 *        sibling of FINS.
 *
 * A Host Link frame is ASCII:
 * @code
 *   @ UU XX <text> FF * CR
 * @endcode
 *  - `@` start, `UU` the 2-digit unit/node number, `XX` the 2-char header code (e.g. `RD`),
 *    `<text>` the data, `FF` the 2-hex-char FCS, then the `*` + CR (0x0D) terminator.
 *  - FCS = the 8-bit XOR of every character from `@` through the last text character,
 *    rendered as two uppercase hex digits.
 *  - A response's text begins with a 2-char end code (00 = normal).
 *
 * This is the frame codec (build + FCS-validated parse); the serial transport is the app's.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_HOSTLINK_H
#define DETERMINISTICESPASYNCWEBSERVER_HOSTLINK_H

#include "ServerConfig.h"

#if DWS_ENABLE_HOSTLINK

#include <stddef.h>
#include <stdint.h>

/** @brief FCS: 8-bit XOR of [data, data+len). */
uint8_t hostlink_fcs(const char *data, size_t len);

/**
 * @brief Build a frame: `@UU` + header_code(2) + text + FCS(2 hex) + `*` + CR.
 * @param node         unit/node number (0-99, rendered as 2 BCD-style digits).
 * @param header_code  the 2-character header code (e.g. "RD"); must be 2 chars.
 * @return total characters written (NOT counting the NUL), or 0 on overflow / bad input.
 * @note  The frame is NUL-terminated, so @p cap must hold the frame plus one terminator
 *        byte; the return value is the frame length, so callers may also treat @p buf as a
 *        C-string.
 */
size_t hostlink_build(char *buf, size_t cap, uint8_t node, const char *header_code, const char *text, size_t text_len);

/** @brief A parsed frame; @ref text points INTO the source buffer (after the header, before the FCS). */
struct HostlinkFrame
{
    uint8_t node;
    char header_code[3]; ///< 2 chars + NUL
    const char *text;
    size_t text_len;
};

/**
 * @brief Parse + FCS-validate a frame (command or response).
 * @return true on a complete, FCS-valid `@...*CR` frame; false otherwise.
 */
bool hostlink_parse(const char *buf, size_t len, HostlinkFrame *out);

/** @brief Read a response's 2-char end code (the first two text characters) as a byte. */
bool hostlink_end_code(const HostlinkFrame *f, uint8_t *code);

#endif // DWS_ENABLE_HOSTLINK

#endif // DETERMINISTICESPASYNCWEBSERVER_HOSTLINK_H
