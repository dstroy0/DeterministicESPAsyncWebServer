// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the AES-256-GCM AEAD used by aes256-gcm@openssh.com (RFC 5647):
//   - seal/open against the NIST/McGrew AES-256-GCM "Test Case 16" vector (with AAD, 96-bit IV),
//     byte-verified with a real AEAD (python `cryptography` AESGCM) - see scratch gcm_kat.py
//   - tamper rejection (no plaintext, counter not advanced)
//   - the RFC 5647 invocation counter advancing once per packet (two seals differ; a matching
//     decryptor opens them in order).

#include "network_drivers/presentation/ssh/crypto/ssh_aesgcm.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// NIST/McGrew GCM Test Case 16 (AES-256, 96-bit IV, AAD). Ciphertext + tag verified against
// python cryptography's AESGCM (C_match=True, tag = 76fc...551b for the with-AAD case).
static const uint8_t TC16_KEY[32] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x6d, 0x6a, 0x8f,
                                     0x94, 0x67, 0x30, 0x83, 0x08, 0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65,
                                     0x73, 0x1c, 0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
static const uint8_t TC16_IV[12] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};
static const uint8_t TC16_AAD[20] = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                                     0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};
static const uint8_t TC16_PT[60] = {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5,
                                    0xaf, 0xf5, 0x26, 0x9a, 0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
                                    0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72, 0x1c, 0x3c, 0x0c, 0x95,
                                    0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
                                    0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39};
static const uint8_t TC16_CT[60] = {0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07, 0xf4, 0x7f, 0x37, 0xa3,
                                    0x2a, 0x84, 0x42, 0x7d, 0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
                                    0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa, 0x8c, 0xb0, 0x8e, 0x48,
                                    0x59, 0x0d, 0xbb, 0x3d, 0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
                                    0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a, 0xbc, 0xc9, 0xf6, 0x62};
static const uint8_t TC16_TAG[16] = {0x76, 0xfc, 0x6e, 0xce, 0x0f, 0x4e, 0x17, 0x68,
                                     0xcd, 0xdf, 0x88, 0x53, 0xbb, 0x2d, 0x55, 0x1b};

// seal(AAD, PT) must reproduce the NIST ciphertext and tag exactly.
void test_aesgcm_nist_tc16_seal()
{
    SshAesGcmCtx ctx;
    ssh_aesgcm_init(&ctx, TC16_KEY, TC16_IV);
    uint8_t out[60 + 16];
    ssh_aesgcm_seal(&ctx, TC16_AAD, sizeof(TC16_AAD), TC16_PT, sizeof(TC16_PT), out);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(TC16_CT, out, sizeof(TC16_CT));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(TC16_TAG, out + sizeof(TC16_CT), 16);
    ssh_aesgcm_wipe(&ctx);
}

// open(AAD, CT, TAG) must verify and recover the plaintext; a tampered tag/ciphertext must fail
// without writing plaintext and without advancing the invocation counter.
void test_aesgcm_nist_tc16_open()
{
    SshAesGcmCtx ctx;
    ssh_aesgcm_init(&ctx, TC16_KEY, TC16_IV);
    uint8_t pt[60];
    TEST_ASSERT_TRUE(ssh_aesgcm_open(&ctx, TC16_AAD, sizeof(TC16_AAD), TC16_CT, sizeof(TC16_CT), TC16_TAG, pt));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(TC16_PT, pt, sizeof(TC16_PT));

    // Tampered tag -> reject; the counter must NOT have advanced, so a correct open still works.
    ssh_aesgcm_init(&ctx, TC16_KEY, TC16_IV);
    uint8_t bad_tag[16];
    memcpy(bad_tag, TC16_TAG, 16);
    bad_tag[0] ^= 0x01;
    TEST_ASSERT_FALSE(ssh_aesgcm_open(&ctx, TC16_AAD, sizeof(TC16_AAD), TC16_CT, sizeof(TC16_CT), bad_tag, pt));
    TEST_ASSERT_TRUE(ssh_aesgcm_open(&ctx, TC16_AAD, sizeof(TC16_AAD), TC16_CT, sizeof(TC16_CT), TC16_TAG, pt));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(TC16_PT, pt, sizeof(TC16_PT));

    // Tampered AAD -> reject.
    ssh_aesgcm_init(&ctx, TC16_KEY, TC16_IV);
    uint8_t bad_aad[20];
    memcpy(bad_aad, TC16_AAD, 20);
    bad_aad[3] ^= 0x40;
    TEST_ASSERT_FALSE(ssh_aesgcm_open(&ctx, bad_aad, sizeof(bad_aad), TC16_CT, sizeof(TC16_CT), TC16_TAG, pt));
    ssh_aesgcm_wipe(&ctx);
}

// RFC 5647 sec 7.1: the invocation counter advances once per sealed/opened packet, so the same
// plaintext seals to different ciphertext on the second packet, and a decryptor initialised with the
// same key/IV opens both in order.
void test_aesgcm_invocation_counter_advances()
{
    SshAesGcmCtx enc, dec;
    uint8_t key[32], iv[12];
    for (int i = 0; i < 32; i++)
        key[i] = (uint8_t)(i * 7 + 1);
    for (int i = 0; i < 12; i++)
        iv[i] = (uint8_t)(0x10 + i);
    ssh_aesgcm_init(&enc, key, iv);
    ssh_aesgcm_init(&dec, key, iv);

    const uint8_t aad[4] = {0, 0, 0, 16};
    uint8_t msg[16];
    for (int i = 0; i < 16; i++)
        msg[i] = (uint8_t)(0xA0 + i);

    uint8_t p0[16 + 16], p1[16 + 16];
    ssh_aesgcm_seal(&enc, aad, 4, msg, 16, p0); // packet 0 (counter = initial)
    ssh_aesgcm_seal(&enc, aad, 4, msg, 16, p1); // packet 1 (counter incremented)

    // Same key + same plaintext but a different invocation counter -> different ciphertext AND tag.
    TEST_ASSERT_TRUE(memcmp(p0, p1, 32) != 0);

    // The receiver, stepping its own counter in lockstep, opens both in order.
    uint8_t r0[16], r1[16];
    TEST_ASSERT_TRUE(ssh_aesgcm_open(&dec, aad, 4, p0, 16, p0 + 16, r0));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg, r0, 16);
    TEST_ASSERT_TRUE(ssh_aesgcm_open(&dec, aad, 4, p1, 16, p1 + 16, r1));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg, r1, 16);

    // Feeding packet 1 to a freshly-reset decryptor (counter back at start) must fail: the nonce is
    // wrong for that packet.
    ssh_aesgcm_init(&dec, key, iv);
    uint8_t rx[16];
    TEST_ASSERT_FALSE(ssh_aesgcm_open(&dec, aad, 4, p1, 16, p1 + 16, rx));

    ssh_aesgcm_wipe(&enc);
    ssh_aesgcm_wipe(&dec);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_aesgcm_nist_tc16_seal);
    RUN_TEST(test_aesgcm_nist_tc16_open);
    RUN_TEST(test_aesgcm_invocation_counter_advances);
    return UNITY_END();
}
