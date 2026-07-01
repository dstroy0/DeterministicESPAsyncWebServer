// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file cip.h
 * @brief CIP (Common Industrial Protocol) message codec (DETWS_ENABLE_CIP) - zero-heap
 *        request builder + response parser for the message that rides inside an EtherNet/IP
 *        Unconnected Data item (services/enip). Together they form a working CIP read path.
 *
 * A CIP message request is:
 * @code
 *   Service(1)  RequestPathSize(1, in 16-bit words)  RequestPath(EPATH)  ServiceData
 * @endcode
 * The EPATH addresses an object with logical segments. A logical segment byte is
 * `0x20 | logical-type | format`, where logical-type is class (0x00), instance (0x04), or
 * attribute (0x10), and format is 8-bit (0x00, then a 1-octet id) or 16-bit (0x01, then a
 * pad octet and a little-endian 2-octet id). A response is `Service|0x80  reserved(0)
 * GeneralStatus(1)  AdditionalStatusSize(1, words)  [additional status]  ServiceData`.
 *
 * Service codes + the logical-segment encoding are verified against the Wireshark CIP
 * dissector. This codec is the CIP message; wrap it with `eip_build_send_rr_data`.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CIP_H
#define DETERMINISTICESPASYNCWEBSERVER_CIP_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_CIP

#include <stddef.h>
#include <stdint.h>

// Common service codes.
#define CIP_SC_GET_ATTR_ALL 0x01
#define CIP_SC_GET_ATTR_LIST 0x03
#define CIP_SC_SET_ATTR_LIST 0x04
#define CIP_SC_GET_ATTR_SINGLE 0x0E
#define CIP_SC_SET_ATTR_SINGLE 0x10
#define CIP_REPLY_FLAG 0x80 ///< OR'd into the service code in a reply

// Logical-segment EPATH encoding (segment byte = base | logical-type | format).
#define CIP_SEG_LOGICAL 0x20   ///< logical segment, segment-type bits
#define CIP_SEG_CLASS 0x00     ///< logical type: class id
#define CIP_SEG_INSTANCE 0x04  ///< logical type: instance id
#define CIP_SEG_ATTRIBUTE 0x10 ///< logical type: attribute id
#define CIP_SEG_8BIT 0x00      ///< format: an 8-bit id follows
#define CIP_SEG_16BIT 0x01     ///< format: a pad octet then a 16-bit (LE) id follows

#define CIP_STATUS_SUCCESS 0x00 ///< General Status: success

/**
 * @brief Build a class/instance[/attribute] EPATH (logical segments) into @p buf.
 * @param with_attribute include the attribute segment.
 * @return EPATH length in octets (always even / word-aligned), or 0 on overflow.
 */
size_t cip_build_epath(uint8_t *buf, size_t cap, uint16_t class_id, uint16_t instance_id, uint16_t attribute_id,
                       bool with_attribute);

/** @brief Build a CIP request: service + path size (words) + EPATH + service data. */
size_t cip_build_request(uint8_t *buf, size_t cap, uint8_t service, const uint8_t *epath, size_t epath_len,
                         const uint8_t *data, size_t data_len);

/** @brief Build a Get_Attribute_Single request for class/instance/attribute. */
size_t cip_build_get_attr_single(uint8_t *buf, size_t cap, uint16_t class_id, uint16_t instance_id,
                                 uint16_t attribute_id);

/** @brief A parsed CIP response. @ref data points INTO the source buffer. */
struct CipResponse
{
    uint8_t service;        ///< reply service (the 0x80 reply bit is set)
    uint8_t general_status; ///< CIP_STATUS_SUCCESS on success
    const uint8_t *data;    ///< service data (the attribute value on a read)
    size_t data_len;
};

/** @brief Parse a CIP response (service + status + additional status + data). */
bool cip_parse_response(const uint8_t *buf, size_t len, CipResponse *out);

#endif // DETWS_ENABLE_CIP

#endif // DETERMINISTICESPASYNCWEBSERVER_CIP_H
