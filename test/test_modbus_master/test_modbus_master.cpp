// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Modbus master codec (services/modbus/modbus_master): request
// framing, plus a full master<->slave round-trip against the slave codec
// (modbus_process_adu) - no hardware needed.

#include "services/modbus/modbus.h"
#include "services/modbus/modbus_master.h"
#include <unity.h>

void setUp()
{
    modbus_server_init();
}
void tearDown()
{
}

void test_build_read_bytes()
{
    uint8_t adu[16];
    size_t n = modbus_build_read(MODBUS_FC_READ_HOLDING_REGS, 1, 1, 0, 2, adu, sizeof(adu));
    TEST_ASSERT_EQUAL_size_t(12, n);
    const uint8_t expect[12] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expect, adu, 12);
}

void test_build_rejects_bad_args()
{
    uint8_t adu[16];
    TEST_ASSERT_EQUAL_size_t(0, modbus_build_read(0x06, 1, 1, 0, 2, adu, sizeof(adu)));   // not a read FC
    TEST_ASSERT_EQUAL_size_t(0, modbus_build_read(0x03, 1, 1, 0, 0, adu, sizeof(adu)));   // count 0
    TEST_ASSERT_EQUAL_size_t(0, modbus_build_read(0x03, 1, 1, 0, 200, adu, sizeof(adu))); // count > 125
    TEST_ASSERT_EQUAL_size_t(0, modbus_build_read(0x03, 1, 1, 0, 2, adu, 4));             // buffer too small
}

void test_round_trip_holding_regs()
{
    modbus_set_holding_reg(0, 0x1234);
    modbus_set_holding_reg(1, 0xABCD);

    uint8_t req[16];
    size_t rn = modbus_build_read(MODBUS_FC_READ_HOLDING_REGS, 7, 1, 0, 2, req, sizeof(req));
    TEST_ASSERT_EQUAL_size_t(12, rn);

    uint8_t resp[MODBUS_ADU_MAX];
    size_t pn = modbus_process_adu(req, rn, resp, sizeof(resp)); // the slave answers
    TEST_ASSERT_TRUE(pn > 0);

    uint16_t regs[4];
    uint8_t ex = 0xFF;
    int got = modbus_parse_response(resp, pn, regs, 4, &ex);
    TEST_ASSERT_EQUAL_INT(2, got);
    TEST_ASSERT_EQUAL_UINT8(0, ex);
    TEST_ASSERT_EQUAL_HEX16(0x1234, regs[0]);
    TEST_ASSERT_EQUAL_HEX16(0xABCD, regs[1]);
}

void test_round_trip_exception()
{
    // Read a wildly out-of-range address: the slave returns an exception ADU.
    uint8_t req[16];
    size_t rn = modbus_build_read(MODBUS_FC_READ_HOLDING_REGS, 9, 1, 60000, 1, req, sizeof(req));
    uint8_t resp[MODBUS_ADU_MAX];
    size_t pn = modbus_process_adu(req, rn, resp, sizeof(resp));
    TEST_ASSERT_TRUE(pn > 0);

    uint16_t regs[4];
    uint8_t ex = 0;
    int got = modbus_parse_response(resp, pn, regs, 4, &ex);
    TEST_ASSERT_EQUAL_INT(0, got);
    TEST_ASSERT_EQUAL_UINT8(MODBUS_EX_ILLEGAL_DATA_ADDRESS, ex);
}

void test_parse_short_frame_fails()
{
    uint8_t buf[4] = {0, 1, 0, 0};
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(buf, sizeof(buf), nullptr, 0, nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_read_bytes);
    RUN_TEST(test_build_rejects_bad_args);
    RUN_TEST(test_round_trip_holding_regs);
    RUN_TEST(test_round_trip_exception);
    RUN_TEST(test_parse_short_frame_fails);
    return UNITY_END();
}
