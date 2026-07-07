// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the shared no-stdlib primitives: the base-10 number parsers
// (shared_primitives/numparse.h - det_strtol / det_strtoul / det_strtof, the
// strtol-family endptr contract) and the strict RFC 3629 UTF-8 validator
// (utf8.h). Pure host tests.

#include "shared_primitives/numparse.h"
#include "shared_primitives/utf8.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_strtol()
{
    const char *s = "  -42xyz";
    const char *end = nullptr;
    TEST_ASSERT_EQUAL_INT(-42, det_strtol(s, &end)); // leading ws + sign + digits
    TEST_ASSERT_EQUAL_PTR(s + 5, end);               // stopped at 'x'
    TEST_ASSERT_EQUAL_INT(7, det_strtol("+7", nullptr));

    const char *bad = "abc";
    const char *e2 = nullptr;
    TEST_ASSERT_EQUAL_INT(0, det_strtol(bad, &e2));
    TEST_ASSERT_EQUAL_PTR(bad, e2); // no digit converted -> end == s
}

void test_strtoul()
{
    const char *s = "  +123abc";
    const char *end = nullptr;
    TEST_ASSERT_EQUAL_UINT32(123, det_strtoul(s, &end)); // ws + '+' + digits
    TEST_ASSERT_EQUAL_PTR(s + 6, end);

    const char *bad = "  x";
    const char *e2 = nullptr;
    TEST_ASSERT_EQUAL_UINT32(0, det_strtoul(bad, &e2));
    TEST_ASSERT_EQUAL_PTR(bad, e2); // no digits -> end == s
}

void test_strtof()
{
    const char *end = nullptr;
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.14f, det_strtof("  3.14", &end)); // ws + int + frac
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -2.5f, det_strtof("-2.5", nullptr));
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 1500.0f, det_strtof("1.5e3", &end));      // exponent
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0125f, det_strtof("1.25E-2", &end)); // negative exponent

    const char *bad = "abc";
    const char *e2 = nullptr;
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, det_strtof(bad, &e2));
    TEST_ASSERT_EQUAL_PTR(bad, e2); // no digits -> end == s
}

void test_utf8_valid()
{
    TEST_ASSERT_TRUE(det_utf8_valid((const uint8_t *)"hello", 5)); // ASCII
    const uint8_t two[] = {0xC3, 0xA9};                            // U+00E9 e-acute
    TEST_ASSERT_TRUE(det_utf8_valid(two, 2));
    const uint8_t three[] = {0xE2, 0x82, 0xAC}; // U+20AC euro
    TEST_ASSERT_TRUE(det_utf8_valid(three, 3));
    const uint8_t four[] = {0xF0, 0x9F, 0x98, 0x80}; // U+1F600 emoji
    TEST_ASSERT_TRUE(det_utf8_valid(four, 4));
    TEST_ASSERT_TRUE(det_utf8_valid(nullptr, 0)); // empty is valid
}

void test_utf8_invalid()
{
    const uint8_t lead_cont[] = {0x80}; // a continuation byte as a lead
    TEST_ASSERT_FALSE(det_utf8_valid(lead_cont, 1));
    const uint8_t lead_f8[] = {0xF8, 0x80, 0x80, 0x80}; // 0xF8 is not a valid lead
    TEST_ASSERT_FALSE(det_utf8_valid(lead_f8, 4));
    const uint8_t truncated[] = {0xE2, 0x82}; // 3-byte lead, sequence cut short
    TEST_ASSERT_FALSE(det_utf8_valid(truncated, 2));
    const uint8_t bad_cont[] = {0xC3, 0x00}; // second byte is not 10xxxxxx
    TEST_ASSERT_FALSE(det_utf8_valid(bad_cont, 2));
    const uint8_t overlong[] = {0xC0, 0x80}; // overlong encoding of U+0000
    TEST_ASSERT_FALSE(det_utf8_valid(overlong, 2));
    const uint8_t surrogate[] = {0xED, 0xA0, 0x80}; // U+D800 surrogate
    TEST_ASSERT_FALSE(det_utf8_valid(surrogate, 3));
    const uint8_t too_big[] = {0xF4, 0x90, 0x80, 0x80}; // U+110000 > U+10FFFF
    TEST_ASSERT_FALSE(det_utf8_valid(too_big, 4));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_strtol);
    RUN_TEST(test_strtoul);
    RUN_TEST(test_strtof);
    RUN_TEST(test_utf8_valid);
    RUN_TEST(test_utf8_invalid);
    return UNITY_END();
}
