// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the CBOR encoder (network_drivers/presentation/cbor). Expected
// byte sequences are the canonical RFC 8949 Appendix A diagnostic vectors.

#include "network_drivers/presentation/cbor.h"
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
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 0);
    TEST_ASSERT_TRUE(cbor_ok(&w));
    uint8_t e0[] = {0x00};
    check(b, cbor_len(&w), e0, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 23);
    uint8_t e23[] = {0x17};
    check(b, cbor_len(&w), e23, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 24);
    uint8_t e24[] = {0x18, 0x18};
    check(b, cbor_len(&w), e24, 2);
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 1000);
    uint8_t e1k[] = {0x19, 0x03, 0xe8};
    check(b, cbor_len(&w), e1k, 3);
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 1000000);
    uint8_t e1m[] = {0x1a, 0x00, 0x0f, 0x42, 0x40};
    check(b, cbor_len(&w), e1m, 5);
}

void test_int()
{
    uint8_t b[16];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_int(&w, -1);
    uint8_t e[] = {0x20};
    check(b, cbor_len(&w), e, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_int(&w, -100);
    uint8_t e2[] = {0x38, 0x63};
    check(b, cbor_len(&w), e2, 2);
    cbor_init(&w, b, sizeof(b));
    cbor_int(&w, -1000);
    uint8_t e3[] = {0x39, 0x03, 0xe7};
    check(b, cbor_len(&w), e3, 3);
    cbor_init(&w, b, sizeof(b));
    cbor_int(&w, 42); // positive routed through cbor_int
    uint8_t e4[] = {0x18, 0x2a};
    check(b, cbor_len(&w), e4, 2);
}

void test_text()
{
    uint8_t b[16];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_text(&w, "");
    uint8_t e[] = {0x60};
    check(b, cbor_len(&w), e, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_text(&w, "a");
    uint8_t e2[] = {0x61, 0x61};
    check(b, cbor_len(&w), e2, 2);
    cbor_init(&w, b, sizeof(b));
    cbor_text(&w, "IETF");
    uint8_t e3[] = {0x64, 0x49, 0x45, 0x54, 0x46};
    check(b, cbor_len(&w), e3, 5);
}

void test_bytes()
{
    uint8_t b[16];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    uint8_t data[] = {1, 2, 3, 4};
    cbor_bytes(&w, data, 4);
    uint8_t e[] = {0x44, 0x01, 0x02, 0x03, 0x04};
    check(b, cbor_len(&w), e, 5);
}

void test_simple()
{
    uint8_t b[8];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_bool(&w, false);
    uint8_t e[] = {0xf4};
    check(b, cbor_len(&w), e, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_bool(&w, true);
    uint8_t e2[] = {0xf5};
    check(b, cbor_len(&w), e2, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_null(&w);
    uint8_t e3[] = {0xf6};
    check(b, cbor_len(&w), e3, 1);
}

void test_float()
{
    uint8_t b[8];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_float(&w, 1.0f);
    uint8_t e[] = {0xfa, 0x3f, 0x80, 0x00, 0x00};
    check(b, cbor_len(&w), e, 5);
}

void test_array_and_map()
{
    uint8_t b[16];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_array(&w, 0);
    uint8_t e[] = {0x80};
    check(b, cbor_len(&w), e, 1);
    cbor_init(&w, b, sizeof(b));
    cbor_array(&w, 3);
    cbor_uint(&w, 1);
    cbor_uint(&w, 2);
    cbor_uint(&w, 3);
    uint8_t e2[] = {0x83, 0x01, 0x02, 0x03};
    check(b, cbor_len(&w), e2, 4);
    cbor_init(&w, b, sizeof(b));
    cbor_map(&w, 2);
    cbor_uint(&w, 1);
    cbor_uint(&w, 2);
    cbor_uint(&w, 3);
    cbor_uint(&w, 4);
    uint8_t e3[] = {0xa2, 0x01, 0x02, 0x03, 0x04};
    check(b, cbor_len(&w), e3, 5);
}

// A write that does not fit sets overflow but still reports the needed size.
void test_overflow_fails_closed()
{
    uint8_t b[2];
    CborWriter w;
    cbor_init(&w, b, sizeof(b));
    cbor_uint(&w, 1000000); // needs 5 bytes
    TEST_ASSERT_FALSE(cbor_ok(&w));
    TEST_ASSERT_EQUAL_size_t(5, cbor_len(&w));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_uint);
    RUN_TEST(test_int);
    RUN_TEST(test_text);
    RUN_TEST(test_bytes);
    RUN_TEST(test_simple);
    RUN_TEST(test_float);
    RUN_TEST(test_array_and_map);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
