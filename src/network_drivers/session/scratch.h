// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file scratch.h
 * @brief Shared per-dispatch scratch arena (Layer 5, session-scoped memory).
 *
 * One fixed BSS arena that codec / protocol handlers borrow transient working
 * memory from, instead of each feature carrying its own dedicated scratch
 * buffer. Many such buffers are mutually exclusive in time - a connection is
 * doing HTTP *or* WebSocket *or* SSH at any instant, and the session layer runs
 * one event to completion before the next - so overlapping them in a single
 * arena cuts peak DRAM without weakening the zero-heap / deterministic
 * guarantee (fixed size, no runtime growth).
 *
 * **Model - region reset per dispatch.** scratch_alloc() bump-allocates from the
 * arena; scratch_reset() empties it in O(1). server_tick() calls scratch_reset()
 * at the top of every event dispatch, so a borrow is valid only until the
 * handler returns. There is no per-allocation free - the whole arena is
 * reclaimed at once.
 *
 * **Race-safety.** All codec / protocol logic runs only in the single loop task
 * (server_tick() / handle()); the lwIP callbacks run in tcpip_thread and only
 * fill the rx ring + enqueue events - they never borrow scratch. The arena
 * therefore has exactly one accessor and needs no lock. In debug builds an
 * owner-task assertion (xTaskGetCurrentTaskHandle) fails loud the instant any
 * other context calls in, turning a future mistake into an immediate visible
 * failure instead of a silent cross-core race.
 *
 * **Exhaustion-safety.** Borrows live only within one dispatch and are
 * auto-reclaimed by the reset, so a forgotten free cannot accumulate (no
 * creeping exhaustion). An over-budget scratch_alloc() returns nullptr; every
 * caller must take a defined fail-closed path (drop the optional optimization,
 * close the connection, answer 503) and must never dereference a null borrow.
 *
 * **No implicit zeroing.** scratch_alloc() returns uninitialized memory and the
 * reset does not wipe; a security-sensitive tenant must wipe its own region
 * after use (as the SSH crypto scratch already does).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SCRATCH_H
#define DETERMINISTICESPASYNCWEBSERVER_SCRATCH_H

#include "DetWebServerConfig.h"
#include <stddef.h>

/**
 * @brief Borrow @p n bytes of scratch, aligned to @p align.
 *
 * The returned pointer is valid only until the next scratch_reset() (i.e. only
 * within the current session dispatch). Returns nullptr if the request does not
 * fit the remaining arena - callers MUST handle null and fail closed.
 *
 * @param n     bytes requested (0 yields a valid non-null pointer when space
 *              remains).
 * @param align required alignment in bytes, a power of two (0 selects the
 *              platform default).
 * @return pointer to @p n writable bytes, or nullptr if it does not fit.
 */
void *scratch_alloc(size_t n, size_t align);

/**
 * @brief Reclaim the whole arena (empties it).
 *
 * Called by server_tick() before each event dispatch. Invalidates every pointer
 * previously returned by scratch_alloc().
 */
void scratch_reset(void);

/**
 * @brief Capture the current arena offset (a savepoint for scratch_release()).
 * @return an opaque mark to pass to scratch_release().
 */
size_t scratch_mark(void);

/**
 * @brief Reclaim everything allocated since @p mark (LIFO).
 *
 * Restores the arena to a previous scratch_mark(), freeing every scratch_alloc()
 * made in between. Marks must be released in reverse order (nested scopes). Use
 * ScratchScope for return-safe scoping.
 *
 * @param mark a value previously returned by scratch_mark() (must be <= the
 *             current offset).
 */
void scratch_release(size_t mark);

/** @brief Bytes currently handed out (0 immediately after a reset). */
size_t scratch_used(void);

/** @brief Largest scratch_used() value seen since boot (for sizing the arena). */
size_t scratch_high_water(void);

/** @brief Total arena capacity in bytes (DETWS_SCRATCH_ARENA_SIZE). */
size_t scratch_capacity(void);

/**
 * @brief RAII scope guard for transient scratch borrows.
 *
 * Marks the arena on construction and restores it on destruction, so every
 * scratch_alloc() made within the scope is reclaimed on *every* exit path
 * (return, break, fall-through) - the safe way to borrow transient scratch in a
 * function with multiple returns. Scopes must nest (LIFO); the per-dispatch
 * scratch_reset() is the backstop if one is ever skipped.
 */
class ScratchScope
{
  public:
    ScratchScope() : m_mark(scratch_mark())
    {
    }
    ~ScratchScope()
    {
        scratch_release(m_mark);
    }
    ScratchScope(const ScratchScope &) = delete;
    ScratchScope &operator=(const ScratchScope &) = delete;

  private:
    size_t m_mark;
};

#endif // DETERMINISTICESPASYNCWEBSERVER_SCRATCH_H
