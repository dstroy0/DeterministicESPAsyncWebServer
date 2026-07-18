// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Zigbee EZSP / ASH framing codec (services/zigbee): the CRC-16/CCITT
// and the encoded RST frame against their documented values (C0 38 BC 7E), an encode ->
// decode round trip, the byte-stuffing of reserved bytes, and malformed framing (bad CRC,
// dangling escape, buffer-too-small, no flag yet). Pure host tests.
//
// The env sizes DWS_ZIGBEE_MAX_DATA = 32.

#include "services/zigbee/zigbee.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_crc16_rst_kat()
{
    // CRC-16/CCITT (poly 0x1021, init 0xFFFF) of {0xC0} is 0x38BC (the ASH RST frame CRC).
    const uint8_t rst[1] = {Ash::ASH_RST};
    TEST_ASSERT_EQUAL_HEX16(0x38BC, dws_ash_crc16(rst, 1));
}

void test_encode_rst_frame_kat()
{
    // The documented ASH RST frame is C0 38 BC 7E (control, CRC hi/lo, flag).
    const uint8_t expect[4] = {0xC0, 0x38, 0xBC, 0x7E};
    uint8_t out[16];
    uint16_t n = dws_ash_frame_encode(Ash::ASH_RST, nullptr, 0, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(4, n);
    TEST_ASSERT_EQUAL_MEMORY(expect, out, 4);
}

void test_encode_decode_round_trip()
{
    const uint8_t payload[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    uint8_t frame[32];
    uint16_t n = dws_ash_frame_encode(0x25, payload, 5, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);

    uint8_t control = 0, pay[16];
    uint16_t plen = 0;
    int c = dws_ash_frame_decode(frame, n, &control, pay, sizeof(pay), &plen);
    TEST_ASSERT_EQUAL_INT((int)n, c);
    TEST_ASSERT_EQUAL_HEX8(0x25, control);
    TEST_ASSERT_EQUAL_UINT16(5, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 5);
}

void test_byte_stuffing_round_trip()
{
    // A payload full of reserved bytes must survive: none may appear raw in the body.
    const uint8_t payload[6] = {0x7E, 0x7D, 0x11, 0x13, 0x18, 0x1A};
    uint8_t frame[64];
    uint16_t n = dws_ash_frame_encode(0x00, payload, 6, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);
    // Only the terminating byte may be the flag; no raw reserved control byte in the body.
    for (uint16_t i = 0; i + 1 < n; i++)
    {
        TEST_ASSERT_NOT_EQUAL_HEX8(0x7E, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x11, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x13, frame[i]);
    }
    TEST_ASSERT_EQUAL_HEX8(0x7E, frame[n - 1]);

    uint8_t control = 0xFF, pay[16];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT((int)n, dws_ash_frame_decode(frame, n, &control, pay, sizeof(pay), &plen));
    TEST_ASSERT_EQUAL_HEX8(0x00, control);
    TEST_ASSERT_EQUAL_UINT16(6, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 6);
}

void test_decode_needs_more_without_flag()
{
    const uint8_t partial[3] = {0xC0, 0x38, 0xBC}; // no 0x7E yet
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_ash_frame_decode(partial, sizeof(partial), nullptr, pay, sizeof(pay), &plen));
}

void test_decode_rejects_bad_crc()
{
    uint8_t frame[4] = {0xC0, 0x38, 0xBC, 0x7E};
    frame[1] ^= 0xFF; // corrupt the CRC
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_ash_frame_decode(frame, sizeof(frame), nullptr, pay, sizeof(pay), &plen));
}

void test_decode_rejects_dangling_escape()
{
    const uint8_t frame[3] = {0xC0, Ash::ASH_ESCAPE, Ash::ASH_FLAG}; // escape with nothing after it before the flag
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_ash_frame_decode(frame, sizeof(frame), nullptr, pay, sizeof(pay), &plen));
}

void test_decode_rejects_small_payload_buffer()
{
    const uint8_t payload[5] = {1, 2, 3, 4, 5};
    uint8_t frame[32];
    uint16_t n = dws_ash_frame_encode(0x10, payload, 5, frame, sizeof(frame));
    uint8_t tiny[2]; // payload is 5, cap 2
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_ash_frame_decode(frame, n, nullptr, tiny, sizeof(tiny), &plen));
}

void test_encode_bounds()
{
    uint8_t data[40] = {0};
    uint8_t out[128];
    TEST_ASSERT_EQUAL_UINT16(0, dws_ash_frame_encode(0x00, data, 33, out, sizeof(out))); // > MAX_DATA 32
    uint8_t small[3];
    TEST_ASSERT_EQUAL_UINT16(0, dws_ash_frame_encode(0x00, data, 8, small, sizeof(small))); // will not fit
}

void test_encode_decode_guards()
{
    uint8_t out[64];
    uint8_t payload[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_UINT16(0, dws_ash_frame_encode(0x00, payload, sizeof(payload), out, 2)); // overflow
    uint8_t control = 0;
    uint8_t pay[64];
    uint16_t pay_len = 0;
    uint8_t short_raw[3] = {0x7E, 0x00, 0x7E}; // destuffs to < control+CRC
    TEST_ASSERT_EQUAL_INT(-1, dws_ash_frame_decode(short_raw, sizeof(short_raw), &control, pay, sizeof(pay), &pay_len));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_crc16_rst_kat);
    RUN_TEST(test_encode_rst_frame_kat);
    RUN_TEST(test_encode_decode_round_trip);
    RUN_TEST(test_byte_stuffing_round_trip);
    RUN_TEST(test_decode_needs_more_without_flag);
    RUN_TEST(test_decode_rejects_bad_crc);
    RUN_TEST(test_decode_rejects_dangling_escape);
    RUN_TEST(test_decode_rejects_small_payload_buffer);
    RUN_TEST(test_encode_bounds);
    RUN_TEST(test_encode_decode_guards);
    return UNITY_END();
}
