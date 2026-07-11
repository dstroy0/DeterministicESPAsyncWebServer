// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sigfox.h
 * @brief Sigfox modem AT-command codec (DETWS_ENABLE_SIGFOX) - Wisol / Murata over UART.
 *
 * The tiny-uplink half of a Sigfox-to-web bridge. A Wisol (SFM10R) / Murata Sigfox modem
 * is driven by AT commands over a UART: sigfox_build_uplink() formats an `AT$SF=<hex>`
 * command for a payload (the Sigfox network caps a message at 12 bytes and ~140 messages
 * per day, so uplinks are rare and small), and sigfox_parse_response() classifies the
 * modem's reply as OK, ERROR, or still pending (nothing conclusive yet). Pure text codec -
 * you carry the bytes over your UART - so it is fully host-testable. This is uplink-only
 * (the common Sigfox use); a device sends readings up, it is not addressed downlink.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SIGFOX_H
#define DETERMINISTICESPASYNCWEBSERVER_SIGFOX_H

#include "ServerConfig.h"

#if DETWS_ENABLE_SIGFOX

#include <stddef.h>
#include <stdint.h>

/** @brief Classification of a Sigfox modem response line. */
enum class sigfox_result : uint8_t
{
    SIGFOX_PENDING = 0, ///< nothing conclusive yet (echo / partial); keep reading
    SIGFOX_OK = 1,      ///< the modem accepted / completed the command
    SIGFOX_ERROR = 2,   ///< the modem reported an error
};

/**
 * @brief Format an `AT$SF=<hex>\r\n` uplink command for @p payload into @p out (a NUL-
 *        terminated C string).
 * @return the command length (excluding the NUL), or 0 if @p len exceeds
 *         DETWS_SIGFOX_MAX_PAYLOAD or the command would not fit @p cap.
 */
uint16_t sigfox_build_uplink(const uint8_t *payload, uint8_t len, char *out, uint16_t cap);

/**
 * @brief Classify a modem reply (scans @p buf for "OK" / "ERROR").
 * @return sigfox_result::SIGFOX_OK, sigfox_result::SIGFOX_ERROR, or sigfox_result::SIGFOX_PENDING if neither is present
 * yet.
 */
sigfox_result sigfox_parse_response(const char *buf, uint16_t len);

#endif // DETWS_ENABLE_SIGFOX

#endif // DETERMINISTICESPASYNCWEBSERVER_SIGFOX_H
