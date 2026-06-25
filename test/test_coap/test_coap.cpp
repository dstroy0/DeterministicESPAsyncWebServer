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
    const uint8_t *payload;
    size_t payload_len;
};

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
        if (opt == 12)
        {
            uint32_t v = 0;
            for (uint32_t k = 0; k < l; k++)
                v = (v << 8) | buf[p + k];
            d->content_format = (uint16_t)v;
        }
        p += l;
    }
    return true;
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

int main()
{
    UNITY_BEGIN();
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
