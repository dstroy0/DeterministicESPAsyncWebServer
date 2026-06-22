// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file base64.cpp
 * @brief Base64 encoder/decoder implementation.
 *
 * On Arduino (ESP32) targets, delegates to mbedtls_base64_encode/decode()
 * which is part of the ESP-IDF mbedTLS bundle - same SDK path as SHA-1.
 *
 * On native (x86) test targets, uses a portable software implementation so
 * unit tests run without mbedTLS installed.
 */

#include "base64.h"

#ifdef ARDUINO

// --- ESP32 / Arduino: use mbedTLS -------------------------------------------
#include "mbedtls/base64.h"
#include <string.h>

void base64_encode(const uint8_t *src, size_t src_len, char *dst)
{
    size_t olen;
    size_t dlen = ((src_len + 2) / 3) * 4 + 1;
    mbedtls_base64_encode((unsigned char *)dst, dlen, &olen, src, src_len);
    dst[olen] = '\0';
}

size_t base64_decode(const char *src, uint8_t *dst)
{
    size_t src_len = strlen(src);
    size_t olen;
    size_t dlen = (src_len / 4) * 3 + 3;
    int ret = mbedtls_base64_decode(dst, dlen, &olen, (const unsigned char *)src, src_len);
    return (ret == 0) ? olen : 0;
}

#else

// --- Native / test: software Base64, no external dependencies ---------------
#include <stddef.h>

static const char B64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const uint8_t *src, size_t src_len, char *dst)
{
    size_t i = 0, j = 0;

    // Full groups: pack 3 input bytes into a 24-bit value, emit as 4 x 6-bit
    // characters (most-significant 6 bits first).
    while (i + 2 < src_len)
    {
        uint32_t v = ((uint32_t)src[i] << 16) | ((uint32_t)src[i + 1] << 8) | (uint32_t)src[i + 2];
        dst[j++] = B64_TABLE[(v >> 18) & 0x3F];
        dst[j++] = B64_TABLE[(v >> 12) & 0x3F];
        dst[j++] = B64_TABLE[(v >> 6) & 0x3F];
        dst[j++] = B64_TABLE[(v) & 0x3F];
        i += 3;
    }

    // Tail of 1 or 2 leftover bytes: emit 2 or 3 characters, '='-padded to 4.
    if (i < src_len)
    {
        uint32_t v = (uint32_t)src[i] << 16;
        if (i + 1 < src_len)
            v |= (uint32_t)src[i + 1] << 8;

        dst[j++] = B64_TABLE[(v >> 18) & 0x3F];
        dst[j++] = B64_TABLE[(v >> 12) & 0x3F];
        dst[j++] = (i + 1 < src_len) ? B64_TABLE[(v >> 6) & 0x3F] : '='; // 3rd char only if 2 input bytes
        dst[j++] = '=';
    }

    dst[j] = '\0';
}

static int b64_val(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    if (c == '=')
        return 0;
    return -1;
}

size_t base64_decode(const char *src, uint8_t *dst)
{
    size_t out = 0;

    // Process 4 characters at a time -> up to 3 output bytes. Any non-base64
    // character (b64_val returns -1) aborts and signals failure by returning 0.
    while (*src)
    {
        int a = b64_val(src[0]);
        int b = (src[1]) ? b64_val(src[1]) : -1;
        int c = (src[2]) ? b64_val(src[2]) : -1;
        int d = (src[3]) ? b64_val(src[3]) : -1;

        if (a < 0 || b < 0 || c < 0 || d < 0)
            return 0;

        // Reassemble the 24-bit group and slice it back into bytes; trailing
        // '=' padding suppresses the corresponding output byte(s).
        dst[out++] = (uint8_t)((a << 2) | (b >> 4));
        if (src[2] && src[2] != '=')
            dst[out++] = (uint8_t)((b << 4) | (c >> 2));
        if (src[3] && src[3] != '=')
            dst[out++] = (uint8_t)((c << 6) | d);

        src += 4;
    }

    return out;
}

#endif // ARDUINO
