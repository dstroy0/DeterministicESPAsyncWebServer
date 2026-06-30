// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file devicenet.h
 * @brief DeviceNet link-adaptation codec (DETWS_ENABLE_DEVICENET) - the CAN-specific layer of
 *        "CIP over CAN".
 *
 * DeviceNet (ODVA) carries CIP over classic CAN. The CIP application layer (services, EPATH,
 * data) is the same one the EtherNet/IP codec uses, so build the message body with the
 * existing `cip_*` functions (`DETWS_ENABLE_CIP`); this module supplies the DeviceNet-specific
 * link adaptation that is NOT part of CIP:
 *
 *  - The 11-bit CAN **identifier** as a Message Group (1..4) + Message ID + MAC ID, per the
 *    DeviceNet identifier allocation:
 *    @code
 *      Group 1: 0  MsgID(4)  SourceMAC(6)              ids 0x000-0x3FF
 *      Group 2: 10 MAC(6)    MsgID(3)                  ids 0x400-0x5FF
 *      Group 3: 11 MsgID(3)  SourceMAC(6)              ids 0x600-0x7BF
 *      Group 4: 11111        MsgID(6)                  ids 0x7C0-0x7EF
 *    @endcode
 *  - The explicit-message **header octet** (FRAG | XID | MAC ID).
 *  - The **fragmentation protocol** (type + modulo-64 count) and a reassembler for explicit
 *    messages longer than one 8-octet frame.
 *
 * Pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to
 * bridge a DeviceNet segment onto Wi-Fi.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DEVICENET_H
#define DETERMINISTICESPASYNCWEBSERVER_DEVICENET_H

#include "shared_primitives/shim.h"

#if DETWS_ENABLE_DEVICENET

#ifndef DETWS_DEVICENET_MSG_MAX
#define DETWS_DEVICENET_MSG_MAX 256 ///< max reassembled fragmented message
#endif

// Message-group identifier bases / field widths.
#define DEVICENET_G1_BASE 0x000u ///< Message Group 1 (0x000-0x3FF)
#define DEVICENET_G2_BASE 0x400u ///< Message Group 2 (0x400-0x5FF)
#define DEVICENET_G3_BASE 0x600u ///< Message Group 3 (0x600-0x7BF)
#define DEVICENET_G4_BASE 0x7C0u ///< Message Group 4 (0x7C0-0x7EF)
#define DEVICENET_MAC_MASK 0x3Fu ///< MAC IDs are 0..63

// Common Group 2 message IDs (predefined master/slave connection set).
#define DEVICENET_G2_UNCONNECTED_EXPLICIT_REQ 4u ///< unconnected explicit request to a slave
#define DEVICENET_G2_EXPLICIT_RESPONSE 3u        ///< explicit / unconnected response from a slave
#define DEVICENET_G2_POLL_COMMAND 5u             ///< Poll command / change-of-state to a slave
#define DEVICENET_G2_DUP_MAC_CHECK 7u            ///< Duplicate MAC ID check

// Explicit-message header octet fields.
#define DEVICENET_HDR_FRAG 0x80u ///< this body is fragmented (a fragmentation octet follows)
#define DEVICENET_HDR_XID 0x40u  ///< transaction-id bit

// Fragmentation octet: type in the top 2 bits, modulo-64 count in the low 6.
#define DEVICENET_FRAG_FIRST 0x00u  ///< first fragment
#define DEVICENET_FRAG_MIDDLE 0x40u ///< middle fragment
#define DEVICENET_FRAG_LAST 0x80u   ///< last fragment
#define DEVICENET_FRAG_ACK 0xC0u    ///< fragment acknowledge
#define DEVICENET_FRAG_TYPE_MASK 0xC0u
#define DEVICENET_FRAG_COUNT_MASK 0x3Fu

/** @brief DeviceNet message groups. */
enum DeviceNetGroup
{
    DEVICENET_GROUP_1 = 1,
    DEVICENET_GROUP_2 = 2,
    DEVICENET_GROUP_3 = 3,
    DEVICENET_GROUP_4 = 4,
};

/** @brief A decoded DeviceNet identifier. */
struct DeviceNetId
{
    DeviceNetGroup group;
    uint8_t msg_id; ///< message id within the group
    uint8_t mac_id; ///< source / node MAC id (0..63; not present for Group 4)
};

/** @brief Result of feeding a frame to the fragmentation reassembler. */
enum DeviceNetFragResult
{
    DEVICENET_FRAG_IGNORED = 0,
    DEVICENET_FRAG_STARTED,
    DEVICENET_FRAG_PROGRESS,
    DEVICENET_FRAG_COMPLETE,
    DEVICENET_FRAG_ERR,
};

/** @brief Fragmented-message reassembly context. */
struct DeviceNetFragRx
{
    bool active;
    uint8_t next_count;                   ///< next expected modulo-64 fragment count
    uint16_t len;                         ///< octets stored so far
    uint8_t buf[DETWS_DEVICENET_MSG_MAX]; ///< reassembled body (excludes the fragmentation octets)
};

// --- identifier ---

/** @brief Encode a DeviceNet 11-bit CAN id. @p mac_id is ignored for Group 4. */
bool devicenet_encode_id(uint32_t *id, DeviceNetGroup group, uint8_t msg_id, uint8_t mac_id);

/** @brief Decode an 11-bit CAN id into its DeviceNet group / message id / MAC id. */
bool devicenet_decode_id(uint32_t can_id, DeviceNetId *out);

// --- explicit-message header + fragmentation octets ---

/** @brief Compose the explicit-message header octet (FRAG / XID / MAC id). */
uint8_t devicenet_msg_header(bool frag, bool xid, uint8_t mac_id);

/** @brief Compose a fragmentation octet from a type (DEVICENET_FRAG_*) and a count. */
uint8_t devicenet_frag_octet(uint8_t type, uint8_t count);

// --- non-fragmented explicit message in one frame ---

/**
 * @brief Build a single-frame explicit message: [header octet][body...] at the group/msg id.
 * @p body is typically a CIP request built with `cip_*`. Fails if it does not fit in 8 octets.
 */
bool devicenet_build_explicit(CanFrame *out, DeviceNetGroup group, uint8_t msg_id, uint8_t mac_id, const uint8_t *body,
                              uint8_t body_len);

// --- fragmentation reassembly (messages longer than one frame) ---

/** @brief Reset a reassembly context to idle. */
void devicenet_frag_reset(DeviceNetFragRx *rx);

/**
 * @brief Feed a received frame's body (the octets after the CAN id) to the reassembler.
 * @p body points at the explicit-message body starting with the header octet; the
 * fragmentation octet (when DEVICENET_HDR_FRAG is set) is the second octet.
 */
DeviceNetFragResult devicenet_frag_feed(DeviceNetFragRx *rx, const uint8_t *body, uint8_t body_len);

#endif // DETWS_ENABLE_DEVICENET
#endif // DETERMINISTICESPASYNCWEBSERVER_DEVICENET_H
