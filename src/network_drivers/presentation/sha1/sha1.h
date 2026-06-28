// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sha1.h
 * @brief Software SHA-1 implementation - no platform dependencies.
 *
 * Used exclusively for the WebSocket opening handshake (RFC 6455 §4.2.2).
 * Output is always a 20-byte digest.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SHA1_H
#define DETERMINISTICESPASYNCWEBSERVER_SHA1_H

#include <stddef.h>
#include <stdint.h>

/** @brief SHA-1 digest length in bytes. */
#define SHA1_DIGEST_LEN 20

/**
 * @brief Compute a SHA-1 digest over an arbitrary byte buffer.
 *
 * @param data    Input bytes.
 * @param len     Number of input bytes.
 * @param digest  Output buffer; must be at least SHA1_DIGEST_LEN bytes.
 */
void sha1(const uint8_t *data, size_t len, uint8_t digest[SHA1_DIGEST_LEN]);

#endif
