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
    w32(m + 20, NTLMSSP_NEGOTIATE_UNICODE | NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_NEGOTIATE_TARGET_INFO);
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
    case SMB2_NEGOTIATE:
        smb2_build_header(resp, sizeof(resp), SMB2_NEGOTIATE, 1, h.message_id, 0, 0);
        w16(b + 0, 65);                // StructureSize
        w16(b + 4, SMB2_DIALECT_0210); // DialectRevision
        rlen = 128;                    // header + 64-byte fixed body, empty security buffer
        break;
    case SMB2_SESSION_SETUP: {
        smb2_build_header(resp, sizeof(resp), SMB2_SESSION_SETUP, 1, h.message_id, 0, m->session_id);
        w16(b + 0, 9); // StructureSize
        if (m->ss_round++ == 0)
        {
            uint8_t chal[64], sctok[128];
            const uint8_t sc[8] = {1, 2, 3, 4, 5, 6, 7, 8};
            size_t chal_n = ntlmssp_challenge(chal, sc);
            size_t sc_n = spnego_wrap_authenticate(chal, chal_n, sctok, sizeof(sctok)); // NegTokenResp shape
            w32(resp + 8, SMB2_STATUS_MORE_PROCESSING_REQUIRED);
            w16(b + 4, 72); // SecurityBufferOffset
            w16(b + 6, (uint16_t)sc_n);
            memcpy(resp + 72, sctok, sc_n);
            rlen = 72 + sc_n;
        }
        else
        {
            w32(resp + 8, m->auth_status);
            rlen = 72; // header + 8-byte body, empty buffer
        }
        break;
    }
    case SMB2_TREE_CONNECT:
        smb2_build_header(resp, sizeof(resp), SMB2_TREE_CONNECT, 1, h.message_id, m->tree_id, m->session_id);
        w32(resp + 8, m->tc_status);
        w16(b + 0, 16); // StructureSize
        b[2] = SMB2_SHARE_TYPE_DISK;
        rlen = 64 + 16;
        break;
    case SMB2_CREATE:
        smb2_build_header(resp, sizeof(resp), SMB2_CREATE, 1, h.message_id, m->tree_id, m->session_id);
        w32(resp + 8, m->create_status);
        w16(b + 0, 89); // StructureSize
        w32(b + 4, 1);  // CreateAction = FILE_OPENED
        w64(b + 48, m->file_size);
        memcpy(b + 64, m->file_id, 16);
        rlen = 64 + 88;
        break;
    case SMB2_READ: {
        const uint8_t *rq = msg + 64; // READ request body
        uint32_t length = rd32(rq + 4);
        uint64_t off = rd64(rq + 8);
        smb2_build_header(resp, sizeof(resp), SMB2_READ, 1, h.message_id, m->tree_id, m->session_id);
        if (off >= m->file_data_len)
        {
            w32(resp + 8, SMB2_STATUS_END_OF_FILE);
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
    case SMB2_WRITE: {
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
        smb2_build_header(resp, sizeof(resp), SMB2_WRITE, 1, h.message_id, m->tree_id, m->session_id);
        w16(b + 0, 17);     // StructureSize
        w32(b + 4, length); // Count
        rlen = 64 + 16;
        break;
    }
    case SMB2_CLOSE:
        smb2_build_header(resp, sizeof(resp), SMB2_CLOSE, 1, h.message_id, m->tree_id, m->session_id);
        w16(b + 0, 60); // StructureSize
        rlen = 64 + 60;
        break;
    default:
        return -1;
    }
    resp[16] |= 0x01; // SMB2_FLAGS_SERVER_TO_REDIR
    if (!(m->cut_after_negotiate && h.command != SMB2_NEGOTIATE))
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
    m.auth_status = SMB2_STATUS_SUCCESS;
    m.tc_status = SMB2_STATUS_SUCCESS;
    m.create_status = SMB2_STATUS_SUCCESS;
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
    c.desired_access = SMB2_FILE_GENERIC_READ;
    c.disposition = SMB2_FILE_OPEN;
    return c;
}

void test_open_close_success()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    memset(&h, 0, sizeof(h));

    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_open(&cfg, &h, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_HEX64(m.session_id, h.session_id);
    TEST_ASSERT_EQUAL_HEX32(m.tree_id, h.tree_id);
    TEST_ASSERT_EQUAL_MEMORY(m.file_id, h.file_id, 16);
    TEST_ASSERT_EQUAL_HEX64(4096, h.file_size);
    TEST_ASSERT_EQUAL_UINT64(5, h.next_message_id);
    // NEGOTIATE + 2x SESSION_SETUP + TREE_CONNECT + CREATE = 5 requests
    TEST_ASSERT_EQUAL_INT(5, m.req_count);

    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_close(&h, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT64(6, h.next_message_id);
    TEST_ASSERT_EQUAL_INT(6, m.req_count);
}

void test_auth_failure()
{
    Mock m = make_mock();
    m.auth_status = 0xC000006D; // STATUS_LOGON_FAILURE
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_AUTH, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_bad_share()
{
    Mock m = make_mock();
    m.tc_status = 0xC00000CC; // STATUS_BAD_NETWORK_NAME
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_create_not_found()
{
    Mock m = make_mock();
    m.create_status = 0xC0000034; // STATUS_OBJECT_NAME_NOT_FOUND
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_PROTOCOL, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_io_error()
{
    Mock m = make_mock();
    m.cut_after_negotiate = true; // server stops responding after NEGOTIATE
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_IO, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

void test_arg_validation()
{
    Mock m = make_mock();
    SmbConfig cfg = make_cfg();
    SmbHandle h;
    cfg.user = nullptr;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_ARG, smb_open(&cfg, &h, mock_send, mock_recv, &m));
    cfg = make_cfg();
    cfg.path = nullptr;
    TEST_ASSERT_EQUAL_INT(SMB_ERR_ARG, smb_open(&cfg, &h, mock_send, mock_recv, &m));
}

static int open_ok(Mock *m, SmbConfig *cfg, SmbHandle *h)
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
    TEST_ASSERT_EQUAL_INT(SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t buf[2048];
    size_t got = 0;
    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_read(&h, 0, buf, 2000, &got, mock_send, mock_recv, &m));
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
    TEST_ASSERT_EQUAL_INT(SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t buf[512];
    size_t got = 999;
    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_read(&h, 0, buf, sizeof(buf), &got, mock_send, mock_recv, &m));
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
    cfg.desired_access = SMB2_FILE_GENERIC_WRITE;
    cfg.disposition = SMB2_FILE_OVERWRITE_IF;
    SmbHandle h;
    TEST_ASSERT_EQUAL_INT(SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t data[2000];
    for (int i = 0; i < 2000; i++)
        data[i] = (uint8_t)(i * 13 + 3);
    size_t wrote = 0;
    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_write(&h, 0, data, sizeof(data), &wrote, mock_send, mock_recv, &m));
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
    TEST_ASSERT_EQUAL_INT(SMB_OK, open_ok(&m, &cfg, &h));

    uint8_t data[1500];
    for (int i = 0; i < 1500; i++)
        data[i] = (uint8_t)(i ^ 0x5A);
    size_t wrote = 0, got = 0;
    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_write(&h, 0, data, sizeof(data), &wrote, mock_send, mock_recv, &m));
    uint8_t back[1500];
    TEST_ASSERT_EQUAL_INT(SMB_OK, smb_read(&h, 0, back, sizeof(back), &got, mock_send, mock_recv, &m));
    TEST_ASSERT_EQUAL_UINT32(1500, got);
    TEST_ASSERT_EQUAL_MEMORY(data, back, 1500);
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
    return UNITY_END();
}
