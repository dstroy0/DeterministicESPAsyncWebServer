// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/southbound: the driver registry + name-dispatched read/write facade.

#include "services/southbound/southbound.h"
#include <unity.h>

// A fake driver backed by a small register file, to prove dispatch reaches the right ctx.
struct FakeCtx
{
    int32_t regs[16];
    int fail_next; // if nonzero, the next read/write returns this negative code once.
};

static int fake_read(void *ctx, uint32_t p, int32_t *out)
{
    FakeCtx *f = (FakeCtx *)ctx;
    if (f->fail_next)
    {
        int e = f->fail_next;
        f->fail_next = 0;
        return e;
    }
    if (p >= 16)
        return Sb::SB_ERR_ARG;
    *out = f->regs[p];
    return Sb::SB_OK;
}

static int fake_write(void *ctx, uint32_t p, int32_t v)
{
    FakeCtx *f = (FakeCtx *)ctx;
    if (p >= 16)
        return Sb::SB_ERR_ARG;
    f->regs[p] = v;
    return Sb::SB_OK;
}

static int fake_read_block(void *ctx, uint32_t first, int32_t *out, size_t n)
{
    FakeCtx *f = (FakeCtx *)ctx;
    if (first + n > 16)
        return Sb::SB_ERR_ARG;
    for (size_t i = 0; i < n; i++)
        out[i] = f->regs[first + i];
    return (int)n;
}

static int fake_write_block(void *ctx, uint32_t first, const int32_t *in, size_t n)
{
    FakeCtx *f = (FakeCtx *)ctx;
    if (first + n > 16)
        return Sb::SB_ERR_ARG;
    for (size_t i = 0; i < n; i++)
        f->regs[first + i] = in[i];
    return (int)n;
}

static FakeCtx g_ctx;
static SouthboundDriver g_full = {"fake", fake_read, fake_write, fake_read_block, fake_write_block, &g_ctx};

void setUp(void)
{
    detws_southbound_clear();
    for (int i = 0; i < 16; i++)
        g_ctx.regs[i] = i * 10;
    g_ctx.fail_next = 0;
}
void tearDown(void)
{
}

void test_register_and_find(void)
{
    TEST_ASSERT_EQUAL_INT(Sb::SB_OK, detws_southbound_register(&g_full));
    TEST_ASSERT_EQUAL_size_t(1, detws_southbound_count());
    TEST_ASSERT_EQUAL_PTR(&g_full, detws_southbound_find("fake"));
    TEST_ASSERT_NULL(detws_southbound_find("nope"));
    // Duplicate name rejected.
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_DUP, detws_southbound_register(&g_full));
    // Bad args.
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_ARG, detws_southbound_register(nullptr));
    SouthboundDriver noname = {nullptr, fake_read, nullptr, nullptr, nullptr, nullptr};
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_ARG, detws_southbound_register(&noname));
}

void test_read_write_dispatch(void)
{
    detws_southbound_register(&g_full);
    int32_t v = -1;
    TEST_ASSERT_EQUAL_INT(Sb::SB_OK, detws_southbound_read("fake", 3, &v));
    TEST_ASSERT_EQUAL_INT32(30, v);
    TEST_ASSERT_EQUAL_INT(Sb::SB_OK, detws_southbound_write("fake", 3, 999));
    TEST_ASSERT_EQUAL_INT(Sb::SB_OK, detws_southbound_read("fake", 3, &v));
    TEST_ASSERT_EQUAL_INT32(999, v);
    // Unknown driver.
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_NOT_FOUND, detws_southbound_read("x", 0, &v));
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_NOT_FOUND, detws_southbound_write("x", 0, 0));
    // Null out.
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_ARG, detws_southbound_read("fake", 0, nullptr));
    // Driver transport error propagated unchanged.
    g_ctx.fail_next = -42;
    TEST_ASSERT_EQUAL_INT(-42, detws_southbound_read("fake", 0, &v));
}

void test_block_atomic(void)
{
    detws_southbound_register(&g_full);
    int32_t out[4] = {0};
    TEST_ASSERT_EQUAL_INT(4, detws_southbound_read_block("fake", 2, out, 4));
    TEST_ASSERT_EQUAL_INT32(20, out[0]);
    TEST_ASSERT_EQUAL_INT32(50, out[3]);
    int32_t in[3] = {7, 8, 9};
    TEST_ASSERT_EQUAL_INT(3, detws_southbound_write_block("fake", 5, in, 3));
    int32_t v = 0;
    detws_southbound_read("fake", 6, &v);
    TEST_ASSERT_EQUAL_INT32(8, v);
    // Zero count / null rejected.
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_ARG, detws_southbound_read_block("fake", 0, out, 0));
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_ARG, detws_southbound_write_block("fake", 0, nullptr, 3));
}

void test_unsupported_capability(void)
{
    // A driver that only implements single-point read.
    SouthboundDriver ro = {"ro", fake_read, nullptr, nullptr, nullptr, &g_ctx};
    detws_southbound_register(&ro);
    int32_t v = 0, out[2] = {0};
    TEST_ASSERT_EQUAL_INT(Sb::SB_OK, detws_southbound_read("ro", 0, &v));
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_UNSUPPORTED, detws_southbound_write("ro", 0, 1));
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_UNSUPPORTED, detws_southbound_read_block("ro", 0, out, 2));
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_UNSUPPORTED, detws_southbound_write_block("ro", 0, out, 2));
}

void test_registry_full(void)
{
    // Fill the registry with distinct-named drivers, then overflow.
    static SouthboundDriver drv[9];
    static char names[9][8];
    int registered = 0;
    for (int i = 0; i < 9; i++)
    {
        names[i][0] = 'd';
        names[i][1] = (char)('0' + i);
        names[i][2] = '\0';
        drv[i] = {names[i], fake_read, nullptr, nullptr, nullptr, &g_ctx};
        if (detws_southbound_register(&drv[i]) == Sb::SB_OK)
            registered++;
        else
            TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_FULL, detws_southbound_register(&drv[i]));
    }
    // Default cap is 8.
    TEST_ASSERT_EQUAL_size_t(8, detws_southbound_count());
    TEST_ASSERT_EQUAL_INT(8, registered);
}

void test_dispatch_not_found_guards()
{
    detws_southbound_clear();
    TEST_ASSERT_NULL(detws_southbound_find("nope")); // not registered -> null
    int32_t v = 0;
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_NOT_FOUND, detws_southbound_read("nope", 0, &v));  // no driver
    TEST_ASSERT_EQUAL_INT(Sb::SB_ERR_NOT_FOUND, detws_southbound_write("nope", 0, 42)); // no driver
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_register_and_find);
    RUN_TEST(test_read_write_dispatch);
    RUN_TEST(test_block_atomic);
    RUN_TEST(test_unsupported_capability);
    RUN_TEST(test_registry_full);
    RUN_TEST(test_dispatch_not_found_guards);
    return UNITY_END();
}
