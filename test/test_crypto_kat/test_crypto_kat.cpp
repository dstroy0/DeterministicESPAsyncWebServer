// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Data-driven external known-answer tests (KAT) for the library's crypto
// primitives. Every vector here was produced OUTSIDE this codebase - Project
// Wycheproof (adversarial edge cases: wrong tags, modified IVs, low-order
// points, signature malleability) and the RFC appendix vectors - so these fail
// if a primitive drifts from the standard even when the self-referential
// protocol tests still pass.
//
// The vectors live as auditable JSON under test/vectors/ (curated by
// tools/curate_crypto_vectors.py) and are compiled to the tables below by
// tools/gen_crypto_vectors.py -> kat_data.inc. To refresh: re-run those tools.
//
// Note: the "private" scalars in the X25519 vectors are published test inputs,
// not secrets - a known-answer test is meaningless if its inputs are not fixed.
// Ephemeral, per-run keys belong in the handshake/round-trip tests, not here.

#include "network_drivers/presentation/http3/quic_aead.h"
#include "network_drivers/presentation/http3/quic_hkdf.h"
#include "network_drivers/presentation/ssh/crypto/ssh_chacha20.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha512.h"
#include "network_drivers/presentation/ssh/crypto/ssh_poly1305.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

// --- Vector table row layouts (kat_data.inc initializes these) --------------
struct KatMac
{
    int tc;
    const char *key;
    const char *msg;
    const char *tag;
    int tag_bits;
    int valid;
};
struct KatAead
{
    int tc;
    const char *key;
    const char *iv;
    const char *aad;
    const char *msg;
    const char *ct;
    const char *tag;
    int valid;
};
struct KatX25519
{
    int tc;
    const char *pub;
    const char *priv;
    const char *shared;
};
struct KatEd25519
{
    int tc;
    const char *pub;
    const char *msg;
    const char *sig;
    int valid;
};
struct KatHkdf
{
    int tc;
    const char *salt;
    const char *ikm;
    const char *prk;
};
struct KatChacha
{
    int tc;
    const char *key;
    uint32_t counter;
    const char *nonce;
    const char *keystream;
};
struct KatPoly
{
    int tc;
    const char *key;
    const char *msg;
    const char *tag;
};

#include "kat_data.inc"

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define MAXB 2048 // largest vector field (ed25519 msg is 1023 bytes)

// Decode a lowercase-hex C-string into @p out; returns the byte length.
static size_t hexdec(const char *h, uint8_t *out)
{
    size_t n = 0;
    for (const char *p = h; p[0] && p[1]; p += 2)
    {
        auto nib = [](char c) -> int {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            return c - 'A' + 10;
        };
        out[n++] = (uint8_t)((nib(p[0]) << 4) | nib(p[1]));
    }
    return n;
}

void setUp()
{
}
void tearDown()
{
}

// ====================================================================
// HMAC (RFC 4231 / Wycheproof): compute the full MAC, compare the first
// tag_bits/8 bytes to the vector; valid must match, invalid must not.
// ====================================================================
static void run_hmac(const KatMac *arr, size_t n, bool is512)
{
    for (size_t i = 0; i < n; i++)
    {
        const KatMac &v = arr[i];
        uint8_t key[MAXB], msg[MAXB], want[64], got[64];
        size_t klen = hexdec(v.key, key), mlen = hexdec(v.msg, msg), wlen = hexdec(v.tag, want);
        if (is512)
            ssh_hmac_sha512(key, klen, msg, mlen, got);
        else
            ssh_hmac_sha256(key, klen, msg, mlen, got);
        size_t cmp = (size_t)v.tag_bits / 8; // truncated-tag length the vector pins
        char m[64];
        snprintf(m, sizeof(m), "HMAC%s tcId=%d", is512 ? "512" : "256", v.tc);
        bool match = (wlen == cmp) && memcmp(got, want, cmp) == 0;
        if (v.valid)
            TEST_ASSERT_TRUE_MESSAGE(match, m);
        else
            TEST_ASSERT_FALSE_MESSAGE(match, m);
    }
}
static void test_hmac_sha256(void)
{
    run_hmac(KAT_HMAC_SHA256, ARRAY_LEN(KAT_HMAC_SHA256), false);
}
static void test_hmac_sha512(void)
{
    run_hmac(KAT_HMAC_SHA512, ARRAY_LEN(KAT_HMAC_SHA512), true);
}

// ====================================================================
// AEAD_AES_128_GCM (Wycheproof / RFC 9001): seal must reproduce ct||tag and
// open must recover the plaintext; a one-bit tag flip must be rejected.
// ====================================================================
static void test_aes128gcm(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_AES128GCM); i++)
    {
        const KatAead &v = KAT_AES128GCM[i];
        uint8_t key[16], iv[12], aad[MAXB], pt[MAXB], ct[MAXB], tag[16];
        uint8_t sealed[MAXB + 16], opened[MAXB];
        hexdec(v.key, key);
        hexdec(v.iv, iv);
        size_t alen = hexdec(v.aad, aad), plen = hexdec(v.msg, pt);
        size_t clen = hexdec(v.ct, ct);
        hexdec(v.tag, tag);
        char m[48];
        snprintf(m, sizeof(m), "AES128GCM tcId=%d", v.tc);

        // seal: out == ciphertext || tag (ciphertext is empty when plaintext is)
        quic_aes128_gcm_seal(key, iv, alen ? aad : nullptr, alen, plen ? pt : nullptr, plen, sealed);
        if (clen)
            TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(ct, sealed, clen, m);
        TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(tag, sealed + plen, 16, m);

        // open: recovers the plaintext and authenticates
        bool ok = quic_aes128_gcm_open(key, iv, alen ? aad : nullptr, alen, sealed, plen + 16, opened);
        TEST_ASSERT_TRUE_MESSAGE(ok, m);
        if (plen)
            TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(pt, opened, plen, m);

        // negative: a flipped tag byte must fail authentication
        sealed[plen + 15] ^= 0x80;
        TEST_ASSERT_FALSE_MESSAGE(quic_aes128_gcm_open(key, iv, alen ? aad : nullptr, alen, sealed, plen + 16, opened),
                                  m);
    }
}

// ====================================================================
// X25519 (RFC 7748 / Wycheproof): scalar*point is deterministic, so the
// computed shared secret must equal the vector for valid and acceptable alike.
// ====================================================================
static void test_x25519(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_X25519); i++)
    {
        const KatX25519 &v = KAT_X25519[i];
        uint8_t pub[32], priv[32], want[32], got[32];
        hexdec(v.pub, pub);
        hexdec(v.priv, priv);
        size_t wlen = hexdec(v.shared, want);
        ssh_x25519(got, priv, pub);
        char m[48];
        snprintf(m, sizeof(m), "X25519 tcId=%d", v.tc);
        TEST_ASSERT_EQUAL_MESSAGE(32, wlen, m);
        TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(want, got, 32, m);
    }
}

// ====================================================================
// Ed25519 verify (RFC 8032 / Wycheproof): valid signatures verify, invalid
// ones (including wrong-length encodings, which a caller rejects up front) fail.
// ====================================================================
static void test_ed25519_verify(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_ED25519); i++)
    {
        const KatEd25519 &v = KAT_ED25519[i];
        uint8_t pub[32], msg[MAXB], sig[64];
        hexdec(v.pub, pub);
        size_t mlen = hexdec(v.msg, msg), slen = hexdec(v.sig, sig);
        char m[48];
        snprintf(m, sizeof(m), "Ed25519 tcId=%d", v.tc);
        if (slen != 64)
        {
            // A non-64-byte signature is malformed; the framing rejects it before
            // the crypto ever runs. Such vectors are all "invalid".
            TEST_ASSERT_FALSE_MESSAGE(v.valid, m);
            continue;
        }
        bool ok = ssh_ed25519_verify(pub, msg, mlen, sig);
        TEST_ASSERT_EQUAL_MESSAGE(v.valid ? true : false, ok, m);
    }
}

// ====================================================================
// HKDF-SHA256 Extract (RFC 5869 Appendix A): PRK = HMAC-SHA256(salt, IKM).
// ====================================================================
static void test_hkdf_extract(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_HKDF_EXTRACT); i++)
    {
        const KatHkdf &v = KAT_HKDF_EXTRACT[i];
        uint8_t salt[MAXB], ikm[MAXB], want[32], got[32];
        size_t slen = hexdec(v.salt, salt), ilen = hexdec(v.ikm, ikm);
        hexdec(v.prk, want);
        quic_hkdf_extract(slen ? salt : nullptr, slen, ikm, ilen, got);
        char m[48];
        snprintf(m, sizeof(m), "HKDF-Extract tcId=%d", v.tc);
        TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(want, got, 32, m);
    }
}

// ====================================================================
// ChaCha20 block (RFC 8439 sec 2.4.2) and Poly1305 (sec 2.5.2).
// ====================================================================
static void test_chacha20_block(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_CHACHA20); i++)
    {
        const KatChacha &v = KAT_CHACHA20[i];
        uint8_t key[32], nonce[12], want[64], got[64];
        hexdec(v.key, key);
        hexdec(v.nonce, nonce);
        hexdec(v.keystream, want);
        ssh_chacha20_block_ietf(key, v.counter, nonce, got);
        char m[48];
        snprintf(m, sizeof(m), "ChaCha20 tcId=%d", v.tc);
        TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(want, got, 64, m);
    }
}
static void test_poly1305(void)
{
    for (size_t i = 0; i < ARRAY_LEN(KAT_POLY1305); i++)
    {
        const KatPoly &v = KAT_POLY1305[i];
        uint8_t key[32], msg[MAXB], want[16], got[16];
        hexdec(v.key, key);
        size_t mlen = hexdec(v.msg, msg);
        hexdec(v.tag, want);
        ssh_poly1305(got, msg, mlen, key);
        char m[48];
        snprintf(m, sizeof(m), "Poly1305 tcId=%d", v.tc);
        TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(want, got, 16, m);
    }
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_hmac_sha256);
    RUN_TEST(test_hmac_sha512);
    RUN_TEST(test_aes128gcm);
    RUN_TEST(test_x25519);
    RUN_TEST(test_ed25519_verify);
    RUN_TEST(test_hkdf_extract);
    RUN_TEST(test_chacha20_block);
    RUN_TEST(test_poly1305);
    return UNITY_END();
}
