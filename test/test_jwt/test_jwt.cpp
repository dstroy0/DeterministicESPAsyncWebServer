// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the JWT HS256 verifier. The reference token below was produced
// with Python (hmac+hashlib) for secret "s3cr3t-key" and payload
// {"sub":"alice","role":"admin","exp":2000000000,"iat":1700000000}.

#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
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

// RFC 7515 5.2: the verifier must check the header "alg". A token whose header
// declares a non-HS256 algorithm must be rejected even when its signature is a VALID
// HMAC-SHA256 over header.payload (algorithm-substitution defense). Built here with
// the library's own HMAC + base64url so it is self-contained.
void test_alg_not_hs256_rejected()
{
    const char *hdr_json = "{\"alg\":\"none\",\"typ\":\"JWT\"}";
    const char *pl_json = "{\"sub\":\"alice\"}";
    char hdr[64], payload[64];
    base64url_encode((const uint8_t *)hdr_json, strlen(hdr_json), hdr);
    base64url_encode((const uint8_t *)pl_json, strlen(pl_json), payload);

    char signing[160];
    int sl = snprintf(signing, sizeof(signing), "%s.%s", hdr, payload);
    uint8_t mac[SSH_HMAC_SHA256_LEN];
    ssh_hmac_sha256(sec(), seclen(), (const uint8_t *)signing, (size_t)sl, mac);
    char sig[48];
    base64url_encode(mac, sizeof(mac), sig);

    char token[256];
    snprintf(token, sizeof(token), "%s.%s.%s", hdr, payload, sig);
    // The HMAC is valid for these bytes, but alg is "none" -> must be rejected.
    TEST_ASSERT_FALSE(jwt_verify_hs256(token, strlen(token), sec(), seclen()));

    // Sanity: the same construction with alg "HS256" verifies (proves the test rig).
    const char *ok_hdr_json = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    char ok_hdr[64];
    base64url_encode((const uint8_t *)ok_hdr_json, strlen(ok_hdr_json), ok_hdr);
    sl = snprintf(signing, sizeof(signing), "%s.%s", ok_hdr, payload);
    ssh_hmac_sha256(sec(), seclen(), (const uint8_t *)signing, (size_t)sl, mac);
    base64url_encode(mac, sizeof(mac), sig);
    snprintf(token, sizeof(token), "%s.%s.%s", ok_hdr, payload, sig);
    TEST_ASSERT_TRUE(jwt_verify_hs256(token, strlen(token), sec(), seclen()));
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

// RFC 4648 section 5 / RFC 7515: base64url decoding must use the '-'/'_' alphabet
// only. The standard '+'/'/' characters are rejected (return 0), while the URL
// forms round-trip exactly. 0xFB,0xFF encodes to "-_8" in base64url ("+/8" in std).
void test_base64url_strict_alphabet()
{
    uint8_t out[8];
    // URL-safe characters decode.
    TEST_ASSERT_EQUAL_size_t(2, base64url_decode("-_8", 3, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xFB, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, out[1]);
    // Standard-alphabet '+' and '/' are not valid base64url -> reject.
    TEST_ASSERT_EQUAL_size_t(0, base64url_decode("+/8", 3, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, base64url_decode("ab+c", 4, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, base64url_decode("ab/c", 4, out, sizeof(out)));
    // Encode produces only the URL alphabet (no '+', '/', or '=').
    const uint8_t raw[] = {0xFB, 0xFF, 0xBF};
    char enc[8];
    size_t n = base64url_encode(raw, sizeof(raw), enc);
    TEST_ASSERT_NULL(strpbrk(enc, "+/="));
    uint8_t back[8];
    TEST_ASSERT_EQUAL_size_t(sizeof(raw), base64url_decode(enc, n, back, sizeof(back)));
    TEST_ASSERT_EQUAL_MEMORY(raw, back, sizeof(raw));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_base64url_strict_alphabet);
    RUN_TEST(test_valid_token_accepts);
    RUN_TEST(test_wrong_secret_rejects);
    RUN_TEST(test_tampered_payload_rejects);
    RUN_TEST(test_tampered_signature_rejects);
    RUN_TEST(test_malformed_rejected);
    RUN_TEST(test_alg_not_hs256_rejected);
    RUN_TEST(test_bearer_header);
    RUN_TEST(test_claim_int);
    RUN_TEST(test_claim_missing);
    RUN_TEST(test_claim_str);
    RUN_TEST(test_scope_allows);
    return UNITY_END();
}
