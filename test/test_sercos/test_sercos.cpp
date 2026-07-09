// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/sercos: the SERCOS III telegram + IDN codec.

#include "services/sercos/sercos.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_idn_roundtrip(void)
{
    // S-0-0100 (velocity loop): S-parameter, set 0, block 100.
    uint16_t idn = detws_sercos_idn(false, 0, 100);
    TEST_ASSERT_EQUAL_HEX16(0x0064, idn);
    bool prod = true;
    uint8_t set = 9;
    uint16_t block = 0;
    detws_sercos_idn_parse(idn, &prod, &set, &block);
    TEST_ASSERT_FALSE(prod);
    TEST_ASSERT_EQUAL_UINT8(0, set);
    TEST_ASSERT_EQUAL_UINT16(100, block);

    // P-1-0016: product param, set 1, block 16 -> bit15 | (1<<12) | 16 = 0x9010.
    idn = detws_sercos_idn(true, 1, 16);
    TEST_ASSERT_EQUAL_HEX16(0x9010, idn);
    detws_sercos_idn_parse(idn, &prod, &set, &block);
    TEST_ASSERT_TRUE(prod);
    TEST_ASSERT_EQUAL_UINT8(1, set);
    TEST_ASSERT_EQUAL_UINT16(16, block);
}

void test_telegram_roundtrip(void)
{
    uint8_t data[4] = {0x10, 0x20, 0x30, 0x40};
    uint8_t out[16];
    size_t n = detws_sercos_build(SERCOS_TEL_MDT, 4, 0x1234, data, 4, out, sizeof(out));
    const uint8_t expect_hdr[] = {SERCOS_TEL_MDT, 4, 0x34, 0x12};
    TEST_ASSERT_EQUAL_size_t(8, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect_hdr, out, 4);

    SercosTelegram t;
    TEST_ASSERT_TRUE(detws_sercos_parse(out, n, &t));
    TEST_ASSERT_EQUAL_HEX8(SERCOS_TEL_MDT, t.type);
    TEST_ASSERT_EQUAL_UINT8(4, t.phase);
    TEST_ASSERT_EQUAL_UINT16(0x1234, t.cycle);
    TEST_ASSERT_EQUAL_size_t(4, t.data_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, t.data, 4);
}

void test_at_telegram_and_rejects(void)
{
    uint8_t out[8];
    size_t n = detws_sercos_build(SERCOS_TEL_AT, 0, 1, nullptr, 0, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(4, n);
    SercosTelegram t;
    TEST_ASSERT_TRUE(detws_sercos_parse(out, n, &t));
    TEST_ASSERT_EQUAL_HEX8(SERCOS_TEL_AT, t.type);
    TEST_ASSERT_EQUAL_size_t(0, t.data_len);
    // Bad type at build; too short + unknown type at parse.
    TEST_ASSERT_EQUAL_size_t(0, detws_sercos_build(0x05, 0, 0, nullptr, 0, out, sizeof(out)));
    TEST_ASSERT_FALSE(detws_sercos_parse(out, 3, &t));
    uint8_t bad[4] = {0x07, 0, 0, 0};
    TEST_ASSERT_FALSE(detws_sercos_parse(bad, 4, &t));
}

void test_sercos_build_guards()
{
    uint8_t out[64];
    uint8_t data[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_size_t(0, detws_sercos_build(0xEE, 0, 0, data, sizeof(data), out, sizeof(out))); // bad type
    TEST_ASSERT_EQUAL_size_t(0, detws_sercos_build(SERCOS_TEL_MDT, 0, 0, data, sizeof(data), out, 2)); // cap too small
    TEST_ASSERT_EQUAL_size_t(0, detws_sercos_build(SERCOS_TEL_MDT, 0, 0, nullptr, 4, out, sizeof(out))); // null data
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_idn_roundtrip);
    RUN_TEST(test_telegram_roundtrip);
    RUN_TEST(test_at_telegram_and_rejects);
    RUN_TEST(test_sercos_build_guards);
    return UNITY_END();
}
