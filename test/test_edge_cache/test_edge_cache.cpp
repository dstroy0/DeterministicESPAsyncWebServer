// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Pure host tests for the CDN edge-cache engine (services/edge_cache): response header-field access,
// HTTP-date parsing, RFC 9111 freshness, and the cache key + digest + Vary secondary key.

#include "server/http_range.h" // http_parse_byte_range (Range/206 serving from the cache)
#include "services/edge_cache/edge_cache.h"
#include "services/httpcache/httpcache.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static const char *RESP_HEAD = "HTTP/1.1 200 OK\r\n"
                               "ETag: \"abc123\"\r\n"
                               "Cache-Control:   max-age=60  \r\n"
                               "Last-Modified: Sun, 06 Nov 1994 08:49:37 GMT\r\n"
                               "Content-Type: text/html\r\n"
                               "\r\n";

// --- header field access -------------------------------------------------------------------------

static void test_header_value_found()
{
    char out[64];
    TEST_ASSERT_TRUE(edge_header_value(RESP_HEAD, strlen(RESP_HEAD), "ETag", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("\"abc123\"", out);
    TEST_ASSERT_TRUE(edge_header_value(RESP_HEAD, strlen(RESP_HEAD), "Content-Type", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("text/html", out);
}

static void test_header_value_case_insensitive_and_ows_trim()
{
    char out[64];
    // case-insensitive name; leading + trailing OWS on the value is stripped
    TEST_ASSERT_TRUE(edge_header_value(RESP_HEAD, strlen(RESP_HEAD), "cache-control", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("max-age=60", out);
}

static void test_header_value_absent_and_too_small()
{
    char out[64];
    TEST_ASSERT_FALSE(edge_header_value(RESP_HEAD, strlen(RESP_HEAD), "X-Missing", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out); // emptied on miss
    char tiny[4];
    // "text/html" does not fit 4 bytes -> refuse (never truncate a validator)
    TEST_ASSERT_FALSE(edge_header_value(RESP_HEAD, strlen(RESP_HEAD), "Content-Type", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_STRING("", tiny);
}

// --- HTTP-date parsing ---------------------------------------------------------------------------

static void test_http_date_all_three_formats()
{
    // RFC 9110 sec 5.6.7 worked example: all three encode 1994-11-06 08:49:37 UTC = 784111777.
    const int64_t expect = 784111777;
    const char *imf = "Sun, 06 Nov 1994 08:49:37 GMT";
    const char *r850 = "Sunday, 06-Nov-94 08:49:37 GMT";
    const char *asc = "Sun Nov  6 08:49:37 1994";
    TEST_ASSERT_EQUAL_INT64(expect, edge_parse_http_date(imf, strlen(imf)));
    TEST_ASSERT_EQUAL_INT64(expect, edge_parse_http_date(r850, strlen(r850)));
    TEST_ASSERT_EQUAL_INT64(expect, edge_parse_http_date(asc, strlen(asc)));
}

static void test_http_date_epoch_zero_and_invalid()
{
    const char *epoch = "Thu, 01 Jan 1970 00:00:00 GMT";
    TEST_ASSERT_EQUAL_INT64(0, edge_parse_http_date(epoch, strlen(epoch)));
    TEST_ASSERT_EQUAL_INT64(-1, edge_parse_http_date("not a date", 10));
    TEST_ASSERT_EQUAL_INT64(-1, edge_parse_http_date("Sun, 06 Zzz 1994 08:49:37 GMT", 29));
}

// --- freshness -----------------------------------------------------------------------------------

static void test_freshness_lifetime_precedence()
{
    DWSCacheControl cc;
    cache_control_init(&cc);
    cc.max_age = 100;
    cc.s_maxage = 50;
    TEST_ASSERT_EQUAL_INT32(50, edge_freshness_lifetime(&cc, /*shared=*/true, -1, -1));   // s-maxage wins (shared)
    TEST_ASSERT_EQUAL_INT32(100, edge_freshness_lifetime(&cc, /*shared=*/false, -1, -1)); // private ignores s-maxage

    DWSCacheControl empty;
    cache_control_init(&empty);
    TEST_ASSERT_EQUAL_INT32(100, edge_freshness_lifetime(&empty, true, 1000, 1100)); // Expires - Date
    TEST_ASSERT_EQUAL_INT32(-1, edge_freshness_lifetime(&empty, true, -1, -1));      // nothing explicit
}

static void test_heuristic_lifetime()
{
    TEST_ASSERT_EQUAL_INT32(100, edge_heuristic_lifetime(1000000, 1000000 - 1000)); // 10% of 1000
    TEST_ASSERT_EQUAL_INT32(-1, edge_heuristic_lifetime(-1, 5));                    // Date absent
    TEST_ASSERT_EQUAL_INT32(-1, edge_heuristic_lifetime(1000, 2000));               // Last-Modified newer than Date
}

static void test_initial_and_current_age()
{
    // no wall clock (response_time_epoch < 0) -> the Age header alone
    TEST_ASSERT_EQUAL_INT32(10, edge_initial_age(10, -1, -1));
    // with a clock: max(apparent = response-date, age header)
    TEST_ASSERT_EQUAL_INT32(50, edge_initial_age(10, 1000, 1050));
    TEST_ASSERT_EQUAL_INT32(0, edge_initial_age(-5, -1, -1)); // negative Age clamped

    TEST_ASSERT_EQUAL_INT32(15, edge_current_age(10, 1000, 6000)); // +5 s resident
    // monotonic wrap: insert near UINT32_MAX, now just past 0
    TEST_ASSERT_EQUAL_INT32(2, edge_current_age(0, 0xFFFFFC18u, 1000u)); // 2000 ms across the wrap
}

static void test_is_fresh()
{
    TEST_ASSERT_TRUE(edge_is_fresh_at(100, 50));
    TEST_ASSERT_FALSE(edge_is_fresh_at(100, 100)); // reached the lifetime
    TEST_ASSERT_FALSE(edge_is_fresh_at(-1, 0));    // no known lifetime
}

// --- cache key + digest --------------------------------------------------------------------------

static void test_key_canon()
{
    char out[128];
    size_t n = edge_key_canon("GET", "Example.COM", "/a/b", "x=1", /*include_query=*/true, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("GET\nexample.com\n/a/b\nx=1", out); // host lowercased
    TEST_ASSERT_EQUAL_UINT(strlen("GET\nexample.com\n/a/b\nx=1"), n);

    n = edge_key_canon("GET", "example.com", "/a/b", "x=1", /*include_query=*/false, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("GET\nexample.com\n/a/b", out); // query excluded

    char tiny[8];
    TEST_ASSERT_EQUAL_UINT(0, edge_key_canon("GET", "example.com", "/a/b", "", false, tiny, sizeof(tiny))); // overflow
}

static void test_key_digest_deterministic_and_distinct()
{
    const char *a = "GET\nexample.com\n/a";
    const char *b = "GET\nexample.com\n/b";
    uint8_t d1[32], d2[32], d3[32];
    edge_key_digest(a, strlen(a), d1);
    edge_key_digest(a, strlen(a), d2);
    edge_key_digest(b, strlen(b), d3);
    TEST_ASSERT_EQUAL_MEMORY(d1, d2, 32);      // deterministic
    TEST_ASSERT_TRUE(memcmp(d1, d3, 32) != 0); // distinct inputs -> distinct digests
}

// --- Vary secondary key --------------------------------------------------------------------------

// Mock request-header lookup: a tiny name->value table.
struct MockHdr
{
    const char *name;
    const char *value;
};
static const MockHdr *g_hdrs = nullptr;
static size_t g_hdr_count = 0;

static const char *mock_lookup(void *ctx, const char *name)
{
    (void)ctx;
    for (size_t i = 0; i < g_hdr_count; i++)
        if (strcasecmp(g_hdrs[i].name, name) == 0)
            return g_hdrs[i].value;
    return nullptr;
}

static void test_vary_serialize_match_and_differ()
{
    static const MockHdr gz_en[] = {{"accept-encoding", "gzip"}, {"accept-language", "en-US"}};
    static const MockHdr br_en[] = {{"accept-encoding", "br"}, {"accept-language", "en-US"}};

    char a[128], b[128];
    g_hdrs = gz_en;
    g_hdr_count = 2;
    TEST_ASSERT_TRUE(edge_vary_serialize("Accept-Encoding, Accept-Language", mock_lookup, nullptr, a, sizeof(a)));
    char a2[128];
    TEST_ASSERT_TRUE(edge_vary_serialize("Accept-Encoding, Accept-Language", mock_lookup, nullptr, a2, sizeof(a2)));
    TEST_ASSERT_EQUAL_STRING(a, a2); // same request values -> same key

    g_hdrs = br_en;
    TEST_ASSERT_TRUE(edge_vary_serialize("Accept-Encoding, Accept-Language", mock_lookup, nullptr, b, sizeof(b)));
    TEST_ASSERT_TRUE(strcmp(a, b) != 0); // different Accept-Encoding -> different key
}

static void test_vary_serialize_star_and_empty()
{
    char out[64];
    TEST_ASSERT_FALSE(edge_vary_serialize("*", mock_lookup, nullptr, out, sizeof(out)));    // uncacheable
    TEST_ASSERT_TRUE(edge_vary_serialize(nullptr, mock_lookup, nullptr, out, sizeof(out))); // no Vary
    TEST_ASSERT_EQUAL_STRING("", out);
}

// --- L1 store ------------------------------------------------------------------------------------

static EdgeCacheStore g_store;

static void test_store_alloc_lookup()
{
    edge_store_init(&g_store);
    EdgeEntry *e = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    TEST_ASSERT_NOT_NULL(e);
    e->status = 200;
    e->body_len = 3;
    memcpy(e->body, "abc", 3);
    TEST_ASSERT_EQUAL_PTR(e, edge_store_lookup(&g_store, "GET\nh\n/a", "", 100));
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/b", "", 100));  // different key
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/a", "x", 100)); // Vary mismatch
}

static void test_store_lru_evict()
{
    edge_store_init(&g_store);
    char key[32];
    for (int i = 0; i < DWS_EDGE_CACHE_SLOTS; i++)
    {
        snprintf(key, sizeof(key), "GET\nh\n/%d", i);
        TEST_ASSERT_NOT_NULL(edge_store_alloc(&g_store, key, ""));
    }
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/3", "", 1)); // touch /3 -> MRU (LRU is /0)
    TEST_ASSERT_NOT_NULL(edge_store_alloc(&g_store, "GET\nh\n/new", ""));   // evicts the true LRU = /0
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/0", "", 1));     // evicted
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/3", "", 1)); // survived (touched)
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/new", "", 1));
    TEST_ASSERT_EQUAL_UINT32(1, g_store.stats.evictions);
}

static void test_store_ttl_sweep()
{
    edge_store_init(&g_store);
    DWSCacheControl s10, s1000;
    cache_control_init(&s10);
    s10.max_age = 10;
    cache_control_init(&s1000);
    s1000.max_age = 1000;

    EdgeEntry *stale_noval = edge_store_alloc(&g_store, "GET\nh\n/x", "");
    edge_entry_set_freshness(stale_noval, &s10, true, -1, -1, -1, 0, -1, 1000);
    EdgeEntry *stale_val = edge_store_alloc(&g_store, "GET\nh\n/y", "");
    edge_entry_set_freshness(stale_val, &s10, true, -1, -1, -1, 0, -1, 1000);
    strcpy(stale_val->etag, "\"v\"");
    EdgeEntry *fresh = edge_store_alloc(&g_store, "GET\nh\n/z", "");
    edge_entry_set_freshness(fresh, &s1000, true, -1, -1, -1, 0, -1, 1000);

    // 20 s later: /x stale+no-validator (swept), /y stale+validator (kept), /z fresh (kept)
    TEST_ASSERT_EQUAL_UINT32(1, edge_store_sweep(&g_store, 21000));
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/x", "", 21000));
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/y", "", 21000));
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/z", "", 21000));
}

static void test_store_purge()
{
    edge_store_init(&g_store);
    edge_store_alloc(&g_store, "GET\nh\n/img/a", "");
    edge_store_alloc(&g_store, "GET\nh\n/img/b", "");
    edge_store_alloc(&g_store, "GET\nh\n/css/c", "");
    TEST_ASSERT_EQUAL_UINT32(1, edge_store_purge(&g_store, "GET\nh\n/img/a")); // single
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/img/a", "", 1));
    TEST_ASSERT_EQUAL_UINT32(1, edge_store_purge_prefix(&g_store, "/img/")); // prefix -> /img/b
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/img/b", "", 1));
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/css/c", "", 1)); // untouched
    TEST_ASSERT_EQUAL_UINT32(0, edge_store_purge(&g_store, "GET\nh\n/nope"));   // miss no-op
}

static void test_store_free_entry()
{
    edge_store_init(&g_store);
    EdgeEntry *a = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    edge_store_alloc(&g_store, "GET\nh\n/b", "");
    edge_store_free_entry(&g_store, a);
    TEST_ASSERT_NULL(edge_store_lookup(&g_store, "GET\nh\n/a", "", 1));
    TEST_ASSERT_NOT_NULL(edge_store_lookup(&g_store, "GET\nh\n/b", "", 1));
    EdgeEntry *c = edge_store_alloc(&g_store, "GET\nh\n/c", ""); // the freed slot is reusable
    TEST_ASSERT_EQUAL_PTR(a, c);
}

static void test_store_find_vary()
{
    static const MockHdr gz_hdr[] = {{"accept-encoding", "gzip"}};
    static const MockHdr br_hdr[] = {{"accept-encoding", "br"}};

    edge_store_init(&g_store);
    // two variants of the same resource, keyed by Accept-Encoding
    EdgeEntry *gz = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    strcpy(gz->vary_names, "Accept-Encoding");
    g_hdrs = gz_hdr;
    g_hdr_count = 1;
    edge_vary_serialize("Accept-Encoding", mock_lookup, nullptr, gz->vary_vals, sizeof(gz->vary_vals));

    EdgeEntry *br = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    strcpy(br->vary_names, "Accept-Encoding");
    g_hdrs = br_hdr;
    edge_vary_serialize("Accept-Encoding", mock_lookup, nullptr, br->vary_vals, sizeof(br->vary_vals));

    g_hdrs = gz_hdr; // a gzip request selects the gzip variant
    TEST_ASSERT_EQUAL_PTR(gz, edge_store_find(&g_store, "GET\nh\n/a", mock_lookup, nullptr, 1));
    g_hdrs = br_hdr; // a br request selects the br variant
    TEST_ASSERT_EQUAL_PTR(br, edge_store_find(&g_store, "GET\nh\n/a", mock_lookup, nullptr, 1));
    g_hdrs = nullptr; // identity (no Accept-Encoding) matches neither variant
    g_hdr_count = 0;
    TEST_ASSERT_NULL(edge_store_find(&g_store, "GET\nh\n/a", mock_lookup, nullptr, 1));

    // an entry with no Vary matches any request
    EdgeEntry *plain = edge_store_alloc(&g_store, "GET\nh\n/b", "");
    g_hdrs = gz_hdr;
    g_hdr_count = 1;
    TEST_ASSERT_EQUAL_PTR(plain, edge_store_find(&g_store, "GET\nh\n/b", mock_lookup, nullptr, 1));
}

static void test_entry_freshness_resolution()
{
    edge_store_init(&g_store);
    DWSCacheControl cc, empty;
    cache_control_init(&cc);
    cc.max_age = 100;
    cache_control_init(&empty);

    EdgeEntry *e = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    edge_entry_set_freshness(e, &cc, true, -1, -1, -1, 0, -1, 1000);
    TEST_ASSERT_EQUAL_INT32(100, (int32_t)e->lifetime_s);
    TEST_ASSERT_TRUE(edge_entry_fresh(e, 1000 + 99000));
    TEST_ASSERT_FALSE(edge_entry_fresh(e, 1000 + 101000));

    EdgeEntry *dflt = edge_store_alloc(&g_store, "GET\nh\n/b", "");
    edge_entry_set_freshness(dflt, &empty, true, -1, -1, -1, 0, -1, 1000); // no directive -> default TTL
    TEST_ASSERT_EQUAL_INT32(DWS_EDGE_DEFAULT_TTL_S, (int32_t)dflt->lifetime_s);

    EdgeEntry *heur = edge_store_alloc(&g_store, "GET\nh\n/c", "");
    edge_entry_set_freshness(heur, &empty, true, 1000000, -1, 1000000 - 1000, 0, -1, 1000); // 10% heuristic
    TEST_ASSERT_EQUAL_INT32(100, (int32_t)heur->lifetime_s);

    TEST_ASSERT_FALSE(edge_entry_has_validator(e));
    strcpy(e->last_modified, "Sun, 06 Nov 1994 08:49:37 GMT");
    TEST_ASSERT_TRUE(edge_entry_has_validator(e));
}

static void test_storeability()
{
    DWSCacheControl cc, ns, pv;
    cache_control_init(&cc);
    cache_control_init(&ns);
    ns.no_store = true;
    cache_control_init(&pv);
    pv.cc_private = true;

    TEST_ASSERT_TRUE(edge_is_storeable(200, "GET", &cc, nullptr, 100));
    TEST_ASSERT_TRUE(edge_is_storeable(200, "GET", nullptr, "Accept-Encoding", DWS_EDGE_BODY_MAX));
    TEST_ASSERT_FALSE(edge_is_storeable(200, "POST", &cc, nullptr, 100)); // not GET
    TEST_ASSERT_FALSE(edge_is_storeable(404, "GET", &cc, nullptr, 100));  // not 200
    TEST_ASSERT_FALSE(edge_is_storeable(200, "GET", &ns, nullptr, 100));  // no-store
    TEST_ASSERT_FALSE(edge_is_storeable(200, "GET", &pv, nullptr, 100));  // private
    TEST_ASSERT_FALSE(edge_is_storeable(200, "GET", &cc, "*", 100));      // Vary: *
    TEST_ASSERT_FALSE(edge_is_storeable(200, "GET", &cc, "Accept-Encoding, *", 100));
    TEST_ASSERT_FALSE(edge_is_storeable(200, "GET", &cc, nullptr, DWS_EDGE_BODY_MAX + 1)); // oversize
}

// --- conditional revalidation --------------------------------------------------------------------

static void test_build_conditional()
{
    edge_store_init(&g_store);
    EdgeEntry *e = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    strcpy(e->etag, "\"v1\"");
    strcpy(e->last_modified, "Sun, 06 Nov 1994 08:49:37 GMT");
    char out[128];
    size_t n = edge_build_conditional(e, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("If-None-Match: \"v1\"\r\nIf-Modified-Since: Sun, 06 Nov 1994 08:49:37 GMT\r\n", out);
    TEST_ASSERT_EQUAL_UINT(strlen(out), n);

    EdgeEntry *no_val = edge_store_alloc(&g_store, "GET\nh\n/b", "");
    TEST_ASSERT_EQUAL_UINT(0, edge_build_conditional(no_val, out, sizeof(out))); // no validator
}

static void test_apply_304()
{
    edge_store_init(&g_store);
    DWSCacheControl s10;
    cache_control_init(&s10);
    s10.max_age = 10;
    EdgeEntry *e = edge_store_alloc(&g_store, "GET\nh\n/a", "");
    e->status = 200;
    e->body_len = 3;
    memcpy(e->body, "abc", 3);
    strcpy(e->etag, "\"v1\"");
    edge_entry_set_freshness(e, &s10, true, -1, -1, -1, 0, -1, 1000);
    TEST_ASSERT_FALSE(edge_entry_fresh(e, 21000)); // stale 20 s later

    // origin answers the revalidation with 304 + a fresh max-age and a stronger validator
    const char *r304 = "HTTP/1.1 304 Not Modified\r\nETag: \"v2\"\r\nCache-Control: max-age=100\r\n\r\n";
    edge_apply_304(e, r304, strlen(r304), -1, 21000);
    TEST_ASSERT_TRUE(edge_entry_fresh(e, 21000));         // refreshed
    TEST_ASSERT_TRUE(edge_entry_fresh(e, 21000 + 99000)); // for the new lifetime
    TEST_ASSERT_EQUAL_STRING("\"v2\"", e->etag);          // validator adopted
    TEST_ASSERT_EQUAL_MEMORY("abc", e->body, 3);          // body kept
    TEST_ASSERT_EQUAL_INT32(100, (int32_t)e->lifetime_s);
}

// --- Range parsing (http_parse_byte_range) - the math behind serving 206 Partial Content from a cached
// body. Return: 0 = no usable range (full 200), 1 = satisfiable [start,end], -1 = unsatisfiable (416). ---

void test_range_explicit_and_open_ended()
{
    size_t s = 0;
    size_t e = 0;
    // bytes=A-B -> inclusive window.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=10-40", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(10, s);
    TEST_ASSERT_EQUAL_UINT(40, e);
    // bytes=A- -> A to the last byte.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=90-", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(90, s);
    TEST_ASSERT_EQUAL_UINT(99, e);
    // end past EOF clamps to the last byte.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=10-9999", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(10, s);
    TEST_ASSERT_EQUAL_UINT(99, e);
    // the whole body.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=0-99", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(0, s);
    TEST_ASSERT_EQUAL_UINT(99, e);
}

void test_range_suffix()
{
    size_t s = 0;
    size_t e = 0;
    // bytes=-N -> the last N bytes.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=-20", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(80, s);
    TEST_ASSERT_EQUAL_UINT(99, e);
    // N >= size -> the whole body.
    TEST_ASSERT_EQUAL_INT(1, http_parse_byte_range("bytes=-500", 100, &s, &e));
    TEST_ASSERT_EQUAL_UINT(0, s);
    TEST_ASSERT_EQUAL_UINT(99, e);
}

void test_range_unsatisfiable()
{
    size_t s = 0;
    size_t e = 0;
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=100-200", 100, &s, &e)); // start past EOF
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=50-10", 100, &s, &e));   // start > end
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=-0", 100, &s, &e));      // last 0 bytes
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=-", 100, &s, &e));       // "-" alone
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=0-0", 0, &s, &e));       // empty body
    // Overflow saturates (never wraps) -> resolves as past-EOF -> unsatisfiable.
    TEST_ASSERT_EQUAL_INT(-1, http_parse_byte_range("bytes=99999999999999999999-", 100, &s, &e));
}

void test_range_ignored_forms()
{
    size_t s = 0;
    size_t e = 0;
    TEST_ASSERT_EQUAL_INT(0, http_parse_byte_range(nullptr, 100, &s, &e));            // absent
    TEST_ASSERT_EQUAL_INT(0, http_parse_byte_range("bytes=0-10,20-30", 100, &s, &e)); // multi-range -> full 200
    TEST_ASSERT_EQUAL_INT(0, http_parse_byte_range("items=0-10", 100, &s, &e));       // wrong unit
    TEST_ASSERT_EQUAL_INT(0, http_parse_byte_range("bytes=10", 100, &s, &e));         // no dash -> malformed
    TEST_ASSERT_EQUAL_INT(0, http_parse_byte_range("bytes=10-40x", 100, &s, &e));     // trailing garbage
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_header_value_found);
    RUN_TEST(test_header_value_case_insensitive_and_ows_trim);
    RUN_TEST(test_header_value_absent_and_too_small);
    RUN_TEST(test_http_date_all_three_formats);
    RUN_TEST(test_http_date_epoch_zero_and_invalid);
    RUN_TEST(test_freshness_lifetime_precedence);
    RUN_TEST(test_heuristic_lifetime);
    RUN_TEST(test_initial_and_current_age);
    RUN_TEST(test_is_fresh);
    RUN_TEST(test_key_canon);
    RUN_TEST(test_key_digest_deterministic_and_distinct);
    RUN_TEST(test_vary_serialize_match_and_differ);
    RUN_TEST(test_vary_serialize_star_and_empty);
    RUN_TEST(test_store_alloc_lookup);
    RUN_TEST(test_store_lru_evict);
    RUN_TEST(test_store_ttl_sweep);
    RUN_TEST(test_store_purge);
    RUN_TEST(test_store_free_entry);
    RUN_TEST(test_store_find_vary);
    RUN_TEST(test_entry_freshness_resolution);
    RUN_TEST(test_storeability);
    RUN_TEST(test_build_conditional);
    RUN_TEST(test_apply_304);
    RUN_TEST(test_range_explicit_and_open_ended);
    RUN_TEST(test_range_suffix);
    RUN_TEST(test_range_unsatisfiable);
    RUN_TEST(test_range_ignored_forms);
    return UNITY_END();
}
