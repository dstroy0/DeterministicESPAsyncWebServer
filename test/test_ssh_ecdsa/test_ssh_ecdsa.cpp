// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// ECDSA P-256 (ecdsa-sha2-nistp256) native software-path tests. Correctness is pinned
// to the RFC 6979 Appendix A.2.5 (P-256, SHA-256) deterministic known-answer vectors:
// the same private key, public point, and the exact (r, s) for messages "sample" and
// "test". A byte-exact deterministic signature proves the whole stack - field/scalar
// arithmetic, Jacobian point math, scalar multiplication, and RFC 6979 nonce generation.

#include "network_drivers/presentation/ssh/crypto/ssh_ecdsa.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static size_t hexdec(const char *h, uint8_t *out)
{
    size_t n = 0;
    for (; h[0] && h[1]; h += 2)
    {
        auto nib = [](char c) -> int { return c >= 'a' ? c - 'a' + 10 : (c >= 'A' ? c - 'A' + 10 : c - '0'); };
        out[n++] = (uint8_t)((nib(h[0]) << 4) | nib(h[1]));
    }
    return n;
}

// RFC 6979 A.2.5 (curve P-256).
static const char *PRIV = "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721";
static const char *UX = "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6";
static const char *UY = "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299";
// message "sample", SHA-256
static const char *SAMPLE_R = "EFD48B2AACB6A8FD1140DD9CD45E81D69D2C877B56AAF991C34D0EA84EAF3716";
static const char *SAMPLE_S = "F7CB1C942D657C41D436C7A1B6E29F65F3E900DBB9AFF4064DC4AB2F843ACDA8";
// message "test", SHA-256
static const char *TEST_R = "F1ABB023518351CD71D881567B1EA663ED3EFCF6C5132B354F28D3B0B7D38367";
static const char *TEST_S = "019F4113742A2B14BD25926B49C649155F267E60D3814B4C0CC84250E46F0083";

static void test_ecdsa_pubkey_matches_rfc6979(void)
{
    uint8_t priv[32];
    hexdec(PRIV, priv);
    uint8_t pub[65];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_pubkey(pub, priv));
    TEST_ASSERT_EQUAL_UINT8(0x04, pub[0]);
    uint8_t ux[32], uy[32];
    hexdec(UX, ux);
    hexdec(UY, uy);
    TEST_ASSERT_EQUAL_MEMORY(ux, pub + 1, 32);
    TEST_ASSERT_EQUAL_MEMORY(uy, pub + 33, 32);
}

// The deterministic (RFC 6979) signature must be byte-exact against the published vector.
static void test_ecdsa_sign_deterministic_sample(void)
{
    uint8_t priv[32];
    hexdec(PRIV, priv);
    uint8_t sig[64];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_sign(sig, (const uint8_t *)"sample", 6, priv));
    uint8_t r[32], s[32];
    hexdec(SAMPLE_R, r);
    hexdec(SAMPLE_S, s);
    TEST_ASSERT_EQUAL_MEMORY(r, sig, 32);
    TEST_ASSERT_EQUAL_MEMORY(s, sig + 32, 32);
}

static void test_ecdsa_sign_deterministic_test(void)
{
    uint8_t priv[32];
    hexdec(PRIV, priv);
    uint8_t sig[64];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_sign(sig, (const uint8_t *)"test", 4, priv));
    uint8_t r[32], s[32];
    hexdec(TEST_R, r);
    hexdec(TEST_S, s);
    TEST_ASSERT_EQUAL_MEMORY(r, sig, 32);
    TEST_ASSERT_EQUAL_MEMORY(s, sig + 32, 32);
}

static void test_ecdsa_verify_valid(void)
{
    uint8_t priv[32];
    hexdec(PRIV, priv);
    uint8_t pub[65];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_pubkey(pub, priv));

    uint8_t sig[64];
    hexdec(SAMPLE_R, sig);
    hexdec(SAMPLE_S, sig + 32);
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"sample", 6, sig));

    hexdec(TEST_R, sig);
    hexdec(TEST_S, sig + 32);
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"test", 4, sig));
}

static void test_ecdsa_verify_rejects_tamper(void)
{
    uint8_t priv[32];
    hexdec(PRIV, priv);
    uint8_t pub[65];
    ssh_ecdsa_p256_pubkey(pub, priv);

    uint8_t sig[64];
    hexdec(SAMPLE_R, sig);
    hexdec(SAMPLE_S, sig + 32);

    // Wrong message (the "test" signature under the "sample" message).
    uint8_t sig_test[64];
    hexdec(TEST_R, sig_test);
    hexdec(TEST_S, sig_test + 32);
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"sample", 6, sig_test));

    // Tampered signature (flip one bit of s).
    sig[63] ^= 0x01;
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"sample", 6, sig));

    // Tampered public key (flip one bit of X -> off curve / wrong key).
    hexdec(SAMPLE_R, sig);
    hexdec(SAMPLE_S, sig + 32);
    pub[1] ^= 0x01;
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"sample", 6, sig));
}

// A fresh key round-trips (exercises sign -> verify with a non-vector key).
static void test_ecdsa_roundtrip_other_key(void)
{
    uint8_t priv[32];
    memset(priv, 0, 32);
    priv[31] = 0x42; // d = 0x42
    uint8_t pub[65];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_pubkey(pub, priv));
    const uint8_t msg[] = "deterministic ecdsa round trip";
    uint8_t sig[64];
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_sign(sig, msg, sizeof(msg) - 1, priv));
    TEST_ASSERT_TRUE(ssh_ecdsa_p256_verify(pub, msg, sizeof(msg) - 1, sig));
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_verify(pub, (const uint8_t *)"other message", 13, sig));
}

// Invalid keys are rejected.
static void test_ecdsa_pubkey_rejects_bad_scalar(void)
{
    uint8_t priv[32];
    uint8_t pub[65];
    memset(priv, 0, 32);
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_pubkey(pub, priv)); // d = 0
    memset(priv, 0xFF, 32);
    TEST_ASSERT_FALSE(ssh_ecdsa_p256_pubkey(pub, priv)); // d = 2^256-1 > n
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_ecdsa_pubkey_matches_rfc6979);
    RUN_TEST(test_ecdsa_sign_deterministic_sample);
    RUN_TEST(test_ecdsa_sign_deterministic_test);
    RUN_TEST(test_ecdsa_verify_valid);
    RUN_TEST(test_ecdsa_verify_rejects_tamper);
    RUN_TEST(test_ecdsa_roundtrip_other_key);
    RUN_TEST(test_ecdsa_pubkey_rejects_bad_scalar);
    return UNITY_END();
}
