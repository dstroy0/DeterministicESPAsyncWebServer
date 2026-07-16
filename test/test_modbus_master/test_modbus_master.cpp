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
    size_t n = modbus_build_read((uint8_t)ModbusFunction::MODBUS_FC_READ_HOLDING_REGS, 1, 1, 0, 2, adu, sizeof(adu));
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
    size_t rn = modbus_build_read((uint8_t)ModbusFunction::MODBUS_FC_READ_HOLDING_REGS, 7, 1, 0, 2, req, sizeof(req));
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
    size_t rn =
        modbus_build_read((uint8_t)ModbusFunction::MODBUS_FC_READ_HOLDING_REGS, 9, 1, 60000, 1, req, sizeof(req));
    uint8_t resp[MODBUS_ADU_MAX];
    size_t pn = modbus_process_adu(req, rn, resp, sizeof(resp));
    TEST_ASSERT_TRUE(pn > 0);

    uint16_t regs[4];
    uint8_t ex = 0;
    int got = modbus_parse_response(resp, pn, regs, 4, &ex);
    TEST_ASSERT_EQUAL_INT(0, got);
    TEST_ASSERT_EQUAL_UINT8(ModbusException::MODBUS_EX_ILLEGAL_DATA_ADDRESS, ex);
}

void test_parse_short_frame_fails()
{
    uint8_t buf[4] = {0, 1, 0, 0};
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(buf, sizeof(buf), nullptr, 0, nullptr));
}

// build rejects a null output buffer, and accepts FC 0x04 (read input registers).
void test_build_null_out_and_input_fc()
{
    uint8_t adu[16];
    TEST_ASSERT_EQUAL_size_t(0, modbus_build_read(0x03, 1, 1, 0, 2, nullptr, 16)); // null out
    size_t n = modbus_build_read(0x04, 1, 1, 0, 2, adu, sizeof(adu));              // FC 0x04 is valid
    TEST_ASSERT_EQUAL_size_t(12, n);
    TEST_ASSERT_EQUAL_HEX8(0x04, adu[7]);
}

// parse rejects a null ADU pointer.
void test_parse_null_adu()
{
    uint16_t regs[4];
    uint8_t ex = 0xFF;
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(nullptr, 12, regs, 4, &ex));
}

// parse rejects a response whose MBAP protocol id is not 0 (either high or low byte non-zero).
void test_parse_bad_protocol_id()
{
    uint16_t regs[4];
    uint8_t ex = 0xFF;
    uint8_t adu[13] = {0, 7, 0, 1, 0, 7, 1, 3, 4, 0, 0, 0, 0}; // proto id low byte = 1
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(adu, sizeof(adu), regs, 4, &ex));
    adu[2] = 1; // proto id high byte = 1
    adu[3] = 0;
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(adu, sizeof(adu), regs, 4, &ex));
}

// parse rejects a non-exception response function that is neither read-holding nor read-input.
void test_parse_unexpected_function()
{
    uint16_t regs[4];
    uint8_t ex = 0xFF;
    uint8_t adu[13] = {0, 7, 0, 0, 0, 7, 1, 0x06, 4, 0, 0, 0, 0}; // FC 0x06 (write single), not a read
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(adu, sizeof(adu), regs, 4, &ex));
}

// An exception response parsed with a null exception_out still returns 0 registers (no write attempted).
void test_parse_exception_null_out()
{
    uint16_t regs[4];
    uint8_t adu[9] = {0, 9, 0, 0, 0, 3, 1, 0x83, 0x02}; // FC 0x03|0x80 exception, code 2
    TEST_ASSERT_EQUAL_INT(0, modbus_parse_response(adu, sizeof(adu), regs, 4, nullptr));
}

// parse rejects an odd byte count and a byte count that runs past the frame.
void test_parse_bad_byte_count()
{
    uint16_t regs[4];
    uint8_t ex = 0xFF;
    uint8_t odd[13] = {0, 7, 0, 0, 0, 7, 1, 3, 3, 0, 0, 0, 0}; // byte count 3 is odd
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(odd, sizeof(odd), regs, 4, &ex));
    uint8_t truncated[11] = {0, 7, 0, 0, 0, 7, 1, 3, 4, 0, 0}; // byte count 4 but only 2 present
    TEST_ASSERT_EQUAL_INT(-1, modbus_parse_response(truncated, sizeof(truncated), regs, 4, &ex));
}

// parse stops at max_regs (extra registers dropped), and tolerates a null regs_out (counts without writing).
void test_parse_max_regs_and_null_out()
{
    uint8_t ex = 0xFF;
    // A 4-register response (byte count 8), len = 9 + 8 = 17.
    uint8_t adu[17] = {0, 7, 0, 0, 0, 11, 1, 3, 8, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint16_t regs[2];
    int got = modbus_parse_response(adu, sizeof(adu), regs, 2, &ex); // only room for 2
    TEST_ASSERT_EQUAL_INT(2, got);
    TEST_ASSERT_EQUAL_HEX16(0x1122, regs[0]);
    TEST_ASSERT_EQUAL_HEX16(0x3344, regs[1]);
    // Null regs_out: still counts every register present.
    int got2 = modbus_parse_response(adu, sizeof(adu), nullptr, 8, &ex);
    TEST_ASSERT_EQUAL_INT(4, got2);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_read_bytes);
    RUN_TEST(test_build_rejects_bad_args);
    RUN_TEST(test_round_trip_holding_regs);
    RUN_TEST(test_round_trip_exception);
    RUN_TEST(test_parse_short_frame_fails);
    RUN_TEST(test_build_null_out_and_input_fc);
    RUN_TEST(test_parse_null_adu);
    RUN_TEST(test_parse_bad_protocol_id);
    RUN_TEST(test_parse_unexpected_function);
    RUN_TEST(test_parse_exception_null_out);
    RUN_TEST(test_parse_bad_byte_count);
    RUN_TEST(test_parse_max_regs_and_null_out);
    return UNITY_END();
}
