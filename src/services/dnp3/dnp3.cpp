// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dnp3.cpp
 * @brief DNP3 (IEEE 1815) data-link frame builder + parser (pure, host-tested).
 */

#include "services/dnp3/dnp3.h"
#include "shared_primitives/crc.h" // DWS_CRC16_DNP

#if DWS_ENABLE_DNP3

#include <string.h>

uint16_t dws_dnp3_crc(const uint8_t *data, size_t len)
{
    // The DNP3 link-layer block check: reflected poly 0xA6BC = reflect(0x3D65), init 0, final XOR
    // 0xFFFF - catalogued as CRC-16/DNP. test_crc diffs the shared engine against the loop that used
    // to live here over every length 0..64, so this is byte-identical to it.
    return (uint16_t)dws_crc(&DWS_CRC16_DNP, data, len);
}

// Append a CRC over [data, data+n) low octet first.
static size_t put_crc(uint8_t *p, const uint8_t *data, size_t n)
{
    uint16_t crc = dws_dnp3_crc(data, n);
    p[0] = (uint8_t)(crc & 0xFF);
    p[1] = (uint8_t)(crc >> 8);
    return DNP3_CRC_LEN;
}

size_t dws_dnp3_build_frame(uint8_t *buf, size_t cap, uint8_t control, uint16_t dest, uint16_t src,
                            const uint8_t *user_data, size_t user_data_len)
{
    if (!buf || (user_data_len && !user_data) || user_data_len > DNP3_MAX_USER_DATA)
        return 0;
    size_t nblocks = (user_data_len + DNP3_BLOCK_LEN - 1) / DNP3_BLOCK_LEN;
    size_t total = DNP3_HEADER_BLOCK_LEN + user_data_len + nblocks * DNP3_CRC_LEN;
    if (total > cap)
        return 0;

    size_t p = 0;
    buf[p++] = DNP3_START0;
    buf[p++] = DNP3_START1;
    buf[p++] = (uint8_t)(DNP3_LEN_OVERHEAD + user_data_len); // LEN counts CTRL + DEST + SRC + data
    buf[p++] = control;
    buf[p++] = (uint8_t)(dest & 0xFF); // LE
    buf[p++] = (uint8_t)(dest >> 8);
    buf[p++] = (uint8_t)(src & 0xFF);
    buf[p++] = (uint8_t)(src >> 8);
    p += put_crc(buf + p, buf, DNP3_HEADER_LEN); // header CRC over the header octets

    size_t off = 0;
    while (off < user_data_len)
    {
        size_t blk = user_data_len - off;
        if (blk > DNP3_BLOCK_LEN)
            blk = DNP3_BLOCK_LEN;
        memcpy(buf + p, user_data + off, blk);
        put_crc(buf + p + blk, buf + p, blk); // CRC over this block's data
        p += blk + DNP3_CRC_LEN;
        off += blk;
    }
    return total;
}

bool dws_dnp3_parse_frame(const uint8_t *buf, size_t len, Dnp3Frame *out, uint8_t *out_user, size_t out_cap,
                          size_t *out_user_len)
{
    if (!buf || !out || len < DNP3_HEADER_BLOCK_LEN)
        return false;
    if (buf[0] != DNP3_START0 || buf[1] != DNP3_START1)
        return false;
    uint8_t length = buf[2];
    if (length < DNP3_LEN_OVERHEAD) // LEN must at least cover CTRL + DEST + SRC
        return false;
    size_t user_len = (size_t)length - DNP3_LEN_OVERHEAD;
    if (user_len > DNP3_MAX_USER_DATA) // GCOVR_EXCL_BR_LINE  unreachable: the line-76 guard above already
                                       // forced length into [DNP3_LEN_OVERHEAD, 255] (length is a uint8_t, so
                                       // 255 is a hard ceiling, not just an untested one), so this unsigned
                                       // subtraction cannot underflow and user_len = length-DNP3_LEN_OVERHEAD is
                                       // confined to [0, 250] = [0, DNP3_MAX_USER_DATA]; the true arm has no
                                       // reachable value of length to take it with
        return false; // GCOVR_EXCL_LINE  unreachable: see the branch comment above - user_len can never exceed
                      // DNP3_MAX_USER_DATA given length's uint8_t domain plus the line-76 lower-bound guard

    uint16_t hcrc = dws_dnp3_crc(buf, DNP3_HEADER_LEN);
    if ((uint16_t)(buf[DNP3_HEADER_LEN] | (buf[DNP3_HEADER_LEN + 1] << 8)) != hcrc)
        return false; // header CRC mismatch

    size_t nblocks = (user_len + DNP3_BLOCK_LEN - 1) / DNP3_BLOCK_LEN;
    size_t total = DNP3_HEADER_BLOCK_LEN + user_len + nblocks * DNP3_CRC_LEN;
    if (total > len)
        return false; // frame not fully buffered

    if (user_len && (!out_user || user_len > out_cap))
        return false;

    size_t p = DNP3_HEADER_BLOCK_LEN;
    size_t off = 0;
    while (off < user_len)
    {
        size_t blk = user_len - off;
        if (blk > DNP3_BLOCK_LEN)
            blk = DNP3_BLOCK_LEN;
        uint16_t bcrc = dws_dnp3_crc(buf + p, blk);
        if ((uint16_t)(buf[p + blk] | (buf[p + blk + 1] << 8)) != bcrc)
            return false; // block CRC mismatch
        memcpy(out_user + off, buf + p, blk);
        p += blk + DNP3_CRC_LEN;
        off += blk;
    }

    out->length = length;
    out->control = buf[3];
    out->dest = (uint16_t)(buf[4] | (buf[5] << 8));
    out->src = (uint16_t)(buf[6] | (buf[7] << 8));
    if (out_user_len)
        *out_user_len = user_len;
    return true;
}

#endif // DWS_ENABLE_DNP3
