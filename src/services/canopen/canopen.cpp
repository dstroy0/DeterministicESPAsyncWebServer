// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file canopen.cpp
 * @brief CANopen (CiA 301) message codec (pure, host-tested).
 */

#include "services/canopen/canopen.h"

#if DWS_ENABLE_CANOPEN

#include <string.h>

// All CANopen default-profile identifiers are 11-bit standard frames.
static void std_frame(CanFrame *f, uint32_t id, uint8_t dlc)
{
    f->id = id & DWS_CAN_STD_ID_MASK;
    f->extended = false;
    f->rtr = false;
    f->dlc = dlc;
    memset(f->data, 0, sizeof(f->data));
}

static bool valid_node(uint8_t node_id)
{
    return node_id >= 1 && node_id <= 127;
}

bool canopen_build_nmt(CanFrame *out, uint8_t command, uint8_t node_id)
{
    if (!out || node_id > 127) // 0 = all nodes
        return false;
    std_frame(out, CANOPEN_COB_NMT, 2);
    out->data[0] = command;
    out->data[1] = node_id;
    return true;
}

bool canopen_build_sync(CanFrame *out)
{
    if (!out)
        return false;
    std_frame(out, CANOPEN_COB_SYNC, 0);
    return true;
}

bool canopen_build_heartbeat(CanFrame *out, uint8_t node_id, uint8_t state)
{
    if (!out || !valid_node(node_id))
        return false;
    std_frame(out, CANOPEN_COB_HEARTBEAT + node_id, 1);
    out->data[0] = state;
    return true;
}

bool canopen_build_emcy(CanFrame *out, uint8_t node_id, uint16_t error_code, uint8_t error_reg, const uint8_t msef[5])
{
    if (!out || !valid_node(node_id))
        return false;
    std_frame(out, CANOPEN_COB_EMCY + node_id, 8);
    out->data[0] = (uint8_t)error_code; // error code, little-endian
    out->data[1] = (uint8_t)(error_code >> 8);
    out->data[2] = error_reg; // object 0x1001 error register
    if (msef)
        memcpy(out->data + 3, msef, 5); // 5 manufacturer-specific error octets
    return true;
}

// Map a PDO number (1..4) to its TPDO / RPDO COB-ID base.
static bool pdo_base(uint8_t pdo_num, bool transmit, uint32_t *base)
{
    if (pdo_num < 1 || pdo_num > 4)
        return false;
    static const uint32_t tx[4] = {CANOPEN_COB_TPDO1, CANOPEN_COB_TPDO2, CANOPEN_COB_TPDO3, CANOPEN_COB_TPDO4};
    static const uint32_t rx[4] = {CANOPEN_COB_RPDO1, CANOPEN_COB_RPDO2, CANOPEN_COB_RPDO3, CANOPEN_COB_RPDO4};
    *base = (transmit ? tx : rx)[pdo_num - 1];
    return true;
}

static bool build_pdo(CanFrame *out, uint8_t pdo_num, bool transmit, uint8_t node_id, const uint8_t *data, uint8_t len)
{
    uint32_t base;
    if (!out || !valid_node(node_id) || len > DWS_CAN_MAX_DLC || (len && !data) || !pdo_base(pdo_num, transmit, &base))
        return false;
    std_frame(out, base + node_id, len);
    if (len)
        memcpy(out->data, data, len);
    return true;
}

bool canopen_build_tpdo(CanFrame *out, uint8_t pdo_num, uint8_t node_id, const uint8_t *data, uint8_t len)
{
    return build_pdo(out, pdo_num, true, node_id, data, len);
}

bool canopen_build_rpdo(CanFrame *out, uint8_t pdo_num, uint8_t node_id, const uint8_t *data, uint8_t len)
{
    return build_pdo(out, pdo_num, false, node_id, data, len);
}

// Fill data[1..3] with the object index (LE) + sub-index common to every SDO frame.
static void sdo_set_object(CanFrame *f, uint16_t index, uint8_t sub)
{
    f->data[1] = (uint8_t)index;
    f->data[2] = (uint8_t)(index >> 8);
    f->data[3] = sub;
}

bool canopen_build_sdo_read(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub)
{
    if (!out || !valid_node(node_id))
        return false;
    std_frame(out, CANOPEN_COB_SDO_RX + node_id, 8);
    out->data[0] = (uint8_t)(CANOPEN_SDO_CCS_UPLOAD << 5); // upload initiate request (0x40)
    sdo_set_object(out, index, sub);
    return true;
}

bool canopen_build_sdo_write(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub, const uint8_t *data,
                             uint8_t len)
{
    if (!out || !valid_node(node_id) || len < 1 || len > 4 || !data)
        return false;
    std_frame(out, CANOPEN_COB_SDO_RX + node_id, 8);
    // download initiate, expedited (e=1), size indicated (s=1); n = unused octets in data[4..7].
    out->data[0] = (uint8_t)((CANOPEN_SDO_CCS_DOWNLOAD << 5) | (((4u - len) & 3u) << 2) | 0x03u);
    sdo_set_object(out, index, sub);
    memcpy(out->data + 4, data, len);
    return true;
}

bool canopen_build_sdo_abort(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub, uint32_t abort_code,
                             bool to_server)
{
    if (!out || !valid_node(node_id))
        return false;
    std_frame(out, (to_server ? CANOPEN_COB_SDO_RX : CANOPEN_COB_SDO_TX) + node_id, 8);
    out->data[0] = (uint8_t)(CANOPEN_SDO_ABORT << 5); // 0x80
    sdo_set_object(out, index, sub);
    out->data[4] = (uint8_t)abort_code; // abort code, little-endian
    out->data[5] = (uint8_t)(abort_code >> 8);
    out->data[6] = (uint8_t)(abort_code >> 16);
    out->data[7] = (uint8_t)(abort_code >> 24);
    return true;
}

bool canopen_parse(const CanFrame *f, CanopenMsg *out)
{
    if (!f || !out || f->extended)
        return false; // CANopen default profile is 11-bit standard frames
    uint32_t id = f->id & DWS_CAN_STD_ID_MASK;
    uint32_t func = id & CANOPEN_FUNC_MASK;
    uint8_t node = (uint8_t)(id & CANOPEN_NODE_MASK);
    out->type = CanopenType::CANOPEN_T_UNKNOWN;
    out->node_id = node;
    out->pdo_num = 0;

    if (id == CANOPEN_COB_NMT)
    {
        out->type = CanopenType::CANOPEN_T_NMT;
        out->node_id = 0;
        return true;
    }
    if (id == CANOPEN_COB_SYNC) // function 0x080 with node 0
    {
        out->type = CanopenType::CANOPEN_T_SYNC;
        out->node_id = 0;
        return true;
    }
    if (id == CANOPEN_COB_TIME)
    {
        out->type = CanopenType::CANOPEN_T_TIME;
        out->node_id = 0;
        return true;
    }
    if (node == 0)
        return true; // a function base with node 0 we don't classify further

    switch (func)
    {
    case CANOPEN_COB_EMCY:
        out->type = CanopenType::CANOPEN_T_EMCY;
        return true;
    case CANOPEN_COB_TPDO1:
        out->type = CanopenType::CANOPEN_T_TPDO;
        out->pdo_num = 1;
        return true;
    case CANOPEN_COB_RPDO1:
        out->type = CanopenType::CANOPEN_T_RPDO;
        out->pdo_num = 1;
        return true;
    case CANOPEN_COB_TPDO2:
        out->type = CanopenType::CANOPEN_T_TPDO;
        out->pdo_num = 2;
        return true;
    case CANOPEN_COB_RPDO2:
        out->type = CanopenType::CANOPEN_T_RPDO;
        out->pdo_num = 2;
        return true;
    case CANOPEN_COB_TPDO3:
        out->type = CanopenType::CANOPEN_T_TPDO;
        out->pdo_num = 3;
        return true;
    case CANOPEN_COB_RPDO3:
        out->type = CanopenType::CANOPEN_T_RPDO;
        out->pdo_num = 3;
        return true;
    case CANOPEN_COB_TPDO4:
        out->type = CanopenType::CANOPEN_T_TPDO;
        out->pdo_num = 4;
        return true;
    case CANOPEN_COB_RPDO4:
        out->type = CanopenType::CANOPEN_T_RPDO;
        out->pdo_num = 4;
        return true;
    case CANOPEN_COB_SDO_TX:
        out->type = CanopenType::CANOPEN_T_SDO_TX;
        return true;
    case CANOPEN_COB_SDO_RX:
        out->type = CanopenType::CANOPEN_T_SDO_RX;
        return true;
    case CANOPEN_COB_HEARTBEAT:
        out->type = CanopenType::CANOPEN_T_HEARTBEAT;
        return true;
    default:
        return true; // unknown function code: type stays CanopenType::CANOPEN_T_UNKNOWN
    }
}

bool canopen_parse_emcy(const CanFrame *f, uint8_t *node_id, uint16_t *error_code, uint8_t *error_reg, uint8_t msef[5])
{
    if (!f || f->extended || f->dlc < 8)
        return false;
    uint32_t id = f->id & DWS_CAN_STD_ID_MASK;
    uint8_t node = (uint8_t)(id & CANOPEN_NODE_MASK);
    if ((id & CANOPEN_FUNC_MASK) != CANOPEN_COB_EMCY || node == 0)
        return false; // 0x080 with node 0 is SYNC, not EMCY
    if (node_id)
        *node_id = node;
    if (error_code)
        *error_code = (uint16_t)(f->data[0] | (f->data[1] << 8));
    if (error_reg)
        *error_reg = f->data[2];
    if (msef)
        memcpy(msef, f->data + 3, 5);
    return true;
}

bool canopen_parse_heartbeat(const CanFrame *f, uint8_t *node_id, uint8_t *state)
{
    if (!f || f->extended || f->dlc < 1)
        return false;
    uint32_t id = f->id & DWS_CAN_STD_ID_MASK;
    uint8_t node = (uint8_t)(id & CANOPEN_NODE_MASK);
    if ((id & CANOPEN_FUNC_MASK) != CANOPEN_COB_HEARTBEAT || node == 0)
        return false;
    if (node_id)
        *node_id = node;
    if (state)
        *state = (uint8_t)(f->data[0] & 0x7Fu); // bit 7 is the boot toggle in some stacks
    return true;
}

bool canopen_parse_sdo_response(const CanFrame *f, CanopenSdoResponse *out)
{
    if (!f || !out || f->extended || f->dlc < 8)
        return false;
    uint32_t id = f->id & DWS_CAN_STD_ID_MASK;
    if ((id & CANOPEN_FUNC_MASK) != CANOPEN_COB_SDO_TX || (id & CANOPEN_NODE_MASK) == 0)
        return false;

    uint8_t cmd = f->data[0];
    uint8_t scs = (uint8_t)(cmd >> 5);
    out->index = (uint16_t)(f->data[1] | (f->data[2] << 8));
    out->sub = f->data[3];
    out->is_abort = false;
    out->abort_code = 0;
    out->is_upload = false;
    out->expedited = false;
    out->len = 0;
    memset(out->data, 0, sizeof(out->data));

    if (scs == CANOPEN_SDO_ABORT)
    {
        out->is_abort = true;
        out->abort_code = (uint32_t)f->data[4] | ((uint32_t)f->data[5] << 8) | ((uint32_t)f->data[6] << 16) |
                          ((uint32_t)f->data[7] << 24);
        return true;
    }
    if (scs == CANOPEN_SDO_SCS_UPLOAD) // upload initiate response
    {
        out->is_upload = true;
        bool e = (cmd & 0x02u) != 0; // expedited
        bool s = (cmd & 0x01u) != 0; // size indicated
        if (e)
        {
            out->expedited = true;
            out->len = s ? (uint8_t)(4u - ((cmd >> 2) & 0x03u)) : 4u;
            memcpy(out->data, f->data + 4, out->len);
        }
        return true; // a non-expedited (segmented) response is reported with len 0
    }
    if (scs == CANOPEN_SDO_SCS_DOWNLOAD) // download initiate response (write acknowledged)
        return true;
    return false; // not a recognised server command specifier
}

#endif // DWS_ENABLE_CANOPEN
