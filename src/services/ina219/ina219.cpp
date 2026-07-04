// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ina219.cpp
 * @brief TI INA219 current / power monitor codec - implementation. See ina219.h.
 */

#include "services/ina219/ina219.h"
#include "DetWebServerConfig.h"

#if DETWS_ENABLE_INA219

int32_t ina219_bus_mv(uint16_t raw)
{
    return (int32_t)((raw >> 3) * 4); // value in bits [15:3], LSB 4 mV
}

int32_t ina219_shunt_uv(int16_t raw)
{
    return (int32_t)raw * 10; // LSB 10 uV, signed
}

uint16_t ina219_calibration(uint32_t current_lsb_ua, uint32_t shunt_mohm)
{
    uint32_t denom = current_lsb_ua * shunt_mohm;
    if (denom == 0)
        return 0;
    // 0.04096 / (lsb[A] * R[ohm]) = 40960000 / (lsb_ua * shunt_mohm).
    uint32_t cal = 40960000u / denom;
    return (uint16_t)(cal > 0xFFFF ? 0xFFFF : cal);
}

int32_t ina219_current_ua(int16_t raw, uint32_t current_lsb_ua)
{
    return (int32_t)((int64_t)raw * current_lsb_ua);
}

int32_t ina219_power_uw(int16_t raw, uint32_t current_lsb_ua)
{
    return (int32_t)((int64_t)raw * 20 * current_lsb_ua); // power LSB = 20 * current LSB
}

// ---------------------------------------------------------------------------
// I2C binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>

namespace
{
uint8_t s_addr = DETWS_INA219_I2C_ADDR;
uint32_t s_lsb_ua = 100;

bool wr16(uint8_t reg, uint16_t v)
{
    Wire.beginTransmission(s_addr);
    Wire.write(reg);
    Wire.write((uint8_t)(v >> 8)); // INA219 registers are big-endian
    Wire.write((uint8_t)(v & 0xFF));
    return Wire.endTransmission() == 0;
}

bool rd16(uint8_t reg, uint16_t *v)
{
    Wire.beginTransmission(s_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;
    if (Wire.requestFrom((int)s_addr, 2) != 2)
        return false;
    uint8_t hi = (uint8_t)Wire.read();
    uint8_t lo = (uint8_t)Wire.read();
    *v = (uint16_t)(((uint16_t)hi << 8) | lo);
    return true;
}
} // namespace

bool ina219_begin(uint8_t addr, uint32_t current_lsb_ua, uint32_t shunt_mohm)
{
    s_addr = addr ? addr : (uint8_t)DETWS_INA219_I2C_ADDR;
    s_lsb_ua = current_lsb_ua ? current_lsb_ua : 100u;
    Wire.begin();
    bool ok = true;
    ok &= wr16(INA219_REG_CALIBRATION, ina219_calibration(s_lsb_ua, shunt_mohm ? shunt_mohm : 100u));
    ok &= wr16(INA219_REG_CONFIG, 0x399F); // 32 V range, /8 gain (320 mV), 12-bit, continuous
    return ok;
}

bool ina219_read_bus_mv(int32_t *millivolts)
{
    uint16_t v = 0;
    if (!rd16(INA219_REG_BUS, &v))
        return false;
    if (millivolts)
        *millivolts = ina219_bus_mv(v);
    return true;
}

bool ina219_read_shunt_uv(int32_t *microvolts)
{
    uint16_t v = 0;
    if (!rd16(INA219_REG_SHUNT, &v))
        return false;
    if (microvolts)
        *microvolts = ina219_shunt_uv((int16_t)v);
    return true;
}

bool ina219_read_current_ua(int32_t *microamps)
{
    uint16_t v = 0;
    if (!rd16(INA219_REG_CURRENT, &v))
        return false;
    if (microamps)
        *microamps = ina219_current_ua((int16_t)v, s_lsb_ua);
    return true;
}

bool ina219_read_power_uw(int32_t *microwatts)
{
    uint16_t v = 0;
    if (!rd16(INA219_REG_POWER, &v))
        return false;
    if (microwatts)
        *microwatts = ina219_power_uw((int16_t)v, s_lsb_ua);
    return true;
}

#else // host build: no I2C. The decode / calibration / scaling above are host-tested.

bool ina219_begin(uint8_t, uint32_t, uint32_t)
{
    return false;
}
bool ina219_read_bus_mv(int32_t *)
{
    return false;
}
bool ina219_read_shunt_uv(int32_t *)
{
    return false;
}
bool ina219_read_current_ua(int32_t *)
{
    return false;
}
bool ina219_read_power_uw(int32_t *)
{
    return false;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_INA219
