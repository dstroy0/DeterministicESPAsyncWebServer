// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/http_delivery: RFC 5861 stale-while-revalidate, RFC 7233 byte ranges,
// and the service-worker precache manifest.

#include "services/http_delivery/http_delivery.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_swr_decision(void)
{
    // max-age=60, swr=30.
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_FRESH, detws_delivery_swr(0, 60, 30));
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_FRESH, detws_delivery_swr(60, 60, 30)); // boundary: <= max-age
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_STALE_REVALIDATE, detws_delivery_swr(61, 60, 30));
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_STALE_REVALIDATE,
                          detws_delivery_swr(90, 60, 30)); // boundary: max+swr
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_EXPIRED, detws_delivery_swr(91, 60, 30));
    // No swr window.
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_EXPIRED, detws_delivery_swr(61, 60, 0));
    // Overflow guard: age > max-age, but max-age + swr wraps uint32 (0xFFFFFFF0 + 0x20 = 0x100000010).
    // A 32-bit add would wrap to 0x10 and wrongly report EXPIRED; the uint64 window keeps it STALE.
    TEST_ASSERT_EQUAL_INT(DeliveryVerdict::DELIVERY_STALE_REVALIDATE,
                          detws_delivery_swr(0xFFFFFFF8u, 0xFFFFFFF0u, 0x20u));
}

void test_cache_control(void)
{
    char buf[64];
    size_t n = detws_delivery_cache_control(60, 30, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_EQUAL_STRING("public, max-age=60, stale-while-revalidate=30", buf);
    detws_delivery_cache_control(3600, 0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("public, max-age=3600", buf);
    // Overflow.
    char tiny[8];
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_cache_control(3600, 30, tiny, sizeof(tiny)));
}

void test_range_forms(void)
{
    uint32_t s = 0, e = 0;
    // X-Y
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=0-499", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(0, s);
    TEST_ASSERT_EQUAL_UINT32(499, e);
    // X- (from offset to end) -- the delta/offset log fetch case.
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=500-", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(500, s);
    TEST_ASSERT_EQUAL_UINT32(999, e);
    // -N (suffix / last N bytes)
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=-200", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(800, s);
    TEST_ASSERT_EQUAL_UINT32(999, e);
    // Suffix larger than resource -> whole resource.
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=-5000", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(0, s);
    TEST_ASSERT_EQUAL_UINT32(999, e);
    // End past resource -> clamped.
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=990-99999", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(990, s);
    TEST_ASSERT_EQUAL_UINT32(999, e);
    // Leading whitespace tolerated.
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("  bytes=0-0", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(0, s);
    TEST_ASSERT_EQUAL_UINT32(0, e);
}

void test_range_rejects(void)
{
    uint32_t s = 0, e = 0;
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=1000-2000", 1000, &s, &e));     // start >= total -> 416
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=500-100", 1000, &s, &e));       // start > end
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=-0", 1000, &s, &e));            // zero suffix
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=-", 1000, &s, &e));             // empty spec
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=0-100,200-300", 1000, &s, &e)); // multi-range unsupported
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("items=0-100", 1000, &s, &e));         // wrong unit
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=0-499", 0, &s, &e));            // empty resource
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=0-abc", 1000, &s, &e));         // garbage end
}

void test_content_range(void)
{
    char buf[48];
    size_t n = detws_delivery_content_range(500, 999, 1000, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_EQUAL_STRING("bytes 500-999/1000", buf);
}

void test_sw_manifest(void)
{
    const char *paths[3] = {"/", "/app.js", "/style.css"};
    char buf[128];
    size_t n = detws_delivery_sw_manifest(paths, 3, "v42", buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_EQUAL_STRING("{\"version\":\"v42\",\"precache\":[\"/\",\"/app.js\",\"/style.css\"]}", buf);
    // Empty list.
    detws_delivery_sw_manifest(nullptr, 0, "v1", buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"version\":\"v1\",\"precache\":[]}", buf);
}

void test_delivery_guards_and_escape()
{
    char buf[256];
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_cache_control(60, 30, nullptr, sizeof(buf))); // null out
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_cache_control(60, 30, buf, 0));               // zero cap
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_content_range(0, 10, 100, buf, 2));           // tiny cap
    uint32_t start = 0, end = 0;
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("garbage", 100, &start, &end)); // no valid range -> ignore
    (void)detws_delivery_range("bytes=500-600", 100, &start, &end);               // exercise the unsatisfiable path
    const char *paths[1] = {"a\"b\\c"};                                           // quote + backslash
    size_t n = detws_delivery_sw_manifest(paths, 1, "1.0", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\\\""));                                        // escaped quote present
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_sw_manifest(paths, 1, "1.0", buf, 4)); // tiny cap fails closed
}

// Byte-range parser edges (u32 overflow guards, missing dash, trailing whitespace) and
// the null-output guards on the content-range / service-worker-manifest builders.
void test_range_and_builder_edge_guards(void)
{
    uint32_t s = 0, e = 0;
    // Oversized start (>10 digits) -> read_u32 overflow guard rejects.
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=99999999999-", 1000, &s, &e));
    // A start with no '-' following -> reject.
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=5", 1000, &s, &e));
    // Oversized end (>10 digits) -> reject.
    TEST_ASSERT_EQUAL_INT(0, detws_delivery_range("bytes=0-99999999999", 1000, &s, &e));
    // Trailing whitespace after a valid range is tolerated.
    TEST_ASSERT_EQUAL_INT(1, detws_delivery_range("bytes=0-5 ", 1000, &s, &e));
    TEST_ASSERT_EQUAL_UINT32(0, s);
    TEST_ASSERT_EQUAL_UINT32(5, e);

    char buf[64];
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_content_range(0, 10, 100, nullptr, sizeof(buf))); // null out
    const char *paths[1] = {"/a"};
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_sw_manifest(paths, 1, "v", nullptr, sizeof(buf))); // null out
    TEST_ASSERT_EQUAL_size_t(0, detws_delivery_sw_manifest(nullptr, 2, "v", buf, sizeof(buf)));   // n>0, null paths
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_range_and_builder_edge_guards);
    RUN_TEST(test_swr_decision);
    RUN_TEST(test_cache_control);
    RUN_TEST(test_range_forms);
    RUN_TEST(test_range_rejects);
    RUN_TEST(test_content_range);
    RUN_TEST(test_sw_manifest);
    RUN_TEST(test_delivery_guards_and_escape);
    return UNITY_END();
}
