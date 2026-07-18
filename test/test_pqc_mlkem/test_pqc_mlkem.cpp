// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Known-answer test for ML-KEM-768 Encaps (network_drivers/presentation/pqc/mlkem), the post-quantum
// half of the mlkem768x25519-sha256 / X25519MLKEM768 hybrids. The vector in mlkem_kat.h is generated
// by tools/gen_mlkem_kat.py from kyber-py (FIPS 203) and round-trip cross-checked, so a byte-exact
// match here proves our independent Encaps agrees with a reference implementation. Pure host tests.

#include "mlkem_kat.h"
#include "network_drivers/presentation/pqc/mlkem.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Deterministic Encaps(ek, m) must reproduce the reference ciphertext and shared secret exactly.
void test_mlkem768_encaps_kat()
{
    uint8_t ct[MLKEM768_CT_BYTES];
    uint8_t ss[MLKEM768_SS_BYTES];
    bool ok = dws_mlkem768_encaps(kat_ek, kat_m, ct, ss);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(kat_ct, ct, MLKEM768_CT_BYTES);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(kat_ss, ss, MLKEM768_SS_BYTES);
}

// A differently keyed but valid message still yields a 32-octet secret and a well-formed ciphertext
// (the shared secret must differ from the pinned one - Encaps mixes m into K via G).
void test_mlkem768_encaps_varies_with_m()
{
    uint8_t m2[MLKEM768_MSG_BYTES];
    memcpy(m2, kat_m, sizeof(m2));
    m2[0] ^= 0xFF;
    uint8_t ct[MLKEM768_CT_BYTES];
    uint8_t ss[MLKEM768_SS_BYTES];
    TEST_ASSERT_TRUE(dws_mlkem768_encaps(kat_ek, m2, ct, ss));
    TEST_ASSERT_NOT_EQUAL(0, memcmp(ss, kat_ss, MLKEM768_SS_BYTES));
    TEST_ASSERT_NOT_EQUAL(0, memcmp(ct, kat_ct, MLKEM768_CT_BYTES));
}

// FIPS 203 modulus check: an encapsulation key with an out-of-range coefficient is rejected.
void test_mlkem768_rejects_malformed_ek()
{
    uint8_t bad[MLKEM768_EK_BYTES];
    memcpy(bad, kat_ek, sizeof(bad));
    // Force the first 12-bit coefficient to 0xFFF (4095 >= q=3329).
    bad[0] = 0xFF;
    bad[1] = (uint8_t)(bad[1] | 0x0F);
    uint8_t ct[MLKEM768_CT_BYTES];
    uint8_t ss[MLKEM768_SS_BYTES];
    TEST_ASSERT_FALSE(dws_mlkem768_encaps(bad, kat_m, ct, ss));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_mlkem768_encaps_kat);
    RUN_TEST(test_mlkem768_encaps_varies_with_m);
    RUN_TEST(test_mlkem768_rejects_malformed_ek);
    return UNITY_END();
}
