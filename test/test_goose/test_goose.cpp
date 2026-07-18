// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/goose: the IEC 61850 GOOSE BER PDU + Ethernet frame codec.

#include "services/goose/goose.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

// Find needle in hay; returns index or -1.
static int find(const uint8_t *hay, size_t hlen, const uint8_t *needle, size_t nlen)
{
    if (nlen > hlen)
        return -1;
    for (size_t i = 0; i + nlen <= hlen; i++)
        if (memcmp(hay + i, needle, nlen) == 0)
            return (int)i;
    return -1;
}

static DWSGoose base(void)
{
    DWSGoose g = {};
    g.gocb_ref = "X";
    g.time_allowed_to_live = 1;
    g.dat_set = "D";
    g.go_id = "G";
    g.t = nullptr; // -> 8 zero octets
    g.st_num = 1;
    g.sq_num = 2;
    g.simulation = false;
    g.conf_rev = 1;
    g.nds_com = false;
    g.num_entries = 0;
    g.all_data = nullptr;
    g.all_data_len = 0;
    return g;
}

void test_pdu_structure(void)
{
    DWSGoose g = base();
    uint8_t out[128];
    size_t n = dws_goose_pdu(&g, out, sizeof(out));
    // Content is 42 octets (see goose.cpp field sizes); PDU = 61 2A <42> = 44.
    TEST_ASSERT_EQUAL_size_t(44, n);
    TEST_ASSERT_EQUAL_HEX8(0x61, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x2A, out[1]); // 42
    // First field: gocbRef "X" -> 80 01 58.
    const uint8_t gocb[] = {0x80, 0x01, 'X'};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(gocb, out + 2, 3);
    // sqNum = 2 -> 86 01 02 present.
    const uint8_t sq[] = {0x86, 0x01, 0x02};
    TEST_ASSERT_TRUE(find(out, n, sq, 3) >= 0);
    // simulation false -> 87 01 00; allData empty -> AB 00.
    const uint8_t sim[] = {0x87, 0x01, 0x00};
    TEST_ASSERT_TRUE(find(out, n, sim, 3) >= 0);
    const uint8_t ad[] = {0xAB, 0x00};
    TEST_ASSERT_TRUE(find(out, n, ad, 2) >= 0);
}

void test_integer_leading_zero(void)
{
    DWSGoose g = base();
    g.st_num = 200; // 0xC8, MSB set -> BER positive INTEGER needs a leading 0x00: 85 02 00 C8.
    uint8_t out[128];
    size_t n = dws_goose_pdu(&g, out, sizeof(out));
    const uint8_t st[] = {0x85, 0x02, 0x00, 0xC8};
    TEST_ASSERT_TRUE(find(out, n, st, 4) >= 0);
    // A value < 0x80 stays one octet.
    g.st_num = 0x7F;
    n = dws_goose_pdu(&g, out, sizeof(out));
    const uint8_t st2[] = {0x85, 0x01, 0x7F};
    TEST_ASSERT_TRUE(find(out, n, st2, 3) >= 0);
}

void test_frame(void)
{
    DWSGoose g = base();
    uint8_t dst[6] = {0x01, 0x0C, 0xCD, 0x01, 0x00, 0x01};
    uint8_t src[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t out[128];
    size_t n = dws_goose_frame(dst, src, 0x1234, &g, out, sizeof(out));
    TEST_ASSERT_TRUE(n > 22);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(dst, out, 6);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(src, out + 6, 6);
    TEST_ASSERT_EQUAL_HEX8(0x88, out[12]); // ethertype GOOSE
    TEST_ASSERT_EQUAL_HEX8(0xB8, out[13]);
    TEST_ASSERT_EQUAL_HEX8(0x12, out[14]); // APPID
    TEST_ASSERT_EQUAL_HEX8(0x34, out[15]);
    uint16_t goose_len = (uint16_t)((out[16] << 8) | out[17]);
    TEST_ASSERT_EQUAL_UINT16(8 + 44, goose_len); // header(8) + PDU(44)
    TEST_ASSERT_EQUAL_HEX8(0x61, out[22]);       // APDU starts here
}

// The PDU/frame guards, the tlv-overflow fail path, and the multi-octet BER length
// encoding (long-form), driven by a >=128-octet allData.
void test_goose_error_and_longform(void)
{
    DWSGoose g = base();
    uint8_t out[512];

    TEST_ASSERT_EQUAL_size_t(0, dws_goose_pdu(nullptr, out, sizeof(out))); // null g
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_pdu(&g, nullptr, sizeof(out)));  // null out
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_pdu(&g, out, 3));                // cap < reserved header
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_pdu(&g, out, 10)); // fits the header but overflows on a field -> !ok

    // A >=128-octet allData forces multi-octet BER lengths (len_octets + write_len long-form).
    static uint8_t big[200];
    memset(big, 0x5A, sizeof(big));
    g.all_data = big;
    g.all_data_len = sizeof(big);
    size_t n = dws_goose_pdu(&g, out, sizeof(out));
    TEST_ASSERT_TRUE(n > 200);
    TEST_ASSERT_EQUAL_HEX8(0x61, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x81, out[1]); // content length >= 128 -> long-form (one following octet)

    uint8_t dst[6] = {0};
    uint8_t src[6] = {0};
    DWSGoose g2 = base();
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_frame(nullptr, src, 0x1234, &g2, out, sizeof(out))); // null dst
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_frame(dst, src, 0x1234, &g2, out, 20));              // cap < 22
    TEST_ASSERT_EQUAL_size_t(0, dws_goose_frame(dst, src, 0x1234, &g2, out, 30)); // >=22 but the PDU cannot fit
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pdu_structure);
    RUN_TEST(test_integer_leading_zero);
    RUN_TEST(test_frame);
    RUN_TEST(test_goose_error_and_longform);
    return UNITY_END();
}
