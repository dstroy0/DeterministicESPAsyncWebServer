// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file bacnet.h
 * @brief BACnet/IP BVLC + NPDU codec (DETWS_ENABLE_BACNET) - zero-heap framing for the
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

#if DETWS_ENABLE_BACNET

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
size_t bvlc_build(uint8_t *buf, size_t cap, uint8_t function, const uint8_t *npdu, size_t npdu_len);

/** @brief Parse a BVLC envelope; reports the function and the NPDU slice. */
bool bvlc_parse(const uint8_t *buf, size_t len, uint8_t *function, const uint8_t **npdu, size_t *npdu_len);

// ---- NPDU ----

/**
 * @brief Build an NPDU carrying @p apdu. With @p has_dest, the destination addressing
 *        (DNET / DLEN / DADR) and the hop count are emitted (DLEN 0 + @p dnet 0xFFFF is a
 *        remote/global broadcast).
 */
size_t npdu_build(uint8_t *buf, size_t cap, bool expecting_reply, uint8_t priority, bool has_dest, uint16_t dnet,
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
bool npdu_parse(const uint8_t *buf, size_t len, NpduInfo *out);

#endif // DETWS_ENABLE_BACNET

#endif // DETERMINISTICESPASYNCWEBSERVER_BACNET_H
