// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file df1.cpp
 * @brief Allen-Bradley DF1 full-duplex frame builder + parser (pure, host-tested).
 */

#include "services/df1/df1.h"

#if DWS_ENABLE_DF1

uint8_t df1_bcc(const uint8_t *data, size_t len)
{
    uint8_t s = 0;
    for (size_t i = 0; i < len; i++)
        s = (uint8_t)(s + data[i]);
    return (uint8_t)(0u - s); // 2's complement (modulo 256)
}

// Reflected CRC-16 (poly 0xA001 = reflect(0x8005)), continuing from @p crc.
static uint16_t crc16(uint16_t crc, const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    return crc;
}

uint16_t df1_crc(const uint8_t *data, size_t len)
{
    return crc16(0x0000, data, len);
}

size_t df1_build_frame(uint8_t *buf, size_t cap, const uint8_t *data, size_t data_len, Df1Check check)
{
    if (!buf || (data_len && !data))
        return 0;
    size_t stuffed = data_len;
    for (size_t i = 0; i < data_len; i++)
        if (data[i] == DF1_DLE)
            stuffed++; // a DLE data byte is doubled on the wire
    size_t checklen = (check == Df1Check::DF1_CHECK_CRC) ? 2 : 1;
    size_t total = 2 + stuffed + 2 + checklen; // DLE STX + data + DLE ETX + check
    if (total > cap)
        return 0;

    size_t p = 0;
    buf[p++] = DF1_DLE;
    buf[p++] = DF1_STX;
    for (size_t i = 0; i < data_len; i++)
    {
        buf[p++] = data[i];
        if (data[i] == DF1_DLE)
            buf[p++] = DF1_DLE;
    }
    buf[p++] = DF1_DLE;
    buf[p++] = DF1_ETX;

    if (check == Df1Check::DF1_CHECK_CRC)
    {
        uint8_t etx = DF1_ETX;
        uint16_t c = crc16(0x0000, data, data_len); // CRC over the data ...
        c = crc16(c, &etx, 1);                      // ... plus the ETX
        buf[p++] = (uint8_t)(c & 0xFF);             // low byte first
        buf[p++] = (uint8_t)(c >> 8);
    }
    else
    {
        buf[p++] = df1_bcc(data, data_len); // BCC excludes the ETX
    }
    return p;
}

bool df1_parse_frame(const uint8_t *buf, size_t len, Df1Check check, uint8_t *out, size_t out_cap, size_t *out_len)
{
    size_t checklen = (check == Df1Check::DF1_CHECK_CRC) ? 2 : 1;
    if (!buf || !out || len < 4 + checklen) // DLE STX DLE ETX + check
        return false;
    if (buf[0] != DF1_DLE || buf[1] != DF1_STX)
        return false;

    size_t i = 2;
    size_t o = 0;
    bool ended = false;
    while (i < len)
    {
        if (buf[i] == DF1_DLE)
        {
            if (i + 1 >= len)
                return false;
            uint8_t next = buf[i + 1];
            if (next == DF1_DLE) // doubled DLE -> one 0x10 data byte
            {
                if (o >= out_cap)
                    return false;
                out[o++] = DF1_DLE;
                i += 2;
            }
            else if (next == DF1_ETX) // end of data
            {
                i += 2;
                ended = true;
                break;
            }
            else
                return false; // an unexpected control symbol inside the data
        }
        else
        {
            if (o >= out_cap)
                return false;
            out[o++] = buf[i++];
        }
    }
    if (!ended)
        return false;

    if (check == Df1Check::DF1_CHECK_CRC)
    {
        if (i + 2 > len)
            return false;
        uint8_t etx = DF1_ETX;
        uint16_t c = crc16(0x0000, out, o);
        c = crc16(c, &etx, 1);
        uint16_t got = (uint16_t)(buf[i] | ((uint16_t)buf[i + 1] << 8)); // low byte first
        if (c != got)
            return false;
    }
    else
    {
        if (i + 1 > len)
            return false;
        if (df1_bcc(out, o) != buf[i])
            return false;
    }
    if (out_len)
        *out_len = o;
    return true;
}

#endif // DWS_ENABLE_DF1
