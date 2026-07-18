// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for QUIC Initial packet crypto (network_drivers/presentation/http3/dws_quic_hkdf,
// dws_quic_aead, dws_quic_crypto; RFC 9001). Everything is checked against authoritative vectors:
//   - AES-128 block:  FIPS 197 Appendix B known-answer.
//   - AES-128-GCM:    the McGrew/Viega GCM Test Case 4 (non-block-aligned plaintext + AAD).
//   - Initial keys:   RFC 9001 Appendix A.1 client/server key/iv/hp.
//   - Server Initial: RFC 9001 A.3, byte-exact protect and round-trip unprotect.
//   - Client Initial: RFC 9001 A.2, protected header + header-protection sample + round-trip.
//   - Retry tag:      RFC 9001 A.4 integrity tag.
// Pure host crypto; no mbedtls on native, so the software AES-128/GHASH paths are what run here.

#include "network_drivers/presentation/http3/quic_aead.h"
#include "network_drivers/presentation/http3/quic_crypto.h"
#include "network_drivers/presentation/http3/quic_hkdf.h"
#include "network_drivers/presentation/http3/quic_packet.h" // QUIC_MAX_CID_LEN (retry-tag guard bound)
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Decode a hex string (spaces/newlines ignored) into buf; returns byte count.
static size_t hx(const char *s, uint8_t *buf, size_t cap)
{
    size_t n = 0;
    int hi = -1;
    for (; *s; s++)
    {
        char c = *s;
        int v;
        if (c >= '0' && c <= '9')
            v = c - '0';
        else if (c >= 'a' && c <= 'f')
            v = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            v = c - 'A' + 10;
        else
            continue;
        if (hi < 0)
            hi = v;
        else
        {
            TEST_ASSERT_TRUE(n < cap);
            buf[n++] = (uint8_t)((hi << 4) | v);
            hi = -1;
        }
    }
    TEST_ASSERT_EQUAL_INT(-1, hi); // even number of hex digits
    return n;
}

// --- AES-128 block: FIPS 197 Appendix B (the worked example) --------------------------------
void test_aes128_block_fips197()
{
    uint8_t key[16], in[16], exp[16], out[16];
    hx("000102030405060708090a0b0c0d0e0f", key, 16);
    hx("00112233445566778899aabbccddeeff", in, 16);
    hx("69c4e0d86a7b0430d8cdb78070b4c55a", exp, 16);
    QuicAes128 aes;
    dws_quic_aes128_init(&aes, key);
    dws_quic_aes128_encrypt_block(&aes, in, out);
    dws_quic_aes128_wipe(&aes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, 16);
}

// --- AES-128-GCM: McGrew/Viega Test Case 4 (60-byte plaintext, 20-byte AAD) -----------------
void test_aes128_gcm_testcase4()
{
    uint8_t key[16], iv[12], pt[60], aad[20], exp_ct[60], exp_tag[16];
    hx("feffe9928665731c6d6a8f9467308308", key, 16);
    hx("cafebabefacedbaddecaf888", iv, 12);
    hx("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
       "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39",
       pt, 60);
    hx("feedfacedeadbeeffeedfacedeadbeefabaddad2", aad, 20);
    hx("42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e"
       "21d514b25466931c7d8f6a5aac84aa051ba30b396a0aac973d58e091",
       exp_ct, 60);
    hx("5bc94fbc3221a5db94fae95ae7121a47", exp_tag, 16);

    uint8_t sealed[60 + 16];
    dws_quic_aes128_gcm_seal(key, iv, aad, 20, pt, 60, sealed);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_ct, sealed, 60);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_tag, sealed + 60, 16);

    uint8_t opened[60];
    TEST_ASSERT_TRUE(dws_quic_aes128_gcm_open(key, iv, aad, 20, sealed, sizeof sealed, opened));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(pt, opened, 60);

    // A single flipped ciphertext bit must fail authentication and write nothing.
    sealed[0] ^= 0x01;
    TEST_ASSERT_FALSE(dws_quic_aes128_gcm_open(key, iv, aad, 20, sealed, sizeof sealed, opened));
}

// --- RFC 9001 A.1: Initial secret derivation from the Destination Connection ID -------------
void test_initial_secrets_appendix_a1()
{
    uint8_t dcid[8];
    hx("8394c8f03e515708", dcid, 8);
    QuicInitialSecrets s;
    dws_quic_derive_initial_secrets(dcid, 8, &s);

    uint8_t ck[16], civ[12], chp[16], sk[16], siv[12], shp[16];
    hx("1f369613dd76d5467730efcbe3b1a22d", ck, 16);
    hx("fa044b2f42a3fd3b46fb255c", civ, 12);
    hx("9f50449e04a0e810283a1e9933adedd2", chp, 16);
    hx("cf3a5331653c364c88f0f379b6067e37", sk, 16);
    hx("0ac1493ca1905853b0bba03e", siv, 12);
    hx("c206b8d9b9f0f37644430b490eeaa314", shp, 16);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(ck, s.client.key, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(civ, s.client.iv, 12);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(chp, s.client.hp, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sk, s.server.key, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(siv, s.server.iv, 12);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(shp, s.server.hp, 16);
}

// --- RFC 9001 A.3: server Initial, byte-exact protect + round-trip unprotect -----------------
void test_server_initial_a3()
{
    uint8_t dcid[8];
    hx("8394c8f03e515708", dcid, 8);
    QuicInitialSecrets s;
    dws_quic_derive_initial_secrets(dcid, 8, &s);

    // Unprotected header (20 bytes: pn_offset 18, 2-byte pn = 1) then the 99-byte payload.
    uint8_t pkt[256];
    size_t hdr = hx("c1000000010008f067a5502a4262b50040750001", pkt, sizeof pkt);
    TEST_ASSERT_EQUAL_INT(20, (int)hdr);
    size_t plen = hx("02000000000600405a020000560303eefce7f7b37ba1d1632e96677825ddf739"
                     "88cfc79825df566dc5430b9a045a1200130100002e00330024001d00209d3c94"
                     "0d89690b84d08a60993c144eca684d1081287c834d5311bcf32bb9da1a002b00"
                     "020304",
                     pkt + hdr, sizeof pkt - hdr);
    TEST_ASSERT_EQUAL_INT(99, (int)plen);

    size_t total = dws_quic_packet_protect(pkt, sizeof pkt, /*pn_offset*/ 18, /*pn_len*/ 2, /*full_pn*/ 1,
                                           /*payload_len*/ 99, &s.server, /*is_long*/ true);
    uint8_t exp[256];
    size_t elen = hx("cf000000010008f067a5502a4262b5004075c0d95a482cd0991cd25b0aac406a"
                     "5816b6394100f37a1c69797554780bb38cc5a99f5ede4cf73c3ec2493a1839b3"
                     "dbcba3f6ea46c5b7684df3548e7ddeb9c3bf9c73cc3f3bded74b562bfb19fb84"
                     "022f8ef4cdd93795d77d06edbb7aaf2f58891850abbdca3d20398c276456cbc4"
                     "2158407dd074ee",
                     exp, sizeof exp);
    TEST_ASSERT_EQUAL_INT(135, (int)elen);
    TEST_ASSERT_EQUAL_INT((int)elen, (int)total);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, pkt, elen);

    // Round-trip: unprotect the wire bytes back to pn=1 and the original payload.
    uint8_t out[128];
    uint64_t pn = 0;
    size_t got = dws_quic_packet_unprotect(exp, /*pn_offset*/ 18, /*length*/ 117, /*largest_pn*/ 0, &s.server,
                                           /*is_long*/ true, out, &pn);
    TEST_ASSERT_EQUAL_INT(99, (int)got);
    TEST_ASSERT_EQUAL_UINT64(1, pn);
    // Compare against a freshly decoded plaintext payload.
    uint8_t pt[128];
    hx("02000000000600405a020000560303eefce7f7b37ba1d1632e96677825ddf739"
       "88cfc79825df566dc5430b9a045a1200130100002e00330024001d00209d3c94"
       "0d89690b84d08a60993c144eca684d1081287c834d5311bcf32bb9da1a002b00"
       "020304",
       pt, sizeof pt);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(pt, out, 99);
}

// --- RFC 9001 A.2: client Initial, protected header + HP sample + round-trip -----------------
void test_client_initial_a2()
{
    uint8_t dcid[8];
    hx("8394c8f03e515708", dcid, 8);
    QuicInitialSecrets s;
    dws_quic_derive_initial_secrets(dcid, 8, &s);

    // Unprotected header (22 bytes: pn_offset 18, 4-byte pn = 2), then a 1162-byte payload made of
    // the CRYPTO frame (245 bytes) zero-padded with PADDING frames, per A.2.
    static uint8_t pkt[1500];
    memset(pkt, 0, sizeof pkt);
    size_t hdr = hx("c300000001088394c8f03e5157080000449e00000002", pkt, sizeof pkt);
    TEST_ASSERT_EQUAL_INT(22, (int)hdr);
    size_t clen = hx("060040f1010000ed0303ebf8fa56f12939b9584a3896472ec40bb863cfd3e868"
                     "04fe3a47f06a2b69484c000004130113020100 00c000000010000e00000b6578"
                     "616d706c652e636f6dff01000100000a00080006001d00170018001000070005"
                     "04616c706e0005000501000000000033 00260024001d00209370b2c9caa47fba"
                     "baf4559fedba753de171fa71f50f1ce15d43e994ec74d748002b000302030400"
                     "0d0010000e0403050306030203080408050806002d00020101001c0002400100"
                     "3900320408ffffffffffffffff05048000ffff07048000ffff08011001048000"
                     "75300901100f088394c8f03e51570806048000ffff",
                     pkt + hdr, sizeof pkt - hdr);
    TEST_ASSERT_EQUAL_INT(245, (int)clen);
    const size_t payload_len = 1162; // CRYPTO frame + zero PADDING to the mandated size

    size_t total = dws_quic_packet_protect(pkt, sizeof pkt, /*pn_offset*/ 18, /*pn_len*/ 4, /*full_pn*/ 2, payload_len,
                                           &s.client, /*is_long*/ true);
    TEST_ASSERT_EQUAL_INT((int)(22 + payload_len + 16), (int)total);

    // The protected header and the first ciphertext block (== the header-protection sample) are the
    // authoritative bytes given in A.2.
    uint8_t exp_hdr[22], exp_sample[16];
    hx("c000000001088394c8f03e5157080000449e7b9aec34", exp_hdr, 22);
    hx("d1b1c98dd7689fb8ec11d242b123dc9b", exp_sample, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_hdr, pkt, 22);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_sample, pkt + 22, 16);

    // Round-trip unprotect recovers pn=2 and the padded payload.
    static uint8_t out[1200];
    uint64_t pn = 0;
    size_t got = dws_quic_packet_unprotect(pkt, /*pn_offset*/ 18, /*length*/ 1182, /*largest_pn*/ 0, &s.client,
                                           /*is_long*/ true, out, &pn);
    TEST_ASSERT_EQUAL_INT((int)payload_len, (int)got);
    TEST_ASSERT_EQUAL_UINT64(2, pn);

    // First 245 bytes are the CRYPTO frame; the remainder is zero PADDING.
    uint8_t crypto[245];
    hx("060040f1010000ed0303ebf8fa56f12939b9584a3896472ec40bb863cfd3e868"
       "04fe3a47f06a2b69484c00000413011302010000c000000010000e00000b6578"
       "616d706c652e636f6dff01000100000a00080006001d00170018001000070005"
       "04616c706e000500050100000000003300260024001d00209370b2c9caa47fba"
       "baf4559fedba753de171fa71f50f1ce15d43e994ec74d748002b000302030400"
       "0d0010000e0403050306030203080408050806002d00020101001c0002400100"
       "3900320408ffffffffffffffff05048000ffff07048000ffff08011001048000"
       "75300901100f088394c8f03e51570806048000ffff",
       crypto, 245);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(crypto, out, 245);
    for (size_t i = 245; i < payload_len; i++)
        TEST_ASSERT_EQUAL_UINT8(0, out[i]);
}

// --- RFC 9001 A.4: Retry integrity tag ------------------------------------------------------
void test_retry_integrity_a4()
{
    uint8_t odcid[8];
    hx("8394c8f03e515708", odcid, 8);
    // Retry packet without the trailing 16-byte integrity tag (first byte .. Retry Token "token").
    uint8_t retry[64];
    size_t rlen = hx("ff000000010008f067a5502a4262b5746f6b656e", retry, sizeof retry);
    TEST_ASSERT_EQUAL_INT(20, (int)rlen);

    uint8_t tag[16], exp[16];
    dws_quic_retry_integrity_tag(odcid, 8, retry, rlen, tag);
    hx("04a265ba2eff4d829058fb3f0f2496ba", exp, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, tag, 16);
}

// --- HKDF-Expand-Label label wire format (RFC 9001 A.1 info strings, sanity via a known secret) --
void test_gcm_open_rejects_short()
{
    uint8_t key[16] = {0}, nonce[12] = {0}, out[8];
    uint8_t ct[8] = {0}; // shorter than a 16-byte tag
    TEST_ASSERT_FALSE(dws_quic_aes128_gcm_open(key, nonce, nullptr, 0, ct, sizeof ct, out));
}

// --- dws_quic_packet_protect parameter/capacity guards ------------------------------------------
// The packet-number length must be 1..4; anything else is a parameter error (returns 0). Drives both
// sides of the `pn_len < 1 || pn_len > 4` reject that the RFC-vector protect/unprotect never hits.
void test_protect_rejects_bad_pn_len()
{
    QuicPacketKeys keys;
    memset(&keys, 0, sizeof keys);
    uint8_t pkt[64];
    memset(pkt, 0, sizeof pkt);
    TEST_ASSERT_EQUAL_INT(0, (int)dws_quic_packet_protect(pkt, sizeof pkt, /*pn_offset*/ 4, /*pn_len*/ 0, /*full_pn*/ 1,
                                                          /*payload_len*/ 8, &keys, /*is_long*/ true));
    TEST_ASSERT_EQUAL_INT(0, (int)dws_quic_packet_protect(pkt, sizeof pkt, /*pn_offset*/ 4, /*pn_len*/ 5, /*full_pn*/ 1,
                                                          /*payload_len*/ 8, &keys, /*is_long*/ true));
}

// header + payload + 16-byte tag must fit the buffer, else 0 (before any crypto runs).
void test_protect_rejects_small_cap()
{
    QuicPacketKeys keys;
    memset(&keys, 0, sizeof keys);
    uint8_t pkt[8];
    memset(pkt, 0, sizeof pkt);
    // hdr(pn_offset 4 + pn_len 2 = 6) + payload(100) + tag(16) = 122 > cap 8.
    TEST_ASSERT_EQUAL_INT(0, (int)dws_quic_packet_protect(pkt, sizeof pkt, /*pn_offset*/ 4, /*pn_len*/ 2, /*full_pn*/ 1,
                                                          /*payload_len*/ 100, &keys, /*is_long*/ true));
}

// --- dws_quic_packet_unprotect reject paths -----------------------------------------------------
// A record whose Length is below the header-protection sample + tag minimum (4 + 16) is rejected up
// front with (size_t)-1, before touching the buffer.
void test_unprotect_rejects_short()
{
    QuicPacketKeys keys;
    memset(&keys, 0, sizeof keys);
    uint8_t pkt[32];
    memset(pkt, 0, sizeof pkt);
    uint8_t out[32];
    uint64_t pn = 0;
    size_t got = dws_quic_packet_unprotect(pkt, /*pn_offset*/ 0, /*length*/ 19, /*largest_pn*/ 0, &keys,
                                           /*is_long*/ true, out, &pn);
    TEST_ASSERT_TRUE(got == (size_t)-1);
}

// A tampered ciphertext must fail the AEAD tag check: unprotect returns (size_t)-1 and writes no
// plaintext. Starts from the byte-exact RFC 9001 A.3 protected server Initial and flips one tag byte
// (outside the header-protection sample, so the packet number still recovers, but the AEAD open fails).
void test_unprotect_rejects_tampered()
{
    uint8_t dcid[8];
    hx("8394c8f03e515708", dcid, 8);
    QuicInitialSecrets s;
    dws_quic_derive_initial_secrets(dcid, 8, &s);

    uint8_t pkt[256];
    size_t elen = hx("cf000000010008f067a5502a4262b5004075c0d95a482cd0991cd25b0aac406a"
                     "5816b6394100f37a1c69797554780bb38cc5a99f5ede4cf73c3ec2493a1839b3"
                     "dbcba3f6ea46c5b7684df3548e7ddeb9c3bf9c73cc3f3bded74b562bfb19fb84"
                     "022f8ef4cdd93795d77d06edbb7aaf2f58891850abbdca3d20398c276456cbc4"
                     "2158407dd074ee",
                     pkt, sizeof pkt);
    TEST_ASSERT_EQUAL_INT(135, (int)elen);
    pkt[elen - 1] ^= 0x01; // corrupt the final auth-tag byte

    uint8_t out[128];
    uint64_t pn = 0;
    size_t got = dws_quic_packet_unprotect(pkt, /*pn_offset*/ 18, /*length*/ 117, /*largest_pn*/ 0, &s.server,
                                           /*is_long*/ true, out, &pn);
    TEST_ASSERT_TRUE(got == (size_t)-1);
}

// --- dws_quic_retry_integrity_tag guard ---------------------------------------------------------
// Either an over-long ODCID (> QUIC_MAX_CID_LEN) or a Retry that would overflow the AAD scratch
// buffer zeroes the tag rather than reading out of bounds. Drives both sides of the compound guard.
void test_retry_tag_rejects_oversize()
{
    uint8_t odcid[QUIC_MAX_CID_LEN + 2];
    memset(odcid, 0xCC, sizeof odcid);
    uint8_t retry[16];
    memset(retry, 0, sizeof retry);
    const uint8_t zero[16] = {0};
    uint8_t tag[16];

    // ODCID length beyond the QUIC maximum -> first guard condition.
    memset(tag, 0xEE, sizeof tag);
    dws_quic_retry_integrity_tag(odcid, QUIC_MAX_CID_LEN + 1, retry, sizeof retry, tag);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(zero, tag, 16);

    // A valid-length ODCID but a Retry so long that 1 + odcid_len + retry_len exceeds the AAD buffer
    // (1 + QUIC_MAX_CID_LEN + 256) -> second guard condition.
    static uint8_t big_retry[300];
    memset(big_retry, 0, sizeof big_retry);
    memset(tag, 0xEE, sizeof tag);
    dws_quic_retry_integrity_tag(odcid, 8, big_retry, sizeof big_retry, tag);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(zero, tag, 16);
}

// --- dws_quic_hkdf HKDF-Expand-Label multi-block output -----------------------------------------
// An independent recomputation of RFC 5869 HKDF-Expand over the RFC 8446 sec 7.1 HkdfLabel, using the
// KAT-verified HMAC-SHA256 primitive. Used to check the > 32-byte (multi-hash-block) path of the
// library's expander, which QUIC itself never exercises (all its outputs are <= 32).
static void expand_label_ref(const uint8_t secret[32], const char *label, uint8_t *out, size_t out_len)
{
    uint8_t info[64];
    size_t p = 0;
    info[p++] = (uint8_t)(out_len >> 8);
    info[p++] = (uint8_t)(out_len & 0xff);
    const char *prefix = "tls13 ";
    size_t plen = strlen(prefix);
    size_t llen = strlen(label);
    info[p++] = (uint8_t)(plen + llen);
    memcpy(info + p, prefix, plen);
    p += plen;
    memcpy(info + p, label, llen);
    p += llen;
    info[p++] = 0; // empty context

    uint8_t t[32];
    size_t t_len = 0;
    size_t done = 0;
    uint8_t counter = 0;
    while (done < out_len)
    {
        counter++;
        SshHmacCtx c;
        ssh_hmac_sha256_init(&c, secret, 32);
        ssh_hmac_sha256_update(&c, t, t_len);
        ssh_hmac_sha256_update(&c, info, p);
        ssh_hmac_sha256_update(&c, &counter, 1);
        ssh_hmac_sha256_final(&c, t);
        t_len = 32;
        size_t take = out_len - done;
        if (take > 32)
            take = 32;
        memcpy(out + done, t, take);
        done += take;
    }
}

// 48 output bytes cross into a second HMAC block: the library must run the N-block loop (T(2) chains
// off T(1)) and match the reference exactly.
void test_hkdf_expand_label_multiblock()
{
    uint8_t secret[32];
    for (int i = 0; i < 32; i++)
        secret[i] = (uint8_t)(0xA0 + i);

    uint8_t got[48], exp[48];
    dws_quic_hkdf_expand_label(secret, "test label", got, sizeof got);
    expand_label_ref(secret, "test label", exp, sizeof exp);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, got, sizeof got);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_aes128_block_fips197);
    RUN_TEST(test_aes128_gcm_testcase4);
    RUN_TEST(test_initial_secrets_appendix_a1);
    RUN_TEST(test_server_initial_a3);
    RUN_TEST(test_client_initial_a2);
    RUN_TEST(test_retry_integrity_a4);
    RUN_TEST(test_gcm_open_rejects_short);
    RUN_TEST(test_protect_rejects_bad_pn_len);
    RUN_TEST(test_protect_rejects_small_cap);
    RUN_TEST(test_unprotect_rejects_short);
    RUN_TEST(test_unprotect_rejects_tampered);
    RUN_TEST(test_retry_tag_rejects_oversize);
    RUN_TEST(test_hkdf_expand_label_multiblock);
    return UNITY_END();
}
