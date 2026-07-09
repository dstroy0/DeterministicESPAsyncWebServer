// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/sqlite: the SQLite3 on-disk file-format parsers. The page-1 vector below is the
// first 512 bytes of a REAL database built by the sqlite3 CLI (3.46.1) with:
//   PRAGMA page_size=512; PRAGMA encoding='UTF-8';
//   CREATE TABLE t(a INTEGER, b TEXT); INSERT INTO t VALUES(42,'hello'); INSERT INTO t VALUES(7,'world');
// so the parsers are checked against bytes an authoritative implementation actually wrote.

#include "db_multipage.h"
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
    return UNITY_END();
}
