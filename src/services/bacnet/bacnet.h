// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file bacnet.h
 * @brief BACnet/IP BVLC + NPDU codec (DWS_ENABLE_BACNET) - zero-heap framing for the
 *        ASHRAE 135 building-automation network layer over UDP (default port 47808).
 *
 * Two stacked layers:
 *  - BVLC (Annex J): `Type(1)=0x81  Function(1)  Length(2, big-endian, whole BVLL)` then the
 *    NPDU. Functions include Original-Unicast-NPDU (0x0A) and Original-Broadcast-NPDU (0x0B).
 *  - NPDU (Clause 6): `Version(1)=0x01  NPCI-Control(1)` then optional addressing. The NPCI
 *    control bits: 0x80 = network-layer message (else APDU), 0x20 = destination present
 *    (DNET(2) DLEN(1) DADR(DLEN); DLEN 0 = remote broadcast), 0x08 = source present (SNET(2)
 *    SLEN(1) SADR), 0x04 = expecting reply, low 2 bits = priority. A hop count octet follows
 *    the source fields when a destination is present. The APDU is whatever remains.
 *
 * The builders frame an APDU into a caller buffer (fail-closed); the parsers validate and
 * report the slices. Layout verified against ASHRAE 135 Annex J / Clause 6.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_BACNET_H
#define DETERMINISTICESPASYNCWEBSERVER_BACNET_H

#include "ServerConfig.h"

#if DWS_ENABLE_BACNET

#include <stddef.h>
#include <stdint.h>

#define BVLC_TYPE_BIP 0x81 ///< BVLC Type: BACnet/IP
#define BVLC_HEADER_SIZE 4 ///< type + function + 2-octet length

// BVLC functions (Annex J).
#define BVLC_FUNC_RESULT 0x00
#define BVLC_FUNC_WRITE_BDT 0x01
#define BVLC_FUNC_FORWARDED_NPDU 0x04
#define BVLC_FUNC_REGISTER_FD 0x05
#define BVLC_FUNC_ORIGINAL_UNICAST 0x0A
#define BVLC_FUNC_ORIGINAL_BROADCAST 0x0B

#define NPDU_VERSION 0x01 ///< ASCII 1, the only defined protocol version

// NPCI control-octet bits (Clause 6.2.2).
#define NPCI_NETWORK_MSG 0x80     ///< NSDU is a network-layer message (else an APDU)
#define NPCI_DEST_PRESENT 0x20    ///< DNET / DLEN / DADR present
#define NPCI_SRC_PRESENT 0x08     ///< SNET / SLEN / SADR present
#define NPCI_EXPECTING_REPLY 0x04 ///< a reply is expected
#define NPCI_PRIORITY_MASK 0x03   ///< message priority (low 2 bits)

// Message priorities.
#define NPDU_PRIO_NORMAL 0x00
#define NPDU_PRIO_URGENT 0x01
#define NPDU_PRIO_CRITICAL 0x02
#define NPDU_PRIO_LIFE_SAFETY 0x03

// ---- BVLC ----

/** @brief Wrap an NPDU in a BVLC envelope. Returns total octets, or 0 on overflow. */
size_t dws_bvlc_build(uint8_t *buf, size_t cap, uint8_t function, const uint8_t *npdu, size_t dws_npdu_len);

/** @brief Parse a BVLC envelope; reports the function and the NPDU slice. */
bool dws_bvlc_parse(const uint8_t *buf, size_t len, uint8_t *function, const uint8_t **npdu, size_t *dws_npdu_len);

// ---- NPDU ----

/**
 * @brief Build an NPDU carrying @p apdu. With @p has_dest, the destination addressing
 *        (DNET / DLEN / DADR) and the hop count are emitted (DLEN 0 + @p dnet 0xFFFF is a
 *        remote/global broadcast).
 */
size_t dws_npdu_build(uint8_t *buf, size_t cap, bool expecting_reply, uint8_t priority, bool has_dest, uint16_t dnet,
                      const uint8_t *dadr, uint8_t dadr_len, uint8_t hop_count, const uint8_t *apdu, size_t apdu_len);

/** @brief A parsed NPDU. @ref apdu points INTO the source buffer. */
struct NpduInfo
{
    uint8_t control;
    bool network_message; ///< control & 0x80
    bool dest_present;
    uint16_t dnet;
    bool src_present;
    uint16_t snet;
    uint8_t hop_count; ///< valid when dest_present
    const uint8_t *apdu;
    size_t apdu_len;
};

/** @brief Parse + validate an NPDU (version, control, optional addressing) and slice the APDU. */
bool dws_npdu_parse(const uint8_t *buf, size_t len, NpduInfo *out);

// --- APDU header (the application layer sliced out by dws_npdu_parse) ---

// PDU types (the high nibble of the first APDU octet).
#define BACNET_PDU_CONFIRMED_REQUEST 0
#define BACNET_PDU_UNCONFIRMED_REQUEST 1
#define BACNET_PDU_SIMPLE_ACK 2
#define BACNET_PDU_COMPLEX_ACK 3
#define BACNET_PDU_SEGMENT_ACK 4
#define BACNET_PDU_ERROR 5
#define BACNET_PDU_REJECT 6
#define BACNET_PDU_ABORT 7

// PDU flags (the low nibble of the first octet, on confirmed-request / complex-ack).
#define BACNET_APDU_SEG 0x08 ///< the message is segmented
#define BACNET_APDU_MOR 0x04 ///< more segments follow
#define BACNET_APDU_SA 0x02  ///< the sender accepts a segmented response (confirmed-request only)

/** @brief A decoded APDU header (from dws_apdu_parse). Service data points INTO the source buffer. */
struct BacnetApdu
{
    uint8_t pdu_type;            ///< PDU type (BACNET_PDU_*)
    bool segmented;              ///< SEG flag (confirmed-request / complex-ack)
    bool more_follows;           ///< MOR flag
    bool sa;                     ///< segmented-response-accepted flag (confirmed-request)
    uint8_t invoke_id;           ///< invoke id (confirmed-request / simple-ack / complex-ack)
    uint8_t service_choice;      ///< service choice
    const uint8_t *service_data; ///< the service parameters after the header, or nullptr if none
    size_t service_data_len;     ///< octets remaining after the header
};

/**
 * @brief Decode an APDU header (PDU type, flags, invoke id, service choice) and slice the service data.
 * @return true iff @p len covers the header for a supported PDU type (confirmed / unconfirmed request,
 *         simple / complex ACK); false for a short buffer or an unsupported type (segment-ack / error /
 *         reject / abort).
 */
bool dws_apdu_parse(const uint8_t *apdu, size_t len, BacnetApdu *out);

#endif // DWS_ENABLE_BACNET

#endif // DETERMINISTICESPASYNCWEBSERVER_BACNET_H
