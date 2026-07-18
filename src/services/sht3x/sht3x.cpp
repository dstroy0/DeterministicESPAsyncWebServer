// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sht3x.cpp
 * @brief Sensirion SHT3x temperature / humidity codec - implementation. See sht3x.h.
 */

#include "services/sht3x/sht3x.h"
#include "ServerConfig.h"
#include "services/clock.h" // dwsdelay

#if DWS_ENABLE_SHT3X

uint8_t dws_sht3x_crc8(const uint8_t *data, size_t len)
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

int32_t dws_sht3x_temp_mc(uint16_t raw)
{
    // T[C] = -45 + 175 * raw / 65535, in milli-degrees (64-bit to avoid overflow).
    return (int32_t)(-45000 + (int64_t)175000 * raw / 65535);
}

int32_t dws_sht3x_rh_mpct(uint16_t raw)
{
    int32_t v = (int32_t)((int64_t)100000 * raw / 65535); // RH[%] = 100 * raw / 65535
    return v > 100000 ? 100000 : v;
}

bool dws_sht3x_parse(const uint8_t resp[6], int32_t *temp_mc, int32_t *rh_mpct)
{
    if (!resp || dws_sht3x_crc8(resp, 2) != resp[2] || dws_sht3x_crc8(resp + 3, 2) != resp[5])
        return false;
    uint16_t traw = (uint16_t)(((uint16_t)resp[0] << 8) | resp[1]);
    uint16_t hraw = (uint16_t)(((uint16_t)resp[3] << 8) | resp[4]);
    if (temp_mc)
        *temp_mc = dws_sht3x_temp_mc(traw);
    if (rh_mpct)
        *rh_mpct = dws_sht3x_rh_mpct(hraw);
    return true;
}

// ---------------------------------------------------------------------------
// I2C binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "services/i2c.h"
#include <Arduino.h>
#include <Wire.h>

namespace
{
// All SHT3x I2C-binding state, owned by one instance (internal linkage): the device address,
// so it is one named owner, unreachable from any other translation unit.
struct Sht3xCtx
{
    uint8_t addr = DWS_SHT3X_I2C_ADDR;
};
Sht3xCtx s_sht;

bool send_cmd(uint16_t cmd)
{
    Wire.beginTransmission(s_sht.addr);
    Wire.write((uint8_t)(cmd >> 8));
    Wire.write((uint8_t)(cmd & 0xFF));
    return Wire.endTransmission() == 0;
}
} // namespace

bool dws_sht3x_begin(uint8_t addr)
{
    s_sht.addr = addr ? addr : (uint8_t)DWS_SHT3X_I2C_ADDR;
    dws_i2c_begin();
    bool ok = send_cmd(SHT3X_CMD_SOFT_RESET);
    dwsdelay(2); // soft reset completes in < 1.5 ms
    return ok;
}

bool dws_sht3x_read(int32_t *temp_mc, int32_t *rh_mpct)
{
    if (!send_cmd(SHT3X_CMD_SINGLE_HIGH))
        return false;
    dwsdelay(20); // a high-repeatability measurement completes in < 15 ms
    if (Wire.requestFrom((int)s_sht.addr, 6) != 6)
        return false;
    uint8_t r[6];
    for (int i = 0; i < 6; i++)
        r[i] = (uint8_t)Wire.read();
    return dws_sht3x_parse(r, temp_mc, rh_mpct);
}

#else // host build: no I2C. The CRC + conversion above are host-tested.

bool dws_sht3x_begin(uint8_t)
{
    return false;
}
bool dws_sht3x_read(int32_t *, int32_t *)
{
    return false;
}

#endif // ARDUINO

#endif // DWS_ENABLE_SHT3X
