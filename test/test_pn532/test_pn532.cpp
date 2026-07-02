// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the PN532 NFC frame codec (services/pn532): the normal-information-frame
// build/parse against the documented GetFirmwareVersion command + response frames (LCS +
// DCS checksums), a round trip, malformed framing (bad preamble / start / LCS / DCS),
// incomplete frames, over-length rejection, and the ACK frame. Pure host tests.
//
// The env sizes DETWS_PN532_MAX_DATA = 8.

#include "services/pn532/pn532.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_build_getfirmwareversion_kat()
{
    // Host -> PN532 GetFirmwareVersion (command 0x02): the documented frame is
    // 00 00 FF 02 FE D4 02 2A 00.
    const uint8_t expect[9] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
    const uint8_t cmd[1] = {0x02};
    uint8_t out[16];
    uint16_t n = pn532_build_frame(PN532_TFI_HOST, cmd, 1, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(9, n);
    TEST_ASSERT_EQUAL_MEMORY(expect, out, 9);
}

void test_parse_getfirmwareversion_response_kat()
{
    // PN532 -> host response: 00 00 FF 06 FA D5 03 32 01 06 07 E8 00.
    const uint8_t frame[13] = {0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03, 0x32, 0x01, 0x06, 0x07, 0xE8, 0x00};
    uint8_t tfi = 0;
    const uint8_t *pd = nullptr;
    uint8_t pdlen = 0;
    int c = pn532_parse_frame(frame, sizeof(frame), &tfi, &pd, &pdlen);
    TEST_ASSERT_EQUAL_INT(13, c);
    TEST_ASSERT_EQUAL_HEX8(PN532_TFI_PN532, tfi);
    TEST_ASSERT_EQUAL_UINT8(5, pdlen);
    const uint8_t expect_pd[5] = {0x03, 0x32, 0x01, 0x06, 0x07};
    TEST_ASSERT_EQUAL_MEMORY(expect_pd, pd, 5);
}

void test_build_then_parse_round_trip()
{
    const uint8_t data[4] = {0x4A, 0x01, 0x00, 0xAB}; // e.g. InListPassiveTarget-ish
    uint8_t buf[24];
    uint16_t n = pn532_build_frame(PN532_TFI_HOST, data, 4, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT16(12, n); // 8 + 4
    uint8_t tfi = 0;
    const uint8_t *pd = nullptr;
    uint8_t pdlen = 0;
    TEST_ASSERT_EQUAL_INT(12, pn532_parse_frame(buf, n, &tfi, &pd, &pdlen));
    TEST_ASSERT_EQUAL_HEX8(PN532_TFI_HOST, tfi);
    TEST_ASSERT_EQUAL_UINT8(4, pdlen);
    TEST_ASSERT_EQUAL_MEMORY(data, pd, 4);
}

static uint16_t sample(uint8_t *buf, uint16_t cap)
{
    const uint8_t d[2] = {0x02, 0x00};
    return pn532_build_frame(PN532_TFI_HOST, d, 2, buf, cap);
}

void test_parse_rejects_bad_preamble_and_start()
{
    uint8_t buf[16];
    uint16_t n = sample(buf, sizeof(buf));
    buf[0] = 0x11;
    TEST_ASSERT_EQUAL_INT(-1, pn532_parse_frame(buf, n, nullptr, nullptr, nullptr));
    n = sample(buf, sizeof(buf));
    buf[2] = 0x00; // start should be 00 FF
    TEST_ASSERT_EQUAL_INT(-1, pn532_parse_frame(buf, n, nullptr, nullptr, nullptr));
}

void test_parse_rejects_bad_lcs()
{
    uint8_t buf[16];
    uint16_t n = sample(buf, sizeof(buf));
    buf[4] ^= 0xFF; // corrupt LCS
    TEST_ASSERT_EQUAL_INT(-1, pn532_parse_frame(buf, n, nullptr, nullptr, nullptr));
}

void test_parse_rejects_bad_dcs()
{
    uint8_t buf[16];
    uint16_t n = sample(buf, sizeof(buf));
    buf[n - 2] ^= 0xFF; // corrupt DCS (byte before the postamble)
    TEST_ASSERT_EQUAL_INT(-1, pn532_parse_frame(buf, n, nullptr, nullptr, nullptr));
}

void test_parse_needs_more_bytes()
{
    uint8_t buf[16];
    uint16_t n = sample(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(0, pn532_parse_frame(buf, 4, nullptr, nullptr, nullptr));     // header incomplete
    TEST_ASSERT_EQUAL_INT(0, pn532_parse_frame(buf, n - 1, nullptr, nullptr, nullptr)); // body incomplete
}

void test_parse_rejects_over_length()
{
    // frame_len 20 (> DETWS_PN532_MAX_DATA + 1 = 9) is rejected early.
    uint8_t buf[8] = {0x00, 0x00, 0xFF, 0x14, 0xEC, 0xD5, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(-1, pn532_parse_frame(buf, sizeof(buf), nullptr, nullptr, nullptr));
}

void test_ack_frame()
{
    uint8_t ack[6];
    TEST_ASSERT_EQUAL_UINT16(6, pn532_build_ack(ack, sizeof(ack)));
    const uint8_t expect[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    TEST_ASSERT_EQUAL_MEMORY(expect, ack, 6);
    TEST_ASSERT_TRUE(pn532_is_ack(ack, 6));

    uint8_t buf[16];
    uint16_t n = sample(buf, sizeof(buf));
    TEST_ASSERT_FALSE(pn532_is_ack(buf, n)); // a normal frame is not an ACK
    uint8_t small[3];
    TEST_ASSERT_EQUAL_UINT16(0, pn532_build_ack(small, sizeof(small))); // too small
}

void test_build_bounds()
{
    uint8_t data[8] = {0};
    uint8_t small[8];
    TEST_ASSERT_EQUAL_UINT16(0, pn532_build_frame(PN532_TFI_HOST, data, 4, small, sizeof(small))); // 12 > 8
    uint8_t big[64];
    TEST_ASSERT_EQUAL_UINT16(0, pn532_build_frame(PN532_TFI_HOST, big, 9, big, sizeof(big))); // 9 > MAX_DATA 8
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_getfirmwareversion_kat);
    RUN_TEST(test_parse_getfirmwareversion_response_kat);
    RUN_TEST(test_build_then_parse_round_trip);
    RUN_TEST(test_parse_rejects_bad_preamble_and_start);
    RUN_TEST(test_parse_rejects_bad_lcs);
    RUN_TEST(test_parse_rejects_bad_dcs);
    RUN_TEST(test_parse_needs_more_bytes);
    RUN_TEST(test_parse_rejects_over_length);
    RUN_TEST(test_ack_frame);
    RUN_TEST(test_build_bounds);
    return UNITY_END();
}
