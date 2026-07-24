// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file bacnet.cpp
 * @brief BACnet/IP BVLC + NPDU builder + parser (pure, host-tested).
 */

#include "services/bacnet/bacnet.h"

#if DWS_ENABLE_BACNET

#include <string.h>

size_t dws_bvlc_build(uint8_t *buf, size_t cap, uint8_t function, const uint8_t *npdu, size_t dws_npdu_len)
{
    if (!buf || (dws_npdu_len && !npdu))
        return 0;
    size_t total = BVLC_HEADER_SIZE + dws_npdu_len;
    if (total > 0xFFFF || total > cap)
        return 0;
    buf[0] = BVLC_TYPE_BIP;
    buf[1] = function;
    buf[2] = (uint8_t)(total >> 8); // length, big-endian, the whole BVLL
    buf[3] = (uint8_t)(total & 0xFF);
    if (dws_npdu_len)
        memcpy(buf + BVLC_HEADER_SIZE, npdu, dws_npdu_len);
    return total;
}

bool dws_bvlc_parse(const uint8_t *buf, size_t len, uint8_t *function, const uint8_t **npdu, size_t *dws_npdu_len)
{
    if (!buf || len < BVLC_HEADER_SIZE || buf[0] != BVLC_TYPE_BIP)
        return false;
    size_t total = ((size_t)buf[2] << 8) | buf[3];
    if (total < BVLC_HEADER_SIZE || total > len)
        return false;
    if (function)
        *function = buf[1];
    if (npdu)
        *npdu = buf + BVLC_HEADER_SIZE;
    if (dws_npdu_len)
        *dws_npdu_len = total - BVLC_HEADER_SIZE;
    return true;
}

size_t dws_npdu_build(uint8_t *buf, size_t cap, bool expecting_reply, uint8_t priority, bool has_dest, uint16_t dnet,
                      const uint8_t *dadr, uint8_t dadr_len, uint8_t hop_count, const uint8_t *apdu, size_t apdu_len)
{
    if (!buf || (apdu_len && !apdu) || (dadr_len && !dadr))
        return 0;
    size_t need = 2 + apdu_len; // version + control + apdu
    if (has_dest)
        need += 2 + 1 + dadr_len + 1; // DNET + DLEN + DADR + hop count
    if (need > cap)
        return 0;

    size_t p = 0;
    buf[p++] = NPDU_VERSION;
    uint8_t control = (uint8_t)(priority & NPCI_PRIORITY_MASK);
    if (expecting_reply)
        control |= NPCI_EXPECTING_REPLY;
    if (has_dest)
        control |= NPCI_DEST_PRESENT;
    buf[p++] = control;
    if (has_dest)
    {
        buf[p++] = (uint8_t)(dnet >> 8);
        buf[p++] = (uint8_t)(dnet & 0xFF);
        buf[p++] = dadr_len;
        if (dadr_len)
        {
            memcpy(buf + p, dadr, dadr_len);
            p += dadr_len;
        }
        buf[p++] = hop_count; // follows the (absent) source fields when a destination is present
    }
    if (apdu_len)
    {
        memcpy(buf + p, apdu, apdu_len);
        p += apdu_len;
    }
    return p;
}

bool dws_npdu_parse(const uint8_t *buf, size_t len, NpduInfo *out)
{
    if (!buf || !out || len < 2 || buf[0] != NPDU_VERSION)
        return false;
    uint8_t control = buf[1];
    size_t p = 2;

    out->control = control;
    out->network_message = (control & NPCI_NETWORK_MSG) != 0;
    out->dest_present = (control & NPCI_DEST_PRESENT) != 0;
    out->src_present = (control & NPCI_SRC_PRESENT) != 0;
    out->dnet = 0;
    out->snet = 0;
    out->hop_count = 0;

    if (out->dest_present)
    {
        if (p + 3 > len)
            return false;
        out->dnet = (uint16_t)((buf[p] << 8) | buf[p + 1]);
        uint8_t dlen = buf[p + 2];
        p += 3 + dlen;
        if (p > len)
            return false;
    }
    if (out->src_present)
    {
        if (p + 3 > len)
            return false;
        out->snet = (uint16_t)((buf[p] << 8) | buf[p + 1]);
        uint8_t slen = buf[p + 2];
        p += 3 + slen;
        if (p > len)
            return false;
    }
    if (out->dest_present) // the hop count follows the source fields
    {
        if (p + 1 > len)
            return false;
        out->hop_count = buf[p++];
    }
    out->apdu = buf + p;
    out->apdu_len = len - p;
    return true;
}

bool dws_apdu_parse(const uint8_t *apdu, size_t len, BacnetApdu *out)
{
    if (!apdu || !out || len < 1)
        return false;
    memset(out, 0, sizeof(*out));
    out->pdu_type = (uint8_t)(apdu[0] >> 4);
    size_t p = 1;
    switch (out->pdu_type)
    {
    case BACNET_PDU_CONFIRMED_REQUEST:
        out->segmented = (apdu[0] & BACNET_APDU_SEG) != 0;
        out->more_follows = (apdu[0] & BACNET_APDU_MOR) != 0;
        out->sa = (apdu[0] & BACNET_APDU_SA) != 0;
        if (len < p + 2) // max-segs/max-apdu octet + invoke id
            return false;
        out->invoke_id = apdu[p + 1]; // apdu[1] is max segments / max APDU, apdu[2] is the invoke id
        p += 2;
        if (out->segmented) // a segmented request carries a sequence number + proposed window size
        {
            if (len < p + 2)
                return false;
            p += 2;
        }
        if (len < p + 1)
            return false;
        out->service_choice = apdu[p++];
        break;
    case BACNET_PDU_UNCONFIRMED_REQUEST:
        if (len < p + 1)
            return false;
        out->service_choice = apdu[p++];
        break;
    case BACNET_PDU_SIMPLE_ACK:
        if (len < p + 2) // invoke id + service-ACK choice
            return false;
        out->invoke_id = apdu[p++];
        out->service_choice = apdu[p++];
        break;
    case BACNET_PDU_COMPLEX_ACK:
        out->segmented = (apdu[0] & BACNET_APDU_SEG) != 0;
        out->more_follows = (apdu[0] & BACNET_APDU_MOR) != 0;
        if (len < p + 1) // invoke id
            return false;
        out->invoke_id = apdu[p++];
        if (out->segmented)
        {
            if (len < p + 2)
                return false;
            p += 2;
        }
        if (len < p + 1)
            return false;
        out->service_choice = apdu[p++];
        break;
    default:
        return false; // segment-ack / error / reject / abort are not decoded here
    }
    out->service_data = (p < len) ? apdu + p : nullptr;
    out->service_data_len = len - p;
    return true;
}

#endif // DWS_ENABLE_BACNET
