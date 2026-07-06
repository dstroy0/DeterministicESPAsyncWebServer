// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/j2735: the ASN.1 UPER primitive codec + the BSMcore block.

#include "services/j2735/j2735.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_cint_bits(void)
{
    TEST_ASSERT_EQUAL_UINT(0, uper_cint_bits(5, 5));   // single value -> 0 bits
    TEST_ASSERT_EQUAL_UINT(1, uper_cint_bits(0, 1));   // 2 values -> 1 bit
    TEST_ASSERT_EQUAL_UINT(7, uper_cint_bits(0, 127)); // 128 values -> 7 bits
    TEST_ASSERT_EQUAL_UINT(8, uper_cint_bits(0, 128)); // 129 values -> 8 bits
    TEST_ASSERT_EQUAL_UINT(16, uper_cint_bits(0, 65535));
}

void test_bit_writer_pattern(void)
{
    // Write 0b101 (3 bits) then 0b11 (2 bits): stream 10111 000 -> 0xB8.
    uint8_t buf[4];
    UperWriter w;
    uper_writer_init(&w, buf, sizeof(buf));
    uper_put_bits(&w, 0b101, 3);
    uper_put_bits(&w, 0b11, 2);
    size_t n = uper_writer_finish(&w);
    TEST_ASSERT_EQUAL_size_t(1, n);
    TEST_ASSERT_EQUAL_HEX8(0xB8, buf[0]);
}

void test_cint_roundtrip(void)
{
    uint8_t buf[8];
    UperWriter w;
    uper_writer_init(&w, buf, sizeof(buf));
    uper_put_cint(&w, 100, 0, 127); // 7 bits: 1100100
    uper_put_cint(&w, -5, -10, 10); // offset 5 in 5 bits: 00101
    size_t n = uper_writer_finish(&w);

    UperReader r;
    uper_reader_init(&r, buf, n * 8);
    TEST_ASSERT_EQUAL_INT64(100, uper_get_cint(&r, 0, 127));
    TEST_ASSERT_EQUAL_INT64(-5, uper_get_cint(&r, -10, 10));
    TEST_ASSERT_TRUE(r.ok);
}

void test_bsm_core_roundtrip(void)
{
    J2735BsmCore c;
    c.msg_count = 12;
    c.id = 0xDEADBEEF;
    c.sec_mark = 34000;
    c.lat = 407127370;  // ~40.7127370 N (NYC)
    c.lon = -740059730; // ~-74.0059730 W
    c.elev = 100;
    c.speed = 500;    // 10 m/s
    c.heading = 7200; // 90 deg
    uint8_t buf[64];
    size_t n = detws_j2735_bsm_core_encode(&c, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);

    J2735BsmCore d;
    memset(&d, 0, sizeof(d));
    TEST_ASSERT_TRUE(detws_j2735_bsm_core_decode(buf, n, &d));
    TEST_ASSERT_EQUAL_UINT8(12, d.msg_count);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, d.id);
    TEST_ASSERT_EQUAL_UINT16(34000, d.sec_mark);
    TEST_ASSERT_EQUAL_INT32(407127370, d.lat);
    TEST_ASSERT_EQUAL_INT32(-740059730, d.lon);
    TEST_ASSERT_EQUAL_INT32(100, d.elev);
    TEST_ASSERT_EQUAL_UINT16(500, d.speed);
    TEST_ASSERT_EQUAL_UINT16(7200, d.heading);
}

void test_bsm_core_bit_length(void)
{
    // msgCnt 7 + id 32 + secMark 16 + lat 31 + long 32 + elev 16 + speed 13 + heading 15 = 162 bits
    // -> ceil(162/8) = 21 octets.
    J2735BsmCore c;
    memset(&c, 0, sizeof(c));
    uint8_t buf[64];
    size_t n = detws_j2735_bsm_core_encode(&c, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(21, n);
}

void test_spat_roundtrip(void)
{
    J2735MovementState st[3];
    st[0] = {1, (uint8_t)J2735_PHASE_PROTECTED_MOVEMENT_ALLOWED, 100, 250};
    st[1] = {2, (uint8_t)J2735_PHASE_STOP_AND_REMAIN, 0, 36000};
    st[2] = {17, (uint8_t)J2735_PHASE_PERMISSIVE_CLEARANCE, 300, 320};
    uint8_t buf[64];
    size_t n = detws_j2735_spat_encode(st, 3, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);

    J2735MovementState out[8];
    size_t count = 0;
    TEST_ASSERT_TRUE(detws_j2735_spat_decode(buf, n, out, 8, &count));
    TEST_ASSERT_EQUAL_size_t(3, count);
    TEST_ASSERT_EQUAL_UINT8(1, out[0].signal_group);
    TEST_ASSERT_EQUAL_UINT8(J2735_PHASE_PROTECTED_MOVEMENT_ALLOWED, out[0].phase);
    TEST_ASSERT_EQUAL_UINT16(100, out[0].min_end_time);
    TEST_ASSERT_EQUAL_UINT16(250, out[0].max_end_time);
    TEST_ASSERT_EQUAL_UINT8(17, out[2].signal_group);
    TEST_ASSERT_EQUAL_UINT16(36000, out[1].max_end_time);
}

void test_spat_decode_too_many(void)
{
    J2735MovementState st[2] = {{1, 6, 0, 0}, {2, 3, 0, 0}};
    uint8_t buf[32];
    size_t n = detws_j2735_spat_encode(st, 2, buf, sizeof(buf));
    J2735MovementState out[1];
    size_t count = 0;
    // Only room for 1 but 2 encoded -> false.
    TEST_ASSERT_FALSE(detws_j2735_spat_decode(buf, n, out, 1, &count));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cint_bits);
    RUN_TEST(test_bit_writer_pattern);
    RUN_TEST(test_cint_roundtrip);
    RUN_TEST(test_bsm_core_roundtrip);
    RUN_TEST(test_bsm_core_bit_length);
    RUN_TEST(test_spat_roundtrip);
    RUN_TEST(test_spat_decode_too_many);
    return UNITY_END();
}
