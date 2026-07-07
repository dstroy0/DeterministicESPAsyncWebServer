// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file arena.cpp
 * @brief Unified double-ended server arena (Phase 1 core). See arena.h for the model.
 */

#include "network_drivers/session/arena.h"
#include <string.h>

namespace
{
// Persistent-pool block header: a chain of these spans [0, persist_end).
struct ABlk
{
    size_t size; ///< payload bytes
    size_t used; ///< 0 = free, 1 = in use
};
// Header size, rounded up to the arena alignment so payloads stay aligned.
const size_t AHDR = (sizeof(ABlk) + (DET_ARENA_ALIGN - 1)) & ~(size_t)(DET_ARENA_ALIGN - 1);

inline size_t align_up(size_t n)
{
    return (n + (DET_ARENA_ALIGN - 1)) & ~(size_t)(DET_ARENA_ALIGN - 1);
}
} // namespace

void det_arena_init(DetArena *a, void *base, size_t size)
{
    // Align the base up to the strongest supported alignment and the size down, so a
    // scratch borrow up to DET_ARENA_MAX_ALIGN is met by aligning its offset alone.
    uintptr_t b = (uintptr_t)base;
    uintptr_t ab = (b + (DET_ARENA_MAX_ALIGN - 1)) & ~(uintptr_t)(DET_ARENA_MAX_ALIGN - 1);
    size_t adj = (size_t)(ab - b);
    a->base = (uint8_t *)ab;
    a->size = (size > adj) ? ((size - adj) & ~(size_t)(DET_ARENA_ALIGN - 1)) : 0;
    a->persist_end = 0;
    a->scratch_top = a->size;
    a->persist_used = 0;
    a->persist_hw = 0;
    a->scratch_hw = 0;
}

// ---------------------------------------------------------------------------
// Persistent end (first-fit, grows up from the bottom)
// ---------------------------------------------------------------------------

void *det_arena_persist_alloc(DetArena *a, size_t n)
{
    n = align_up(n ? n : DET_ARENA_ALIGN);

    // First-fit over the existing block chain.
    size_t off = 0;
    while (off < a->persist_end)
    {
        ABlk *b = (ABlk *)(a->base + off);
        if (!b->used && b->size >= n)
        {
            // Split if the remainder can hold another header + a minimum payload.
            if (b->size >= n + AHDR + DET_ARENA_ALIGN)
            {
                ABlk *nb = (ABlk *)(a->base + off + AHDR + n);
                nb->size = b->size - n - AHDR;
                nb->used = 0;
                b->size = n;
            }
            b->used = 1;
            a->persist_used += b->size;
            void *pl = a->base + off + AHDR;
            memset(pl, 0, b->size);
            return pl;
        }
        off += AHDR + b->size;
    }

    // No reusable block: carve a fresh one from the free middle (grow the boundary up),
    // but only if it will not cross the scratch end.
    size_t need = AHDR + n;
    if (a->persist_end + need <= a->scratch_top && a->persist_end + need >= need)
    {
        ABlk *b = (ABlk *)(a->base + a->persist_end);
        b->size = n;
        b->used = 1;
        void *pl = a->base + a->persist_end + AHDR;
        a->persist_end += need;
        if (a->persist_end > a->persist_hw)
            a->persist_hw = a->persist_end;
        a->persist_used += n;
        memset(pl, 0, n);
        return pl;
    }
    return nullptr; // fail closed
}

void det_arena_persist_free(DetArena *a, void *p)
{
    if (!p)
        return;
    ABlk *b = (ABlk *)((uint8_t *)p - AHDR);
    if (b->used)
    {
        b->used = 0;
        if (a->persist_used >= b->size)
            a->persist_used -= b->size;
    }

    // Coalesce adjacent free blocks front-to-back.
    size_t off = 0;
    while (off < a->persist_end)
    {
        ABlk *cur = (ABlk *)(a->base + off);
        size_t next_off = off + AHDR + cur->size;
        if (!cur->used && next_off < a->persist_end)
        {
            ABlk *nxt = (ABlk *)(a->base + next_off);
            if (!nxt->used)
            {
                cur->size += AHDR + nxt->size; // merge; recheck this block
                continue;
            }
        }
        off = next_off;
    }

    // If the last block is free, hand it back to the free middle (shrink the boundary).
    off = 0;
    size_t last = 0;
    while (off < a->persist_end)
    {
        last = off;
        ABlk *cur = (ABlk *)(a->base + off);
        off += AHDR + cur->size;
    }
    if (a->persist_end > 0 && !((ABlk *)(a->base + last))->used)
        a->persist_end = last;
}

// ---------------------------------------------------------------------------
// Scratch end (bump, grows down from the top)
// ---------------------------------------------------------------------------

void *det_arena_scratch_alloc_aligned(DetArena *a, size_t n, size_t align)
{
    if (align < DET_ARENA_ALIGN)
        align = DET_ARENA_ALIGN;
    if (align > DET_ARENA_MAX_ALIGN)
        align = DET_ARENA_MAX_ALIGN; // the base only guarantees this much
    n = align_up(n ? n : DET_ARENA_ALIGN);
    if (a->scratch_top < n)
        return nullptr;
    // The base is DET_ARENA_MAX_ALIGN-aligned, so aligning the offset down aligns the pointer.
    size_t nt = (a->scratch_top - n) & ~(size_t)(align - 1);
    if (nt < a->persist_end || nt > a->scratch_top)
        return nullptr; // would cross the persistent end (or underflow)
    a->scratch_top = nt;
    size_t used = a->size - a->scratch_top;
    if (used > a->scratch_hw)
        a->scratch_hw = used;
    return a->base + a->scratch_top;
}

void *det_arena_scratch_alloc(DetArena *a, size_t n)
{
    return det_arena_scratch_alloc_aligned(a, n, DET_ARENA_ALIGN);
}

size_t det_arena_scratch_mark(const DetArena *a)
{
    return a->scratch_top;
}

void det_arena_scratch_release(DetArena *a, size_t mark)
{
    // A mark is an earlier (higher) scratch_top; releasing frees everything below it.
    if (mark >= a->scratch_top && mark <= a->size)
        a->scratch_top = mark;
}

void det_arena_scratch_reset(DetArena *a)
{
    a->scratch_top = a->size;
}

// ---------------------------------------------------------------------------
// Observability
// ---------------------------------------------------------------------------

size_t det_arena_free_bytes(const DetArena *a)
{
    size_t mid = (a->scratch_top > a->persist_end) ? a->scratch_top - a->persist_end : 0;
    return mid > AHDR ? mid - AHDR : 0; // usable payload of one new persistent block
}

size_t det_arena_persist_used(const DetArena *a)
{
    return a->persist_used;
}

size_t det_arena_scratch_used(const DetArena *a)
{
    return a->size - a->scratch_top;
}

// ===========================================================================
// Multi-region set (DRAM base + PSRAM extension)
// ===========================================================================

void det_arena_set_init(DetArenaSet *s)
{
    s->count = 0;
}

bool det_arena_set_add(DetArenaSet *s, void *base, size_t size)
{
    if (s->count >= DET_ARENA_MAX_REGIONS)
        return false;
    DetArena *r = &s->region[s->count];
    det_arena_init(r, base, size);
    if (r->size < AHDR + DET_ARENA_ALIGN)
        return false; // too small to hold even one block
    s->count++;
    return true;
}

void *det_arena_set_persist_alloc(DetArenaSet *s, size_t n)
{
    for (size_t i = 0; i < s->count; i++)
    {
        void *p = det_arena_persist_alloc(&s->region[i], n);
        if (p)
            return p;
    }
    return nullptr; // fail closed
}

void det_arena_set_persist_free(DetArenaSet *s, void *p)
{
    if (!p)
        return;
    uint8_t *b = (uint8_t *)p;
    for (size_t i = 0; i < s->count; i++)
    {
        DetArena *r = &s->region[i];
        if (b >= r->base && b < r->base + r->size)
        {
            det_arena_persist_free(r, p);
            return;
        }
    }
}

void *det_arena_set_scratch_alloc_aligned(DetArenaSet *s, size_t n, size_t align)
{
    for (size_t i = 0; i < s->count; i++)
    {
        void *p = det_arena_scratch_alloc_aligned(&s->region[i], n, align);
        if (p)
            return p;
    }
    return nullptr; // fail closed
}

void *det_arena_set_scratch_alloc(DetArenaSet *s, size_t n)
{
    return det_arena_set_scratch_alloc_aligned(s, n, DET_ARENA_ALIGN);
}

DetArenaMark det_arena_set_scratch_mark(const DetArenaSet *s)
{
    DetArenaMark m;
    m.count = s->count;
    for (size_t i = 0; i < s->count; i++)
        m.top[i] = s->region[i].scratch_top;
    return m;
}

void det_arena_set_scratch_release(DetArenaSet *s, const DetArenaMark *m)
{
    size_t n = m->count < s->count ? m->count : s->count;
    for (size_t i = 0; i < n; i++)
        det_arena_scratch_release(&s->region[i], m->top[i]);
}

void det_arena_set_scratch_reset(DetArenaSet *s)
{
    for (size_t i = 0; i < s->count; i++)
        det_arena_scratch_reset(&s->region[i]);
}

size_t det_arena_set_free_bytes(const DetArenaSet *s)
{
    size_t t = 0;
    for (size_t i = 0; i < s->count; i++)
        t += det_arena_free_bytes(&s->region[i]);
    return t;
}

size_t det_arena_set_persist_used(const DetArenaSet *s)
{
    size_t t = 0;
    for (size_t i = 0; i < s->count; i++)
        t += det_arena_persist_used(&s->region[i]);
    return t;
}

size_t det_arena_set_scratch_used(const DetArenaSet *s)
{
    size_t t = 0;
    for (size_t i = 0; i < s->count; i++)
        t += det_arena_scratch_used(&s->region[i]);
    return t;
}
