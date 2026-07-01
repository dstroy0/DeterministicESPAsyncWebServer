// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file iec60870.cpp
 * @brief IEC 60870-5-101 / -104 telecontrol codec (pure, host-tested).
 */

#include "services/iec60870/iec60870.h"

#if DETWS_ENABLE_IEC60870

#include "shared_primitives/shim.h"

// --- -104 APCI ---

size_t iec104_build_i(uint8_t *buf, size_t cap, uint16_t ns, uint16_t nr, const uint8_t *asdu, size_t asdu_len)
{
    if (!buf || (asdu_len && !asdu) || asdu_len > 249) // APDU length octet maxes at 253 (= 4 + 249)
        return 0;
    size_t total = IEC104_APCI_LEN + asdu_len;
    if (cap < total)
        return 0;
    buf[0] = IEC_START_104;
    buf[1] = (uint8_t)(4 + asdu_len);      // APDU length: 4 control octets + ASDU
    buf[2] = (uint8_t)((ns << 1) & 0xFEu); // I-format: bit0 of octet 1 is 0
    buf[3] = (uint8_t)((ns >> 7) & 0xFFu);
    buf[4] = (uint8_t)((nr << 1) & 0xFEu);
    buf[5] = (uint8_t)((nr >> 7) & 0xFFu);
    if (asdu_len)
        memcpy(buf + 6, asdu, asdu_len);
    return total;
}

size_t iec104_build_s(uint8_t *buf, size_t cap, uint16_t nr)
{
    if (!buf || cap < IEC104_APCI_LEN)
        return 0;
    buf[0] = IEC_START_104;
    buf[1] = 4;
    buf[2] = 0x01; // S-format
    buf[3] = 0x00;
    buf[4] = (uint8_t)((nr << 1) & 0xFEu);
    buf[5] = (uint8_t)((nr >> 7) & 0xFFu);
    return IEC104_APCI_LEN;
}

size_t iec104_build_u(uint8_t *buf, size_t cap, uint8_t u_cmd)
{
    if (!buf || cap < IEC104_APCI_LEN)
        return 0;
    buf[0] = IEC_START_104;
    buf[1] = 4;
    buf[2] = u_cmd; // U-format: bits 0-1 are 11 in every defined command
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    return IEC104_APCI_LEN;
}

bool iec104_parse(const uint8_t *buf, size_t len, Iec104Apci *out, size_t *consumed)
{
    if (!buf || !out || len < 2 || buf[0] != IEC_START_104)
        return false;
    uint8_t L = buf[1];
    if (L < 4)
        return false;
    size_t total = (size_t)2 + L;
    if (len < total)
        return false;
    uint8_t c0 = buf[2];
    out->ns = out->nr = 0;
    out->u_cmd = 0;
    out->asdu = nullptr;
    out->asdu_len = 0;
    if ((c0 & 0x01u) == 0) // I-format
    {
        out->format = IEC104_I;
        out->ns = (uint16_t)((buf[2] >> 1) | ((uint16_t)buf[3] << 7));
        out->nr = (uint16_t)((buf[4] >> 1) | ((uint16_t)buf[5] << 7));
        out->asdu_len = (size_t)(L - 4);
        out->asdu = out->asdu_len ? buf + 6 : nullptr;
    }
    else if ((c0 & 0x03u) == 0x01u) // S-format
    {
        out->format = IEC104_S;
        out->nr = (uint16_t)((buf[4] >> 1) | ((uint16_t)buf[5] << 7));
    }
    else // U-format (bits 0-1 == 11)
    {
        out->format = IEC104_U;
        out->u_cmd = c0;
    }
    if (consumed)
        *consumed = total;
    return true;
}

// --- ASDU header + IOA ---

size_t iec_asdu_build_header(uint8_t *buf, size_t cap, const IecAsduHeader *h)
{
    if (!buf || !h || cap < 6)
        return 0;
    buf[0] = h->type_id;
    buf[1] = (uint8_t)((h->sq ? 0x80u : 0u) | (h->count & 0x7Fu)); // variable structure qualifier
    buf[2] = (uint8_t)((h->test ? 0x80u : 0u) | (h->negative ? 0x40u : 0u) | (h->cot & 0x3Fu));
    buf[3] = h->orig_addr;
    buf[4] = (uint8_t)(h->common_addr & 0xFFu); // common address, little-endian
    buf[5] = (uint8_t)((h->common_addr >> 8) & 0xFFu);
    return 6;
}

bool iec_asdu_parse_header(const uint8_t *buf, size_t len, IecAsduHeader *out, size_t *consumed)
{
    if (!buf || !out || len < 6)
        return false;
    out->type_id = buf[0];
    out->sq = (buf[1] & 0x80u) != 0;
    out->count = (uint8_t)(buf[1] & 0x7Fu);
    out->test = (buf[2] & 0x80u) != 0;
    out->negative = (buf[2] & 0x40u) != 0;
    out->cot = (uint8_t)(buf[2] & 0x3Fu);
    out->orig_addr = buf[3];
    out->common_addr = (uint16_t)(buf[4] | ((uint16_t)buf[5] << 8));
    if (consumed)
        *consumed = 6;
    return true;
}

size_t iec_put_ioa(uint8_t *buf, size_t cap, uint32_t ioa)
{
    if (!buf || cap < 3)
        return 0;
    buf[0] = (uint8_t)(ioa & 0xFFu);
    buf[1] = (uint8_t)((ioa >> 8) & 0xFFu);
    buf[2] = (uint8_t)((ioa >> 16) & 0xFFu);
    return 3;
}

uint32_t iec_get_ioa(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

// --- -101 FT1.2 link frames ---

static uint8_t sum8(const uint8_t *p, size_t n)
{
    uint8_t s = 0;
    for (size_t i = 0; i < n; i++)
        s = (uint8_t)(s + p[i]);
    return s;
}

size_t iec101_build_fixed(uint8_t *buf, size_t cap, uint8_t control, uint8_t addr)
{
    if (!buf || cap < 5)
        return 0;
    buf[0] = IEC_START_FIXED;
    buf[1] = control;
    buf[2] = addr;
    buf[3] = (uint8_t)(control + addr); // checksum over control + address
    buf[4] = IEC_STOP;
    return 5;
}

size_t iec101_build_variable(uint8_t *buf, size_t cap, uint8_t control, uint8_t addr, const uint8_t *asdu,
                             uint8_t asdu_len)
{
    if (!buf || (asdu_len && !asdu) || asdu_len > 253)
        return 0;
    uint8_t L = (uint8_t)(2 + asdu_len); // L counts control + address + ASDU
    size_t total = (size_t)6 + L;
    if (cap < total)
        return 0;
    buf[0] = IEC_START_104; // 0x68
    buf[1] = L;
    buf[2] = L;
    buf[3] = IEC_START_104;
    buf[4] = control;
    buf[5] = addr;
    if (asdu_len)
        memcpy(buf + 6, asdu, asdu_len);
    buf[4 + L] = sum8(buf + 4, L); // checksum over control..end of ASDU
    buf[5 + L] = IEC_STOP;
    return total;
}

bool iec101_parse(const uint8_t *buf, size_t len, Iec101Frame *out, size_t *consumed)
{
    if (!buf || !out || len < 1)
        return false;
    out->fixed = false;
    out->control = out->addr = 0;
    out->asdu = nullptr;
    out->asdu_len = 0;

    if (buf[0] == IEC_START_FIXED)
    {
        if (len < 5 || buf[4] != IEC_STOP || buf[3] != (uint8_t)(buf[1] + buf[2]))
            return false;
        out->fixed = true;
        out->control = buf[1];
        out->addr = buf[2];
        if (consumed)
            *consumed = 5;
        return true;
    }
    if (buf[0] == IEC_START_104)
    {
        if (len < 4)
            return false;
        uint8_t L = buf[1];
        if (L < 2 || buf[2] != L || buf[3] != IEC_START_104)
            return false;
        size_t total = (size_t)6 + L;
        if (len < total || buf[5 + L] != IEC_STOP || sum8(buf + 4, L) != buf[4 + L])
            return false;
        out->control = buf[4];
        out->addr = buf[5];
        out->asdu_len = (uint8_t)(L - 2);
        out->asdu = out->asdu_len ? buf + 6 : nullptr;
        if (consumed)
            *consumed = total;
        return true;
    }
    return false;
}

#endif // DETWS_ENABLE_IEC60870
