// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file amqp.h
 * @brief AMQP 0-9-1 frame codec (DWS_ENABLE_AMQP) - zero-heap frame builder + parser for
 *        the RabbitMQ wire protocol, so a device can be an AMQP client over the shipped
 *        outbound client transport.
 *
 * The connection opens with an 8-octet protocol header (`"AMQP" 0 0 9 1`). After that every
 * frame is:
 * @code
 *   type(1)  channel(2, big-endian)  size(4, big-endian)  payload(size)  frame-end(1)=0xCE
 * @endcode
 *  - Frame types: 1 METHOD, 2 HEADER (content header), 3 BODY (content body), 8 HEARTBEAT.
 *  - A method frame's payload is `class-id(2) method-id(2)` then the method arguments.
 *  - A heartbeat is type 8 on channel 0 with an empty payload.
 *
 * The builders frame a payload into a caller buffer (fail-closed, validating the 0xCE end on
 * parse); the method arguments are the application's. Frame format per the AMQP 0-9-1 spec.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_AMQP_H
#define DETERMINISTICESPASYNCWEBSERVER_AMQP_H

#include "ServerConfig.h"

#if DWS_ENABLE_AMQP

#include <stddef.h>
#include <stdint.h>

// Frame types (octet 0).
#define AMQP_FRAME_METHOD 1
#define AMQP_FRAME_HEADER 2 ///< content header
#define AMQP_FRAME_BODY 3   ///< content body
#define AMQP_FRAME_HEARTBEAT 8

#define AMQP_FRAME_END 0xCE   ///< the mandatory frame terminator octet
#define AMQP_FRAME_OVERHEAD 8 ///< type(1) + channel(2) + size(4) + frame-end(1)

/** @brief Write the 8-octet protocol header ("AMQP" + 0 0 9 1). Returns 8, or 0 on overflow. */
size_t amqp_protocol_header(uint8_t *buf, size_t cap);

/** @brief Build a frame: type + channel + size + payload + 0xCE. Returns total octets, or 0. */
size_t amqp_build_frame(uint8_t *buf, size_t cap, uint8_t type, uint16_t channel, const uint8_t *payload,
                        size_t payload_len);

/** @brief Build a METHOD frame: payload = class-id + method-id + @p args. */
size_t amqp_build_method(uint8_t *buf, size_t cap, uint16_t channel, uint16_t class_id, uint16_t method_id,
                         const uint8_t *args, size_t args_len);

/** @brief Build a heartbeat frame (type 8, channel 0, empty payload). Returns 8, or 0. */
size_t amqp_build_heartbeat(uint8_t *buf, size_t cap);

/** @brief A parsed frame. @ref payload points INTO the source buffer. */
struct AmqpFrame
{
    uint8_t type;
    uint16_t channel;
    const uint8_t *payload;
    size_t payload_len;
};

/**
 * @brief Parse one frame at the head of [buf, buf+len), validating the 0xCE frame-end.
 * @param consumed receives the full frame length so the caller can advance.
 * @return true on a complete, terminated frame; false if incomplete or the end octet is wrong.
 */
bool amqp_parse_frame(const uint8_t *buf, size_t len, AmqpFrame *out, size_t *consumed);

/** @brief Split a METHOD frame payload into class-id / method-id / arguments. */
bool amqp_parse_method(const uint8_t *payload, size_t payload_len, uint16_t *class_id, uint16_t *method_id,
                       const uint8_t **args, size_t *args_len);

#endif // DWS_ENABLE_AMQP

#endif // DETERMINISTICESPASYNCWEBSERVER_AMQP_H
