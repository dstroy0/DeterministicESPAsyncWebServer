// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SMB2 client wire codec (services/smb, MS-SMB2 increment 1): the
// Direct-TCP transport frame, the 64-byte sync header (build/parse), and the NEGOTIATE
// exchange. All fields little-endian. Pure host tests against the MS-SMB2 field layout.

#include "services/smb/smb2.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// little-endian writers (mirror the codec's internal ones) for building response vectors
static void w16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
static void w32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}
static uint16_t r16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

void test_transport_frame()
{
    const uint8_t msg[] = {1, 2, 3, 4, 5};
    uint8_t out[16];
    size_t n = smb2_transport_frame(out, sizeof(out), msg, sizeof(msg));
    TEST_ASSERT_EQUAL_size_t(4 + 5, n);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[0]); // Direct-TCP: leading zero
    TEST_ASSERT_EQUAL_HEX8(0x00, out[1]); // 24-bit big-endian length = 5
    TEST_ASSERT_EQUAL_HEX8(0x00, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x05, out[3]);
    TEST_ASSERT_EQUAL_MEMORY(msg, out + 4, 5);
    TEST_ASSERT_EQUAL_UINT32(5, smb2_transport_len(out, n));
    // fail closed: too small, and a non-zero leading byte
    TEST_ASSERT_EQUAL_size_t(0, smb2_transport_frame(out, 3, msg, sizeof(msg)));
    uint8_t bad[4] = {0x01, 0, 0, 5};
    TEST_ASSERT_EQUAL_UINT32(0, smb2_transport_len(bad, 4));
}

void test_build_and_parse_header()
{
    uint8_t buf[64];
    TEST_ASSERT_EQUAL_size_t(64, smb2_build_header(buf, sizeof(buf), SMB2_TREE_CONNECT, 8, 0x1122334455667788ULL,
                                                   0xABCD, 0x99AABBCCDDEEFF00ULL));
    // ProtocolId + StructureSize + Command at their offsets
    const uint8_t pid[4] = {0xFE, 'S', 'M', 'B'};
    TEST_ASSERT_EQUAL_MEMORY(pid, buf, 4);
    TEST_ASSERT_EQUAL_UINT16(64, r16(buf + 4));
    TEST_ASSERT_EQUAL_UINT16(SMB2_TREE_CONNECT, r16(buf + 12));

    Smb2Header h;
    TEST_ASSERT_TRUE(smb2_parse_header(buf, sizeof(buf), &h));
    TEST_ASSERT_EQUAL_UINT16(SMB2_TREE_CONNECT, h.command);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455667788ULL, h.message_id);
    TEST_ASSERT_EQUAL_HEX32(0xABCD, h.tree_id);
    TEST_ASSERT_EQUAL_HEX64(0x99AABBCCDDEEFF00ULL, h.session_id);
}

void test_parse_header_rejects()
{
    uint8_t buf[64];
    smb2_build_header(buf, sizeof(buf), SMB2_NEGOTIATE, 1, 0, 0, 0);
    Smb2Header h;
    TEST_ASSERT_FALSE(smb2_parse_header(buf, 63, &h)); // too short
    uint8_t b2[64];
    memcpy(b2, buf, 64);
    b2[0] = 0x00; // bad ProtocolId
    TEST_ASSERT_FALSE(smb2_parse_header(b2, 64, &h));
    memcpy(b2, buf, 64);
    w16(b2 + 4, 63); // bad StructureSize
    TEST_ASSERT_FALSE(smb2_parse_header(b2, 64, &h));
}

void test_build_negotiate()
{
    uint8_t gid[16];
    for (int i = 0; i < 16; i++)
        gid[i] = (uint8_t)(0x10 + i);
    uint8_t buf[160];
    size_t n = smb2_build_negotiate(buf, sizeof(buf), gid, SMB2_NEGOTIATE_SIGNING_ENABLED);
    TEST_ASSERT_EQUAL_size_t(64 + 36 + 8, n); // header + fixed body + 4 dialects

    Smb2Header h;
    TEST_ASSERT_TRUE(smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(SMB2_NEGOTIATE, h.command);

    const uint8_t *b = buf + 64;              // NEGOTIATE request body
    TEST_ASSERT_EQUAL_UINT16(36, r16(b + 0)); // StructureSize
    TEST_ASSERT_EQUAL_UINT16(4, r16(b + 2));  // DialectCount
    TEST_ASSERT_EQUAL_UINT16(SMB2_NEGOTIATE_SIGNING_ENABLED, r16(b + 4));
    TEST_ASSERT_EQUAL_MEMORY(gid, b + 12, 16); // ClientGuid
    TEST_ASSERT_EQUAL_UINT16(SMB2_DIALECT_0202, r16(b + 36));
    TEST_ASSERT_EQUAL_UINT16(SMB2_DIALECT_0210, r16(b + 38));
    TEST_ASSERT_EQUAL_UINT16(SMB2_DIALECT_0300, r16(b + 40));
    TEST_ASSERT_EQUAL_UINT16(SMB2_DIALECT_0302, r16(b + 42));
    // overflow fails closed
    TEST_ASSERT_EQUAL_size_t(0, smb2_build_negotiate(buf, 100, gid, 0));
}

// Build a well-formed NEGOTIATE response message into m; returns its length.
static size_t build_neg_resp(uint8_t *m, uint16_t dialect, const uint8_t *sec, uint16_t sec_len)
{
    smb2_build_header(m, 256, SMB2_NEGOTIATE, 1, 5, 0, 0);
    m[16] |= 0x01; // SMB2_FLAGS_SERVER_TO_REDIR
    uint8_t *b = m + 64;
    memset(b, 0, 64);
    w16(b + 0, 65);                              // StructureSize
    w16(b + 2, SMB2_NEGOTIATE_SIGNING_REQUIRED); // SecurityMode
    w16(b + 4, dialect);                         // DialectRevision
    for (int i = 0; i < 16; i++)
        b[8 + i] = (uint8_t)(0xA0 + i); // ServerGuid
    w32(b + 24, 0);                     // Capabilities
    w32(b + 28, 0x00100000);            // MaxTransactSize
    w32(b + 32, 0x00080000);            // MaxReadSize
    w32(b + 36, 0x00040000);            // MaxWriteSize
    size_t total = 128;                 // header + 64-byte fixed body
    uint16_t off = 0;
    if (sec_len)
    {
        off = 128;
        memcpy(m + off, sec, sec_len);
        total = (size_t)off + sec_len;
    }
    w16(b + 56, off);     // SecurityBufferOffset
    w16(b + 58, sec_len); // SecurityBufferLength
    return total;
}

void test_parse_negotiate_response()
{
    const uint8_t token[] = {0x60, 0x28, 0x06, 0x06, 'S', 'P', 'N', 'E'}; // a fake SPNEGO-ish blob
    uint8_t m[256];
    size_t n = build_neg_resp(m, SMB2_DIALECT_0300, token, sizeof(token));

    Smb2NegotiateResp r;
    TEST_ASSERT_TRUE(smb2_parse_negotiate_response(m, n, &r));
    TEST_ASSERT_EQUAL_UINT16(SMB2_DIALECT_0300, r.dialect);
    TEST_ASSERT_EQUAL_UINT16(SMB2_NEGOTIATE_SIGNING_REQUIRED, r.security_mode);
    TEST_ASSERT_EQUAL_UINT32(0x00080000, r.max_read);
    TEST_ASSERT_EQUAL_UINT32(0x00040000, r.max_write);
    TEST_ASSERT_EQUAL_HEX8(0xA0, r.server_guid[0]);
    TEST_ASSERT_EQUAL_UINT16(sizeof(token), r.sec_buf_len);
    TEST_ASSERT_EQUAL_MEMORY(token, r.sec_buf, sizeof(token));

    // an empty security buffer -> nullptr, still valid
    n = build_neg_resp(m, SMB2_DIALECT_0210, nullptr, 0);
    TEST_ASSERT_TRUE(smb2_parse_negotiate_response(m, n, &r));
    TEST_ASSERT_NULL(r.sec_buf);
    TEST_ASSERT_EQUAL_UINT16(0, r.sec_buf_len);
}

void test_parse_negotiate_response_rejects()
{
    const uint8_t token[] = {1, 2, 3, 4};
    uint8_t m[256];
    size_t n = build_neg_resp(m, SMB2_DIALECT_0202, token, sizeof(token));
    Smb2NegotiateResp r;

    uint8_t bad[256];
    memcpy(bad, m, n);
    w16(bad + 64, 64); // wrong StructureSize (must be 65)
    TEST_ASSERT_FALSE(smb2_parse_negotiate_response(bad, n, &r));

    memcpy(bad, m, n);
    w16(bad + 12, SMB2_READ); // wrong command
    TEST_ASSERT_FALSE(smb2_parse_negotiate_response(bad, n, &r));

    memcpy(bad, m, n);
    w16(bad + 64 + 58, 5000); // SecurityBufferLength past the message
    TEST_ASSERT_FALSE(smb2_parse_negotiate_response(bad, n, &r));

    TEST_ASSERT_FALSE(smb2_parse_negotiate_response(m, 100, &r)); // truncated before the body
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_transport_frame);
    RUN_TEST(test_build_and_parse_header);
    RUN_TEST(test_parse_header_rejects);
    RUN_TEST(test_build_negotiate);
    RUN_TEST(test_parse_negotiate_response);
    RUN_TEST(test_parse_negotiate_response_rejects);
    return UNITY_END();
}
