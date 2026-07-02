// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file thread.cpp
 * @brief Thread spinel / HDLC-lite framing codec - implementation.
 *
 * HDLC-lite: [payload | FCS(lo,hi)] byte-stuffed and Flag-terminated. The FCS is CRC-16/X-25
 * (poly 0x1021 reflected = 0x8408, init 0xFFFF, reflected, final XOR 0xFFFF); the reserved
 * bytes 0x7E / 0x7D / 0x11 / 0x13 are escaped as 0x7D, (byte XOR 0x20).
 */

#include "services/thread/thread.h"

#if DETWS_ENABLE_THREAD

namespace
{
bool is_reserved(uint8_t b)
{
    return b == 0x7E || b == 0x7D || b == 0x11 || b == 0x13;
}

// Append a byte with HDLC stuffing; return false if it would overflow cap.
bool put_stuffed(uint8_t *out, uint16_t *p, uint16_t cap, uint8_t b)
{
    if (is_reserved(b))
    {
        if (*p + 2 > cap)
            return false;
        out[(*p)++] = HDLC_ESCAPE;
        out[(*p)++] = (uint8_t)(b ^ 0x20);
    }
    else
    {
        if (*p + 1 > cap)
            return false;
        out[(*p)++] = b;
    }
    return true;
}
} // namespace

uint16_t spinel_fcs(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0x8408) : (uint16_t)(crc >> 1);
    }
    return (uint16_t)(crc ^ 0xFFFF);
}

uint16_t spinel_frame_encode(const uint8_t *payload, uint16_t len, uint8_t *out, uint16_t cap)
{
    if (!out || len > DETWS_THREAD_MAX_DATA || (payload == nullptr && len > 0))
        return 0;
    uint16_t fcs = spinel_fcs(payload, len);
    uint16_t p = 0;
    for (uint16_t i = 0; i < len; i++)
        if (!put_stuffed(out, &p, cap, payload[i]))
            return 0;
    if (!put_stuffed(out, &p, cap, (uint8_t)(fcs & 0xFF)) || // FCS low byte first
        !put_stuffed(out, &p, cap, (uint8_t)(fcs >> 8)))
        return 0;
    if (p + 1 > cap)
        return 0;
    out[p++] = HDLC_FLAG;
    return p;
}

int spinel_frame_decode(const uint8_t *raw, uint16_t len, uint8_t *payload, uint16_t pay_cap, uint16_t *pay_len)
{
    if (!raw)
        return 0;
    uint16_t flag = 0;
    while (flag < len && raw[flag] != HDLC_FLAG)
        flag++;
    if (flag >= len)
        return 0; // no complete frame yet

    // Remove the byte-stuffing from raw[0, flag) into a scratch: payload + FCS(2).
    uint8_t un[DETWS_THREAD_MAX_DATA + 2];
    uint16_t n = 0;
    for (uint16_t i = 0; i < flag; i++)
    {
        uint8_t b = raw[i];
        if (b == HDLC_ESCAPE)
        {
            if (++i >= flag)
                return -1; // dangling escape
            b = (uint8_t)(raw[i] ^ 0x20);
        }
        if (n >= sizeof(un))
            return -1;
        un[n++] = b;
    }
    if (n < 2)
        return -1; // need at least the FCS
    uint16_t plen = (uint16_t)(n - 2);
    uint16_t fcs = spinel_fcs(un, plen);
    if ((uint16_t)(un[plen] | (un[plen + 1] << 8)) != fcs)
        return -1; // FCS mismatch (transmitted low byte first)
    if (plen > pay_cap)
        return -1;
    for (uint16_t i = 0; i < plen; i++)
        payload[i] = un[i];
    if (pay_len)
        *pay_len = plen;
    return (int)(flag + 1);
}

#endif // DETWS_ENABLE_THREAD
