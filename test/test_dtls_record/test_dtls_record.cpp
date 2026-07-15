// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DTLS 1.3 record layer tests (RFC 9147 §4). The record + key derivation is pinned byte-for-byte
// to an INDEPENDENT reconstruction (test/scratch dtls_kat.py): HKDF-Expand-Label via stdlib
// hmac/hashlib, AEAD_AES_128_GCM + AES-ECB via the `cryptography` library. A byte-exact match of
// the whole DTLSCiphertext record proves the unified header, the AEAD nonce (§4.2.2), the AAD
// selection, and the sequence-number encryption (§4.2.3) are all assembled correctly.

#include "network_drivers/presentation/dtls/dtls_record.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ---- KAT (dtls_kat.py): secret 0x00..0x1f, epoch 3, seq 5, content_type 23, "hello dtls 1.3" ----
static const uint8_t KAT_SECRET[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
static const uint8_t KAT_KEY[16] = {0x9c, 0x97, 0x83, 0xcf, 0x77, 0xea, 0x32, 0xd4,
                                    0x4f, 0x36, 0x9d, 0xa4, 0x1f, 0x19, 0xf3, 0xcc};
static const uint8_t KAT_IV[12] = {0x2f, 0x41, 0xc8, 0x46, 0xa4, 0x31, 0xa1, 0x63, 0x81, 0x4b, 0xcd, 0x71};
static const uint8_t KAT_SN[16] = {0x50, 0x1c, 0x58, 0xbb, 0x54, 0x33, 0x79, 0x8c,
                                   0xc5, 0xd7, 0x89, 0xca, 0xfa, 0xbc, 0xde, 0x6d};
static const uint8_t KAT_WIRE[36] = {0x2f, 0x54, 0x06, 0x00, 0x1f, 0x09, 0xf7, 0xc0, 0x34, 0x70, 0xed, 0x00,
                                     0x93, 0x31, 0x9a, 0xac, 0xaf, 0x04, 0x7a, 0x29, 0x0f, 0x77, 0xcd, 0x1c,
                                     0x30, 0x12, 0xee, 0x2a, 0x86, 0x44, 0xeb, 0x53, 0x7c, 0xf9, 0xc6, 0xa6};
static const char *KAT_PLAINTEXT = "hello dtls 1.3";
static const uint16_t KAT_EPOCH = 3;
static const uint64_t KAT_SEQ = 5;
static const uint8_t KAT_CT = DTLS_CT_APPLICATION_DATA; // 23

// The record keys are derived from the traffic secret exactly like the independent tool.
static void test_dtls_record_keys_derive_kat(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, KAT_EPOCH, KAT_SECRET);
    TEST_ASSERT_EQUAL_MEMORY(KAT_KEY, k.key, 16);
    TEST_ASSERT_EQUAL_MEMORY(KAT_IV, k.iv, 12);
    TEST_ASSERT_EQUAL_MEMORY(KAT_SN, k.sn_key, 16);
    TEST_ASSERT_EQUAL_UINT16(KAT_EPOCH, k.epoch);
}

// dtls_ciphertext_protect must produce the exact on-wire record byte-for-byte.
static void test_dtls_ciphertext_protect_kat(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, KAT_EPOCH, KAT_SECRET);
    uint8_t out[64];
    size_t n = dtls_ciphertext_protect(&k, KAT_SEQ, KAT_CT, (const uint8_t *)KAT_PLAINTEXT, strlen(KAT_PLAINTEXT), out,
                                       sizeof(out));
    TEST_ASSERT_EQUAL_size_t(sizeof(KAT_WIRE), n);
    TEST_ASSERT_EQUAL_MEMORY(KAT_WIRE, out, sizeof(KAT_WIRE));
}

// Unprotecting the KAT record recovers the plaintext, content type, and full sequence number.
static void test_dtls_ciphertext_unprotect_kat(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, KAT_EPOCH, KAT_SECRET);
    uint8_t out[64];
    DtlsCiphertext info;
    TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&k, KAT_SEQ, KAT_WIRE, sizeof(KAT_WIRE), out, sizeof(out), &info));
    TEST_ASSERT_EQUAL_UINT8(KAT_CT, info.content_type);
    TEST_ASSERT_EQUAL_UINT64(KAT_SEQ, info.seq);
    TEST_ASSERT_EQUAL_size_t(strlen(KAT_PLAINTEXT), info.pt_len);
    TEST_ASSERT_EQUAL_MEMORY(KAT_PLAINTEXT, out, info.pt_len);
}

// Round-trip a range of payload sizes, content types, and sequence numbers.
static void test_dtls_ciphertext_roundtrip(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, 2, KAT_SECRET);

    const size_t sizes[] = {0, 1, 15, 16, 40, 250};
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
    {
        uint8_t pt[256];
        for (size_t j = 0; j < sizes[i]; j++)
            pt[j] = (uint8_t)(j * 7 + i);
        uint64_t seq = 1000 + i;
        uint8_t ct = (i & 1) ? DTLS_CT_HANDSHAKE : DTLS_CT_APPLICATION_DATA;

        uint8_t wire[320];
        size_t n = dtls_ciphertext_protect(&k, seq, ct, pt, sizes[i], wire, sizeof(wire));
        TEST_ASSERT_TRUE(n > 0);

        uint8_t out[320];
        DtlsCiphertext info;
        TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&k, seq, wire, n, out, sizeof(out), &info));
        TEST_ASSERT_EQUAL_UINT8(ct, info.content_type);
        TEST_ASSERT_EQUAL_UINT64(seq, info.seq);
        TEST_ASSERT_EQUAL_size_t(sizes[i], info.pt_len);
        if (sizes[i])
            TEST_ASSERT_EQUAL_MEMORY(pt, out, sizes[i]);
    }
}

// The 16-bit on-wire sequence number is reconstructed to the full value nearest the expected one,
// including across a 16-bit rollover (RFC 9147 §4.2.2 / RFC 9000 App. A.3).
static void test_dtls_seq_reconstruction(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, 2, KAT_SECRET);
    const uint8_t pt[4] = {0xDE, 0xAD, 0xBE, 0xEF};

    // Full sequence numbers whose low 16 bits are all that travel on the wire; unprotect with the
    // correct "expected next" recovers the full value.
    const uint64_t seqs[] = {0x0001u, 0xFFFFu, 0x10000u, 0x1FFFFu, 0x20003u, 0xABCDEu};
    for (unsigned i = 0; i < sizeof(seqs) / sizeof(seqs[0]); i++)
    {
        uint8_t wire[64];
        size_t n = dtls_ciphertext_protect(&k, seqs[i], DTLS_CT_APPLICATION_DATA, pt, sizeof(pt), wire, sizeof(wire));
        TEST_ASSERT_TRUE(n > 0);
        uint8_t out[64];
        DtlsCiphertext info;
        // next_seq = seqs[i] (the receiver expects this record next); reconstruction returns it exactly.
        TEST_ASSERT_TRUE(dtls_ciphertext_unprotect(&k, seqs[i], wire, n, out, sizeof(out), &info));
        TEST_ASSERT_EQUAL_UINT64(seqs[i], info.seq);
        TEST_ASSERT_EQUAL_MEMORY(pt, out, sizeof(pt));
    }
}

// A tampered ciphertext, a wrong-epoch key, and a connection-id record are all rejected.
static void test_dtls_ciphertext_unprotect_rejects(void)
{
    DtlsRecordKeys k;
    dtls_record_keys_derive(&k, DtlsCipher::AES_128_GCM_SHA256, KAT_EPOCH, KAT_SECRET);
    uint8_t out[64];
    DtlsCiphertext info;

    // Flip one ciphertext byte -> AEAD tag fails.
    uint8_t bad[36];
    memcpy(bad, KAT_WIRE, sizeof(bad));
    bad[20] ^= 0x01;
    TEST_ASSERT_FALSE(dtls_ciphertext_unprotect(&k, KAT_SEQ, bad, sizeof(bad), out, sizeof(out), &info));

    // Keys for a different epoch (low 2 bits mismatch the header) -> rejected before AEAD.
    DtlsRecordKeys k2;
    dtls_record_keys_derive(&k2, DtlsCipher::AES_128_GCM_SHA256, 2, KAT_SECRET); // epoch 2 != header's 3
    TEST_ASSERT_FALSE(dtls_ciphertext_unprotect(&k2, KAT_SEQ, KAT_WIRE, sizeof(KAT_WIRE), out, sizeof(out), &info));

    // A connection-id record (C bit set) is not supported in this phase.
    memcpy(bad, KAT_WIRE, sizeof(bad));
    bad[0] |= 0x10; // set C
    TEST_ASSERT_FALSE(dtls_ciphertext_unprotect(&k, KAT_SEQ, bad, sizeof(bad), out, sizeof(out), &info));

    // Truncated record (no room for the 16-byte sequence-number sample) is rejected.
    TEST_ASSERT_FALSE(dtls_ciphertext_unprotect(&k, KAT_SEQ, KAT_WIRE, 10, out, sizeof(out), &info));
}

// DTLSPlaintext builds and parses back to the same fields (initial-flight / alert records).
static void test_dtls_plaintext_roundtrip(void)
{
    const uint8_t frag[19] = "handshake fragment";
    uint8_t rec[64];
    size_t n = dtls_plaintext_build(DTLS_CT_HANDSHAKE, 0, 0x0102030405ull, frag, sizeof(frag), rec, sizeof(rec));
    TEST_ASSERT_EQUAL_size_t(DTLS_PLAINTEXT_HDR_LEN + sizeof(frag), n);
    // legacy_version is DTLS 1.2 on the wire.
    TEST_ASSERT_EQUAL_UINT8(0xFE, rec[1]);
    TEST_ASSERT_EQUAL_UINT8(0xFD, rec[2]);

    DtlsPlaintext pt;
    size_t consumed = dtls_plaintext_parse(rec, n, &pt);
    TEST_ASSERT_EQUAL_size_t(n, consumed);
    TEST_ASSERT_EQUAL_UINT8(DTLS_CT_HANDSHAKE, pt.content_type);
    TEST_ASSERT_EQUAL_UINT16(0, pt.epoch);
    TEST_ASSERT_EQUAL_UINT64(0x0102030405ull, pt.seq);
    TEST_ASSERT_EQUAL_size_t(sizeof(frag), pt.frag_len);
    TEST_ASSERT_EQUAL_MEMORY(frag, pt.fragment, sizeof(frag));

    // A wrong legacy_version is rejected.
    rec[1] = 0x03;
    TEST_ASSERT_EQUAL_size_t(0, dtls_plaintext_parse(rec, n, &pt));
}

// The anti-replay sliding window accepts new records, rejects replays, and ages out old ones.
static void test_dtls_replay_window(void)
{
    DtlsReplayWindow w;
    dtls_replay_init(&w);

    TEST_ASSERT_TRUE(dtls_replay_check(&w, 5)); // first record, any seq
    dtls_replay_mark(&w, 5);
    TEST_ASSERT_FALSE(dtls_replay_check(&w, 5)); // replay of the highest

    TEST_ASSERT_TRUE(dtls_replay_check(&w, 6));
    dtls_replay_mark(&w, 6);
    TEST_ASSERT_TRUE(dtls_replay_check(&w, 3)); // older but in-window and unseen
    dtls_replay_mark(&w, 3);
    TEST_ASSERT_FALSE(dtls_replay_check(&w, 3)); // now a replay

    // Jump far ahead; records older than the 64-wide window are rejected.
    dtls_replay_mark(&w, 200);
    TEST_ASSERT_FALSE(dtls_replay_check(&w, 6));   // 200 - 6 = 194 >= 64
    TEST_ASSERT_FALSE(dtls_replay_check(&w, 200)); // replay of the new highest
    TEST_ASSERT_TRUE(dtls_replay_check(&w, 180));  // in-window, unseen
    TEST_ASSERT_TRUE(dtls_replay_check(&w, 201));  // ahead
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_dtls_record_keys_derive_kat);
    RUN_TEST(test_dtls_ciphertext_protect_kat);
    RUN_TEST(test_dtls_ciphertext_unprotect_kat);
    RUN_TEST(test_dtls_ciphertext_roundtrip);
    RUN_TEST(test_dtls_seq_reconstruction);
    RUN_TEST(test_dtls_ciphertext_unprotect_rejects);
    RUN_TEST(test_dtls_plaintext_roundtrip);
    RUN_TEST(test_dtls_replay_window);
    return UNITY_END();
}
