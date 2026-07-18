// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the EnOcean ESP3 codec (services/enocean): the CRC-8 (poly 0x07) against
// known-answer values, a build -> parse round trip, malformed framing (bad sync / header
// CRC / data CRC), incomplete telegrams, over-length rejection, and resynchronisation.
// Pure host tests.
//
// The env sizes DWS_ENOCEAN_MAX_DATA = 16.

#include "services/enocean/enocean.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_crc8_known_answers()
{
    const uint8_t one[1] = {0x01};
    TEST_ASSERT_EQUAL_HEX8(0x07, dws_esp3_crc8(one, 1)); // hand-derived for poly 0x07
    // CRC-8 (poly 0x07, init 0, no reflection) check value for "123456789" is 0xF4.
    const uint8_t check[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    TEST_ASSERT_EQUAL_HEX8(0xF4, dws_esp3_crc8(check, 9));
}

void test_build_then_parse_round_trip()
{
    const uint8_t data[7] = {0xF6, 0x50, 0x01, 0x02, 0x03, 0x04, 0x30}; // RORG + payload + sender + status
    const uint8_t opt[3] = {0x03, 0x00, 0x00};
    uint8_t buf[64];
    uint16_t n = dws_esp3_build(dws_esp3_type::ESP3_RADIO_ERP1, data, 7, opt, 3, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT16(17, n); // 6 header/crc + 7 data + 3 opt + 1 crc
    TEST_ASSERT_EQUAL_HEX8(ESP3_SYNC, buf[0]);

    dws_esp3_packet p = {};
    int c = dws_esp3_parse(buf, n, &p);
    TEST_ASSERT_EQUAL_INT(17, c);
    TEST_ASSERT_EQUAL_UINT8(dws_esp3_type::ESP3_RADIO_ERP1, p.type);
    TEST_ASSERT_EQUAL_UINT16(7, p.data_len);
    TEST_ASSERT_EQUAL_UINT8(3, p.opt_len);
    TEST_ASSERT_EQUAL_MEMORY(data, p.data, 7);
    TEST_ASSERT_EQUAL_MEMORY(opt, p.opt, 3);
}

static uint16_t sample(uint8_t *buf, uint16_t cap)
{
    const uint8_t data[4] = {0xD5, 0x08, 0x11, 0x22};
    return dws_esp3_build(dws_esp3_type::ESP3_RADIO_ERP1, data, 4, nullptr, 0, buf, cap);
}

void test_parse_rejects_bad_sync()
{
    uint8_t buf[32];
    uint16_t n = sample(buf, sizeof(buf));
    buf[0] = 0x00;
    TEST_ASSERT_EQUAL_INT(-1, dws_esp3_parse(buf, n, nullptr));
}

void test_parse_rejects_bad_header_crc()
{
    uint8_t buf[32];
    uint16_t n = sample(buf, sizeof(buf));
    buf[5] ^= 0xFF; // corrupt CRC8H
    TEST_ASSERT_EQUAL_INT(-1, dws_esp3_parse(buf, n, nullptr));
}

void test_parse_rejects_bad_data_crc()
{
    uint8_t buf[32];
    uint16_t n = sample(buf, sizeof(buf));
    buf[n - 1] ^= 0xFF; // corrupt CRC8D
    TEST_ASSERT_EQUAL_INT(-1, dws_esp3_parse(buf, n, nullptr));
}

void test_parse_needs_more_bytes()
{
    uint8_t buf[32];
    uint16_t n = sample(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(0, dws_esp3_parse(buf, 3, nullptr));     // header not complete
    TEST_ASSERT_EQUAL_INT(0, dws_esp3_parse(buf, n - 1, nullptr)); // data not complete
}

void test_parse_rejects_over_length()
{
    // A header claiming data_len 100 (> DWS_ENOCEAN_MAX_DATA = 16) is rejected early.
    uint8_t buf[8] = {ESP3_SYNC, 0x00, 100, 0x00, (uint8_t)dws_esp3_type::ESP3_RADIO_ERP1, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL_INT(-1, dws_esp3_parse(buf, sizeof(buf), nullptr));
}

void test_parse_resynchronises_after_junk()
{
    uint8_t tg[32];
    uint16_t n = sample(tg, sizeof(tg));
    uint8_t buf[40];
    buf[0] = 0x00; // a stray byte before the telegram
    memcpy(buf + 1, tg, n);
    TEST_ASSERT_EQUAL_INT(-1, dws_esp3_parse(buf, (uint16_t)(n + 1), nullptr)); // junk at [0]
    dws_esp3_packet p = {};
    TEST_ASSERT_EQUAL_INT((int)n, dws_esp3_parse(buf + 1, n, &p)); // resynced at the sync byte
    TEST_ASSERT_EQUAL_UINT16(4, p.data_len);
}

void test_build_bounds()
{
    uint8_t data[8] = {0};
    uint8_t small[10];
    TEST_ASSERT_EQUAL_UINT16(
        0, dws_esp3_build(dws_esp3_type::ESP3_RADIO_ERP1, data, 8, nullptr, 0, small, sizeof(small))); // 15 > 10
    uint8_t big[64];
    TEST_ASSERT_EQUAL_UINT16(
        0, dws_esp3_build(dws_esp3_type::ESP3_RADIO_ERP1, big, 17, nullptr, 0, big, sizeof(big))); // 17 > MAX_DATA 16
}

void test_esp3_parse_null_guard()
{
    dws_esp3_packet pkt;
    TEST_ASSERT_EQUAL_INT(0, dws_esp3_parse(nullptr, 10, &pkt)); // null raw
    uint8_t tiny[1] = {0};
    TEST_ASSERT_EQUAL_INT(0, dws_esp3_parse(tiny, 0, &pkt)); // zero length
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_crc8_known_answers);
    RUN_TEST(test_build_then_parse_round_trip);
    RUN_TEST(test_parse_rejects_bad_sync);
    RUN_TEST(test_parse_rejects_bad_header_crc);
    RUN_TEST(test_parse_rejects_bad_data_crc);
    RUN_TEST(test_parse_needs_more_bytes);
    RUN_TEST(test_parse_rejects_over_length);
    RUN_TEST(test_parse_resynchronises_after_junk);
    RUN_TEST(test_build_bounds);
    RUN_TEST(test_esp3_parse_null_guard);
    return UNITY_END();
}
