// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/wifi_sniffer: 802.11 header decode, traffic tally, roaming decision.

#include "services/wifi_sniffer/wifi_sniffer.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

// A data frame from an AP (FromDS=1): FC=0x08,0x02, dur, 3 addresses, seq.
static const uint8_t DATA_FROMDS[] = {
    0x08, 0x02,                         // FC: type=2 (data), subtype=0, FromDS
    0x00, 0x00,                         // duration
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, // addr1 (RA / destination)
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, // addr2 (TA / BSSID)
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // addr3 (SA)
    0x00, 0x00                          // seq ctrl
};

// A beacon (management, subtype 8): FC=0x80,0x00.
static const uint8_t BEACON[] = {0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xaa, 0xbb,
                                 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00};

// A CTS control frame (subtype 0xC): FC=0xC4,0x00, dur, addr1 only (10 bytes).
static const uint8_t CTS[] = {0xc4, 0x00, 0x3a, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

void test_parse_data(void)
{
    WifiFrame f;
    TEST_ASSERT_TRUE(detws_wifi_parse(DATA_FROMDS, sizeof(DATA_FROMDS), &f));
    TEST_ASSERT_EQUAL_UINT8(WIFI_TYPE_DATA, f.type);
    TEST_ASSERT_EQUAL_UINT8(0, f.subtype);
    TEST_ASSERT_TRUE(f.from_ds);
    TEST_ASSERT_FALSE(f.to_ds);
    TEST_ASSERT_EQUAL_UINT8(3, f.naddr);
    uint8_t a2[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(a2, f.addr2, 6);
}

void test_parse_beacon(void)
{
    WifiFrame f;
    TEST_ASSERT_TRUE(detws_wifi_parse(BEACON, sizeof(BEACON), &f));
    TEST_ASSERT_EQUAL_UINT8(WIFI_TYPE_MGMT, f.type);
    TEST_ASSERT_EQUAL_UINT8(8, f.subtype); // beacon subtype
    TEST_ASSERT_EQUAL_UINT8(3, f.naddr);
}

void test_parse_ctrl_short(void)
{
    WifiFrame f;
    TEST_ASSERT_TRUE(detws_wifi_parse(CTS, sizeof(CTS), &f));
    TEST_ASSERT_EQUAL_UINT8(WIFI_TYPE_CTRL, f.type);
    TEST_ASSERT_EQUAL_UINT8(0x0C, f.subtype); // CTS
    TEST_ASSERT_EQUAL_UINT8(1, f.naddr);      // only addr1 fits
    // Too short is rejected.
    TEST_ASSERT_FALSE(detws_wifi_parse(CTS, 9, &f));
    TEST_ASSERT_FALSE(detws_wifi_parse(nullptr, 24, &f));
}

void test_stats(void)
{
    WifiStats s;
    detws_wifi_stats_reset(&s);
    WifiFrame f;
    detws_wifi_parse(BEACON, sizeof(BEACON), &f);
    detws_wifi_stats_add(&s, &f);
    detws_wifi_parse(DATA_FROMDS, sizeof(DATA_FROMDS), &f);
    detws_wifi_stats_add(&s, &f);
    detws_wifi_parse(CTS, sizeof(CTS), &f);
    detws_wifi_stats_add(&s, &f);
    TEST_ASSERT_EQUAL_UINT32(1, s.mgmt);
    TEST_ASSERT_EQUAL_UINT32(1, s.data);
    TEST_ASSERT_EQUAL_UINT32(1, s.ctrl);
    TEST_ASSERT_EQUAL_UINT32(3, s.total);
}

void test_roam(void)
{
    // Current -80 dBm, candidate -70 dBm, 8 dB hysteresis: 10 > 8 -> roam.
    TEST_ASSERT_TRUE(detws_wifi_should_roam(-80, -70, 8));
    // Candidate only 5 dB stronger -> stay.
    TEST_ASSERT_FALSE(detws_wifi_should_roam(-80, -75, 8));
    // Candidate weaker -> stay.
    TEST_ASSERT_FALSE(detws_wifi_should_roam(-60, -75, 8));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_data);
    RUN_TEST(test_parse_beacon);
    RUN_TEST(test_parse_ctrl_short);
    RUN_TEST(test_stats);
    RUN_TEST(test_roam);
    return UNITY_END();
}
