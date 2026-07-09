// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file wal_store.h
 * @brief Durable write-ahead store: A/B superblock + checkpoint over a block-device seam (DETWS_ENABLE_WAL).
 *
 * Increment 2 on top of the pure record codec in wal.h. It turns the codec into a mountable, power-loss-safe
 * append log on a fixed backing region (a preallocated file on any fs::FS - SD card or LittleFS - or any RAM
 * buffer for host tests). All I/O goes through a ::WalDev of three function pointers, so the superblock and
 * checkpoint logic is pure and fully host-testable over a RAM device.
 *
 * **On-media layout** of the backing region:
 *
 *     [ superblock A : WAL_SUPER_SIZE ][ superblock B : WAL_SUPER_SIZE ][ WAL data region ... ]
 *
 * Records (wal.h framing) are **appended sequentially** into the data region and are *not* synced per op -
 * that matches the measured envelope (docs/FEATURE_PERFORMANCE.md): batch, then checkpoint in bulk.
 *
 * **Checkpoint = the atomic commit.** ::wal_store_checkpoint syncs the appended data, then writes an updated
 * superblock (with an incremented generation, the new durable head, and the next seq) to the *inactive* of
 * the two copies and syncs that. Flipping which copy is newest is the single durable pointer move. If a
 * crash tears the new superblock, its CRC fails and mount falls back to the older copy; either way mount
 * then **replays the tail** past the committed head - each record is self-validating (CRC), so records
 * appended after the last checkpoint are still recovered, and recovery stops at the first torn record.
 *
 * So durability is layered: the superblock bounds how far mount must scan, and the per-record CRC provides
 * the actual atomicity. A crash costs at most the last, un-appended (torn) record.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_WAL_STORE_H
#define DETERMINISTICESPASYNCWEBSERVER_WAL_STORE_H

#include "ServerConfig.h"
#include "services/wal/wal.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_WAL

/** @brief Superblock magic ("WSB1", little-endian). */
#define WAL_SUPER_MAGIC 0x31425357u

/** @brief Reserved bytes per superblock copy (28 used + 4 CRC = 32; padded for headroom). */
#define WAL_SUPER_SIZE 64

/** @brief Byte offset where the WAL data region begins (past both superblock copies). */
#define WAL_DATA_OFFSET (2u * (uint32_t)WAL_SUPER_SIZE)

/**
 * @brief The block-device seam. Every store I/O is one of these three ops, so the store is host-testable
 * over a RAM buffer and binds to fs::FS (or anything else) with a thin adapter.
 *
 * @c read / @c write move exactly @p len bytes at absolute byte offset @p off and return the number of
 * bytes moved (== @p len on success; a short/zero return is a failure). @c sync is the durability barrier
 * (fsync / File::flush) and returns true on success. @c size is the total bytes of the backing region.
 */
typedef struct WalDev
{
    size_t (*read)(void *ctx, uint64_t off, uint8_t *buf, size_t len);
    size_t (*write)(void *ctx, uint64_t off, const uint8_t *buf, size_t len);
    bool (*sync)(void *ctx);
    void *ctx;
    uint64_t size;
} WalDev;

/** @brief A mounted durable WAL store. Treat fields as read-only; use the accessors below. */
typedef struct WalStore
{
    WalDev dev;
    uint64_t data_off;  ///< first byte of the data region (== ::WAL_DATA_OFFSET)
    uint64_t data_cap;  ///< usable data-region bytes (dev.size - data_off)
    uint64_t head;      ///< bytes used in the data region, including not-yet-checkpointed appends
    uint64_t committed; ///< head as of the last durable checkpoint
    uint64_t next_seq;  ///< sequence number the next appended record will carry
    uint64_t gen;       ///< generation of the currently newest superblock
    int ab;             ///< index (0/1) of the newest superblock copy; the next checkpoint writes 1 - ab
} WalStore;

/**
 * @brief Format @p dev into an empty store (writes a generation-1 superblock). Erases any existing log.
 * @return false if @p dev is too small to hold both superblocks plus a data region.
 */
bool wal_store_format(WalStore *s, const WalDev *dev);

/**
 * @brief Mount an existing store: pick the newest valid superblock, then replay the tail past its committed
 * head to recover records appended since the last checkpoint. @return false if neither superblock is valid
 * (unformatted / both torn).
 */
bool wal_store_mount(WalStore *s, const WalDev *dev);

/**
 * @brief Append one record (wal.h framing) at the current head. Does **not** sync - call ::wal_store_checkpoint
 * to make appends durable. @return false if the record does not fit the remaining data region (log full) or a
 * device write is short.
 */
bool wal_store_append(WalStore *s, const uint8_t *payload, uint32_t len);

/**
 * @brief Make every append so far durable and advance the committed head: sync data, write the next-generation
 * superblock to the inactive copy, sync it. @return false on a device write/sync failure.
 */
bool wal_store_checkpoint(WalStore *s);

/** @brief Bytes used in the data region (including appends not yet checkpointed). */
static inline uint64_t wal_store_used(const WalStore *s)
{
    return s->head;
}
/** @brief The durable committed head as of the last checkpoint. */
static inline uint64_t wal_store_committed(const WalStore *s)
{
    return s->committed;
}
/** @brief Total usable data-region capacity in bytes. */
static inline uint64_t wal_store_capacity(const WalStore *s)
{
    return s->data_cap;
}

#endif // DETWS_ENABLE_WAL
#endif // DETERMINISTICESPASYNCWEBSERVER_WAL_STORE_H
