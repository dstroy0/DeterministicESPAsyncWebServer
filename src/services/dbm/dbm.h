// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dbm.h
 * @brief Log-structured hash key-value store on the WAL (DETWS_ENABLE_DBM, requires DETWS_ENABLE_WAL).
 *
 * A Bitcask-style key-value store: the value data lives append-only in the write-ahead log (wal_store.h)
 * and an in-RAM open-addressed hash index maps each live key to where its latest value sits in the log.
 * This is the design the measured SD envelope wants (docs/FEATURE_PERFORMANCE.md): every write is one of
 * the WAL's fast sequential appends, never a slow durable random write.
 *
 *  - **put / delete** append one record to the WAL and update the index. Writes are batched (unsynced);
 *    call ::detws_dbm_sync to checkpoint the WAL and make them durable.
 *  - **get** looks up the index and re-reads the value straight from the log (no per-key RAM copy).
 *  - **open** rebuilds the index by scanning the WAL, replaying puts and deletes in order, so the live
 *    key set is exactly what survived the last mount of the underlying store.
 *
 * The index is a fixed BSS array of ::DETWS_DBM_SLOTS slots (no heap); keys are bounded by
 * ::DETWS_DBM_KEY_MAX and values by ::DETWS_DBM_VAL_MAX. Everything fails closed at those bounds. Like the
 * other services, drive it from one context (a worker / loop), not concurrently.
 *
 * On-media record payload (inside a WAL record): `[op u8][key_len u16][val_len u32][key][value]` (LE),
 * op 0 = put, op 1 = delete (a tombstone, val_len 0).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DBM_H
#define DETERMINISTICESPASYNCWEBSERVER_DBM_H

#include "ServerConfig.h"

#if DETWS_ENABLE_DBM

#include "services/wal/wal_store.h"
#include <stddef.h>
#include <stdint.h>

/** @brief One in-RAM index slot. `state`: 0 empty, 1 live, 2 deleted (tombstone, still probed through). */
struct DetwsDbmSlot
{
    uint8_t state;
    uint16_t key_len;
    uint64_t hash;
    uint64_t val_off; ///< data-region offset of the value bytes in the WAL
    uint32_t val_len;
    char key[DETWS_DBM_KEY_MAX];
};

/** @brief A dbm handle bound to a mounted ::WalStore. Declare one (static for BSS); no heap. */
struct DetwsDbm
{
    WalStore *wal;
    uint32_t count; ///< live keys
    DetwsDbmSlot slots[DETWS_DBM_SLOTS];
};

/**
 * @brief Bind @p db to a mounted @p wal and rebuild the index by replaying the log.
 * @return false if the log holds more distinct live keys than ::DETWS_DBM_SLOTS (index would overflow).
 */
bool detws_dbm_open(DetwsDbm *db, WalStore *wal);

/**
 * @brief Insert or overwrite @p key -> @p val. Appends a WAL record and updates the index (not synced).
 * @return false if @p key_len > ::DETWS_DBM_KEY_MAX, @p val_len > ::DETWS_DBM_VAL_MAX, the index is full
 * (a new key with no free slot), or the WAL is full.
 */
bool detws_dbm_put(DetwsDbm *db, const char *key, uint16_t key_len, const uint8_t *val, uint32_t val_len);

/**
 * @brief Fetch @p key's value into @p buf (up to @p cap).
 * @return the value length on success, or -1 if the key is absent or the value is larger than @p cap.
 */
long detws_dbm_get(DetwsDbm *db, const char *key, uint16_t key_len, uint8_t *buf, size_t cap);

/**
 * @brief Delete @p key (appends a tombstone record and drops it from the index).
 * @return true if the key existed (and the tombstone was appended); false if absent or the WAL is full.
 */
bool detws_dbm_del(DetwsDbm *db, const char *key, uint16_t key_len);

/** @brief @return true if @p key is live. */
bool detws_dbm_contains(DetwsDbm *db, const char *key, uint16_t key_len);

/** @brief @return the number of live keys. */
uint32_t detws_dbm_count(DetwsDbm *db);

/** @brief Make all writes since the last sync durable (checkpoints the WAL). @return false on I/O failure. */
bool detws_dbm_sync(DetwsDbm *db);

/** @brief Per-key callback for ::detws_dbm_iterate; return false to stop early. The key bytes are not
 * NUL-terminated. Do not put/delete during iteration (it mutates the index). */
typedef bool (*DetwsDbmIterCb)(const char *key, uint16_t key_len, void *ctx);

/** @brief Visit every live key (unordered). @return the number of keys visited. */
uint32_t detws_dbm_iterate(DetwsDbm *db, DetwsDbmIterCb cb, void *ctx);

#endif // DETWS_ENABLE_DBM
#endif // DETERMINISTICESPASYNCWEBSERVER_DBM_H
