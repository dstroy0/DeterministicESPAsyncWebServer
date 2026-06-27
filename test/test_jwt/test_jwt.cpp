// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the JWT HS256 verifier. The reference token below was produced
// with Python (hmac+hashlib) for secret "s3cr3t-key" and payload
// {"sub":"alice","role":"admin","exp":2000000000,"iat":1700000000}.

#include "services/jwt/jwt.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static const char *SECRET = "s3cr3t-key";
static const char *TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
                           "eyJzdWIiOiJhbGljZSIsInJvbGUiOiJhZG1pbiIsImV4cCI6MjAwMDAwMDAwMCwiaWF0IjoxNzAwMDAwMDAwfQ."
                           "oaEaMu7USfUlYDaLYQlogmRd_1ZPBr7cKrPIo5lXdxc";

static const uint8_t *sec()
{
    return (const uint8_t *)SECRET;
}
static size_t seclen()
{
    return strlen(SECRET);
}

void setUp()
{
}
void tearDown()
{
}

void test_valid_token_accepts()
{
    TEST_ASSERT_TRUE(jwt_verify_hs256(TOKEN, strlen(TOKEN), sec(), seclen()));
}

void test_wrong_secret_rejects()
{
    const char *bad = "wrong-key";
    TEST_ASSERT_FALSE(jwt_verify_hs256(TOKEN, strlen(TOKEN), (const uint8_t *)bad, strlen(bad)));
}

void test_tampered_payload_rejects()
{
    char t[400];
    strcpy(t, TOKEN);
    // Flip a character in the payload segment (after the first dot).
    char *dot = strchr(t, '.');
    dot[3] = (dot[3] == 'A') ? 'B' : 'A';
    TEST_ASSERT_FALSE(jwt_verify_hs256(t, strlen(t), sec(), seclen()));
}

void test_tampered_signature_rejects()
{
    char t[400];
    strcpy(t, TOKEN);
    size_t n = strlen(t);
    t[n - 1] = (t[n - 1] == 'c') ? 'd' : 'c'; // last sig char
    TEST_ASSERT_FALSE(jwt_verify_hs256(t, strlen(t), sec(), seclen()));
}

void test_malformed_rejected()
{
    TEST_ASSERT_FALSE(jwt_verify_hs256("not-a-jwt", 9, sec(), seclen()));
    TEST_ASSERT_FALSE(jwt_verify_hs256("only.one-dot", 12, sec(), seclen()));
    TEST_ASSERT_FALSE(jwt_verify_hs256("", 0, sec(), seclen()));
}

void test_bearer_header()
{
    char hdr[400];
    snprintf(hdr, sizeof(hdr), "Bearer %s", TOKEN);
    TEST_ASSERT_TRUE(jwt_bearer_valid(hdr, sec(), seclen()));
    // case-insensitive scheme
    snprintf(hdr, sizeof(hdr), "bearer %s", TOKEN);
    TEST_ASSERT_TRUE(jwt_bearer_valid(hdr, sec(), seclen()));
    // missing scheme
    TEST_ASSERT_FALSE(jwt_bearer_valid(TOKEN, sec(), seclen()));
    TEST_ASSERT_FALSE(jwt_bearer_valid(nullptr, sec(), seclen()));
}

void test_claim_int()
{
    long v = 0;
    TEST_ASSERT_TRUE(jwt_claim_int(TOKEN, strlen(TOKEN), "exp", &v));
    TEST_ASSERT_EQUAL_INT32(2000000000, v);
    TEST_ASSERT_TRUE(jwt_claim_int(TOKEN, strlen(TOKEN), "iat", &v));
    TEST_ASSERT_EQUAL_INT32(1700000000, v);
}

void test_claim_missing()
{
    long v = 0;
    TEST_ASSERT_FALSE(jwt_claim_int(TOKEN, strlen(TOKEN), "nbf", &v));
}

void test_claim_str()
{
    char buf[32];
    TEST_ASSERT_TRUE(jwt_claim_str(TOKEN, strlen(TOKEN), "sub", buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("alice", buf);
    TEST_ASSERT_TRUE(jwt_claim_str(TOKEN, strlen(TOKEN), "role", buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("admin", buf);
    TEST_ASSERT_FALSE(jwt_claim_str(TOKEN, strlen(TOKEN), "nope", buf, sizeof(buf)));
    TEST_ASSERT_FALSE(jwt_claim_str(TOKEN, strlen(TOKEN), "exp", buf, sizeof(buf))); // numeric, not a string
}

void test_scope_allows()
{
    TEST_ASSERT_TRUE(jwt_scope_allows("read write admin", "read"));
    TEST_ASSERT_TRUE(jwt_scope_allows("read write admin", "write"));
    TEST_ASSERT_TRUE(jwt_scope_allows("read write admin", "admin"));
    TEST_ASSERT_TRUE(jwt_scope_allows("admin", "admin")); // single scope
    TEST_ASSERT_FALSE(jwt_scope_allows("read write admin", "delete"));
    TEST_ASSERT_FALSE(jwt_scope_allows("read write admin", "adm")); // whole-token match only
    TEST_ASSERT_FALSE(jwt_scope_allows("read write admin", ""));
    TEST_ASSERT_FALSE(jwt_scope_allows(nullptr, "read"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_valid_token_accepts);
    RUN_TEST(test_wrong_secret_rejects);
    RUN_TEST(test_tampered_payload_rejects);
    RUN_TEST(test_tampered_signature_rejects);
    RUN_TEST(test_malformed_rejected);
    RUN_TEST(test_bearer_header);
    RUN_TEST(test_claim_int);
    RUN_TEST(test_claim_missing);
    RUN_TEST(test_claim_str);
    RUN_TEST(test_scope_allows);
    return UNITY_END();
}
