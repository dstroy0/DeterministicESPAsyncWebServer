// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sht3x.cpp
 * @brief Sensirion SHT3x temperature / humidity codec - implementation. See sht3x.h.
 */

#include "services/sht3x/sht3x.h"
#include "DetWebServerConfig.h"

#if DETWS_ENABLE_SHT3X

uint8_t sht3x_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF; // Sensirion CRC-8: poly 0x31, init 0xFF, MSB-first, no final XOR
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
    return crc;
}

int32_t sht3x_temp_mc(uint16_t raw)
{
    // T[C] = -45 + 175 * raw / 65535, in milli-degrees (64-bit to avoid overflow).
    return (int32_t)(-45000 + (int64_t)175000 * raw / 65535);
}

int32_t sht3x_rh_mpct(uint16_t raw)
{
    int32_t v = (int32_t)((int64_t)100000 * raw / 65535); // RH[%] = 100 * raw / 65535
    return v > 100000 ? 100000 : v;
}

bool sht3x_parse(const uint8_t resp[6], int32_t *temp_mc, int32_t *rh_mpct)
{
    if (!resp || sht3x_crc8(resp, 2) != resp[2] || sht3x_crc8(resp + 3, 2) != resp[5])
        return false;
    uint16_t traw = (uint16_t)(((uint16_t)resp[0] << 8) | resp[1]);
    uint16_t hraw = (uint16_t)(((uint16_t)resp[3] << 8) | resp[4]);
    if (temp_mc)
        *temp_mc = sht3x_temp_mc(traw);
    if (rh_mpct)
        *rh_mpct = sht3x_rh_mpct(hraw);
    return true;
}

// ---------------------------------------------------------------------------
// I2C binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>

namespace
{
uint8_t s_addr = DETWS_SHT3X_I2C_ADDR;

bool send_cmd(uint16_t cmd)
{
    Wire.beginTransmission(s_addr);
    Wire.write((uint8_t)(cmd >> 8));
    Wire.write((uint8_t)(cmd & 0xFF));
    return Wire.endTransmission() == 0;
}
} // namespace

bool sht3x_begin(uint8_t addr)
{
    s_addr = addr ? addr : (uint8_t)DETWS_SHT3X_I2C_ADDR;
    Wire.begin();
    bool ok = send_cmd(SHT3X_CMD_SOFT_RESET);
    delay(2); // soft reset completes in < 1.5 ms
    return ok;
}

bool sht3x_read(int32_t *temp_mc, int32_t *rh_mpct)
{
    if (!send_cmd(SHT3X_CMD_SINGLE_HIGH))
        return false;
    delay(20); // a high-repeatability measurement completes in < 15 ms
    if (Wire.requestFrom((int)s_addr, 6) != 6)
        return false;
    uint8_t r[6];
    for (int i = 0; i < 6; i++)
        r[i] = (uint8_t)Wire.read();
    return sht3x_parse(r, temp_mc, rh_mpct);
}

#else // host build: no I2C. The CRC + conversion above are host-tested.

bool sht3x_begin(uint8_t)
{
    return false;
}
bool sht3x_read(int32_t *, int32_t *)
{
    return false;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_SHT3X
