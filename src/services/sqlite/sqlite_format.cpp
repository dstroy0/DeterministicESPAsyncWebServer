// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sqlite_format.cpp
 * @brief SQLite3 on-disk file-format parsers (see sqlite_format.h).
 */

#include "services/sqlite/sqlite_format.h"

#if DETWS_ENABLE_SQLITE

#include <string.h>

namespace
{
uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
// The 16-byte file magic, including the trailing NUL.
const char SQLITE_MAGIC[16] = {'S', 'Q', 'L', 'i', 't', 'e', ' ', 'f', 'o', 'r', 'm', 'a', 't', ' ', '3', '\0'};

bool is_pow2(uint32_t v)
{
    return v && (v & (v - 1)) == 0;
}
} // namespace

size_t sqlite_varint_decode(const uint8_t *buf, size_t len, uint64_t *out)
{
    uint64_t v = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (i >= len)
            return 0; // incomplete varint
        v = (v << 7) | (uint8_t)(buf[i] & 0x7f);
        if ((buf[i] & 0x80) == 0)
        {
            *out = v;
            return i + 1;
        }
    }
    if (len < 9)
        return 0;
    v = (v << 8) | buf[8]; // 9th byte carries all 8 bits
    *out = v;
    return 9;
}

uint64_t sqlite_serial_type_size(uint64_t t)
{
    switch (t)
    {
    case 0:
        return 0; // NULL
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 6;
    case 6:
        return 8;
    case 7:
        return 8; // IEEE 754 float64
    case 8:
        return 0; // integer 0
    case 9:
        return 0; // integer 1
    case 10:
    case 11:
        return 0; // reserved for internal use
    default:
        // >=12 even -> BLOB of (t-12)/2; >=13 odd -> TEXT of (t-13)/2. Floor division covers both.
        return t >= 12 ? (t - 12) / 2 : 0;
    }
}

bool sqlite_parse_db_header(const uint8_t *buf, size_t len, SqliteDbHeader *out)
{
    if (len < 100 || memcmp(buf, SQLITE_MAGIC, 16) != 0)
        return false;

    uint16_t raw_ps = be16(buf + 16);
    uint32_t page_size = (raw_ps == 1) ? 65536u : raw_ps;
    // A valid page size is a power of two in [512, 65536].
    if (page_size < 512 || page_size > 65536 || !is_pow2(page_size))
        return false;

    out->page_size = page_size;
    out->write_version = buf[18];
    out->read_version = buf[19];
    out->reserved_per_page = buf[20];
    out->file_change_counter = be32(buf + 24);
    out->page_count = be32(buf + 28);
    out->freelist_first = be32(buf + 32);
    out->freelist_count = be32(buf + 36);
    out->schema_cookie = be32(buf + 40);
    out->schema_format = be32(buf + 44);
    out->text_encoding = be32(buf + 56);
    out->user_version = be32(buf + 60);
    out->application_id = be32(buf + 68);
    out->sqlite_version = be32(buf + 96);
    return true;
}

bool sqlite_parse_btree_header(const uint8_t *page, size_t page_len, size_t offset, SqliteBtreeHeader *out)
{
    if (offset + 8 > page_len)
        return false;
    const uint8_t *p = page + offset;
    uint8_t type = p[0];
    bool interior = (type == SQLITE_BTREE_INTERIOR_INDEX || type == SQLITE_BTREE_INTERIOR_TABLE);
    bool leaf = (type == SQLITE_BTREE_LEAF_INDEX || type == SQLITE_BTREE_LEAF_TABLE);
    if (!interior && !leaf)
        return false;
    uint8_t hdr = interior ? 12 : 8;
    if (offset + hdr > page_len)
        return false;

    out->type = type;
    out->first_freeblock = be16(p + 1);
    out->cell_count = be16(p + 3);
    uint16_t ccs = be16(p + 5);
    out->cell_content_start = (ccs == 0) ? 65536u : ccs;
    out->frag_free_bytes = p[7];
    out->right_most_page = interior ? be32(p + 8) : 0;
    out->header_size = hdr;
    return true;
}

#endif // DETWS_ENABLE_SQLITE
