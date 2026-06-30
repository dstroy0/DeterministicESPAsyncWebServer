// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dmx.cpp
 * @brief DMX512 + RDM (ANSI E1.20) codec (pure, host-tested).
 */

#include "services/dmx/dmx.h"

#if DETWS_ENABLE_DMX

size_t dmx_build(uint8_t *buf, size_t cap, uint8_t start_code, const uint8_t *channels, uint16_t n)
{
    if (!buf || n > DMX_MAX_CHANNELS || (n && !channels))
        return 0;
    size_t total = (size_t)1 + n;
    if (cap < total)
        return 0;
    buf[0] = start_code;
    if (n)
        memcpy(buf + 1, channels, n);
    return total;
}

uint8_t dmx_get_channel(const uint8_t *buf, size_t len, uint16_t ch)
{
    if (!buf || ch < 1 || ch > DMX_MAX_CHANNELS || (size_t)ch >= len)
        return 0; // slot ch lives at buf[ch] (buf[0] is the start code)
    return buf[ch];
}

uint64_t rdm_uid(uint16_t manufacturer, uint32_t device)
{
    return ((uint64_t)manufacturer << 32) | device;
}

uint16_t rdm_checksum(const uint8_t *buf, size_t len)
{
    uint16_t s = 0;
    for (size_t i = 0; i < len; i++)
        s = (uint16_t)(s + buf[i]);
    return s;
}

// Write a 48-bit UID big-endian (manufacturer high).
static void put_uid(uint8_t *p, uint64_t uid)
{
    p[0] = (uint8_t)(uid >> 40);
    p[1] = (uint8_t)(uid >> 32);
    p[2] = (uint8_t)(uid >> 24);
    p[3] = (uint8_t)(uid >> 16);
    p[4] = (uint8_t)(uid >> 8);
    p[5] = (uint8_t)uid;
}

static uint64_t get_uid(const uint8_t *p)
{
    return ((uint64_t)p[0] << 40) | ((uint64_t)p[1] << 32) | ((uint64_t)p[2] << 24) | ((uint64_t)p[3] << 16) |
           ((uint64_t)p[4] << 8) | (uint64_t)p[5];
}

size_t rdm_build(uint8_t *buf, size_t cap, const RdmPacket *p, const uint8_t *pdata, uint8_t pdl)
{
    if (!buf || !p || (pdl && !pdata))
        return 0;
    uint8_t ml = (uint8_t)(24 + pdl); // message length: SC..end of parameter data (excludes checksum)
    size_t total = (size_t)ml + 2;
    if (cap < total)
        return 0;
    buf[0] = RDM_SC;
    buf[1] = RDM_SUB_SC;
    buf[2] = ml;
    put_uid(buf + 3, p->dest_uid);
    put_uid(buf + 9, p->src_uid);
    buf[15] = p->tn;
    buf[16] = p->port_id;
    buf[17] = p->msg_count;
    buf[18] = (uint8_t)(p->sub_device >> 8); // sub-device, big-endian
    buf[19] = (uint8_t)p->sub_device;
    buf[20] = p->cc;
    buf[21] = (uint8_t)(p->pid >> 8); // PID, big-endian
    buf[22] = (uint8_t)p->pid;
    buf[23] = pdl;
    if (pdl)
        memcpy(buf + 24, pdata, pdl);
    uint16_t cs = rdm_checksum(buf, ml); // checksum over SC..end of parameter data
    buf[ml] = (uint8_t)(cs >> 8);
    buf[ml + 1] = (uint8_t)cs;
    return total;
}

bool rdm_parse(const uint8_t *buf, size_t len, RdmPacket *out, size_t *consumed)
{
    if (!buf || !out || len < RDM_OVERHEAD)
        return false;
    if (buf[0] != RDM_SC || buf[1] != RDM_SUB_SC)
        return false;
    uint8_t ml = buf[2];
    if (ml < 24)
        return false;
    uint8_t pdl = buf[23];
    if (ml != (uint8_t)(24 + pdl))
        return false; // message length must match the declared PDL
    size_t total = (size_t)ml + 2;
    if (len < total)
        return false;
    uint16_t cs = (uint16_t)((buf[ml] << 8) | buf[ml + 1]);
    if (cs != rdm_checksum(buf, ml))
        return false;

    out->dest_uid = get_uid(buf + 3);
    out->src_uid = get_uid(buf + 9);
    out->tn = buf[15];
    out->port_id = buf[16];
    out->msg_count = buf[17];
    out->sub_device = (uint16_t)((buf[18] << 8) | buf[19]);
    out->cc = buf[20];
    out->pid = (uint16_t)((buf[21] << 8) | buf[22]);
    out->pdl = pdl;
    out->pdata = pdl ? buf + 24 : nullptr;
    if (consumed)
        *consumed = total;
    return true;
}

#endif // DETWS_ENABLE_DMX
