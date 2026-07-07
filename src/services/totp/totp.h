// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file totp.h
 * @brief TOTP two-factor auth (RFC 6238) (DETWS_ENABLE_TOTP).
 *
 * Time-based one-time passwords over HMAC-SHA1 (the existing software SHA-1) -
 * Google Authenticator / Authy compatible. Compute the current code from a shared
 * secret and the Unix time, verify a submitted code within a +/- step window (for
 * clock skew), and decode the base32 secret a provisioning QR/app gives the user.
 * Pure, no heap, host-tested against the RFC 6238 test vectors.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TOTP_H
#define DETERMINISTICESPASYNCWEBSERVER_TOTP_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_TOTP

/**
 * @brief HOTP / TOTP code for @p counter (RFC 4226 dynamic truncation).
 * @param key      shared secret bytes.
 * @param keylen   secret length.
 * @param counter  moving factor (TOTP: floor(unix_time / period)).
 * @param digits   code length (6 or 8).
 * @return the @p digits-digit code (zero-padded by the caller when formatting).
 */
uint32_t detws_hotp(const uint8_t *key, size_t keylen, uint64_t counter, uint8_t digits);

/**
 * @brief TOTP code (RFC 6238) at @p unix_time.
 * @param period  time step in seconds (30 is standard).
 */
uint32_t detws_totp(const uint8_t *key, size_t keylen, uint64_t unix_time, uint32_t period, uint8_t digits);

/**
 * @brief Verify @p code against the current step +/- @p window steps (clock skew).
 * @return true if @p code matches any step in the window.
 */
bool detws_totp_verify(const uint8_t *key, size_t keylen, uint64_t unix_time, uint32_t code, uint32_t period,
                       uint8_t digits, int window);

/**
 * @brief Decode a base32 (RFC 4648) secret into @p out; ignores padding/spaces/dashes.
 * @return decoded byte count, or -1 on an invalid character / overflow.
 */
int detws_base32_decode(const char *b32, uint8_t *out, size_t cap);

#endif // DETWS_ENABLE_TOTP
#endif // DETERMINISTICESPASYNCWEBSERVER_TOTP_H
