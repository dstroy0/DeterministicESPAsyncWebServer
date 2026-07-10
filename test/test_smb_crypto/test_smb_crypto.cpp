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
    hmac_md5(k1, sizeof(k1), (const uint8_t *)"Hi There", 8, d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("9294727a3638bb1c13f48ef8158bfc9d", hex); // RFC 2104 case 1

    hmac_md5((const uint8_t *)"Jefe", 4, (const uint8_t *)"what do ya want for nothing?", 28, d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("750c783e6ab0b503eaa86e310a5db738", hex);

    uint8_t k3[16], m3[50];
    memset(k3, 0xaa, sizeof(k3));
    memset(m3, 0xdd, sizeof(m3));
    hmac_md5(k3, sizeof(k3), m3, sizeof(m3), d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("56be34521d144c88dbb8c733f0e8b3f6", hex);

    // a key longer than the 64-byte block is hashed down first (RFC 2104)
    uint8_t klong[80];
    memset(klong, 0xaa, sizeof(klong));
    hmac_md5(klong, sizeof(klong), (const uint8_t *)"Test Using Larger Than Block-Size Key - Hash Key First", 54, d);
    to_hex(d, hex);
    TEST_ASSERT_EQUAL_STRING("6b1ab7fe4bd7bf8f0b62e6ce61b9d0cd", hex); // RFC 2202 case 6
}

// The streaming API (chunked update) must equal the one-shot.
void test_streaming_equals_oneshot()
{
    const char *s = "The quick brown fox jumps over the lazy dog";
    size_t n = strlen(s);
    uint8_t one[16], strm[16];
    md5((const uint8_t *)s, n, one);
    MdCtx c;
    md5_init(&c);
    md5_update(&c, (const uint8_t *)s, 10);
    md5_update(&c, (const uint8_t *)s + 10, 1); // odd split across the buffer boundary
    md5_update(&c, (const uint8_t *)s + 11, n - 11);
    md5_final(&c, strm);
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

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_md5_vectors);
    RUN_TEST(test_md4_vectors);
    RUN_TEST(test_hmac_md5_vectors);
    RUN_TEST(test_streaming_equals_oneshot);
    RUN_TEST(test_nt_hash);
    return UNITY_END();
}
