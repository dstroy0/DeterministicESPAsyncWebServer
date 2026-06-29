// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Allen-Bradley DF1 full-duplex frame codec (services/df1): the BCC and
// CRC-16, the frame builder (with DLE byte-stuffing), and the validating, un-stuffing parser.
// Vectors verified against AB pub. 1770-6.5.16. Pure host tests.

#include "services/df1/df1.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The manual's BCC example: data sum 0x20 -> BCC 0xE0 (2's complement).
void test_bcc_vector()
{
    const uint8_t data[] = {0x07, 0x19}; // 0x07 + 0x19 = 0x20
    TEST_ASSERT_EQUAL_HEX8(0xE0, df1_bcc(data, sizeof(data)));
    TEST_ASSERT_EQUAL_HEX8(0x00, df1_bcc(nullptr, 0)); // empty -> 0
}

// CRC-16/ARC (DF1 polynomial, init 0) check value for "123456789" is 0xBB3D.
void test_crc_vector()
{
    TEST_ASSERT_EQUAL_HEX16(0xBB3D, df1_crc((const uint8_t *)"123456789", 9));
}

void test_build_bcc_frame()
{
    const uint8_t data[] = {0x07, 0x19};
    uint8_t buf[16];
    size_t n = df1_build_frame(buf, sizeof(buf), data, sizeof(data), DF1_CHECK_BCC);
    const uint8_t expect[] = {DF1_DLE, DF1_STX, 0x07, 0x19, DF1_DLE, DF1_ETX, 0xE0};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// A DLE (0x10) data byte is doubled on the wire but counted once in the BCC.
void test_build_dle_stuffing()
{
    const uint8_t data[] = {0x10, 0x05}; // sum 0x15 -> BCC 0xEB
    uint8_t buf[16];
    size_t n = df1_build_frame(buf, sizeof(buf), data, sizeof(data), DF1_CHECK_BCC);
    const uint8_t expect[] = {DF1_DLE, DF1_STX, 0x10, 0x10, 0x05, DF1_DLE, DF1_ETX, 0xEB};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_round_trip_bcc()
{
    const uint8_t data[] = {0x10, 0x05, 0x10, 0xAB}; // two embedded DLEs
    uint8_t buf[32];
    size_t n = df1_build_frame(buf, sizeof(buf), data, sizeof(data), DF1_CHECK_BCC);

    uint8_t out[32];
    size_t out_len;
    TEST_ASSERT_TRUE(df1_parse_frame(buf, n, DF1_CHECK_BCC, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), out_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, out_len);
}

void test_round_trip_crc()
{
    const uint8_t data[] = {0x00, 0x05, 0x0F, 0x00, 0x10, 0x42}; // includes a DLE (0x10)
    uint8_t buf[32];
    size_t n = df1_build_frame(buf, sizeof(buf), data, sizeof(data), DF1_CHECK_CRC);

    uint8_t out[32];
    size_t out_len;
    TEST_ASSERT_TRUE(df1_parse_frame(buf, n, DF1_CHECK_CRC, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), out_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, out_len);
}

void test_empty_data_frame()
{
    uint8_t buf[16];
    size_t n = df1_build_frame(buf, sizeof(buf), nullptr, 0, DF1_CHECK_BCC);
    const uint8_t expect[] = {DF1_DLE, DF1_STX, DF1_DLE, DF1_ETX, 0x00};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    uint8_t out[4];
    size_t out_len;
    TEST_ASSERT_TRUE(df1_parse_frame(buf, n, DF1_CHECK_BCC, out, sizeof(out), &out_len));
    TEST_ASSERT_EQUAL_size_t(0, out_len);
}

void test_parse_rejects_bad()
{
    const uint8_t data[] = {0x07, 0x19};
    uint8_t buf[16];
    size_t n = df1_build_frame(buf, sizeof(buf), data, sizeof(data), DF1_CHECK_BCC);

    uint8_t out[16];
    size_t out_len;
    uint8_t corrupt[16];

    // Corrupt a data byte -> BCC mismatch.
    memcpy(corrupt, buf, n);
    corrupt[2] ^= 0x01;
    TEST_ASSERT_FALSE(df1_parse_frame(corrupt, n, DF1_CHECK_BCC, out, sizeof(out), &out_len));

    // Missing DLE STX leader.
    memcpy(corrupt, buf, n);
    corrupt[1] = 0x55;
    TEST_ASSERT_FALSE(df1_parse_frame(corrupt, n, DF1_CHECK_BCC, out, sizeof(out), &out_len));

    // Truncated frame.
    TEST_ASSERT_FALSE(df1_parse_frame(buf, n - 1, DF1_CHECK_BCC, out, sizeof(out), &out_len));

    // out buffer too small for the un-stuffed data.
    TEST_ASSERT_FALSE(df1_parse_frame(buf, n, DF1_CHECK_BCC, out, 1, &out_len));
}

void test_build_overflow_fails_closed()
{
    const uint8_t data[] = {0x07, 0x19, 0x2A};
    uint8_t small[6]; // needs 2 + 3 + 2 + 1 = 8
    TEST_ASSERT_EQUAL_size_t(0, df1_build_frame(small, sizeof(small), data, sizeof(data), DF1_CHECK_BCC));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_bcc_vector);
    RUN_TEST(test_crc_vector);
    RUN_TEST(test_build_bcc_frame);
    RUN_TEST(test_build_dle_stuffing);
    RUN_TEST(test_round_trip_bcc);
    RUN_TEST(test_round_trip_crc);
    RUN_TEST(test_empty_data_frame);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
