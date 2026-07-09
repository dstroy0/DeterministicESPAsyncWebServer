// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/wal wal_store: A/B superblock + checkpoint + mount/recover over a RAM device.
// A crash is modeled as: stop touching the store, then mount() a fresh WalStore over the same buffer.

#include "services/wal/wal_store.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

// A RAM-backed WalDev: the "flash" is just a byte buffer; sync is a no-op that succeeds.
struct RamDisk
{
    uint8_t *buf;
    uint64_t size;
    int syncs;
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
static bool ram_sync(void *ctx)
{
    ((RamDisk *)ctx)->syncs++;
    return true;
}

static WalDev make_dev(RamDisk *d)
{
    WalDev dev;
    dev.read = ram_read;
    dev.write = ram_write;
    dev.sync = ram_sync;
    dev.ctx = d;
    dev.size = d->size;
    return dev;
}

// Shared backing buffer big enough for both superblocks + a comfortable data region.
static uint8_t g_disk[4096];

static const uint32_t REC = (uint32_t)WAL_RECORD_HEADER; // header-only record size base

void test_format_then_mount_empty(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev));
    TEST_ASSERT_EQUAL_UINT64(0, wal_store_used(&s));
    TEST_ASSERT_EQUAL_UINT64(0, wal_store_committed(&s));

    WalStore m;
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(0, wal_store_used(&m));
    TEST_ASSERT_EQUAL_UINT64(0, wal_store_committed(&m));
}

void test_mount_unformatted_fails(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    memset(g_disk, 0xAB, sizeof(g_disk)); // no valid superblock anywhere
    WalDev dev = make_dev(&d);
    WalStore m;
    TEST_ASSERT_FALSE(wal_store_mount(&m, &dev));
}

// Records appended but never checkpointed must still be recovered by the tail replay (crash-before-commit).
void test_append_without_checkpoint_recovers_via_tail(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"alpha", 5));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"bravo", 5));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"c", 1));
    uint64_t expect = (REC + 5) + (REC + 5) + (REC + 1);
    TEST_ASSERT_EQUAL_UINT64(expect, wal_store_used(&s));

    WalStore m; // "reboot"
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(expect, wal_store_used(&m)); // all three recovered
    TEST_ASSERT_EQUAL_UINT64(0, wal_store_committed(&m)); // but none were checkpointed
}

// Checkpoint commits the head; a further append past it recovers on top of the committed head.
void test_checkpoint_commits_then_tail(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"one", 3));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"two", 3));
    TEST_ASSERT_TRUE(wal_store_checkpoint(&s));
    uint64_t committed = 2 * (REC + 3);
    TEST_ASSERT_EQUAL_UINT64(committed, wal_store_committed(&s));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"three", 5)); // not checkpointed

    WalStore m;
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(committed, wal_store_committed(&m));      // durable pointer
    TEST_ASSERT_EQUAL_UINT64(committed + REC + 5, wal_store_used(&m)); // + the replayed 3rd
}

// A torn record after the checkpoint is discarded; mount recovers to the last good record.
void test_torn_tail_recovers_to_last_good(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"one", 3));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"two", 3));
    TEST_ASSERT_TRUE(wal_store_checkpoint(&s));
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"three", 5));
    uint64_t good = 2 * (REC + 3);

    // Corrupt a payload byte of the 3rd record on media -> its CRC now fails.
    g_disk[WAL_DATA_OFFSET + good + WAL_RECORD_HEADER + 1] ^= 0xFF;

    WalStore m;
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(good, wal_store_used(&m)); // torn 3rd dropped
}

// Two checkpoints make B the newest superblock; wiping B must fall back to A and still recover correctly.
void test_ab_superblock_fallback(void)
{
    RamDisk d = {g_disk, sizeof(g_disk), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev)); // super in copy A
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"one", 3));
    TEST_ASSERT_TRUE(wal_store_checkpoint(&s)); // writes copy B (gen 2)
    TEST_ASSERT_TRUE(wal_store_append(&s, (const uint8_t *)"two", 3));
    TEST_ASSERT_TRUE(wal_store_checkpoint(&s)); // writes copy A (gen 3) - A now newest
    uint64_t committed = 2 * (REC + 3);
    TEST_ASSERT_EQUAL_INT(0, s.ab); // newest is copy A

    // Tear the *newest* superblock (copy A). Mount must fall back to copy B (gen 2, committed after 1 rec)
    // and then replay the tail to recover the 2nd record anyway.
    memset(g_disk + 0 * WAL_SUPER_SIZE, 0xFF, WAL_SUPER_SIZE);

    WalStore m;
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(REC + 3, wal_store_committed(&m)); // fell back to B's committed head
    TEST_ASSERT_EQUAL_UINT64(committed, wal_store_used(&m));    // but recovered both via tail replay
}

// Appends fail closed when the data region is full; head never exceeds capacity.
void test_append_full_fails_closed(void)
{
    static uint8_t tiny[WAL_DATA_OFFSET + 100];
    RamDisk d = {tiny, sizeof(tiny), 0};
    WalDev dev = make_dev(&d);
    WalStore s;
    TEST_ASSERT_TRUE(wal_store_format(&s, &dev));
    TEST_ASSERT_EQUAL_UINT64(100, wal_store_capacity(&s));

    // Each header-only (len=0) record is WAL_RECORD_HEADER=20 bytes: 5 fit in 100, the 6th must fail.
    int ok = 0;
    for (int i = 0; i < 10; i++)
        if (wal_store_append(&s, nullptr, 0))
            ok++;
    TEST_ASSERT_EQUAL_INT(5, ok);
    TEST_ASSERT_TRUE(wal_store_used(&s) <= wal_store_capacity(&s));

    // The full log still mounts back to exactly what fit.
    WalStore m;
    TEST_ASSERT_TRUE(wal_store_mount(&m, &dev));
    TEST_ASSERT_EQUAL_UINT64(5 * REC, wal_store_used(&m));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_format_then_mount_empty);
    RUN_TEST(test_mount_unformatted_fails);
    RUN_TEST(test_append_without_checkpoint_recovers_via_tail);
    RUN_TEST(test_checkpoint_commits_then_tail);
    RUN_TEST(test_torn_tail_recovers_to_last_good);
    RUN_TEST(test_ab_superblock_fallback);
    RUN_TEST(test_append_full_fails_closed);
    return UNITY_END();
}
