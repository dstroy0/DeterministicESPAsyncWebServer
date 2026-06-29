// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Protocol Buffers wire codec (services/protobuf): the streaming
// writer and the cursor reader, anchored on the canonical spec vectors. Pure host tests.

#include "services/protobuf/protobuf.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The spec's canonical example: field 1 = 150 encodes as 08 96 01.
void test_vector_field1_150()
{
    PbWriter w;
    uint8_t buf[16];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_uint64(&w, 1, 150);
    size_t n = pb_writer_finish(&w);
    const uint8_t expect[] = {0x08, 0x96, 0x01};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// The spec's string example: field 2 = "testing".
void test_vector_string_testing()
{
    PbWriter w;
    uint8_t buf[32];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_string(&w, 2, "testing");
    size_t n = pb_writer_finish(&w);
    const uint8_t expect[] = {0x12, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_zigzag_mapping()
{
    // Decode: encoded 1 -> -1, 2 -> 1, 3 -> -2.
    TEST_ASSERT_EQUAL_INT64(-1, pb_zigzag64(1));
    TEST_ASSERT_EQUAL_INT64(1, pb_zigzag64(2));
    TEST_ASSERT_EQUAL_INT64(-2, pb_zigzag64(3));
    TEST_ASSERT_EQUAL_INT32(-2, pb_zigzag32(3));

    // Encode sint64(-1) -> tag 08, varint 01.
    PbWriter w;
    uint8_t buf[16];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_sint64(&w, 1, -1);
    size_t n = pb_writer_finish(&w);
    const uint8_t expect[] = {0x08, 0x01};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_fixed_and_float_bytes()
{
    PbWriter w;
    uint8_t buf[32];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_fixed32(&w, 3, 0xDEADBEEF);
    pb_float(&w, 4, 1.5f); // 1.5f bits == 0x3FC00000
    size_t n = pb_writer_finish(&w);
    const uint8_t expect[] = {
        0x1D, 0xEF, 0xBE, 0xAD, 0xDE, // field 3 (3<<3|5), LE 0xDEADBEEF
        0x25, 0x00, 0x00, 0xC0, 0x3F  // field 4 (4<<3|5), LE 1.5f
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// Round-trip a mixed message through the writer and the cursor reader.
void test_round_trip_reader()
{
    PbWriter w;
    uint8_t buf[64];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_uint64(&w, 1, 150);
    pb_string(&w, 2, "hi");
    pb_fixed32(&w, 3, 0x01020304);
    pb_double(&w, 4, 2.5);
    pb_sint64(&w, 5, -1234567);
    size_t total = pb_writer_finish(&w);
    TEST_ASSERT_GREATER_THAN(0, (int)total);

    size_t pos = 0;
    PbField f;

    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(1, f.field_number);
    TEST_ASSERT_EQUAL_UINT8(PB_WT_VARINT, f.wire_type);
    TEST_ASSERT_EQUAL_UINT64(150, f.value);

    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(2, f.field_number);
    TEST_ASSERT_EQUAL_UINT8(PB_WT_LEN, f.wire_type);
    TEST_ASSERT_EQUAL_size_t(2, f.len);
    TEST_ASSERT_EQUAL_MEMORY("hi", f.data, 2);

    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(3, f.field_number);
    TEST_ASSERT_EQUAL_UINT8(PB_WT_I32, f.wire_type);
    TEST_ASSERT_EQUAL_HEX32(0x01020304, (uint32_t)f.value);

    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(4, f.field_number);
    TEST_ASSERT_EQUAL_UINT8(PB_WT_I64, f.wire_type);
    TEST_ASSERT_EQUAL_HEX64(0x4004000000000000ULL, f.value); // IEEE-754 bits of 2.5
    TEST_ASSERT_TRUE(pb_double_bits(f.value) == 2.5);

    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(5, f.field_number);
    TEST_ASSERT_EQUAL_INT64(-1234567, pb_zigzag64(f.value));

    TEST_ASSERT_FALSE(pb_read_field(buf, total, &pos, &f)); // end of buffer
}

// int64 negative encodes as a 10-byte two's-complement varint.
void test_int64_negative()
{
    PbWriter w;
    uint8_t buf[16];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_int64(&w, 1, -1);
    size_t total = pb_writer_finish(&w);
    TEST_ASSERT_EQUAL_size_t(11, total); // 1 tag + 10 varint

    size_t pos = 0;
    PbField f;
    TEST_ASSERT_TRUE(pb_read_field(buf, total, &pos, &f));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFFFFFFFFFFFFFULL, f.value);
    TEST_ASSERT_EQUAL_INT64(-1, (int64_t)f.value);
}

void test_varint_and_overflow()
{
    // A multi-byte varint round-trips.
    PbWriter w;
    uint8_t buf[16];
    pb_writer_init(&w, buf, sizeof(buf));
    pb_write_varint(&w, 300); // 0xAC 0x02
    TEST_ASSERT_EQUAL_size_t(2, pb_writer_finish(&w));
    TEST_ASSERT_EQUAL_HEX8(0xAC, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[1]);
    size_t pos = 0;
    uint64_t v;
    TEST_ASSERT_TRUE(pb_read_varint(buf, 2, &pos, &v));
    TEST_ASSERT_EQUAL_UINT64(300, v);

    // Overflow fails closed.
    PbWriter sw;
    uint8_t small[4];
    pb_writer_init(&sw, small, sizeof(small));
    pb_string(&sw, 1, "this is way too long");
    TEST_ASSERT_EQUAL_size_t(0, pb_writer_finish(&sw));
}

void test_malformed_reads()
{
    size_t pos = 0;
    uint64_t v;
    const uint8_t trunc[] = {0x80}; // continuation bit set, no terminating byte
    TEST_ASSERT_FALSE(pb_read_varint(trunc, sizeof(trunc), &pos, &v));

    PbField f;
    pos = 0;
    const uint8_t bad_len[] = {0x12, 0x05, 'h', 'i'}; // LEN says 5 but only 2 bytes follow
    TEST_ASSERT_FALSE(pb_read_field(bad_len, sizeof(bad_len), &pos, &f));

    pos = 0;
    const uint8_t group[] = {0x0B}; // field 1 wire type 3 (SGROUP) - unsupported
    TEST_ASSERT_FALSE(pb_read_field(group, sizeof(group), &pos, &f));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_vector_field1_150);
    RUN_TEST(test_vector_string_testing);
    RUN_TEST(test_zigzag_mapping);
    RUN_TEST(test_fixed_and_float_bytes);
    RUN_TEST(test_round_trip_reader);
    RUN_TEST(test_int64_negative);
    RUN_TEST(test_varint_and_overflow);
    RUN_TEST(test_malformed_reads);
    return UNITY_END();
}
