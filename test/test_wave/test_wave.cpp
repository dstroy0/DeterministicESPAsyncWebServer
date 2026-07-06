// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/wave: the IEEE 1609 WSMP + 1609.2 envelope + PSID codec.

#include "services/wave/wave.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_psid_p_encoding(void)
{
    uint8_t out[4];
    uint32_t psid = 0;
    // 1-octet: 0x20 -> 20.
    TEST_ASSERT_EQUAL_size_t(1, detws_wave_encode_psid(0x20, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0x20, out[0]);
    TEST_ASSERT_EQUAL_size_t(1, detws_wave_decode_psid(out, 1, &psid));
    TEST_ASSERT_EQUAL_UINT32(0x20, psid);

    // 2-octet: 0x8002 -> 80 | (0x8002>>8)=0xC0?? no: 0x8002 >= 0x4000 so it's 3-octet.
    TEST_ASSERT_EQUAL_size_t(3, detws_wave_encode_psid(0x8002, out, sizeof(out)));
    // C0 | (0x8002>>16=0) = C0, then 80 02.
    TEST_ASSERT_EQUAL_HEX8(0xC0, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x80, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, out[2]);
    TEST_ASSERT_EQUAL_size_t(3, detws_wave_decode_psid(out, 3, &psid));
    TEST_ASSERT_EQUAL_UINT32(0x8002, psid);

    // 2-octet range: 0x0100 -> 81 00.
    TEST_ASSERT_EQUAL_size_t(2, detws_wave_encode_psid(0x0100, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0x81, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[1]);
    detws_wave_decode_psid(out, 2, &psid);
    TEST_ASSERT_EQUAL_UINT32(0x0100, psid);
}

void test_wsmp_roundtrip(void)
{
    uint8_t payload[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t out[16];
    size_t n = detws_wsmp_build(WAVE_PSID_BSM, payload, 4, out, sizeof(out));
    TEST_ASSERT_EQUAL_HEX8(WSMP_VERSION, out[0]);
    WsmpFrame f;
    TEST_ASSERT_TRUE(detws_wsmp_parse(out, n, &f));
    TEST_ASSERT_EQUAL_UINT32(WAVE_PSID_BSM, f.psid);
    TEST_ASSERT_EQUAL_size_t(4, f.payload_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, f.payload, 4);

    // A multi-octet PSID (SPaT).
    n = detws_wsmp_build(WAVE_PSID_SPAT, payload, 2, out, sizeof(out));
    TEST_ASSERT_TRUE(detws_wsmp_parse(out, n, &f));
    TEST_ASSERT_EQUAL_UINT32(WAVE_PSID_SPAT, f.psid);
}

void test_1609dot2_wrap(void)
{
    uint8_t inner[3] = {0x01, 0x02, 0x03};
    uint8_t out[8];
    size_t n = detws_wave_1609dot2_wrap(WAVE_16092_SIGNED, inner, 3, out, sizeof(out));
    const uint8_t expect[] = {WAVE_16092_VERSION, WAVE_16092_SIGNED, 0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);
}

void test_wsmp_parse_rejects(void)
{
    WsmpFrame f;
    uint8_t bad_ver[3] = {0x00, 0x20, 0x00};
    TEST_ASSERT_FALSE(detws_wsmp_parse(bad_ver, 3, &f));     // wrong version
    uint8_t truncated[4] = {WSMP_VERSION, 0x20, 0x05, 0x11}; // len says 5, only 1 present
    TEST_ASSERT_FALSE(detws_wsmp_parse(truncated, 4, &f));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_psid_p_encoding);
    RUN_TEST(test_wsmp_roundtrip);
    RUN_TEST(test_1609dot2_wrap);
    RUN_TEST(test_wsmp_parse_rejects);
    return UNITY_END();
}
