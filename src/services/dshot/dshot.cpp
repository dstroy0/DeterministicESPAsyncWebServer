// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dshot.cpp
 * @brief DShot ESC throttle protocol codec (see dshot.h).
 */

#include "services/dshot/dshot.h"

#if DETWS_ENABLE_DSHOT

namespace
{
// The DShot CRC: xor of the three 4-bit nibbles of the 12-bit (value<<1 | telemetry) word.
uint8_t dshot_crc(uint16_t v12, bool bidirectional)
{
    uint8_t crc = (uint8_t)((v12 ^ (v12 >> 4) ^ (v12 >> 8)) & 0x0F);
    if (bidirectional)
        crc = (uint8_t)((~crc) & 0x0F); // bidirectional/extended DShot inverts the CRC
    return crc;
}
} // namespace

uint16_t detws_dshot_encode(uint16_t value11, bool telemetry, bool bidirectional)
{
    value11 &= 0x07FF; // 11-bit value field
    uint16_t v12 = (uint16_t)((value11 << 1) | (telemetry ? 1 : 0));
    uint8_t crc = dshot_crc(v12, bidirectional);
    return (uint16_t)((v12 << 4) | crc);
}

bool detws_dshot_decode(uint16_t frame, uint16_t *value11, bool *telemetry, bool bidirectional)
{
    uint16_t v12 = (uint16_t)(frame >> 4);
    uint8_t got = (uint8_t)(frame & 0x0F);
    if (got != dshot_crc(v12, bidirectional))
        return false;
    if (value11)
        *value11 = (uint16_t)(v12 >> 1);
    if (telemetry)
        *telemetry = (v12 & 1) != 0;
    return true;
}

uint32_t detws_dshot_bit_ns(uint16_t rate_kbit, bool bit)
{
    uint32_t period_ns; // one bit-period, in ns, at rate_kbit kbit/s
    switch (rate_kbit)
    {
    case 150:
        period_ns = 6667;
        break;
    case 300:
        period_ns = 3333;
        break;
    case 600:
        period_ns = 1667;
        break;
    case 1200:
        period_ns = 833;
        break;
    default:
        return 0;
    }
    // A "1" holds high ~3/4 of the period, a "0" ~3/8 (T1H = 2 * T0H); the ESC samples the pulse width.
    return bit ? (period_ns * 3 / 4) : (period_ns * 3 / 8);
}

#endif // DETWS_ENABLE_DSHOT
