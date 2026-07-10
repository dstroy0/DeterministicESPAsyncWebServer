// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SPNEGO GSS-API DER wrapping (services/smb/spnego): the InitialContextToken
// wrapping (byte-exact vs a hand-computed DER), the NegTokenResp round-trip, and extracting the
// responseToken from a realistic server NegTokenResp (skipping negState + supportedMech).

#include "services/smb/spnego.h"
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

// The InitialContextToken for the 4-byte token {01,02,03,04}, hand-computed DER:
//   60 24 | 06 06 (SPNEGO OID) | a0 1a | 30 18 | a0 0e | 30 0c | 06 0a (NTLM OID) | a2 06 | 04 04 <token>
void test_wrap_negotiate_bytes()
{
    const uint8_t tok[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t out[128];
    char hex[257];
    size_t n = spnego_wrap_negotiate(tok, sizeof(tok), out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(38, n);
    to_hex(out, n, hex);
    TEST_ASSERT_EQUAL_STRING("602406062b0601050502a01a3018a00e300c060a2b06010401823702020aa206040401020304", hex);
    // overflow fails closed
    TEST_ASSERT_EQUAL_size_t(0, spnego_wrap_negotiate(tok, sizeof(tok), out, 20));
}

// wrap_authenticate produces a NegTokenResp [1]{SEQ{[2] OCTET(token)}}; parse_response extracts it.
void test_authenticate_roundtrip()
{
    uint8_t tok[200];
    for (int i = 0; i < (int)sizeof(tok); i++)
        tok[i] = (uint8_t)(i * 7 + 1);
    uint8_t out[256];
    size_t n = spnego_wrap_authenticate(tok, sizeof(tok), out, sizeof(out));
    TEST_ASSERT_GREATER_THAN_size_t(sizeof(tok), n);

    const uint8_t *rt = nullptr;
    size_t rl = 0;
    TEST_ASSERT_TRUE(spnego_parse_response(out, n, &rt, &rl));
    TEST_ASSERT_EQUAL_size_t(sizeof(tok), rl);
    TEST_ASSERT_EQUAL_MEMORY(tok, rt, sizeof(tok));
}

// A realistic server NegTokenResp: [1]{SEQ{ negState[0], supportedMech[1], responseToken[2] }}.
// parse_response must skip the first two and return the responseToken {11,22,33,44}.
void test_parse_server_response()
{
    uint8_t blob[64];
    size_t n = unhex("a11d301ba0030a0101a10c060a2b06010401823702020aa206040411223344", blob);
    const uint8_t *rt = nullptr;
    size_t rl = 0;
    TEST_ASSERT_TRUE(spnego_parse_response(blob, n, &rt, &rl));
    TEST_ASSERT_EQUAL_size_t(4, rl);
    const uint8_t want[] = {0x11, 0x22, 0x33, 0x44};
    TEST_ASSERT_EQUAL_MEMORY(want, rt, 4);
}

void test_parse_rejects()
{
    uint8_t blob[64];
    size_t n = unhex("a11d301ba0030a0101a10c060a2b06010401823702020aa206040411223344", blob);
    const uint8_t *rt = nullptr;
    size_t rl = 0;
    uint8_t bad[64];
    memcpy(bad, blob, n);
    bad[0] = 0x30; // not a NegTokenResp [1]
    TEST_ASSERT_FALSE(spnego_parse_response(bad, n, &rt, &rl));
    TEST_ASSERT_FALSE(spnego_parse_response(blob, 10, &rt, &rl)); // truncated
    // a NegTokenResp with no responseToken (only negState) -> not found
    uint8_t nort[16];
    size_t m = unhex("a107300530030a0101", nort);
    TEST_ASSERT_FALSE(spnego_parse_response(nort, m, &rt, &rl));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_wrap_negotiate_bytes);
    RUN_TEST(test_authenticate_roundtrip);
    RUN_TEST(test_parse_server_response);
    RUN_TEST(test_parse_rejects);
    return UNITY_END();
}
