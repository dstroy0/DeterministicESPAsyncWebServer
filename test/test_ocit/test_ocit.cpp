// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/ocit: the OCIT-Outstations message codec.

#include "services/ocit/ocit.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_build_and_parse(void)
{
    uint8_t value[4] = {0x00, 0x00, 0x12, 0x34};
    uint8_t out[16];
    size_t n = dws_ocit_build(OcitMsgType::OCIT_MSG_SET, 0x0102, 0x0003, OcitType::OCIT_TYPE_UINT32, value, 4, out,
                              sizeof(out));
    // [02][01 02][00 03][04][00 00 12 34] = 10 bytes.
    const uint8_t expect[] = {0x02, 0x01, 0x02, 0x00, 0x03, 0x04, 0x00, 0x00, 0x12, 0x34};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);

    OcitMsg m;
    TEST_ASSERT_TRUE(dws_ocit_parse(out, n, &m));
    TEST_ASSERT_EQUAL_HEX8(OcitMsgType::OCIT_MSG_SET, m.msg_type);
    TEST_ASSERT_EQUAL_HEX16(0x0102, m.object_type);
    TEST_ASSERT_EQUAL_HEX16(0x0003, m.instance);
    TEST_ASSERT_EQUAL_HEX8(OcitType::OCIT_TYPE_UINT32, m.data_type);
    TEST_ASSERT_EQUAL_size_t(4, m.value_len);
}

void test_set_u16_helper(void)
{
    uint8_t out[16];
    size_t n = dws_ocit_set_u16(0x00A0, 0x0005, 0xBEEF, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(8, n);
    OcitMsg m;
    TEST_ASSERT_TRUE(dws_ocit_parse(out, n, &m));
    TEST_ASSERT_EQUAL_HEX8(OcitMsgType::OCIT_MSG_SET, m.msg_type);
    TEST_ASSERT_EQUAL_HEX8(OcitType::OCIT_TYPE_UINT16, m.data_type);
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, dws_ocit_value_u16(&m));
}

void test_get_no_value(void)
{
    uint8_t out[8];
    size_t n = dws_ocit_build(OcitMsgType::OCIT_MSG_GET, 0x0102, 0x0003, OcitType::OCIT_TYPE_UINT16, nullptr, 0, out,
                              sizeof(out));
    TEST_ASSERT_EQUAL_size_t(6, n);
    OcitMsg m;
    TEST_ASSERT_TRUE(dws_ocit_parse(out, n, &m));
    TEST_ASSERT_EQUAL_size_t(0, m.value_len);
    TEST_ASSERT_NULL(m.value);
    // value_u16 on a mismatched value -> 0.
    TEST_ASSERT_EQUAL_HEX16(0, dws_ocit_value_u16(&m));
}

void test_parse_rejects_short(void)
{
    OcitMsg m;
    uint8_t tooshort[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    TEST_ASSERT_FALSE(dws_ocit_parse(tooshort, 5, &m));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_build_and_parse);
    RUN_TEST(test_set_u16_helper);
    RUN_TEST(test_get_no_value);
    RUN_TEST(test_parse_rejects_short);
    return UNITY_END();
}
