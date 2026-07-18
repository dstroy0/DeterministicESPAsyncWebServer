// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SMB2 client wire codec (services/smb, MS-SMB2): the Direct-TCP transport
// frame, the 64-byte sync header (build/parse), the NEGOTIATE exchange, and the SESSION_SETUP
// request/response framing - including a full auth round routed through the framing (SPNEGO +
// NTLMSSP). All fields little-endian. Pure host tests against the MS-SMB2 field layout.

#include "services/smb/ntlmssp.h"
#include "services/smb/smb2.h"
#include "services/smb/spnego.h"
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
static void w64(uint8_t *p, uint64_t v)
{
    w32(p, (uint32_t)v);
    w32(p + 4, (uint32_t)(v >> 32));
}
static uint16_t r16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}
static uint32_t r32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint64_t r64(const uint8_t *p)
{
    return (uint64_t)r32(p) | ((uint64_t)r32(p + 4) << 32);
}

void test_transport_frame()
{
    const uint8_t msg[] = {1, 2, 3, 4, 5};
    uint8_t out[16];
    size_t n = dws_smb2_transport_frame(out, sizeof(out), msg, sizeof(msg));
    TEST_ASSERT_EQUAL_size_t(4 + 5, n);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[0]); // Direct-TCP: leading zero
    TEST_ASSERT_EQUAL_HEX8(0x00, out[1]); // 24-bit big-endian length = 5
    TEST_ASSERT_EQUAL_HEX8(0x00, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x05, out[3]);
    TEST_ASSERT_EQUAL_MEMORY(msg, out + 4, 5);
    TEST_ASSERT_EQUAL_UINT32(5, dws_smb2_transport_len(out, n));
    // fail closed: too small, and a non-zero leading byte
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_transport_frame(out, 3, msg, sizeof(msg)));
    uint8_t bad[4] = {0x01, 0, 0, 5};
    TEST_ASSERT_EQUAL_UINT32(0, dws_smb2_transport_len(bad, 4));
}

void test_build_and_parse_header()
{
    uint8_t buf[64];
    TEST_ASSERT_EQUAL_size_t(64, dws_smb2_build_header(buf, sizeof(buf), Smb2Command::SMB2_TREE_CONNECT, 8,
                                                       0x1122334455667788ULL, 0xABCD, 0x99AABBCCDDEEFF00ULL));
    // ProtocolId + StructureSize + Command at their offsets
    const uint8_t pid[4] = {0xFE, 'S', 'M', 'B'};
    TEST_ASSERT_EQUAL_MEMORY(pid, buf, 4);
    TEST_ASSERT_EQUAL_UINT16(64, r16(buf + 4));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_TREE_CONNECT, r16(buf + 12));

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, sizeof(buf), &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_TREE_CONNECT, h.command);
    TEST_ASSERT_EQUAL_HEX64(0x1122334455667788ULL, h.message_id);
    TEST_ASSERT_EQUAL_HEX32(0xABCD, h.tree_id);
    TEST_ASSERT_EQUAL_HEX64(0x99AABBCCDDEEFF00ULL, h.session_id);
}

void test_parse_header_rejects()
{
    uint8_t buf[64];
    dws_smb2_build_header(buf, sizeof(buf), Smb2Command::SMB2_NEGOTIATE, 1, 0, 0, 0);
    Smb2Header h;
    TEST_ASSERT_FALSE(dws_smb2_parse_header(buf, 63, &h)); // too short
    uint8_t b2[64];
    memcpy(b2, buf, 64);
    b2[0] = 0x00; // bad ProtocolId
    TEST_ASSERT_FALSE(dws_smb2_parse_header(b2, 64, &h));
    memcpy(b2, buf, 64);
    w16(b2 + 4, 63); // bad StructureSize
    TEST_ASSERT_FALSE(dws_smb2_parse_header(b2, 64, &h));
}

void test_build_negotiate()
{
    uint8_t gid[16];
    for (int i = 0; i < 16; i++)
        gid[i] = (uint8_t)(0x10 + i);
    uint8_t buf[160];
    size_t n = dws_smb2_build_negotiate(buf, sizeof(buf), gid, Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_ENABLED);
    TEST_ASSERT_EQUAL_size_t(64 + 36 + 8, n); // header + fixed body + 4 dialects

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_NEGOTIATE, h.command);

    const uint8_t *b = buf + 64;              // NEGOTIATE request body
    TEST_ASSERT_EQUAL_UINT16(36, r16(b + 0)); // StructureSize
    TEST_ASSERT_EQUAL_UINT16(4, r16(b + 2));  // DialectCount
    TEST_ASSERT_EQUAL_UINT16(Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_ENABLED, r16(b + 4));
    TEST_ASSERT_EQUAL_MEMORY(gid, b + 12, 16); // ClientGuid
    TEST_ASSERT_EQUAL_UINT16(Smb2Dialect::SMB2_DIALECT_0202, r16(b + 36));
    TEST_ASSERT_EQUAL_UINT16(Smb2Dialect::SMB2_DIALECT_0210, r16(b + 38));
    TEST_ASSERT_EQUAL_UINT16(Smb2Dialect::SMB2_DIALECT_0300, r16(b + 40));
    TEST_ASSERT_EQUAL_UINT16(Smb2Dialect::SMB2_DIALECT_0302, r16(b + 42));
    // overflow fails closed
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_negotiate(buf, 100, gid, 0));
}

// Build a well-formed NEGOTIATE response message into m; returns its length.
static size_t build_neg_resp(uint8_t *m, Smb2Dialect dialect, const uint8_t *sec, uint16_t sec_len)
{
    dws_smb2_build_header(m, 256, Smb2Command::SMB2_NEGOTIATE, 1, 5, 0, 0);
    m[16] |= 0x01; // Smb2HeaderFlags::SMB2_FLAGS_SERVER_TO_REDIR
    uint8_t *b = m + 64;
    memset(b, 0, 64);
    w16(b + 0, 65);                                                // StructureSize
    w16(b + 2, Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_REQUIRED); // SecurityMode
    w16(b + 4, (uint16_t)dialect);                                 // DialectRevision
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
    size_t n = build_neg_resp(m, Smb2Dialect::SMB2_DIALECT_0300, token, sizeof(token));

    Smb2NegotiateResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_negotiate_response(m, n, &r));
    TEST_ASSERT_EQUAL_UINT16(Smb2Dialect::SMB2_DIALECT_0300, r.dialect);
    TEST_ASSERT_EQUAL_UINT16(Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_REQUIRED, r.security_mode);
    TEST_ASSERT_EQUAL_UINT32(0x00080000, r.max_read);
    TEST_ASSERT_EQUAL_UINT32(0x00040000, r.max_write);
    TEST_ASSERT_EQUAL_HEX8(0xA0, r.server_guid[0]);
    TEST_ASSERT_EQUAL_UINT16(sizeof(token), r.sec_buf_len);
    TEST_ASSERT_EQUAL_MEMORY(token, r.sec_buf, sizeof(token));

    // an empty security buffer -> nullptr, still valid
    n = build_neg_resp(m, Smb2Dialect::SMB2_DIALECT_0210, nullptr, 0);
    TEST_ASSERT_TRUE(dws_smb2_parse_negotiate_response(m, n, &r));
    TEST_ASSERT_NULL(r.sec_buf);
    TEST_ASSERT_EQUAL_UINT16(0, r.sec_buf_len);
}

void test_parse_negotiate_response_rejects()
{
    const uint8_t token[] = {1, 2, 3, 4};
    uint8_t m[256];
    size_t n = build_neg_resp(m, Smb2Dialect::SMB2_DIALECT_0202, token, sizeof(token));
    Smb2NegotiateResp r;

    uint8_t bad[256];
    memcpy(bad, m, n);
    w16(bad + 64, 64); // wrong StructureSize (must be 65)
    TEST_ASSERT_FALSE(dws_smb2_parse_negotiate_response(bad, n, &r));

    memcpy(bad, m, n);
    w16(bad + 12, (uint16_t)Smb2Command::SMB2_READ); // wrong command
    TEST_ASSERT_FALSE(dws_smb2_parse_negotiate_response(bad, n, &r));

    memcpy(bad, m, n);
    w16(bad + 64 + 58, 5000); // SecurityBufferLength past the message
    TEST_ASSERT_FALSE(dws_smb2_parse_negotiate_response(bad, n, &r));

    TEST_ASSERT_FALSE(dws_smb2_parse_negotiate_response(m, 100, &r)); // truncated before the body
}

void test_build_session_setup()
{
    uint8_t tok[40];
    for (int i = 0; i < 40; i++)
        tok[i] = (uint8_t)(i + 1);
    uint8_t buf[256];
    size_t n = dws_smb2_build_session_setup(buf, sizeof(buf), 7, 0xDEADBEEFULL,
                                            Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_ENABLED, tok, sizeof(tok));
    TEST_ASSERT_EQUAL_size_t(64 + 24 + 40, n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_SESSION_SETUP, h.command);
    TEST_ASSERT_EQUAL_HEX64(0xDEADBEEFULL, h.session_id); // echoes the server SessionId
    TEST_ASSERT_EQUAL_HEX64(7, h.message_id);

    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(25, r16(b + 0)); // StructureSize
    TEST_ASSERT_EQUAL_HEX8(Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_ENABLED, b[3]);
    TEST_ASSERT_EQUAL_UINT16(64 + 24, r16(b + 12)); // SecurityBufferOffset = 88
    TEST_ASSERT_EQUAL_UINT16(40, r16(b + 14));      // SecurityBufferLength
    TEST_ASSERT_EQUAL_MEMORY(tok, buf + 88, 40);
    // overflow + empty token fail closed
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_session_setup(buf, 100, 7, 0, 0, tok, sizeof(tok)));
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_session_setup(buf, sizeof(buf), 7, 0, 0, tok, 0));
}

// Build a well-formed SESSION_SETUP response message into m; returns its length.
static size_t build_ss_resp(uint8_t *m, uint64_t session_id, uint32_t status, uint16_t flags, const uint8_t *sec,
                            uint16_t sec_len)
{
    dws_smb2_build_header(m, 512, Smb2Command::SMB2_SESSION_SETUP, 1, 6, 0, session_id);
    w32(m + 8, status); // Status (STATUS_MORE_PROCESSING_REQUIRED then SUCCESS)
    m[16] |= 0x01;      // Smb2HeaderFlags::SMB2_FLAGS_SERVER_TO_REDIR
    uint8_t *b = m + 64;
    memset(b, 0, 8);
    w16(b + 0, 9);     // StructureSize
    w16(b + 2, flags); // SessionFlags
    size_t total = 72; // header + 8-byte fixed body
    uint16_t off = 0;
    if (sec_len)
    {
        off = 72;
        memcpy(m + off, sec, sec_len);
        total = (size_t)off + sec_len;
    }
    w16(b + 4, off);     // SecurityBufferOffset
    w16(b + 6, sec_len); // SecurityBufferLength
    return total;
}

void test_parse_session_setup_response()
{
    const uint8_t tok[] = {0xa1, 0x05, 'c', 'h', 'a', 'l'};
    uint8_t m[256];
    size_t n = build_ss_resp(m, 0x1234ULL, Smb2Status::SMB2_STATUS_MORE_PROCESSING_REQUIRED,
                             Smb2SessionFlags::SMB2_SESSION_FLAG_IS_GUEST, tok, sizeof(tok));

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(m, n, &h));
    TEST_ASSERT_EQUAL_HEX32(Smb2Status::SMB2_STATUS_MORE_PROCESSING_REQUIRED, h.status);
    TEST_ASSERT_EQUAL_HEX64(0x1234ULL, h.session_id);

    Smb2SessionSetupResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_session_setup_response(m, n, &r));
    TEST_ASSERT_EQUAL_UINT16(Smb2SessionFlags::SMB2_SESSION_FLAG_IS_GUEST, r.session_flags);
    TEST_ASSERT_EQUAL_UINT16(sizeof(tok), r.sec_buf_len);
    TEST_ASSERT_EQUAL_MEMORY(tok, r.sec_buf, sizeof(tok));

    // the final SUCCESS round carries no security buffer -> nullptr, still valid
    n = build_ss_resp(m, 0x1234ULL, Smb2Status::SMB2_STATUS_SUCCESS, 0, nullptr, 0);
    TEST_ASSERT_TRUE(dws_smb2_parse_session_setup_response(m, n, &r));
    TEST_ASSERT_NULL(r.sec_buf);
    TEST_ASSERT_EQUAL_UINT16(0, r.sec_buf_len);
}

void test_session_setup_rejects()
{
    const uint8_t tok[] = {1, 2, 3, 4};
    uint8_t m[256];
    size_t n = build_ss_resp(m, 1, 0, 0, tok, sizeof(tok));
    Smb2SessionSetupResp r;
    uint8_t bad[256];

    memcpy(bad, m, n);
    w16(bad + 64, 8); // wrong StructureSize (must be 9)
    TEST_ASSERT_FALSE(dws_smb2_parse_session_setup_response(bad, n, &r));
    memcpy(bad, m, n);
    w16(bad + 12, (uint16_t)Smb2Command::SMB2_READ); // wrong command
    TEST_ASSERT_FALSE(dws_smb2_parse_session_setup_response(bad, n, &r));
    memcpy(bad, m, n);
    w16(bad + 64 + 6, 5000); // SecurityBufferLength past the message
    TEST_ASSERT_FALSE(dws_smb2_parse_session_setup_response(bad, n, &r));
    TEST_ASSERT_FALSE(dws_smb2_parse_session_setup_response(m, 68, &r)); // truncated before the body
}

// A minimal NTLMSSP CHALLENGE (server type-2) with the given server challenge and a single-EOL
// target info, so the client can parse it out at the end of the SESSION_SETUP flow.
static size_t build_ntlmssp_challenge(uint8_t *m, const uint8_t sc[8])
{
    memset(m, 0, 52);
    const uint8_t sig[8] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};
    memcpy(m, sig, 8);
    w32(m + 8, 2); // MessageType CHALLENGE
    w32(m + 20, NtlmsspFlags::NTLMSSP_NEGOTIATE_UNICODE | NtlmsspFlags::NTLMSSP_NEGOTIATE_NTLM |
                    NtlmsspFlags::NTLMSSP_NEGOTIATE_TARGET_INFO);
    memcpy(m + 24, sc, 8); // ServerChallenge
    w16(m + 40, 4);        // TargetInfoLen (a lone MsvAvEOL pair)
    w16(m + 42, 4);
    w32(m + 44, 48); // TargetInfoBufferOffset
    // m[48..51] = 00 00 00 00 (AvId=0, AvLen=0)
    return 52;
}

// End to end through the SESSION_SETUP framing: the client wraps an NTLMSSP NEGOTIATE in SPNEGO and
// frames it as a request; the server's SESSION_SETUP response carries a SPNEGO-wrapped CHALLENGE;
// the client unwinds framing -> SPNEGO -> NTLMSSP and recovers the server challenge intact.
void test_session_setup_spnego_flow()
{
    uint8_t neg[64];
    size_t neg_n = dws_ntlmssp_build_negotiate(neg, sizeof(neg), NtlmsspFlags::NTLMSSP_CLIENT_DEFAULT_FLAGS);
    uint8_t spnego[128];
    size_t sp_n = dws_spnego_wrap_negotiate(neg, neg_n, spnego, sizeof(spnego));
    uint8_t req[256];
    size_t req_n = dws_smb2_build_session_setup(req, sizeof(req), 1, 0,
                                                Smb2SecurityMode::SMB2_NEGOTIATE_SIGNING_ENABLED, spnego, sp_n);
    TEST_ASSERT_GREATER_THAN_size_t(0, req_n);
    TEST_ASSERT_EQUAL_MEMORY(spnego, req + 88, sp_n); // the token is framed at offset 88

    const uint8_t sc[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t chal[64];
    size_t chal_n = build_ntlmssp_challenge(chal, sc);
    uint8_t srv_tok[128];
    size_t srv_n = dws_spnego_wrap_authenticate(chal, chal_n, srv_tok, sizeof(srv_tok)); // server NegTokenResp shape
    uint8_t resp[256];
    size_t dws_resp_n =
        build_ss_resp(resp, 0xABCDULL, Smb2Status::SMB2_STATUS_MORE_PROCESSING_REQUIRED, 0, srv_tok, (uint16_t)srv_n);

    Smb2SessionSetupResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_session_setup_response(resp, dws_resp_n, &r));
    const uint8_t *ct = nullptr;
    size_t cl = 0;
    TEST_ASSERT_TRUE(dws_spnego_parse_response(r.sec_buf, r.sec_buf_len, &ct, &cl));
    NtlmChallenge nch;
    TEST_ASSERT_TRUE(dws_ntlmssp_parse_challenge(ct, cl, &nch));
    TEST_ASSERT_EQUAL_MEMORY(sc, nch.server_challenge, 8); // survived framing -> SPNEGO -> NTLMSSP
}

void test_build_tree_connect()
{
    const uint8_t path[] = {'\\', 0, '\\', 0, 's', 0, 'r', 0, 'v', 0, '\\', 0, 's', 0, 'h', 0}; // \\srv\sh
    uint8_t buf[128];
    size_t n = dws_smb2_build_tree_connect(buf, sizeof(buf), 2, 0xABCDULL, path, sizeof(path));
    TEST_ASSERT_EQUAL_size_t(64 + 8 + sizeof(path), n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_TREE_CONNECT, h.command);
    TEST_ASSERT_EQUAL_HEX64(0xABCDULL, h.session_id);

    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(9, r16(b + 0));            // StructureSize
    TEST_ASSERT_EQUAL_UINT16(72, r16(b + 4));           // PathOffset
    TEST_ASSERT_EQUAL_UINT16(sizeof(path), r16(b + 6)); // PathLength
    TEST_ASSERT_EQUAL_MEMORY(path, buf + 72, sizeof(path));
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_tree_connect(buf, 60, 2, 0, path, sizeof(path))); // overflow
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_tree_connect(buf, sizeof(buf), 2, 0, path, 0));   // empty path
}

static size_t build_tc_resp(uint8_t *m, uint32_t tree_id, uint8_t share_type, uint32_t maximal_access)
{
    dws_smb2_build_header(m, 128, Smb2Command::SMB2_TREE_CONNECT, 1, 2, tree_id, 0xABCD);
    m[16] |= 0x01; // SERVER_TO_REDIR
    uint8_t *b = m + 64;
    memset(b, 0, 16);
    w16(b + 0, 16);    // StructureSize
    b[2] = share_type; // ShareType
    w32(b + 12, maximal_access);
    return 64 + 16;
}

void test_parse_tree_connect_response()
{
    uint8_t m[128];
    size_t n = build_tc_resp(m, 0x777, Smb2ShareType::SMB2_SHARE_TYPE_DISK, 0x001f01ff);
    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(m, n, &h));
    TEST_ASSERT_EQUAL_HEX32(0x777, h.tree_id); // TreeId comes from the header

    Smb2TreeConnectResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_tree_connect_response(m, n, &r));
    TEST_ASSERT_EQUAL_HEX8(Smb2ShareType::SMB2_SHARE_TYPE_DISK, r.share_type);
    TEST_ASSERT_EQUAL_HEX32(0x001f01ff, r.maximal_access);

    uint8_t bad[128];
    memcpy(bad, m, n);
    w16(bad + 64, 15); // wrong StructureSize (must be 16)
    TEST_ASSERT_FALSE(dws_smb2_parse_tree_connect_response(bad, n, &r));
    TEST_ASSERT_FALSE(dws_smb2_parse_tree_connect_response(m, 70, &r)); // truncated
}

void test_build_create()
{
    const uint8_t name[] = {'a', 0, '.', 0, 'n', 0, 'c', 0}; // "a.nc" UTF-16LE
    uint8_t buf[256];
    size_t n = dws_smb2_build_create(buf, sizeof(buf), 3, 0xAAAA, 0x777, Smb2Access::SMB2_FILE_GENERIC_READ,
                                     Smb2ShareAccess::SMB2_FILE_SHARE_READ, Smb2Disposition::SMB2_FILE_OPEN,
                                     Smb2CreateOptions::SMB2_FILE_NON_DIRECTORY_FILE, name, sizeof(name));
    TEST_ASSERT_EQUAL_size_t(64 + 56 + sizeof(name), n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_CREATE, h.command);
    TEST_ASSERT_EQUAL_HEX32(0x777, h.tree_id);

    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(57, r16(b + 0)); // StructureSize
    TEST_ASSERT_EQUAL_UINT32(2, r32(b + 4));  // ImpersonationLevel
    TEST_ASSERT_EQUAL_UINT32(Smb2Access::SMB2_FILE_GENERIC_READ, r32(b + 24));
    TEST_ASSERT_EQUAL_UINT32(Smb2ShareAccess::SMB2_FILE_SHARE_READ, r32(b + 32));
    TEST_ASSERT_EQUAL_UINT32(Smb2Disposition::SMB2_FILE_OPEN, r32(b + 36));
    TEST_ASSERT_EQUAL_UINT32(Smb2CreateOptions::SMB2_FILE_NON_DIRECTORY_FILE, r32(b + 40));
    TEST_ASSERT_EQUAL_UINT16(120, r16(b + 44));          // NameOffset
    TEST_ASSERT_EQUAL_UINT16(sizeof(name), r16(b + 46)); // NameLength
    TEST_ASSERT_EQUAL_MEMORY(name, buf + 120, sizeof(name));
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_create(buf, 100, 3, 0, 0, 0, 0, 0, 0, name, sizeof(name))); // overflow
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_create(buf, sizeof(buf), 3, 0, 0, 0, 0, 0, 0, name, 0));    // empty name
}

static size_t build_create_resp(uint8_t *m, const uint8_t fid[16], uint64_t eof, uint32_t action)
{
    dws_smb2_build_header(m, 256, Smb2Command::SMB2_CREATE, 1, 3, 0x777, 0xAAAA);
    m[16] |= 0x01;
    uint8_t *b = m + 64;
    memset(b, 0, 88);
    w16(b + 0, 89);     // StructureSize
    w32(b + 4, action); // CreateAction
    w64(b + 48, eof);   // EndofFile
    w32(b + 56, 0x80);  // FileAttributes = NORMAL
    memcpy(b + 64, fid, 16);
    return 64 + 88;
}

void test_parse_create_response()
{
    uint8_t fid[16];
    for (int i = 0; i < 16; i++)
        fid[i] = (uint8_t)(0xF0 + i);
    uint8_t m[256];
    size_t n = build_create_resp(m, fid, 0x123456789ULL, 1);

    Smb2CreateResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_create_response(m, n, &r));
    TEST_ASSERT_EQUAL_MEMORY(fid, r.file_id, 16);
    TEST_ASSERT_EQUAL_HEX64(0x123456789ULL, r.end_of_file);
    TEST_ASSERT_EQUAL_UINT32(1, r.create_action);
    TEST_ASSERT_EQUAL_HEX32(0x80, r.file_attributes);

    uint8_t bad[256];
    memcpy(bad, m, n);
    w16(bad + 64, 88); // wrong StructureSize (must be 89)
    TEST_ASSERT_FALSE(dws_smb2_parse_create_response(bad, n, &r));
    TEST_ASSERT_FALSE(dws_smb2_parse_create_response(m, 100, &r)); // truncated
}

void test_close_roundtrip()
{
    uint8_t fid[16];
    for (int i = 0; i < 16; i++)
        fid[i] = (uint8_t)(i + 1);
    uint8_t buf[128];
    size_t n = dws_smb2_build_close(buf, sizeof(buf), 4, 0xAAAA, 0x777, fid);
    TEST_ASSERT_EQUAL_size_t(64 + 24, n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_CLOSE, h.command);
    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(24, r16(b + 0));                                 // StructureSize
    TEST_ASSERT_EQUAL_MEMORY(fid, b + 8, 16);                                 // FileId
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_close(buf, 80, 4, 0, 0, fid)); // overflow

    uint8_t m[128];
    dws_smb2_build_header(m, 128, Smb2Command::SMB2_CLOSE, 1, 4, 0x777, 0xAAAA);
    m[16] |= 0x01;
    uint8_t *rb = m + 64;
    memset(rb, 0, 60);
    w16(rb + 0, 60);         // StructureSize
    w64(rb + 48, 0x4000ULL); // EndofFile
    w32(rb + 56, 0x80);      // FileAttributes
    Smb2CloseResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_close_response(m, 64 + 60, &r));
    TEST_ASSERT_EQUAL_HEX64(0x4000ULL, r.end_of_file);
    TEST_ASSERT_EQUAL_HEX32(0x80, r.file_attributes);
    uint8_t bad[128];
    memcpy(bad, m, 64 + 60);
    w16(bad + 64, 59); // wrong StructureSize (must be 60)
    TEST_ASSERT_FALSE(dws_smb2_parse_close_response(bad, 64 + 60, &r));
}

void test_build_read()
{
    uint8_t fid[16];
    for (int i = 0; i < 16; i++)
        fid[i] = (uint8_t)(0xC0 + i);
    uint8_t buf[128];
    size_t n = dws_smb2_build_read(buf, sizeof(buf), 5, 0xAAAA, 0x777, fid, 0x10000, 0x1000ULL);
    TEST_ASSERT_EQUAL_size_t(64 + 48 + 1, n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_READ, h.command);
    TEST_ASSERT_EQUAL_HEX32(0x777, h.tree_id);

    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(49, r16(b + 0));                                       // StructureSize
    TEST_ASSERT_EQUAL_HEX8(80, b[2]);                                               // Padding = header + 16
    TEST_ASSERT_EQUAL_UINT32(0x10000, r32(b + 4));                                  // Length
    TEST_ASSERT_EQUAL_HEX64(0x1000ULL, r64(b + 8));                                 // Offset
    TEST_ASSERT_EQUAL_MEMORY(fid, b + 16, 16);                                      // FileId
    TEST_ASSERT_EQUAL_UINT32(1, r32(b + 32));                                       // MinimumCount
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_read(buf, 100, 5, 0, 0, fid, 1, 0)); // overflow
}

static size_t build_read_resp(uint8_t *m, const uint8_t *data, uint32_t data_len)
{
    dws_smb2_build_header(m, 512, Smb2Command::SMB2_READ, 1, 5, 0x777, 0xAAAA);
    m[16] |= 0x01;
    uint8_t *b = m + 64;
    memset(b, 0, 16);
    w16(b + 0, 17); // StructureSize
    b[2] = 80;      // DataOffset = header + 16
    w32(b + 4, data_len);
    size_t total = 80;
    if (data_len)
    {
        memcpy(m + 80, data, data_len);
        total = 80 + data_len;
    }
    return total;
}

void test_parse_read_response()
{
    const uint8_t data[] = "N123 G01 X10 Y20\r\n";
    uint8_t m[512];
    size_t n = build_read_resp(m, data, sizeof(data));

    Smb2ReadResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_read_response(m, n, &r));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data), r.data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, r.data, sizeof(data));

    // empty read (EOF) -> nullptr, still valid
    n = build_read_resp(m, nullptr, 0);
    TEST_ASSERT_TRUE(dws_smb2_parse_read_response(m, n, &r));
    TEST_ASSERT_NULL(r.data);
    TEST_ASSERT_EQUAL_UINT32(0, r.data_len);

    // rejects
    n = build_read_resp(m, data, sizeof(data));
    uint8_t bad[512];
    memcpy(bad, m, n);
    w16(bad + 64, 16); // wrong StructureSize (must be 17)
    TEST_ASSERT_FALSE(dws_smb2_parse_read_response(bad, n, &r));
    memcpy(bad, m, n);
    w32(bad + 64 + 4, 9000); // DataLength past the message
    TEST_ASSERT_FALSE(dws_smb2_parse_read_response(bad, n, &r));
    TEST_ASSERT_FALSE(dws_smb2_parse_read_response(m, 70, &r)); // truncated before the body
}

void test_build_write()
{
    uint8_t fid[16];
    for (int i = 0; i < 16; i++)
        fid[i] = (uint8_t)(i + 1);
    const uint8_t data[] = "O0001 (PART)\r\n";
    uint8_t buf[256];
    size_t n = dws_smb2_build_write(buf, sizeof(buf), 6, 0xAAAA, 0x777, fid, data, sizeof(data), 0x800ULL);
    TEST_ASSERT_EQUAL_size_t(64 + 48 + sizeof(data), n);

    Smb2Header h;
    TEST_ASSERT_TRUE(dws_smb2_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT16(Smb2Command::SMB2_WRITE, h.command);

    const uint8_t *b = buf + 64;
    TEST_ASSERT_EQUAL_UINT16(49, r16(b + 0));           // StructureSize
    TEST_ASSERT_EQUAL_UINT16(112, r16(b + 2));          // DataOffset
    TEST_ASSERT_EQUAL_UINT32(sizeof(data), r32(b + 4)); // Length
    TEST_ASSERT_EQUAL_HEX64(0x800ULL, r64(b + 8));      // Offset
    TEST_ASSERT_EQUAL_MEMORY(fid, b + 16, 16);          // FileId
    TEST_ASSERT_EQUAL_MEMORY(data, buf + 112, sizeof(data));
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_write(buf, 100, 6, 0, 0, fid, data, sizeof(data), 0)); // overflow
    TEST_ASSERT_EQUAL_size_t(0, dws_smb2_build_write(buf, sizeof(buf), 6, 0, 0, fid, data, 0, 0));    // empty data
}

void test_parse_write_response()
{
    uint8_t m[128];
    dws_smb2_build_header(m, 128, Smb2Command::SMB2_WRITE, 1, 6, 0x777, 0xAAAA);
    m[16] |= 0x01;
    uint8_t *b = m + 64;
    memset(b, 0, 16);
    w16(b + 0, 17);   // StructureSize
    w32(b + 4, 4096); // Count

    Smb2WriteResp r;
    TEST_ASSERT_TRUE(dws_smb2_parse_write_response(m, 64 + 16, &r));
    TEST_ASSERT_EQUAL_UINT32(4096, r.count);

    uint8_t bad[128];
    memcpy(bad, m, 64 + 16);
    w16(bad + 64, 16); // wrong StructureSize (must be 17)
    TEST_ASSERT_FALSE(dws_smb2_parse_write_response(bad, 64 + 16, &r));
    TEST_ASSERT_FALSE(dws_smb2_parse_write_response(m, 70, &r)); // truncated
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
    RUN_TEST(test_build_session_setup);
    RUN_TEST(test_parse_session_setup_response);
    RUN_TEST(test_session_setup_rejects);
    RUN_TEST(test_session_setup_spnego_flow);
    RUN_TEST(test_build_tree_connect);
    RUN_TEST(test_parse_tree_connect_response);
    RUN_TEST(test_build_create);
    RUN_TEST(test_parse_create_response);
    RUN_TEST(test_close_roundtrip);
    RUN_TEST(test_build_read);
    RUN_TEST(test_parse_read_response);
    RUN_TEST(test_build_write);
    RUN_TEST(test_parse_write_response);
    return UNITY_END();
}
