// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DNP3 (IEEE 1815) data-link frame codec (services/dnp3): CRC-16/DNP,
// the frame builder, and the CRC-validating, de-blocking parser. Pure host tests.

#include "services/dnp3/dnp3.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// CRC-16/DNP canonical check value for "123456789" is 0xEA82.
void test_crc_check_value()
{
    TEST_ASSERT_EQUAL_HEX16(0xEA82, dws_dnp3_crc((const uint8_t *)"123456789", 9));
}

// A single-block frame has the documented header bytes (little-endian addresses).
void test_build_header_bytes()
{
    const uint8_t data[] = {'a', 'b', 'c'};
    uint8_t buf[32];
    size_t n = dws_dnp3_build_frame(buf, sizeof(buf), 0x44, 0x0004, 0x0001, data, sizeof(data));
    // 10 header + 3 data + 2 block CRC = 15
    TEST_ASSERT_EQUAL_size_t(15, n);
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x64, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x08, buf[2]); // LEN = 5 + 3
    TEST_ASSERT_EQUAL_HEX8(0x44, buf[3]); // control
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[4]); // dest LSB
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[5]); // dest MSB
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[6]); // src LSB
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[7]); // src MSB
    // header CRC over the 8 header octets, low byte first.
    uint16_t hcrc = dws_dnp3_crc(buf, 8);
    TEST_ASSERT_EQUAL_HEX8((uint8_t)(hcrc & 0xFF), buf[8]);
    TEST_ASSERT_EQUAL_HEX8((uint8_t)(hcrc >> 8), buf[9]);
}

void test_round_trip_single_block()
{
    const uint8_t data[] = {0xC0, 0x01, 0x3C, 0x02, 0x06}; // an app-layer fragment, say
    uint8_t buf[32];
    size_t n = dws_dnp3_build_frame(buf, sizeof(buf), 0x44, 0x1234, 0x0A0B, data, sizeof(data));
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    Dnp3Frame f;
    uint8_t user[64];
    size_t user_len;
    TEST_ASSERT_TRUE(dws_dnp3_parse_frame(buf, n, &f, user, sizeof(user), &user_len));
    TEST_ASSERT_EQUAL_HEX8(0x44, f.control);
    TEST_ASSERT_EQUAL_HEX16(0x1234, f.dest);
    TEST_ASSERT_EQUAL_HEX16(0x0A0B, f.src);
    TEST_ASSERT_EQUAL_size_t(sizeof(data), user_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, user, sizeof(data));
}

// More than 16 user octets span multiple CRC'd blocks and reassemble.
void test_round_trip_multi_block()
{
    uint8_t data[20];
    for (size_t i = 0; i < sizeof(data); i++)
        data[i] = (uint8_t)(i + 1);
    uint8_t buf[64];
    size_t n = dws_dnp3_build_frame(buf, sizeof(buf), 0x44, 1, 2, data, sizeof(data));
    // 10 header + 20 data + 2 blocks * 2 CRC = 34
    TEST_ASSERT_EQUAL_size_t(34, n);

    Dnp3Frame f;
    uint8_t user[32];
    size_t user_len;
    TEST_ASSERT_TRUE(dws_dnp3_parse_frame(buf, n, &f, user, sizeof(user), &user_len));
    TEST_ASSERT_EQUAL_size_t(20, user_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, user, sizeof(data));
}

void test_header_only_frame()
{
    uint8_t buf[16];
    size_t n = dws_dnp3_build_frame(buf, sizeof(buf), 0x49, 3, 4, nullptr, 0);
    TEST_ASSERT_EQUAL_size_t(10, n);      // header block only
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[2]); // LEN = 5

    Dnp3Frame f;
    size_t user_len = 999;
    TEST_ASSERT_TRUE(dws_dnp3_parse_frame(buf, n, &f, nullptr, 0, &user_len));
    TEST_ASSERT_EQUAL_HEX8(0x49, f.control);
    TEST_ASSERT_EQUAL_size_t(0, user_len);
}

void test_parse_rejects_bad()
{
    const uint8_t data[] = {'x', 'y', 'z'};
    uint8_t buf[32];
    size_t n = dws_dnp3_build_frame(buf, sizeof(buf), 0x44, 1, 2, data, sizeof(data));

    Dnp3Frame f;
    uint8_t user[32];
    size_t user_len;

    // A corrupted data octet fails the block CRC.
    uint8_t corrupt[32];
    memcpy(corrupt, buf, n);
    corrupt[10] ^= 0xFF;
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(corrupt, n, &f, user, sizeof(user), &user_len));

    // A corrupted header octet fails the header CRC.
    memcpy(corrupt, buf, n);
    corrupt[3] ^= 0x01;
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(corrupt, n, &f, user, sizeof(user), &user_len));

    // Wrong start word.
    memcpy(corrupt, buf, n);
    corrupt[1] = 0x65;
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(corrupt, n, &f, user, sizeof(user), &user_len));

    // Truncated frame.
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(buf, n - 1, &f, user, sizeof(user), &user_len));

    // out_user too small.
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(buf, n, &f, user, 2, &user_len));
}

void test_build_overflow_fails_closed()
{
    const uint8_t data[] = {'a', 'b', 'c'};
    uint8_t small[8];
    TEST_ASSERT_EQUAL_size_t(0, dws_dnp3_build_frame(small, sizeof(small), 0x44, 1, 2, data, sizeof(data)));
    // Over the 250-octet user-data limit also fails.
    uint8_t big[300] = {0};
    uint8_t out[400];
    TEST_ASSERT_EQUAL_size_t(0, dws_dnp3_build_frame(out, sizeof(out), 0x44, 1, 2, big, sizeof(big)));
}

// Frame parsing rejects null / too-short input and a LENGTH field below the overhead.
void test_dnp3_parse_guards()
{
    Dnp3Frame f;
    uint8_t user[256];
    size_t ul = 0;
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(nullptr, 20, &f, user, sizeof(user), &ul));
    uint8_t tiny[4] = {0};
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(tiny, sizeof(tiny), &f, user, sizeof(user), &ul)); // < header block
    uint8_t bad_len[10] = {DNP3_START0, DNP3_START1, 3, 0, 0, 0, 0, 0, 0, 0};                 // LENGTH 3 < overhead 5
    TEST_ASSERT_FALSE(dws_dnp3_parse_frame(bad_len, sizeof(bad_len), &f, user, sizeof(user), &ul));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_dnp3_parse_guards);
    RUN_TEST(test_crc_check_value);
    RUN_TEST(test_build_header_bytes);
    RUN_TEST(test_round_trip_single_block);
    RUN_TEST(test_round_trip_multi_block);
    RUN_TEST(test_header_only_frame);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
