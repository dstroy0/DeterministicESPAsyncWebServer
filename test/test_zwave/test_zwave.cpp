// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Z-Wave Serial API frame codec (services/zwave): the data-frame
// build/parse against the documented GetVersion request (01 03 00 15 E9), the XOR checksum,
// a round trip, malformed framing (bad SOF / checksum / length), incomplete frames, and the
// ACK / NAK / CAN control bytes. Pure host tests.
//
// The env sizes DETWS_ZWAVE_MAX_DATA = 16.

#include "services/zwave/zwave.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_build_getversion_kat()
{
    // Host -> controller FUNC_ID_ZW_GET_VERSION (0x15), a REQ with no data: the documented
    // frame is 01 03 00 15 E9 (checksum = 0xFF ^ 0x03 ^ 0x00 ^ 0x15 = 0xE9).
    const uint8_t expect[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
    uint8_t out[16];
    uint16_t n = zwave_build_frame(zwave_type::ZWAVE_REQ, 0x15, nullptr, 0, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(5, n);
    TEST_ASSERT_EQUAL_MEMORY(expect, out, 5);
}

void test_build_then_parse_round_trip()
{
    const uint8_t data[3] = {0x01, 0x0A, 0xAB};
    uint8_t buf[24];
    uint16_t n = zwave_build_frame(zwave_type::ZWAVE_RES, 0x04, data, 3, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT16(8, n); // SOF + LEN + type + cmd + 3 data + checksum

    uint8_t type = 0, cmd = 0, pdlen = 0;
    const uint8_t *pd = nullptr;
    TEST_ASSERT_EQUAL_INT(8, zwave_parse_frame(buf, n, &type, &cmd, &pd, &pdlen));
    TEST_ASSERT_EQUAL_UINT8(zwave_type::ZWAVE_RES, type);
    TEST_ASSERT_EQUAL_UINT8(0x04, cmd);
    TEST_ASSERT_EQUAL_UINT8(3, pdlen);
    TEST_ASSERT_EQUAL_MEMORY(data, pd, 3);
}

void test_parse_getversion_kat()
{
    const uint8_t frame[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
    uint8_t type = 0, cmd = 0, pdlen = 0xFF;
    TEST_ASSERT_EQUAL_INT(5, zwave_parse_frame(frame, sizeof(frame), &type, &cmd, nullptr, &pdlen));
    TEST_ASSERT_EQUAL_UINT8(zwave_type::ZWAVE_REQ, type);
    TEST_ASSERT_EQUAL_UINT8(0x15, cmd);
    TEST_ASSERT_EQUAL_UINT8(0, pdlen);
}

void test_parse_rejects_bad_sof()
{
    const uint8_t frame[5] = {0x00, 0x03, 0x00, 0x15, 0xE9};
    TEST_ASSERT_EQUAL_INT(-1, zwave_parse_frame(frame, sizeof(frame), nullptr, nullptr, nullptr, nullptr));
}

void test_parse_rejects_bad_checksum()
{
    uint8_t frame[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
    frame[4] ^= 0xFF;
    TEST_ASSERT_EQUAL_INT(-1, zwave_parse_frame(frame, sizeof(frame), nullptr, nullptr, nullptr, nullptr));
}

void test_parse_needs_more_bytes()
{
    const uint8_t frame[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
    TEST_ASSERT_EQUAL_INT(0, zwave_parse_frame(frame, 1, nullptr, nullptr, nullptr, nullptr)); // just SOF
    TEST_ASSERT_EQUAL_INT(0, zwave_parse_frame(frame, 4, nullptr, nullptr, nullptr, nullptr)); // missing checksum
}

void test_parse_rejects_over_length()
{
    // frame_len 80 (> DETWS_ZWAVE_MAX_DATA + 3 = 19) is rejected early.
    const uint8_t frame[4] = {0x01, 0x50, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(-1, zwave_parse_frame(frame, sizeof(frame), nullptr, nullptr, nullptr, nullptr));
}

void test_control_bytes()
{
    TEST_ASSERT_TRUE(zwave_is_ack(Zwave::ZWAVE_ACK));
    TEST_ASSERT_TRUE(zwave_is_nak(Zwave::ZWAVE_NAK));
    TEST_ASSERT_TRUE(zwave_is_can(Zwave::ZWAVE_CAN));
    TEST_ASSERT_FALSE(zwave_is_ack(Zwave::ZWAVE_SOF));
    uint8_t ack[1];
    TEST_ASSERT_EQUAL_UINT16(1, zwave_build_ack(ack, sizeof(ack)));
    TEST_ASSERT_EQUAL_HEX8(Zwave::ZWAVE_ACK, ack[0]);
    TEST_ASSERT_EQUAL_UINT16(0, zwave_build_ack(ack, 0)); // no room
}

void test_build_bounds()
{
    uint8_t data[8] = {0};
    uint8_t small[6];
    TEST_ASSERT_EQUAL_UINT16(0, zwave_build_frame(zwave_type::ZWAVE_REQ, 0x13, data, 4, small, sizeof(small))); // 9 > 6
    uint8_t big[64];
    TEST_ASSERT_EQUAL_UINT16(
        0, zwave_build_frame(zwave_type::ZWAVE_REQ, 0x13, big, 17, big, sizeof(big))); // 17 > MAX_DATA 16
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_getversion_kat);
    RUN_TEST(test_build_then_parse_round_trip);
    RUN_TEST(test_parse_getversion_kat);
    RUN_TEST(test_parse_rejects_bad_sof);
    RUN_TEST(test_parse_rejects_bad_checksum);
    RUN_TEST(test_parse_needs_more_bytes);
    RUN_TEST(test_parse_rejects_over_length);
    RUN_TEST(test_control_bytes);
    RUN_TEST(test_build_bounds);
    return UNITY_END();
}
