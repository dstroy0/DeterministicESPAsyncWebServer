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

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ntowfv2);
    RUN_TEST(test_ntlmv2_response);
    RUN_TEST(test_fail_closed);
    return UNITY_END();
}
