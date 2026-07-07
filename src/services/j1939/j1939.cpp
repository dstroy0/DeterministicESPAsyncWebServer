// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file j1939.cpp
 * @brief SAE J1939 message codec (pure, host-tested).
 */

#include "services/j1939/j1939.h"

#if DETWS_ENABLE_J1939

#include <string.h>

bool j1939_encode_id(uint32_t *id, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da)
{
    if (!id || priority > 7 || pgn > 0x3FFFFu)
        return false;
    uint8_t edp = (uint8_t)((pgn >> 17) & 1u);
    uint8_t dp = (uint8_t)((pgn >> 16) & 1u);
    uint8_t pf = (uint8_t)((pgn >> 8) & 0xFFu);
    // PDU1 (peer-to-peer) carries the destination address in PS; PDU2 (broadcast) carries
    // the PGN's group-extension low octet there.
    uint8_t ps = (pf < J1939_PDU2_THRESHOLD) ? da : (uint8_t)(pgn & 0xFFu);
    *id = ((uint32_t)(priority & 7u) << 26) | ((uint32_t)edp << 25) | ((uint32_t)dp << 24) | ((uint32_t)pf << 16) |
          ((uint32_t)ps << 8) | (uint32_t)sa;
    return true;
}

bool j1939_decode_id(uint32_t id, J1939Id *out)
{
    if (!out)
        return false;
    id &= DET_CAN_EXT_ID_MASK;
    out->priority = (uint8_t)((id >> 26) & 7u);
    uint8_t edp = (uint8_t)((id >> 25) & 1u);
    uint8_t dp = (uint8_t)((id >> 24) & 1u);
    out->pf = (uint8_t)((id >> 16) & 0xFFu);
    out->ps = (uint8_t)((id >> 8) & 0xFFu);
    out->sa = (uint8_t)(id & 0xFFu);
    out->pdu1 = out->pf < J1939_PDU2_THRESHOLD;
    if (out->pdu1)
    {
        out->da = out->ps;
        out->pgn = ((uint32_t)edp << 17) | ((uint32_t)dp << 16) | ((uint32_t)out->pf << 8);
    }
    else
    {
        out->da = J1939_ADDR_GLOBAL;
        out->pgn = ((uint32_t)edp << 17) | ((uint32_t)dp << 16) | ((uint32_t)out->pf << 8) | out->ps;
    }
    return true;
}

// Fill a CanFrame as a 29-bit extended frame.
static bool ext_frame(CanFrame *f, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, uint8_t dlc)
{
    uint32_t id;
    if (!j1939_encode_id(&id, priority, pgn, sa, da))
        return false;
    f->id = id;
    f->extended = true;
    f->rtr = false;
    f->dlc = dlc;
    memset(f->data, 0xFF, sizeof(f->data)); // J1939 pads unused octets with 0xFF (not available)
    return true;
}

bool j1939_build_message(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                         uint8_t len)
{
    if (!out || len > DET_CAN_MAX_DLC || (len && !data))
        return false;
    if (!ext_frame(out, priority, pgn, sa, da, len))
        return false;
    if (len)
        memcpy(out->data, data, len);
    return true;
}

bool j1939_build_request(CanFrame *out, uint8_t sa, uint8_t da, uint32_t requested_pgn)
{
    if (!out || requested_pgn > 0x3FFFFu)
        return false;
    // Request PGN (priority 6): 3-octet little-endian requested PGN.
    if (!ext_frame(out, 6, J1939_PGN_REQUEST, sa, da, 3))
        return false; // GCOVR_EXCL_LINE  unreachable: fixed priority 6 + J1939_PGN_REQUEST (<=0x3FFFF), encode can't
                      // fail
    out->data[0] = (uint8_t)requested_pgn;
    out->data[1] = (uint8_t)(requested_pgn >> 8);
    out->data[2] = (uint8_t)(requested_pgn >> 16);
    return true;
}

uint64_t j1939_build_name(bool arbitrary_address_capable, uint8_t industry_group, uint8_t vehicle_system_instance,
                          uint8_t vehicle_system, uint8_t function, uint8_t function_instance, uint8_t ecu_instance,
                          uint16_t manufacturer_code, uint32_t identity_number)
{
    // NAME bit layout (J1939-81), LSB first:
    //  [0..20] identity number, [21..31] manufacturer code, [32..34] ECU instance,
    //  [35..39] function instance, [40..47] function, [48] reserved, [49..55] vehicle system,
    //  [56..59] vehicle system instance, [60..62] industry group, [63] arbitrary-address-capable.
    uint64_t n = 0;
    n |= (uint64_t)(identity_number & 0x1FFFFFu);
    n |= (uint64_t)(manufacturer_code & 0x7FFu) << 21;
    n |= (uint64_t)(ecu_instance & 0x7u) << 32;
    n |= (uint64_t)(function_instance & 0x1Fu) << 35;
    n |= (uint64_t)(function & 0xFFu) << 40;
    n |= (uint64_t)(vehicle_system & 0x7Fu) << 49;
    n |= (uint64_t)(vehicle_system_instance & 0xFu) << 56;
    n |= (uint64_t)(industry_group & 0x7u) << 60;
    n |= (uint64_t)(arbitrary_address_capable ? 1u : 0u) << 63;
    return n;
}

bool j1939_build_address_claim(CanFrame *out, uint8_t sa, uint64_t name)
{
    // Address Claimed (priority 6, broadcast): NAME as 8 octets, little-endian.
    if (!ext_frame(out, 6, J1939_PGN_ADDRESS_CLAIM, sa, J1939_ADDR_GLOBAL, 8))
        return false; // GCOVR_EXCL_LINE  unreachable: fixed priority 6 + J1939_PGN_ADDRESS_CLAIM (<=0x3FFFF), encode
                      // can't fail
    for (int i = 0; i < 8; i++)
        out->data[i] = (uint8_t)(name >> (8 * i));
    return true;
}

uint8_t j1939_tp_num_packets(uint16_t total_size)
{
    return (uint8_t)((total_size + (J1939_TP_DT_LEN - 1)) / J1939_TP_DT_LEN);
}

bool j1939_build_bam_cm(CanFrame *out, uint8_t sa, uint32_t pgn, uint16_t total_size)
{
    if (!out || total_size < 9 || total_size > DETWS_J1939_TP_MAX || pgn > 0x3FFFFu)
        return false; // BAM is for 9..1785 octet messages
    if (!ext_frame(out, 7, J1939_PGN_TP_CM, sa, J1939_ADDR_GLOBAL, 8))
        return false; // GCOVR_EXCL_LINE  unreachable: fixed priority 7 + J1939_PGN_TP_CM (<=0x3FFFF), encode can't fail
    out->data[0] = J1939_TP_CM_BAM;
    out->data[1] = (uint8_t)total_size; // message size, little-endian
    out->data[2] = (uint8_t)(total_size >> 8);
    out->data[3] = j1939_tp_num_packets(total_size); // total packets
    out->data[4] = 0xFF;                             // reserved
    out->data[5] = (uint8_t)pgn;                     // transported PGN, little-endian
    out->data[6] = (uint8_t)(pgn >> 8);
    out->data[7] = (uint8_t)(pgn >> 16);
    return true;
}

bool j1939_build_tp_dt(CanFrame *out, uint8_t sa, uint8_t da, uint8_t seq, const uint8_t *chunk, uint8_t chunk_len)
{
    if (!out || seq == 0 || chunk_len == 0 || chunk_len > J1939_TP_DT_LEN || !chunk)
        return false;
    if (!ext_frame(out, 7, J1939_PGN_TP_DT, sa, da, 8))
        return false; // GCOVR_EXCL_LINE  unreachable: fixed priority 7 + J1939_PGN_TP_DT (<=0x3FFFF), encode can't fail
    out->data[0] = seq;                      // sequence number, 1-based
    memcpy(out->data + 1, chunk, chunk_len); // remaining octets stay 0xFF padding
    return true;
}

void j1939_tp_reset(J1939TpRx *rx)
{
    if (rx)
        memset(rx, 0, sizeof(*rx));
}

J1939TpResult j1939_tp_feed(J1939TpRx *rx, const CanFrame *f)
{
    if (!rx || !f || !f->extended)
        return J1939_TP_IGNORED;
    J1939Id id;
    if (!j1939_decode_id(f->id, &id))
        return J1939_TP_IGNORED; // GCOVR_EXCL_LINE  unreachable: decode_id only fails on a null out, and &id is
                                 // non-null

    if (id.pgn == J1939_PGN_TP_CM && f->dlc >= 8)
    {
        uint8_t control = f->data[0];
        if (control != J1939_TP_CM_BAM && control != J1939_TP_CM_RTS)
            return J1939_TP_IGNORED; // CTS / EOM / Abort are not receiver-side session starts
        uint16_t total = (uint16_t)(f->data[1] | (f->data[2] << 8));
        uint8_t packets = f->data[3];
        uint32_t pgn = (uint32_t)f->data[5] | ((uint32_t)f->data[6] << 8) | ((uint32_t)f->data[7] << 16);
        if (total < 9 || total > DETWS_J1939_TP_MAX || packets != j1939_tp_num_packets(total))
            return J1939_TP_ERROR;
        rx->active = true;
        rx->sa = id.sa;
        rx->pgn = pgn;
        rx->total_size = total;
        rx->num_packets = packets;
        rx->next_seq = 1;
        rx->received = 0;
        return J1939_TP_STARTED;
    }

    if (id.pgn == J1939_PGN_TP_DT && f->dlc >= 1)
    {
        if (!rx->active || id.sa != rx->sa)
            return J1939_TP_IGNORED;
        uint8_t seq = f->data[0];
        if (seq != rx->next_seq)
        {
            j1939_tp_reset(rx);
            return J1939_TP_ERROR; // out-of-sequence: abort the session
        }
        uint16_t remaining = (uint16_t)(rx->total_size - rx->received);
        uint8_t take = remaining < J1939_TP_DT_LEN ? (uint8_t)remaining : (uint8_t)J1939_TP_DT_LEN;
        memcpy(rx->buf + rx->received, f->data + 1, take);
        rx->received = (uint16_t)(rx->received + take);
        rx->next_seq++;
        if (rx->received >= rx->total_size)
        {
            rx->active = false;
            return J1939_TP_COMPLETE;
        }
        return J1939_TP_PROGRESS;
    }

    return J1939_TP_IGNORED;
}

#endif // DETWS_ENABLE_J1939
