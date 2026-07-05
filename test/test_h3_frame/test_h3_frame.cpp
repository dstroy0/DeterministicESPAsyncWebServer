// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HTTP/3 framing layer (network_drivers/presentation/http3/h3_frame, RFC 9114
// sec 7): the type+length varint header parse/write (incl. a multi-byte length), the DATA /
// HEADERS / SETTINGS / GOAWAY builders, the SETTINGS payload round-trip + reserved-id rejection,
// and the reserved HTTP/2 frame-type check.

#include "network_drivers/presentation/http3/h3_frame.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_header_roundtrip()
{
    uint8_t b[8];
    // SETTINGS(4), length 0 -> two 1-byte varints.
    TEST_ASSERT_EQUAL_INT(2, (int)h3_frame_write_header(b, sizeof b, H3_SETTINGS, 0));
    const uint8_t exp[2] = {0x04, 0x00};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, b, 2);
    H3Frame f;
    TEST_ASSERT_TRUE(h3_frame_parse(b, 2, &f));
    TEST_ASSERT_TRUE(f.type == H3_SETTINGS && f.length == 0 && f.header_len == 2);

    // HEADERS(1), length 1000 -> a 2-byte length varint (0x43E8).
    TEST_ASSERT_EQUAL_INT(3, (int)h3_frame_write_header(b, sizeof b, H3_HEADERS, 1000));
    const uint8_t exp2[3] = {0x01, 0x43, 0xE8};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp2, b, 3);
    TEST_ASSERT_TRUE(h3_frame_parse(b, 3, &f));
    TEST_ASSERT_TRUE(f.type == H3_HEADERS && f.length == 1000 && f.header_len == 3);
}

void test_build_data_and_goaway()
{
    uint8_t b[32];
    TEST_ASSERT_EQUAL_INT(7, (int)h3_build_data(b, sizeof b, (const uint8_t *)"hello", 5));
    const uint8_t data[7] = {0x00, 0x05, 'h', 'e', 'l', 'l', 'o'};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, b, 7);

    TEST_ASSERT_EQUAL_INT(3, (int)h3_build_goaway(b, sizeof b, 8));
    const uint8_t ga[3] = {0x07, 0x01, 0x08};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(ga, b, 3);
}

void test_settings_roundtrip()
{
    const uint64_t ids[2] = {H3_SETTINGS_QPACK_MAX_TABLE_CAPACITY, H3_SETTINGS_MAX_FIELD_SECTION_SIZE};
    const uint64_t vals[2] = {4096, 1048576};
    uint8_t b[32];
    size_t n = h3_build_settings(b, sizeof b, ids, vals, 2);
    // header (type 0x04 + length 0x08) + payload: 01 5000 06 80100000
    const uint8_t exp[10] = {0x04, 0x08, 0x01, 0x50, 0x00, 0x06, 0x80, 0x10, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(10, (int)n);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, b, 10);

    H3Frame f;
    TEST_ASSERT_TRUE(h3_frame_parse(b, n, &f));
    TEST_ASSERT_TRUE(f.type == H3_SETTINGS && f.length == 8);
    H3Settings s;
    h3_settings_defaults(&s);
    TEST_ASSERT_TRUE(h3_parse_settings(b + f.header_len, (size_t)f.length, &s));
    TEST_ASSERT_TRUE(s.qpack_max_table_capacity == 4096);
    TEST_ASSERT_TRUE(s.max_field_section_size == 1048576);
    TEST_ASSERT_TRUE(s.qpack_blocked_streams == 0);
}

void test_reserved()
{
    TEST_ASSERT_TRUE(h3_frame_type_reserved(0x02));
    TEST_ASSERT_TRUE(h3_frame_type_reserved(0x06));
    TEST_ASSERT_TRUE(h3_frame_type_reserved(0x08));
    TEST_ASSERT_TRUE(h3_frame_type_reserved(0x09));
    TEST_ASSERT_FALSE(h3_frame_type_reserved(H3_DATA));
    TEST_ASSERT_FALSE(h3_frame_type_reserved(H3_HEADERS));
    TEST_ASSERT_FALSE(h3_frame_type_reserved(H3_SETTINGS));

    // A SETTINGS payload using a reserved HTTP/2 settings id (0x02) must be rejected.
    H3Settings s;
    h3_settings_defaults(&s);
    const uint8_t bad[2] = {0x02, 0x00};
    TEST_ASSERT_FALSE(h3_parse_settings(bad, 2, &s));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_header_roundtrip);
    RUN_TEST(test_build_data_and_goaway);
    RUN_TEST(test_settings_roundtrip);
    RUN_TEST(test_reserved);
    return UNITY_END();
}
