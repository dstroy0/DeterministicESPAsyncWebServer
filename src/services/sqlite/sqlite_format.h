// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sqlite_format.h
 * @brief Reader for the SQLite3 on-disk file format (DETWS_ENABLE_SQLITE).
 *
 * This is **file-format access**, not the SQLite library: the documented database file structure
 * (https://www.sqlite.org/fileformat2.html) parsed by hand - the 100-byte database header, the b-tree
 * page header, the record varint, and record serial types. It is read-first (a bounded writer is a later
 * step), pure (no I/O; you hand it page bytes), and host-testable against real files produced by the
 * sqlite3 CLI. It deliberately does not pull in the SQLite amalgamation, which needs a heap and stdio and
 * does not fit the no-stdlib zero-heap model.
 *
 * A caller reads a page (::SqliteDbHeader::page_size bytes) from the backing store - e.g. via wal_fs /
 * fs::FS - and walks it with these parsers: page 1 begins with the database header (100 bytes) immediately
 * followed by the root b-tree page header of the schema table.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SQLITE_FORMAT_H
#define DETERMINISTICESPASYNCWEBSERVER_SQLITE_FORMAT_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_SQLITE

/**
 * @brief Decode a SQLite variable-length integer (1-9 bytes, big-endian; the high bit of each of the first
 * 8 bytes is a continuation flag, the 9th byte contributes all 8 bits).
 * @return the number of bytes consumed (1-9), or 0 if @p len is too short for a complete varint.
 */
size_t sqlite_varint_decode(const uint8_t *buf, size_t len, uint64_t *out);

/**
 * @brief Content byte size of a record column with the given record serial type.
 *
 * 0 -> NULL (0), 1..6 -> 1/2/3/4/6/8-byte ints, 7 -> 8-byte float, 8/9 -> the constants 0/1 (0 bytes),
 * N>=12 even -> BLOB of (N-12)/2 bytes, N>=13 odd -> TEXT of (N-13)/2 bytes. Reserved 10/11 -> 0.
 */
uint64_t sqlite_serial_type_size(uint64_t serial_type);

/** @brief Parsed subset of the 100-byte database header (all fields are big-endian on media). */
struct SqliteDbHeader
{
    uint32_t page_size;           ///< 512..65536 (the on-disk value 1 means 65536)
    uint8_t write_version;        ///< 1 = legacy (rollback journal), 2 = WAL
    uint8_t read_version;         ///< 1 = legacy, 2 = WAL
    uint8_t reserved_per_page;    ///< reserved bytes at the end of each page
    uint32_t file_change_counter; ///< bumped on each transaction
    uint32_t page_count;          ///< database size in pages (valid when it matches version-valid-for)
    uint32_t freelist_first;      ///< first freelist trunk page (0 = none)
    uint32_t freelist_count;      ///< number of freelist pages
    uint32_t schema_cookie;       ///< schema version cookie
    uint32_t schema_format;       ///< schema format number (1..4)
    uint32_t text_encoding;       ///< 1 = UTF-8, 2 = UTF-16le, 3 = UTF-16be
    uint32_t user_version;        ///< user_version pragma
    uint32_t application_id;      ///< application_id pragma
    uint32_t sqlite_version;      ///< SQLITE_VERSION_NUMBER that last wrote the file
};

/**
 * @brief Parse and validate the 100-byte database header at @p buf (@p len must be >= 100).
 * @return false if the magic string is wrong or the page size is not a valid power of two.
 */
bool sqlite_parse_db_header(const uint8_t *buf, size_t len, SqliteDbHeader *out);

/** @brief B-tree page types (the first byte of a b-tree page header). */
enum
{
    SQLITE_BTREE_INTERIOR_INDEX = 2,
    SQLITE_BTREE_INTERIOR_TABLE = 5,
    SQLITE_BTREE_LEAF_INDEX = 10,
    SQLITE_BTREE_LEAF_TABLE = 13,
};

/** @brief Parsed b-tree page header (8 bytes for a leaf, 12 for an interior page). */
struct SqliteBtreeHeader
{
    uint8_t type;                ///< one of the SQLITE_BTREE_* values
    uint16_t first_freeblock;    ///< offset of the first freeblock (0 = none)
    uint16_t cell_count;         ///< number of cells on the page
    uint32_t cell_content_start; ///< start of the cell content area (on-disk 0 means 65536)
    uint8_t frag_free_bytes;     ///< fragmented free bytes in the cell content area
    uint32_t right_most_page;    ///< right-most child page (interior pages only; 0 on a leaf)
    uint8_t header_size;         ///< 8 (leaf) or 12 (interior) - where the cell pointer array begins
};

/**
 * @brief Parse a b-tree page header located at @p page + @p offset (@p offset is 100 for page 1, else 0).
 * @param page_len bounds the read. @return false if the page type byte is not a valid b-tree type or the
 * header runs past @p page_len.
 */
bool sqlite_parse_btree_header(const uint8_t *page, size_t page_len, size_t offset, SqliteBtreeHeader *out);

#endif // DETWS_ENABLE_SQLITE
#endif // DETERMINISTICESPASYNCWEBSERVER_SQLITE_FORMAT_H
