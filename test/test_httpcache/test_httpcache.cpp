// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HTTP Cache-Control helpers (services/httpcache): the directive
// builder + presets, the tolerant parser, a build->parse round-trip, and the RFC 9111
// freshness-lifetime calculation. Pure host tests.

#include "services/httpcache/httpcache.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// --- presets / builder -----------------------------------------------------

void test_preset_immutable()
{
    DetwsCacheControl cc;
    cache_immutable_asset(&cc, 31536000u); // 1 year
    char b[96];
    size_t n = cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("public, max-age=31536000, immutable", b);
    TEST_ASSERT_EQUAL_size_t(n, strlen(b));
}

void test_preset_no_store_and_shared_and_revalidatable()
{
    DetwsCacheControl cc;
    char b[96];

    cache_no_store(&cc);
    cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("no-store", b);

    cache_shared(&cc, 60, 600);
    cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("public, max-age=60, s-maxage=600", b);

    cache_revalidatable(&cc, 300, 60);
    cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("public, max-age=300, stale-while-revalidate=60", b);

    cache_revalidatable(&cc, 300, -1); // swr omitted
    cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("public, max-age=300", b);
}

void test_build_manual_and_edges()
{
    DetwsCacheControl cc;
    cache_control_init(&cc);
    cc.cc_private = true;
    cc.no_cache = true;
    cc.max_age = 0;
    cc.must_revalidate = true;
    char b[96];
    cache_control_build(b, sizeof(b), &cc);
    TEST_ASSERT_EQUAL_STRING("private, no-cache, max-age=0, must-revalidate", b);

    // empty set -> 0 (nothing to emit)
    cache_control_init(&cc);
    TEST_ASSERT_EQUAL_size_t(0, cache_control_build(b, sizeof(b), &cc));

    // overflow -> 0, and never overruns
    cache_immutable_asset(&cc, 31536000u);
    char tiny[10];
    TEST_ASSERT_EQUAL_size_t(0, cache_control_build(tiny, sizeof(tiny), &cc));
}

// --- parser ----------------------------------------------------------------

void test_parse_response_directives()
{
    DetwsCacheControl cc;
    const char *s = "public, max-age=31536000, immutable";
    TEST_ASSERT_TRUE(cache_control_parse(s, strlen(s), &cc));
    TEST_ASSERT_TRUE(cc.cc_public);
    TEST_ASSERT_TRUE(cc.cc_immutable);
    TEST_ASSERT_EQUAL_INT32(31536000, cc.max_age);
    TEST_ASSERT_EQUAL_INT32(-1, cc.s_maxage); // absent
}

void test_parse_case_insensitive_and_quoted_and_unknown()
{
    DetwsCacheControl cc;
    // case-insensitive names, a quoted delta, extra OWS, and an unknown directive to ignore
    const char *s = "  No-Store ,  MAX-AGE=\"3600\" , community=cats , s-maxage = 120 ";
    TEST_ASSERT_TRUE(cache_control_parse(s, strlen(s), &cc));
    TEST_ASSERT_TRUE(cc.no_store);
    TEST_ASSERT_EQUAL_INT32(3600, cc.max_age);
    TEST_ASSERT_EQUAL_INT32(120, cc.s_maxage);
    TEST_ASSERT_FALSE(cc.cc_public);

    // a header of only unknown directives -> false
    const char *u = "foo, bar=1";
    TEST_ASSERT_FALSE(cache_control_parse(u, strlen(u), &cc));
}

void test_parse_request_directives()
{
    DetwsCacheControl cc;
    const char *s = "max-stale, min-fresh=30, only-if-cached";
    TEST_ASSERT_TRUE(cache_control_parse(s, strlen(s), &cc));
    TEST_ASSERT_EQUAL_INT32(-2, cc.max_stale); // present, no value = "any"
    TEST_ASSERT_EQUAL_INT32(30, cc.min_fresh);
    TEST_ASSERT_TRUE(cc.only_if_cached);

    const char *s2 = "max-stale=90";
    TEST_ASSERT_TRUE(cache_control_parse(s2, strlen(s2), &cc));
    TEST_ASSERT_EQUAL_INT32(90, cc.max_stale);
}

// build -> parse -> the response directives survive intact.
void test_build_parse_roundtrip()
{
    DetwsCacheControl a;
    cache_control_init(&a);
    a.cc_public = true;
    a.max_age = 300;
    a.s_maxage = 600;
    a.must_revalidate = true;
    a.no_transform = true;
    a.stale_if_error = 120;
    char b[128];
    size_t n = cache_control_build(b, sizeof(b), &a);
    TEST_ASSERT_GREATER_THAN_size_t(0, n);

    DetwsCacheControl c;
    TEST_ASSERT_TRUE(cache_control_parse(b, n, &c));
    TEST_ASSERT_EQUAL(a.cc_public, c.cc_public);
    TEST_ASSERT_EQUAL(a.must_revalidate, c.must_revalidate);
    TEST_ASSERT_EQUAL(a.no_transform, c.no_transform);
    TEST_ASSERT_EQUAL_INT32(a.max_age, c.max_age);
    TEST_ASSERT_EQUAL_INT32(a.s_maxage, c.s_maxage);
    TEST_ASSERT_EQUAL_INT32(a.stale_if_error, c.stale_if_error);
}

// --- freshness (RFC 9111 4.2.1) --------------------------------------------

void test_freshness_precedence()
{
    DetwsCacheControl cc;
    cache_control_init(&cc);
    cc.max_age = 100;
    cc.s_maxage = 200;

    // shared cache honors s-maxage first
    TEST_ASSERT_EQUAL_INT(200, (int)cache_freshness_lifetime(&cc, true, 999));
    // private cache ignores s-maxage, uses max-age
    TEST_ASSERT_EQUAL_INT(100, (int)cache_freshness_lifetime(&cc, false, 999));

    // no max-age/s-maxage -> Expires minus Date
    cache_control_init(&cc);
    TEST_ASSERT_EQUAL_INT(50, (int)cache_freshness_lifetime(&cc, true, 50));

    // nothing explicit -> -1 (heuristic needed)
    TEST_ASSERT_EQUAL_INT(-1, (int)cache_freshness_lifetime(&cc, true, -1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_preset_immutable);
    RUN_TEST(test_preset_no_store_and_shared_and_revalidatable);
    RUN_TEST(test_build_manual_and_edges);
    RUN_TEST(test_parse_response_directives);
    RUN_TEST(test_parse_case_insensitive_and_quoted_and_unknown);
    RUN_TEST(test_parse_request_directives);
    RUN_TEST(test_build_parse_roundtrip);
    RUN_TEST(test_freshness_precedence);
    return UNITY_END();
}
