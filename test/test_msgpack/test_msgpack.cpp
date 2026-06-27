// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the MessagePack encoder (network_drivers/presentation/msgpack).
// Expected byte sequences follow the MessagePack spec encodings.

#include "network_drivers/presentation/msgpack.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void check(const uint8_t *got, size_t got_len, const uint8_t *exp, size_t exp_len)
{
    TEST_ASSERT_EQUAL_size_t(exp_len, got_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, got, exp_len);
}

void test_uint()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 0);
    uint8_t e0[] = {0x00};
    check(b, msgpack_len(&w), e0, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 127);
    uint8_t e7f[] = {0x7f};
    check(b, msgpack_len(&w), e7f, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 128);
    uint8_t e80[] = {0xcc, 0x80};
    check(b, msgpack_len(&w), e80, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 256);
    uint8_t e256[] = {0xcd, 0x01, 0x00};
    check(b, msgpack_len(&w), e256, 3);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 65536);
    uint8_t e64k[] = {0xce, 0x00, 0x01, 0x00, 0x00};
    check(b, msgpack_len(&w), e64k, 5);
}

void test_int()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -1);
    uint8_t e[] = {0xff};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -32);
    uint8_t e32[] = {0xe0};
    check(b, msgpack_len(&w), e32, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -33);
    uint8_t e33[] = {0xd0, 0xdf};
    check(b, msgpack_len(&w), e33, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -129);
    uint8_t e129[] = {0xd1, 0xff, 0x7f};
    check(b, msgpack_len(&w), e129, 3);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, 42); // positive via msgpack_int -> fixint
    uint8_t e42[] = {0x2a};
    check(b, msgpack_len(&w), e42, 1);
}

void test_str()
{
    uint8_t b[40];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "");
    uint8_t e[] = {0xa0};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "a");
    uint8_t e2[] = {0xa1, 0x61};
    check(b, msgpack_len(&w), e2, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "hello");
    uint8_t e3[] = {0xa5, 0x68, 0x65, 0x6c, 0x6c, 0x6f};
    check(b, msgpack_len(&w), e3, 6);
}

void test_bytes()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    uint8_t data[] = {1, 2, 3};
    msgpack_bytes(&w, data, 3);
    uint8_t e[] = {0xc4, 0x03, 0x01, 0x02, 0x03};
    check(b, msgpack_len(&w), e, 5);
}

void test_simple()
{
    uint8_t b[8];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_bool(&w, false);
    uint8_t e[] = {0xc2};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_bool(&w, true);
    uint8_t e2[] = {0xc3};
    check(b, msgpack_len(&w), e2, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_nil(&w);
    uint8_t e3[] = {0xc0};
    check(b, msgpack_len(&w), e3, 1);
}

void test_float()
{
    uint8_t b[8];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_float(&w, 1.0f);
    uint8_t e[] = {0xca, 0x3f, 0x80, 0x00, 0x00};
    check(b, msgpack_len(&w), e, 5);
}

void test_array_and_map()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_array(&w, 3);
    msgpack_uint(&w, 1);
    msgpack_uint(&w, 2);
    msgpack_uint(&w, 3);
    uint8_t e[] = {0x93, 0x01, 0x02, 0x03};
    check(b, msgpack_len(&w), e, 4);
    msgpack_init(&w, b, sizeof(b));
    msgpack_map(&w, 1);
    msgpack_uint(&w, 1);
    msgpack_uint(&w, 2);
    uint8_t e2[] = {0x81, 0x01, 0x02};
    check(b, msgpack_len(&w), e2, 3);
}

void test_overflow_fails_closed()
{
    uint8_t b[2];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 65536); // needs 5 bytes
    TEST_ASSERT_FALSE(msgpack_ok(&w));
    TEST_ASSERT_EQUAL_size_t(5, msgpack_len(&w));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_uint);
    RUN_TEST(test_int);
    RUN_TEST(test_str);
    RUN_TEST(test_bytes);
    RUN_TEST(test_simple);
    RUN_TEST(test_float);
    RUN_TEST(test_array_and_map);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
