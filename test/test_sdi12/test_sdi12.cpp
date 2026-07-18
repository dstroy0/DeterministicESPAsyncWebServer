// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SDI-12 codec (services/sdi12): the command builders, the measurement
// response parser (atttn), the data-value splitter, and the SDI-12 CRC (compute / encode /
// verify round-trip). Pure host tests.

#include "services/sdi12/sdi12.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_command_builders()
{
    char buf[16];
    TEST_ASSERT_EQUAL_size_t(2, dws_sdi12_build_ack(buf, sizeof(buf), '0'));
    TEST_ASSERT_EQUAL_STRING("0!", buf);
    TEST_ASSERT_EQUAL_size_t(3, dws_sdi12_build_identify(buf, sizeof(buf), '0'));
    TEST_ASSERT_EQUAL_STRING("0I!", buf);
    dws_sdi12_build_measure(buf, sizeof(buf), '3', false);
    TEST_ASSERT_EQUAL_STRING("3M!", buf);
    dws_sdi12_build_measure(buf, sizeof(buf), '3', true);
    TEST_ASSERT_EQUAL_STRING("3MC!", buf);
    dws_sdi12_build_concurrent(buf, sizeof(buf), '1', false);
    TEST_ASSERT_EQUAL_STRING("1C!", buf);
    dws_sdi12_build_data(buf, sizeof(buf), '0', 0);
    TEST_ASSERT_EQUAL_STRING("0D0!", buf);
    dws_sdi12_build_change_address(buf, sizeof(buf), '0', '5');
    TEST_ASSERT_EQUAL_STRING("0A5!", buf);
    dws_sdi12_build_query_address(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("?!", buf);
}

void test_parse_measure_m()
{
    // aM! response "0" + "012" (12 s) + "2" (2 values).
    const char *resp = "00122\r\n";
    char addr = 0;
    uint16_t ready = 0;
    uint8_t n = 0;
    TEST_ASSERT_TRUE(dws_sdi12_parse_measure(resp, strlen(resp), &addr, &ready, &n));
    TEST_ASSERT_EQUAL_CHAR('0', addr);
    TEST_ASSERT_EQUAL_UINT16(12, ready);
    TEST_ASSERT_EQUAL_UINT8(2, n);
}

void test_parse_measure_concurrent_two_digit_count()
{
    // aC! response "0" + "013" (13 s) + "10" (10 values).
    const char *resp = "001310\r\n";
    char addr = 0;
    uint16_t ready = 0;
    uint8_t n = 0;
    TEST_ASSERT_TRUE(dws_sdi12_parse_measure(resp, strlen(resp), &addr, &ready, &n));
    TEST_ASSERT_EQUAL_UINT16(13, ready);
    TEST_ASSERT_EQUAL_UINT8(10, n);
}

void test_parse_values()
{
    const char *resp = "0+3.14-2.5+0.001\r\n";
    float v[4];
    size_t n = 0;
    TEST_ASSERT_TRUE(dws_sdi12_parse_values(resp, strlen(resp), v, 4, &n));
    TEST_ASSERT_EQUAL_size_t(3, n);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.14f, v[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -2.5f, v[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.001f, v[2]);
}

void test_crc_roundtrip()
{
    // Build a response, append the SDI-12 CRC, then verify it (and that corruption fails).
    char resp[32];
    strcpy(resp, "0+3.14");
    size_t len = strlen(resp);
    char crc[SDI12_CRC_CHARS];
    dws_sdi12_crc_encode(dws_sdi12_crc16((const uint8_t *)resp, len), crc);
    memcpy(resp + len, crc, SDI12_CRC_CHARS);
    size_t total = len + SDI12_CRC_CHARS;

    TEST_ASSERT_TRUE(dws_sdi12_check_crc(resp, total));

    // a CRLF after the CRC is tolerated.
    resp[total] = '\r';
    resp[total + 1] = '\n';
    TEST_ASSERT_TRUE(dws_sdi12_check_crc(resp, total + 2));

    // corrupting a data octet breaks the CRC.
    char bad[32];
    memcpy(bad, resp, total);
    bad[1] ^= 0x01;
    TEST_ASSERT_FALSE(dws_sdi12_check_crc(bad, total));
}

// The CRC octets are always printable (0x40..0x7F), per the SDI-12 encoding.
void test_crc_encode_printable()
{
    char crc[SDI12_CRC_CHARS];
    dws_sdi12_crc_encode(0xFFFF, crc);
    for (int i = 0; i < SDI12_CRC_CHARS; i++)
        TEST_ASSERT_TRUE((uint8_t)crc[i] >= 0x40 && (uint8_t)crc[i] <= 0x7F);
}

// Builder / parser / CRC reject and skip branches.
void test_sdi12_error_paths()
{
    char buf[16];
    TEST_ASSERT_EQUAL_size_t(0, dws_sdi12_build(nullptr, sizeof(buf), '0', "M")); // null buf
    TEST_ASSERT_EQUAL_size_t(0, dws_sdi12_build(buf, sizeof(buf), '0', nullptr)); // null body
    TEST_ASSERT_EQUAL_size_t(0, dws_sdi12_build(buf, 2, '0', "M"));               // cap too small
    TEST_ASSERT_EQUAL_size_t(0, dws_sdi12_build_data(buf, sizeof(buf), '0', 10)); // d_index > 9

    char addr;
    uint16_t ready;
    uint8_t n;
    TEST_ASSERT_FALSE(dws_sdi12_parse_measure(nullptr, 5, &addr, &ready, &n)); // null resp
    TEST_ASSERT_FALSE(dws_sdi12_parse_measure("012", 3, &addr, &ready, &n));   // len < 5
    TEST_ASSERT_FALSE(dws_sdi12_parse_measure("0X122", 5, &addr, &ready, &n)); // non-digit in the ttt field
    TEST_ASSERT_FALSE(dws_sdi12_parse_measure("0120X", 5, &addr, &ready, &n)); // non-digit value count

    float v[4];
    size_t cnt = 0;
    TEST_ASSERT_FALSE(dws_sdi12_parse_values(nullptr, 3, v, 4, &cnt));     // null resp
    TEST_ASSERT_FALSE(dws_sdi12_parse_values("0+1", 3, nullptr, 4, &cnt)); // null out
    TEST_ASSERT_FALSE(dws_sdi12_parse_values("0+1", 3, v, 4, nullptr));    // null count
    // A non +/- separator is skipped; a trailing '+' with no digits is skipped.
    const char *r = "0X+1.5+\r\n";
    TEST_ASSERT_TRUE(dws_sdi12_parse_values(r, strlen(r), v, 4, &cnt));
    TEST_ASSERT_EQUAL_size_t(1, cnt);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.5f, v[0]);

    TEST_ASSERT_FALSE(dws_sdi12_check_crc(nullptr, 5));  // null
    TEST_ASSERT_FALSE(dws_sdi12_check_crc("ab\r\n", 4)); // after trimming CRLF, too short for data + CRC
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_command_builders);
    RUN_TEST(test_parse_measure_m);
    RUN_TEST(test_parse_measure_concurrent_two_digit_count);
    RUN_TEST(test_parse_values);
    RUN_TEST(test_crc_roundtrip);
    RUN_TEST(test_crc_encode_printable);
    RUN_TEST(test_sdi12_error_paths);
    return UNITY_END();
}
