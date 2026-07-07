// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dds.h
 * @brief DDS / RTPS wire-protocol codec (DETWS_ENABLE_DDS).
 *
 * DDS (OMG Data Distribution Service) publishes on the wire as **RTPS** (DDSI-RTPS, the Real-Time
 * Publish-Subscribe protocol), normally over UDP multicast. An RTPS **message** is a 20-octet header
 * followed by a sequence of **submessages**:
 *
 *   Header (20): "RTPS" | protocol version (2) | vendorId (2) | guidPrefix (12)
 *   Submessage:  submessageId (1) | flags (1) | octetsToNextHeader (2, endian per the E flag) | body
 *
 * This is the message + submessage framing codec: `detws_rtps_header` / `detws_rtps_submessage` build
 * them and `detws_rtps_parse` validates the header (magic + version) and walks the submessages,
 * surfacing each via a callback. The per-submessage bodies (DATA serialized-payload/CDR, HEARTBEAT
 * sequence-number sets, the discovery SPDP/SEDP topics) layer on top of this framing.
 *
 * Pure, zero heap, no stdlib, host-testable.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DDS_H
#define DETERMINISTICESPASYNCWEBSERVER_DDS_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_DDS

/** @brief RTPS submessage kinds (DDSI-RTPS 8.3.7) + the flag bit for little-endian. */
enum
{
    RTPS_SM_PAD = 0x01,
    RTPS_SM_ACKNACK = 0x06,
    RTPS_SM_HEARTBEAT = 0x07,
    RTPS_SM_GAP = 0x08,
    RTPS_SM_INFO_TS = 0x09,
    RTPS_SM_INFO_SRC = 0x0c,
    RTPS_SM_INFO_REPLY_IP4 = 0x0d,
    RTPS_SM_INFO_DST = 0x0e,
    RTPS_SM_INFO_REPLY = 0x0f,
    RTPS_SM_DATA = 0x15,
    RTPS_SM_DATA_FRAG = 0x16,
    RTPS_FLAG_ENDIAN = 0x01, ///< E flag: submessage (and header) fields are little-endian.
    RTPS_HEADER_LEN = 20,
    RTPS_GUIDPREFIX_LEN = 12
};

/** @brief RTPS protocol version carried in the header (major, minor). */
extern const uint8_t RTPS_VERSION[2]; ///< {2, 4}.

/**
 * @brief Build the 20-octet RTPS message header.
 * @param guid_prefix 12-byte participant GUID prefix.
 * @param vendor_id   2-byte vendor id (0x0000 = unknown).
 * @return 20, or 0 if @p cap < 20 or a pointer is null.
 */
size_t detws_rtps_header(const uint8_t *guid_prefix, const uint8_t *vendor_id, uint8_t *out, size_t cap);

/**
 * @brief Build one RTPS submessage `[id][flags][octetsToNextHeader][body]`.
 * @param id       RTPS_SM_*.
 * @param flags    submessage flags (bit 0 = little-endian; set RTPS_FLAG_ENDIAN for LE bodies).
 * @param body     submessage contents (may be null when body_len == 0).
 * @param body_len contents length (the octetsToNextHeader value).
 * @return 4 + body_len bytes written, or 0 if it won't fit.
 */
size_t detws_rtps_submessage(uint8_t id, uint8_t flags, const uint8_t *body, uint16_t body_len, uint8_t *out,
                             size_t cap);

/** @brief One submessage surfaced by detws_rtps_parse. */
typedef void (*DetwsRtpsCb)(uint8_t id, uint8_t flags, const uint8_t *body, size_t body_len, void *arg);

/**
 * @brief Validate an RTPS message header and walk its submessages.
 * @return true if the header is a well-formed RTPS message (magic + version <= ours) and every
 *         submessage fits. An octetsToNextHeader of 0 on the last submessage means "to end of message".
 */
bool detws_rtps_parse(const uint8_t *msg, size_t len, DetwsRtpsCb cb, void *arg);

#endif // DETWS_ENABLE_DDS
#endif // DETERMINISTICESPASYNCWEBSERVER_DDS_H
