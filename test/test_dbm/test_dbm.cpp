// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/dbm: a log-structured hash KV over the WAL, exercised on a RAM-backed WalDev.
// "Reboot" is modeled by remounting a fresh WalStore + reopening the dbm over the same disk buffer.

#include "services/dbm/dbm.h"
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

// RAM-backed WalDev (same shape as the wal_store tests).
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

static uint8_t g_disk[64 * 1024];
static RamDisk g_d;
static WalDev g_dev;

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

// Fresh, formatted store + dbm over the shared disk.
static WalStore g_wal;
static DetwsDbm g_db;
static void fresh(void)
{
    g_d.buf = g_disk;
    g_d.size = sizeof(g_disk);
    g_dev = dev_over(&g_d);
    TEST_ASSERT_TRUE(wal_store_format(&g_wal, &g_dev));
    TEST_ASSERT_TRUE(detws_dbm_open(&g_db, &g_wal));
}
// Remount the store + reopen the dbm over the SAME disk bytes (a "reboot").
static bool reboot(void)
{
    g_dev = dev_over(&g_d);
    if (!wal_store_mount(&g_wal, &g_dev))
        return false;
    return detws_dbm_open(&g_db, &g_wal);
}

static bool put_s(const char *k, const char *v)
{
    return detws_dbm_put(&g_db, k, (uint16_t)strlen(k), (const uint8_t *)v, (uint32_t)strlen(v));
}
// Get and compare to expected string. Returns true if present and equal.
static bool get_eq(const char *k, const char *expect)
{
    uint8_t buf[DETWS_DBM_VAL_MAX];
    long n = detws_dbm_get(&g_db, k, (uint16_t)strlen(k), buf, sizeof(buf));
    if (n < 0)
        return false;
    return (size_t)n == strlen(expect) && memcmp(buf, expect, n) == 0;
}

void test_put_get_overwrite(void)
{
    fresh();
    TEST_ASSERT_TRUE(put_s("alpha", "one"));
    TEST_ASSERT_TRUE(put_s("beta", "two"));
    TEST_ASSERT_TRUE(get_eq("alpha", "one"));
    TEST_ASSERT_TRUE(get_eq("beta", "two"));
    TEST_ASSERT_EQUAL_UINT32(2, detws_dbm_count(&g_db));

    TEST_ASSERT_TRUE(put_s("alpha", "ONE-UPDATED")); // overwrite
    TEST_ASSERT_TRUE(get_eq("alpha", "ONE-UPDATED"));
    TEST_ASSERT_EQUAL_UINT32(2, detws_dbm_count(&g_db)); // still 2 live keys

    uint8_t b[8];
    TEST_ASSERT_EQUAL_INT(-1, detws_dbm_get(&g_db, "missing", 7, b, sizeof(b))); // absent
}

void test_delete_and_contains(void)
{
    fresh();
    put_s("k1", "v1");
    put_s("k2", "v2");
    TEST_ASSERT_TRUE(detws_dbm_contains(&g_db, "k1", 2));
    TEST_ASSERT_TRUE(detws_dbm_del(&g_db, "k1", 2));
    TEST_ASSERT_FALSE(detws_dbm_contains(&g_db, "k1", 2));
    TEST_ASSERT_FALSE(detws_dbm_del(&g_db, "k1", 2)); // already gone
    TEST_ASSERT_EQUAL_UINT32(1, detws_dbm_count(&g_db));

    // A tombstoned key can be re-inserted (resurrect through the tombstone slot).
    TEST_ASSERT_TRUE(put_s("k1", "again"));
    TEST_ASSERT_TRUE(get_eq("k1", "again"));
    TEST_ASSERT_EQUAL_UINT32(2, detws_dbm_count(&g_db));
}

void test_persist_across_reboot_with_checkpoint(void)
{
    fresh();
    put_s("name", "detws");
    put_s("role", "server");
    put_s("name", "detws2"); // overwrite before checkpoint
    TEST_ASSERT_TRUE(detws_dbm_sync(&g_db));

    TEST_ASSERT_TRUE(reboot());
    TEST_ASSERT_TRUE(get_eq("name", "detws2")); // latest value wins
    TEST_ASSERT_TRUE(get_eq("role", "server"));
    TEST_ASSERT_EQUAL_UINT32(2, detws_dbm_count(&g_db));
}

// Writes that were never checkpointed still recover, because the WAL tail-replays and the dbm replays it.
void test_persist_across_reboot_without_checkpoint(void)
{
    fresh();
    put_s("a", "1");
    put_s("b", "2");
    put_s("c", "3");
    // no sync/checkpoint
    TEST_ASSERT_TRUE(reboot());
    TEST_ASSERT_TRUE(get_eq("a", "1"));
    TEST_ASSERT_TRUE(get_eq("b", "2"));
    TEST_ASSERT_TRUE(get_eq("c", "3"));
    TEST_ASSERT_EQUAL_UINT32(3, detws_dbm_count(&g_db));
}

void test_delete_persists_across_reboot(void)
{
    fresh();
    put_s("keep", "y");
    put_s("drop", "n");
    TEST_ASSERT_TRUE(detws_dbm_del(&g_db, "drop", 4));
    TEST_ASSERT_TRUE(detws_dbm_sync(&g_db));

    TEST_ASSERT_TRUE(reboot());
    TEST_ASSERT_TRUE(detws_dbm_contains(&g_db, "keep", 4));
    TEST_ASSERT_FALSE(detws_dbm_contains(&g_db, "drop", 4)); // tombstone replayed
    TEST_ASSERT_EQUAL_UINT32(1, detws_dbm_count(&g_db));
}

// Many keys (forces hash collisions / probing) all retrievable and survive a reboot.
void test_many_keys_and_collisions(void)
{
    fresh();
    const int N = 100;
    char k[16], v[16];
    for (int i = 0; i < N; i++)
    {
        snprintf(k, sizeof(k), "key%04d", i);
        snprintf(v, sizeof(v), "val%d", i * 7);
        TEST_ASSERT_TRUE(put_s(k, v));
    }
    TEST_ASSERT_EQUAL_UINT32((uint32_t)N, detws_dbm_count(&g_db));
    TEST_ASSERT_TRUE(detws_dbm_sync(&g_db));
    TEST_ASSERT_TRUE(reboot());
    for (int i = 0; i < N; i++)
    {
        snprintf(k, sizeof(k), "key%04d", i);
        snprintf(v, sizeof(v), "val%d", i * 7);
        TEST_ASSERT_TRUE(get_eq(k, v));
    }
    TEST_ASSERT_EQUAL_UINT32((uint32_t)N, detws_dbm_count(&g_db));
}

void test_index_full_fails_closed(void)
{
    fresh();
    char k[16];
    // Fill every slot with a distinct live key.
    for (int i = 0; i < DETWS_DBM_SLOTS; i++)
    {
        snprintf(k, sizeof(k), "s%05d", i);
        TEST_ASSERT_TRUE(put_s(k, "x"));
    }
    TEST_ASSERT_EQUAL_UINT32((uint32_t)DETWS_DBM_SLOTS, detws_dbm_count(&g_db));
    // A brand-new key has no slot -> fail closed.
    TEST_ASSERT_FALSE(put_s("overflow-key", "x"));
    // But overwriting an existing key still works (no new slot needed).
    TEST_ASSERT_TRUE(put_s("s00000", "updated"));
    TEST_ASSERT_TRUE(get_eq("s00000", "updated"));
}

void test_bounds_and_empty_value(void)
{
    fresh();
    char bigk[DETWS_DBM_KEY_MAX + 2];
    memset(bigk, 'k', sizeof(bigk));
    TEST_ASSERT_FALSE(detws_dbm_put(&g_db, bigk, DETWS_DBM_KEY_MAX + 1, (const uint8_t *)"v", 1)); // key too long

    uint8_t bigv[DETWS_DBM_VAL_MAX + 1];
    memset(bigv, 0xAB, sizeof(bigv));
    TEST_ASSERT_FALSE(detws_dbm_put(&g_db, "k", 1, bigv, DETWS_DBM_VAL_MAX + 1)); // value too long

    // Empty value is valid: get returns 0, key is present.
    TEST_ASSERT_TRUE(detws_dbm_put(&g_db, "empty", 5, nullptr, 0));
    uint8_t b[4];
    TEST_ASSERT_EQUAL_INT(0, detws_dbm_get(&g_db, "empty", 5, b, sizeof(b)));
    TEST_ASSERT_TRUE(detws_dbm_contains(&g_db, "empty", 5));
}

void test_max_value_roundtrip(void)
{
    fresh();
    uint8_t val[DETWS_DBM_VAL_MAX];
    for (int i = 0; i < DETWS_DBM_VAL_MAX; i++)
        val[i] = (uint8_t)(i * 13 + 7);
    TEST_ASSERT_TRUE(detws_dbm_put(&g_db, "big", 3, val, DETWS_DBM_VAL_MAX));
    TEST_ASSERT_TRUE(detws_dbm_sync(&g_db));
    TEST_ASSERT_TRUE(reboot());
    uint8_t out[DETWS_DBM_VAL_MAX];
    TEST_ASSERT_EQUAL_INT(DETWS_DBM_VAL_MAX, detws_dbm_get(&g_db, "big", 3, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(val, out, DETWS_DBM_VAL_MAX);

    // A too-small buffer fails rather than truncating.
    uint8_t small[4];
    TEST_ASSERT_EQUAL_INT(-1, detws_dbm_get(&g_db, "big", 3, small, sizeof(small)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_put_get_overwrite);
    RUN_TEST(test_delete_and_contains);
    RUN_TEST(test_persist_across_reboot_with_checkpoint);
    RUN_TEST(test_persist_across_reboot_without_checkpoint);
    RUN_TEST(test_delete_persists_across_reboot);
    RUN_TEST(test_many_keys_and_collisions);
    RUN_TEST(test_index_full_fails_closed);
    RUN_TEST(test_bounds_and_empty_value);
    RUN_TEST(test_max_value_roundtrip);
    return UNITY_END();
}
