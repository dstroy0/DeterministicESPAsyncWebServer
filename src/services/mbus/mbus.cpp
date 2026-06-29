// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mbus.cpp
 * @brief Wired M-Bus (EN 13757) frame + data-record codec (pure, host-tested).
 */

#include "services/mbus/mbus.h"

#if DETWS_ENABLE_MBUS

#include <string.h>

// The M-Bus checksum is the 8-bit arithmetic sum of the covered octets.
static uint8_t checksum(const uint8_t *p, size_t n)
{
    uint8_t s = 0;
    for (size_t i = 0; i < n; i++)
        s = (uint8_t)(s + p[i]);
    return s;
}

size_t mbus_build_ack(uint8_t *buf, size_t cap)
{
    if (!buf || cap < 1)
        return 0;
    buf[0] = MBUS_ACK;
    return 1;
}

size_t mbus_build_short(uint8_t *buf, size_t cap, uint8_t c, uint8_t a)
{
    if (!buf || cap < 5)
        return 0;
    buf[0] = MBUS_START_SHORT;
    buf[1] = c;
    buf[2] = a;
    buf[3] = (uint8_t)(c + a); // checksum over C + A
    buf[4] = MBUS_STOP;
    return 5;
}

size_t mbus_build_long(uint8_t *buf, size_t cap, uint8_t c, uint8_t a, uint8_t ci, const uint8_t *data,
                       uint8_t data_len)
{
    if (!buf || data_len > MBUS_MAX_DATA || (data_len && !data))
        return 0;
    uint8_t L = (uint8_t)(3 + data_len); // L counts C + A + CI + user data
    size_t total = (size_t)6 + L;        // 68 L L 68 [L octets] CS 16
    if (cap < total)
        return 0;
    buf[0] = MBUS_START_LONG;
    buf[1] = L;
    buf[2] = L;
    buf[3] = MBUS_START_LONG;
    buf[4] = c;
    buf[5] = a;
    buf[6] = ci;
    if (data_len)
        memcpy(buf + 7, data, data_len);
    buf[4 + L] = checksum(buf + 4, L); // sum of C..end of user data
    buf[5 + L] = MBUS_STOP;
    return total;
}

size_t mbus_build_snd_nke(uint8_t *buf, size_t cap, uint8_t a)
{
    return mbus_build_short(buf, cap, MBUS_C_SND_NKE, a);
}

size_t mbus_build_req_ud2(uint8_t *buf, size_t cap, uint8_t a, bool fcb)
{
    return mbus_build_short(buf, cap, (uint8_t)(fcb ? 0x7Bu : MBUS_C_REQ_UD2), a);
}

bool mbus_parse(const uint8_t *buf, size_t len, MbusFrame *out, size_t *consumed)
{
    if (!buf || !out || len < 1)
        return false;
    out->type = MBUS_FRAME_NONE;
    out->c = out->a = out->ci = 0;
    out->data = nullptr;
    out->data_len = 0;

    if (buf[0] == MBUS_ACK)
    {
        out->type = MBUS_FRAME_ACK;
        if (consumed)
            *consumed = 1;
        return true;
    }
    if (buf[0] == MBUS_START_SHORT)
    {
        if (len < 5 || buf[4] != MBUS_STOP)
            return false;
        if (buf[3] != (uint8_t)(buf[1] + buf[2]))
            return false;
        out->type = MBUS_FRAME_SHORT;
        out->c = buf[1];
        out->a = buf[2];
        if (consumed)
            *consumed = 5;
        return true;
    }
    if (buf[0] == MBUS_START_LONG)
    {
        if (len < 4)
            return false;
        uint8_t L = buf[1];
        if (L < 3 || buf[2] != L || buf[3] != MBUS_START_LONG)
            return false;
        size_t total = (size_t)6 + L;
        if (len < total || buf[5 + L] != MBUS_STOP)
            return false;
        if (checksum(buf + 4, L) != buf[4 + L])
            return false;
        out->type = MBUS_FRAME_LONG;
        out->c = buf[4];
        out->a = buf[5];
        out->ci = buf[6];
        out->data_len = (uint8_t)(L - 3);
        out->data = out->data_len ? buf + 7 : nullptr;
        if (consumed)
            *consumed = total;
        return true;
    }
    return false;
}

uint8_t mbus_dif_data_len(uint8_t coding)
{
    switch (coding & 0x0Fu)
    {
    case MBUS_DIF_NONE:
        return 0;
    case MBUS_DIF_INT8:
        return 1;
    case MBUS_DIF_INT16:
        return 2;
    case MBUS_DIF_INT24:
        return 3;
    case MBUS_DIF_INT32:
        return 4;
    case MBUS_DIF_REAL32:
        return 4;
    case MBUS_DIF_INT48:
        return 6;
    case MBUS_DIF_INT64:
        return 8;
    case MBUS_DIF_READOUT:
        return 0;
    case MBUS_DIF_BCD2:
        return 1;
    case MBUS_DIF_BCD4:
        return 2;
    case MBUS_DIF_BCD6:
        return 3;
    case MBUS_DIF_BCD8:
        return 4;
    case MBUS_DIF_VARIABLE:
        return 0; // length carried in the LVAR octet
    case MBUS_DIF_BCD12:
        return 6;
    default: // MBUS_DIF_SPECIAL
        return 0;
    }
}

bool mbus_record_next(const uint8_t *body, size_t len, size_t *pos, MbusRecord *out)
{
    if (!body || !pos || !out || *pos >= len)
        return false;
    size_t p = *pos;
    uint8_t dif = body[p++];
    uint8_t coding = (uint8_t)(dif & 0x0Fu);

    // Skip the DIFE extension chain (each DIFE's high bit flags another).
    uint8_t ext = dif;
    while (ext & 0x80u)
    {
        if (p >= len)
            return false;
        ext = body[p++];
    }

    out->dif = dif;
    out->coding = coding;
    out->vif = 0;
    out->data = nullptr;
    out->data_len = 0;

    if (coding == MBUS_DIF_SPECIAL) // manufacturer-specific / idle: no VIF, no fixed data
    {
        *pos = p;
        return true;
    }

    // VIF (mandatory) + its VIFE extension chain.
    if (p >= len)
        return false;
    out->vif = body[p++];
    ext = out->vif;
    while (ext & 0x80u)
    {
        if (p >= len)
            return false;
        ext = body[p++];
    }

    uint8_t dlen;
    if (coding == MBUS_DIF_VARIABLE)
    {
        if (p >= len)
            return false;
        uint8_t lvar = body[p++];
        if (lvar > 0xBFu)
            return false; // only the LVAR raw/ASCII form (0x00..0xBF) is supported
        dlen = lvar;
    }
    else
    {
        dlen = mbus_dif_data_len(coding);
    }

    if (p + dlen > len)
        return false;
    out->data = dlen ? body + p : nullptr;
    out->data_len = dlen;
    *pos = p + dlen;
    return true;
}

#endif // DETWS_ENABLE_MBUS
