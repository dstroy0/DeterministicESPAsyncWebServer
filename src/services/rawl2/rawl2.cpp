// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file rawl2.cpp
 * @brief Raw Layer-2 Ethernet frame codec (see rawl2.h).
 */

#include "services/rawl2/rawl2.h"

#if DETWS_ENABLE_RAWL2

#include <string.h>

size_t detws_eth_build(const uint8_t *dst, const uint8_t *src, uint16_t ethertype, const uint8_t *payload,
                       size_t payload_len, uint8_t *out, size_t cap)
{
    if (!dst || !src || !out || (payload_len && !payload))
        return 0;
    size_t n = RawL2::ETH_HDR_LEN + payload_len;
    if (n > cap)
        return 0;
    memcpy(out, dst, RawL2::ETH_ALEN);
    memcpy(out + RawL2::ETH_ALEN, src, RawL2::ETH_ALEN);
    out[12] = (uint8_t)(ethertype >> 8);
    out[13] = (uint8_t)ethertype;
    if (payload_len)
        memcpy(out + RawL2::ETH_HDR_LEN, payload, payload_len);
    return n;
}

size_t detws_eth_build_vlan(const uint8_t *dst, const uint8_t *src, uint8_t pcp, bool dei, uint16_t vid,
                            uint16_t ethertype, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t cap)
{
    if (!dst || !src || !out || (payload_len && !payload))
        return 0;
    size_t n = RawL2::ETH_VLAN_HDR_LEN + payload_len;
    if (n > cap)
        return 0;
    memcpy(out, dst, RawL2::ETH_ALEN);
    memcpy(out + RawL2::ETH_ALEN, src, RawL2::ETH_ALEN);
    out[12] = (uint8_t)(RawL2::ETH_TPID_8021Q >> 8);
    out[13] = (uint8_t)RawL2::ETH_TPID_8021Q;
    uint16_t tci = (uint16_t)(((pcp & 0x7) << 13) | ((dei ? 1 : 0) << 12) | (vid & 0x0FFF));
    out[14] = (uint8_t)(tci >> 8);
    out[15] = (uint8_t)tci;
    out[16] = (uint8_t)(ethertype >> 8);
    out[17] = (uint8_t)ethertype;
    if (payload_len)
        memcpy(out + RawL2::ETH_VLAN_HDR_LEN, payload, payload_len);
    return n;
}

bool detws_eth_parse(const uint8_t *frame, size_t len, EthFrame *out)
{
    if (!frame || !out || len < RawL2::ETH_HDR_LEN)
        return false;
    out->dst = frame;
    out->src = frame + RawL2::ETH_ALEN;
    uint16_t et = (uint16_t)((frame[12] << 8) | frame[13]);
    if (et == RawL2::ETH_TPID_8021Q)
    {
        if (len < RawL2::ETH_VLAN_HDR_LEN)
            return false;
        uint16_t tci = (uint16_t)((frame[14] << 8) | frame[15]);
        out->vlan = true;
        out->pcp = (uint8_t)((tci >> 13) & 0x7);
        out->vid = (uint16_t)(tci & 0x0FFF);
        out->ethertype = (uint16_t)((frame[16] << 8) | frame[17]);
        out->payload = frame + RawL2::ETH_VLAN_HDR_LEN;
        out->payload_len = len - RawL2::ETH_VLAN_HDR_LEN;
    }
    else
    {
        out->vlan = false;
        out->pcp = 0;
        out->vid = 0;
        out->ethertype = et;
        out->payload = frame + RawL2::ETH_HDR_LEN;
        out->payload_len = len - RawL2::ETH_HDR_LEN;
    }
    return true;
}

uint32_t detws_eth_fcs(const uint8_t *bytes, size_t len)
{
    // CRC-32/ISO-HDLC (the Ethernet FCS): reflected poly 0xEDB88320, init/xorout 0xFFFFFFFF.
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= bytes[i];
        for (int b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
    }
    return ~crc;
}

#endif // DETWS_ENABLE_RAWL2
