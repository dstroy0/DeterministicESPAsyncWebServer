// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the unified double-ended arena (network_drivers/session/det_arena):
// first-fit persistent end (bottom) + bump scratch end (top) sharing a floating middle.

#include "network_drivers/session/det_arena.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

static uint8_t g_buf[4096];
static DetArena a;

void setUp()
{
    memset(g_buf, 0xAA, sizeof(g_buf)); // poison so we can see zeroing
    det_arena_init(&a, g_buf, sizeof(g_buf));
}
void tearDown()
{
}

static bool aligned8(const void *p)
{
    return ((uintptr_t)p & 7u) == 0;
}
static bool in_region(const DetArena *ar, const void *p, size_t n)
{
    const uint8_t *b = (const uint8_t *)p;
    return b >= ar->base && b + n <= ar->base + ar->size;
}

// ---- persistent end -------------------------------------------------------

void test_persist_basic_alloc()
{
    void *p = det_arena_persist_alloc(&a, 100);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_TRUE(aligned8(p));
    TEST_ASSERT_TRUE(in_region(&a, p, 100));
    void *q = det_arena_persist_alloc(&a, 100);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_TRUE(p != q);
    TEST_ASSERT_TRUE((uintptr_t)q >= (uintptr_t)p + 100); // grows up, no overlap
}

void test_persist_zeroed()
{
    uint8_t *p = (uint8_t *)det_arena_persist_alloc(&a, 64);
    for (int i = 0; i < 64; i++)
        TEST_ASSERT_EQUAL_UINT8(0, p[i]); // zeroed despite 0xAA poison
    memset(p, 0xFF, 64);
    det_arena_persist_free(&a, p);
    uint8_t *q = (uint8_t *)det_arena_persist_alloc(&a, 64); // reuses the slot
    TEST_ASSERT_EQUAL_PTR(p, q);
    for (int i = 0; i < 64; i++)
        TEST_ASSERT_EQUAL_UINT8(0, q[i]); // re-zeroed
}

void test_persist_first_fit_reuse()
{
    void *A = det_arena_persist_alloc(&a, 64);
    void *B = det_arena_persist_alloc(&a, 64);
    void *C = det_arena_persist_alloc(&a, 64);
    (void)A;
    (void)C;
    det_arena_persist_free(&a, B);
    void *D = det_arena_persist_alloc(&a, 64); // first-fit reuses B's hole
    TEST_ASSERT_EQUAL_PTR(B, D);
}

void test_persist_coalesce()
{
    void *A = det_arena_persist_alloc(&a, 64);
    void *B = det_arena_persist_alloc(&a, 64);
    void *C = det_arena_persist_alloc(&a, 64);
    (void)A;
    det_arena_persist_free(&a, B);
    det_arena_persist_free(&a, C); // B+C coalesce into one hole
    void *big = det_arena_persist_alloc(&a, 150);
    TEST_ASSERT_NOT_NULL(big);
    TEST_ASSERT_EQUAL_PTR(B, big); // the merged B+C region holds 150
}

void test_persist_free_shrinks_boundary()
{
    size_t free0 = det_arena_free_bytes(&a);
    void *p = det_arena_persist_alloc(&a, 1024);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_TRUE(det_arena_free_bytes(&a) < free0);
    det_arena_persist_free(&a, p); // top block freed -> returns to middle
    TEST_ASSERT_EQUAL_size_t(free0, det_arena_free_bytes(&a));
    TEST_ASSERT_EQUAL_size_t(0, det_arena_persist_used(&a));
}

// ---- scratch end ----------------------------------------------------------

void test_scratch_bump_and_reset()
{
    TEST_ASSERT_EQUAL_size_t(0, det_arena_scratch_used(&a));
    void *p = det_arena_scratch_alloc(&a, 100);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_TRUE(aligned8(p));
    TEST_ASSERT_TRUE(det_arena_scratch_used(&a) >= 100);
    void *q = det_arena_scratch_alloc(&a, 100);
    TEST_ASSERT_TRUE(p != q);
    TEST_ASSERT_TRUE((uintptr_t)q < (uintptr_t)p); // grows down
    det_arena_scratch_reset(&a);
    TEST_ASSERT_EQUAL_size_t(0, det_arena_scratch_used(&a));
}

void test_scratch_mark_release()
{
    det_arena_scratch_alloc(&a, 100);
    size_t used1 = det_arena_scratch_used(&a);
    size_t mk = det_arena_scratch_mark(&a);
    det_arena_scratch_alloc(&a, 200);
    det_arena_scratch_alloc(&a, 50);
    TEST_ASSERT_TRUE(det_arena_scratch_used(&a) > used1);
    det_arena_scratch_release(&a, mk); // frees everything after the mark
    TEST_ASSERT_EQUAL_size_t(used1, det_arena_scratch_used(&a));
}

// ---- both ends / floating boundary ----------------------------------------

void test_persist_and_scratch_no_overlap()
{
    uint8_t *p = (uint8_t *)det_arena_persist_alloc(&a, 500);
    uint8_t *s = (uint8_t *)det_arena_scratch_alloc(&a, 500);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(s);
    // persistent is below scratch, no overlap
    TEST_ASSERT_TRUE(p + 500 <= s);
    memset(p, 0x11, 500);
    memset(s, 0x22, 500);
    TEST_ASSERT_EQUAL_UINT8(0x11, p[499]);
    TEST_ASSERT_EQUAL_UINT8(0x22, s[0]);
}

void test_boundary_collision_fail_closed()
{
    // Take most of the arena from the bottom, then from the top, until they nearly meet.
    void *p = det_arena_persist_alloc(&a, 1800);
    void *s = det_arena_scratch_alloc(&a, 1800);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(s);
    // The middle is now tiny; a big alloc from either end must fail (not cross).
    TEST_ASSERT_NULL(det_arena_persist_alloc(&a, 1000));
    TEST_ASSERT_NULL(det_arena_scratch_alloc(&a, 1000));
    // A failed alloc must not have moved either boundary / corrupted the survivors.
    TEST_ASSERT_TRUE((uint8_t *)p + 1800 <= (uint8_t *)s);
}

void test_scratch_reset_frees_middle_for_persist()
{
    det_arena_scratch_alloc(&a, 3000); // hog the top
    TEST_ASSERT_NULL(det_arena_persist_alloc(&a, 2000));
    det_arena_scratch_reset(&a); // give it back
    TEST_ASSERT_NOT_NULL(det_arena_persist_alloc(&a, 2000));
}

void test_alignment_various_sizes()
{
    size_t sizes[] = {1, 3, 7, 8, 9, 17, 33, 100, 255};
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
    {
        void *p = det_arena_persist_alloc(&a, sizes[i]);
        void *s = det_arena_scratch_alloc(&a, sizes[i]);
        TEST_ASSERT_TRUE(aligned8(p));
        TEST_ASSERT_TRUE(aligned8(s));
    }
}

void test_scratch_alignment_16()
{
    // The real scratch callers (SSH, WS deflate) ask for 16-byte alignment.
    for (int i = 0; i < 20; i++)
    {
        void *p = det_arena_scratch_alloc_aligned(&a, 17 + i, 16);
        TEST_ASSERT_NOT_NULL(p);
        TEST_ASSERT_TRUE(((uintptr_t)p & 15u) == 0); // 16-aligned
    }
    void *q = det_arena_scratch_alloc_aligned(&a, 1, 1); // align clamps up to 8, never 0
    TEST_ASSERT_TRUE(aligned8(q));
}

void test_zero_size_and_null_free()
{
    void *p = det_arena_persist_alloc(&a, 0); // rounds up to alignment, non-null
    TEST_ASSERT_NOT_NULL(p);
    det_arena_persist_free(&a, nullptr); // no-op, no crash
    void *s = det_arena_scratch_alloc(&a, 0);
    TEST_ASSERT_NOT_NULL(s);
}

// ---- multi-region set (DRAM base + PSRAM extension) -----------------------

static uint8_t g_r0[512];  // stand-in for internal DRAM (preferred)
static uint8_t g_r1[2048]; // stand-in for the PSRAM extension

static bool in_buf(const void *p, const uint8_t *buf, size_t n)
{
    const uint8_t *b = (const uint8_t *)p;
    return b >= buf && b < buf + n;
}

void test_set_add_limits()
{
    DetArenaSet s;
    det_arena_set_init(&s);
    TEST_ASSERT_TRUE(det_arena_set_add(&s, g_r0, sizeof(g_r0)));
    TEST_ASSERT_TRUE(det_arena_set_add(&s, g_r1, sizeof(g_r1)));
    static uint8_t extra[256];
    TEST_ASSERT_FALSE(det_arena_set_add(&s, extra, sizeof(extra))); // full (max 2)
    DetArenaSet t;
    det_arena_set_init(&t);
    static uint8_t tiny[4];
    TEST_ASSERT_FALSE(det_arena_set_add(&t, tiny, sizeof(tiny))); // too small
}

void test_set_persist_overflow_and_prefer()
{
    DetArenaSet s;
    det_arena_set_init(&s);
    det_arena_set_add(&s, g_r0, sizeof(g_r0));
    det_arena_set_add(&s, g_r1, sizeof(g_r1));
    void *big = det_arena_set_persist_alloc(&s, 700); // too big for r0 -> spills to r1
    TEST_ASSERT_NOT_NULL(big);
    TEST_ASSERT_TRUE(in_buf(big, g_r1, sizeof(g_r1)));
    void *small = det_arena_set_persist_alloc(&s, 32); // prefers r0
    TEST_ASSERT_NOT_NULL(small);
    TEST_ASSERT_TRUE(in_buf(small, g_r0, sizeof(g_r0)));
    TEST_ASSERT_TRUE(det_arena_set_persist_used(&s) >= 732); // >= requested (alignment rounds up)
}

void test_set_persist_free_routes_by_address()
{
    DetArenaSet s;
    det_arena_set_init(&s);
    det_arena_set_add(&s, g_r0, sizeof(g_r0));
    det_arena_set_add(&s, g_r1, sizeof(g_r1));
    void *inR1 = det_arena_set_persist_alloc(&s, 700);
    TEST_ASSERT_TRUE(in_buf(inR1, g_r1, sizeof(g_r1)));
    det_arena_set_persist_free(&s, inR1); // must route to r1, not r0
    TEST_ASSERT_EQUAL_size_t(0, det_arena_set_persist_used(&s));
    det_arena_set_persist_free(&s, nullptr); // no-op
}

void test_set_scratch_overflow_and_unwind()
{
    DetArenaSet s;
    det_arena_set_init(&s);
    det_arena_set_add(&s, g_r0, sizeof(g_r0));
    det_arena_set_add(&s, g_r1, sizeof(g_r1));
    void *a0 = det_arena_set_scratch_alloc(&s, 400); // fits r0
    TEST_ASSERT_TRUE(in_buf(a0, g_r0, sizeof(g_r0)));
    DetArenaMark mk = det_arena_set_scratch_mark(&s);
    void *a1 = det_arena_set_scratch_alloc(&s, 900); // r0 too full -> spills to r1
    TEST_ASSERT_TRUE(in_buf(a1, g_r1, sizeof(g_r1)));
    TEST_ASSERT_TRUE(det_arena_set_scratch_used(&s) >= 1300);
    det_arena_set_scratch_release(&s, &mk); // unwinds the r1 spill too
    TEST_ASSERT_TRUE(det_arena_set_scratch_used(&s) >= 400);
    TEST_ASSERT_TRUE(det_arena_set_scratch_used(&s) < 900);
    det_arena_set_scratch_reset(&s);
    TEST_ASSERT_EQUAL_size_t(0, det_arena_set_scratch_used(&s));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_persist_basic_alloc);
    RUN_TEST(test_persist_zeroed);
    RUN_TEST(test_persist_first_fit_reuse);
    RUN_TEST(test_persist_coalesce);
    RUN_TEST(test_persist_free_shrinks_boundary);
    RUN_TEST(test_scratch_bump_and_reset);
    RUN_TEST(test_scratch_mark_release);
    RUN_TEST(test_persist_and_scratch_no_overlap);
    RUN_TEST(test_boundary_collision_fail_closed);
    RUN_TEST(test_scratch_reset_frees_middle_for_persist);
    RUN_TEST(test_alignment_various_sizes);
    RUN_TEST(test_scratch_alignment_16);
    RUN_TEST(test_zero_size_and_null_free);
    RUN_TEST(test_set_add_limits);
    RUN_TEST(test_set_persist_overflow_and_prefer);
    RUN_TEST(test_set_persist_free_routes_by_address);
    RUN_TEST(test_set_scratch_overflow_and_unwind);
    return UNITY_END();
}
