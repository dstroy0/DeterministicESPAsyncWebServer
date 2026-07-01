// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file amqp.cpp
 * @brief AMQP 0-9-1 frame builder + parser (pure, host-tested).
 */

#include "services/amqp/amqp.h"

#if DETWS_ENABLE_AMQP

#include <string.h>

// AMQP integers are big-endian (network order).
static size_t put16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
    return 2;
}

static size_t put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
    return 4;
}

static uint16_t get16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t get32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

size_t amqp_protocol_header(uint8_t *buf, size_t cap)
{
    static const uint8_t hdr[8] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    if (!buf || cap < sizeof(hdr))
        return 0;
    memcpy(buf, hdr, sizeof(hdr));
    return sizeof(hdr);
}

// Write a frame header (type, channel, size) at buf; the caller fills the payload + 0xCE.
static size_t write_frame_header(uint8_t *buf, uint8_t type, uint16_t channel, uint32_t size)
{
    size_t p = 0;
    buf[p++] = type;
    p += put16(buf + p, channel);
    p += put32(buf + p, size);
    return p; // 7
}

size_t amqp_build_frame(uint8_t *buf, size_t cap, uint8_t type, uint16_t channel, const uint8_t *payload,
                        size_t payload_len)
{
    if (!buf || (payload_len && !payload) || payload_len > 0xFFFFFFFFu)
        return 0;
    size_t total = AMQP_FRAME_OVERHEAD + payload_len;
    if (total > cap)
        return 0;
    size_t p = write_frame_header(buf, type, channel, (uint32_t)payload_len);
    if (payload_len)
    {
        memcpy(buf + p, payload, payload_len);
        p += payload_len;
    }
    buf[p++] = AMQP_FRAME_END;
    return p;
}

size_t amqp_build_method(uint8_t *buf, size_t cap, uint16_t channel, uint16_t class_id, uint16_t method_id,
                         const uint8_t *args, size_t args_len)
{
    if (!buf || (args_len && !args))
        return 0;
    size_t payload_len = 4 + args_len; // class-id + method-id + args
    size_t total = AMQP_FRAME_OVERHEAD + payload_len;
    if (total > cap)
        return 0;
    // Write directly into buf (no temp): header, then the method payload, then the 0xCE end.
    size_t p = write_frame_header(buf, AMQP_FRAME_METHOD, channel, (uint32_t)payload_len);
    p += put16(buf + p, class_id);
    p += put16(buf + p, method_id);
    if (args_len)
    {
        memcpy(buf + p, args, args_len);
        p += args_len;
    }
    buf[p++] = AMQP_FRAME_END;
    return p;
}

size_t amqp_build_heartbeat(uint8_t *buf, size_t cap)
{
    return amqp_build_frame(buf, cap, AMQP_FRAME_HEARTBEAT, 0, nullptr, 0);
}

bool amqp_parse_frame(const uint8_t *buf, size_t len, AmqpFrame *out, size_t *consumed)
{
    if (!buf || !out || len < AMQP_FRAME_OVERHEAD)
        return false;
    uint32_t size = get32(buf + 3);
    // Compare against the remaining capacity without adding (a 32-bit size_t would wrap if we
    // computed 8 + size first), so an attacker-controlled size can't slip past the bound.
    if (size > len - AMQP_FRAME_OVERHEAD)
        return false;                                  // not fully buffered
    size_t total = AMQP_FRAME_OVERHEAD + (size_t)size; // header(7) + payload + frame-end(1)
    if (buf[7 + size] != AMQP_FRAME_END)
        return false; // missing / corrupt frame terminator
    out->type = buf[0];
    out->channel = get16(buf + 1);
    out->payload = buf + 7;
    out->payload_len = size;
    if (consumed)
        *consumed = total;
    return true;
}

bool amqp_parse_method(const uint8_t *payload, size_t payload_len, uint16_t *class_id, uint16_t *method_id,
                       const uint8_t **args, size_t *args_len)
{
    if (!payload || payload_len < 4)
        return false;
    if (class_id)
        *class_id = get16(payload);
    if (method_id)
        *method_id = get16(payload + 2);
    if (args)
        *args = payload + 4;
    if (args_len)
        *args_len = payload_len - 4;
    return true;
}

#endif // DETWS_ENABLE_AMQP
