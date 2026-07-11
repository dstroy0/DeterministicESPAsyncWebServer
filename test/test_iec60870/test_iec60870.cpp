// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the IEC 60870-5-101/-104 codec (services/iec60870): the -104 APCI (I/S/U
// formats), the shared ASDU header + 3-octet IOA, and the -101 FT1.2 link frames (fixed +
// variable, sum checksum). Frame layout checked against IEC 60870-5-101/-104. Pure host tests.

#include "services/iec60870/iec60870.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// -104 I-format: numbered transfer carrying an ASDU.
void test_104_i_format_roundtrip()
{
    const uint8_t asdu[6] = {0x09, 0x01, 0x03, 0x00, 0x0A, 0x00}; // a tiny ASDU header
    uint8_t buf[32];
    size_t n = iec104_build_i(buf, sizeof(buf), 100, 50, asdu, 6);
    TEST_ASSERT_EQUAL_size_t(6 + 6, n);
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(4 + 6, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2] & 0x01); // I-format: bit0 = 0

    Iec104Apci a;
    size_t c;
    TEST_ASSERT_TRUE(iec104_parse(buf, n, &a, &c));
    TEST_ASSERT_EQUAL_INT(Iec104Format::IEC104_I, a.format);
    TEST_ASSERT_EQUAL_UINT16(100, a.ns);
    TEST_ASSERT_EQUAL_UINT16(50, a.nr);
    TEST_ASSERT_EQUAL_size_t(6, a.asdu_len);
    TEST_ASSERT_EQUAL_MEMORY(asdu, a.asdu, 6);
    TEST_ASSERT_EQUAL_size_t(n, c);
}

void test_104_s_format()
{
    uint8_t buf[8];
    size_t n = iec104_build_s(buf, sizeof(buf), 1234);
    TEST_ASSERT_EQUAL_size_t(6, n);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[2]);
    Iec104Apci a;
    size_t c;
    TEST_ASSERT_TRUE(iec104_parse(buf, n, &a, &c));
    TEST_ASSERT_EQUAL_INT(Iec104Format::IEC104_S, a.format);
    TEST_ASSERT_EQUAL_UINT16(1234, a.nr);
}

void test_104_u_format()
{
    uint8_t buf[8];
    size_t n = iec104_build_u(buf, sizeof(buf), IEC104_STARTDT_ACT);
    TEST_ASSERT_EQUAL_size_t(6, n);
    Iec104Apci a;
    size_t c;
    TEST_ASSERT_TRUE(iec104_parse(buf, n, &a, &c));
    TEST_ASSERT_EQUAL_INT(Iec104Format::IEC104_U, a.format);
    TEST_ASSERT_EQUAL_HEX8(IEC104_STARTDT_ACT, a.u_cmd);
}

// The 15-bit sequence numbers survive values above one octet.
void test_104_sequence_numbers_15bit()
{
    uint8_t buf[8];
    iec104_build_i(buf, sizeof(buf), 0x7FFF, 0x4001, nullptr, 0);
    Iec104Apci a;
    size_t c;
    TEST_ASSERT_TRUE(iec104_parse(buf, 6, &a, &c));
    TEST_ASSERT_EQUAL_UINT16(0x7FFF, a.ns);
    TEST_ASSERT_EQUAL_UINT16(0x4001, a.nr);
}

void test_asdu_header_roundtrip()
{
    IecAsduHeader h;
    h.type_id = IEC_TYPE_M_ME_NC_1;
    h.sq = false;
    h.count = 3;
    h.test = false;
    h.negative = false;
    h.cot = IEC_COT_SPONTANEOUS;
    h.orig_addr = 0;
    h.common_addr = 0x000A;
    uint8_t buf[8];
    TEST_ASSERT_EQUAL_size_t(6, iec_asdu_build_header(buf, sizeof(buf), &h));
    TEST_ASSERT_EQUAL_HEX8(13, buf[0]);   // M_ME_NC_1
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[1]); // SQ=0, count=3
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[2]); // COT spontaneous

    IecAsduHeader g;
    size_t c;
    TEST_ASSERT_TRUE(iec_asdu_parse_header(buf, 6, &g, &c));
    TEST_ASSERT_EQUAL_UINT8(IEC_TYPE_M_ME_NC_1, g.type_id);
    TEST_ASSERT_EQUAL_UINT8(3, g.count);
    TEST_ASSERT_FALSE(g.sq);
    TEST_ASSERT_EQUAL_UINT8(IEC_COT_SPONTANEOUS, g.cot);
    TEST_ASSERT_EQUAL_UINT16(0x000A, g.common_addr);
}

void test_ioa_roundtrip()
{
    uint8_t buf[4] = {0};
    TEST_ASSERT_EQUAL_size_t(3, iec_put_ioa(buf, sizeof(buf), 0x123456));
    TEST_ASSERT_EQUAL_HEX8(0x56, buf[0]); // little-endian
    TEST_ASSERT_EQUAL_HEX8(0x34, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x12, buf[2]);
    TEST_ASSERT_EQUAL_HEX32(0x123456u, iec_get_ioa(buf));
}

void test_101_fixed_frame()
{
    uint8_t buf[8];
    size_t n = iec101_build_fixed(buf, sizeof(buf), IEC_FC_REQUEST_CLASS2, 0x01);
    TEST_ASSERT_EQUAL_size_t(5, n);
    const uint8_t expect[] = {0x10, IEC_FC_REQUEST_CLASS2, 0x01, (uint8_t)(IEC_FC_REQUEST_CLASS2 + 1), 0x16};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, 5);

    Iec101Frame f;
    size_t c;
    TEST_ASSERT_TRUE(iec101_parse(buf, n, &f, &c));
    TEST_ASSERT_TRUE(f.fixed);
    TEST_ASSERT_EQUAL_HEX8(IEC_FC_REQUEST_CLASS2, f.control);
    TEST_ASSERT_EQUAL_HEX8(0x01, f.addr);
}

void test_101_variable_frame_roundtrip()
{
    const uint8_t asdu[6] = {0x01, 0x01, 0x03, 0x00, 0x0A, 0x00};
    uint8_t buf[32];
    size_t n = iec101_build_variable(buf, sizeof(buf), IEC_FC_USER_DATA_CONFIRM, 0x01, asdu, 6);
    TEST_ASSERT_EQUAL_size_t(6 + (2 + 6), n);
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(8, buf[1]); // L = 2 + 6
    TEST_ASSERT_EQUAL_HEX8(0x16, buf[n - 1]);

    Iec101Frame f;
    size_t c;
    TEST_ASSERT_TRUE(iec101_parse(buf, n, &f, &c));
    TEST_ASSERT_FALSE(f.fixed);
    TEST_ASSERT_EQUAL_HEX8(IEC_FC_USER_DATA_CONFIRM, f.control);
    TEST_ASSERT_EQUAL_HEX8(0x01, f.addr);
    TEST_ASSERT_EQUAL_UINT8(6, f.asdu_len);
    TEST_ASSERT_EQUAL_MEMORY(asdu, f.asdu, 6);

    // a corrupted checksum is rejected.
    buf[n - 2] ^= 0xFF;
    TEST_ASSERT_FALSE(iec101_parse(buf, n, &f, &c));
}

// -104 builders reject null buffers, oversize ASDUs, and buffers too small.
void test_104_build_guards()
{
    uint8_t buf[32];
    const uint8_t asdu[6] = {0};
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_i(nullptr, sizeof(buf), 0, 0, asdu, 6)); // null buf
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_i(buf, sizeof(buf), 0, 0, nullptr, 6));  // len but null asdu
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_i(buf, 512, 0, 0, asdu, 250));           // asdu_len > 249
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_i(buf, 8, 0, 0, asdu, 6));               // cap < 6 + asdu_len
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_s(nullptr, sizeof(buf), 0));             // null buf
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_s(buf, 4, 0));                           // cap < APCI len
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_u(nullptr, sizeof(buf), 0));             // null buf
    TEST_ASSERT_EQUAL_size_t(0, iec104_build_u(buf, 4, 0));                           // cap < APCI len
}

// -104 parse rejects a wrong start octet, an APDU length below 4, and a truncated frame.
void test_104_parse_rejects()
{
    Iec104Apci a;
    size_t c;
    uint8_t bad_start[6] = {0x00, 0x04, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(iec104_parse(bad_start, sizeof(bad_start), &a, &c)); // buf[0] != 0x68
    TEST_ASSERT_FALSE(iec104_parse(nullptr, 6, &a, &c));                   // null buf
    uint8_t short_l[6] = {IEC_START_104, 0x03, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(iec104_parse(short_l, sizeof(short_l), &a, &c)); // L < 4
    uint8_t trunc[2] = {IEC_START_104, 0x0A};
    TEST_ASSERT_FALSE(iec104_parse(trunc, sizeof(trunc), &a, &c)); // len < 2 + L
}

// ASDU header / IOA helpers reject null buffers and buffers too small.
void test_asdu_ioa_guards()
{
    uint8_t buf[8];
    IecAsduHeader h = {};
    IecAsduHeader g;
    size_t c;
    TEST_ASSERT_EQUAL_size_t(0, iec_asdu_build_header(buf, 4, &h)); // cap < 6
    TEST_ASSERT_EQUAL_size_t(0, iec_asdu_build_header(nullptr, 8, &h));
    TEST_ASSERT_FALSE(iec_asdu_parse_header(buf, 5, &g, &c));   // len < 6
    TEST_ASSERT_EQUAL_size_t(0, iec_put_ioa(buf, 2, 0x123456)); // cap < 3
    TEST_ASSERT_EQUAL_size_t(0, iec_put_ioa(nullptr, 8, 0));
}

// -101 builders reject null buffers, oversize ASDUs, and buffers too small.
void test_101_build_guards()
{
    uint8_t buf[32];
    const uint8_t asdu[6] = {0};
    TEST_ASSERT_EQUAL_size_t(0, iec101_build_fixed(buf, 4, 0x49, 0x01));                    // cap < 5
    TEST_ASSERT_EQUAL_size_t(0, iec101_build_fixed(nullptr, sizeof(buf), 0x49, 0x01));      // null buf
    TEST_ASSERT_EQUAL_size_t(0, iec101_build_variable(buf, sizeof(buf), 0, 0, nullptr, 6)); // len but null asdu
    TEST_ASSERT_EQUAL_size_t(0, iec101_build_variable(buf, 512, 0, 0, asdu, 254));          // asdu_len > 253
    TEST_ASSERT_EQUAL_size_t(0, iec101_build_variable(buf, 10, 0, 0, asdu, 6));             // cap < 6 + L
}

// -101 parse rejects an empty buffer, a corrupt fixed frame, and malformed variable frames.
void test_101_parse_rejects()
{
    Iec101Frame f;
    size_t c;
    TEST_ASSERT_FALSE(iec101_parse(nullptr, 5, &f, &c)); // null buf
    uint8_t empty[1] = {0};
    TEST_ASSERT_FALSE(iec101_parse(empty, 0, &f, &c)); // len < 1

    uint8_t bad_cksum[5] = {IEC_START_FIXED, 0x49, 0x01, 0xFF, IEC_STOP};
    TEST_ASSERT_FALSE(iec101_parse(bad_cksum, sizeof(bad_cksum), &f, &c)); // checksum mismatch
    uint8_t bad_stop[5] = {IEC_START_FIXED, 0x49, 0x01, 0x4A, 0x00};
    TEST_ASSERT_FALSE(iec101_parse(bad_stop, sizeof(bad_stop), &f, &c)); // no 0x16 stop

    uint8_t var_trunc[3] = {IEC_START_104, 0x08, 0x08};
    TEST_ASSERT_FALSE(iec101_parse(var_trunc, sizeof(var_trunc), &f, &c)); // len < 4
    uint8_t var_badhdr[8] = {IEC_START_104, 0x01, 0x01, IEC_START_104, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(iec101_parse(var_badhdr, sizeof(var_badhdr), &f, &c)); // L < 2
    uint8_t var_mismatch[8] = {IEC_START_104, 0x08, 0x09, IEC_START_104, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(iec101_parse(var_mismatch, sizeof(var_mismatch), &f, &c)); // buf[2] != L

    uint8_t unknown[5] = {0x99, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(iec101_parse(unknown, sizeof(unknown), &f, &c)); // neither 0x10 nor 0x68
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_104_i_format_roundtrip);
    RUN_TEST(test_104_s_format);
    RUN_TEST(test_104_u_format);
    RUN_TEST(test_104_sequence_numbers_15bit);
    RUN_TEST(test_asdu_header_roundtrip);
    RUN_TEST(test_ioa_roundtrip);
    RUN_TEST(test_101_fixed_frame);
    RUN_TEST(test_101_variable_frame_roundtrip);
    RUN_TEST(test_104_build_guards);
    RUN_TEST(test_104_parse_rejects);
    RUN_TEST(test_asdu_ioa_guards);
    RUN_TEST(test_101_build_guards);
    RUN_TEST(test_101_parse_rejects);
    return UNITY_END();
}
