// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file j1939.h
 * @brief SAE J1939 message codec (DETWS_ENABLE_J1939) - the heavy-duty-vehicle / agriculture /
 *        marine / genset CAN higher-layer protocol, over 29-bit extended CAN frames.
 *
 * J1939 packs a 29-bit extended identifier as:
 * @code
 *   bits 28-26 Priority | 25 EDP | 24 DP | 23-16 PF | 15-8 PS | 7-0 SA
 * @endcode
 * The 18-bit Parameter Group Number (PGN) is EDP|DP|PF|PS, where PS is part of the PGN only
 * for PDU2 (broadcast, PF >= 240); for PDU1 (PF < 240) PS is the destination address (DA) and
 * the PGN's low octet is 0. This codec encodes / decodes that id, builds single-frame
 * messages, runs the Transport Protocol (BAM broadcast + RTS/CTS connection mode) with a
 * reassembler for messages up to `DETWS_J1939_TP_MAX` octets, and builds the Address Claimed
 * (with a 64-bit NAME) and Request PGN messages.
 *
 * Pure and host-tested. Drive it from the ESP32 TWAI peripheral (or an MCP2515 over SPI) to
 * bridge a J1939 bus onto Wi-Fi - decode engine / transmission / genset PGNs and publish them.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_J1939_H
#define DETERMINISTICESPASYNCWEBSERVER_J1939_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_J1939

#include "shared_primitives/det_can.h"
#include <stddef.h>
#include <stdint.h>

// Well-known PGNs and addresses.
#define J1939_PGN_TP_CM 0x00EC00u         ///< Transport Protocol - Connection Management (60416)
#define J1939_PGN_TP_DT 0x00EB00u         ///< Transport Protocol - Data Transfer (60160)
#define J1939_PGN_ADDRESS_CLAIM 0x00EE00u ///< Address Claimed / Cannot Claim (60928)
#define J1939_PGN_REQUEST 0x00EA00u       ///< Request PGN (59904)
#define J1939_ADDR_GLOBAL 0xFFu           ///< broadcast destination address
#define J1939_ADDR_NULL 0xFEu             ///< null / unclaimed source address
#define J1939_PDU2_THRESHOLD 240u         ///< PF >= 240 is PDU2 (broadcast); < 240 is PDU1 (peer)

// TP.CM control bytes (data[0] of a TP.CM frame).
#define J1939_TP_CM_RTS 0x10u     ///< Request To Send (connection mode)
#define J1939_TP_CM_CTS 0x11u     ///< Clear To Send
#define J1939_TP_CM_EOM_ACK 0x13u ///< End Of Message Acknowledge
#define J1939_TP_CM_BAM 0x20u     ///< Broadcast Announce Message
#define J1939_TP_CM_ABORT 0xFFu   ///< Connection Abort

#define J1939_TP_DT_LEN 7u ///< data octets carried per TP.DT packet (1 seq byte + 7 data)

/** @brief A decoded J1939 identifier. */
struct J1939Id
{
    uint8_t priority; ///< 0 (highest) .. 7
    uint32_t pgn;     ///< 18-bit Parameter Group Number
    uint8_t sa;       ///< source address
    uint8_t da;       ///< destination address (PDU1), or J1939_ADDR_GLOBAL (PDU2)
    uint8_t pf;       ///< PDU format
    uint8_t ps;       ///< PDU specific (DA for PDU1, group extension for PDU2)
    bool pdu1;        ///< true => peer-to-peer (PF < 240); false => broadcast
};

/** @brief Result of feeding a frame to the TP reassembler. */
enum J1939TpResult
{
    J1939_TP_IGNORED = 0, ///< not a TP frame for the active session
    J1939_TP_STARTED,     ///< a BAM / RTS opened a session
    J1939_TP_PROGRESS,    ///< a data packet was accepted, more to come
    J1939_TP_COMPLETE,    ///< the message is fully reassembled (see fields below)
    J1939_TP_ERROR,       ///< malformed / out-of-sequence / too large
};

/** @brief Transport-Protocol reassembly context (one in-flight message). */
struct J1939TpRx
{
    bool active;
    uint8_t sa;          ///< source of the session
    uint32_t pgn;        ///< the transported PGN
    uint16_t total_size; ///< announced message size
    uint8_t num_packets; ///< announced packet count
    uint8_t next_seq;    ///< next expected sequence number (1-based)
    uint16_t received;   ///< octets stored so far
    uint8_t buf[DETWS_J1939_TP_MAX];
};

// --- identifier ---

/** @brief Encode a 29-bit J1939 id. @p da is used only for a PDU1 (PF < 240) PGN. */
bool j1939_encode_id(uint32_t *id, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da);

/** @brief Decode a 29-bit J1939 id into its fields. */
bool j1939_decode_id(uint32_t id, J1939Id *out);

// --- single-frame messages ---

/** @brief Build a single-frame (<= 8 octet) J1939 message. */
bool j1939_build_message(CanFrame *out, uint8_t priority, uint32_t pgn, uint8_t sa, uint8_t da, const uint8_t *data,
                         uint8_t len);

/** @brief Build a Request-PGN frame asking @p da for @p requested_pgn. */
bool j1939_build_request(CanFrame *out, uint8_t sa, uint8_t da, uint32_t requested_pgn);

/** @brief Build an Address-Claimed frame announcing @p sa with the 64-bit @p name. */
bool j1939_build_address_claim(CanFrame *out, uint8_t sa, uint64_t name);

/** @brief Compose a 64-bit J1939 NAME from its fields (see J1939-81). */
uint64_t j1939_build_name(bool arbitrary_address_capable, uint8_t industry_group, uint8_t vehicle_system_instance,
                          uint8_t vehicle_system, uint8_t function, uint8_t function_instance, uint8_t ecu_instance,
                          uint16_t manufacturer_code, uint32_t identity_number);

// --- transport protocol (multi-packet) ---

/** @brief Octet count -> TP packet count (ceil(size / 7)). */
uint8_t j1939_tp_num_packets(uint16_t total_size);

/** @brief Build the BAM (broadcast) TP.CM announce frame for @p pgn / @p total_size. */
bool j1939_build_bam_cm(CanFrame *out, uint8_t sa, uint32_t pgn, uint16_t total_size);

/** @brief Build TP.DT data packet @p seq (1-based) carrying @p chunk_len (1..7) octets. */
bool j1939_build_tp_dt(CanFrame *out, uint8_t sa, uint8_t da, uint8_t seq, const uint8_t *chunk, uint8_t chunk_len);

/** @brief Reset a reassembly context to idle. */
void j1939_tp_reset(J1939TpRx *rx);

/** @brief Feed a received frame to the reassembler; see @ref J1939TpResult. */
J1939TpResult j1939_tp_feed(J1939TpRx *rx, const CanFrame *f);

#endif // DETWS_ENABLE_J1939
#endif // DETERMINISTICESPASYNCWEBSERVER_J1939_H
