// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the NMEA 0183 codec (services/nmea0183): the XOR checksum, sentence build,
// parse (field splitting, talker/type, checksum validation), and the field-value helpers. The
// canonical GGA example (checksum 0x47) is used as an independent known-answer vector. Pure
// host tests.

#include "services/nmea0183/nmea0183.h"
#include <stdio.h>
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
    TEST_ASSERT_EQUAL_HEX8(0x47, dws_nmea0183_checksum(GGA_BODY, strlen(GGA_BODY)));
}

void test_build()
{
    char buf[96];
    size_t n = dws_nmea0183_build(buf, sizeof(buf), GGA_BODY);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_CHAR('$', buf[0]);
    TEST_ASSERT_EQUAL_CHAR('\n', buf[n - 1]);
    // The built sentence must carry the correct *47 checksum and CRLF.
    TEST_ASSERT_TRUE(strstr(buf, "*47\r\n") != nullptr);
}

void test_parse_gga()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(GGA, strlen(GGA), &m));
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
    TEST_ASSERT_TRUE(dws_nmea0183_parse(GGA, strlen(GGA), &m));
    float lat = 0;
    TEST_ASSERT_TRUE(dws_nmea0183_field_float(&m, 2, &lat));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4807.038f, lat);
    long sats = 0;
    TEST_ASSERT_TRUE(dws_nmea0183_field_int(&m, 7, &sats)); // "08"
    TEST_ASSERT_EQUAL_INT(8, sats);
    // an empty field yields no value.
    float none;
    TEST_ASSERT_FALSE(dws_nmea0183_field_float(&m, 13, &none));
    // a non-numeric field (the "N" hemisphere) yields no value.
    TEST_ASSERT_FALSE(dws_nmea0183_field_int(&m, 3, &sats));
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
    TEST_ASSERT_FALSE(dws_nmea0183_parse(bad, strlen(bad), &m));
}

void test_parse_rejects_no_dollar()
{
    Nmea0183 m;
    TEST_ASSERT_FALSE(dws_nmea0183_parse("GPGGA,1,2*00\r\n", 14, &m));
}

// Round-trip: build from a body, then parse the result back.
void test_build_then_parse()
{
    char buf[96];
    size_t n = dws_nmea0183_build(buf, sizeof(buf), "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W");
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_STRING("GP", m.talker);
    TEST_ASSERT_EQUAL_STRING("RMC", m.type);
    TEST_ASSERT_EQUAL_MEMORY("A", m.fields[2], 1); // status = active
}

// Builder guards, the lowercase / invalid checksum-hex paths, a no-checksum sentence,
// and the field-helper reject branches.
void test_nmea0183_error_paths()
{
    char buf[96];
    TEST_ASSERT_EQUAL_size_t(0, dws_nmea0183_build(nullptr, sizeof(buf), "GPGGA")); // null buf
    TEST_ASSERT_EQUAL_size_t(0, dws_nmea0183_build(buf, sizeof(buf), nullptr));     // null body
    TEST_ASSERT_EQUAL_size_t(0, dws_nmea0183_build(buf, 4, "GPGGA"));               // cap too small

    Nmea0183 m;
    // A lowercase checksum still validates (hex_val a-f). "GPGGA,1" -> checksum 0x4B -> "4b".
    const char *body = "GPGGA,1";
    uint8_t cs = dws_nmea0183_checksum(body, strlen(body));
    char lower[32];
    snprintf(lower, sizeof(lower), "$%s*%02x\r\n", body, cs); // %02x emits lowercase hex
    TEST_ASSERT_TRUE(dws_nmea0183_parse(lower, strlen(lower), &m));

    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,1*4G\r\n", 13, &m)); // 'G' is not a hex digit
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,123\r\n", 12, &m));  // no '*' checksum introducer

    TEST_ASSERT_TRUE(dws_nmea0183_parse(GGA, strlen(GGA), &m));
    float f;
    TEST_ASSERT_FALSE(dws_nmea0183_field_float(&m, 3, &f)); // "N": non-empty but not a number
    long v;
    TEST_ASSERT_FALSE(dws_nmea0183_field_int(nullptr, 0, &v)); // null message
    TEST_ASSERT_FALSE(dws_nmea0183_field_int(&m, 13, &v));     // empty field
}

// hex_val's low-end reject range (below '0') and lowercase-beyond-'f' reject range, plus the
// checksum guard's "first digit invalid" short-circuit (hi < 0, so lo is never evaluated).
void test_nmea0183_hex_val_edges()
{
    Nmea0183 m;
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,1*.4\r\n", 13, &m)); // '.' < '0': hi invalid
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,1*4z\r\n", 13, &m)); // 'z' > 'f': lo invalid
}

// Guard-clause branches in dws_nmea0183_parse: null s, null out, too-short len, and the '!'
// (AIS-encapsulated) leading character as an alternative to '$'.
void test_nmea0183_parse_guards()
{
    Nmea0183 m;
    TEST_ASSERT_FALSE(dws_nmea0183_parse(nullptr, 10, &m));           // null s
    TEST_ASSERT_FALSE(dws_nmea0183_parse(GGA, strlen(GGA), nullptr)); // null out
    TEST_ASSERT_FALSE(dws_nmea0183_parse("ab", 2, &m));               // len < 4

    char bang[96];
    strcpy(bang, GGA);
    bang[0] = '!'; // AIS-style leading char; checksum is unaffected (computed from s+1 onward).
    TEST_ASSERT_TRUE(dws_nmea0183_parse(bang, strlen(bang), &m));
    TEST_ASSERT_EQUAL_STRING("GP", m.talker);
}

// The '*'-search loop's natural (non-break) exit at i == len, a bare '\n' with no preceding '\r'
// as a stop character, and a '*' with only one trailing character (too few for two hex digits).
void test_nmea0183_parse_scan_edges()
{
    Nmea0183 m;
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,123", 10, &m)); // no '*', no CR/LF: loop runs to len
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,1\n", 9, &m));  // bare '\n' stops the scan
    TEST_ASSERT_FALSE(dws_nmea0183_parse("$GPGGA,1*4", 10, &m)); // '*' with only 1 trailing char
}

// The field-split loop's overflow guard (more comma-separated fields than
// DWS_NMEA0183_MAX_FIELDS) and talker/type derivation when the address field is shorter than
// either fixed-width copy, so the field-length check (not the index cap) ends each inner loop.
void test_nmea0183_field_overflow_and_short_address()
{
    char body[80];
    size_t bp = 0;
    body[bp++] = 'G';
    body[bp++] = 'P';
    for (int i = 0; i < 40; i++)
        body[bp++] = ',';
    body[bp] = '\0';
    char buf[160];
    size_t n = dws_nmea0183_build(buf, sizeof(buf), body);
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_UINT8(DWS_NMEA0183_MAX_FIELDS, m.field_count); // capped, extras dropped

    char buf2[32];
    size_t n2 = dws_nmea0183_build(buf2, sizeof(buf2), "G,1"); // 1-char address (< talker/type width)
    TEST_ASSERT_TRUE(n2 > 0);
    Nmea0183 m2;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(buf2, n2, &m2));
    TEST_ASSERT_EQUAL_STRING("G", m2.talker);
    TEST_ASSERT_EQUAL_STRING("", m2.type);
}

// Out-of-range idx and null out-pointer branches of the field-value helpers.
void test_nmea0183_field_helpers_more_guards()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(GGA, strlen(GGA), &m));
    float f;
    TEST_ASSERT_FALSE(dws_nmea0183_field_float(nullptr, 0, &f));        // null message
    TEST_ASSERT_FALSE(dws_nmea0183_field_float(&m, m.field_count, &f)); // idx out of range
    TEST_ASSERT_FALSE(dws_nmea0183_field_float(&m, 0, nullptr));        // null out
    long v;
    TEST_ASSERT_FALSE(dws_nmea0183_field_int(&m, m.field_count, &v)); // idx out of range
    TEST_ASSERT_FALSE(dws_nmea0183_field_int(&m, 0, nullptr));        // null out
}

// The classic textbook RMC (23 Mar 1994, 22.4 kn, course 84.4).
static const char *RMC = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";

void test_decode_gga()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(GGA, strlen(GGA), &m));
    DwsNmeaGga g;
    TEST_ASSERT_TRUE(dws_nmea0183_parse_gga(&m, &g));
    TEST_ASSERT_EQUAL_UINT8(12, g.hour);
    TEST_ASSERT_EQUAL_UINT8(35, g.minute);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 19.0f, g.second);
    TEST_ASSERT_FLOAT_WITHIN(0.0005f, 48.1173f, (float)g.lat_deg);   // 4807.038 N
    TEST_ASSERT_FLOAT_WITHIN(0.0005f, 11.516667f, (float)g.lon_deg); // 01131.000 E
    TEST_ASSERT_EQUAL_UINT8(1, g.fix_quality);
    TEST_ASSERT_EQUAL_UINT8(8, g.num_sats);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.9f, g.hdop);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 545.4f, g.alt_m);
}

void test_decode_rmc()
{
    Nmea0183 m;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(RMC, strlen(RMC), &m));
    DwsNmeaRmc r;
    TEST_ASSERT_TRUE(dws_nmea0183_parse_rmc(&m, &r));
    TEST_ASSERT_TRUE(r.valid); // status 'A'
    TEST_ASSERT_EQUAL_UINT8(12, r.hour);
    TEST_ASSERT_EQUAL_UINT8(35, r.minute);
    TEST_ASSERT_FLOAT_WITHIN(0.0005f, 48.1173f, (float)r.lat_deg);
    TEST_ASSERT_FLOAT_WITHIN(0.0005f, 11.516667f, (float)r.lon_deg);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 22.4f, r.speed_knots);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 84.4f, r.course_deg);
    TEST_ASSERT_EQUAL_UINT8(23, r.day);
    TEST_ASSERT_EQUAL_UINT8(3, r.month);
    TEST_ASSERT_EQUAL_UINT8(94, r.year);

    // A southern/western hemisphere sentence flips the coordinate signs, and a 'V' status is decoded as
    // invalid (but still parses). Build it so the checksum is correct.
    char buf[96];
    size_t bn = dws_nmea0183_build(buf, sizeof(buf), "GPRMC,000000,V,3345.678,S,15112.345,W,000.0,000.0,010100,,");
    TEST_ASSERT_TRUE(bn > 0);
    Nmea0183 m2;
    TEST_ASSERT_TRUE(dws_nmea0183_parse(buf, bn, &m2));
    DwsNmeaRmc r2;
    TEST_ASSERT_TRUE(dws_nmea0183_parse_rmc(&m2, &r2));
    TEST_ASSERT_FALSE(r2.valid);
    TEST_ASSERT_TRUE(r2.lat_deg < 0.0); // S -> negative
    TEST_ASSERT_TRUE(r2.lon_deg < 0.0); // W -> negative
}

void test_decode_type_mismatch()
{
    Nmea0183 m;
    dws_nmea0183_parse(GGA, strlen(GGA), &m);
    DwsNmeaRmc r;
    TEST_ASSERT_FALSE(dws_nmea0183_parse_rmc(&m, &r)); // a GGA is not an RMC
    DwsNmeaGga g;
    dws_nmea0183_parse(RMC, strlen(RMC), &m);
    TEST_ASSERT_FALSE(dws_nmea0183_parse_gga(&m, &g)); // an RMC is not a GGA
    // Null guards.
    TEST_ASSERT_FALSE(dws_nmea0183_parse_gga(nullptr, &g));
    TEST_ASSERT_FALSE(dws_nmea0183_parse_gga(&m, nullptr));
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
    RUN_TEST(test_nmea0183_error_paths);
    RUN_TEST(test_nmea0183_hex_val_edges);
    RUN_TEST(test_nmea0183_parse_guards);
    RUN_TEST(test_nmea0183_parse_scan_edges);
    RUN_TEST(test_nmea0183_field_overflow_and_short_address);
    RUN_TEST(test_nmea0183_field_helpers_more_guards);
    RUN_TEST(test_decode_gga);
    RUN_TEST(test_decode_rmc);
    RUN_TEST(test_decode_type_mismatch);
    return UNITY_END();
}
