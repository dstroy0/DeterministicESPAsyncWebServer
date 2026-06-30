// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file melsec.cpp
 * @brief Mitsubishi MELSEC MC binary 3E builder + parser (pure, host-tested).
 */

#include "services/melsec/melsec.h"

#if DETWS_ENABLE_MELSEC

// MC binary multi-octet fields are little-endian.
static size_t put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)(v >> 8);
    return 2;
}

static uint16_t get16le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

size_t melsec_build_read(uint8_t *buf, size_t cap, uint8_t device_code, uint32_t head_device, uint16_t points,
                         uint16_t monitoring_timer)
{
    if (!buf || cap < MELSEC_3E_READ_REQ_LEN)
        return 0;
    size_t p = 0;
    buf[p++] = MELSEC_3E_REQ_SUBHEADER0;
    buf[p++] = MELSEC_3E_REQ_SUBHEADER1;
    buf[p++] = MELSEC_NETWORK_DEFAULT;
    buf[p++] = MELSEC_PC_DEFAULT;
    p += put16le(buf + p, MELSEC_DEST_IO_DEFAULT);
    buf[p++] = MELSEC_DEST_MULTIDROP_DEFAULT;
    // request data length = the octets from the monitoring timer onward:
    // timer(2) + command(2) + subcommand(2) + head device(3) + device code(1) + points(2) = 12
    p += put16le(buf + p, MELSEC_3E_READ_REQ_DATA_LEN);
    p += put16le(buf + p, monitoring_timer);
    p += put16le(buf + p, MELSEC_CMD_BATCH_READ);
    p += put16le(buf + p, MELSEC_SUBCMD_WORD);
    buf[p++] = (uint8_t)(head_device & 0xFF); // head device number, 3 octets little-endian
    buf[p++] = (uint8_t)((head_device >> 8) & 0xFF);
    buf[p++] = (uint8_t)((head_device >> 16) & 0xFF);
    buf[p++] = device_code;
    p += put16le(buf + p, points);
    return p; // == MELSEC_3E_READ_REQ_LEN
}

bool melsec_parse_response(const uint8_t *buf, size_t len, MelsecResponse *out)
{
    // subheader(2)+net(1)+pc(1)+io(2)+multidrop(1)+length(2)+endcode(2) = MELSEC_3E_RES_MIN_LEN
    if (!buf || !out || len < MELSEC_3E_RES_MIN_LEN)
        return false;
    if (buf[0] != MELSEC_3E_RES_SUBHEADER0 || buf[1] != MELSEC_3E_RES_SUBHEADER1)
        return false;
    uint16_t data_length = get16le(buf + MELSEC_3E_RES_LEN_OFFSET); // covers the end code + the response data
    if (data_length < MELSEC_ENDCODE_LEN)
        return false;
    if (MELSEC_3E_RES_DATALEN_BASE + (size_t)data_length > len)
        return false;
    out->end_code = get16le(buf + MELSEC_3E_RES_DATALEN_BASE);
    out->data = buf + MELSEC_3E_RES_DATA_OFFSET;
    out->data_len = (size_t)data_length - MELSEC_ENDCODE_LEN; // minus the 2-octet end code
    return true;
}

#endif // DETWS_ENABLE_MELSEC
