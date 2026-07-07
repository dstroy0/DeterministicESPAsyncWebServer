// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the wired M-Bus codec (services/mbus): the ACK / short / long frame builders
// and parser (start/stop octets, doubled length, 8-bit sum checksum), and the EN 13757-3
// variable-data record walker (DIF coding lengths, DIFE/VIFE extension chains, LVAR strings).
// Pure host tests.

#include "services/mbus/mbus.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_ack()
{
    uint8_t buf[4];
    TEST_ASSERT_EQUAL_size_t(1, mbus_build_ack(buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_HEX8(0xE5, buf[0]);
    MbusFrame f;
    size_t c;
    TEST_ASSERT_TRUE(mbus_parse(buf, 1, &f, &c));
    TEST_ASSERT_EQUAL_INT(MBUS_FRAME_ACK, f.type);
    TEST_ASSERT_EQUAL_size_t(1, c);
}

void test_short_frame_roundtrip()
{
    uint8_t buf[8];
    size_t n = mbus_build_snd_nke(buf, sizeof(buf), 0x05);
    TEST_ASSERT_EQUAL_size_t(5, n);
    const uint8_t expect[] = {0x10, 0x40, 0x05, 0x45, 0x16}; // CS = 0x40 + 0x05
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, 5);

    MbusFrame f;
    size_t c;
    TEST_ASSERT_TRUE(mbus_parse(buf, n, &f, &c));
    TEST_ASSERT_EQUAL_INT(MBUS_FRAME_SHORT, f.type);
    TEST_ASSERT_EQUAL_HEX8(MBUS_C_SND_NKE, f.c);
    TEST_ASSERT_EQUAL_HEX8(0x05, f.a);
    TEST_ASSERT_EQUAL_size_t(5, c);
}

void test_req_ud2_fcb()
{
    uint8_t buf[8];
    mbus_build_req_ud2(buf, sizeof(buf), 0x01, false);
    TEST_ASSERT_EQUAL_HEX8(0x5B, buf[1]);
    mbus_build_req_ud2(buf, sizeof(buf), 0x01, true);
    TEST_ASSERT_EQUAL_HEX8(0x7B, buf[1]); // FCB set
}

void test_long_frame_roundtrip()
{
    const uint8_t data[4] = {0x2A, 0x00, 0x00, 0x00};
    uint8_t buf[32];
    size_t n = mbus_build_long(buf, sizeof(buf), MBUS_C_RSP_UD, 0x01, MBUS_CI_RSP_VARIABLE, data, 4);
    TEST_ASSERT_EQUAL_size_t(6 + (3 + 4), n); // 6 framing octets + L (=7)
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(7, buf[1]); // L = 3 + 4
    TEST_ASSERT_EQUAL_HEX8(7, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x68, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x16, buf[n - 1]);

    MbusFrame f;
    size_t c;
    TEST_ASSERT_TRUE(mbus_parse(buf, n, &f, &c));
    TEST_ASSERT_EQUAL_INT(MBUS_FRAME_LONG, f.type);
    TEST_ASSERT_EQUAL_HEX8(MBUS_C_RSP_UD, f.c);
    TEST_ASSERT_EQUAL_HEX8(0x01, f.a);
    TEST_ASSERT_EQUAL_HEX8(MBUS_CI_RSP_VARIABLE, f.ci);
    TEST_ASSERT_EQUAL_UINT8(4, f.data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, f.data, 4);
    TEST_ASSERT_EQUAL_size_t(n, c);
}

void test_parse_rejects_corruption()
{
    const uint8_t data[2] = {0xAA, 0xBB};
    uint8_t buf[32];
    size_t n = mbus_build_long(buf, sizeof(buf), 0x08, 0x01, 0x72, data, 2);
    MbusFrame f;
    size_t c;

    uint8_t bad_cs[32];
    memcpy(bad_cs, buf, n);
    bad_cs[n - 2] ^= 0xFF; // corrupt the checksum
    TEST_ASSERT_FALSE(mbus_parse(bad_cs, n, &f, &c));

    uint8_t bad_stop[32];
    memcpy(bad_stop, buf, n);
    bad_stop[n - 1] = 0x00; // wrong stop octet
    TEST_ASSERT_FALSE(mbus_parse(bad_stop, n, &f, &c));

    uint8_t bad_len[32];
    memcpy(bad_len, buf, n);
    bad_len[2] ^= 0x01; // the two L octets disagree
    TEST_ASSERT_FALSE(mbus_parse(bad_len, n, &f, &c));
}

void test_dif_data_len()
{
    TEST_ASSERT_EQUAL_UINT8(1, mbus_dif_data_len(MBUS_DIF_INT8));
    TEST_ASSERT_EQUAL_UINT8(4, mbus_dif_data_len(MBUS_DIF_INT32));
    TEST_ASSERT_EQUAL_UINT8(4, mbus_dif_data_len(MBUS_DIF_REAL32));
    TEST_ASSERT_EQUAL_UINT8(6, mbus_dif_data_len(MBUS_DIF_INT48));
    TEST_ASSERT_EQUAL_UINT8(8, mbus_dif_data_len(MBUS_DIF_INT64));
    TEST_ASSERT_EQUAL_UINT8(3, mbus_dif_data_len(MBUS_DIF_BCD6));
    TEST_ASSERT_EQUAL_UINT8(0, mbus_dif_data_len(MBUS_DIF_VARIABLE));
}

// Walk a record stream: INT32, INT16, an INT32 with a DIFE, and an LVAR ASCII string.
void test_record_walk()
{
    const uint8_t body[] = {
        0x04, 0x13, 0x2A, 0x00, 0x00, 0x00,       // DIF INT32, VIF 0x13, value 42
        0x02, 0x5A, 0x10, 0x27,                   // DIF INT16, VIF 0x5A, value 0x2710
        0x84, 0x01, 0x13, 0x01, 0x00, 0x00, 0x00, // DIF INT32 + DIFE 0x01, VIF 0x13, value 1
        0x0D, 0x7C, 0x03, 'A',  'B',  'C',        // DIF variable, VIF 0x7C, LVAR 3, "ABC"
    };
    size_t pos = 0;
    MbusRecord r;

    TEST_ASSERT_TRUE(mbus_record_next(body, sizeof(body), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(MBUS_DIF_INT32, r.coding);
    TEST_ASSERT_EQUAL_HEX8(0x13, r.vif);
    TEST_ASSERT_EQUAL_UINT8(4, r.data_len);
    TEST_ASSERT_EQUAL_HEX8(0x2A, r.data[0]);

    TEST_ASSERT_TRUE(mbus_record_next(body, sizeof(body), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(MBUS_DIF_INT16, r.coding);
    TEST_ASSERT_EQUAL_HEX8(0x5A, r.vif);
    TEST_ASSERT_EQUAL_UINT8(2, r.data_len);

    TEST_ASSERT_TRUE(mbus_record_next(body, sizeof(body), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(MBUS_DIF_INT32, r.coding); // DIFE skipped
    TEST_ASSERT_EQUAL_HEX8(0x13, r.vif);
    TEST_ASSERT_EQUAL_UINT8(4, r.data_len);

    TEST_ASSERT_TRUE(mbus_record_next(body, sizeof(body), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(MBUS_DIF_VARIABLE, r.coding);
    TEST_ASSERT_EQUAL_UINT8(3, r.data_len);
    TEST_ASSERT_EQUAL_MEMORY("ABC", r.data, 3);

    TEST_ASSERT_FALSE(mbus_record_next(body, sizeof(body), &pos, &r)); // end of data
}

void test_record_truncated_fails()
{
    const uint8_t body[] = {0x04, 0x13, 0x2A, 0x00}; // INT32 claims 4 octets, only 2 present
    size_t pos = 0;
    MbusRecord r;
    TEST_ASSERT_FALSE(mbus_record_next(body, sizeof(body), &pos, &r));
}

void test_build_and_parse_guards()
{
    uint8_t buf[32];
    const uint8_t d[4] = {1, 2, 3, 4};
    // Builder guards.
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_ack(nullptr, 4));
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_ack(buf, 0));
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_short(nullptr, 8, 0x40, 1));
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_short(buf, 4, 0x40, 1)); // cap < 5
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_long(nullptr, 32, 0, 0, 0, d, 4));
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_long(buf, 32, 0, 0, 0, nullptr, 4)); // data_len && !data
    TEST_ASSERT_EQUAL_size_t(0, mbus_build_long(buf, 5, 0, 0, 0, d, 4));        // cap < total

    // Parser guards.
    MbusFrame f;
    size_t c;
    TEST_ASSERT_FALSE(mbus_parse(nullptr, 5, &f, &c));
    TEST_ASSERT_FALSE(mbus_parse(buf, 0, &f, &c));
    const uint8_t unknown[] = {0x99, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(mbus_parse(unknown, sizeof(unknown), &f, &c)); // unrecognized start octet

    uint8_t s[5];
    mbus_build_short(s, sizeof(s), 0x40, 0x05);
    TEST_ASSERT_FALSE(mbus_parse(s, 4, &f, &c)); // len < 5
    uint8_t s2[5];
    memcpy(s2, s, 5);
    s2[4] = 0x00; // bad stop
    TEST_ASSERT_FALSE(mbus_parse(s2, 5, &f, &c));
    memcpy(s2, s, 5);
    s2[3] ^= 0xFF; // bad checksum
    TEST_ASSERT_FALSE(mbus_parse(s2, 5, &f, &c));

    const uint8_t l[] = {0x68, 0x07, 0x07}; // long start, header truncated (< 4)
    TEST_ASSERT_FALSE(mbus_parse(l, sizeof(l), &f, &c));
}

void test_dif_data_len_remaining()
{
    TEST_ASSERT_EQUAL_UINT8(0, mbus_dif_data_len(MBUS_DIF_NONE));
    TEST_ASSERT_EQUAL_UINT8(3, mbus_dif_data_len(MBUS_DIF_INT24));
    TEST_ASSERT_EQUAL_UINT8(0, mbus_dif_data_len(MBUS_DIF_READOUT));
    TEST_ASSERT_EQUAL_UINT8(1, mbus_dif_data_len(MBUS_DIF_BCD2));
    TEST_ASSERT_EQUAL_UINT8(2, mbus_dif_data_len(MBUS_DIF_BCD4));
    TEST_ASSERT_EQUAL_UINT8(4, mbus_dif_data_len(MBUS_DIF_BCD8));
    TEST_ASSERT_EQUAL_UINT8(6, mbus_dif_data_len(MBUS_DIF_BCD12));
    TEST_ASSERT_EQUAL_UINT8(0, mbus_dif_data_len(MBUS_DIF_SPECIAL));
}

void test_record_edges()
{
    size_t pos;
    MbusRecord r;

    const uint8_t dife_trunc[] = {0x84}; // continuation bit set, DIFE missing
    pos = 0;
    TEST_ASSERT_FALSE(mbus_record_next(dife_trunc, sizeof(dife_trunc), &pos, &r));

    const uint8_t special[] = {0x0F, 0xAB, 0xCD}; // SPECIAL coding: no VIF, no data
    pos = 0;
    TEST_ASSERT_TRUE(mbus_record_next(special, sizeof(special), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(MBUS_DIF_SPECIAL, r.coding);
    TEST_ASSERT_EQUAL_size_t(1, pos); // consumed only the DIF

    const uint8_t no_vif[] = {0x04}; // INT32 DIF with no VIF
    pos = 0;
    TEST_ASSERT_FALSE(mbus_record_next(no_vif, sizeof(no_vif), &pos, &r));

    const uint8_t vife_trunc[] = {0x04, 0x93}; // VIF continuation bit but VIFE missing
    pos = 0;
    TEST_ASSERT_FALSE(mbus_record_next(vife_trunc, sizeof(vife_trunc), &pos, &r));

    const uint8_t lvar_missing[] = {0x0D, 0x7C}; // VARIABLE coding, LVAR octet missing
    pos = 0;
    TEST_ASSERT_FALSE(mbus_record_next(lvar_missing, sizeof(lvar_missing), &pos, &r));

    const uint8_t lvar_big[] = {0x0D, 0x7C, 0xC0}; // LVAR > 0xBF unsupported
    pos = 0;
    TEST_ASSERT_FALSE(mbus_record_next(lvar_big, sizeof(lvar_big), &pos, &r));
}

// A VIF with the extension bit set is followed by a present VIFE octet (the chain read).
void test_record_vife_chain()
{
    const uint8_t body[] = {MBUS_DIF_INT8, 0x93, 0x13, 0x42}; // DIF INT8, VIF 0x93 (ext), VIFE 0x13, data
    size_t pos = 0;
    MbusRecord r;
    TEST_ASSERT_TRUE(mbus_record_next(body, sizeof(body), &pos, &r));
    TEST_ASSERT_EQUAL_HEX8(0x93, r.vif);
    TEST_ASSERT_EQUAL_UINT8(1, r.data_len);
    TEST_ASSERT_EQUAL_HEX8(0x42, r.data[0]);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ack);
    RUN_TEST(test_short_frame_roundtrip);
    RUN_TEST(test_req_ud2_fcb);
    RUN_TEST(test_long_frame_roundtrip);
    RUN_TEST(test_parse_rejects_corruption);
    RUN_TEST(test_dif_data_len);
    RUN_TEST(test_record_walk);
    RUN_TEST(test_record_truncated_fails);
    RUN_TEST(test_build_and_parse_guards);
    RUN_TEST(test_dif_data_len_remaining);
    RUN_TEST(test_record_edges);
    RUN_TEST(test_record_vife_chain);
    return UNITY_END();
}
