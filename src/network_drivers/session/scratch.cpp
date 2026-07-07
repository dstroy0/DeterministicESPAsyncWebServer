// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file scratch.cpp
 * @brief Per-worker per-dispatch scratch arenas - implementation.
 *
 * One bump allocator per worker (DETWS_WORKER_COUNT arenas in BSS), selected by
 * the caller's worker id. Each arena has exactly one accessor (its worker), so
 * the lock-free / fixed-size guarantees in scratch.h hold per worker. With
 * DETWS_WORKER_COUNT == 1 this is byte-for-byte the original single arena.
 */

#include "scratch.h"
#include "network_drivers/session/worker.h"
#include <assert.h>
#include <stdint.h>

#if defined(ARDUINO) && !defined(NDEBUG)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace
{
// All per-worker scratch state, owned by one instance (internal linkage): the bump arenas, the
// per-worker bump offsets, and the high-water marks. One named owner, unreachable cross-TU.
struct ScratchCtx
{
    // scratch_alloc aligns the bump OFFSET, so the arena base must itself be aligned to the
    // strictest alignment callers can request. As a struct member the base would otherwise only
    // inherit the struct's 8-byte (size_t) alignment; force 32 so a 16-/32-aligned request lands
    // on an aligned address (a standalone array got this incidentally from the linker before).
    alignas(32) uint8_t arena[DETWS_WORKER_COUNT][DETWS_SCRATCH_ARENA_SIZE]; // the arenas (BSS)
    size_t off[DETWS_WORKER_COUNT];                                          // bump offset per worker
    size_t high_water[DETWS_WORKER_COUNT];                                   // peak off per worker
};
ScratchCtx s_scratch;

// Default alignment when the caller passes 0: the strictest the platform needs.
constexpr size_t SCRATCH_DEFAULT_ALIGN = sizeof(void *) > 8 ? sizeof(void *) : 8;

// Resolve the calling worker, clamped into range so a stray id can never index
// out of bounds (an unbound caller is worker 0, the only worker by default).
inline int cur_worker()
{
    int w = detws_worker_self();
    return (w >= 0 && w < DETWS_WORKER_COUNT) ? w : 0;
}

// Debug tripwire: each arena must only ever be touched from one task (its
// worker). Record the first caller per arena and assert every later call
// matches. Compiled out on native (no FreeRTOS) and in NDEBUG builds; the
// structural single-accessor-per-arena invariant is what makes this lock-free,
// the assert just makes a future violation loud instead of a silent cross-core
// race.
inline void assert_single_owner(int w)
{
#if defined(ARDUINO) && !defined(NDEBUG)
    static TaskHandle_t s_owner[DETWS_WORKER_COUNT] = {nullptr};
    TaskHandle_t cur = xTaskGetCurrentTaskHandle();
    if (s_owner[w] == nullptr)
        s_owner[w] = cur;
    else
        assert(s_owner[w] == cur && "scratch arena borrowed from a foreign task");
#else
    (void)w;
#endif
}
} // namespace

void *scratch_alloc(size_t n, size_t align)
{
    int w = cur_worker();
    assert_single_owner(w);

    if (align == 0)
        align = SCRATCH_DEFAULT_ALIGN;
    assert((align & (align - 1)) == 0 && "scratch alignment must be a power of two");

    // Round the current offset up to the requested alignment.
    size_t base = (s_scratch.off[w] + (align - 1)) & ~(align - 1);

    // Overflow-safe capacity check: reject if n alone exceeds the arena, or if
    // the aligned allocation would run past the end. (n <= capacity here, so
    // capacity - n does not underflow; base may exceed it after rounding.)
    if (n > DETWS_SCRATCH_ARENA_SIZE || base > DETWS_SCRATCH_ARENA_SIZE - n)
        return nullptr;

    void *p = &s_scratch.arena[w][base];
    s_scratch.off[w] = base + n;
    if (s_scratch.off[w] > s_scratch.high_water[w])
        s_scratch.high_water[w] = s_scratch.off[w];
    return p;
}

void scratch_reset(void)
{
    int w = cur_worker();
    assert_single_owner(w);
    s_scratch.off[w] = 0;
}

size_t scratch_mark(void)
{
    int w = cur_worker();
    assert_single_owner(w);
    return s_scratch.off[w];
}

void scratch_release(size_t mark)
{
    int w = cur_worker();
    assert_single_owner(w);
    assert(mark <= s_scratch.off[w] && "scratch_release mark is above the current offset");
    s_scratch.off[w] = mark;
}

size_t scratch_used(void)
{
    return s_scratch.off[cur_worker()];
}

size_t scratch_high_water(void)
{
    // Peak any single arena reached - the value to size DETWS_SCRATCH_ARENA_SIZE by.
    size_t peak = 0;
    for (int w = 0; w < DETWS_WORKER_COUNT; w++)
        if (s_scratch.high_water[w] > peak)
            peak = s_scratch.high_water[w];
    return peak;
}

size_t scratch_capacity(void)
{
    return DETWS_SCRATCH_ARENA_SIZE;
}
