// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the NMEA 0183 codec (services/nmea0183): the XOR checksum, sentence build,
// parse (field splitting, talker/type, checksum validation), and the field-value helpers. The
// canonical GGA example (checksum 0x47) is used as an independent known-answer vector. Pure
// host tests.

#include "services/nmea0183/nmea0183.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The standard GGA example; its documented checksum is 0x47.
static const char *GGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
static const char *GGA_BODY = "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";

void test_checksum_known_vector()
{
    TEST_ASSERT_EQUAL_HEX8(0x47, nmea0183_checksum(GGA_BODY, strlen(GGA_BODY)));
}

void test_build()
{
    char buf[96];
    size_t n = nmea0183_build(buf, sizeof(buf), GGA_BODY);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_CHAR('$', buf[0]);
    TEST_ASSERT_EQUAL_CHAR('\n', buf[n - 1]);
    // The built sentence must carry the correct *47 checksum and CRLF.
    TEST_ASSERT_TRUE(strstr(buf, "*47\r\n") != nullptr);
}

void test_parse_gga()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(GGA, strlen(GGA), &m));
    TEST_ASSERT_EQUAL_STRING("GP", m.talker);
    TEST_ASSERT_EQUAL_STRING("GGA", m.type);
    // 15 fields: address + 14 data fields (two trailing empty).
    TEST_ASSERT_EQUAL_UINT8(15, m.field_count);
    TEST_ASSERT_EQUAL_UINT8(8, m.field_len[2]); // "4807.038"
    TEST_ASSERT_EQUAL_MEMORY("4807.038", m.fields[2], 8);
    TEST_ASSERT_EQUAL_MEMORY("N", m.fields[3], 1);
    TEST_ASSERT_EQUAL_UINT8(0, m.field_len[13]); // trailing empty field
}

void test_field_helpers()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(GGA, strlen(GGA), &m));
    float lat = 0;
    TEST_ASSERT_TRUE(nmea0183_field_float(&m, 2, &lat));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4807.038f, lat);
    long sats = 0;
    TEST_ASSERT_TRUE(nmea0183_field_int(&m, 7, &sats)); // "08"
    TEST_ASSERT_EQUAL_INT(8, sats);
    // an empty field yields no value.
    float none;
    TEST_ASSERT_FALSE(nmea0183_field_float(&m, 13, &none));
    // a non-numeric field (the "N" hemisphere) yields no value.
    TEST_ASSERT_FALSE(nmea0183_field_int(&m, 3, &sats));
}

void test_parse_rejects_bad_checksum()
{
    // Flip the checksum digits.
    char bad[96];
    strcpy(bad, GGA);
    char *star = strchr(bad, '*');
    star[1] = '0';
    star[2] = '0';
    Nmea0183 m;
    TEST_ASSERT_FALSE(nmea0183_parse(bad, strlen(bad), &m));
}

void test_parse_rejects_no_dollar()
{
    Nmea0183 m;
    TEST_ASSERT_FALSE(nmea0183_parse("GPGGA,1,2*00\r\n", 14, &m));
}

// Round-trip: build from a body, then parse the result back.
void test_build_then_parse()
{
    char buf[96];
    size_t n = nmea0183_build(buf, sizeof(buf), "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W");
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_STRING("GP", m.talker);
    TEST_ASSERT_EQUAL_STRING("RMC", m.type);
    TEST_ASSERT_EQUAL_MEMORY("A", m.fields[2], 1); // status = active
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_checksum_known_vector);
    RUN_TEST(test_build);
    RUN_TEST(test_parse_gga);
    RUN_TEST(test_field_helpers);
    RUN_TEST(test_parse_rejects_bad_checksum);
    RUN_TEST(test_parse_rejects_no_dollar);
    RUN_TEST(test_build_then_parse);
    return UNITY_END();
}
