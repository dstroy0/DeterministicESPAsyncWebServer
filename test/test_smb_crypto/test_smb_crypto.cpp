// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// KAT tests for the NTLM digests (services/smb/smb_md): MD5 (RFC 1321 App A.5),
// MD4 (RFC 1320 App A.5), HMAC-MD5 (RFC 2104 / RFC 2202). MD5 + HMAC expected
// values are also cross-checked against python hashlib; MD4 against the RFC text.

#include "services/smb/smb_md.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void to_hex(const uint8_t d[16], char out[33])
{
    static const char *h = "0123456789abcdef";
    for (int i = 0; i < 16; i++)
    {
        out[i * 2] = h[d[i] >> 4];
        out[i * 2 + 1] = h[d[i] & 0xF];
    }
    out[32] = 0;
}

static void check_md5(const char *msg, const char *expect)
{
    uint8_t d[16];
    char hex[33];
    md5((const uint8_t *)msg, strlen(msg), d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING(expect, hex);
}
static void check_md4(const char *msg, const char *expect)
{
    uint8_t d[16];
    char hex[33];
    md4((const uint8_t *)msg, strlen(msg), d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING(expect, hex);
}

void test_md5_vectors()
{
    check_md5("", "d41d8cd98f00b204e9800998ecf8427e");
    check_md5("abc", "900150983cd24fb0d6963f7d28e17f72");
    check_md5("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
    check_md5("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
    // 62 bytes -> spans two 64-byte blocks (RFC 1321 A.5)
    check_md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f");
}

void test_md4_vectors()
{
    check_md4("", "31d6cfe0d16ae931b73c59d7e0c089c0");
    check_md4("a", "bde52cb31de33e46245e05fbdbd6fb24");
    check_md4("abc", "a448017aaf21d8525fc10ae87aa6729d");
    check_md4("message digest", "d9130a8164549fe818874806e1c7014b");
    check_md4("abcdefghijklmnopqrstuvwxyz", "d79e1c308aa5bbcdeea8ed63df412da9");
    check_md4("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "043f8582f241db351ce627e153e7f0e4");
}

void test_hmac_md5_vectors()
{
    uint8_t d[16];
    char hex[33];

    uint8_t k1[16];
    memset(k1, 0x0b, sizeof(k1));
    dws_hmac_md5(k1, sizeof(k1), (const uint8_t *)"Hi There", 8, d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("9294727a3638bb1c13f48ef8158bfc9d", hex); // RFC 2104 case 1

    dws_hmac_md5((const uint8_t *)"Jefe", 4, (const uint8_t *)"what do ya want for nothing?", 28, d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("750c783e6ab0b503eaa86e310a5db738", hex);

    uint8_t k3[16], m3[50];
    memset(k3, 0xaa, sizeof(k3));
    memset(m3, 0xdd, sizeof(m3));
    dws_hmac_md5(k3, sizeof(k3), m3, sizeof(m3), d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("56be34521d144c88dbb8c733f0e8b3f6", hex);

    // a key longer than the 64-byte block is hashed down first (RFC 2104)
    uint8_t klong[80];
    memset(klong, 0xaa, sizeof(klong));
    dws_hmac_md5(klong, sizeof(klong), (const uint8_t *)"Test Using Larger Than Block-Size Key - Hash Key First", 54,
                 d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("6b1ab7fe4bd7bf8f0b62e6ce61b9d0cd", hex); // RFC 2202 case 6
}

// SHA-256 known-answer vectors (FIPS 180-4), including a two-block message.
void test_sha256_vectors()
{
    uint8_t d[32];
    dws_sha256((const uint8_t *)"", 0, d);
    const uint8_t empty[32] = {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
                               0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
                               0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(empty, d, 32);

    dws_sha256((const uint8_t *)"abc", 3, d);
    const uint8_t abc[32] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
                             0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
                             0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(abc, d, 32);

    // 56-byte message: crosses the padding boundary into a second block (FIPS 180-4 two-block example).
    const char *two = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    dws_sha256((const uint8_t *)two, 56, d);
    const uint8_t twob[32] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26,
                              0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff,
                              0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(twob, d, 32);
}

// HMAC-SHA256 known-answer vectors (RFC 4231), including an over-block-size key (hashed down first).
void test_hmac_sha256_vectors()
{
    uint8_t d[32];

    uint8_t k1[20];
    memset(k1, 0x0b, sizeof(k1));
    dws_hmac_sha256(k1, sizeof(k1), (const uint8_t *)"Hi There", 8, d);
    const uint8_t tc1[32] = {0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf,
                             0xce, 0xaf, 0x0b, 0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83,
                             0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(tc1, d, 32); // RFC 4231 test case 1

    // A key longer than the 64-byte block is hashed to 32 octets first (RFC 4231 test case 6).
    uint8_t k6[131];
    memset(k6, 0xaa, sizeof(k6));
    dws_hmac_sha256(k6, sizeof(k6), (const uint8_t *)"Test Using Larger Than Block-Size Key - Hash Key First", 54, d);
    const uint8_t tc6[32] = {0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f, 0x0d, 0x8a, 0x26,
                             0xaa, 0xcb, 0xf5, 0xb7, 0x7f, 0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28,
                             0xc5, 0x14, 0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(tc6, d, 32);
}

// The streaming API (chunked update) must equal the one-shot.
void test_streaming_equals_oneshot()
{
    const char *s = "The quick brown fox jumps over the lazy dog";
    size_t n = strlen(s);
    uint8_t one[16], strm[16];
    md5((const uint8_t *)s, n, one);
    MdCtx c;
    dws_md5_init(&c);
    dws_md5_update(&c, (const uint8_t *)s, 10);
    dws_md5_update(&c, (const uint8_t *)s + 10, 1); // odd split across the buffer boundary
    dws_md5_update(&c, (const uint8_t *)s + 11, n - 11);
    dws_md5_final(&c, strm);
    TEST_ASSERT_EQUAL_MEMORY(one, strm, 16);
}

// The NT hash: MD4 of the UTF-16LE password (MS-NLMP). Spot-check "password".
void test_nt_hash()
{
    const char *pw = "password";
    uint8_t utf16[16];
    for (int i = 0; i < 8; i++)
    {
        utf16[i * 2] = (uint8_t)pw[i];
        utf16[i * 2 + 1] = 0;
    }
    uint8_t nt[16];
    char hex[33];
    md4(utf16, sizeof(utf16), nt);
    to_hex(nt, hex);
    TEST_ASSERT_EQUAL_STRING("8846f7eaee8fb117ad06bdd830b7586c", hex); // the well-known NT hash of "password"
}

// SP800-108 counter-mode KDF (HMAC-SHA256, BEFORE_FIXED, RLEN=32) vs the NIST CAVP KBKDF (KDFCTR)
// vectors - the SMB 3.x key-derivation primitive (MS-SMB2 §3.1.4.2).
void test_kdf_ctr_hmac_sha256_nist()
{
    // NIST CAVP KDFCTR COUNT=0, L=128 (a single HMAC block).
    const uint8_t ki1[32] = {0xdd, 0x1d, 0x91, 0xb7, 0xd9, 0x0b, 0x2b, 0xd3, 0x13, 0x85, 0x33,
                             0xce, 0x92, 0xb2, 0x72, 0xfb, 0xf8, 0xa3, 0x69, 0x31, 0x6a, 0xef,
                             0xe2, 0x42, 0xe6, 0x59, 0xcc, 0x0a, 0xe2, 0x38, 0xaf, 0xe0};
    const uint8_t fix1[60] = {0x01, 0x32, 0x2b, 0x96, 0xb3, 0x0a, 0xcd, 0x19, 0x79, 0x79, 0x44, 0x4e, 0x46, 0x8e, 0x1c,
                              0x5c, 0x68, 0x59, 0xbf, 0x1b, 0x1c, 0xf9, 0x51, 0xb7, 0xe7, 0x25, 0x30, 0x3e, 0x23, 0x7e,
                              0x46, 0xb8, 0x64, 0xa1, 0x45, 0xfa, 0xb2, 0x5e, 0x51, 0x7b, 0x08, 0xf8, 0x68, 0x3d, 0x03,
                              0x15, 0xbb, 0x29, 0x11, 0xd8, 0x0a, 0x0e, 0x8a, 0xba, 0x17, 0xf3, 0xb4, 0x13, 0xfa, 0xac};
    const uint8_t ko1[16] = {0x10, 0x62, 0x13, 0x42, 0xbf, 0xb0, 0xfd, 0x40,
                             0x04, 0x6c, 0x0e, 0x29, 0xf2, 0xcf, 0xdb, 0xf0};
    uint8_t out1[16] = {0};
    TEST_ASSERT_TRUE(dws_kdf_ctr_hmac_sha256(ki1, sizeof(ki1), fix1, sizeof(fix1), out1, sizeof(out1)));
    TEST_ASSERT_EQUAL_MEMORY(ko1, out1, 16);

    // NIST CAVP KDFCTR COUNT=30, L=320 (40 bytes: two HMAC blocks, exercises the counter loop + truncation).
    const uint8_t ki2[32] = {0xc4, 0xbe, 0xdb, 0xdd, 0xb6, 0x64, 0x93, 0xe7, 0xc7, 0x25, 0x9a,
                             0x3b, 0xbb, 0xc2, 0x5f, 0x8c, 0x7e, 0x0c, 0xa7, 0xfe, 0x28, 0x4d,
                             0x92, 0xd4, 0x31, 0xd9, 0xcd, 0x99, 0xa0, 0xd2, 0x14, 0xac};
    const uint8_t fix2[60] = {0x1c, 0x69, 0xc5, 0x47, 0x66, 0x79, 0x1e, 0x31, 0x5c, 0x2c, 0xc5, 0xc4, 0x7e, 0xcd, 0x3f,
                              0xfa, 0xb8, 0x7d, 0x0d, 0x27, 0x3d, 0xd9, 0x20, 0xe7, 0x09, 0x55, 0x81, 0x4c, 0x22, 0x0e,
                              0xac, 0xac, 0xe6, 0xa5, 0x94, 0x65, 0x42, 0xda, 0x3d, 0xfe, 0x24, 0xff, 0x62, 0x6b, 0x48,
                              0x97, 0x89, 0x8c, 0xaf, 0xb7, 0xdb, 0x83, 0xbd, 0xff, 0x3c, 0x14, 0xfa, 0x46, 0xfd, 0x4b};
    const uint8_t ko2[40] = {0x1d, 0xa4, 0x76, 0x38, 0xd6, 0xc9, 0xc4, 0xd0, 0x4d, 0x74, 0xd4, 0x64, 0x0b, 0xbd,
                             0x42, 0xab, 0x81, 0x4d, 0x9e, 0x8c, 0xc2, 0x2f, 0x43, 0x26, 0x69, 0x52, 0x39, 0xf9,
                             0x6b, 0x06, 0x93, 0xf1, 0x2d, 0x0d, 0xd1, 0x15, 0x2c, 0xf4, 0x44, 0x30};
    uint8_t out2[40] = {0};
    TEST_ASSERT_TRUE(dws_kdf_ctr_hmac_sha256(ki2, sizeof(ki2), fix2, sizeof(fix2), out2, sizeof(out2)));
    TEST_ASSERT_EQUAL_MEMORY(ko2, out2, 40);

    // fail-closed on bad args
    uint8_t tmp[16] = {0};
    TEST_ASSERT_FALSE(dws_kdf_ctr_hmac_sha256(nullptr, 32, fix1, sizeof(fix1), tmp, sizeof(tmp)));
    TEST_ASSERT_FALSE(dws_kdf_ctr_hmac_sha256(ki1, sizeof(ki1), nullptr, 0, tmp, sizeof(tmp)));
    TEST_ASSERT_FALSE(dws_kdf_ctr_hmac_sha256(ki1, sizeof(ki1), fix1, sizeof(fix1), tmp, 0));
}

// SHA-512 known-answer vectors (FIPS 180-4), including the 112-byte two-block example.
void test_sha512_vectors()
{
    uint8_t d[64];
    dws_sha512((const uint8_t *)"", 0, d);
    const uint8_t empty[64] = {0xcf, 0x83, 0xe1, 0x35, 0x7e, 0xef, 0xb8, 0xbd, 0xf1, 0x54, 0x28, 0x50, 0xd6,
                               0x6d, 0x80, 0x07, 0xd6, 0x20, 0xe4, 0x05, 0x0b, 0x57, 0x15, 0xdc, 0x83, 0xf4,
                               0xa9, 0x21, 0xd3, 0x6c, 0xe9, 0xce, 0x47, 0xd0, 0xd1, 0x3c, 0x5d, 0x85, 0xf2,
                               0xb0, 0xff, 0x83, 0x18, 0xd2, 0x87, 0x7e, 0xec, 0x2f, 0x63, 0xb9, 0x31, 0xbd,
                               0x47, 0x41, 0x7a, 0x81, 0xa5, 0x38, 0x32, 0x7a, 0xf9, 0x27, 0xda, 0x3e};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(empty, d, 64);

    dws_sha512((const uint8_t *)"abc", 3, d);
    const uint8_t abc[64] = {0xdd, 0xaf, 0x35, 0xa1, 0x93, 0x61, 0x7a, 0xba, 0xcc, 0x41, 0x73, 0x49, 0xae,
                             0x20, 0x41, 0x31, 0x12, 0xe6, 0xfa, 0x4e, 0x89, 0xa9, 0x7e, 0xa2, 0x0a, 0x9e,
                             0xee, 0xe6, 0x4b, 0x55, 0xd3, 0x9a, 0x21, 0x92, 0x99, 0x2a, 0x27, 0x4f, 0xc1,
                             0xa8, 0x36, 0xba, 0x3c, 0x23, 0xa3, 0xfe, 0xeb, 0xbd, 0x45, 0x4d, 0x44, 0x23,
                             0x64, 0x3c, 0xe8, 0x0e, 0x2a, 0x9a, 0xc9, 0x4f, 0xa5, 0x4c, 0xa4, 0x9f};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(abc, d, 64);

    // 112-byte message: spans two 128-byte blocks (FIPS 180-4 two-block example).
    const char *two = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
                      "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    dws_sha512((const uint8_t *)two, 112, d);
    const uint8_t twob[64] = {0x8e, 0x95, 0x9b, 0x75, 0xda, 0xe3, 0x13, 0xda, 0x8c, 0xf4, 0xf7, 0x28, 0x14,
                              0xfc, 0x14, 0x3f, 0x8f, 0x77, 0x79, 0xc6, 0xeb, 0x9f, 0x7f, 0xa1, 0x72, 0x99,
                              0xae, 0xad, 0xb6, 0x88, 0x90, 0x18, 0x50, 0x1d, 0x28, 0x9e, 0x49, 0x00, 0xf7,
                              0xe4, 0x33, 0x1b, 0x99, 0xde, 0xc4, 0xb5, 0x43, 0x3a, 0xc7, 0xd3, 0x29, 0xee,
                              0xb6, 0xdd, 0x26, 0x54, 0x5e, 0x96, 0xe5, 0x5b, 0x87, 0x4b, 0xe9, 0x09};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(twob, d, 64);

    // streaming (odd chunk splits) must equal the one-shot
    Sha512Ctx sc;
    uint8_t strm[64];
    dws_sha512_init(&sc);
    dws_sha512_update(&sc, (const uint8_t *)two, 5);
    dws_sha512_update(&sc, (const uint8_t *)two + 5, 100);
    dws_sha512_update(&sc, (const uint8_t *)two + 105, 7);
    dws_sha512_final(&sc, strm);
    TEST_ASSERT_EQUAL_MEMORY(twob, strm, 64);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_md5_vectors);
    RUN_TEST(test_md4_vectors);
    RUN_TEST(test_hmac_md5_vectors);
    RUN_TEST(test_sha256_vectors);
    RUN_TEST(test_sha512_vectors);
    RUN_TEST(test_hmac_sha256_vectors);
    RUN_TEST(test_streaming_equals_oneshot);
    RUN_TEST(test_nt_hash);
    RUN_TEST(test_kdf_ctr_hmac_sha256_nist);
    return UNITY_END();
}
