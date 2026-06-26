// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the CoAP server core (coap_server_process). Each test encodes a
// real RFC 7252 request datagram, runs it through the server, and decodes the
// response - no sockets, no heap.

#include "services/coap/coap.h"
#include <string.h>
#include <unity.h>

// ---------------------------------------------------------------------------
// Captured request state (what the handler saw on the last call)
// ---------------------------------------------------------------------------

static uint8_t g_method;
static char g_path[128];
static char g_query[128];
static uint16_t g_cf;
static uint8_t g_payload[128];
static size_t g_payload_len;
static bool g_called;

static void record(const CoapRequest *req)
{
    g_called = true;
    g_method = req->method;
    strncpy(g_path, req->path, sizeof(g_path) - 1);
    g_path[sizeof(g_path) - 1] = '\0';
    strncpy(g_query, req->query, sizeof(g_query) - 1);
    g_query[sizeof(g_query) - 1] = '\0';
    g_cf = req->content_format;
    g_payload_len = req->payload_len < sizeof(g_payload) ? req->payload_len : sizeof(g_payload);
    if (g_payload_len)
        memcpy(g_payload, req->payload, g_payload_len);
}

// A handler that records the request and replies per method.
static void h_resource(const CoapRequest *req, CoapResponse *resp)
{
    record(req);
    switch (req->method)
    {
    case COAP_GET:
        resp->code = COAP_RSP_CONTENT;
        memcpy(resp->payload, "hi", 2);
        resp->payload_len = 2;
        resp->content_format = COAP_CF_TEXT;
        break;
    case COAP_POST:
        resp->code = COAP_RSP_CREATED;
        resp->payload_len = 0;
        break;
    case COAP_PUT:
        resp->code = COAP_RSP_CHANGED;
        resp->payload_len = 0;
        break;
    case COAP_DELETE:
        resp->code = COAP_RSP_DELETED;
        resp->payload_len = 0;
        break;
    }
}

// Deterministic 150-byte representation for block-wise (Block2) tests: big[i] =
// 'A' + (i % 26). Larger than the test env's 64-byte max block (SZX_MAX=2).
static const size_t BIG_LEN = 150;
static uint8_t big_expected(size_t i)
{
    return (uint8_t)('A' + (int)(i % 26));
}
static void h_big(const CoapRequest *req, CoapResponse *resp)
{
    record(req);
    resp->code = COAP_RSP_CONTENT;
    resp->content_format = COAP_CF_TEXT;
    resp->payload_len = BIG_LEN;
    for (size_t i = 0; i < BIG_LEN; i++)
        resp->payload[i] = big_expected(i);
}

void setUp()
{
    g_called = false;
    g_method = 0;
    g_path[0] = '\0';
    g_query[0] = '\0';
    g_cf = COAP_CF_NONE;
    g_payload_len = 0;

    coap_server_init();
    coap_server_add_resource("/temp", COAP_ALLOW_GET | COAP_ALLOW_POST | COAP_ALLOW_PUT | COAP_ALLOW_DELETE,
                             h_resource);
    coap_server_add_resource("/ro", COAP_ALLOW_GET, h_resource);
    coap_server_add_resource("/a/b", COAP_ALLOW_GET, h_resource);
    coap_server_add_resource("/longresourcename12345", COAP_ALLOW_GET, h_resource); // >12 chars: extended opt length
    coap_server_add_resource("/", COAP_ALLOW_GET, h_resource);
    coap_server_add_resource("/big", COAP_ALLOW_GET | COAP_ALLOW_POST | COAP_ALLOW_PUT, h_big);
}

void tearDown()
{
}

// ---------------------------------------------------------------------------
// Request encoder
// ---------------------------------------------------------------------------

struct CoapEnc
{
    uint8_t *buf;
    size_t len;
    uint32_t last_opt;
};

static void enc_init(CoapEnc *e, uint8_t *buf, uint8_t type, uint8_t code, const uint8_t *token, uint8_t tkl,
                     uint16_t mid)
{
    e->buf = buf;
    e->len = 0;
    e->last_opt = 0;
    buf[e->len++] = (uint8_t)((1 << 6) | (type << 4) | tkl);
    buf[e->len++] = code;
    buf[e->len++] = (uint8_t)(mid >> 8);
    buf[e->len++] = (uint8_t)(mid & 0xFF);
    for (uint8_t i = 0; i < tkl; i++)
        buf[e->len++] = token[i];
}

static void enc_nibble(uint8_t *out_nib, uint8_t *ext, int *next, uint32_t v)
{
    if (v < 13)
    {
        *out_nib = (uint8_t)v;
        *next = 0;
    }
    else if (v < 269)
    {
        *out_nib = 13;
        ext[0] = (uint8_t)(v - 13);
        *next = 1;
    }
    else
    {
        *out_nib = 14;
        uint32_t x = v - 269;
        ext[0] = (uint8_t)(x >> 8);
        ext[1] = (uint8_t)(x & 0xFF);
        *next = 2;
    }
}

static void enc_option(CoapEnc *e, uint32_t num, const uint8_t *val, size_t vlen)
{
    uint32_t delta = num - e->last_opt;
    e->last_opt = num;
    uint8_t dn, ln, de[2], le[2];
    int nde, nle;
    enc_nibble(&dn, de, &nde, delta);
    enc_nibble(&ln, le, &nle, (uint32_t)vlen);
    e->buf[e->len++] = (uint8_t)((dn << 4) | ln);
    for (int i = 0; i < nde; i++)
        e->buf[e->len++] = de[i];
    for (int i = 0; i < nle; i++)
        e->buf[e->len++] = le[i];
    for (size_t i = 0; i < vlen; i++)
        e->buf[e->len++] = val[i];
}

static void enc_payload(CoapEnc *e, const uint8_t *pl, size_t n)
{
    e->buf[e->len++] = 0xFF;
    memcpy(e->buf + e->len, pl, n);
    e->len += n;
}

// Build a request with path segments, optional query segments, optional request
// Content-Format (req_cf < 0 = none) and optional payload. Options are emitted in
// ascending option-number order (Uri-Path 11, Content-Format 12, Uri-Query 15).
static size_t build(uint8_t *buf, uint8_t type, uint8_t code, const uint8_t *token, uint8_t tkl, uint16_t mid,
                    const char *const *paths, int npaths, const char *const *queries, int nq, int req_cf,
                    const uint8_t *payload, size_t plen)
{
    CoapEnc e;
    enc_init(&e, buf, type, code, token, tkl, mid);
    for (int i = 0; i < npaths; i++)
        enc_option(&e, 11, (const uint8_t *)paths[i], strlen(paths[i]));
    if (req_cf >= 0)
    {
        uint8_t cfv[2];
        int n = 0;
        if (req_cf > 0xFF)
        {
            cfv[n++] = (uint8_t)(req_cf >> 8);
            cfv[n++] = (uint8_t)(req_cf & 0xFF);
        }
        else if (req_cf > 0)
        {
            cfv[n++] = (uint8_t)req_cf;
        }
        enc_option(&e, 12, cfv, (size_t)n);
    }
    for (int i = 0; i < nq; i++)
        enc_option(&e, 15, (const uint8_t *)queries[i], strlen(queries[i]));
    if (payload && plen)
        enc_payload(&e, payload, plen);
    return e.len;
}

// ---------------------------------------------------------------------------
// Response decoder
// ---------------------------------------------------------------------------

struct CoapDec
{
    uint8_t ver, type, tkl, code;
    uint16_t mid;
    const uint8_t *token;
    uint16_t content_format;
    int observe; // Observe option value (RFC 7641), or -1 if absent
    int block1;  // Block1 option value (RFC 7959), or -1 if absent
    int block2;  // Block2 option value, or -1 if absent
    const uint8_t *payload;
    size_t payload_len;
};

// Block option field accessors (RFC 7959 §2.2: value = (NUM<<4)|(M<<3)|SZX).
#define BLK_NUM(v) ((uint32_t)(v) >> 4)
#define BLK_M(v) (((uint32_t)(v) >> 3) & 1)
#define BLK_SZX(v) ((uint32_t)(v) & 7)

static bool dec(const uint8_t *buf, size_t len, CoapDec *d)
{
    if (len < 4)
        return false;
    d->ver = buf[0] >> 6;
    d->type = (buf[0] >> 4) & 0x03;
    d->tkl = buf[0] & 0x0F;
    d->code = buf[1];
    d->mid = (uint16_t)((buf[2] << 8) | buf[3]);
    d->token = buf + 4;
    d->content_format = COAP_CF_NONE;
    d->observe = -1;
    d->block1 = -1;
    d->block2 = -1;
    d->payload = nullptr;
    d->payload_len = 0;
    size_t p = 4 + d->tkl;
    uint32_t opt = 0;
    while (p < len)
    {
        if (buf[p] == 0xFF)
        {
            d->payload = buf + p + 1;
            d->payload_len = len - p - 1;
            break;
        }
        uint8_t b = buf[p++];
        uint32_t delta = b >> 4, l = b & 0x0F;
        if (delta == 13)
            delta = buf[p++] + 13;
        else if (delta == 14)
        {
            delta = ((buf[p] << 8) | buf[p + 1]) + 269;
            p += 2;
        }
        if (l == 13)
            l = buf[p++] + 13;
        else if (l == 14)
        {
            l = ((buf[p] << 8) | buf[p + 1]) + 269;
            p += 2;
        }
        opt += delta;
        if (opt == 12 || opt == 6 || opt == 23 || opt == 27)
        {
            uint32_t v = 0;
            for (uint32_t k = 0; k < l; k++)
                v = (v << 8) | buf[p + k];
            if (opt == 12)
                d->content_format = (uint16_t)v;
            else if (opt == 6)
                d->observe = (int)v;
            else if (opt == 23)
                d->block2 = (int)v;
            else
                d->block1 = (int)v;
        }
        p += l;
    }
    return true;
}

// Encode a Block option (RFC 7959) with the minimal-byte value into the request.
static void enc_block(CoapEnc *e, uint32_t optnum, uint32_t num, uint8_t m, uint8_t szx)
{
    uint32_t v = (num << 4) | ((uint32_t)(m & 1) << 3) | (szx & 7);
    uint8_t vb[3];
    uint8_t k = 0;
    if (v & 0xFF0000)
        vb[k++] = (uint8_t)(v >> 16);
    if (v & 0xFFFF00)
        vb[k++] = (uint8_t)(v >> 8);
    if (v)
        vb[k++] = (uint8_t)v;
    enc_option(e, optnum, vb, k);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void test_get_content()
{
    const char *paths[] = {"temp"};
    uint8_t tok[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, tok, 4, 0x1234, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    TEST_ASSERT_GREATER_THAN_UINT(0, n);

    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(1, d.ver);
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_ACK, d.type); // CON -> piggybacked ACK
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
    TEST_ASSERT_EQUAL_UINT(0x1234, d.mid);
    TEST_ASSERT_EQUAL_UINT(4, d.tkl);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(tok, d.token, 4);
    TEST_ASSERT_EQUAL_UINT(COAP_CF_TEXT, d.content_format);
    TEST_ASSERT_EQUAL_UINT(2, d.payload_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("hi", d.payload, 2);

    TEST_ASSERT_TRUE(g_called);
    TEST_ASSERT_EQUAL_UINT(COAP_GET, g_method);
    TEST_ASSERT_EQUAL_STRING("/temp", g_path);
}

void test_not_found()
{
    const char *paths[] = {"missing"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x0001, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_NOT_FOUND, d.code);
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_ACK, d.type);
    TEST_ASSERT_FALSE(g_called);
}

void test_method_not_allowed()
{
    const char *paths[] = {"ro"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_PUT, nullptr, 0, 0x0002, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_METHOD_NOT_ALLOWED, d.code);
    TEST_ASSERT_FALSE(g_called);
}

void test_non_request_type()
{
    const char *paths[] = {"temp"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_NON, COAP_GET, nullptr, 0, 0x0003, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_NON, d.type); // NON request -> NON response
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
}

void test_put_with_payload()
{
    const char *paths[] = {"temp"};
    const uint8_t body[] = {'2', '5'};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_PUT, nullptr, 0, 0x0004, paths, 1, nullptr, 0, COAP_CF_TEXT, body, 2);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CHANGED, d.code);

    TEST_ASSERT_TRUE(g_called);
    TEST_ASSERT_EQUAL_UINT(COAP_PUT, g_method);
    TEST_ASSERT_EQUAL_UINT(2, g_payload_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(body, g_payload, 2);
    TEST_ASSERT_EQUAL_UINT(COAP_CF_TEXT, g_cf);
}

void test_multi_segment_path()
{
    const char *paths[] = {"a", "b"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x0005, paths, 2, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
    TEST_ASSERT_EQUAL_STRING("/a/b", g_path);
}

void test_uri_query()
{
    const char *paths[] = {"temp"};
    const char *queries[] = {"x=1", "y=2"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x0006, paths, 1, queries, 2, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
    TEST_ASSERT_EQUAL_STRING("x=1&y=2", g_query);
}

void test_empty_con_ping_rst()
{
    uint8_t req[8], resp[16];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, 0 /* empty */, nullptr, 0, 0x4242);
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_RST, d.type);
    TEST_ASSERT_EQUAL_UINT(0, d.code);
    TEST_ASSERT_EQUAL_UINT(0x4242, d.mid);
    TEST_ASSERT_EQUAL_UINT(0, d.tkl);
}

void test_bad_version_rst()
{
    uint8_t req[8] = {0};
    req[0] = (uint8_t)((2 << 6) | (COAP_TYPE_CON << 4) | 0); // Ver=2 (invalid)
    req[1] = COAP_GET;
    req[2] = 0x12;
    req[3] = 0x34;
    uint8_t resp[16];
    size_t n = coap_server_process(req, 4, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_RST, d.type);
    TEST_ASSERT_EQUAL_UINT(0x1234, d.mid);
}

void test_delete()
{
    const char *paths[] = {"temp"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_DELETE, nullptr, 0, 0x0007, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_DELETED, d.code);
    TEST_ASSERT_EQUAL_UINT(COAP_DELETE, g_method);
}

void test_token_8_bytes()
{
    const char *paths[] = {"temp"};
    uint8_t tok[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, tok, 8, 0x0008, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(8, d.tkl);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(tok, d.token, 8);
}

void test_extended_option_length()
{
    const char *paths[] = {"longresourcename12345"}; // 21 bytes -> option length 13 (extended)
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x0009, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
    TEST_ASSERT_EQUAL_STRING("/longresourcename12345", g_path);
}

void test_ack_ignored()
{
    uint8_t req[8];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_ACK, COAP_RSP_CONTENT, nullptr, 0, 0x00AA);
    uint8_t resp[16];
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    TEST_ASSERT_EQUAL_UINT(0, n); // ACK is not a request
}

void test_root_path()
{
    uint8_t req[16], resp[64];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x000B, nullptr, 0, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
    TEST_ASSERT_EQUAL_STRING("/", g_path);
}

void test_unknown_method_not_implemented()
{
    const char *paths[] = {"temp"};
    uint8_t req[128], resp[128];
    // Code 0.05 (FETCH) is a valid class-0 code we don't implement.
    size_t rl = build(req, COAP_TYPE_CON, COAP_CODE(0, 5), nullptr, 0, 0x000C, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_NOT_IMPLEMENTED, d.code);
}

// RFC 7641: coap_server_process_ex() includes an Observe option (6, before
// Content-Format 12) carrying the notification sequence on a 2.xx response.
void test_observe_option_in_response()
{
    const char *paths[] = {"ro"};
    uint8_t tok[] = {0x01, 0x02};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, tok, 2, 0x2222, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process_ex(req, rl, resp, sizeof(resp), 5);
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_TYPE_ACK, d.type);
    TEST_ASSERT_EQUAL_INT(5, d.observe);                      // Observe option present, seq 5
    TEST_ASSERT_EQUAL_UINT16(COAP_CF_TEXT, d.content_format); // still ordered/decoded after Observe
    TEST_ASSERT_EQUAL_size_t(2, d.payload_len);
}

void test_no_observe_option_when_seq_negative()
{
    const char *paths[] = {"ro"};
    uint8_t req[128], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x2223, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process_ex(req, rl, resp, sizeof(resp), -1);
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_INT(-1, d.observe); // no Observe option
}

// ---------------------------------------------------------------------------
// Block-wise transfer (RFC 7959). The test env builds with a 64-byte max block
// (SZX_MAX=2) and a 128-byte Block1 reassembly buffer.
// ---------------------------------------------------------------------------

// Block2: a client paging a 150-byte resource in 64-byte blocks (SZX=2) gets the
// right block contents with the More bit set until the final (short) block.
void test_block2_explicit_paging()
{
    const uint8_t expect_more[] = {1, 1, 0};             // blocks 0,1 have More; block 2 is last
    const size_t expect_len[] = {64, 64, BIG_LEN - 128}; // 64, 64, 22
    for (uint32_t num = 0; num < 3; num++)
    {
        uint8_t req[64], resp[256];
        CoapEnc e;
        enc_init(&e, req, COAP_TYPE_CON, COAP_GET, nullptr, 0, (uint16_t)(0x3000 + num));
        enc_option(&e, 11, (const uint8_t *)"big", 3);
        enc_block(&e, 23, num, 0, 2); // Block2: NUM, M=0, SZX=2 (64 bytes)
        size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
        TEST_ASSERT_GREATER_THAN_UINT(0, n);

        CoapDec d;
        TEST_ASSERT_TRUE(dec(resp, n, &d));
        TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTENT, d.code);
        TEST_ASSERT_TRUE(d.block2 >= 0);
        TEST_ASSERT_EQUAL_UINT(num, BLK_NUM(d.block2));
        TEST_ASSERT_EQUAL_UINT(2, BLK_SZX(d.block2));
        TEST_ASSERT_EQUAL_UINT(expect_more[num], BLK_M(d.block2));
        TEST_ASSERT_EQUAL_size_t(expect_len[num], d.payload_len);
        for (size_t i = 0; i < d.payload_len; i++)
            TEST_ASSERT_EQUAL_UINT8(big_expected(num * 64 + i), d.payload[i]);
    }
}

// Block2: with no Block2 option, a representation larger than the server's max
// block size is served block-wise starting at block 0 (server max SZX = 2).
void test_block2_auto_when_large()
{
    uint8_t req[64], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x3100);
    enc_option(&e, 11, (const uint8_t *)"big", 3);
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_TRUE(d.block2 >= 0);
    TEST_ASSERT_EQUAL_UINT(0, BLK_NUM(d.block2));
    TEST_ASSERT_EQUAL_UINT(1, BLK_M(d.block2));
    TEST_ASSERT_EQUAL_UINT(2, BLK_SZX(d.block2)); // clamped to server max
    TEST_ASSERT_EQUAL_size_t(64, d.payload_len);
}

// Block2: a client requesting a block size larger than the server supports is
// clamped down to the server's max SZX (2).
void test_block2_szx_clamped()
{
    uint8_t req[64], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x3200);
    enc_option(&e, 11, (const uint8_t *)"big", 3);
    enc_block(&e, 23, 0, 0, 6); // ask for 1024-byte blocks
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(2, BLK_SZX(d.block2)); // server clamps to SZX_MAX=2
    TEST_ASSERT_EQUAL_size_t(64, d.payload_len);
}

// Block2: a small representation (no Block2 requested) is returned whole, with no
// Block2 option in the response.
void test_block2_absent_for_small()
{
    const char *paths[] = {"temp"};
    uint8_t req[64], resp[128];
    size_t rl = build(req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x3300, paths, 1, nullptr, 0, -1, nullptr, 0);
    size_t n = coap_server_process(req, rl, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_INT(-1, d.block2);
    TEST_ASSERT_EQUAL_size_t(2, d.payload_len);
}

// Block2: a block number beyond the end of the representation is a bad request.
void test_block2_out_of_range()
{
    uint8_t req[64], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x3400);
    enc_option(&e, 11, (const uint8_t *)"big", 3);
    enc_block(&e, 23, 10, 0, 2); // offset 640 > 150
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_BAD_REQUEST, d.code);
}

// Block2: the reserved block-size exponent SZX=7 is rejected with 4.02 Bad Option.
void test_block2_reserved_szx()
{
    uint8_t req[64], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_GET, nullptr, 0, 0x3500);
    enc_option(&e, 11, (const uint8_t *)"big", 3);
    enc_block(&e, 23, 0, 0, 7);
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_BAD_OPTION, d.code);
}

// Block1: a two-block POST upload is acknowledged 2.31 Continue on the first
// block and dispatched (with the full reassembled payload) on the last.
void test_block1_upload_two_blocks()
{
    uint8_t chunk0[64], chunk1[20];
    for (int i = 0; i < 64; i++)
        chunk0[i] = (uint8_t)i;
    for (int i = 0; i < 20; i++)
        chunk1[i] = (uint8_t)(100 + i);

    // Block 0 (More=1): expect 2.31 Continue, no handler dispatch yet.
    uint8_t req[128], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_POST, nullptr, 0, 0x3600);
    enc_option(&e, 11, (const uint8_t *)"temp", 4);
    enc_block(&e, 27, 0, 1, 2); // Block1: NUM=0, M=1, SZX=2
    enc_payload(&e, chunk0, 64);
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTINUE, d.code);
    TEST_ASSERT_TRUE(d.block1 >= 0);
    TEST_ASSERT_EQUAL_UINT(0, BLK_NUM(d.block1));
    TEST_ASSERT_EQUAL_UINT(1, BLK_M(d.block1));
    TEST_ASSERT_FALSE(g_called);

    // Block 1 (More=0): handler runs with the whole 84-byte payload.
    enc_init(&e, req, COAP_TYPE_CON, COAP_POST, nullptr, 0, 0x3601);
    enc_option(&e, 11, (const uint8_t *)"temp", 4);
    enc_block(&e, 27, 1, 0, 2); // Block1: NUM=1, M=0
    enc_payload(&e, chunk1, 20);
    n = coap_server_process(req, e.len, resp, sizeof(resp));
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_CREATED, d.code);
    TEST_ASSERT_TRUE(d.block1 >= 0);
    TEST_ASSERT_EQUAL_UINT(1, BLK_NUM(d.block1));
    TEST_ASSERT_EQUAL_UINT(0, BLK_M(d.block1));

    TEST_ASSERT_TRUE(g_called);
    TEST_ASSERT_EQUAL_UINT(84, g_payload_len);
    for (int i = 0; i < 64; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, g_payload[i]);
    for (int i = 0; i < 20; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(100 + i), g_payload[64 + i]);
}

// Block1: a gap in the block sequence (a lost block) yields 4.08 Incomplete.
void test_block1_out_of_order()
{
    uint8_t chunk[64];
    for (int i = 0; i < 64; i++)
        chunk[i] = (uint8_t)i;
    uint8_t req[128], resp[256];
    CoapEnc e;
    enc_init(&e, req, COAP_TYPE_CON, COAP_POST, nullptr, 0, 0x3700);
    enc_option(&e, 11, (const uint8_t *)"temp", 4);
    enc_block(&e, 27, 0, 1, 2);
    enc_payload(&e, chunk, 64);
    coap_server_process(req, e.len, resp, sizeof(resp)); // block 0 -> Continue

    enc_init(&e, req, COAP_TYPE_CON, COAP_POST, nullptr, 0, 0x3701);
    enc_option(&e, 11, (const uint8_t *)"temp", 4);
    enc_block(&e, 27, 2, 0, 2); // skip block 1
    enc_payload(&e, chunk, 64);
    size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
    CoapDec d;
    TEST_ASSERT_TRUE(dec(resp, n, &d));
    TEST_ASSERT_EQUAL_UINT(COAP_RSP_REQUEST_INCOMPLETE, d.code);
}

// Block1: an upload exceeding the reassembly buffer (128 bytes here) yields 4.13.
void test_block1_too_large()
{
    uint8_t chunk[64];
    for (int i = 0; i < 64; i++)
        chunk[i] = (uint8_t)i;
    uint8_t req[128], resp[256];
    CoapDec d;
    for (uint32_t num = 0; num < 3; num++)
    {
        CoapEnc e;
        enc_init(&e, req, COAP_TYPE_CON, COAP_POST, nullptr, 0, (uint16_t)(0x3800 + num));
        enc_option(&e, 11, (const uint8_t *)"temp", 4);
        enc_block(&e, 27, num, 1, 2); // all More=1
        enc_payload(&e, chunk, 64);
        size_t n = coap_server_process(req, e.len, resp, sizeof(resp));
        TEST_ASSERT_TRUE(dec(resp, n, &d));
        if (num < 2)
            TEST_ASSERT_EQUAL_UINT(COAP_RSP_CONTINUE, d.code); // 64, 128 bytes buffered
        else
            TEST_ASSERT_EQUAL_UINT(COAP_RSP_REQUEST_TOO_LARGE, d.code); // 192 > 128
    }
    TEST_ASSERT_FALSE(g_called); // handler never ran
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_block2_explicit_paging);
    RUN_TEST(test_block2_auto_when_large);
    RUN_TEST(test_block2_szx_clamped);
    RUN_TEST(test_block2_absent_for_small);
    RUN_TEST(test_block2_out_of_range);
    RUN_TEST(test_block2_reserved_szx);
    RUN_TEST(test_block1_upload_two_blocks);
    RUN_TEST(test_block1_out_of_order);
    RUN_TEST(test_block1_too_large);
    RUN_TEST(test_observe_option_in_response);
    RUN_TEST(test_no_observe_option_when_seq_negative);
    RUN_TEST(test_get_content);
    RUN_TEST(test_not_found);
    RUN_TEST(test_method_not_allowed);
    RUN_TEST(test_non_request_type);
    RUN_TEST(test_put_with_payload);
    RUN_TEST(test_multi_segment_path);
    RUN_TEST(test_uri_query);
    RUN_TEST(test_empty_con_ping_rst);
    RUN_TEST(test_bad_version_rst);
    RUN_TEST(test_delete);
    RUN_TEST(test_token_8_bytes);
    RUN_TEST(test_extended_option_length);
    RUN_TEST(test_ack_ignored);
    RUN_TEST(test_root_path);
    RUN_TEST(test_unknown_method_not_implemented);
    return UNITY_END();
}
