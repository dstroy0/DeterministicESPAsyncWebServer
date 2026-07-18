// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file interbus.cpp
 * @brief INTERBUS summation-frame codec (see interbus.h).
 */

#include "services/interbus/interbus.h"

#if DWS_ENABLE_INTERBUS

uint16_t dws_interbus_fcs(const uint8_t *bytes, size_t len)
{
    // CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, xorout 0.
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)bytes[i] << 8;
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

size_t dws_interbus_build(const uint16_t *words, size_t word_count, uint8_t *out, size_t cap)
{
    if (!out || (word_count && !words))
        return 0;
    size_t n = 2 + word_count * 2 + 2; // loopback + words + FCS
    if (n > cap)
        return 0;
    size_t i = 0;
    out[i++] = (uint8_t)(INTERBUS_LOOPBACK >> 8);
    out[i++] = (uint8_t)INTERBUS_LOOPBACK;
    for (size_t w = 0; w < word_count; w++)
    {
        out[i++] = (uint8_t)(words[w] >> 8); // big-endian
        out[i++] = (uint8_t)words[w];
    }
    uint16_t crc = dws_interbus_fcs(out, i); // FCS over loopback + words
    out[i++] = (uint8_t)(crc >> 8);
    out[i++] = (uint8_t)crc;
    return i;
}

bool dws_interbus_parse(const uint8_t *frame, size_t len, uint16_t *out_words, size_t max_words, size_t *out_count)
{
    if (!frame || !out_words || !out_count || len < 4) // loopback + FCS minimum
        return false;
    if (((frame[0] << 8) | frame[1]) != INTERBUS_LOOPBACK)
        return false;
    if ((len - 4) % 2 != 0) // the words region must be whole 16-bit words
        return false;
    size_t word_count = (len - 4) / 2;
    if (word_count > max_words)
        return false;
    uint16_t want = dws_interbus_fcs(frame, len - 2);
    uint16_t got = (uint16_t)((frame[len - 2] << 8) | frame[len - 1]);
    if (want != got)
        return false;
    for (size_t w = 0; w < word_count; w++)
        out_words[w] = (uint16_t)((frame[2 + w * 2] << 8) | frame[2 + w * 2 + 1]);
    *out_count = word_count;
    return true;
}

#endif // DWS_ENABLE_INTERBUS
