// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SNMP ASN.1 BER codec. Encodings are checked against
// independent known-answer vectors (the standard BER byte sequences), then
// round-tripped through the decoder.

#include "services/snmp/snmp_ber.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ==================== INTEGER known-answer vectors ====================

static void check_int(long v, const uint8_t *exp, size_t explen)
{
    uint8_t buf[16];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_integer(&e, v);
    TEST_ASSERT_TRUE(e.ok);
    TEST_ASSERT_EQUAL_UINT(explen, e.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, buf, explen);
}

void test_integer_vectors()
{
    const uint8_t v0[] = {0x02, 0x01, 0x00};
    check_int(0, v0, sizeof(v0));
    const uint8_t v1[] = {0x02, 0x01, 0x01};
    check_int(1, v1, sizeof(v1));
    const uint8_t v127[] = {0x02, 0x01, 0x7F};
    check_int(127, v127, sizeof(v127));
    const uint8_t v128[] = {0x02, 0x02, 0x00, 0x80};
    check_int(128, v128, sizeof(v128));
    const uint8_t v256[] = {0x02, 0x02, 0x01, 0x00};
    check_int(256, v256, sizeof(v256));
    const uint8_t vm1[] = {0x02, 0x01, 0xFF};
    check_int(-1, vm1, sizeof(vm1));
    const uint8_t vm128[] = {0x02, 0x01, 0x80};
    check_int(-128, vm128, sizeof(vm128));
}

void test_oid_vector()
{
    // 1.3.6.1 -> 06 03 2B 06 01
    uint32_t a[] = {1, 3, 6, 1};
    uint8_t buf[16];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_oid(&e, a, 4);
    const uint8_t exp[] = {0x06, 0x03, 0x2B, 0x06, 0x01};
    TEST_ASSERT_TRUE(e.ok);
    TEST_ASSERT_EQUAL_UINT(sizeof(exp), e.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, buf, sizeof(exp));

    // sysName.0 = 1.3.6.1.2.1.1.5.0 -> 06 08 2B 06 01 02 01 01 05 00
    uint32_t b[] = {1, 3, 6, 1, 2, 1, 1, 5, 0};
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_oid(&e, b, 9);
    const uint8_t exp2[] = {0x06, 0x08, 0x2B, 0x06, 0x01, 0x02, 0x01, 0x01, 0x05, 0x00};
    TEST_ASSERT_TRUE(e.ok);
    TEST_ASSERT_EQUAL_UINT(sizeof(exp2), e.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp2, buf, sizeof(exp2));
}

void test_octet_string_and_null()
{
    uint8_t buf[16];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)"public", 6);
    const uint8_t exp[] = {0x04, 0x06, 'p', 'u', 'b', 'l', 'i', 'c'};
    TEST_ASSERT_TRUE(e.ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, buf, sizeof(exp));

    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_null(&e);
    const uint8_t expn[] = {0x05, 0x00};
    TEST_ASSERT_EQUAL_UINT(2, e.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expn, buf, 2);
}

void test_counter32_keeps_unsigned()
{
    // 0x80000000 has the top bit set -> a leading 0x00 must be added.
    uint8_t buf[16];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_uint(&e, SNMP_COUNTER32, 0x80000000u);
    const uint8_t exp[] = {0x41, 0x05, 0x00, 0x80, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE(e.ok);
    TEST_ASSERT_EQUAL_UINT(sizeof(exp), e.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, buf, sizeof(exp));
}

// ==================== round-trip + SEQUENCE ====================

void test_sequence_roundtrip()
{
    uint8_t buf[64];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    size_t seq = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, 1); // e.g. SNMP version (v2c=1)
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)"public", 6);
    ber_seq_end(&e, seq);
    TEST_ASSERT_TRUE(e.ok);

    // Decode: outer SEQUENCE, then INTEGER + OCTET STRING.
    BerDec d;
    ber_dec_init(&d, buf, e.len);
    uint8_t tag;
    size_t len;
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_EQUAL_HEX8(BER_SEQUENCE, tag);

    long ver = -99;
    TEST_ASSERT_TRUE(ber_read_integer(&d, &ver));
    TEST_ASSERT_EQUAL_INT(1, ver);

    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_EQUAL_HEX8(BER_OCTET_STRING, tag);
    TEST_ASSERT_EQUAL_UINT(6, len);
    TEST_ASSERT_EQUAL_MEMORY("public", d.buf + d.pos, 6);
}

void test_oid_roundtrip()
{
    uint32_t in[] = {1, 3, 6, 1, 2, 1, 1, 5, 0};
    uint8_t buf[32];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_oid(&e, in, 9);
    TEST_ASSERT_TRUE(e.ok);

    BerDec d;
    ber_dec_init(&d, buf, e.len);
    uint32_t out[SNMP_MAX_OID_LEN];
    size_t n = 0;
    TEST_ASSERT_TRUE(ber_read_oid(&d, out, SNMP_MAX_OID_LEN, &n));
    TEST_ASSERT_EQUAL_UINT(9, n);
    for (size_t i = 0; i < 9; i++)
        TEST_ASSERT_EQUAL_UINT32(in[i], out[i]);
}

void test_large_arc_roundtrip()
{
    // An arc > 127 exercises multi-byte base-128 encoding (e.g. enterprise 8072).
    uint32_t in[] = {1, 3, 6, 1, 4, 1, 8072, 3, 2, 10};
    uint8_t buf[32];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_oid(&e, in, 10);
    TEST_ASSERT_TRUE(e.ok);

    BerDec d;
    ber_dec_init(&d, buf, e.len);
    uint32_t out[SNMP_MAX_OID_LEN];
    size_t n = 0;
    TEST_ASSERT_TRUE(ber_read_oid(&d, out, SNMP_MAX_OID_LEN, &n));
    TEST_ASSERT_EQUAL_UINT(10, n);
    TEST_ASSERT_EQUAL_UINT32(8072u, out[6]);
}

// X.690 8.19.4: the FIRST subidentifier (40*arc0 + arc1) is itself base-128 and can
// span multiple octets when arc1 is large. OID 2.100.3 -> first subid = 40*2+100 = 180
// (>= 128, two octets). The decoder must split it back to {2, 100, 3}, not misread it.
void test_oid_large_first_subidentifier_roundtrip()
{
    uint32_t in[] = {2, 100, 3};
    uint8_t buf[16];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_oid(&e, in, 3);
    TEST_ASSERT_TRUE(e.ok);

    BerDec d;
    ber_dec_init(&d, buf, e.len);
    uint32_t out[SNMP_MAX_OID_LEN];
    size_t n = 0;
    TEST_ASSERT_TRUE(ber_read_oid(&d, out, SNMP_MAX_OID_LEN, &n));
    TEST_ASSERT_EQUAL_UINT(3, n);
    TEST_ASSERT_EQUAL_UINT32(2u, out[0]);
    TEST_ASSERT_EQUAL_UINT32(100u, out[1]);
    TEST_ASSERT_EQUAL_UINT32(3u, out[2]);
}

// ==================== bounds / error handling ====================

void test_encoder_overflow_sets_not_ok()
{
    uint8_t buf[3];
    BerEnc e;
    ber_enc_init(&e, buf, sizeof(buf));
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)"too long", 8);
    TEST_ASSERT_FALSE(e.ok);
}

void test_decoder_truncated_length_fails()
{
    // Claims 10 bytes of content but only 2 are present.
    const uint8_t bad[] = {0x04, 0x0A, 0x01, 0x02};
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    uint8_t tag;
    size_t len;
    TEST_ASSERT_FALSE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_FALSE(d.ok);
}

// Long-form length whose count byte (0x84 = "4 length octets follow") runs past
// the buffer: the count-byte bounds check must reject it, not over-read.
void test_decoder_longform_length_count_past_buffer_fails()
{
    const uint8_t bad[] = {0x04, 0x84, 0x00, 0x00}; // says 4 len octets, only 2 present
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    uint8_t tag;
    size_t len;
    TEST_ASSERT_FALSE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_FALSE(d.ok);
}

// Long-form length with an over-wide count (> 4 octets) is rejected (a huge
// length can't be represented / is a malformed/attack input).
void test_decoder_longform_length_too_wide_fails()
{
    const uint8_t bad[] = {0x04, 0x85, 0x01, 0x00, 0x00, 0x00, 0x00}; // 5 length octets
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    uint8_t tag;
    size_t len;
    TEST_ASSERT_FALSE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_FALSE(d.ok);
}

// Long-form length that parses but then claims more content than is present.
void test_decoder_longform_length_content_past_buffer_fails()
{
    // 0x82 0x01 0x00 = long form, length 256; only a few content bytes follow.
    const uint8_t bad[] = {0x04, 0x82, 0x01, 0x00, 0xAA, 0xBB};
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    uint8_t tag;
    size_t len;
    TEST_ASSERT_FALSE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_FALSE(d.ok);
}

// An indefinite-length encoding (0x80) is not valid in DER/this decoder.
void test_decoder_indefinite_length_fails()
{
    const uint8_t bad[] = {0x30, 0x80, 0x00, 0x00};
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    uint8_t tag;
    size_t len;
    TEST_ASSERT_FALSE(ber_read_header(&d, &tag, &len));
    TEST_ASSERT_FALSE(d.ok);
}

// An INTEGER whose length exceeds the supported width (> 8 octets) is rejected.
void test_decoder_oversized_integer_fails()
{
    const uint8_t bad[] = {0x02, 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 1}; // 9-octet INTEGER
    BerDec d;
    ber_dec_init(&d, bad, sizeof(bad));
    long v;
    TEST_ASSERT_FALSE(ber_read_integer(&d, &v));
    TEST_ASSERT_FALSE(d.ok);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_integer_vectors);
    RUN_TEST(test_oid_vector);
    RUN_TEST(test_octet_string_and_null);
    RUN_TEST(test_counter32_keeps_unsigned);
    RUN_TEST(test_sequence_roundtrip);
    RUN_TEST(test_oid_roundtrip);
    RUN_TEST(test_large_arc_roundtrip);
    RUN_TEST(test_oid_large_first_subidentifier_roundtrip);
    RUN_TEST(test_encoder_overflow_sets_not_ok);
    RUN_TEST(test_decoder_truncated_length_fails);
    RUN_TEST(test_decoder_longform_length_count_past_buffer_fails);
    RUN_TEST(test_decoder_longform_length_too_wide_fails);
    RUN_TEST(test_decoder_longform_length_content_past_buffer_fails);
    RUN_TEST(test_decoder_indefinite_length_fails);
    RUN_TEST(test_decoder_oversized_integer_fails);
    return UNITY_END();
}
