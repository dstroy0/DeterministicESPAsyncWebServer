// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file wal.cpp
 * @brief Write-ahead journal record framing + crash-recovery replay (see wal.h).
 */

#include "services/wal/wal.h"

#if DETWS_ENABLE_WAL

#include <string.h>

namespace
{
// CRC-32 (IEEE 802.3) core: no init/final xor, so it composes across a header + payload in two steps.
uint32_t crc32_step(uint32_t crc, const uint8_t *d, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        crc ^= d[i];
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return crc;
}

void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}
void put_u64(uint8_t *p, uint64_t v)
{
    for (int i = 0; i < 8; i++)
        p[i] = (uint8_t)(v >> (8 * i));
}
uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
uint64_t get_u64(const uint8_t *p)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
        v |= (uint64_t)p[i] << (8 * i);
    return v;
}
} // namespace

uint32_t wal_crc32(const uint8_t *data, size_t len)
{
    return crc32_step(0xFFFFFFFFu, data, len) ^ 0xFFFFFFFFu;
}

size_t wal_record_encode(uint8_t *out, size_t cap, uint64_t seq, const uint8_t *payload, uint32_t len)
{
    size_t need = (size_t)WAL_RECORD_HEADER + len;
    if (out == nullptr || need > cap)
        return 0;
    put_u32(out + 0, WAL_MAGIC);
    put_u64(out + 4, seq);
    put_u32(out + 12, len);
    // crc over the 16 header bytes (magic+seq+len) then the payload
    uint32_t crc = crc32_step(0xFFFFFFFFu, out, 16);
    crc = crc32_step(crc, payload, len) ^ 0xFFFFFFFFu;
    put_u32(out + 16, crc);
    if (len)
        memcpy(out + WAL_RECORD_HEADER, payload, len);
    return need;
}

size_t wal_replay(const uint8_t *img, size_t len, WalRecordCb cb, void *ctx)
{
    size_t off = 0;
    while (off + WAL_RECORD_HEADER <= len)
    {
        const uint8_t *r = img + off;
        if (get_u32(r) != WAL_MAGIC)
            break; // not a record start (end of log or garbage)
        uint64_t seq = get_u64(r + 4);
        uint32_t plen = get_u32(r + 12);
        uint32_t crc_stored = get_u32(r + 16);
        if (off + (size_t)WAL_RECORD_HEADER + plen > len)
            break; // truncated tail (power loss mid-record)
        uint32_t crc = crc32_step(0xFFFFFFFFu, r, 16);
        crc = crc32_step(crc, r + WAL_RECORD_HEADER, plen) ^ 0xFFFFFFFFu;
        if (crc != crc_stored)
            break; // torn / corrupt record - stop here, this is the durable end
        if (cb)
            cb(seq, r + WAL_RECORD_HEADER, plen, ctx);
        off += (size_t)WAL_RECORD_HEADER + plen;
    }
    return off;
}

#endif // DETWS_ENABLE_WAL
