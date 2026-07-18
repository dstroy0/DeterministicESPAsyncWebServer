// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file devicenet.cpp
 * @brief DeviceNet link-adaptation codec (pure, host-tested).
 */

#include "services/devicenet/devicenet.h"

#if DWS_ENABLE_DEVICENET

#include <string.h>

bool dws_devicenet_encode_id(uint32_t *id, DeviceNetGroup group, uint8_t msg_id, uint8_t mac_id)
{
    if (!id || mac_id > DEVICENET_MAC_MASK)
        return false;
    switch (group)
    {
    case DeviceNetGroup::DEVICENET_GROUP_1:
        if (msg_id > 0x0Fu)
            return false;
        *id = DEVICENET_G1_BASE | ((uint32_t)msg_id << 6) | mac_id;
        return true;
    case DeviceNetGroup::DEVICENET_GROUP_2:
        if (msg_id > 0x07u)
            return false;
        *id = DEVICENET_G2_BASE | ((uint32_t)mac_id << 3) | msg_id;
        return true;
    case DeviceNetGroup::DEVICENET_GROUP_3:
        if (msg_id > 0x07u)
            return false;
        *id = DEVICENET_G3_BASE | ((uint32_t)msg_id << 6) | mac_id;
        return true;
    case DeviceNetGroup::DEVICENET_GROUP_4:
        if (msg_id > 0x2Fu) // Group 4 has no MAC id; message ids 0x00..0x2F
            return false;
        *id = DEVICENET_G4_BASE | msg_id;
        return true;
    default:
        return false;
    }
}

bool dws_devicenet_decode_id(uint32_t can_id, DeviceNetId *out)
{
    if (!out)
        return false;
    uint32_t id = can_id & DWS_CAN_STD_ID_MASK;
    if (id < DEVICENET_G2_BASE) // Group 1: 0 MsgID(4) MAC(6)
    {
        out->group = DeviceNetGroup::DEVICENET_GROUP_1;
        out->msg_id = (uint8_t)((id >> 6) & 0x0Fu);
        out->mac_id = (uint8_t)(id & DEVICENET_MAC_MASK);
        return true;
    }
    if (id < DEVICENET_G3_BASE) // Group 2: 10 MAC(6) MsgID(3)
    {
        out->group = DeviceNetGroup::DEVICENET_GROUP_2;
        out->mac_id = (uint8_t)((id >> 3) & DEVICENET_MAC_MASK);
        out->msg_id = (uint8_t)(id & 0x07u);
        return true;
    }
    if (id < DEVICENET_G4_BASE) // Group 3: 11 MsgID(3) MAC(6)
    {
        out->group = DeviceNetGroup::DEVICENET_GROUP_3;
        out->msg_id = (uint8_t)((id >> 6) & 0x07u);
        out->mac_id = (uint8_t)(id & DEVICENET_MAC_MASK);
        return true;
    }
    if (id <= 0x7EFu) // Group 4: 11111 MsgID(6)
    {
        out->group = DeviceNetGroup::DEVICENET_GROUP_4;
        out->msg_id = (uint8_t)(id & 0x3Fu);
        out->mac_id = 0;
        return true;
    }
    return false; // 0x7F0..0x7FF are invalid identifiers
}

uint8_t dws_devicenet_msg_header(bool frag, bool xid, uint8_t mac_id)
{
    return (uint8_t)((frag ? DEVICENET_HDR_FRAG : 0u) | (xid ? DEVICENET_HDR_XID : 0u) | (mac_id & DEVICENET_MAC_MASK));
}

uint8_t dws_devicenet_frag_octet(uint8_t type, uint8_t count)
{
    return (uint8_t)((type & DEVICENET_FRAG_TYPE_MASK) | (count & DEVICENET_FRAG_COUNT_MASK));
}

bool dws_devicenet_build_explicit(CanFrame *out, DeviceNetGroup group, uint8_t msg_id, uint8_t mac_id,
                                  const uint8_t *body, uint8_t body_len)
{
    if (!out || body_len > 7 || (body_len && !body)) // 1 header octet + up to 7 body octets
        return false;
    uint32_t id;
    if (!dws_devicenet_encode_id(&id, group, msg_id, mac_id))
        return false;
    out->id = id;
    out->extended = false;
    out->rtr = false;
    out->dlc = (uint8_t)(1 + body_len);
    memset(out->data, 0, sizeof(out->data));
    out->data[0] = dws_devicenet_msg_header(false, false, mac_id); // not fragmented
    if (body_len)
        memcpy(out->data + 1, body, body_len);
    return true;
}

void dws_devicenet_frag_reset(DeviceNetFragRx *rx)
{
    if (rx)
        memset(rx, 0, sizeof(*rx));
}

// Append @p n octets to the reassembly buffer; false if it would overflow.
static bool frag_append(DeviceNetFragRx *rx, const uint8_t *p, uint8_t n)
{
    if ((uint32_t)rx->len + n > DWS_DEVICENET_MSG_MAX)
        return false;
    memcpy(rx->buf + rx->len, p, n);
    rx->len = (uint16_t)(rx->len + n);
    return true;
}

DeviceNetFragResult dws_devicenet_frag_feed(DeviceNetFragRx *rx, const uint8_t *body, uint8_t body_len)
{
    if (!rx || !body || body_len < 1)
        return DeviceNetFragResult::DEVICENET_FRAG_IGNORED;

    if (!(body[0] & DEVICENET_HDR_FRAG)) // a complete, non-fragmented message in one frame
    {
        dws_devicenet_frag_reset(rx);
        if (body_len > 1 && !frag_append(rx, body + 1, (uint8_t)(body_len - 1)))
            return DeviceNetFragResult::DEVICENET_FRAG_ERR; // GCOVR_EXCL_LINE  unreachable: reset()->len 0, then a
                                                            // single append of <=254 (uint8 body_len-1) < 256=MSG_MAX
        return DeviceNetFragResult::DEVICENET_FRAG_COMPLETE;
    }
    if (body_len < 2)
        return DeviceNetFragResult::DEVICENET_FRAG_ERR; // FRAG set but no fragmentation octet
    uint8_t type = body[1] & DEVICENET_FRAG_TYPE_MASK;
    uint8_t count = body[1] & DEVICENET_FRAG_COUNT_MASK;
    const uint8_t *data = body + 2;
    uint8_t data_len = (uint8_t)(body_len - 2);

    switch (type)
    {
    case DEVICENET_FRAG_FIRST:
        dws_devicenet_frag_reset(rx);
        rx->active = true;
        rx->next_count = (uint8_t)((count + 1u) & DEVICENET_FRAG_COUNT_MASK);
        if (data_len && !frag_append(rx, data, data_len))
            return DeviceNetFragResult::DEVICENET_FRAG_ERR; // GCOVR_EXCL_LINE  unreachable: FIRST reset()->len 0, then
                                                            // a single append of
                                                            // <=253 (uint8 body_len-2) < 256=MSG_MAX
        return DeviceNetFragResult::DEVICENET_FRAG_STARTED;
    case DEVICENET_FRAG_MIDDLE:
        if (!rx->active || count != rx->next_count)
        {
            dws_devicenet_frag_reset(rx);
            return DeviceNetFragResult::DEVICENET_FRAG_ERR;
        }
        if (data_len && !frag_append(rx, data, data_len))
        {
            dws_devicenet_frag_reset(rx);
            return DeviceNetFragResult::DEVICENET_FRAG_ERR;
        }
        rx->next_count = (uint8_t)((count + 1u) & DEVICENET_FRAG_COUNT_MASK);
        return DeviceNetFragResult::DEVICENET_FRAG_PROGRESS;
    case DEVICENET_FRAG_LAST:
        if (!rx->active || count != rx->next_count)
        {
            dws_devicenet_frag_reset(rx);
            return DeviceNetFragResult::DEVICENET_FRAG_ERR;
        }
        if (data_len && !frag_append(rx, data, data_len))
        {
            dws_devicenet_frag_reset(rx);
            return DeviceNetFragResult::DEVICENET_FRAG_ERR;
        }
        rx->active = false;
        return DeviceNetFragResult::DEVICENET_FRAG_COMPLETE;
    default: // DEVICENET_FRAG_ACK is flow control, not data
        return DeviceNetFragResult::DEVICENET_FRAG_IGNORED;
    }
}

#endif // DWS_ENABLE_DEVICENET
