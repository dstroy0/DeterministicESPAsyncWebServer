// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Thread spinel / HDLC-lite framing codec (services/thread): the FCS
// (CRC-16/X-25) against its catalog check value (0x906E), an encode -> decode round trip,
// the byte-stuffing of reserved bytes, and malformed framing (bad FCS, dangling escape,
// buffer-too-small, no flag). Pure host tests.
//
// The env sizes DETWS_THREAD_MAX_DATA = 64.

#include "services/thread/thread.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_fcs_x25_check_value()
{
    // CRC-16/X-25 (poly 0x8408, init 0xFFFF, reflected, xorout 0xFFFF) of "123456789" = 0x906E.
    const uint8_t check[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    TEST_ASSERT_EQUAL_HEX16(0x906E, spinel_fcs(check, 9));
}

void test_encode_decode_round_trip()
{
    // A tiny spinel frame: header (flag|iid|tid) + command (PROP_VALUE_GET) + property.
    const uint8_t payload[3] = {0x81, 0x02, 0x02};
    uint8_t frame[32];
    uint16_t n = spinel_frame_encode(payload, 3, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);
    TEST_ASSERT_EQUAL_HEX8(HDLC_FLAG, frame[n - 1]);

    uint8_t pay[16];
    uint16_t plen = 0;
    int c = spinel_frame_decode(frame, n, pay, sizeof(pay), &plen);
    TEST_ASSERT_EQUAL_INT((int)n, c);
    TEST_ASSERT_EQUAL_UINT16(3, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 3);
}

void test_byte_stuffing_round_trip()
{
    const uint8_t payload[4] = {0x7E, 0x7D, 0x11, 0x13}; // all reserved
    uint8_t frame[32];
    uint16_t n = spinel_frame_encode(payload, 4, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);
    for (uint16_t i = 0; i + 1 < n; i++) // no raw reserved byte in the body
    {
        TEST_ASSERT_NOT_EQUAL_HEX8(0x7E, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x11, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x13, frame[i]);
    }
    uint8_t pay[16];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT((int)n, spinel_frame_decode(frame, n, pay, sizeof(pay), &plen));
    TEST_ASSERT_EQUAL_UINT16(4, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 4);
}

void test_decode_needs_more_without_flag()
{
    const uint8_t partial[3] = {0x81, 0x02, 0x02};
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(0, spinel_frame_decode(partial, sizeof(partial), pay, sizeof(pay), &plen));
}

void test_decode_rejects_bad_fcs()
{
    const uint8_t payload[3] = {0x81, 0x02, 0x02};
    uint8_t frame[32];
    uint16_t n = spinel_frame_encode(payload, 3, frame, sizeof(frame));
    frame[0] ^= 0xFF; // corrupt the payload so the FCS no longer matches
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, spinel_frame_decode(frame, n, pay, sizeof(pay), &plen));
}

void test_decode_rejects_dangling_escape()
{
    const uint8_t frame[2] = {HDLC_ESCAPE, HDLC_FLAG}; // escape with nothing before the flag
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, spinel_frame_decode(frame, sizeof(frame), pay, sizeof(pay), &plen));
}

void test_decode_rejects_small_payload_buffer()
{
    const uint8_t payload[6] = {1, 2, 3, 4, 5, 6};
    uint8_t frame[32];
    uint16_t n = spinel_frame_encode(payload, 6, frame, sizeof(frame));
    uint8_t tiny[3];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, spinel_frame_decode(frame, n, tiny, sizeof(tiny), &plen));
}

void test_encode_bounds()
{
    uint8_t data[80] = {0};
    uint8_t out[256];
    TEST_ASSERT_EQUAL_UINT16(0, spinel_frame_encode(data, 65, out, sizeof(out))); // > MAX_DATA 64
    uint8_t small[3];
    TEST_ASSERT_EQUAL_UINT16(0, spinel_frame_encode(data, 8, small, sizeof(small))); // will not fit
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fcs_x25_check_value);
    RUN_TEST(test_encode_decode_round_trip);
    RUN_TEST(test_byte_stuffing_round_trip);
    RUN_TEST(test_decode_needs_more_without_flag);
    RUN_TEST(test_decode_rejects_bad_fcs);
    RUN_TEST(test_decode_rejects_dangling_escape);
    RUN_TEST(test_decode_rejects_small_payload_buffer);
    RUN_TEST(test_encode_bounds);
    return UNITY_END();
}
