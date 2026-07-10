// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/sqlite: the SQLite3 on-disk file-format parsers. The page-1 vector below is the
// first 512 bytes of a REAL database built by the sqlite3 CLI (3.46.1) with:
//   PRAGMA page_size=512; PRAGMA encoding='UTF-8';
//   CREATE TABLE t(a INTEGER, b TEXT); INSERT INTO t VALUES(42,'hello'); INSERT INTO t VALUES(7,'world');
// so the parsers are checked against bytes an authoritative implementation actually wrote.

#include "db_multipage.h"
#include "db_overflow.h"
#include "services/sqlite/sqlite_format.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

// First 512 bytes (page 1) of the real database file.
static const uint8_t PAGE1[512] = {
    0x53, 0x51, 0x4c, 0x69, 0x74, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x20, 0x33, 0x00, //
    0x02, 0x00, 0x01, 0x01, 0x00, 0x40, 0x20, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, //
    0x00, 0x2e, 0x7a, 0x71, 0x0d, 0x00, 0x00, 0x00, 0x01, 0x01, 0xcf, 0x00, 0x01, 0xcf, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2f, //
    0x01, 0x06, 0x17, 0x0f, 0x0f, 0x01, 0x4f, 0x74, 0x61, 0x62, 0x6c, 0x65, 0x74, 0x74, 0x02, 0x43, //
    0x52, 0x45, 0x41, 0x54, 0x45, 0x20, 0x54, 0x41, 0x42, 0x4c, 0x45, 0x20, 0x74, 0x28, 0x61, 0x20, //
    0x49, 0x4e, 0x54, 0x45, 0x47, 0x45, 0x52, 0x2c, 0x20, 0x62, 0x20, 0x54, 0x45, 0x58, 0x54, 0x29, //
};

void test_db_header_real_file(void)
{
    SqliteDbHeader h;
    TEST_ASSERT_TRUE(sqlite_parse_db_header(PAGE1, sizeof(PAGE1), &h));
    TEST_ASSERT_EQUAL_UINT32(512, h.page_size);
    TEST_ASSERT_EQUAL_UINT8(1, h.write_version); // legacy rollback journal
    TEST_ASSERT_EQUAL_UINT8(1, h.read_version);
    TEST_ASSERT_EQUAL_UINT8(0, h.reserved_per_page);
    TEST_ASSERT_EQUAL_UINT32(2, h.page_count);    // matches PRAGMA page_count
    TEST_ASSERT_EQUAL_UINT32(1, h.schema_cookie); // matches PRAGMA schema_version
    TEST_ASSERT_EQUAL_UINT32(4, h.schema_format);
    TEST_ASSERT_EQUAL_UINT32(1, h.text_encoding);        // UTF-8
    TEST_ASSERT_EQUAL_UINT32(3046001, h.sqlite_version); // SQLite 3.46.1
    TEST_ASSERT_EQUAL_UINT32(0, h.freelist_first);
}

void test_db_header_rejects_bad_magic(void)
{
    uint8_t bad[100];
    for (int i = 0; i < 100; i++)
        bad[i] = PAGE1[i];
    bad[0] = 'X'; // corrupt the magic
    SqliteDbHeader h;
    TEST_ASSERT_FALSE(sqlite_parse_db_header(bad, sizeof(bad), &h));
    // Too short also fails.
    TEST_ASSERT_FALSE(sqlite_parse_db_header(PAGE1, 99, &h));
}

void test_btree_header_real_page1(void)
{
    SqliteBtreeHeader b;
    // Page 1's b-tree header follows the 100-byte database header.
    TEST_ASSERT_TRUE(sqlite_parse_btree_header(PAGE1, sizeof(PAGE1), 100, &b));
    TEST_ASSERT_EQUAL_UINT8(SQLITE_BTREE_LEAF_TABLE, b.type); // 13 - the sqlite_schema table
    TEST_ASSERT_EQUAL_UINT16(0, b.first_freeblock);
    TEST_ASSERT_EQUAL_UINT16(1, b.cell_count); // one schema row (the CREATE TABLE)
    TEST_ASSERT_EQUAL_UINT32(463, b.cell_content_start);
    TEST_ASSERT_EQUAL_UINT8(8, b.header_size); // leaf
    TEST_ASSERT_EQUAL_UINT32(0, b.right_most_page);
}

void test_btree_header_rejects_bad_type(void)
{
    uint8_t page[16];
    for (int i = 0; i < 16; i++)
        page[i] = 0;
    page[0] = 99; // not a valid b-tree page type
    SqliteBtreeHeader b;
    TEST_ASSERT_FALSE(sqlite_parse_btree_header(page, sizeof(page), 0, &b));
    // A valid leaf type but the header runs past the buffer.
    page[0] = SQLITE_BTREE_LEAF_TABLE;
    TEST_ASSERT_FALSE(sqlite_parse_btree_header(page, 4, 0, &b));
}

void test_first_cell_varints(void)
{
    // The single cell pointer lives right after the 8-byte leaf header (offset 108), big-endian u16.
    uint32_t cell_off = ((uint32_t)PAGE1[108] << 8) | PAGE1[109];
    TEST_ASSERT_EQUAL_UINT32(463, cell_off);
    // A leaf-table cell begins with: payload-length varint, then rowid varint.
    uint64_t payload_len = 0, rowid = 0;
    size_t n = sqlite_varint_decode(PAGE1 + cell_off, sizeof(PAGE1) - cell_off, &payload_len);
    TEST_ASSERT_EQUAL_size_t(1, n);
    TEST_ASSERT_EQUAL_UINT64(47, payload_len); // 0x2f
    size_t n2 = sqlite_varint_decode(PAGE1 + cell_off + n, sizeof(PAGE1) - cell_off - n, &rowid);
    TEST_ASSERT_EQUAL_size_t(1, n2);
    TEST_ASSERT_EQUAL_UINT64(1, rowid);
}

void test_varint_spec_vectors(void)
{
    uint64_t v = 0;
    const uint8_t a[] = {0x00};
    TEST_ASSERT_EQUAL_size_t(1, sqlite_varint_decode(a, 1, &v));
    TEST_ASSERT_EQUAL_UINT64(0, v);

    const uint8_t b[] = {0x7f};
    TEST_ASSERT_EQUAL_size_t(1, sqlite_varint_decode(b, 1, &v));
    TEST_ASSERT_EQUAL_UINT64(127, v);

    const uint8_t c[] = {0x81, 0x00}; // 0x80 continuation -> (1<<7)|0 = 128
    TEST_ASSERT_EQUAL_size_t(2, sqlite_varint_decode(c, 2, &v));
    TEST_ASSERT_EQUAL_UINT64(128, v);

    const uint8_t d[] = {0x82, 0x2f}; // (2<<7)|0x2f = 303
    TEST_ASSERT_EQUAL_size_t(2, sqlite_varint_decode(d, 2, &v));
    TEST_ASSERT_EQUAL_UINT64(303, v);

    // All nine bytes set -> the full 64-bit value.
    const uint8_t big[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    TEST_ASSERT_EQUAL_size_t(9, sqlite_varint_decode(big, 9, &v));
    TEST_ASSERT_EQUAL_UINT64(0xFFFFFFFFFFFFFFFFull, v);

    // Incomplete varint (continuation bit set but no more bytes) fails closed.
    const uint8_t inc[] = {0x81};
    TEST_ASSERT_EQUAL_size_t(0, sqlite_varint_decode(inc, 1, &v));
}

void test_serial_type_sizes(void)
{
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(0)); // NULL
    TEST_ASSERT_EQUAL_UINT64(1, sqlite_serial_type_size(1));
    TEST_ASSERT_EQUAL_UINT64(2, sqlite_serial_type_size(2));
    TEST_ASSERT_EQUAL_UINT64(3, sqlite_serial_type_size(3));
    TEST_ASSERT_EQUAL_UINT64(4, sqlite_serial_type_size(4));
    TEST_ASSERT_EQUAL_UINT64(6, sqlite_serial_type_size(5));
    TEST_ASSERT_EQUAL_UINT64(8, sqlite_serial_type_size(6));
    TEST_ASSERT_EQUAL_UINT64(8, sqlite_serial_type_size(7));  // float64
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(8));  // int 0
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(9));  // int 1
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(10)); // reserved
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(11)); // reserved
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(12)); // BLOB len 0
    TEST_ASSERT_EQUAL_UINT64(1, sqlite_serial_type_size(14)); // BLOB len 1
    TEST_ASSERT_EQUAL_UINT64(0, sqlite_serial_type_size(13)); // TEXT len 0
    TEST_ASSERT_EQUAL_UINT64(1, sqlite_serial_type_size(15)); // TEXT len 1  (seen in the real record)
    TEST_ASSERT_EQUAL_UINT64(5, sqlite_serial_type_size(23)); // TEXT len 5  ("table", the real record)
}

// Read the one real row on page 1: the sqlite_schema entry describing table t.
// Columns are (type, name, tbl_name, rootpage, sql) = ("table","t","t",2,"CREATE TABLE t(a INTEGER, b TEXT)").
void test_read_schema_row(void)
{
    SqliteBtreeHeader b;
    TEST_ASSERT_TRUE(sqlite_parse_btree_header(PAGE1, sizeof(PAGE1), 100, &b));
    uint32_t cp = sqlite_cell_pointer(PAGE1, sizeof(PAGE1), &b, 100, 0);
    TEST_ASSERT_EQUAL_UINT32(463, cp);

    SqliteTableLeafCell cell;
    TEST_ASSERT_TRUE(sqlite_parse_table_leaf_cell(PAGE1, sizeof(PAGE1), 512, 0, cp, &cell));
    TEST_ASSERT_EQUAL_UINT64(1, cell.rowid);
    TEST_ASSERT_EQUAL_UINT32(47, cell.payload_len);
    TEST_ASSERT_FALSE(cell.has_overflow);
    TEST_ASSERT_EQUAL_UINT32(47, cell.local_len);

    SqliteRecordCursor c;
    TEST_ASSERT_TRUE(sqlite_record_begin(&c, PAGE1 + cell.local_off, cell.local_len));
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;

    TEST_ASSERT_TRUE(sqlite_record_next(&c, &st, &v, &vl)); // type
    TEST_ASSERT_EQUAL_UINT64(23, st);                       // text, 5 bytes
    TEST_ASSERT_EQUAL_UINT32(5, vl);
    TEST_ASSERT_EQUAL_MEMORY("table", v, 5);

    TEST_ASSERT_TRUE(sqlite_record_next(&c, &st, &v, &vl)); // name
    TEST_ASSERT_EQUAL_UINT32(1, vl);
    TEST_ASSERT_EQUAL_MEMORY("t", v, 1);

    TEST_ASSERT_TRUE(sqlite_record_next(&c, &st, &v, &vl)); // tbl_name
    TEST_ASSERT_EQUAL_MEMORY("t", v, 1);

    TEST_ASSERT_TRUE(sqlite_record_next(&c, &st, &v, &vl)); // rootpage
    TEST_ASSERT_EQUAL_UINT64(1, st);                        // 1-byte int
    TEST_ASSERT_EQUAL_INT64(2, sqlite_column_int(st, v, vl));

    TEST_ASSERT_TRUE(sqlite_record_next(&c, &st, &v, &vl)); // sql
    TEST_ASSERT_EQUAL_UINT64(79, st);                       // text, 33 bytes
    TEST_ASSERT_EQUAL_UINT32(33, vl);
    TEST_ASSERT_EQUAL_MEMORY("CREATE TABLE t(a INTEGER, b TEXT)", v, 33);

    TEST_ASSERT_FALSE(sqlite_record_next(&c, &st, &v, &vl)); // exactly five columns
}

void test_column_int_signextend(void)
{
    const uint8_t m1[] = {0xFF};
    TEST_ASSERT_EQUAL_INT64(-1, sqlite_column_int(1, m1, 1));
    const uint8_t p127[] = {0x7F};
    TEST_ASSERT_EQUAL_INT64(127, sqlite_column_int(1, p127, 1));
    const uint8_t m2[] = {0xFF, 0xFE};
    TEST_ASSERT_EQUAL_INT64(-2, sqlite_column_int(2, m2, 2));
    const uint8_t big[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00}; // 256 as a 64-bit int
    TEST_ASSERT_EQUAL_INT64(256, sqlite_column_int(6, big, 8));
    TEST_ASSERT_EQUAL_INT64(0, sqlite_column_int(8, nullptr, 0)); // constant 0
    TEST_ASSERT_EQUAL_INT64(1, sqlite_column_int(9, nullptr, 0)); // constant 1
}

// A payload larger than the local max for a 512-byte page must be flagged as overflowing.
void test_leaf_cell_overflow_detection(void)
{
    uint8_t page[512];
    for (int i = 0; i < 512; i++)
        page[i] = 0;
    // cell at offset 100: payload-length varint 478 (0x83,0x5E), rowid varint 1.
    page[100] = 0x83;
    page[101] = 0x5E;
    page[102] = 0x01;
    SqliteTableLeafCell cell;
    TEST_ASSERT_TRUE(sqlite_parse_table_leaf_cell(page, sizeof(page), 512, 0, 100, &cell));
    TEST_ASSERT_EQUAL_UINT64(1, cell.rowid);
    TEST_ASSERT_EQUAL_UINT32(478, cell.payload_len);
    TEST_ASSERT_TRUE(cell.has_overflow);
    TEST_ASSERT_EQUAL_UINT32(39, cell.local_len); // min_local per the SQLite threshold formula
}

// Page reader over an in-memory database image (pages are 1-based).
struct MemDb
{
    const uint8_t *data;
    uint32_t size;
};
static bool mem_read(void *ctx, uint32_t pgno, uint8_t *page, uint32_t page_size)
{
    MemDb *m = (MemDb *)ctx;
    if (pgno < 1)
        return false;
    uint32_t off = (pgno - 1) * page_size;
    if (off + page_size > m->size)
        return false;
    memcpy(page, m->data + off, page_size);
    return true;
}

// Walk a real 2-level table b-tree (interior root on page 2) and read all 40 rows in rowid order.
void test_table_cursor_multipage(void)
{
    // The table's root page (page 2) is an interior table page, so this exercises the descent stack.
    SqliteBtreeHeader bh;
    TEST_ASSERT_TRUE(
        sqlite_parse_btree_header(DB_MULTIPAGE + DB_MP_PAGE_SIZE, sizeof(DB_MULTIPAGE) - DB_MP_PAGE_SIZE, 0, &bh));
    TEST_ASSERT_EQUAL_UINT8(SQLITE_BTREE_INTERIOR_TABLE, bh.type);

    MemDb db = {DB_MULTIPAGE, sizeof(DB_MULTIPAGE)};
    static uint8_t leaf[512], work[512];
    SqliteTableCursor c;
    TEST_ASSERT_TRUE(sqlite_table_cursor_begin(&c, mem_read, &db, DB_MP_PAGE_SIZE, 0, 2, leaf, work));

    uint32_t n = 0;
    uint64_t rowid = 0;
    SqliteRecordCursor row;
    while (sqlite_table_cursor_next(&c, &rowid, &row))
    {
        n++;
        TEST_ASSERT_EQUAL_UINT64(n, rowid); // rowids 1..40, in order, across pages

        uint64_t st = 0;
        const uint8_t *v = nullptr;
        uint32_t vl = 0;
        TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // column a (INTEGER) == rowid
        TEST_ASSERT_EQUAL_INT64((int64_t)n, sqlite_column_int(st, v, vl));

        TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // column b (TEXT)
        char expect[64];
        snprintf(expect, sizeof(expect), "row%04d-abcdefghijklmnopqrstuvwxyz", (int)n);
        TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(expect), vl);
        TEST_ASSERT_EQUAL_MEMORY(expect, v, vl);

        TEST_ASSERT_FALSE(sqlite_record_next(&row, &st, &v, &vl)); // exactly two columns
    }
    TEST_ASSERT_EQUAL_UINT32(DB_MP_ROWS, n); // every row visited exactly once
}

// Page reader over the overflow fixture, whose pages are separate 512-byte arrays (1-based).
static bool ovf_read(void *ctx, uint32_t pgno, uint8_t *page, uint32_t page_size)
{
    (void)ctx;
    if (pgno < 1 || pgno > OVF_PAGE_COUNT || page_size != OVF_PAGE_SIZE)
        return false;
    memcpy(page, OVF_PAGES[pgno - 1], OVF_PAGE_SIZE);
    return true;
}

// Read the TEXT column b of a row record and assert it is `len` copies of `ch`.
static void assert_text_run(SqliteRecordCursor *row, uint32_t len, char ch)
{
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;
    TEST_ASSERT_TRUE(sqlite_record_next(row, &st, &v, &vl)); // column a (INTEGER id)
    TEST_ASSERT_TRUE(sqlite_record_next(row, &st, &v, &vl)); // column b (TEXT data)
    TEST_ASSERT_EQUAL_UINT64(13u + 2u * len, st);            // TEXT serial type = 13 + 2*len
    TEST_ASSERT_EQUAL_UINT32(len, vl);
    for (uint32_t i = 0; i < vl; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)ch, v[i]);
}

// Read the TEXT column b of a row record and assert it equals the exact string `s`.
static void assert_text_eq(SqliteRecordCursor *row, const char *s)
{
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;
    uint32_t len = (uint32_t)strlen(s);
    TEST_ASSERT_TRUE(sqlite_record_next(row, &st, &v, &vl)); // column a (INTEGER id)
    TEST_ASSERT_TRUE(sqlite_record_next(row, &st, &v, &vl)); // column b (TEXT data)
    TEST_ASSERT_EQUAL_UINT64(13u + 2u * len, st);
    TEST_ASSERT_EQUAL_UINT32(len, vl);
    TEST_ASSERT_EQUAL_MEMORY(s, v, vl);
}

// Scan the image for the first leaf-table page holding an overflowing cell; copy that leaf page into
// @p leaf_out and return the parsed cell. (The table root here is an interior page whose leaves sit on
// later pages, so we locate the leaf rather than assume the root is one.)
static bool find_overflow_cell(uint8_t *leaf_out, SqliteTableLeafCell *cell_out)
{
    for (uint32_t pg = 1; pg <= OVF_PAGE_COUNT; pg++)
    {
        uint8_t page[OVF_PAGE_SIZE];
        if (!ovf_read(nullptr, pg, page, OVF_PAGE_SIZE))
            continue;
        size_t off = (pg == 1) ? 100 : 0;
        SqliteBtreeHeader bh;
        if (!sqlite_parse_btree_header(page, OVF_PAGE_SIZE, off, &bh) || bh.type != SQLITE_BTREE_LEAF_TABLE)
            continue;
        for (uint16_t i = 0; i < bh.cell_count; i++)
        {
            uint32_t cp = sqlite_cell_pointer(page, OVF_PAGE_SIZE, &bh, off, i);
            SqliteTableLeafCell cell;
            if (cp && sqlite_parse_table_leaf_cell(page, OVF_PAGE_SIZE, OVF_PAGE_SIZE, 0, cp, &cell) &&
                cell.has_overflow)
            {
                memcpy(leaf_out, page, OVF_PAGE_SIZE);
                *cell_out = cell;
                return true;
            }
        }
    }
    return false;
}

// Reassemble an overflowing row's payload directly with sqlite_read_payload and verify the full TEXT.
void test_overflow_read_payload(void)
{
    static uint8_t leaf[OVF_PAGE_SIZE], work[OVF_PAGE_SIZE], payload[4096];
    SqliteTableLeafCell cell;
    TEST_ASSERT_TRUE(find_overflow_cell(leaf, &cell));
    TEST_ASSERT_TRUE(cell.has_overflow);
    TEST_ASSERT_TRUE(cell.local_len < cell.payload_len); // the record really spills onto overflow pages
    TEST_ASSERT_TRUE(
        sqlite_read_payload(ovf_read, nullptr, OVF_PAGE_SIZE, 0, leaf, &cell, payload, sizeof(payload), work));

    // The reassembled record decodes to (id INTEGER, data TEXT) where data is a run of one character.
    SqliteRecordCursor row;
    TEST_ASSERT_TRUE(sqlite_record_begin(&row, payload, cell.payload_len));
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;
    TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // id
    TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // data (TEXT)
    TEST_ASSERT_TRUE(vl == OVF_ROW2_LEN || vl == OVF_ROW3_LEN);
    TEST_ASSERT_EQUAL_UINT64(13u + 2u * vl, st);
    char ch = (char)v[0];
    TEST_ASSERT_TRUE(ch == 'A' || ch == 'B');
    for (uint32_t i = 0; i < vl; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)ch, v[i]); // every byte survived the chain intact
}

// A non-overflow cell reassembles trivially: sqlite_read_payload copies the in-page prefix and succeeds.
void test_read_payload_nonoverflow(void)
{
    static uint8_t leaf[OVF_PAGE_SIZE], work[OVF_PAGE_SIZE], out[OVF_PAGE_SIZE];
    memset(leaf, 0, sizeof(leaf));
    for (uint32_t i = 0; i < 50; i++)
        leaf[8 + i] = (uint8_t)(i + 1); // a known local payload at offset 8
    SqliteTableLeafCell cell;
    cell.rowid = 1;
    cell.payload_len = 50;
    cell.local_off = 8;
    cell.local_len = 50;
    cell.has_overflow = false;
    TEST_ASSERT_TRUE(sqlite_read_payload(ovf_read, nullptr, OVF_PAGE_SIZE, 0, leaf, &cell, out, sizeof(out), work));
    TEST_ASSERT_EQUAL_MEMORY(leaf + 8, out, 50);
}

// A corrupt overflow next-pointer (to a page the reader rejects) must fail closed, not crash.
void test_read_payload_bad_overflow_pointer(void)
{
    static uint8_t leaf[OVF_PAGE_SIZE], work[OVF_PAGE_SIZE], out[4096];
    memset(leaf, 0, sizeof(leaf));
    SqliteTableLeafCell cell;
    cell.rowid = 1;
    cell.payload_len = 1000; // > local -> claims an overflow chain
    cell.local_off = 8;
    cell.local_len = 100;
    cell.has_overflow = true;
    // The 4-byte first-overflow pointer sits right after the local prefix: point it at page 9999, which
    // ovf_read (only 11 pages) refuses -> the read fails and the reassembly returns false.
    uint32_t ptr = cell.local_off + cell.local_len;
    leaf[ptr] = 0x00;
    leaf[ptr + 1] = 0x00;
    leaf[ptr + 2] = 0x27;
    leaf[ptr + 3] = 0x0f; // 9999
    TEST_ASSERT_FALSE(sqlite_read_payload(ovf_read, nullptr, OVF_PAGE_SIZE, 0, leaf, &cell, out, sizeof(out), work));
}

// A short output buffer must fail closed, not overrun.
void test_overflow_read_payload_bounds(void)
{
    static uint8_t leaf[OVF_PAGE_SIZE], work[OVF_PAGE_SIZE];
    SqliteTableLeafCell cell;
    TEST_ASSERT_TRUE(find_overflow_cell(leaf, &cell));
    uint8_t tiny[16]; // far smaller than the >=1000-byte overflowing payload
    TEST_ASSERT_FALSE(sqlite_read_payload(ovf_read, nullptr, OVF_PAGE_SIZE, 0, leaf, &cell, tiny, sizeof(tiny), work));
}

// Drive the table cursor with an overflow buffer: every row (incl. the overflowing ones) fully reassembled.
void test_overflow_cursor(void)
{
    static uint8_t leaf[OVF_PAGE_SIZE], work[OVF_PAGE_SIZE], ovf[4096];
    SqliteTableCursor c;
    TEST_ASSERT_TRUE(sqlite_table_cursor_begin(&c, ovf_read, nullptr, OVF_PAGE_SIZE, 0, OVF_ROOTPAGE, leaf, work));
    sqlite_table_cursor_set_overflow_buf(&c, ovf, sizeof(ovf));

    uint64_t rowid = 0;
    SqliteRecordCursor row;
    uint32_t n = 0;
    while (sqlite_table_cursor_next(&c, &rowid, &row))
    {
        n++;
        TEST_ASSERT_EQUAL_UINT64(n, rowid);
        if (n == 1)
            assert_text_eq(&row, OVF_ROW1);
        else if (n == 2)
            assert_text_run(&row, OVF_ROW2_LEN, 'A');
        else
            assert_text_run(&row, OVF_ROW3_LEN, 'B');
    }
    TEST_ASSERT_EQUAL_UINT32(3, n);
}

// ---- Writer (bounded single-table builder) ----

void test_varint_encode_roundtrip(void)
{
    const uint64_t vals[] = {0,
                             1,
                             127,
                             128,
                             16383,
                             16384,
                             2097151,
                             2097152,
                             0xFFFFFFFFull,
                             0x7FFFFFFFFFull,
                             0x1FFFFFFFFFFFFull,
                             0xFFFFFFFFFFFFFFull,
                             0xFFFFFFFFFFFFFFFFull};
    for (unsigned k = 0; k < sizeof(vals) / sizeof(vals[0]); k++)
    {
        uint8_t buf[9];
        size_t n = sqlite_varint_encode(vals[k], buf, sizeof(buf));
        TEST_ASSERT_TRUE(n >= 1 && n <= 9);
        uint64_t back = 0;
        size_t m = sqlite_varint_decode(buf, n, &back);
        TEST_ASSERT_EQUAL_size_t(n, m); // encode and decode agree on the length
        TEST_ASSERT_EQUAL_UINT64(vals[k], back);
    }
    // A capacity that is one short must fail closed.
    uint8_t one[1];
    TEST_ASSERT_EQUAL_size_t(0, sqlite_varint_encode(128, one, 1));
}

void test_encode_record_roundtrip(void)
{
    // A row of (INT, TEXT, FLOAT, NULL, INT=0) round-trips through the record reader.
    SqliteValue cols[5];
    cols[0] = {SQLITE_COL_INT, -12345, 0, nullptr, 0};
    cols[1] = {SQLITE_COL_TEXT, 0, 0, (const uint8_t *)"hello", 5};
    cols[2] = {SQLITE_COL_FLOAT, 0, 3.14159, nullptr, 0};
    cols[3] = {SQLITE_COL_NULL, 0, 0, nullptr, 0};
    cols[4] = {SQLITE_COL_INT, 0, 0, nullptr, 0}; // the constant 0 (serial type 8, 0 bytes)

    uint8_t rec[128];
    uint32_t rl = sqlite_encode_record(cols, 5, rec, sizeof(rec));
    TEST_ASSERT_TRUE(rl > 0);

    SqliteRecordCursor rc;
    TEST_ASSERT_TRUE(sqlite_record_begin(&rc, rec, rl));
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;

    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl)); // INT -12345
    TEST_ASSERT_EQUAL_INT64(-12345, sqlite_column_int(st, v, vl));
    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl)); // TEXT "hello"
    TEST_ASSERT_EQUAL_UINT64(13u + 2u * 5u, st);
    TEST_ASSERT_EQUAL_UINT32(5, vl);
    TEST_ASSERT_EQUAL_MEMORY("hello", v, 5);
    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl)); // FLOAT 3.14159
    TEST_ASSERT_EQUAL_UINT64(7, st);
    // Unity's double asserts are disabled in this build; compare the IEEE-754 bit pattern instead (the
    // decoder reads back the exact 8 bytes we wrote, so it is bit-exact).
    double got = sqlite_column_float(v, vl), want = 3.14159;
    uint64_t got_bits = 0, want_bits = 0;
    memcpy(&got_bits, &got, 8);
    memcpy(&want_bits, &want, 8);
    TEST_ASSERT_EQUAL_HEX64(want_bits, got_bits);
    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl)); // NULL
    TEST_ASSERT_EQUAL_UINT64(0, st);
    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl)); // INT 0 (constant)
    TEST_ASSERT_EQUAL_UINT64(8, st);
    TEST_ASSERT_EQUAL_INT64(0, sqlite_column_int(st, v, vl));
    TEST_ASSERT_FALSE(sqlite_record_next(&rc, &st, &v, &vl)); // exactly five columns
}

void test_build_table_db_roundtrip(void)
{
    // Build a real 2-page DB, then read it back with our own reader.
    struct RowSpec
    {
        long a;
        const char *b;
    } spec[] = {{100, "alpha"}, {-5, "beta"}, {999999, "gamma"}};
    SqliteValue rowvals[3][2];
    SqliteRow rows[3];
    for (int r = 0; r < 3; r++)
    {
        rowvals[r][0] = {SQLITE_COL_INT, spec[r].a, 0, nullptr, 0};
        rowvals[r][1] = {SQLITE_COL_TEXT, 0, 0, (const uint8_t *)spec[r].b, (uint32_t)strlen(spec[r].b)};
        rows[r] = {(uint64_t)(r + 1), rowvals[r], 2};
    }

    static uint8_t img[1024]; // 2 * 512
    uint32_t len = sqlite_build_table_db(512, "t", "CREATE TABLE t(a INTEGER, b TEXT)", rows, 3, img, sizeof(img));
    TEST_ASSERT_EQUAL_UINT32(1024, len);

    // The database header parses and reports what we wrote.
    SqliteDbHeader h;
    TEST_ASSERT_TRUE(sqlite_parse_db_header(img, len, &h));
    TEST_ASSERT_EQUAL_UINT32(512, h.page_size);
    TEST_ASSERT_EQUAL_UINT32(2, h.page_count);
    TEST_ASSERT_EQUAL_UINT32(1, h.text_encoding);

    // The sqlite_schema row on page 1: type='table', name='t', rootpage=2, sql=the CREATE.
    SqliteBtreeHeader p1;
    TEST_ASSERT_TRUE(sqlite_parse_btree_header(img, 512, 100, &p1));
    TEST_ASSERT_EQUAL_UINT16(1, p1.cell_count);
    uint32_t mcp = sqlite_cell_pointer(img, 512, &p1, 100, 0);
    SqliteTableLeafCell mcell;
    TEST_ASSERT_TRUE(sqlite_parse_table_leaf_cell(img, 512, 512, 0, mcp, &mcell));
    SqliteRecordCursor mrec;
    TEST_ASSERT_TRUE(sqlite_record_begin(&mrec, img + mcell.local_off, mcell.local_len));
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;
    TEST_ASSERT_TRUE(sqlite_record_next(&mrec, &st, &v, &vl)); // type
    TEST_ASSERT_EQUAL_MEMORY("table", v, 5);
    TEST_ASSERT_TRUE(sqlite_record_next(&mrec, &st, &v, &vl)); // name
    TEST_ASSERT_EQUAL_MEMORY("t", v, 1);
    TEST_ASSERT_TRUE(sqlite_record_next(&mrec, &st, &v, &vl)); // tbl_name
    TEST_ASSERT_EQUAL_MEMORY("t", v, 1);
    TEST_ASSERT_TRUE(sqlite_record_next(&mrec, &st, &v, &vl)); // rootpage
    TEST_ASSERT_EQUAL_INT64(2, sqlite_column_int(st, v, vl));

    // The table rows on page 2, read via the cursor.
    MemDb db = {img, len};
    static uint8_t leaf[512], work[512];
    SqliteTableCursor c;
    TEST_ASSERT_TRUE(sqlite_table_cursor_begin(&c, mem_read, &db, 512, 0, 2, leaf, work));
    uint64_t rowid = 0;
    SqliteRecordCursor row;
    int n = 0;
    while (sqlite_table_cursor_next(&c, &rowid, &row))
    {
        TEST_ASSERT_EQUAL_UINT64((uint64_t)(n + 1), rowid);
        TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // a INTEGER
        TEST_ASSERT_EQUAL_INT64(spec[n].a, sqlite_column_int(st, v, vl));
        TEST_ASSERT_TRUE(sqlite_record_next(&row, &st, &v, &vl)); // b TEXT
        TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(spec[n].b), vl);
        TEST_ASSERT_EQUAL_MEMORY(spec[n].b, v, vl);
        n++;
    }
    TEST_ASSERT_EQUAL_INT(3, n);
}

void test_encode_record_int_widths(void)
{
    // Every integer serial type: the value round-trips and the encoder picks the minimal type.
    struct
    {
        int64_t v;
        uint64_t st;
    } cases[] = {
        {0, 8},
        {1, 9},
        {-1, 1},
        {127, 1},
        {-128, 1},
        {128, 2},
        {-129, 2},
        {32767, 2},
        {40000, 3},
        {-40000, 3},
        {8388607, 3},
        {10000000, 4},
        {-10000000, 4},
        {2147483647, 4},
        {3000000000LL, 5},
        {-3000000000LL, 5},
        {140737488355327LL, 5},
        {200000000000000LL, 6},
        {-9000000000000000000LL, 6},
    };
    for (unsigned k = 0; k < sizeof(cases) / sizeof(cases[0]); k++)
    {
        SqliteValue col = {SQLITE_COL_INT, cases[k].v, 0, nullptr, 0};
        uint8_t rec[32];
        uint32_t rl = sqlite_encode_record(&col, 1, rec, sizeof(rec));
        TEST_ASSERT_TRUE(rl > 0);
        SqliteRecordCursor rc;
        TEST_ASSERT_TRUE(sqlite_record_begin(&rc, rec, rl));
        uint64_t st = 0;
        const uint8_t *v = nullptr;
        uint32_t vl = 0;
        TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl));
        TEST_ASSERT_EQUAL_UINT64(cases[k].st, st); // minimal serial type
        TEST_ASSERT_EQUAL_INT64(cases[k].v, sqlite_column_int(st, v, vl));
        TEST_ASSERT_FALSE(sqlite_record_next(&rc, &st, &v, &vl)); // single column
    }
}

void test_encode_record_blob(void)
{
    // A BLOB column (serial type 12 + 2n) round-trips its raw bytes, including embedded NULs.
    const uint8_t blob[] = {0x00, 0xff, 0x10, 0x00, 0x7f, 0x80};
    SqliteValue col = {SQLITE_COL_BLOB, 0, 0, blob, sizeof(blob)};
    uint8_t rec[32];
    uint32_t rl = sqlite_encode_record(&col, 1, rec, sizeof(rec));
    TEST_ASSERT_TRUE(rl > 0);
    SqliteRecordCursor rc;
    TEST_ASSERT_TRUE(sqlite_record_begin(&rc, rec, rl));
    uint64_t st = 0;
    const uint8_t *v = nullptr;
    uint32_t vl = 0;
    TEST_ASSERT_TRUE(sqlite_record_next(&rc, &st, &v, &vl));
    TEST_ASSERT_EQUAL_UINT64(12u + 2u * sizeof(blob), st); // BLOB serial type
    TEST_ASSERT_EQUAL_UINT32(sizeof(blob), vl);
    TEST_ASSERT_EQUAL_MEMORY(blob, v, sizeof(blob));
}

void test_build_table_db_page_overflow_fails_closed(void)
{
    // Many rows that each fit but collectively exceed one leaf page must fail closed (distinct from the
    // single-oversized-row path): the cells + pointer array do not fit page 2.
    static SqliteValue rowvals[60][1];
    static SqliteRow rows[60];
    for (int r = 0; r < 60; r++)
    {
        rowvals[r][0] = {SQLITE_COL_INT, 1000000 + r, 0, nullptr, 0}; // ~4-byte int + framing each
        rows[r] = {(uint64_t)(r + 1), rowvals[r], 1};
    }
    static uint8_t img[1024];
    TEST_ASSERT_EQUAL_UINT32(0, sqlite_build_table_db(512, "t", "CREATE TABLE t(a)", rows, 60, img, sizeof(img)));
}

void test_build_table_db_fails_closed(void)
{
    // A single row larger than one leaf page can hold must fail closed (bounded writer, no overflow pages).
    static uint8_t big[600];
    memset(big, 'X', sizeof(big));
    SqliteValue col = {SQLITE_COL_TEXT, 0, 0, big, sizeof(big)};
    SqliteRow row = {1, &col, 1};
    static uint8_t img[1024];
    TEST_ASSERT_EQUAL_UINT32(0, sqlite_build_table_db(512, "t", "CREATE TABLE t(b TEXT)", &row, 1, img, sizeof(img)));
    // Too small an output buffer also fails closed.
    SqliteValue small = {SQLITE_COL_INT, 1, 0, nullptr, 0};
    SqliteRow r2 = {1, &small, 1};
    TEST_ASSERT_EQUAL_UINT32(0, sqlite_build_table_db(512, "t", "CREATE TABLE t(a)", &r2, 1, img, 512));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_db_header_real_file);
    RUN_TEST(test_db_header_rejects_bad_magic);
    RUN_TEST(test_btree_header_real_page1);
    RUN_TEST(test_btree_header_rejects_bad_type);
    RUN_TEST(test_first_cell_varints);
    RUN_TEST(test_varint_spec_vectors);
    RUN_TEST(test_serial_type_sizes);
    RUN_TEST(test_read_schema_row);
    RUN_TEST(test_column_int_signextend);
    RUN_TEST(test_leaf_cell_overflow_detection);
    RUN_TEST(test_table_cursor_multipage);
    RUN_TEST(test_overflow_read_payload);
    RUN_TEST(test_read_payload_nonoverflow);
    RUN_TEST(test_read_payload_bad_overflow_pointer);
    RUN_TEST(test_overflow_read_payload_bounds);
    RUN_TEST(test_overflow_cursor);
    RUN_TEST(test_varint_encode_roundtrip);
    RUN_TEST(test_encode_record_roundtrip);
    RUN_TEST(test_build_table_db_roundtrip);
    RUN_TEST(test_encode_record_int_widths);
    RUN_TEST(test_encode_record_blob);
    RUN_TEST(test_build_table_db_page_overflow_fails_closed);
    RUN_TEST(test_build_table_db_fails_closed);
    return UNITY_END();
}
