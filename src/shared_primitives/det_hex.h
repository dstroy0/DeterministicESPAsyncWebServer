// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_hex.h
 * @brief Tiny no-stdlib hex encode / decode / nibble helpers (one shared copy).
 *
 * Hex was open-coded in ~10 places (CSRF token, WebDAV %XX, provisioning, audit-log
 * hash, device-id, HTTP auth, OAuth2 percent-encode). These header-only inline
 * helpers are the single home for it, mirroring det_numparse.h - no `<stdlib.h>`,
 * no heap, and zero link cost when unused.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_HEX_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_HEX_H

#include <stddef.h>
#include <stdint.h>

/** @brief Nibble 0..15 -> hex char (lowercase, or uppercase when @p upper). */
inline char det_hex_digit(int nibble, bool upper = false)
{
    const char *H = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    return H[nibble & 0xF];
}

/** @brief Hex digit -> 0..15, or -1 if @p c is not a hex digit (either case). */
inline int det_hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/**
 * @brief Encode @p n bytes as 2*n hex chars + NUL into @p out (needs cap >= 2*n+1).
 * @param upper  uppercase A-F when true, else lowercase a-f.
 */
inline void det_hex_encode(const uint8_t *in, size_t n, char *out, bool upper = false)
{
    const char *H = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (size_t i = 0; i < n; i++)
    {
        out[2 * i] = H[(in[i] >> 4) & 0xF];
        out[2 * i + 1] = H[in[i] & 0xF];
    }
    out[2 * n] = '\0';
}

/**
 * @brief Decode exactly @p hexlen hex chars into @p out (hexlen/2 bytes).
 * @return byte count, or -1 on odd length / overflow (> @p out_cap) / non-hex input.
 */
inline int det_hex_decode(const char *in, size_t hexlen, uint8_t *out, size_t out_cap)
{
    if ((hexlen % 2) != 0 || (hexlen / 2) > out_cap)
        return -1;
    for (size_t i = 0; i < hexlen; i += 2)
    {
        int hi = det_hex_val(in[i]);
        int lo = det_hex_val(in[i + 1]);
        if (hi < 0 || lo < 0)
            return -1;
        out[i / 2] = (uint8_t)((hi << 4) | lo);
    }
    return (int)(hexlen / 2);
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_HEX_H
