// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/hart: the HART command frame + HART-IP header codec.

#include "services/hart/hart.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_checksum(void)
{
    // XOR longitudinal parity.
    const uint8_t b[] = {0x02, 0x80, 0x00, 0x00};
    TEST_ASSERT_EQUAL_HEX8(0x82, detws_hart_checksum(b, sizeof(b)));
}

void test_build_command0_short(void)
{
    // Command 0 (read unique id), STX, primary-master short address 0, no data.
    uint8_t addr = 0x80;
    uint8_t out[16];
    size_t n = detws_hart_build(HART_DELIM_STX, &addr, 1, 0x00, nullptr, 0, out, sizeof(out));
    const uint8_t expect[] = {0x02, 0x80, 0x00, 0x00, 0x82};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, sizeof(expect));
}

void test_build_with_data(void)
{
    uint8_t addr = 0x80;
    uint8_t data[] = {0xAB, 0xCD};
    uint8_t out[16];
    size_t n = detws_hart_build(HART_DELIM_STX, &addr, 1, 0x01, data, 2, out, sizeof(out));
    // [02 80 01 02 AB CD ck], ck = 02^80^01^02^AB^CD = 0xE7.
    const uint8_t expect[] = {0x02, 0x80, 0x01, 0x02, 0xAB, 0xCD, 0xE7};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, sizeof(expect));
}

void test_build_long_address(void)
{
    uint8_t addr[5] = {0x86, 0x01, 0x02, 0x03, 0x04}; // long addr, master bit set
    uint8_t out[24];
    size_t n =
        detws_hart_build((uint8_t)(HART_DELIM_STX | HART_DELIM_LONG_ADDR), addr, 5, 0x03, nullptr, 0, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(1 + 5 + 1 + 1 + 0 + 1, n); // delim+addr+cmd+bc+ck
    HartFrame f;
    TEST_ASSERT_TRUE(detws_hart_parse(out, n, &f));
    TEST_ASSERT_EQUAL_size_t(5, f.addr_len);
    TEST_ASSERT_EQUAL_HEX8(0x03, f.command);
}

void test_parse_roundtrip_and_bad_checksum(void)
{
    uint8_t addr = 0x80;
    uint8_t data[] = {0x11, 0x22, 0x33};
    uint8_t buf[16];
    size_t n = detws_hart_build(HART_DELIM_STX, &addr, 1, 0x2A, data, 3, buf, sizeof(buf));
    HartFrame f;
    TEST_ASSERT_TRUE(detws_hart_parse(buf, n, &f));
    TEST_ASSERT_EQUAL_HEX8(0x2A, f.command);
    TEST_ASSERT_EQUAL_size_t(3, f.data_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, f.data, 3);
    // Corrupt the checksum -> parse fails.
    buf[n - 1] ^= 0xFF;
    TEST_ASSERT_FALSE(detws_hart_parse(buf, n, &f));
    // Truncated -> fails.
    TEST_ASSERT_FALSE(detws_hart_parse(buf, 3, &f));
}

void test_hartip_header(void)
{
    uint8_t out[8];
    size_t n = detws_hartip_build_header(HARTIP_MSG_REQUEST, HARTIP_ID_TOKEN_PDU, 0, 0x1234, 13, out, sizeof(out));
    const uint8_t expect[] = {0x01, 0x00, 0x03, 0x00, 0x12, 0x34, 0x00, 0x0D};
    TEST_ASSERT_EQUAL_size_t(8, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 8);
    // Too small a buffer -> 0.
    TEST_ASSERT_EQUAL_size_t(0, detws_hartip_build_header(0, 0, 0, 0, 0, out, 4));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_checksum);
    RUN_TEST(test_build_command0_short);
    RUN_TEST(test_build_with_data);
    RUN_TEST(test_build_long_address);
    RUN_TEST(test_parse_roundtrip_and_bad_checksum);
    RUN_TEST(test_hartip_header);
    return UNITY_END();
}
