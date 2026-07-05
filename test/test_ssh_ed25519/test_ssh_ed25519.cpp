// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Known-answer tests for the modern SSH crypto suite (curve25519-sha256 KEX +
// ssh-ed25519 host key / client auth): SHA-512 (FIPS 180-4), X25519 (RFC 7748),
// and Ed25519 (RFC 8032). Vectors are the published RFC test vectors and
// tool-generated digests (sha512sum), so the tests ground the implementation
// against the standards, not against itself.

#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha512.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void tohex(const uint8_t *d, size_t n, char *out)
{
    static const char *h = "0123456789abcdef";
    for (size_t i = 0; i < n; i++)
    {
        out[2 * i] = h[d[i] >> 4];
        out[2 * i + 1] = h[d[i] & 0xF];
    }
    out[2 * n] = 0;
}

static int hexval(char c)
{
    return (c <= '9') ? (c - '0') : ((c | 0x20) - 'a' + 10);
}
static void fromhex(const char *h, uint8_t *out, size_t n)
{
    for (size_t i = 0; i < n; i++)
        out[i] = (uint8_t)((hexval(h[2 * i]) << 4) | hexval(h[2 * i + 1]));
}

// ---- SHA-512 (FIPS 180-4) --------------------------------------------------

static void sha512_check(const void *msg, size_t len, const char *expect)
{
    uint8_t d[SSH_SHA512_DIGEST_LEN];
    ssh_sha512((const uint8_t *)msg, len, d);
    char hex[2 * SSH_SHA512_DIGEST_LEN + 1];
    tohex(d, sizeof(d), hex);
    TEST_ASSERT_EQUAL_STRING(expect, hex);
}

void test_sha512_empty()
{
    sha512_check("", 0,
                 "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce"
                 "47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
}

void test_sha512_abc()
{
    sha512_check("abc", 3,
                 "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
                 "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
}

// 56-byte message: padding fits in one block (56 + 1 <= 112).
void test_sha512_one_block_boundary()
{
    sha512_check("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
                 "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c335"
                 "96fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445");
}

// 112-byte message: padding overflows into a second block (112 + 1 > 112).
void test_sha512_two_block_boundary()
{
    sha512_check("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
                 "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
                 112,
                 "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
                 "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909");
}

// One million 'a' bytes, fed in 1000-byte chunks: exercises many blocks + the
// streaming path + a large length field.
void test_sha512_million_a_streaming()
{
    SshSha512Ctx c;
    ssh_sha512_init(&c);
    uint8_t a[1000];
    memset(a, 'a', sizeof(a));
    for (int i = 0; i < 1000; i++)
        ssh_sha512_update(&c, a, sizeof(a));
    uint8_t d[SSH_SHA512_DIGEST_LEN];
    ssh_sha512_final(&c, d);
    char hex[2 * SSH_SHA512_DIGEST_LEN + 1];
    tohex(d, sizeof(d), hex);
    TEST_ASSERT_EQUAL_STRING("e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
                             "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b",
                             hex);
}

// Byte-at-a-time streaming must match the one-shot digest.
void test_sha512_streaming_matches_oneshot()
{
    const char *msg = "The quick brown fox jumps over the lazy dog";
    uint8_t one[SSH_SHA512_DIGEST_LEN];
    ssh_sha512((const uint8_t *)msg, strlen(msg), one);

    SshSha512Ctx c;
    ssh_sha512_init(&c);
    for (const char *p = msg; *p; p++)
        ssh_sha512_update(&c, (const uint8_t *)p, 1);
    uint8_t str[SSH_SHA512_DIGEST_LEN];
    ssh_sha512_final(&c, str);

    TEST_ASSERT_EQUAL_MEMORY(one, str, SSH_SHA512_DIGEST_LEN);
}

// ---- X25519 (RFC 7748) -----------------------------------------------------

static void x25519_check(const char *scalar_hex, const char *u_hex, const char *expect)
{
    uint8_t scalar[32], u[32], out[32];
    fromhex(scalar_hex, scalar, 32);
    fromhex(u_hex, u, 32);
    ssh_x25519(out, scalar, u);
    char hex[65];
    tohex(out, 32, hex);
    TEST_ASSERT_EQUAL_STRING(expect, hex);
}

void test_x25519_rfc7748_vector1()
{
    x25519_check("a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
                 "e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c",
                 "c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552");
}

void test_x25519_rfc7748_vector2()
{
    x25519_check("4b66e9d4d1b4673c5ad22691957d6af5c11b6421e0ea01d42ca4169e7918ba0d",
                 "e5210f12786811d3f4b7959d0538ae2c31dbe7106fc03c3efc4cd549c715a493",
                 "95cbde9476e8907d7aade45cb4b873f88b595a68799fa152e6f8f7647aac7957");
}

// RFC 7748 §5.2 iterated test: k = u = base(9); repeatedly (k, u) = (X25519(k,u), k).
static void x25519_iterate(int iters, const char *expect)
{
    uint8_t k[32] = {9}, u[32] = {9};
    for (int i = 0; i < iters; i++)
    {
        uint8_t r[32];
        ssh_x25519(r, k, u);
        memcpy(u, k, 32);
        memcpy(k, r, 32);
    }
    char hex[65];
    tohex(k, 32, hex);
    TEST_ASSERT_EQUAL_STRING(expect, hex);
}

void test_x25519_iterated_1()
{
    x25519_iterate(1, "422c8e7a6227d7bca1350b3e2bb7279f7897b87bb6854b783c60e80311ae3079");
}

void test_x25519_iterated_1000()
{
    x25519_iterate(1000, "684cf59ba83309552800ef566f2f4d3c1c3887c49360e3875f2eb94d99532c51");
}

// A Diffie-Hellman round-trip: both sides derive the same shared secret.
void test_x25519_dh_agreement()
{
    uint8_t a[32], b[32], apub[32], bpub[32], ss1[32], ss2[32];
    memset(a, 0x11, 32);
    memset(b, 0x22, 32);
    ssh_x25519_base(apub, a);
    ssh_x25519_base(bpub, b);
    ssh_x25519(ss1, a, bpub);
    ssh_x25519(ss2, b, apub);
    TEST_ASSERT_EQUAL_MEMORY(ss1, ss2, 32);
}

// ---- Ed25519 (RFC 8032) ----------------------------------------------------

// Vectors from a reference implementation (python cryptography); t2 is RFC 8032 §7.1
// TEST 2 exactly, t3 is the all-zero-seed key. Checks pubkey-from-seed, deterministic
// signing, and verification acceptance.
static void ed_check(const char *seedh, const char *msgh, const char *pubh, const char *sigh)
{
    uint8_t seed[32], pub[32], expsig[64], msg[64];
    fromhex(seedh, seed, 32);
    fromhex(pubh, pub, 32);
    fromhex(sigh, expsig, 64);
    size_t mlen = strlen(msgh) / 2;
    if (mlen)
        fromhex(msgh, msg, mlen);

    uint8_t gotpub[32];
    ssh_ed25519_pubkey(gotpub, seed);
    TEST_ASSERT_EQUAL_MEMORY(pub, gotpub, 32);

    uint8_t gotsig[64];
    ssh_ed25519_sign(gotsig, msg, mlen, seed);
    TEST_ASSERT_EQUAL_MEMORY(expsig, gotsig, 64);

    TEST_ASSERT_TRUE(ssh_ed25519_verify(pub, msg, mlen, expsig));
}

void test_ed25519_vector_empty_msg()
{
    ed_check("9d61b19deffebc3d1e2a1e6c3f2b1e1c8a20e11a3b3f2b1e6c3f2b1e6c3f2b1e", "",
             "a0627d24a3f12377d4abfb6217177eff6f6e219aab3b2dc6b900a10f273ae0e2",
             "daca64499a34c94d9c68894dff64a40b86b76b661a5cf6489c6e80933f9498b7"
             "21a854cac5683ecf9508988bfdc181839330579266c5ff3d48454527d64d130f");
}

void test_ed25519_vector_rfc8032_test2()
{
    ed_check("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb", "72",
             "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c",
             "92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da"
             "085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00");
}

void test_ed25519_vector_zero_seed()
{
    ed_check("0000000000000000000000000000000000000000000000000000000000000000", "deadbeefcafe",
             "3b6a27bcceb6a42d62a3a8d02a6f0d73653215771de243a63ac048a18b59da29",
             "1b407f4e31d6463f23e7f40174c56d2aac0bdb06ce5c5921e322e352e7b4e43c"
             "b4d0da2c3b854f0c00b09ba30445bd8eb8af623f7c32344b918f0d30ddd0590d");
}

// A tampered signature, message, or public key must be rejected.
void test_ed25519_verify_rejects_tampering()
{
    uint8_t seed[32], pub[32], sig[64], msg[3] = {'a', 'b', 'c'};
    fromhex("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb", seed, 32);
    ssh_ed25519_pubkey(pub, seed);
    ssh_ed25519_sign(sig, msg, 3, seed);
    TEST_ASSERT_TRUE(ssh_ed25519_verify(pub, msg, 3, sig));

    for (int i = 0; i < 64; i += 21) // flip a bit in R and in S
    {
        uint8_t bad[64];
        memcpy(bad, sig, 64);
        bad[i] ^= 0x01;
        TEST_ASSERT_FALSE(ssh_ed25519_verify(pub, msg, 3, bad));
    }
    uint8_t badmsg[3] = {'a', 'b', 'd'};
    TEST_ASSERT_FALSE(ssh_ed25519_verify(pub, badmsg, 3, sig));
    uint8_t badpub[32];
    memcpy(badpub, pub, 32);
    badpub[0] ^= 0x01;
    TEST_ASSERT_FALSE(ssh_ed25519_verify(badpub, msg, 3, sig));
}

// Sign/verify round-trip over a longer, multi-block message.
void test_ed25519_roundtrip_long()
{
    uint8_t seed[32];
    for (int i = 0; i < 32; i++)
        seed[i] = (uint8_t)(i * 7 + 1);
    uint8_t pub[32], sig[64], msg[200];
    for (int i = 0; i < 200; i++)
        msg[i] = (uint8_t)(i ^ 0x5a);
    ssh_ed25519_pubkey(pub, seed);
    ssh_ed25519_sign(sig, msg, sizeof(msg), seed);
    TEST_ASSERT_TRUE(ssh_ed25519_verify(pub, msg, sizeof(msg), sig));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_sha512_empty);
    RUN_TEST(test_sha512_abc);
    RUN_TEST(test_sha512_one_block_boundary);
    RUN_TEST(test_sha512_two_block_boundary);
    RUN_TEST(test_sha512_million_a_streaming);
    RUN_TEST(test_sha512_streaming_matches_oneshot);
    RUN_TEST(test_x25519_rfc7748_vector1);
    RUN_TEST(test_x25519_rfc7748_vector2);
    RUN_TEST(test_x25519_iterated_1);
    RUN_TEST(test_x25519_iterated_1000);
    RUN_TEST(test_x25519_dh_agreement);
    RUN_TEST(test_ed25519_vector_empty_msg);
    RUN_TEST(test_ed25519_vector_rfc8032_test2);
    RUN_TEST(test_ed25519_vector_zero_seed);
    RUN_TEST(test_ed25519_verify_rejects_tampering);
    RUN_TEST(test_ed25519_roundtrip_long);
    return UNITY_END();
}
