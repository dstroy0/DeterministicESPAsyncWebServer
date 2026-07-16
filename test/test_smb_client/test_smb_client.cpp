// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SMB2 client dialogue engine (services/smb/smb_client): smb_open drives the
// full NEGOTIATE -> two-round NTLMv2 SESSION_SETUP -> TREE_CONNECT -> CREATE handshake, and
// smb_close releases the handle. Exercised end to end on the host with a scripted mock SMB2 server
// (a send/recv seam), so no lwIP or real share is needed.

#include "services/smb/ntlmssp.h"
#include "services/smb/smb2.h"
#include "services/smb/smb_client.h"
#include "services/smb/spnego.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void w16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
static void w32(uint8_t *p, uint32_t v)
{
    for (int i = 0; i < 4; i++)
        p[i] = (uint8_t)(v >> (8 * i));
}
static void w64(uint8_t *p, uint64_t v)
{
    w32(p, (uint32_t)v);
    w32(p + 4, (uint32_t)(v >> 32));
}
static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}
static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint64_t rd64(const uint8_t *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

// A minimal NTLMSSP CHALLENGE (server type-2) with a timestamp+EOL target info.
static size_t ntlmssp_challenge(uint8_t *m, const uint8_t sc[8])
{
    memset(m, 0, 64);
    const uint8_t sig[8] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};
    memcpy(m, sig, 8);
    w32(m + 8, 2); // CHALLENGE
    w32(m + 20, NtlmsspFlags::NTLMSSP_NEGOTIATE_UNICODE | NtlmsspFlags::NTLMSSP_NEGOTIATE_NTLM |
                    NtlmsspFlags::NTLMSSP_NEGOTIATE_TARGET_INFO);
    memcpy(m + 24, sc, 8);
    // target info at 48: MsvAvTimestamp(7, 8 bytes) + MsvAvEOL
    uint8_t *ti = m + 48;
    w16(ti + 0, 7);
    w16(ti + 2, 8);
    w64(ti + 4, 0x01D000000000ULL); // a FILETIME
    w16(ti + 12, 0);
    w16(ti + 14, 0);
    uint16_t ti_len = 16;
    w16(m + 40, ti_len);
    w16(m + 42, ti_len);
    w32(m + 44, 48);
    return 48 + ti_len;
}

// Same CHALLENGE, but with a caller-supplied target-info blob (exercises find_av_timestamp's
// scan/skip/EOL branches and large target infos that overflow the client's crypto buffers).
static size_t ntlmssp_challenge_ti(uint8_t *m, const uint8_t sc[8], const uint8_t *ti, size_t ti_len)
{
    memset(m, 0, 48);
    const uint8_t sig[8] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};
    memcpy(m, sig, 8);
    w32(m + 8, 2); // CHALLENGE
    w32(m + 20, NtlmsspFlags::NTLMSSP_NEGOTIATE_UNICODE | NtlmsspFlags::NTLMSSP_NEGOTIATE_NTLM |
                    NtlmsspFlags::NTLMSSP_NEGOTIATE_TARGET_INFO);
    memcpy(m + 24, sc, 8);
    memcpy(m + 48, ti, ti_len);
    w16(m + 40, (uint16_t)ti_len);
    w16(m + 42, (uint16_t)ti_len);
    w32(m + 44, 48);
    return 48 + ti_len;
}

// Fault the mock injects on a chosen request number (1-based over the open handshake).
enum MockFault
{
    FAULT_NONE = 0,
    FAULT_DROP,       // append no response (the peer closes mid-handshake)
    FAULT_BAD_HEADER, // corrupt the response ProtocolId so smb2_parse_header fails
    FAULT_BAD_BODY,   // corrupt the response body StructureSize so the body parser fails
};

// How the round-1 SESSION_SETUP security buffer is shaped, to drive the client's decode rejects.
enum SsSecBufMode
{
    SSBUF_NORMAL = 0, // a valid SPNEGO-wrapped NTLMSSP CHALLENGE
    SSBUF_EMPTY,      // no security buffer at all
    SSBUF_RAW_JUNK,   // bytes that are not a SPNEGO NegTokenResp
    SSBUF_SPNEGO_JUNK // a valid SPNEGO wrap whose inner token is not an NTLMSSP CHALLENGE
};

// The scripted mock SMB2 server: on each client request it appends the matching framed response.
struct Mock
{
    uint8_t rx[8192];
    size_t rx_len, rx_pos;
    int ss_round;
    uint64_t session_id;
    uint32_t tree_id;
    uint8_t file_id[16];
    uint64_t file_size;
    uint32_t auth_status; // status for the 2nd SESSION_SETUP (SUCCESS or a logon failure)
    uint32_t tc_status;   // TREE_CONNECT status
    uint32_t create_status;
    bool cut_after_negotiate; // simulate the peer closing mid-handshake
    int req_count;
    uint8_t file_data[8192]; // the "file" backing READ / WRITE
    size_t file_data_len;
    uint32_t ss1_status;    // status for the 1st SESSION_SETUP (default MORE_PROCESSING_REQUIRED)
    int fault_at_req;       // 1-based request number at which to inject fault_kind (0 = none)
    int fault_kind;         // MockFault
    int ss1_secbuf_mode;    // SsSecBufMode for the round-1 security buffer
    const uint8_t *chal_ti; // optional custom target-info for the round-1 CHALLENGE (null => default)
    size_t chal_ti_len;
};

static void append_frame(Mock *m, const uint8_t *resp, size_t rlen)
{
    m->rx_len += smb2_transport_frame(m->rx + m->rx_len, sizeof(m->rx) - m->rx_len, resp, rlen);
}

static int mock_send(void *c, const uint8_t *d, size_t n)
{
    Mock *m = (Mock *)c;
    m->req_count++;
    const uint8_t *msg = d + 4; // skip the Direct-TCP prefix
    size_t mlen = n - 4;
    Smb2Header h;
    if (!smb2_parse_header(msg, mlen, &h))
        return -1;

    uint8_t resp[DETWS_SMB_BUF + 128];
    memset(resp, 0, sizeof(resp));
    size_t rlen = 0;
    uint8_t *b = resp + 64;
    switch (h.command)
    {
    case Smb2Command::SMB2_NEGOTIATE:
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_NEGOTIATE, 1, h.message_id, 0, 0);
        w16(b + 0, 65);                                       // StructureSize
        w16(b + 4, (uint16_t)Smb2Dialect::SMB2_DIALECT_0210); // DialectRevision
        rlen = 128;                                           // header + 64-byte fixed body, empty security buffer
        break;
    case Smb2Command::SMB2_SESSION_SETUP: {
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_SESSION_SETUP, 1, h.message_id, 0, m->session_id);
        w16(b + 0, 9); // StructureSize
        if (m->ss_round++ == 0)
        {
            w32(resp + 8, m->ss1_status);
            if (m->ss1_secbuf_mode == SSBUF_EMPTY)
            {
                w16(b + 4, 0); // SecurityBufferOffset
                w16(b + 6, 0); // SecurityBufferLength (nullptr sec_buf on the client)
                rlen = 72;
            }
            else
            {
                uint8_t chal[1024];
                uint8_t sctok[1200];
                const uint8_t sc[8] = {1, 2, 3, 4, 5, 6, 7, 8};
                size_t sc_n = 0;
                if (m->ss1_secbuf_mode == SSBUF_RAW_JUNK)
                {
                    memset(sctok, 0x77, 16); // not a SPNEGO NegTokenResp
                    sc_n = 16;
                }
                else if (m->ss1_secbuf_mode == SSBUF_SPNEGO_JUNK)
                {
                    uint8_t junk[16];
                    memset(junk, 0x55, sizeof(junk)); // wrapped, but too short/wrong to be an NTLMSSP CHALLENGE
                    sc_n = spnego_wrap_authenticate(junk, sizeof(junk), sctok, sizeof(sctok));
                }
                else
                {
                    size_t chal_n = m->chal_ti ? ntlmssp_challenge_ti(chal, sc, m->chal_ti, m->chal_ti_len)
                                               : ntlmssp_challenge(chal, sc);
                    sc_n = spnego_wrap_authenticate(chal, chal_n, sctok, sizeof(sctok)); // NegTokenResp shape
                }
                w16(b + 4, 72); // SecurityBufferOffset
                w16(b + 6, (uint16_t)sc_n);
                memcpy(resp + 72, sctok, sc_n);
                rlen = 72 + sc_n;
            }
        }
        else
        {
            w32(resp + 8, m->auth_status);
            rlen = 72; // header + 8-byte body, empty buffer
        }
        break;
    }
    case Smb2Command::SMB2_TREE_CONNECT:
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_TREE_CONNECT, 1, h.message_id, m->tree_id,
                          m->session_id);
        w32(resp + 8, m->tc_status);
        w16(b + 0, 16); // StructureSize
        b[2] = Smb2ShareType::SMB2_SHARE_TYPE_DISK;
        rlen = 64 + 16;
        break;
    case Smb2Command::SMB2_CREATE:
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_CREATE, 1, h.message_id, m->tree_id, m->session_id);
        w32(resp + 8, m->create_status);
        w16(b + 0, 89); // StructureSize
        w32(b + 4, 1);  // CreateAction = FILE_OPENED
        w64(b + 48, m->file_size);
        memcpy(b + 64, m->file_id, 16);
        rlen = 64 + 88;
        break;
    case Smb2Command::SMB2_READ: {
        const uint8_t *rq = msg + 64; // READ request body
        uint32_t length = rd32(rq + 4);
        uint64_t off = rd64(rq + 8);
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_READ, 1, h.message_id, m->tree_id, m->session_id);
        if (off >= m->file_data_len)
        {
            w32(resp + 8, Smb2Status::SMB2_STATUS_END_OF_FILE);
            w16(b + 0, 17); // StructureSize, no data
            rlen = 64 + 16;
        }
        else
        {
            uint32_t avail = (uint32_t)(m->file_data_len - off);
            uint32_t n2 = length < avail ? length : avail;
            w16(b + 0, 17); // StructureSize
            b[2] = 80;      // DataOffset (header + 16-byte body)
            w32(b + 4, n2); // DataLength
            memcpy(resp + 80, m->file_data + off, n2);
            rlen = 80 + n2;
        }
        break;
    }
    case Smb2Command::SMB2_WRITE: {
        const uint8_t *wq = msg + 64; // WRITE request body
        uint16_t data_off = rd16(wq + 2);
        uint32_t length = rd32(wq + 4);
        uint64_t off = rd64(wq + 8);
        if (off + length <= sizeof(m->file_data))
        {
            memcpy(m->file_data + off, msg + data_off, length);
            if (off + length > m->file_data_len)
                m->file_data_len = (size_t)(off + length);
        }
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_WRITE, 1, h.message_id, m->tree_id, m->session_id);
        w16(b + 0, 17);     // StructureSize
        w32(b + 4, length); // Count
        rlen = 64 + 16;
        break;
    }
    case Smb2Command::SMB2_CLOSE:
        smb2_build_header(resp, sizeof(resp), Smb2Command::SMB2_CLOSE, 1, h.message_id, m->tree_id, m->session_id);
        w16(b + 0, 60); // StructureSize
        rlen = 64 + 60;
        break;
    default:
        return -1;
    }
    resp[16] |= 0x01; // Smb2HeaderFlags::SMB2_FLAGS_SERVER_TO_REDIR
    bool drop = m->cut_after_negotiate && h.command != Smb2Command::SMB2_NEGOTIATE;
    if (m->fault_at_req == m->req_count)
    {
        if (m->fault_kind == FAULT_DROP)
            drop = true;
        else if (m->fault_kind == FAULT_BAD_HEADER)
            resp[0] = 0x00; // break the ProtocolId magic (FE 53 4D 42) -> smb2_parse_header fails
        else if (m->fault_kind == FAULT_BAD_BODY)
            w16(resp + 64, 0xFFFF); // break the body StructureSize -> the body parser fails
    }
    if (!drop)
        append_frame(m, resp, rlen);
    return (int)n;
}

static int mock_recv(void *c, uint8_t *buf, size_t cap)
{
    Mock *m = (Mock *)c;
    if (m->rx_pos >= m->rx_len)
        return 0; // peer has nothing more -> closed
    size_t avail = m->rx_len - m->rx_pos;
    size_t take = avail < cap ? avail : cap;
    memcpy(buf, m->rx + m->rx_pos, take);
    m->rx_pos += take;
    return (int)take;
}

static Mock make_mock()
{
    Mock m;
    memset(&m, 0, sizeof(m));
    m.session_id = 0x1122334455667788ULL;
    m.tree_id = 0x00A1;
    for (int i = 0; i < 16; i++)
        m.file_id[i] = (uint8_t)(0xE0 + i);
    m.file_size = 4096;
    m.auth_status = Smb2Status::SMB2_STATUS_SUCCESS;
    m.tc_status = Smb2Status::SMB2_STATUS_SUCCESS;
    m.create_status = Smb2Status::SMB2_STATUS_SUCCESS;
    m.ss1_status = Smb2Status::SMB2_STATUS_MORE_PROCESSING_REQUIRED;
    return m;
}

static SmbConfig make_cfg()
{
    SmbConfig c;
    memset(&c, 0, sizeof(c));
    c.user = "operator";
    c.pass = "secretpassword";
    c.domain = "SHOP";
    c.workstation = "ESP32";
    c.share = "\\\\nc01\\programs";
    c.path = "A.NC";
    c.desired_access = Smb2Access::SMB2_FILE_GENERIC_READ;
    c.disposition = Smb2Disposition::SMB2_FILE_OPEN;
    return c;
}

void test_open_close_success()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    memset(&h, 0, sizeof(h));

    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_open(&cfg, &h, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_HEX64(m.session_id, h.session_id);
    TEST_ASSERT_EQUAL_HEX32(m.tree_id, h.tree_id);
    TEST_ASSERT_EQUAL_MEMORY(m.file_id, h.file_id, 16);
    TEST_ASSERT_EQUAL_HEX64(4096, h.file_size);
    TEST_ASSERT_EQUAL_UINT64(5, h.next_message_id);
    // NEGOTIATE + 2x SESSION_SETUP + TREE_CONNECT + CREATE = 5 requests
    TEST_ASSERT_EQUAL_INT(5, m.req_count);

    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_close(&h, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT64(6, h.next_message_id);
    TEST_ASSERT_EQUAL_INT(6, m.req_count);
}

void test_auth_failure()
{
    Mock m = make_mock();
    m.auth_status = 0xC000006D; // STATUS_LOGON_FAILURE
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_AUTH, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_bad_share()
{
    Mock m = make_mock();
    m.tc_status = 0xC00000CC; // STATUS_BAD_NETWORK_NAME
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_create_not_found()
{
    Mock m = make_mock();
    m.create_status = 0xC0000034; // STATUS_OBJECT_NAME_NOT_FOUND
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_io_error()
{
    Mock m = make_mock();
    m.cut_after_negotiate = true; // server stops responding after NEGOTIATE
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_arg_validation()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    cfg.user = nullptr;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG, smb_open(&cfg, &h, mock_send, mock_recv, &m));
    cfg = make_cfg();
    cfg.path = nullptr;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

static SmbResult open_ok(Mock *m, SmbConfig *cfg, SmbHandle *h)
{
    memset(h, 0, sizeof(*h));
    return smb_open(cfg, h, mock_send, mock_recv, m);
}

// Read a 2000-byte file: spans multiple READ round trips (chunk_max < 2000).
void test_read_file()
{
    Mock m = make_mock();
    for (int i = 0; i < 2000; i++)
        m.file_data[i] = (uint8_t)(i * 31 + 7);
    m.file_data_len = 2000;
    m.file_size = 2000;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t buf[2048];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_read(&h, 0, buf, 2000, &got, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT32(2000, got);
    TEST_ASSERT_EQUAL_MEMORY(m.file_data, buf, 2000);
}

// Read with a buffer larger than the file: stops at EOF (short read / STATUS_END_OF_FILE).
void test_read_past_eof()
{
    Mock m = make_mock();
    for (int i = 0; i < 100; i++)
        m.file_data[i] = (uint8_t)i;
    m.file_data_len = 100;
    m.file_size = 100;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t buf[512];
    size_t got = 999;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_read(&h, 0, buf, sizeof(buf), &got, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT32(100, got);
    TEST_ASSERT_EQUAL_MEMORY(m.file_data, buf, 100);
}

// Write a 2000-byte file: spans multiple WRITE round trips; the cached file_size grows.
void test_write_file()
{
    Mock m = make_mock();
    m.file_data_len = 0;
    m.file_size = 0;
    SmbConfig cfg = make_cfg();
    cfg.desired_access = Smb2Access::SMB2_FILE_GENERIC_WRITE;
    cfg.disposition = Smb2Disposition::SMB2_FILE_OVERWRITE_IF;
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t data[2000];
    for (int i = 0; i < 2000; i++)
        data[i] = (uint8_t)(i * 13 + 3);
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_write(&h, 0, data, sizeof(data), &wrote, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT32(2000, wrote);
    TEST_ASSERT_EQUAL_size_t(2000, m.file_data_len);
    TEST_ASSERT_EQUAL_MEMORY(data, m.file_data, 2000);
    TEST_ASSERT_EQUAL_HEX64(2000, h.file_size);
}

// Write then read back the same bytes through the mock (a byte-exact round trip).
void test_write_then_read_roundtrip()
{
    Mock m = make_mock();
    m.file_data_len = 0;
    m.file_size = 0;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t data[1500];
    for (int i = 0; i < 1500; i++)
        data[i] = (uint8_t)(i ^ 0x5A);
    size_t wrote = 0, got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_write(&h, 0, data, sizeof(data), &wrote, mock_send, mock_recv, &m));
    uint8_t back[1500];
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_read(&h, 0, back, sizeof(back), &got, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT32(1500, got);
    TEST_ASSERT_EQUAL_MEMORY(data, back, 1500);
}

// ---- smb_open handshake error / edge paths (the negative sides the round-trip test skips) ----

// NEGOTIATE reply is malformed (bad StructureSize) -> protocol error out of smb_open.
void test_negotiate_malformed()
{
    Mock m = make_mock();
    m.fault_at_req = 1;
    m.fault_kind = FAULT_BAD_BODY;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// The peer closes before answering NEGOTIATE -> IO error.
void test_negotiate_dropped()
{
    Mock m = make_mock();
    m.fault_at_req = 1;
    m.fault_kind = FAULT_DROP;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 1 header is unparseable -> auth error (the parse side of the guard).
void test_session1_bad_header()
{
    Mock m = make_mock();
    m.fault_at_req = 2;
    m.fault_kind = FAULT_BAD_HEADER;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_AUTH, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 1 returns SUCCESS instead of MORE_PROCESSING_REQUIRED -> auth error (the
// status side of the guard).
void test_session1_wrong_status()
{
    Mock m = make_mock();
    m.ss1_status = Smb2Status::SMB2_STATUS_SUCCESS;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_AUTH, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 1 body is unparseable (bad StructureSize) -> protocol error (the parse side
// of the sec-buf guard).
void test_session1_bad_body()
{
    Mock m = make_mock();
    m.fault_at_req = 2;
    m.fault_kind = FAULT_BAD_BODY;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 1 has no security buffer -> protocol error (the !sec_buf side).
void test_session1_no_secbuf()
{
    Mock m = make_mock();
    m.ss1_secbuf_mode = SSBUF_EMPTY;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 1 security buffer is not a SPNEGO NegTokenResp -> protocol error.
void test_session1_bad_spnego()
{
    Mock m = make_mock();
    m.ss1_secbuf_mode = SSBUF_RAW_JUNK;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SPNEGO unwraps but the inner token is not an NTLMSSP CHALLENGE -> protocol error.
void test_session1_bad_ntlmssp()
{
    Mock m = make_mock();
    m.ss1_secbuf_mode = SSBUF_SPNEGO_JUNK;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// The peer closes before answering SESSION_SETUP round 2 -> IO error.
void test_session2_dropped()
{
    Mock m = make_mock();
    m.fault_at_req = 3;
    m.fault_kind = FAULT_DROP;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// SESSION_SETUP round 2 header is unparseable -> protocol error.
void test_session2_bad_header()
{
    Mock m = make_mock();
    m.fault_at_req = 3;
    m.fault_kind = FAULT_BAD_HEADER;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// The peer closes before answering TREE_CONNECT -> IO error.
void test_tree_dropped()
{
    Mock m = make_mock();
    m.fault_at_req = 4;
    m.fault_kind = FAULT_DROP;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// TREE_CONNECT header is SUCCESS but the body is unparseable -> protocol error.
void test_tree_bad_body()
{
    Mock m = make_mock();
    m.fault_at_req = 4;
    m.fault_kind = FAULT_BAD_BODY;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// The peer closes before answering CREATE -> IO error.
void test_create_dropped()
{
    Mock m = make_mock();
    m.fault_at_req = 5;
    m.fault_kind = FAULT_DROP;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// CREATE header is SUCCESS but the body is unparseable -> protocol error.
void test_create_bad_body()
{
    Mock m = make_mock();
    m.fault_at_req = 5;
    m.fault_kind = FAULT_BAD_BODY;
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A share path longer than the UTF-16LE scratch overflows in TREE_CONNECT -> overflow error.
void test_long_share_overflow()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    char share[300];
    memset(share, 'S', sizeof(share) - 1); // 299 chars -> 598 bytes UTF-16LE, over the 512-byte scratch
    share[sizeof(share) - 1] = 0;
    cfg.share = share;
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A file path longer than the UTF-16LE scratch overflows in CREATE -> overflow error.
void test_long_path_overflow()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    char path[300];
    memset(path, 'P', sizeof(path) - 1);
    path[sizeof(path) - 1] = 0;
    cfg.path = path;
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A user longer than the NTOWFv2 scratch overflows during SESSION_SETUP -> overflow error.
void test_long_user_overflow()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    char user[300];
    memset(user, 'u', sizeof(user) - 1); // 299 chars -> ntlm_ntowfv2 fails closed
    user[sizeof(user) - 1] = 0;
    cfg.user = user;
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A CHALLENGE target-info so large the NTLMv2 response overflows nt_resp -> overflow error.
void test_challenge_ti_ntlmv2_overflow()
{
    Mock m = make_mock();
    uint8_t ti[500];
    memset(ti, 0, sizeof(ti)); // starts with an EOL pair -> find_av_timestamp returns a zero time
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti); // 48 + 500 = 548 > the 512-byte nt_resp buffer
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A smaller target-info: the NTLMv2 response fits but the NTLMSSP AUTHENTICATE overflows -> overflow.
void test_challenge_ti_authenticate_overflow()
{
    Mock m = make_mock();
    uint8_t ti[400];
    memset(ti, 0, sizeof(ti));
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti); // nt_resp = 448 (fits 512); AUTHENTICATE = 64+448+identity > 512
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// Smaller still: AUTHENTICATE fits but its SPNEGO wrap overflows sp2 -> overflow error.
void test_challenge_ti_spnego_overflow()
{
    Mock m = make_mock();
    uint8_t ti[360];
    memset(ti, 0, sizeof(ti));
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti); // AUTHENTICATE = 506 (fits 512); its SPNEGO wrap = 522 > 512
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// Target info that is only an EOL: find_av_timestamp breaks with a zero time; handshake still OK.
void test_av_eol_only()
{
    Mock m = make_mock();
    const uint8_t ti[4] = {0x00, 0x00, 0x00, 0x00}; // MsvAvEOL
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti);
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// Target info that skips a non-timestamp pair and a wrong-length AvId 7 before the real 8-byte
// timestamp: exercises find_av_timestamp's skip and length-mismatch branches.
void test_av_skip_then_find()
{
    Mock m = make_mock();
    const uint8_t ti[] = {
        0x02, 0x00, 0x04, 0x00, 0xDE, 0xAD, 0xBE, 0xEF,                         // AvId 2, len 4 -> skipped
        0x07, 0x00, 0x04, 0x00, 0x11, 0x22, 0x33, 0x44,                         // AvId 7 but len 4 -> skipped
        0x07, 0x00, 0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // AvId 7 len 8 -> time
        0x00, 0x00, 0x00, 0x00,                                                 // MsvAvEOL
    };
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti);
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// A truncated AvId 7 whose 8-byte value runs past the blob end: the in-bounds check fails and
// find_av_timestamp falls through to a zero time; the handshake still succeeds.
void test_av_truncated_timestamp()
{
    Mock m = make_mock();
    const uint8_t ti[4] = {0x07, 0x00, 0x08, 0x00}; // AvId 7 len 8 header, but no value bytes
    m.chal_ti = ti;
    m.chal_ti_len = sizeof(ti);
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

// ---- a canned single-response seam for smb_read / smb_write / smb_close error paths, driven on a
// hand-built handle (no open handshake needed). ----
struct Canned
{
    uint8_t resp[512];
    size_t resp_len;
    size_t pos;
    bool short_send; // send returns a short count (1) instead of the true length
};

static int canned_send(void *c, const uint8_t *d, size_t n)
{
    Canned *cn = (Canned *)c;
    (void)d;
    return cn->short_send ? 1 : (int)n;
}

static int canned_recv(void *c, uint8_t *buf, size_t cap)
{
    Canned *cn = (Canned *)c;
    if (cn->pos >= cn->resp_len)
        return 0; // exhausted -> peer closed
    size_t avail = cn->resp_len - cn->pos;
    size_t take = avail < cap ? avail : cap;
    memcpy(buf, cn->resp + cn->pos, take);
    cn->pos += take;
    return (int)take;
}

// Build a 64-byte SMB2 response header (command + status) into msg; returns the body pointer.
static uint8_t *resp_hdr(uint8_t *msg, Smb2Command cmd, uint32_t status)
{
    smb2_build_header(msg, 64, cmd, 1, 5, 0x00A1, 0x1122334455667788ULL);
    w32(msg + 8, status);
    msg[16] |= 0x01; // server-to-redir
    return msg + 64;
}

static void canned_frame(Canned *cn, const uint8_t *msg, size_t mlen)
{
    cn->resp_len = smb2_transport_frame(cn->resp, sizeof(cn->resp), msg, mlen);
    cn->pos = 0;
}

static SmbHandle make_handle()
{
    SmbHandle h;
    memset(&h, 0, sizeof(h));
    h.session_id = 0x1122334455667788ULL;
    h.tree_id = 0x00A1;
    for (int i = 0; i < 16; i++)
        h.file_id[i] = (uint8_t)(0xE0 + i);
    h.file_size = 4096;
    h.next_message_id = 5;
    return h;
}

void test_read_arg()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_read(&h, 0, nullptr, sizeof(buf), &got, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_read(&h, 0, buf, sizeof(buf), nullptr, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_read(nullptr, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// send returns a short count -> the round trip reports IO (smb_round_trip's send-fail path).
void test_read_send_io()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.short_send = true;
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// The peer sends nothing back -> IO error.
void test_read_recv_io()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn)); // resp_len 0 -> recv returns 0
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// The READ reply header is unparseable -> protocol error.
void test_read_bad_header()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_READ, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    b[2] = 80;
    w32(b + 4, 0);
    msg[0] = 0x00; // corrupt ProtocolId
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// A READ status that is neither SUCCESS nor END_OF_FILE -> protocol error.
void test_read_status_error()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_READ, 0xC0000022); // STATUS_ACCESS_DENIED
    w16(b + 0, 17);
    b[2] = 80;
    w32(b + 4, 0);
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// A READ reply whose body StructureSize is wrong -> protocol error.
void test_read_bad_body()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_READ, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 99); // not 17
    b[2] = 80;
    w32(b + 4, 0);
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// A READ reply that returns more data than requested -> protocol error (data_len > want).
void test_read_data_too_long()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[256] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_READ, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    b[2] = 80;       // DataOffset
    w32(b + 4, 100); // DataLength 100, but only 16 requested
    memset(msg + 80, 0xAB, 100);
    canned_frame(&cn, msg, 180);
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
}

// A READ reply with zero data stops the loop cleanly at 0 bytes.
void test_read_zero_data()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_READ, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    b[2] = 80;
    w32(b + 4, 0); // DataLength 0
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t buf[16];
    size_t got = 999;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_OK, smb_read(&h, 0, buf, sizeof(buf), &got, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_UINT32(0, got);
}

void test_write_arg()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_write(&h, 0, nullptr, sizeof(data), &wrote, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_write(&h, 0, data, sizeof(data), nullptr, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG,
                          smb_write(nullptr, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// send returns a short count -> IO error (smb_write uses send_msg directly).
void test_write_send_io()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.short_send = true;
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// The peer sends nothing back -> IO error.
void test_write_recv_io()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A reply whose transport length prefix exceeds the work buffer -> overflow error.
void test_write_recv_overflow()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.resp[0] = 0x00;
    cn.resp[1] = 0x00;
    cn.resp[2] = 0x20;
    cn.resp[3] = 0x00; // length 0x2000 = 8192 > DETWS_SMB_BUF (1024)
    cn.resp_len = 4;
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A WRITE reply header is unparseable -> protocol error.
void test_write_bad_header()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_WRITE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    w32(b + 4, 16); // Count
    msg[0] = 0x00;  // corrupt ProtocolId
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A WRITE status that is not SUCCESS -> protocol error.
void test_write_status_error()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_WRITE, 0xC0000022);
    w16(b + 0, 17);
    w32(b + 4, 16);
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A WRITE reply body is unparseable (bad StructureSize) -> protocol error.
void test_write_bad_body()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_WRITE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 99); // not 17
    w32(b + 4, 16);
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A WRITE that acknowledges zero bytes -> protocol error (no forward progress).
void test_write_zero_count()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_WRITE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    w32(b + 4, 0); // Count 0
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

// A WRITE that claims more bytes than were sent -> protocol error (count > want).
void test_write_count_too_big()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_WRITE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 17);
    w32(b + 4, 999); // Count > 16 requested
    canned_frame(&cn, msg, 80);
    SmbHandle h = make_handle();
    uint8_t data[16] = {0};
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL,
                          smb_write(&h, 0, data, sizeof(data), &wrote, canned_send, canned_recv, &cn));
}

void test_close_arg()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG, smb_close(nullptr, canned_send, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG, smb_close(&h, nullptr, canned_recv, &cn));
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_ARG, smb_close(&h, canned_send, nullptr, &cn));
}

// send returns a short count -> IO error.
void test_close_send_io()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.short_send = true;
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_close(&h, canned_send, canned_recv, &cn));
}

// A reply whose transport length prefix exceeds the 128-byte close buffer -> overflow error.
void test_close_recv_overflow()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.resp[0] = 0x00;
    cn.resp[1] = 0x00;
    cn.resp[2] = 0x00;
    cn.resp[3] = 0xC8; // length 200 > 128
    cn.resp_len = 4;
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_OVERFLOW, smb_close(&h, canned_send, canned_recv, &cn));
}

// A reply with a zero-length transport prefix -> IO error.
void test_close_recv_zero_len()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.resp[0] = 0x00; // length 0
    cn.resp[1] = 0x00;
    cn.resp[2] = 0x00;
    cn.resp[3] = 0x00;
    cn.resp_len = 4;
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_close(&h, canned_send, canned_recv, &cn));
}

// The transport prefix promises more bytes than the peer delivers -> IO error (truncated body).
void test_close_recv_trunc_body()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    cn.resp[0] = 0x00;
    cn.resp[1] = 0x00;
    cn.resp[2] = 0x00;
    cn.resp[3] = 0x64;    // length 100
    cn.resp_len = 4 + 40; // only 40 of the 100 body bytes follow
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_IO, smb_close(&h, canned_send, canned_recv, &cn));
}

// The CLOSE reply header is unparseable -> protocol error.
void test_close_bad_header()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_CLOSE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 60);
    msg[0] = 0x00; // corrupt ProtocolId
    canned_frame(&cn, msg, 124);
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_close(&h, canned_send, canned_recv, &cn));
}

// A CLOSE status that is not SUCCESS -> protocol error.
void test_close_status_error()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_CLOSE, 0xC0000022);
    w16(b + 0, 60);
    canned_frame(&cn, msg, 124);
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_close(&h, canned_send, canned_recv, &cn));
}

// A CLOSE reply body is unparseable (bad StructureSize) -> protocol error.
void test_close_bad_body()
{
    Canned cn;
    memset(&cn, 0, sizeof(cn));
    uint8_t msg[128] = {0};
    uint8_t *b = resp_hdr(msg, Smb2Command::SMB2_CLOSE, Smb2Status::SMB2_STATUS_SUCCESS);
    w16(b + 0, 99); // not 60
    canned_frame(&cn, msg, 124);
    SmbHandle h = make_handle();
    TEST_ASSERT_EQUAL_INT(SmbResult::SMB_ERR_PROTOCOL, smb_close(&h, canned_send, canned_recv, &cn));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_close_success);
    RUN_TEST(test_auth_failure);
    RUN_TEST(test_bad_share);
    RUN_TEST(test_create_not_found);
    RUN_TEST(test_io_error);
    RUN_TEST(test_arg_validation);
    RUN_TEST(test_read_file);
    RUN_TEST(test_read_past_eof);
    RUN_TEST(test_write_file);
    RUN_TEST(test_write_then_read_roundtrip);
    RUN_TEST(test_negotiate_malformed);
    RUN_TEST(test_negotiate_dropped);
    RUN_TEST(test_session1_bad_header);
    RUN_TEST(test_session1_wrong_status);
    RUN_TEST(test_session1_bad_body);
    RUN_TEST(test_session1_no_secbuf);
    RUN_TEST(test_session1_bad_spnego);
    RUN_TEST(test_session1_bad_ntlmssp);
    RUN_TEST(test_session2_dropped);
    RUN_TEST(test_session2_bad_header);
    RUN_TEST(test_tree_dropped);
    RUN_TEST(test_tree_bad_body);
    RUN_TEST(test_create_dropped);
    RUN_TEST(test_create_bad_body);
    RUN_TEST(test_long_share_overflow);
    RUN_TEST(test_long_path_overflow);
    RUN_TEST(test_long_user_overflow);
    RUN_TEST(test_challenge_ti_ntlmv2_overflow);
    RUN_TEST(test_challenge_ti_authenticate_overflow);
    RUN_TEST(test_challenge_ti_spnego_overflow);
    RUN_TEST(test_av_eol_only);
    RUN_TEST(test_av_skip_then_find);
    RUN_TEST(test_av_truncated_timestamp);
    RUN_TEST(test_read_arg);
    RUN_TEST(test_read_send_io);
    RUN_TEST(test_read_recv_io);
    RUN_TEST(test_read_bad_header);
    RUN_TEST(test_read_status_error);
    RUN_TEST(test_read_bad_body);
    RUN_TEST(test_read_data_too_long);
    RUN_TEST(test_read_zero_data);
    RUN_TEST(test_write_arg);
    RUN_TEST(test_write_send_io);
    RUN_TEST(test_write_recv_io);
    RUN_TEST(test_write_recv_overflow);
    RUN_TEST(test_write_bad_header);
    RUN_TEST(test_write_status_error);
    RUN_TEST(test_write_bad_body);
    RUN_TEST(test_write_zero_count);
    RUN_TEST(test_write_count_too_big);
    RUN_TEST(test_close_arg);
    RUN_TEST(test_close_send_io);
    RUN_TEST(test_close_recv_overflow);
    RUN_TEST(test_close_recv_zero_len);
    RUN_TEST(test_close_recv_trunc_body);
    RUN_TEST(test_close_bad_header);
    RUN_TEST(test_close_status_error);
    RUN_TEST(test_close_bad_body);
    return UNITY_END();
}
