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

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_ARENA_H
