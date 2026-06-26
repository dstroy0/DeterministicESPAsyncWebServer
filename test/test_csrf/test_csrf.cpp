// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the stateless HMAC-signed CSRF token (services/csrf). A fixed
// secret makes issue/verify deterministic on the host; the tests cover the
// round trip, tamper rejection, secret binding, format, and edge cases.

#include "services/csrf/csrf.h"
#include <string.h>
#include <unity.h>

static const uint8_t SECRET[32] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
                                   0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x10, 0x32, 0x54, 0x76, 0x98, 0xba,
                                   0xdc, 0xfe, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

void setUp()
{
    csrf_reset();
    csrf_set_secret(SECRET, sizeof(SECRET));
}
void tearDown()
{
}

// A freshly issued token verifies.
void test_issue_verify_roundtrip()
{
    char t[CSRF_TOKEN_BUF];
    int n = csrf_issue(t, sizeof(t));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(csrf_verify(t));
}

// The token has the documented `<nonce>.<sig>` shape and length.
void test_token_format_and_length()
{
    char t[CSRF_TOKEN_BUF];
    int n = csrf_issue(t, sizeof(t));
    TEST_ASSERT_EQUAL_INT(CSRF_NONCE_BYTES * 2 + 1 + CSRF_SIG_BYTES * 2, n);
    const char *dot = strchr(t, '.');
    TEST_ASSERT_NOT_NULL(dot);
    TEST_ASSERT_EQUAL_size_t(CSRF_NONCE_BYTES * 2, (size_t)(dot - t));
}

// Flipping a signature character invalidates the token.
void test_verify_rejects_tampered_sig()
{
    char t[CSRF_TOKEN_BUF];
    csrf_issue(t, sizeof(t));
    size_t len = strlen(t);
    t[len - 1] = (t[len - 1] == 'a') ? 'b' : 'a';
    TEST_ASSERT_FALSE(csrf_verify(t));
}

// Flipping a nonce character invalidates the token (the recomputed sig changes).
void test_verify_rejects_tampered_nonce()
{
    char t[CSRF_TOKEN_BUF];
    csrf_issue(t, sizeof(t));
    t[0] = (t[0] == 'a') ? 'b' : 'a';
    TEST_ASSERT_FALSE(csrf_verify(t));
}

// Malformed strings are rejected, not crashed on.
void test_verify_rejects_garbage()
{
    TEST_ASSERT_FALSE(csrf_verify("notatoken"));
    TEST_ASSERT_FALSE(csrf_verify(""));
    TEST_ASSERT_FALSE(csrf_verify("."));
    TEST_ASSERT_FALSE(csrf_verify("abcd.ef"));
}

// A token signed under one secret does not verify under another.
void test_different_secret_rejects()
{
    char t[CSRF_TOKEN_BUF];
    csrf_issue(t, sizeof(t));
    uint8_t other[32];
    memset(other, 0xAB, sizeof(other));
    csrf_set_secret(other, sizeof(other));
    TEST_ASSERT_FALSE(csrf_verify(t));
}

// With no secret set, issuing and verifying both fail closed.
void test_no_secret_fails_closed()
{
    csrf_reset();
    char t[CSRF_TOKEN_BUF];
    TEST_ASSERT_EQUAL_INT(0, csrf_issue(t, sizeof(t)));
    TEST_ASSERT_FALSE(csrf_verify("0102030405060708090a0b0c.0102030405060708090a0b0c0d0e"));
}

// Successive tokens differ (the nonce counter advances) and both verify.
void test_issue_unique()
{
    char a[CSRF_TOKEN_BUF], b[CSRF_TOKEN_BUF];
    csrf_issue(a, sizeof(a));
    csrf_issue(b, sizeof(b));
    TEST_ASSERT_TRUE(strcmp(a, b) != 0);
    TEST_ASSERT_TRUE(csrf_verify(a));
    TEST_ASSERT_TRUE(csrf_verify(b));
}

// A buffer that cannot hold a token yields 0 (no overflow).
void test_issue_rejects_small_buffer()
{
    char small[10];
    TEST_ASSERT_EQUAL_INT(0, csrf_issue(small, sizeof(small)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_issue_verify_roundtrip);
    RUN_TEST(test_token_format_and_length);
    RUN_TEST(test_verify_rejects_tampered_sig);
    RUN_TEST(test_verify_rejects_tampered_nonce);
    RUN_TEST(test_verify_rejects_garbage);
    RUN_TEST(test_different_secret_rejects);
    RUN_TEST(test_no_secret_fails_closed);
    RUN_TEST(test_issue_unique);
    RUN_TEST(test_issue_rejects_small_buffer);
    return UNITY_END();
}
