// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file scratch.cpp
 * @brief Shared per-dispatch scratch arena - implementation.
 *
 * A bump allocator over one fixed BSS buffer. See scratch.h for the model and
 * the race / exhaustion safety argument.
 */

#include "scratch.h"
#include <assert.h>
#include <stdint.h>

#if defined(ARDUINO) && !defined(NDEBUG)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace
{
uint8_t s_arena[DETWS_SCRATCH_ARENA_SIZE]; // the arena (BSS)
size_t s_off;                              // bytes currently handed out (bump offset)
size_t s_high_water;                       // peak s_off since boot

// Default alignment when the caller passes 0: the strictest the platform needs.
constexpr size_t SCRATCH_DEFAULT_ALIGN = sizeof(void *) > 8 ? sizeof(void *) : 8;

// Debug tripwire: the arena must only ever be touched from the single session
// loop task. Record the first caller's task and assert every later call matches.
// Compiled out on native (no FreeRTOS) and in NDEBUG builds; the structural
// single-accessor invariant is what makes this lock-free, the assert just makes
// a future violation loud instead of a silent cross-core race.
inline void assert_single_owner()
{
#if defined(ARDUINO) && !defined(NDEBUG)
    static TaskHandle_t s_owner = nullptr;
    TaskHandle_t cur = xTaskGetCurrentTaskHandle();
    if (s_owner == nullptr)
        s_owner = cur;
    else
        assert(s_owner == cur && "scratch arena borrowed from a foreign task");
#endif
}
} // namespace

void *scratch_alloc(size_t n, size_t align)
{
    assert_single_owner();

    if (align == 0)
        align = SCRATCH_DEFAULT_ALIGN;
    assert((align & (align - 1)) == 0 && "scratch alignment must be a power of two");

    // Round the current offset up to the requested alignment.
    size_t base = (s_off + (align - 1)) & ~(align - 1);

    // Overflow-safe capacity check: reject if n alone exceeds the arena, or if
    // the aligned allocation would run past the end. (n <= capacity here, so
    // capacity - n does not underflow; base may exceed it after rounding.)
    if (n > DETWS_SCRATCH_ARENA_SIZE || base > DETWS_SCRATCH_ARENA_SIZE - n)
        return nullptr;

    void *p = &s_arena[base];
    s_off = base + n;
    if (s_off > s_high_water)
        s_high_water = s_off;
    return p;
}

void scratch_reset(void)
{
    assert_single_owner();
    s_off = 0;
}

size_t scratch_mark(void)
{
    assert_single_owner();
    return s_off;
}

void scratch_release(size_t mark)
{
    assert_single_owner();
    assert(mark <= s_off && "scratch_release mark is above the current offset");
    s_off = mark;
}

size_t scratch_used(void)
{
    return s_off;
}

size_t scratch_high_water(void)
{
    return s_high_water;
}

size_t scratch_capacity(void)
{
    return DETWS_SCRATCH_ARENA_SIZE;
}
