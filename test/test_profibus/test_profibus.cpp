// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/profibus: the PROFIBUS-DP FDL telegram codec.

#include "services/profibus/profibus.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_fcs(void)
{
    const uint8_t b[] = {0x03, 0x02, 0x49};
    TEST_ASSERT_EQUAL_HEX8(0x4E, detws_pb_fcs(b, sizeof(b))); // 0x03+0x02+0x49
}

void test_sd1(void)
{
    uint8_t out[8];
    size_t n = detws_pb_build_sd1(0x03, 0x02, PB_FC_REQUEST_FDL_STATUS, out, sizeof(out));
    // SD1 DA SA FC FCS ED : 10 03 02 49 4E 16
    const uint8_t expect[] = {PB_SD1, 0x03, 0x02, 0x49, 0x4E, PB_ED};
    TEST_ASSERT_EQUAL_size_t(6, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);

    PbTelegram t;
    TEST_ASSERT_TRUE(detws_pb_parse(out, n, &t));
    TEST_ASSERT_EQUAL_HEX8(PB_SD1, t.sd);
    TEST_ASSERT_EQUAL_HEX8(0x03, t.da);
    TEST_ASSERT_EQUAL_HEX8(0x02, t.sa);
    TEST_ASSERT_EQUAL_HEX8(0x49, t.fc);
    TEST_ASSERT_EQUAL_size_t(0, t.data_len);
}

void test_sd2_roundtrip(void)
{
    uint8_t data[3] = {0xAA, 0xBB, 0xCC};
    uint8_t out[16];
    size_t n = detws_pb_build_sd2(0x05, 0x02, PB_FC_SRD_HIGH, data, 3, out, sizeof(out));
    // le = 3 + 3 = 6; total = 4 + 6 + 2 = 12.
    TEST_ASSERT_EQUAL_size_t(12, n);
    TEST_ASSERT_EQUAL_HEX8(PB_SD2, out[0]);
    TEST_ASSERT_EQUAL_HEX8(6, out[1]);
    TEST_ASSERT_EQUAL_HEX8(6, out[2]); // LEr
    TEST_ASSERT_EQUAL_HEX8(PB_SD2, out[3]);

    PbTelegram t;
    TEST_ASSERT_TRUE(detws_pb_parse(out, n, &t));
    TEST_ASSERT_EQUAL_HEX8(PB_SD2, t.sd);
    TEST_ASSERT_EQUAL_HEX8(0x05, t.da);
    TEST_ASSERT_EQUAL_HEX8(PB_FC_SRD_HIGH, t.fc);
    TEST_ASSERT_EQUAL_size_t(3, t.data_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, t.data, 3);
}

void test_parse_rejects(void)
{
    uint8_t data[2] = {0x11, 0x22};
    uint8_t out[16];
    size_t n = detws_pb_build_sd2(0x05, 0x02, PB_FC_SRD_LOW, data, 2, out, sizeof(out));
    PbTelegram t;
    out[n - 2] ^= 0xFF; // corrupt FCS
    TEST_ASSERT_FALSE(detws_pb_parse(out, n, &t));
    // Mismatched LE/LEr.
    n = detws_pb_build_sd2(0x05, 0x02, PB_FC_SRD_LOW, data, 2, out, sizeof(out));
    out[2] = 0x99;
    TEST_ASSERT_FALSE(detws_pb_parse(out, n, &t));
    // data_len > 246 rejected at build.
    TEST_ASSERT_EQUAL_size_t(0, detws_pb_build_sd2(0x05, 0x02, PB_FC_SRD_LOW, data, 300, out, sizeof(out)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fcs);
    RUN_TEST(test_sd1);
    RUN_TEST(test_sd2_roundtrip);
    RUN_TEST(test_parse_rejects);
    return UNITY_END();
}
