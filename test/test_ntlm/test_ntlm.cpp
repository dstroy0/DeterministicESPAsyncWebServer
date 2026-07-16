// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// KAT test for the NTLMv2 response (services/smb/ntlm) against the MS-NLMP section 4.2
// worked example. The expected NTOWFv2 is the MS-NLMP published value; NTProofStr /
// SessionBaseKey / NtChallengeResponse were computed by an independent reference whose
// NTOWFv2 matches the published value (so the whole pipeline is validated).

#include "services/smb/ntlm.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static size_t unhex(const char *h, uint8_t *out)
{
    size_t n = 0;
    for (; h[0] && h[1]; h += 2)
    {
        auto nib = [](char x) -> int { return x <= '9' ? x - '0' : (x | 0x20) - 'a' + 10; };
        out[n++] = (uint8_t)((nib(h[0]) << 4) | nib(h[1]));
    }
    return n;
}
static void to_hex(const uint8_t *d, size_t n, char *out)
{
    static const char *H = "0123456789abcdef";
    for (size_t i = 0; i < n; i++)
    {
        out[i * 2] = H[d[i] >> 4];
        out[i * 2 + 1] = H[d[i] & 0xF];
    }
    out[n * 2] = 0;
}

void test_ntowfv2()
{
    uint8_t nt[16], owf[16];
    char hex[33];
    ntlm_nt_hash("Password", nt);
    TEST_ASSERT_TRUE(ntlm_ntowfv2(nt, "User", "Domain", owf));
    to_hex(owf, 16, hex);
    // MS-NLMP 4.2.4.1 published value
    TEST_ASSERT_EQUAL_STRING("0c868a403bfd7a93a3001ef22ef02e3f", hex);
    // the NT hash of "password" (lowercase) is the well-known 8846f7ea...
    ntlm_nt_hash("password", nt);
    to_hex(nt, 16, hex);
    TEST_ASSERT_EQUAL_STRING("8846f7eaee8fb117ad06bdd830b7586c", hex);
}

void test_ntlmv2_response()
{
    uint8_t nt[16], owf[16];
    ntlm_nt_hash("Password", nt);
    ntlm_ntowfv2(nt, "User", "Domain", owf);

    uint8_t srv[8], cli[8], ti[64];
    unhex("0123456789abcdef", srv);
    unhex("aaaaaaaaaaaaaaaa", cli);
    uint8_t time[8] = {0};
    size_t ti_len = unhex("02000c0044006f006d00610069006e0001000c0053006500720076006500720000000000", ti);

    uint8_t out[256], skey[16];
    size_t n = ntlm_v2_response(owf, srv, cli, time, ti, ti_len, out, sizeof(out), skey);
    TEST_ASSERT_EQUAL_size_t(48 + ti_len, n);

    char hex[513];
    to_hex(out, 16, hex);
    TEST_ASSERT_EQUAL_STRING("68cd0ab851e51c96aabc927bebef6a1c", hex); // NTProofStr
    to_hex(skey, 16, hex);
    TEST_ASSERT_EQUAL_STRING("8de40ccadbc14a82f15cb0ad0de95ca3", hex); // SessionBaseKey
    to_hex(out, n, hex);
    TEST_ASSERT_EQUAL_STRING("68cd0ab851e51c96aabc927bebef6a1c"
                             "01010000000000000000000000000000aaaaaaaaaaaaaaaa00000000"
                             "02000c0044006f006d00610069006e0001000c0053006500720076006500720000000000"
                             "00000000",
                             hex); // full NtChallengeResponse = NTProofStr + temp
}

void test_fail_closed()
{
    uint8_t owf[16] = {0}, srv[8] = {0}, cli[8] = {0}, time[8] = {0}, ti[4] = {0}, out[16], skey[16];
    TEST_ASSERT_EQUAL_size_t(0, ntlm_v2_response(owf, srv, cli, time, ti, sizeof(ti), out, sizeof(out), skey));
}

// A user long enough that its UTF-16LE expansion overflows the 256-char (512-byte) scratch: the
// user loop's overflow guard fails closed (ntlm.cpp:37-38).
void test_ntowfv2_user_overflow()
{
    uint8_t nt[16] = {0}, owf[16];
    char user[300];
    memset(user, 'a', sizeof(user) - 1); // 299 chars -> 598 bytes UTF-16LE, over the 512-byte buffer
    user[sizeof(user) - 1] = 0;
    TEST_ASSERT_FALSE(ntlm_ntowfv2(nt, user, "X", owf));
}

// A user that fits but a domain that pushes the concatenation over the scratch: the domain loop's
// overflow guard fails closed (ntlm.cpp:44-45).
void test_ntowfv2_domain_overflow()
{
    uint8_t nt[16] = {0}, owf[16];
    char user[251];
    memset(user, 'b', 250); // 250 chars -> 500 bytes, fits
    user[250] = 0;
    char domain[40];
    memset(domain, 'c', 39); // 39 chars -> tips n past 512 in the domain loop
    domain[39] = 0;
    TEST_ASSERT_FALSE(ntlm_ntowfv2(nt, user, domain, owf));
}

// A user char that is >= 'a' but > 'z' ('{') exercises the ASCII-uppercase compound guard's
// untaken side (ntlm.cpp:35): it must be left unchanged, not uppercased.
void test_ntowfv2_upper_high_char()
{
    uint8_t nt[16] = {0}, owf[16];
    TEST_ASSERT_TRUE(ntlm_ntowfv2(nt, "a{z", "", owf));
}

// A null out buffer fails closed before any write (ntlm.cpp:86, the !out side of the guard).
void test_v2_response_null_out()
{
    uint8_t owf[16] = {0}, srv[8] = {0}, cli[8] = {0}, time[8] = {0}, ti[4] = {0}, skey[16];
    TEST_ASSERT_EQUAL_size_t(0, ntlm_v2_response(owf, srv, cli, time, ti, sizeof(ti), nullptr, 100, skey));
}

// A null session_key skips the SessionBaseKey derivation (ntlm.cpp:109 false side); the returned
// NtChallengeResponse (out) is identical to the MS-NLMP 4.2 vector regardless.
void test_v2_response_null_skey()
{
    uint8_t nt[16], owf[16];
    ntlm_nt_hash("Password", nt);
    ntlm_ntowfv2(nt, "User", "Domain", owf);

    uint8_t srv[8], cli[8], ti[64];
    unhex("0123456789abcdef", srv);
    unhex("aaaaaaaaaaaaaaaa", cli);
    uint8_t time[8] = {0};
    size_t ti_len = unhex("02000c0044006f006d00610069006e0001000c0053006500720076006500720000000000", ti);

    uint8_t out[256];
    size_t n = ntlm_v2_response(owf, srv, cli, time, ti, ti_len, out, sizeof(out), nullptr);
    TEST_ASSERT_EQUAL_size_t(48 + ti_len, n);
    char hex[513];
    to_hex(out, 16, hex);
    TEST_ASSERT_EQUAL_STRING("68cd0ab851e51c96aabc927bebef6a1c", hex); // NTProofStr unaffected
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ntowfv2);
    RUN_TEST(test_ntlmv2_response);
    RUN_TEST(test_fail_closed);
    RUN_TEST(test_ntowfv2_user_overflow);
    RUN_TEST(test_ntowfv2_domain_overflow);
    RUN_TEST(test_ntowfv2_upper_high_char);
    RUN_TEST(test_v2_response_null_out);
    RUN_TEST(test_v2_response_null_skey);
    return UNITY_END();
}
