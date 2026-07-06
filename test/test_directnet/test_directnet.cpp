// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/directnet: the DirectNET serial frame codec.

#include "services/directnet/directnet.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_lrc(void)
{
    const uint8_t b[] = {0x31, 0x32, 0x33}; // '1'^'2'^'3' = 0x30
    TEST_ASSERT_EQUAL_HEX8(0x30, detws_dnet_lrc(b, sizeof(b)));
}

void test_header_frame(void)
{
    uint8_t out[16];
    size_t n = detws_dnet_header(1, DNET_READ, 0x0040, 2, out, sizeof(out));
    // SOH(1) + slave(2) + type(1) + addr(4) + blocks(2) + ETB(1) + LRC(1) = 12.
    TEST_ASSERT_EQUAL_size_t(12, n);
    TEST_ASSERT_EQUAL_HEX8(DNET_SOH, out[0]);
    // slave 1 -> "01", type READ '0'(0x30), addr 0x0040 -> "0040", blocks 2 -> "02", then ETB.
    const uint8_t expect_body[] = {'0', '1', 0x30, '0', '0', '4', '0', '0', '2', DNET_ETB};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect_body, out + 1, sizeof(expect_body)); // 10 bytes, out[1..10]
    // LRC = XOR of the framed body (out[1..10], 10 bytes), placed at out[11].
    uint8_t lrc = detws_dnet_lrc(out + 1, 10);
    TEST_ASSERT_EQUAL_HEX8(lrc, out[11]);
}

void test_data_frame_roundtrip(void)
{
    uint8_t payload[4] = {'A', 'B', 'C', 'D'};
    uint8_t buf[16];
    size_t n = detws_dnet_data(payload, 4, buf, sizeof(buf));
    // STX + ABCD + ETX + LRC = 7.
    TEST_ASSERT_EQUAL_size_t(7, n);
    TEST_ASSERT_EQUAL_HEX8(DNET_STX, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(DNET_ETX, buf[5]);

    const uint8_t *d = nullptr;
    size_t dl = 0;
    TEST_ASSERT_TRUE(detws_dnet_data_parse(buf, n, &d, &dl));
    TEST_ASSERT_EQUAL_size_t(4, dl);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, d, 4);
}

void test_data_parse_rejects(void)
{
    uint8_t payload[2] = {0x10, 0x20};
    uint8_t buf[16];
    size_t n = detws_dnet_data(payload, 2, buf, sizeof(buf));
    const uint8_t *d;
    size_t dl;
    buf[n - 1] ^= 0xFF; // bad LRC
    TEST_ASSERT_FALSE(detws_dnet_data_parse(buf, n, &d, &dl));
    // Missing STX.
    uint8_t bad[4] = {0x00, 0x11, DNET_ETX, 0x11 ^ DNET_ETX};
    TEST_ASSERT_FALSE(detws_dnet_data_parse(bad, sizeof(bad), &d, &dl));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_lrc);
    RUN_TEST(test_header_frame);
    RUN_TEST(test_data_frame_roundtrip);
    RUN_TEST(test_data_parse_rejects);
    return UNITY_END();
}
