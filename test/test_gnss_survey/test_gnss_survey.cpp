// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the GNSS survey-in core (services/gnss/gnss_survey): the WGS84 geodetic->ECEF transform,
// its iterative inverse, the fixed-position averager + 3-D accuracy estimate, and the GGA->geodetic fold.
// The ECEF reference vectors below were produced by pyproj 3.7.2 transforming EPSG:4979 (WGS84 lat/lon/
// ellipsoidal height) to EPSG:4978 (WGS84 geocentric ECEF); the C transform must match to <= 0.1 mm.

#include "services/gnss/gnss_survey.h"
#include "services/nmea0183/nmea0183.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// One pyproj-derived case: geodetic in, ECEF (metres) out.
struct EcefCase
{
    double lat, lon, h;
    double x, y, z;
};

static const EcefCase CASES[] = {
    {0.0, 0.0, 0.0, 6378137.000000, 0.000000, 0.000000},
    {45.0, 45.0, 100.0, 3194469.145061, 3194469.145061, 4487419.119544},
    {37.7749, -122.4194, 52.0, -2706196.881923, -4261094.185420, 3885757.343188},
    {-33.8688, 151.2093, 58.0, -4646093.477288, 2553229.535817, -3534404.710910},
};

void test_geodetic_to_ecef_matches_pyproj()
{
    for (unsigned i = 0; i < sizeof(CASES) / sizeof(CASES[0]); i++)
    {
        GnssGeodetic g = {CASES[i].lat, CASES[i].lon, CASES[i].h};
        GnssEcef e;
        gnss_geodetic_to_ecef(&g, &e);
        // pyproj prints to 1e-6 m; allow 0.2 mm for that rounding plus our own.
        TEST_ASSERT_DOUBLE_WITHIN(2e-4, CASES[i].x, e.x);
        TEST_ASSERT_DOUBLE_WITHIN(2e-4, CASES[i].y, e.y);
        TEST_ASSERT_DOUBLE_WITHIN(2e-4, CASES[i].z, e.z);
    }
}

void test_ecef_to_geodetic_roundtrip()
{
    for (unsigned i = 0; i < sizeof(CASES) / sizeof(CASES[0]); i++)
    {
        GnssEcef e = {CASES[i].x, CASES[i].y, CASES[i].z};
        GnssGeodetic g;
        gnss_ecef_to_geodetic(&e, &g);
        TEST_ASSERT_DOUBLE_WITHIN(1e-8, CASES[i].lat, g.lat_deg); // ~1 mm in latitude
        TEST_ASSERT_DOUBLE_WITHIN(1e-8, CASES[i].lon, g.lon_deg);
        TEST_ASSERT_DOUBLE_WITHIN(1e-3, CASES[i].h, g.height_m);
    }
}

void test_m_to_01mm_rounds_half_away()
{
    TEST_ASSERT_EQUAL_INT64(63781370000LL, gnss_ecef_m_to_01mm(6378137.0));
    TEST_ASSERT_EQUAL_INT64(1, gnss_ecef_m_to_01mm(0.00005));   // 0.00005 m = 0.5 units -> 1
    TEST_ASSERT_EQUAL_INT64(-1, gnss_ecef_m_to_01mm(-0.00005)); // -0.5 units -> -1
    TEST_ASSERT_EQUAL_INT64(0, gnss_ecef_m_to_01mm(0.0));
}

// A single fix converts, averages to itself, and yields the same 0.1 mm ECEF pyproj gave.
void test_survey_single_fix_matches_reference()
{
    GnssSurvey s;
    gnss_survey_reset(&s);
    GnssGeodetic g = {37.7749, -122.4194, 52.0};
    gnss_survey_add_geodetic(&s, &g);
    TEST_ASSERT_EQUAL_UINT32(1, gnss_survey_count(&s));

    GnssEcef mean;
    TEST_ASSERT_TRUE(gnss_survey_mean(&s, &mean));
    TEST_ASSERT_EQUAL_INT64(-27061968819LL, gnss_ecef_m_to_01mm(mean.x));
    TEST_ASSERT_EQUAL_INT64(-42610941854LL, gnss_ecef_m_to_01mm(mean.y));
    TEST_ASSERT_EQUAL_INT64(38857573432LL, gnss_ecef_m_to_01mm(mean.z));
    // One fix -> no spread, and not "complete" (needs >= 2 obs).
    TEST_ASSERT_EQUAL_DOUBLE(0.0, gnss_survey_accuracy_m(&s));
    TEST_ASSERT_FALSE(gnss_survey_complete(&s, 1, 10.0));
}

// Symmetric scatter about a true point: the mean lands on the truth and the accuracy is the RMS spread.
void test_survey_averages_out_scatter()
{
    GnssEcef truth = {-2706196.881923, -4261094.185420, 3885757.343188};
    GnssSurvey s;
    gnss_survey_reset(&s);
    const double d = 1.5; // metres of jitter on each axis
    // Four fixes: +/-d on x and +/-d on y, truth on z. Mean of the offsets is exactly zero.
    GnssEcef fixes[4] = {
        {truth.x + d, truth.y + d, truth.z},
        {truth.x - d, truth.y - d, truth.z},
        {truth.x + d, truth.y - d, truth.z},
        {truth.x - d, truth.y + d, truth.z},
    };
    for (int i = 0; i < 4; i++)
        gnss_survey_add_ecef(&s, &fixes[i]);

    TEST_ASSERT_EQUAL_UINT32(4, gnss_survey_count(&s));
    GnssEcef mean;
    TEST_ASSERT_TRUE(gnss_survey_mean(&s, &mean));
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, truth.x, mean.x);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, truth.y, mean.y);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, truth.z, mean.z);

    // Per-axis population variance is d^2 on x and y, 0 on z -> 3-D sigma = sqrt(2) * d.
    double expect = 1.4142135623730951 * d;
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, expect, gnss_survey_accuracy_m(&s));

    // Convergence gate: enough obs + spread within limit.
    TEST_ASSERT_TRUE(gnss_survey_complete(&s, 4, 3.0));
    TEST_ASSERT_FALSE(gnss_survey_complete(&s, 4, 2.0)); // spread ~2.12 m > 2.0 m
    TEST_ASSERT_FALSE(gnss_survey_complete(&s, 5, 3.0)); // not enough observations
}

void test_survey_empty_has_no_mean()
{
    GnssSurvey s;
    gnss_survey_reset(&s);
    GnssEcef mean;
    memset(&mean, 0x5A, sizeof(mean));
    TEST_ASSERT_FALSE(gnss_survey_mean(&s, &mean));
    TEST_ASSERT_EQUAL_UINT32(0, gnss_survey_count(&s));
}

// A GT-U7-style GGA: 37 deg 23.2475' N, 122 deg 02.1236' W, MSL 30.5 m, geoid -25.0 m. Build it (so the
// checksum is correct by construction) then parse it back.
void test_gga_to_geodetic()
{
    char buf[96];
    const char *body = "GPGGA,124515.00,3723.2475,N,12202.1236,W,1,08,0.9,30.5,M,-25.0,M,,";
    size_t n = nmea0183_build(buf, sizeof(buf), body);
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(buf, n, &m));

    GnssGeodetic g;
    TEST_ASSERT_TRUE(gnss_gga_to_geodetic(&m, &g));
    // 3723.2475 -> 37 + 23.2475/60 = 37.387458...; W/S negate.
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 37.3874583333, g.lat_deg);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, -122.0353933333, g.lon_deg);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 5.5, g.height_m); // 30.5 + (-25.0)
}

void test_gga_no_fix_rejected()
{
    // Fix quality field (index 6) = 0 -> no fix -> reject.
    char buf[96];
    const char *body = "GPGGA,124515.00,3723.2475,N,12202.1236,W,0,00,99.9,,M,,M,,";
    size_t n = nmea0183_build(buf, sizeof(buf), body);
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(buf, n, &m));
    GnssGeodetic g;
    TEST_ASSERT_FALSE(gnss_gga_to_geodetic(&m, &g));
}

void test_survey_add_gga_folds_fix()
{
    char buf[96];
    const char *body = "GPGGA,124515.00,3723.2475,N,12202.1236,W,1,08,0.9,30.5,M,-25.0,M,,";
    size_t n = nmea0183_build(buf, sizeof(buf), body);
    TEST_ASSERT_TRUE(n > 0);
    Nmea0183 m;
    TEST_ASSERT_TRUE(nmea0183_parse(buf, n, &m));

    GnssSurvey s;
    gnss_survey_reset(&s);
    TEST_ASSERT_TRUE(gnss_survey_add_gga(&s, &m));
    TEST_ASSERT_EQUAL_UINT32(1, gnss_survey_count(&s));

    // The folded fix matches a direct geodetic->ECEF of the same position.
    GnssGeodetic g = {37.3874583333, -122.0353933333, 5.5};
    GnssEcef direct;
    gnss_geodetic_to_ecef(&g, &direct);
    GnssEcef mean;
    TEST_ASSERT_TRUE(gnss_survey_mean(&s, &mean));
    TEST_ASSERT_DOUBLE_WITHIN(1e-2, direct.x, mean.x);
    TEST_ASSERT_DOUBLE_WITHIN(1e-2, direct.y, mean.y);
    TEST_ASSERT_DOUBLE_WITHIN(1e-2, direct.z, mean.z);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_geodetic_to_ecef_matches_pyproj);
    RUN_TEST(test_ecef_to_geodetic_roundtrip);
    RUN_TEST(test_m_to_01mm_rounds_half_away);
    RUN_TEST(test_survey_single_fix_matches_reference);
    RUN_TEST(test_survey_averages_out_scatter);
    RUN_TEST(test_survey_empty_has_no_mean);
    RUN_TEST(test_gga_to_geodetic);
    RUN_TEST(test_gga_no_fix_rejected);
    RUN_TEST(test_survey_add_gga_folds_fix);
    return UNITY_END();
}
