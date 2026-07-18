// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file canopen.h
 * @brief CANopen (CiA 301) application-layer message codec (DWS_ENABLE_CANOPEN).
 *
 * A pure, zero-heap builder + parser for the CANopen messaging set carried over classic
 * CAN frames (see shared_primitives/can.h): NMT node control, SYNC, TIME, the
 * heartbeat / boot-up (NMT error control), EMCY, PDO (process data), and expedited SDO
 * (service data object) read / write / abort. The 11-bit CAN identifier is a 4-bit
 * function code plus a 7-bit node id; each builder computes the right COB-ID and each
 * parser classifies a received frame back to its function + node.
 *
 * Scope: the CANopen object dictionary itself is the application's; this is the wire codec.
 * SDO transfers are expedited only (<= 4 octets); segmented / block SDO is not yet covered.
 *
 * Bridging: pair with the ESP32's TWAI peripheral (or an MCP2515 over SPI) to bridge a
 * CANopen field bus onto Wi-Fi - expose node state / PDOs over HTTP, MQTT, or a WebSocket.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CANOPEN_H
#define DETERMINISTICESPASYNCWEBSERVER_CANOPEN_H

#include "ServerConfig.h"

#if DWS_ENABLE_CANOPEN

#include "shared_primitives/can.h"
#include <stddef.h>
#include <stdint.h>

// Function-code COB-ID bases. The 11-bit id is (function-code | node-id); the node id is
// 1..127 (0 = broadcast for NMT / SYNC / TIME). EMCY shares 0x080 with SYNC: SYNC is the
// node-id == 0 case, EMCY is 0x081..0x0FF.
#define CANOPEN_COB_NMT 0x000u       ///< NMT node control (broadcast), 2 data octets
#define CANOPEN_COB_SYNC 0x080u      ///< SYNC (broadcast), 0 data octets
#define CANOPEN_COB_EMCY 0x080u      ///< EMCY base (+ node id)
#define CANOPEN_COB_TIME 0x100u      ///< TIME stamp (broadcast)
#define CANOPEN_COB_TPDO1 0x180u     ///< transmit PDO 1 base (+ node id)
#define CANOPEN_COB_RPDO1 0x200u     ///< receive PDO 1 base (+ node id)
#define CANOPEN_COB_TPDO2 0x280u     ///< transmit PDO 2 base
#define CANOPEN_COB_RPDO2 0x300u     ///< receive PDO 2 base
#define CANOPEN_COB_TPDO3 0x380u     ///< transmit PDO 3 base
#define CANOPEN_COB_RPDO3 0x400u     ///< receive PDO 3 base
#define CANOPEN_COB_TPDO4 0x480u     ///< transmit PDO 4 base
#define CANOPEN_COB_RPDO4 0x500u     ///< receive PDO 4 base
#define CANOPEN_COB_SDO_TX 0x580u    ///< SDO server -> client (response), + node id
#define CANOPEN_COB_SDO_RX 0x600u    ///< SDO client -> server (request), + node id
#define CANOPEN_COB_HEARTBEAT 0x700u ///< NMT error control (heartbeat / boot-up), + node id
#define CANOPEN_FUNC_MASK 0x780u     ///< top 4 bits select the function code
#define CANOPEN_NODE_MASK 0x07Fu     ///< low 7 bits select the node id

// NMT node-control commands (CANOPEN_COB_NMT data[0]).
#define CANOPEN_NMT_START 0x01u      ///< enter Operational
#define CANOPEN_NMT_STOP 0x02u       ///< enter Stopped
#define CANOPEN_NMT_PRE_OP 0x80u     ///< enter Pre-operational
#define CANOPEN_NMT_RESET_NODE 0x81u ///< reset application
#define CANOPEN_NMT_RESET_COMM 0x82u ///< reset communication

// NMT states reported in a heartbeat (data[0], low 7 bits).
#define CANOPEN_STATE_BOOTUP 0x00u
#define CANOPEN_STATE_STOPPED 0x04u
#define CANOPEN_STATE_OPERATIONAL 0x05u
#define CANOPEN_STATE_PRE_OP 0x7Fu

// SDO command specifier (high 3 bits of data[0]).
#define CANOPEN_SDO_CCS_DOWNLOAD 1u ///< client download initiate (write)
#define CANOPEN_SDO_CCS_UPLOAD 2u   ///< client upload initiate (read)
#define CANOPEN_SDO_SCS_UPLOAD 2u   ///< server upload initiate response
#define CANOPEN_SDO_SCS_DOWNLOAD 3u ///< server download initiate response (ack)
#define CANOPEN_SDO_ABORT 4u        ///< abort transfer (either direction)

// A few common SDO abort codes (the field is any 32-bit value).
#define CANOPEN_ABORT_TOGGLE 0x05030000u      ///< toggle bit not alternated
#define CANOPEN_ABORT_TIMEOUT 0x05040000u     ///< SDO protocol timed out
#define CANOPEN_ABORT_NO_OBJECT 0x06020000u   ///< object does not exist
#define CANOPEN_ABORT_NO_SUBINDEX 0x06090011u ///< sub-index does not exist
#define CANOPEN_ABORT_GENERAL 0x08000000u     ///< general error

/** @brief CANopen message classes (the function decoded from the COB-ID). */
enum class CanopenType : uint8_t
{
    CANOPEN_T_UNKNOWN = 0,
    CANOPEN_T_NMT,
    CANOPEN_T_SYNC,
    CANOPEN_T_EMCY,
    CANOPEN_T_TIME,
    CANOPEN_T_TPDO,
    CANOPEN_T_RPDO,
    CANOPEN_T_SDO_TX,
    CANOPEN_T_SDO_RX,
    CANOPEN_T_HEARTBEAT,
};

/** @brief A classified CANopen frame (the function code + node, from canopen_parse). */
struct CanopenMsg
{
    CanopenType type;
    uint8_t node_id; ///< 1..127, or 0 for a broadcast (NMT / SYNC / TIME)
    uint8_t pdo_num; ///< 1..4 for TPDO / RPDO, else 0
};

/** @brief A decoded SDO initiate response (from canopen_parse_sdo_response). */
struct CanopenSdoResponse
{
    uint16_t index;      ///< object index echoed by the server
    uint8_t sub;         ///< sub-index echoed by the server
    bool is_abort;       ///< true => the server aborted the transfer
    uint32_t abort_code; ///< valid when is_abort
    bool is_upload;      ///< true => upload (read) response; false => download (write) ack
    bool expedited;      ///< true => the payload is inline in data[0..len-1]
    uint8_t data[4];     ///< expedited upload payload
    uint8_t len;         ///< expedited payload length 0..4
};

// --- builders: fill *out and return true; false on a bad argument ---

/** @brief NMT node-control frame. @p node_id 0 addresses all nodes. */
bool canopen_build_nmt(CanFrame *out, uint8_t command, uint8_t node_id);

/** @brief SYNC frame (zero-length, broadcast). */
bool canopen_build_sync(CanFrame *out);

/** @brief Heartbeat / boot-up frame for @p node_id reporting @p state. */
bool canopen_build_heartbeat(CanFrame *out, uint8_t node_id, uint8_t state);

/** @brief Emergency (EMCY) frame: 16-bit error code (LE), error register, 5 manufacturer octets. */
bool canopen_build_emcy(CanFrame *out, uint8_t node_id, uint16_t error_code, uint8_t error_reg, const uint8_t msef[5]);

/** @brief Transmit-PDO frame (@p pdo_num 1..4): up to 8 raw mapped octets. */
bool canopen_build_tpdo(CanFrame *out, uint8_t pdo_num, uint8_t node_id, const uint8_t *data, uint8_t len);

/** @brief Receive-PDO frame (@p pdo_num 1..4): up to 8 raw mapped octets. */
bool canopen_build_rpdo(CanFrame *out, uint8_t pdo_num, uint8_t node_id, const uint8_t *data, uint8_t len);

/** @brief SDO expedited upload (read) request for object @p index / @p sub on @p node_id. */
bool canopen_build_sdo_read(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub);

/** @brief SDO expedited download (write) of @p len (1..4) octets to @p index / @p sub. */
bool canopen_build_sdo_write(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub, const uint8_t *data,
                             uint8_t len);

/** @brief SDO abort frame. @p to_server true => client->server (0x600), false => server->client (0x580). */
bool canopen_build_sdo_abort(CanFrame *out, uint8_t node_id, uint16_t index, uint8_t sub, uint32_t abort_code,
                             bool to_server);

// --- parsers ---

/** @brief Classify any frame by COB-ID into its CANopen function + node. */
bool canopen_parse(const CanFrame *f, CanopenMsg *out);

/** @brief Decode an EMCY frame (must be a 0x080+node, 8-octet frame). */
bool canopen_parse_emcy(const CanFrame *f, uint8_t *node_id, uint16_t *error_code, uint8_t *error_reg, uint8_t msef[5]);

/** @brief Decode a heartbeat frame (0x700+node, 1 octet). */
bool canopen_parse_heartbeat(const CanFrame *f, uint8_t *node_id, uint8_t *state);

/** @brief Decode an SDO server response (0x580+node): upload data, download ack, or abort. */
bool canopen_parse_sdo_response(const CanFrame *f, CanopenSdoResponse *out);

#endif // DWS_ENABLE_CANOPEN
#endif // DETERMINISTICESPASYNCWEBSERVER_CANOPEN_H
