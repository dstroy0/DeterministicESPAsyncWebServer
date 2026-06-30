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

#include "network_drivers/presentation/base64/base64.h"

#ifdef ARDUINO

// --- ESP32 / Arduino: use mbedTLS -------------------------------------------

void base64_encode(const uint8_t *src, size_t src_len, char *dst)
{
    size_t olen;
    size_t dlen = ((src_len + 2) / 3) * 4 + 1;
    mbedtls_base64_encode((unsigned char *)dst, dlen, &olen, src, src_len);
    dst[olen] = '\0';
}

size_t base64_decode(const char *src, uint8_t *dst, size_t dst_cap)
{
    size_t src_len = strlen(src);
    size_t olen;
    // Pass the caller's true capacity; mbedtls returns BUFFER_TOO_SMALL (and we
    // map it to 0) rather than overrunning dst.
    int ret = mbedtls_base64_decode(dst, dst_cap, &olen, (const unsigned char *)src, src_len);
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
    // '=' is NOT a value here - padding is validated positionally by the
    // decoder (1-2 '=' only at the end of the final quad).
    return -1;
}

size_t base64_decode(const char *src, uint8_t *dst, size_t dst_cap)
{
    size_t out = 0;

    // Canonical Base64: process complete 4-character quads. A truncated final
    // group, any non-base64 character, or misplaced '=' padding fails (return 0).
    while (*src)
    {
        char c0 = src[0], c1 = src[1], c2 = src[2], c3 = src[3];
        if (!c1 || !c2 || !c3)
            return 0; // input length must be a multiple of 4

        // The first two characters of every quad must be real Base64 (never pad).
        int a = b64_val(c0);
        int b = b64_val(c1);
        if (a < 0 || b < 0)
            return 0;

        // '=' is permitted only as 1-2 trailing pad characters of the FINAL quad.
        bool pad2 = (c2 == '=');
        bool pad3 = (c3 == '=');
        if (pad2 && !pad3)
            return 0; // "xx=y" - a single pad must be in position 4, not 3
        if ((pad2 || pad3) && src[4] != '\0')
            return 0; // padding only allowed in the last quad

        int c = pad2 ? 0 : b64_val(c2);
        int d = pad3 ? 0 : b64_val(c3);
        if (c < 0 || d < 0)
            return 0;

        // Reassemble the 24-bit group and slice it back into bytes. Each write is
        // bounded by dst_cap; an over-capacity decode fails rather than overruns.
        if (out >= dst_cap)
            return 0;
        dst[out++] = (uint8_t)((a << 2) | (b >> 4));
        if (!pad2)
        {
            if (out >= dst_cap)
                return 0;
            dst[out++] = (uint8_t)((b << 4) | (c >> 2));
        }
        if (!pad3)
        {
            if (out >= dst_cap)
                return 0;
            dst[out++] = (uint8_t)((c << 6) | d);
        }

        src += 4;
    }

    return out;
}

#endif // ARDUINO

// ---------------------------------------------------------------------------
// Base64url (RFC 4648 section 5): '-' / '_' replace '+' / '/', no '=' padding.
// Shared by JWT (HS256) and OIDC (RS256) so the alphabet lives in one place.
// Platform-independent: encode builds on base64_encode then rewrites the two
// differing characters in place; decode is a direct streaming decoder that
// accepts the URL and standard alphabets and stops at '=', so it needs no temp
// buffer or re-padding.
// ---------------------------------------------------------------------------

size_t base64url_encode(const uint8_t *src, size_t src_len, char *dst)
{
    base64_encode(src, src_len, dst); // standard base64, '='-padded, NUL-terminated
    size_t n = 0;
    for (size_t i = 0; dst[i]; i++)
    {
        char c = dst[i];
        if (c == '=')
            break; // base64url carries no padding
        if (c == '+')
            dst[i] = '-';
        else if (c == '/')
            dst[i] = '_';
        n = i + 1;
    }
    dst[n] = '\0';
    return n;
}

static int base64url_val(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    // RFC 4648 section 5: the URL-safe alphabet uses '-'/'_'. The standard '+'/'/'
    // are NOT valid here; a strict JWS/JWT decoder rejects them rather than
    // silently treating two alphabets as one (RFC 7515).
    if (c == '-')
        return 62;
    if (c == '_')
        return 63;
    return -1;
}

size_t base64url_decode(const char *src, size_t src_len, uint8_t *dst, size_t dst_cap)
{
    size_t o = 0;
    uint32_t acc = 0;
    int bits = 0;
    for (size_t i = 0; i < src_len; i++)
    {
        if (src[i] == '=')
            break; // optional padding ends the input
        int v = base64url_val(src[i]);
        if (v < 0)
            return 0;
        acc = (acc << 6) | (uint32_t)v;
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            if (o >= dst_cap)
                return 0;
            dst[o++] = (uint8_t)((acc >> bits) & 0xFF);
        }
    }
    return o;
}
