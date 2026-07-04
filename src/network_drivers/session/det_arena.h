// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_arena.h
 * @brief Unified double-ended server arena (Phase 1: core allocator, one region).
 *
 * One contiguous region is shared by two allocators that grow toward each other, with
 * the free space floating in the middle:
 *
 *   [ persistent  --grows up-->        | free |        <--grows down--  scratch ]
 *     low addr                    (floating boundary)                   high addr
 *
 * - **Persistent** (bottom): a first-fit free-list. Long-lived objects that are freed
 *   individually, in arbitrary order (e.g. per-connection state). Grows up into the
 *   middle only as far as the scratch end allows; a freed top block shrinks it back.
 * - **Scratch** (top): a bump allocator reclaimed in bulk. Transient per-dispatch
 *   buffers. `scratch_reset()` empties it in O(1); `mark`/`release` give nested savepoints.
 *
 * Whichever side needs more room grows into the shared middle - that is the win over two
 * fixed pools. Both ends fail closed (return NULL) rather than crossing the boundary.
 *
 * All state lives in ::DetArena (no globals), so it is unit-testable and can back several
 * arenas (a DRAM base and a PSRAM extension, in a later phase). No heap; no stdlib.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_ARENA_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_ARENA_H

#include <stddef.h>
#include <stdint.h>

/** @brief Alignment (bytes) applied to every arena allocation and to headers. */
#define DET_ARENA_ALIGN 8u

/**
 * @brief Double-ended arena over one region `[base, base+size)`.
 *
 * `persist_end` and `scratch_top` are byte offsets from `base`; the free middle is
 * `[persist_end, scratch_top)`. The persistent pool owns `[0, persist_end)` (a chain of
 * first-fit blocks); scratch owns `[scratch_top, size)` (bump). Initialize with
 * det_arena_init(); do not touch the fields directly.
 */
typedef struct
{
    uint8_t *base;       ///< Region start.
    size_t size;         ///< Region length in bytes.
    size_t persist_end;  ///< Persistent pool occupies [0, persist_end).
    size_t scratch_top;  ///< Scratch occupies [scratch_top, size).
    size_t persist_used; ///< Bytes currently handed out by the persistent pool (payload).
    size_t persist_hw;   ///< High-water of persist_end (for sizing).
    size_t scratch_hw;   ///< High-water of scratch use (size - min scratch_top).
} DetArena;

/**
 * @brief Initialize @p a over the region `[base, base+size)`.
 *
 * @p base should be DET_ARENA_ALIGN-aligned; @p size must be at least a few blocks.
 */
void det_arena_init(DetArena *a, void *base, size_t size);

// --- persistent end (first-fit, individual free, grows up) ------------------

/**
 * @brief Allocate @p n zero-initialized bytes of long-lived storage.
 *
 * First-fits an existing free block; otherwise carves a new block from the free middle
 * (growing the persistent end up) if it will not cross the scratch end.
 * @return aligned, zeroed pointer, or NULL if the arena cannot satisfy it.
 */
void *det_arena_persist_alloc(DetArena *a, size_t n);

/**
 * @brief Free a pointer previously returned by det_arena_persist_alloc().
 *
 * Coalesces with adjacent free blocks; if the freed block sits at the top of the
 * persistent region it returns that space to the free middle (shrinks the boundary).
 * Passing NULL is a no-op.
 */
void det_arena_persist_free(DetArena *a, void *p);

// --- scratch end (bump, bulk reset, grows down) -----------------------------

/**
 * @brief Bump-allocate @p n bytes of transient storage (valid until the next reset/release).
 * @return aligned pointer (NOT zeroed), or NULL if it would cross the persistent end.
 */
void *det_arena_scratch_alloc(DetArena *a, size_t n);

/** @brief Capture the current scratch position (a savepoint for det_arena_scratch_release()). */
size_t det_arena_scratch_mark(const DetArena *a);

/** @brief Free every scratch allocation made since @p mark (a value from det_arena_scratch_mark()). */
void det_arena_scratch_release(DetArena *a, size_t mark);

/** @brief Free ALL scratch allocations in O(1). */
void det_arena_scratch_reset(DetArena *a);

// --- observability ----------------------------------------------------------

/** @brief Free bytes in the middle (max a single new allocation could take, minus a header). */
size_t det_arena_free_bytes(const DetArena *a);

/** @brief Persistent payload bytes currently allocated. */
size_t det_arena_persist_used(const DetArena *a);

/** @brief Scratch bytes currently allocated. */
size_t det_arena_scratch_used(const DetArena *a);

// ===========================================================================
// Multi-region extension: a DRAM base + an optional PSRAM extension.
// ===========================================================================
//
// A ::DetArenaSet chains a few ::DetArena regions in preference order (add DRAM
// first, PSRAM second). Allocations try each region in turn and take the first
// that fits, so hot state stays in fast internal RAM and only the overflow
// spills into external RAM. Frees are routed to the owning region by address.
// This is how "arena extension" works: enable PSRAM by adding a second region;
// leave it out and the set is just the single DRAM arena.

/** @brief Max regions in a ::DetArenaSet (DRAM base + PSRAM extension). */
#ifndef DET_ARENA_MAX_REGIONS
#define DET_ARENA_MAX_REGIONS 2u
#endif

/** @brief A set of ::DetArena regions searched in insertion (preference) order. */
typedef struct
{
    DetArena region[DET_ARENA_MAX_REGIONS];
    size_t count; ///< Regions in use.
} DetArenaSet;

/** @brief A scratch savepoint across every region of a ::DetArenaSet. */
typedef struct
{
    size_t top[DET_ARENA_MAX_REGIONS];
    size_t count;
} DetArenaMark;

/** @brief Initialize an empty set (no regions yet). */
void det_arena_set_init(DetArenaSet *s);

/**
 * @brief Add a region `[base, base+size)`; regions are searched in the order added.
 * @return true if added, false if the set is full or the region is too small.
 */
bool det_arena_set_add(DetArenaSet *s, void *base, size_t size);

/** @brief Persistent alloc from the first region that fits (see det_arena_persist_alloc()). */
void *det_arena_set_persist_alloc(DetArenaSet *s, size_t n);

/** @brief Free a persistent pointer, routed to its owning region by address. */
void det_arena_set_persist_free(DetArenaSet *s, void *p);

/** @brief Scratch alloc from the first region that fits (see det_arena_scratch_alloc()). */
void *det_arena_set_scratch_alloc(DetArenaSet *s, size_t n);

/** @brief Capture the scratch position of every region. */
DetArenaMark det_arena_set_scratch_mark(const DetArenaSet *s);

/** @brief Restore every region's scratch position to @p m (frees scratch made since). */
void det_arena_set_scratch_release(DetArenaSet *s, const DetArenaMark *m);

/** @brief Reset scratch in every region. */
void det_arena_set_scratch_reset(DetArenaSet *s);

/** @brief Total free middle bytes summed over all regions. */
size_t det_arena_set_free_bytes(const DetArenaSet *s);

/** @brief Persistent payload bytes allocated, summed over all regions. */
size_t det_arena_set_persist_used(const DetArenaSet *s);

/** @brief Scratch bytes allocated, summed over all regions. */
size_t det_arena_set_scratch_used(const DetArenaSet *s);

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_ARENA_H
