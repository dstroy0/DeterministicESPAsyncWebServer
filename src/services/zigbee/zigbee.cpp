// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file zigbee.cpp
 * @brief Zigbee EZSP / ASH framing codec - implementation.
 *
 * ASH (UG101): [control | payload | CRC16] byte-stuffed and Flag-terminated. The CRC is
 * CRC-16/CCITT (poly 0x1021, init 0xFFFF); the reserved bytes 0x7E / 0x7D / 0x11 / 0x13 /
 * 0x18 / 0x1A are escaped as 0x7D, (byte XOR 0x20).
 */

#include "services/zigbee/zigbee.h"

#if DETWS_ENABLE_ZIGBEE

namespace
{
bool is_reserved(uint8_t b)
{
    return b == 0x7E || b == 0x7D || b == 0x11 || b == 0x13 || b == 0x18 || b == 0x1A;
}

uint16_t crc16_byte(uint16_t crc, uint8_t b)
{
    crc ^= (uint16_t)b << 8;
    for (uint8_t i = 0; i < 8; i++)
        crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    return crc;
}

// Append a byte to out with ASH stuffing; return false if it would overflow cap.
bool put_stuffed(uint8_t *out, uint16_t *p, uint16_t cap, uint8_t b)
{
    if (is_reserved(b))
    {
        if (*p + 2 > cap)
            return false;
        out[(*p)++] = ASH_ESCAPE;
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

uint16_t ash_crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
        crc = crc16_byte(crc, buf[i]);
    return crc;
}

uint16_t ash_frame_encode(uint8_t control, const uint8_t *payload, uint16_t len, uint8_t *out, uint16_t cap)
{
    if (!out || len > DETWS_ZIGBEE_MAX_DATA || (payload == nullptr && len > 0))
        return 0;
    // CRC over control + payload.
    uint16_t crc = crc16_byte(0xFFFF, control);
    for (uint16_t i = 0; i < len; i++)
        crc = crc16_byte(crc, payload[i]);

    uint16_t p = 0;
    if (!put_stuffed(out, &p, cap, control))
        return 0;
    for (uint16_t i = 0; i < len; i++)
        if (!put_stuffed(out, &p, cap, payload[i]))
            return 0;
    if (!put_stuffed(out, &p, cap, (uint8_t)(crc >> 8)) || !put_stuffed(out, &p, cap, (uint8_t)(crc & 0xFF)))
        return 0;
    if (p + 1 > cap)
        return 0;
    out[p++] = ASH_FLAG; // the delimiter is never stuffed
    return p;
}

int ash_frame_decode(const uint8_t *raw, uint16_t len, uint8_t *control, uint8_t *payload, uint16_t pay_cap,
                     uint16_t *pay_len)
{
    if (!raw)
        return 0;
    // Find the frame delimiter.
    uint16_t flag = 0;
    while (flag < len && raw[flag] != ASH_FLAG)
        flag++;
    if (flag >= len)
        return 0; // no complete frame yet

    // Remove the byte-stuffing from raw[0, flag) into a fixed scratch: control + payload + CRC(2).
    uint8_t un[DETWS_ZIGBEE_MAX_DATA + 3];
    uint16_t n = 0;
    for (uint16_t i = 0; i < flag; i++)
    {
        uint8_t b = raw[i];
        if (b == ASH_ESCAPE)
        {
            if (++i >= flag)
                return -1; // dangling escape
            b = (uint8_t)(raw[i] ^ 0x20);
        }
        if (n >= sizeof(un))
            return -1; // frame longer than we accept
        un[n++] = b;
    }
    if (n < 3)
        return -1; // need at least control + CRC(2)
    uint16_t body = (uint16_t)(n - 2);
    uint16_t crc = ash_crc16(un, body);
    if ((uint16_t)((un[n - 2] << 8) | un[n - 1]) != crc)
        return -1; // CRC mismatch

    uint16_t plen = (uint16_t)(body - 1); // minus the control byte
    if (plen > pay_cap)
        return -1; // caller buffer too small
    if (control)
        *control = un[0];
    for (uint16_t i = 0; i < plen; i++)
        payload[i] = un[1 + i];
    if (pay_len)
        *pay_len = plen;
    return (int)(flag + 1); // consume up to and including the flag
}

#endif // DETWS_ENABLE_ZIGBEE
