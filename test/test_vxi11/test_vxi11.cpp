// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the VXI-11 codec over ONC RPC / XDR (services/vxi11): the record-marking header,
// the CALL builders (create_link byte-exact against the spec worked example), and the REPLY parsers
// (driven by hand-built reply buffers). Pure host tests.

#include "services/vxi11/vxi11.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Write a big-endian 32-bit word at offset o; return o+4.
static size_t put32(uint8_t *b, size_t o, uint32_t v)
{
    b[o] = (uint8_t)(v >> 24);
    b[o + 1] = (uint8_t)(v >> 16);
    b[o + 2] = (uint8_t)(v >> 8);
    b[o + 3] = (uint8_t)v;
    return o + 4;
}

// Build an accepted-reply RPC header (no record mark); return the offset where results begin.
static size_t reply_header(uint8_t *b, uint32_t xid, uint32_t accept_stat)
{
    size_t o = 0;
    o = put32(b, o, xid);
    o = put32(b, o, 1); // REPLY
    o = put32(b, o, 0); // MSG_ACCEPTED
    o = put32(b, o, 0); // verf.flavor = AUTH_NONE
    o = put32(b, o, 0); // verf.length = 0
    o = put32(b, o, accept_stat);
    return o;
}

// ── record marking ──────────────────────────────────────────────────────────────────────────────

void test_record_mark()
{
    uint8_t buf[4];
    TEST_ASSERT_EQUAL_size_t(4, dws_rpc_record_mark(buf, sizeof(buf), 64));
    const uint8_t expected[] = {0x80, 0x00, 0x00, 0x40}; // last-frag flag + length 64
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, 4);

    bool last = false;
    uint32_t frag = 0;
    TEST_ASSERT_TRUE(dws_rpc_parse_record_mark(buf, 4, &last, &frag));
    TEST_ASSERT_TRUE(last);
    TEST_ASSERT_EQUAL_UINT32(64, frag);

    // a non-final fragment
    const uint8_t partial[] = {0x00, 0x00, 0x00, 0x10};
    TEST_ASSERT_TRUE(dws_rpc_parse_record_mark(partial, 4, &last, &frag));
    TEST_ASSERT_FALSE(last);
    TEST_ASSERT_EQUAL_UINT32(16, frag);

    // a length that does not fit 31 bits is rejected
    TEST_ASSERT_EQUAL_size_t(0, dws_rpc_record_mark(buf, sizeof(buf), 0x80000000u));
}

// ── create_link CALL (byte-exact VXI-11 spec worked example) ────────────────────────────────────

void test_create_link_vector()
{
    uint8_t buf[128];
    size_t n = dws_vxi11_build_create_link(buf, sizeof(buf), 1, 0x12345678, false, 0, "inst0");
    const uint8_t expected[] = {
        0x80, 0x00, 0x00, 0x40, // record mark: last, 64
        0x00, 0x00, 0x00, 0x01, // xid = 1
        0x00, 0x00, 0x00, 0x00, // CALL
        0x00, 0x00, 0x00, 0x02, // rpcvers = 2
        0x00, 0x06, 0x07, 0xAF, // prog = DEVICE_CORE
        0x00, 0x00, 0x00, 0x01, // vers = 1
        0x00, 0x00, 0x00, 0x0A, // proc = create_link (10)
        0x00, 0x00, 0x00, 0x00, // cred.flavor = AUTH_NONE
        0x00, 0x00, 0x00, 0x00, // cred.length = 0
        0x00, 0x00, 0x00, 0x00, // verf.flavor = AUTH_NONE
        0x00, 0x00, 0x00, 0x00, // verf.length = 0
        0x12, 0x34, 0x56, 0x78, // clientId
        0x00, 0x00, 0x00, 0x00, // lockDevice = FALSE
        0x00, 0x00, 0x00, 0x00, // lock_timeout = 0
        0x00, 0x00, 0x00, 0x05, // device.length = 5
        'i',  'n',  's',  't',  // "inst"
        '0',  0x00, 0x00, 0x00, // "0" + 3 pad
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expected), n);
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, sizeof(expected));
}

void test_create_link_reply()
{
    uint8_t b[64];
    size_t o = reply_header(b, 1, 0 /*SUCCESS*/);
    o = put32(b, o, 0);      // error = no error
    o = put32(b, o, 0x0100); // lid
    o = put32(b, o, 0x1234); // abortPort
    o = put32(b, o, 0x4000); // maxRecvSize
    Vxi11CreateLinkResp resp;
    TEST_ASSERT_TRUE(dws_vxi11_parse_create_link_resp(b, o, &resp));
    TEST_ASSERT_EQUAL_INT32(0, resp.error);
    TEST_ASSERT_EQUAL_INT32(0x0100, resp.lid);
    TEST_ASSERT_EQUAL_UINT32(0x1234, resp.abort_port);
    TEST_ASSERT_EQUAL_UINT32(0x4000, resp.max_recv_size);
}

// ── portmapper GETPORT ──────────────────────────────────────────────────────────────────────────

void test_getport()
{
    uint8_t buf[64];
    size_t n =
        dws_vxi11_build_getport(buf, sizeof(buf), 7, DWS_VXI11_CORE_PROG, DWS_VXI11_CORE_VERS, DWS_RPC_PROTO_TCP);
    // spot-check the call header: prog=portmapper, proc=GETPORT, and the mapping.prog word
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    // header prog is at record-mark(4) + 3 words in (xid, CALL, rpcvers) = offset 16
    const uint8_t pmap_prog[] = {0x00, 0x01, 0x86, 0xA0}; // 100000
    TEST_ASSERT_EQUAL_MEMORY(pmap_prog, buf + 16, 4);

    uint8_t b[64];
    size_t o = reply_header(b, 7, 0);
    o = put32(b, o, 1280); // the DEVICE_CORE port
    uint32_t port = 0;
    TEST_ASSERT_TRUE(dws_vxi11_parse_getport_resp(b, o, &port));
    TEST_ASSERT_EQUAL_UINT32(1280, port);
}

// ── device_write ────────────────────────────────────────────────────────────────────────────────

void test_device_write()
{
    uint8_t buf[128];
    const uint8_t scpi[] = {'*', 'I', 'D', 'N', '?', '\n'}; // 6 bytes -> 2 pad
    size_t n = dws_vxi11_build_device_write(buf, sizeof(buf), 2, 0x0100, 10000, 0, DWS_VXI11_FLAG_END, scpi, 6);
    // header(40) + record-mark(4) + lid,io,lock,flags (16) + opaque(len 4 + 6 data + 2 pad = 12) = 72
    TEST_ASSERT_EQUAL_size_t(72, n);
    // the flags word sits after record-mark(4)+header(40)+lid(4)+io(4)+lock(4) = offset 56
    const uint8_t end_flag[] = {0x00, 0x00, 0x00, 0x08}; // DWS_VXI11_FLAG_END
    TEST_ASSERT_EQUAL_MEMORY(end_flag, buf + 56, 4);
    // the opaque length + data follow at offset 60
    const uint8_t opaque[] = {0x00, 0x00, 0x00, 0x06, '*', 'I', 'D', 'N', '?', '\n', 0x00, 0x00};
    TEST_ASSERT_EQUAL_MEMORY(opaque, buf + 60, sizeof(opaque));

    uint8_t b[48];
    size_t o = reply_header(b, 2, 0);
    o = put32(b, o, 0); // error
    o = put32(b, o, 6); // size written
    Vxi11WriteResp wr;
    TEST_ASSERT_TRUE(dws_vxi11_parse_write_resp(b, o, &wr));
    TEST_ASSERT_EQUAL_INT32(0, wr.error);
    TEST_ASSERT_EQUAL_UINT32(6, wr.size);
    // null data with a non-zero length fails closed
    TEST_ASSERT_EQUAL_size_t(0, dws_vxi11_build_device_write(buf, sizeof(buf), 2, 1, 0, 0, 0, nullptr, 4));
}

// ── device_read ─────────────────────────────────────────────────────────────────────────────────

void test_device_read()
{
    uint8_t buf[128];
    size_t n =
        dws_vxi11_build_device_read(buf, sizeof(buf), 3, 0x0100, 4096, 10000, 0, DWS_VXI11_FLAG_TERMCHRSET, '\n');
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    // a read reply carrying an identity string (11 bytes -> 1 pad)
    uint8_t b[64];
    const char *idn = "ACME,X,1,2\n";
    size_t o = reply_header(b, 3, 0);
    o = put32(b, o, 0);                     // error
    o = put32(b, o, DWS_VXI11_REASON_END);  // reason = END
    o = put32(b, o, (uint32_t)strlen(idn)); // opaque length = 11
    memcpy(b + o, idn, strlen(idn));
    o += strlen(idn);
    b[o++] = 0; // 1 pad byte to a 4-byte boundary
    Vxi11ReadResp rr;
    TEST_ASSERT_TRUE(dws_vxi11_parse_read_resp(b, o, &rr));
    TEST_ASSERT_EQUAL_INT32(0, rr.error);
    TEST_ASSERT_EQUAL_INT32(DWS_VXI11_REASON_END, rr.reason);
    TEST_ASSERT_EQUAL_size_t(11, rr.data_len);
    TEST_ASSERT_EQUAL_MEMORY(idn, rr.data, 11);
}

// ── device_readstb + destroy_link (Device_Error) ────────────────────────────────────────────────

void test_readstb_and_destroy()
{
    uint8_t buf[128];
    TEST_ASSERT_GREATER_THAN(0, (int)dws_vxi11_build_device_readstb(buf, sizeof(buf), 4, 0x0100, 0, 0, 10000));
    uint8_t b[48];
    size_t o = reply_header(b, 4, 0);
    o = put32(b, o, 0);    // error
    o = put32(b, o, 0x40); // stb (status byte; sent as a full word)
    Vxi11ReadStbResp sr;
    TEST_ASSERT_TRUE(dws_vxi11_parse_readstb_resp(b, o, &sr));
    TEST_ASSERT_EQUAL_INT32(0, sr.error);
    TEST_ASSERT_EQUAL_HEX8(0x40, sr.stb);

    TEST_ASSERT_GREATER_THAN(0, (int)dws_vxi11_build_destroy_link(buf, sizeof(buf), 5, 0x0100));
    o = reply_header(b, 5, 0);
    o = put32(b, o, 0); // Device_Error.error
    int32_t err = -1;
    TEST_ASSERT_TRUE(dws_vxi11_parse_error_resp(b, o, &err));
    TEST_ASSERT_EQUAL_INT32(0, err);
}

// ── reply rejection paths ───────────────────────────────────────────────────────────────────────

void test_reply_rejects()
{
    uint8_t b[64];
    uint32_t xid, astat;
    size_t off;
    // MSG_DENIED (reply_stat = 1)
    size_t o = 0;
    o = put32(b, o, 1);
    o = put32(b, o, 1); // REPLY
    o = put32(b, o, 1); // MSG_DENIED
    TEST_ASSERT_FALSE(dws_rpc_parse_reply(b, o, &xid, &astat, &off));

    // not a REPLY (mtype = CALL)
    o = 0;
    o = put32(b, o, 1);
    o = put32(b, o, 0); // CALL
    o = put32(b, o, 0);
    TEST_ASSERT_FALSE(dws_rpc_parse_reply(b, o, &xid, &astat, &off));

    // accepted but accept_stat != SUCCESS -> the per-proc parser refuses to read results
    o = reply_header(b, 1, 1 /*PROG_UNAVAIL*/);
    o = put32(b, o, 0);
    o = put32(b, o, 0);
    o = put32(b, o, 0);
    o = put32(b, o, 0);
    Vxi11CreateLinkResp resp;
    TEST_ASSERT_FALSE(dws_vxi11_parse_create_link_resp(b, o, &resp));

    // truncated reply header
    TEST_ASSERT_FALSE(dws_rpc_parse_reply(b, 8, &xid, &astat, &off));
}

void test_error_str()
{
    TEST_ASSERT_EQUAL_STRING("no error", dws_vxi11_error_str(0));
    TEST_ASSERT_EQUAL_STRING("invalid link identifier", dws_vxi11_error_str(4));
    TEST_ASSERT_EQUAL_STRING("device locked by another link", dws_vxi11_error_str(11));
    TEST_ASSERT_EQUAL_STRING("I/O timeout", dws_vxi11_error_str(15));
    TEST_ASSERT_EQUAL_STRING("unknown error", dws_vxi11_error_str(999));
}

void test_build_overflow()
{
    uint8_t small[16];
    TEST_ASSERT_EQUAL_size_t(0, dws_vxi11_build_create_link(small, sizeof(small), 1, 0, false, 0, "inst0"));
    TEST_ASSERT_EQUAL_size_t(0, dws_vxi11_build_create_link(nullptr, 128, 1, 0, false, 0, "inst0"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_record_mark);
    RUN_TEST(test_create_link_vector);
    RUN_TEST(test_create_link_reply);
    RUN_TEST(test_getport);
    RUN_TEST(test_device_write);
    RUN_TEST(test_device_read);
    RUN_TEST(test_readstb_and_destroy);
    RUN_TEST(test_reply_rejects);
    RUN_TEST(test_error_str);
    RUN_TEST(test_build_overflow);
    return UNITY_END();
}
