// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/edge_cache/edge_cache_sd: the CDN edge cache's L2 SD-persistence tier over a
// RAM-backed WalDev + dbm. Covers the entry <-> dbm-value serialization roundtrip (incl. a max body, Vary
// variants, and binary bodies), the spill/promote/del/purge API, the L1 store's on_evict write-back hook,
// storeability gates (no-validator and oversize entries stay L1-only), reboot survival (remount + reopen),
// and that a shared dbm's foreign values are never touched.
//
// "Reboot" is modeled by remounting a fresh WalStore + reopening the dbm over the same disk buffer.

#include "services/dbm/dbm.h"
#include "services/edge_cache/edge_cache.h"
#include "services/edge_cache/edge_cache_sd.h"
#include "services/wal/wal_store.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

// --- RAM-backed WalDev (same shape as the dbm / dws_wal_store tests) ---------------------------------
struct RamDisk
{
    uint8_t *buf;
    uint64_t size;
};
static size_t ram_read(void *ctx, uint64_t off, uint8_t *buf, size_t len)
{
    RamDisk *d = (RamDisk *)ctx;
    if (off + len > d->size)
        return 0;
    memcpy(buf, d->buf + off, len);
    return len;
}
static size_t ram_write(void *ctx, uint64_t off, const uint8_t *buf, size_t len)
{
    RamDisk *d = (RamDisk *)ctx;
    if (off + len > d->size)
        return 0;
    memcpy(d->buf + off, buf, len);
    return len;
}
static bool ram_sync(void *)
{
    return true;
}

static uint8_t g_disk[128 * 1024];
static RamDisk g_d;
static WalDev g_dev;
static WalStore g_wal;
static DetwsDbm g_db;
static uint8_t g_scratch[EDGE_SD_VALUE_MAX];

static WalDev dev_over(RamDisk *d)
{
    WalDev v;
    v.read = ram_read;
    v.write = ram_write;
    v.sync = ram_sync;
    v.ctx = d;
    v.size = d->size;
    return v;
}
static void fresh(void)
{
    g_d.buf = g_disk;
    g_d.size = sizeof(g_disk);
    g_dev = dev_over(&g_d);
    TEST_ASSERT_TRUE(dws_wal_store_format(&g_wal, &g_dev));
    TEST_ASSERT_TRUE(dws_dbm_open(&g_db, &g_wal));
}
static bool reboot(void)
{
    g_dev = dev_over(&g_d);
    if (!dws_wal_store_mount(&g_wal, &g_dev))
        return false;
    return dws_dbm_open(&g_db, &g_wal);
}

// --- entry construction helpers ------------------------------------------------------------------
static void mkcanon(char *out, size_t cap, const char *path)
{
    snprintf(out, cap, "GET\nexample.com\n%s", path);
}

// Fill a bare entry with a validator + body. Not linked to any store (fields only).
static void fill_entry(EdgeEntry *e, const char *canon, const char *etag, const uint8_t *body, uint16_t body_len)
{
    memset(e, 0, sizeof(*e));
    strncpy(e->key, canon, sizeof(e->key) - 1);
    edge_key_digest(e->key, strlen(e->key), e->digest);
    e->status = 200;
    strncpy(e->content_type, "text/plain", sizeof(e->content_type) - 1);
    strncpy(e->etag, etag, sizeof(e->etag) - 1);
    if (body && body_len)
        memcpy(e->body, body, body_len);
    e->body_len = body_len;
}

static bool digest_eq(const uint8_t a[32], const uint8_t b[32])
{
    return memcmp(a, b, 32) == 0;
}

// --- serialization ------------------------------------------------------------------------------
void test_serialize_roundtrip_all_fields(void)
{
    EdgeEntry in;
    memset(&in, 0, sizeof(in));
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/img.png?w=64");
    strncpy(in.key, canon, sizeof(in.key) - 1);
    edge_key_digest(in.key, strlen(in.key), in.digest);
    in.status = 200;
    strncpy(in.content_type, "image/png", sizeof(in.content_type) - 1);
    strncpy(in.etag, "\"v1-abc\"", sizeof(in.etag) - 1);
    strncpy(in.last_modified, "Wed, 01 Jan 2025 00:00:00 GMT", sizeof(in.last_modified) - 1);
    strncpy(in.content_encoding, "gzip", sizeof(in.content_encoding) - 1);
    strncpy(in.vary_names, "Accept-Encoding", sizeof(in.vary_names) - 1);
    strncpy(in.vary_vals, "Accept-Encoding\x1egzip\x1f", sizeof(in.vary_vals) - 1);
    uint8_t body[300];
    for (int i = 0; i < 300; i++)
        body[i] = (uint8_t)(i * 7 + 3); // includes NUL bytes and high bytes
    memcpy(in.body, body, sizeof(body));
    in.body_len = sizeof(body);

    size_t n = edge_sd_serialize(&in, g_scratch, sizeof(g_scratch));
    TEST_ASSERT_TRUE(n > 0);

    EdgeEntry out;
    memset(&out, 0xEE, sizeof(out)); // poison, ensure deserialize writes what it should
    TEST_ASSERT_TRUE(edge_sd_deserialize(g_scratch, n, &out));

    TEST_ASSERT_EQUAL_STRING(in.key, out.key);
    TEST_ASSERT_EQUAL_INT(in.status, out.status);
    TEST_ASSERT_EQUAL_STRING(in.content_type, out.content_type);
    TEST_ASSERT_EQUAL_STRING(in.etag, out.etag);
    TEST_ASSERT_EQUAL_STRING(in.last_modified, out.last_modified);
    TEST_ASSERT_EQUAL_STRING(in.content_encoding, out.content_encoding);
    TEST_ASSERT_EQUAL_STRING(in.vary_names, out.vary_names);
    TEST_ASSERT_EQUAL_STRING(in.vary_vals, out.vary_vals);
    TEST_ASSERT_EQUAL_UINT16(in.body_len, out.body_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(in.body, out.body, in.body_len);
    TEST_ASSERT_TRUE(digest_eq(in.digest, out.digest)); // re-derived from the restored key
}

void test_serialize_max_body(void)
{
    EdgeEntry in;
    uint8_t body[DWS_EDGE_BODY_MAX];
    for (int i = 0; i < DWS_EDGE_BODY_MAX; i++)
        body[i] = (uint8_t)(i * 131 + 17);
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/big.bin");
    fill_entry(&in, canon, "\"big\"", body, DWS_EDGE_BODY_MAX);

    size_t n = edge_sd_serialize(&in, g_scratch, sizeof(g_scratch));
    TEST_ASSERT_TRUE(n > 0);
    EdgeEntry out;
    TEST_ASSERT_TRUE(edge_sd_deserialize(g_scratch, n, &out));
    TEST_ASSERT_EQUAL_UINT16(DWS_EDGE_BODY_MAX, out.body_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(body, out.body, DWS_EDGE_BODY_MAX);
}

void test_serialize_too_small_scratch_fails(void)
{
    EdgeEntry in;
    uint8_t body[300];
    memset(body, 'x', sizeof(body));
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/x");
    fill_entry(&in, canon, "\"e\"", body, sizeof(body));
    uint8_t tiny[16];
    TEST_ASSERT_EQUAL_UINT(0, edge_sd_serialize(&in, tiny, sizeof(tiny))); // won't fit -> 0
}

void test_deserialize_corrupt_fails_closed(void)
{
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/y");
    fill_entry(&in, canon, "\"e\"", (const uint8_t *)"hello", 5);
    size_t n = edge_sd_serialize(&in, g_scratch, sizeof(g_scratch));
    TEST_ASSERT_TRUE(n > 0);

    EdgeEntry out;
    uint8_t bad = g_scratch[0];
    g_scratch[0] = 0x42; // wrong version
    TEST_ASSERT_FALSE(edge_sd_deserialize(g_scratch, n, &out));
    g_scratch[0] = bad;
    TEST_ASSERT_FALSE(edge_sd_deserialize(g_scratch, 2, &out));     // truncated header
    TEST_ASSERT_FALSE(edge_sd_deserialize(g_scratch, n - 3, &out)); // truncated body
}

// --- put / get over dbm --------------------------------------------------------------------------
void test_put_get_roundtrip(void)
{
    fresh();
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/a.txt");
    fill_entry(&in, canon, "\"a1\"", (const uint8_t *)"payload-A", 9);
    TEST_ASSERT_TRUE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch)));

    EdgeEntry out;
    memset(&out, 0, sizeof(out));
    TEST_ASSERT_TRUE(edge_sd_get(&g_db, in.digest, &out, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_EQUAL_STRING(canon, out.key);
    TEST_ASSERT_EQUAL_STRING("\"a1\"", out.etag);
    TEST_ASSERT_EQUAL_UINT16(9, out.body_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("payload-A", out.body, 9);

    // A digest that was never stored misses.
    EdgeEntry in2;
    char c2[DWS_EDGE_KEY_MAX];
    mkcanon(c2, sizeof(c2), "/cdn/never");
    fill_entry(&in2, c2, "\"n\"", (const uint8_t *)"x", 1);
    TEST_ASSERT_FALSE(edge_sd_get(&g_db, in2.digest, &out, g_scratch, sizeof(g_scratch)));
}

void test_no_validator_not_spilled(void)
{
    fresh();
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/novalidator");
    fill_entry(&in, canon, "", (const uint8_t *)"body", 4); // no etag / last-modified
    in.last_modified[0] = '\0';
    TEST_ASSERT_FALSE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch))); // nothing to revalidate -> skip

    EdgeEntry out;
    TEST_ASSERT_FALSE(edge_sd_get(&g_db, in.digest, &out, g_scratch, sizeof(g_scratch)));
}

void test_oversize_body_stays_l1_only(void)
{
    fresh();
    // A body whose serialized size exceeds DWS_DBM_VAL_MAX must not be spilled (stays L1-only).
    EdgeEntry in;
    uint8_t body[DWS_EDGE_BODY_MAX];
    memset(body, 'Z', sizeof(body));
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/toobig");
    fill_entry(&in, canon, "\"big\"", body, DWS_EDGE_BODY_MAX);
    size_t serialized = edge_sd_serialize(&in, g_scratch, sizeof(g_scratch));
    TEST_ASSERT_TRUE(serialized > DWS_DBM_VAL_MAX); // the env sizes DBM_VAL_MAX below a full entry
    TEST_ASSERT_FALSE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch)));
}

// --- L1 on_evict write-back hook -----------------------------------------------------------------
static uint32_t g_spills = 0;
static void spill_cb(void *ctx, const EdgeEntry *v)
{
    (void)ctx;
    if (edge_sd_put((DetwsDbm *)ctx, v, g_scratch, sizeof(g_scratch)))
        g_spills++;
}

static EdgeEntry *store_mk(EdgeCacheStore *s, const char *path, const char *etag, const char *body)
{
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), path);
    EdgeEntry *e = edge_store_alloc(s, canon, "");
    TEST_ASSERT_NOT_NULL(e);
    e->status = 200;
    strncpy(e->content_type, "text/plain", sizeof(e->content_type) - 1);
    strncpy(e->etag, etag, sizeof(e->etag) - 1);
    size_t bl = strlen(body);
    memcpy(e->body, body, bl);
    e->body_len = (uint16_t)bl;
    return e;
}

void test_spill_on_evict_and_promote(void)
{
    fresh();
    g_spills = 0;
    EdgeCacheStore store;
    edge_store_init(&store);
    store.on_evict = spill_cb;
    store.evict_ctx = &g_db;

    // Fill every L1 slot, then one more: the LRU victim (the first inserted) is evicted -> spilled to L2.
    char first_canon[DWS_EDGE_KEY_MAX];
    mkcanon(first_canon, sizeof(first_canon), "/cdn/e0");
    uint8_t first_digest[32];
    edge_key_digest(first_canon, strlen(first_canon), first_digest);

    for (int i = 0; i < DWS_EDGE_CACHE_SLOTS; i++)
    {
        char path[24];
        snprintf(path, sizeof(path), "/cdn/e%d", i);
        char et[16];
        snprintf(et, sizeof(et), "\"e%d\"", i);
        store_mk(&store, path, et, "body");
    }
    TEST_ASSERT_EQUAL_UINT32(0, g_spills); // nothing evicted yet
    store_mk(&store, "/cdn/eN", "\"eN\"", "body");
    TEST_ASSERT_EQUAL_UINT32(1, g_spills); // the LRU victim spilled

    // The evicted entry is now promotable from L2; a still-resident one is not there.
    EdgeEntry out;
    TEST_ASSERT_TRUE(edge_sd_get(&g_db, first_digest, &out, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_EQUAL_STRING(first_canon, out.key);
    TEST_ASSERT_EQUAL_STRING("\"e0\"", out.etag);

    char last_canon[DWS_EDGE_KEY_MAX];
    mkcanon(last_canon, sizeof(last_canon), "/cdn/eN");
    uint8_t last_digest[32];
    edge_key_digest(last_canon, strlen(last_canon), last_digest);
    TEST_ASSERT_FALSE(edge_sd_get(&g_db, last_digest, &out, g_scratch, sizeof(g_scratch)));
}

void test_transient_entry_not_spilled(void)
{
    fresh();
    g_spills = 0;
    EdgeCacheStore store;
    edge_store_init(&store);
    store.on_evict = spill_cb;
    store.evict_ctx = &g_db;

    // Fill the store with transient (empty-key) entries; evicting one must NOT fire the write-back hook.
    for (int i = 0; i <= DWS_EDGE_CACHE_SLOTS; i++)
    {
        EdgeEntry *e = edge_store_alloc(&store, "", "");
        TEST_ASSERT_NOT_NULL(e);
        e->body_len = 4;
        memcpy(e->body, "data", 4);
    }
    TEST_ASSERT_EQUAL_UINT32(0, g_spills); // empty-key victims are never offered to on_evict
}

// --- reboot survival -----------------------------------------------------------------------------
void test_survives_reboot(void)
{
    fresh();
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/persist");
    fill_entry(&in, canon, "\"p9\"", (const uint8_t *)"survive-me", 10);
    strncpy(in.last_modified, "Wed, 01 Jan 2025 00:00:00 GMT", sizeof(in.last_modified) - 1);
    TEST_ASSERT_TRUE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_TRUE(dws_dbm_sync(&g_db));

    TEST_ASSERT_TRUE(reboot());
    EdgeEntry out;
    TEST_ASSERT_TRUE(edge_sd_get(&g_db, in.digest, &out, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_EQUAL_STRING(canon, out.key);
    TEST_ASSERT_EQUAL_STRING("\"p9\"", out.etag);
    TEST_ASSERT_EQUAL_STRING("Wed, 01 Jan 2025 00:00:00 GMT", out.last_modified);
    TEST_ASSERT_EQUAL_UINT16(10, out.body_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("survive-me", out.body, 10);
}

// --- del / purge ---------------------------------------------------------------------------------
void test_del(void)
{
    fresh();
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/del");
    fill_entry(&in, canon, "\"d\"", (const uint8_t *)"gone", 4);
    TEST_ASSERT_TRUE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_TRUE(edge_sd_del(&g_db, in.digest));
    EdgeEntry out;
    TEST_ASSERT_FALSE(edge_sd_get(&g_db, in.digest, &out, g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_FALSE(edge_sd_del(&g_db, in.digest)); // already gone
}

static void put_path(const char *path)
{
    EdgeEntry in;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), path);
    fill_entry(&in, canon, "\"v\"", (const uint8_t *)"x", 1);
    TEST_ASSERT_TRUE(edge_sd_put(&g_db, &in, g_scratch, sizeof(g_scratch)));
}
static bool has_path(const char *path)
{
    EdgeEntry in, out;
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), path);
    fill_entry(&in, canon, "\"v\"", (const uint8_t *)"x", 1);
    return edge_sd_get(&g_db, in.digest, &out, g_scratch, sizeof(g_scratch));
}

void test_purge_prefix(void)
{
    fresh();
    put_path("/cdn/a");
    put_path("/cdn/b");
    put_path("/other/c");
    TEST_ASSERT_EQUAL_UINT32(2, edge_sd_purge_prefix(&g_db, "/cdn/", g_scratch, sizeof(g_scratch)));
    TEST_ASSERT_FALSE(has_path("/cdn/a"));
    TEST_ASSERT_FALSE(has_path("/cdn/b"));
    TEST_ASSERT_TRUE(has_path("/other/c")); // outside the prefix, untouched
}

void test_purge_prefix_multipass(void)
{
    fresh();
    // More than the internal batch size, to exercise the collect-delete-reiterate loop.
    const int N = 20;
    char path[24];
    for (int i = 0; i < N; i++)
    {
        snprintf(path, sizeof(path), "/cdn/p%d", i);
        put_path(path);
    }
    put_path("/keep/one");
    TEST_ASSERT_EQUAL_UINT32((uint32_t)N, edge_sd_purge_prefix(&g_db, "/cdn/", g_scratch, sizeof(g_scratch)));
    for (int i = 0; i < N; i++)
    {
        snprintf(path, sizeof(path), "/cdn/p%d", i);
        TEST_ASSERT_FALSE(has_path(path));
    }
    TEST_ASSERT_TRUE(has_path("/keep/one"));
}

void test_purge_all(void)
{
    fresh();
    put_path("/cdn/a");
    put_path("/cdn/b");
    put_path("/x/y");
    TEST_ASSERT_EQUAL_UINT32(3, edge_sd_purge_all(&g_db));
    TEST_ASSERT_FALSE(has_path("/cdn/a"));
    TEST_ASSERT_FALSE(has_path("/x/y"));
}

void test_shared_dbm_foreign_value_untouched(void)
{
    fresh();
    // A foreign (non-edge) value under a 32-byte key: purge_all must skip it (peek_canon guards on version).
    uint8_t foreign_key[32];
    memset(foreign_key, 0xA5, sizeof(foreign_key));
    uint8_t foreign_val[16];
    memset(foreign_val, 0xFF, sizeof(foreign_val)); // first byte 0xFF != edge version 1
    TEST_ASSERT_TRUE(dws_dbm_put(&g_db, (const char *)foreign_key, 32, foreign_val, sizeof(foreign_val)));
    put_path("/cdn/mine");

    TEST_ASSERT_EQUAL_UINT32(1, edge_sd_purge_all(&g_db)); // only the edge value
    TEST_ASSERT_FALSE(has_path("/cdn/mine"));
    uint8_t out[16];
    TEST_ASSERT_EQUAL_INT(16, dws_dbm_get(&g_db, (const char *)foreign_key, 32, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(foreign_val, out, 16); // foreign value intact
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_serialize_roundtrip_all_fields);
    RUN_TEST(test_serialize_max_body);
    RUN_TEST(test_serialize_too_small_scratch_fails);
    RUN_TEST(test_deserialize_corrupt_fails_closed);
    RUN_TEST(test_put_get_roundtrip);
    RUN_TEST(test_no_validator_not_spilled);
    RUN_TEST(test_oversize_body_stays_l1_only);
    RUN_TEST(test_spill_on_evict_and_promote);
    RUN_TEST(test_transient_entry_not_spilled);
    RUN_TEST(test_survives_reboot);
    RUN_TEST(test_del);
    RUN_TEST(test_purge_prefix);
    RUN_TEST(test_purge_prefix_multipass);
    RUN_TEST(test_purge_all);
    RUN_TEST(test_shared_dbm_foreign_value_untouched);
    return UNITY_END();
}
