// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file base64.h
 * @brief Base64 encoder/decoder.
 *
 * On Arduino (ESP32) targets, delegates to mbedtls_base64_encode/decode()
 * from the ESP-IDF mbedTLS bundle — same SDK path as SHA-1.
 *
 * On native (x86) test targets, uses a portable software implementation so
 * unit tests run without mbedTLS installed.
 *
 * Used to encode the SHA-1 digest in the WebSocket handshake response
 * (RFC 6455 §4.2.2) and to decode Basic Auth credentials (RFC 7617).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_BASE64_H
#define DETERMINISTICESPASYNCWEBSERVER_BASE64_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Encode @p src_len bytes of @p src as Base64.
 *
 * Writes a null-terminated string into @p dst.  @p dst must be at least
 * `((src_len + 2) / 3) * 4 + 1` bytes.
 *
 * @param src     Input bytes.
 * @param src_len Number of input bytes.
 * @param dst     Output buffer (null-terminated Base64 string).
 */
void base64_encode(const uint8_t *src, size_t src_len, char *dst);

/**
 * @brief Decode a null-terminated Base64 string.
 *
 * Writes decoded bytes into @p dst.  @p dst must be at least
 * `(strlen(src) / 4) * 3` bytes.  Returns the number of decoded bytes.
 * Returns 0 on invalid input.
 *
 * @param src     Null-terminated Base64 input string.
 * @param dst     Output byte buffer.
 * @return        Number of bytes written to @p dst, or 0 on error.
 */
size_t base64_decode(const char *src, uint8_t *dst);

#endif
