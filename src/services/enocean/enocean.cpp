// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file enocean.cpp
 * @brief EnOcean ESP3 serial codec - implementation.
 *
 * ESP3 telegram: 0x55 | data-len(2) | opt-len(1) | type(1) | CRC8H | data | opt | CRC8D.
 * CRC-8 is polynomial 0x07, MSB-first, init 0x00 (the ESP3 u8CRC8Table generator).
 */

#include "services/enocean/enocean.h"

#if DETWS_ENABLE_ENOCEAN

#include <string.h>

uint8_t esp3_crc8(const uint8_t *buf, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    }
    return crc;
}

int esp3_parse(const uint8_t *raw, uint16_t len, esp3_packet *out)
{
    if (!raw || len < 1)
        return 0;
    if (raw[0] != ESP3_SYNC)
        return -1; // not a telegram start
    if (len < 6)
        return 0; // need sync + 4-byte header + CRC8H
    uint16_t data_len = (uint16_t)((raw[1] << 8) | raw[2]);
    uint8_t opt_len = raw[3];
    uint8_t type = raw[4];
    if (data_len > DETWS_ENOCEAN_MAX_DATA)
        return -1; // implausible length -> resynchronise
    if (esp3_crc8(&raw[1], 4) != raw[5])
        return -1; // header CRC mismatch
    uint32_t total = 6u + data_len + opt_len + 1u;
    if (len < total)
        return 0; // wait for the rest of the telegram
    if (esp3_crc8(&raw[6], (uint16_t)(data_len + opt_len)) != raw[6 + data_len + opt_len])
        return -1; // data CRC mismatch
    if (out)
    {
        out->type = (esp3_type)type;
        out->data = &raw[6];
        out->data_len = data_len;
        out->opt = &raw[6 + data_len];
        out->opt_len = opt_len;
    }
    return (int)total;
}

uint16_t esp3_build(esp3_type type, const uint8_t *data, uint16_t data_len, const uint8_t *opt, uint8_t opt_len,
                    uint8_t *out, uint16_t cap)
{
    if (!out || data_len > DETWS_ENOCEAN_MAX_DATA)
        return 0;
    uint32_t total = 6u + data_len + opt_len + 1u;
    if (total > cap)
        return 0;
    out[0] = ESP3_SYNC;
    out[1] = (uint8_t)(data_len >> 8);
    out[2] = (uint8_t)(data_len & 0xFF);
    out[3] = opt_len;
    out[4] = (uint8_t)type;
    out[5] = esp3_crc8(&out[1], 4);
    for (uint16_t i = 0; i < data_len; i++)
        out[6 + i] = data[i];
    for (uint8_t i = 0; i < opt_len; i++)
        out[6 + data_len + i] = opt[i];
    out[6 + data_len + opt_len] = esp3_crc8(&out[6], (uint16_t)(data_len + opt_len));
    return (uint16_t)total;
}

#endif // DETWS_ENABLE_ENOCEAN
