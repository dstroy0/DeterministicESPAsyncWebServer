// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file modbus_master.cpp
 * @brief Modbus TCP master codec - build read requests, parse responses (pure).
 */

#include "services/modbus/modbus_master.h"

#if DWS_ENABLE_MODBUS_MASTER

size_t modbus_build_read(uint8_t fc, uint16_t txid, uint8_t unit, uint16_t start, uint16_t count, uint8_t *out,
                         size_t cap)
{
    if (!out || cap < 12)
        return 0;
    if (fc != 0x03 && fc != 0x04) // read holding / input registers only
        return 0;
    if (count < 1 || count > 125)
        return 0;

    // MBAP header
    out[0] = (uint8_t)(txid >> 8);
    out[1] = (uint8_t)(txid & 0xFF);
    out[2] = 0; // protocol id (Modbus) = 0
    out[3] = 0;
    out[4] = 0; // length = unit(1) + PDU(5) = 6
    out[5] = 6;
    out[6] = unit;
    // PDU
    out[7] = fc;
    out[8] = (uint8_t)(start >> 8);
    out[9] = (uint8_t)(start & 0xFF);
    out[10] = (uint8_t)(count >> 8);
    out[11] = (uint8_t)(count & 0xFF);
    return 12;
}

int modbus_parse_response(const uint8_t *adu, size_t len, uint16_t *regs_out, size_t max_regs, uint8_t *exception_out)
{
    if (exception_out)
        *exception_out = 0;
    if (!adu || len < 9) // MBAP(7) + FC(1) + at least one more byte
        return -1;
    if (adu[2] != 0 || adu[3] != 0) // protocol id must be 0
        return -1;

    uint8_t fc = adu[7];
    if (fc & 0x80) // exception response: FC | 0x80, then the exception code
    {
        if (exception_out)
            *exception_out = adu[8];
        return 0;
    }
    if (fc != 0x03 && fc != 0x04)
        return -1;

    uint8_t byte_count = adu[8];
    if ((byte_count & 1) || len < (size_t)(9 + byte_count)) // must be even and present
        return -1;

    int nregs = byte_count / 2;
    int copied = 0;
    for (int i = 0; i < nregs && (size_t)i < max_regs; i++)
    {
        if (regs_out)
            regs_out[i] = (uint16_t)((adu[9 + i * 2] << 8) | adu[9 + i * 2 + 1]);
        copied++;
    }
    return copied;
}

#endif // DWS_ENABLE_MODBUS_MASTER
