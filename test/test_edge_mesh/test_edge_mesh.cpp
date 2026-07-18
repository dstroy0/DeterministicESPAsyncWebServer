// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/edge_cache/edge_mesh: the CDN edge cache's mesh (sibling-cache) wire codec and
// async peer-query engine. Covers the request/response frame build + tri-state accumulating parse (partial
// reads, magic/version/opcode validation), the freshness-carrying entry frame round-trip (content via the
// shared edge_sd body plus the timing trailer, incl. a binary body), RFC 9111 age propagation (a peer's
// corrected age transfers so the receiver stays fresh for the remaining lifetime and its age keeps growing),
// and the requester engine reaching HIT / MISS / FAILED (open fail, send fail, timeout, peer close, malformed)
// over a scripted mock transport seam.

#include "services/edge_cache/edge_cache.h"
#include "services/edge_cache/edge_mesh.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// --- helpers -------------------------------------------------------------------------------------
static void mkcanon(char *out, size_t cap, const char *path)
{
    snprintf(out, cap, "GET\nexample.com\n%s", path);
}

// Fill a bare entry (fields only, not linked to a store) with a validator + body + freshness.
static void fill_entry(EdgeEntry *e, const char *canon, const char *etag, const uint8_t *body, uint16_t body_len)
{
    memset(e, 0, sizeof(*e));
    strncpy(e->key, canon, sizeof(e->key) - 1);
    edge_key_digest(e->key, strlen(e->key), e->digest);
    e->status = 200;
    strncpy(e->content_type, "text/plain", sizeof(e->content_type) - 1);
    strncpy(e->etag, etag, sizeof(e->etag) - 1);
    if (body && body_len)
        memcpy(e->body, body, body_len);
    e->body_len = body_len;
    e->date_epoch = -1;
    e->expires_epoch = -1;
    e->age_hdr = 0;
    e->lifetime_s = 60;
    e->initial_age = 0;
    e->insert_ms = 0;
}

// --- request frame -------------------------------------------------------------------------------
static void test_request_roundtrip()
{
    uint8_t digest[32];
    for (int i = 0; i < 32; i++)
        digest[i] = (uint8_t)(i * 7 + 1);
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/app.js?v=2");
    const char *vary = "Accept-Encoding\x1egzip\x1f";

    uint8_t req[EDGE_MESH_REQ_MAX];
    size_t n = edge_mesh_build_request(digest, canon, vary, req, sizeof(req));
    TEST_ASSERT_TRUE(n > 0);

    uint8_t d2[32];
    char c2[DWS_EDGE_KEY_MAX];
    char v2[DWS_MESH_HDRS_MAX];
    TEST_ASSERT_EQUAL(EdgeMeshParse::HIT, edge_mesh_parse_request(req, n, d2, c2, sizeof(c2), v2, sizeof(v2)));
    TEST_ASSERT_EQUAL_MEMORY(digest, d2, 32);
    TEST_ASSERT_EQUAL_STRING(canon, c2);
    TEST_ASSERT_EQUAL_STRING(vary, v2);
}

static void test_request_incomplete_then_complete()
{
    uint8_t digest[32];
    memset(digest, 0xAB, 32);
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/x");
    uint8_t req[EDGE_MESH_REQ_MAX];
    size_t n = edge_mesh_build_request(digest, canon, "Accept\x1e*\x1f", req, sizeof(req));
    TEST_ASSERT_TRUE(n > 3);

    uint8_t d2[32];
    char c2[DWS_EDGE_KEY_MAX];
    char v2[DWS_MESH_HDRS_MAX];
    // A valid prefix (short of the full frame) accumulates rather than erroring.
    TEST_ASSERT_EQUAL(EdgeMeshParse::INCOMPLETE, edge_mesh_parse_request(req, 2, d2, c2, sizeof(c2), v2, sizeof(v2)));
    TEST_ASSERT_EQUAL(EdgeMeshParse::INCOMPLETE,
                      edge_mesh_parse_request(req, n - 1, d2, c2, sizeof(c2), v2, sizeof(v2)));
    TEST_ASSERT_EQUAL(EdgeMeshParse::HIT, edge_mesh_parse_request(req, n, d2, c2, sizeof(c2), v2, sizeof(v2)));
}

static void test_request_malformed()
{
    uint8_t digest[32];
    memset(digest, 1, 32);
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/y");
    uint8_t req[EDGE_MESH_REQ_MAX];
    size_t n = edge_mesh_build_request(digest, canon, "", req, sizeof(req));

    uint8_t d2[32];
    char c2[DWS_EDGE_KEY_MAX];
    char v2[DWS_MESH_HDRS_MAX];
    uint8_t bad = req[0];
    req[0] = 'X'; // wrong magic
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_request(req, n, d2, c2, sizeof(c2), v2, sizeof(v2)));
    req[0] = bad;
    req[3] = 9; // unknown opcode
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_request(req, n, d2, c2, sizeof(c2), v2, sizeof(v2)));
    req[3] = EDGE_MESH_OP_GET;
    // a key that cannot fit the destination buffer is malformed, not silently truncated
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_request(req, n, d2, c2, 4, v2, sizeof(v2)));
}

// --- entry frame (content + freshness) -----------------------------------------------------------
static void test_entry_frame_roundtrip()
{
    static const uint8_t body[] = {0x00, 0xFF, 0x10, 'a', 0x00, 'z', 0x7F, 0x80}; // binary-safe
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/img.png");
    EdgeEntry in;
    fill_entry(&in, canon, "\"etag-v1\"", body, sizeof(body));
    strncpy(in.content_encoding, "gzip", sizeof(in.content_encoding) - 1);
    strncpy(in.last_modified, "Wed, 21 Oct 2026 07:28:00 GMT", sizeof(in.last_modified) - 1);
    strncpy(in.vary_names, "Accept-Encoding", sizeof(in.vary_names) - 1);
    strncpy(in.vary_vals, "Accept-Encoding\x1egzip\x1f", sizeof(in.vary_vals) - 1);
    in.date_epoch = 1000000;
    in.expires_epoch = 1000060;
    in.age_hdr = 3;
    in.lifetime_s = 60;

    long cur = 12;
    uint8_t frame[EDGE_MESH_ENTRY_MAX];
    size_t n = edge_mesh_serialize_entry(&in, cur, frame, sizeof(frame));
    TEST_ASSERT_TRUE(n > 0);

    EdgeEntry out;
    memset(&out, 0, sizeof(out));
    uint32_t now2 = 50000;
    TEST_ASSERT_TRUE(edge_mesh_deserialize_entry(frame, n, &out, now2));
    // content
    TEST_ASSERT_EQUAL_STRING(in.key, out.key);
    TEST_ASSERT_EQUAL_INT(in.status, out.status);
    TEST_ASSERT_EQUAL_STRING(in.content_type, out.content_type);
    TEST_ASSERT_EQUAL_STRING(in.etag, out.etag);
    TEST_ASSERT_EQUAL_STRING(in.content_encoding, out.content_encoding);
    TEST_ASSERT_EQUAL_STRING(in.last_modified, out.last_modified);
    TEST_ASSERT_EQUAL_STRING(in.vary_names, out.vary_names);
    TEST_ASSERT_EQUAL_STRING(in.vary_vals, out.vary_vals);
    TEST_ASSERT_EQUAL_UINT(in.body_len, out.body_len);
    TEST_ASSERT_EQUAL_MEMORY(in.body, out.body, in.body_len);
    TEST_ASSERT_EQUAL_MEMORY(in.digest, out.digest, 32); // re-derived from the restored key
    // freshness
    TEST_ASSERT_EQUAL_INT64(in.date_epoch, out.date_epoch);
    TEST_ASSERT_EQUAL_INT64(in.expires_epoch, out.expires_epoch);
    TEST_ASSERT_EQUAL_INT(in.lifetime_s, out.lifetime_s);
    TEST_ASSERT_EQUAL_INT(in.age_hdr, out.age_hdr);
    TEST_ASSERT_EQUAL_INT(cur, out.initial_age);
    TEST_ASSERT_EQUAL_UINT32(now2, out.insert_ms);
}

static void test_age_propagation()
{
    static const uint8_t body[] = {'h', 'i'};
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/z");
    EdgeEntry peer;
    fill_entry(&peer, canon, "\"v1\"", body, sizeof(body));
    peer.lifetime_s = 60;
    peer.initial_age = 5;
    peer.insert_ms = 10000;

    uint32_t send_now = 10000 + 12000;                                       // 12 s resident on the peer
    long cur = edge_current_age(peer.initial_age, peer.insert_ms, send_now); // 5 + 12
    TEST_ASSERT_EQUAL_INT(17, cur);

    uint8_t frame[EDGE_MESH_ENTRY_MAX];
    size_t n = edge_mesh_serialize_entry(&peer, cur, frame, sizeof(frame));
    EdgeEntry recv;
    memset(&recv, 0, sizeof(recv));
    uint32_t recv_now = 999000; // the receiver's clock is unrelated to the peer's
    TEST_ASSERT_TRUE(edge_mesh_deserialize_entry(frame, n, &recv, recv_now));

    // the receiver's age at receipt equals the transferred age, and grows with local hold
    TEST_ASSERT_EQUAL_INT(17, edge_current_age(recv.initial_age, recv.insert_ms, recv_now));
    TEST_ASSERT_EQUAL_INT(17 + 30, edge_current_age(recv.initial_age, recv.insert_ms, recv_now + 30000));
    // still fresh (17 < 60), and stale once the remaining lifetime (43 s) elapses
    TEST_ASSERT_TRUE(edge_entry_fresh(&recv, recv_now));
    TEST_ASSERT_TRUE(edge_entry_fresh(&recv, recv_now + 42000));
    TEST_ASSERT_FALSE(edge_entry_fresh(&recv, recv_now + 44000));
}

// --- response frame ------------------------------------------------------------------------------
static void build_hit_frame(uint8_t *frame, size_t cap, size_t *fn_out, long current_age)
{
    static const uint8_t body[] = {'p', 'a', 'y', 'l', 'o', 'a', 'd'};
    char canon[DWS_EDGE_KEY_MAX];
    mkcanon(canon, sizeof(canon), "/cdn/r");
    EdgeEntry e;
    fill_entry(&e, canon, "\"rv\"", body, sizeof(body));
    *fn_out = edge_mesh_serialize_entry(&e, current_age, frame, cap);
}

static void test_response_roundtrip()
{
    uint8_t frame[EDGE_MESH_ENTRY_MAX];
    size_t fn = 0;
    build_hit_frame(frame, sizeof(frame), &fn, 0);
    TEST_ASSERT_TRUE(fn > 0);

    uint8_t resp[EDGE_MESH_RESP_MAX];
    size_t rn = edge_mesh_build_response(true, frame, fn, resp, sizeof(resp));
    TEST_ASSERT_TRUE(rn > 0);

    size_t eoff = 0;
    size_t elen = 0;
    TEST_ASSERT_EQUAL(EdgeMeshParse::HIT, edge_mesh_parse_response(resp, rn, &eoff, &elen));
    TEST_ASSERT_EQUAL_UINT(fn, elen);
    TEST_ASSERT_EQUAL_MEMORY(frame, resp + eoff, fn);
    // a truncated HIT accumulates
    TEST_ASSERT_EQUAL(EdgeMeshParse::INCOMPLETE, edge_mesh_parse_response(resp, 5, &eoff, &elen));
    TEST_ASSERT_EQUAL(EdgeMeshParse::INCOMPLETE, edge_mesh_parse_response(resp, rn - 1, &eoff, &elen));

    uint8_t miss[8];
    size_t mn = edge_mesh_build_response(false, nullptr, 0, miss, sizeof(miss));
    TEST_ASSERT_EQUAL(EdgeMeshParse::MISS, edge_mesh_parse_response(miss, mn, &eoff, &elen));
}

static void test_response_malformed()
{
    size_t eoff = 0;
    size_t elen = 0;
    uint8_t bad_magic[6] = {'X', 'M', EDGE_MESH_VERSION, 1, 0, 0};
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_response(bad_magic, 6, &eoff, &elen));
    uint8_t bad_status[6] = {'E', 'M', EDGE_MESH_VERSION, 5, 0, 0};
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_response(bad_status, 6, &eoff, &elen));
    uint8_t zero_len[6] = {'E', 'M', EDGE_MESH_VERSION, 1, 0, 0}; // HIT with entry_len 0
    TEST_ASSERT_EQUAL(EdgeMeshParse::MALFORMED, edge_mesh_parse_response(zero_len, 6, &eoff, &elen));
}

// --- scripted mock peer transport (delivers a canned response) -----------------------------------
struct MockPeer
{
    const uint8_t *data;
    size_t len, cursor, throttle;
    bool closes;
    int open_ret;
    bool send_ok;
};
static int p_open(void *c, const char *h, uint16_t p, uint32_t t)
{
    (void)h;
    (void)p;
    (void)t;
    return ((MockPeer *)c)->open_ret;
}
static bool p_send(void *c, int cid, const void *d, size_t l)
{
    (void)cid;
    (void)d;
    (void)l;
    return ((MockPeer *)c)->send_ok;
}
static size_t p_read(void *c, int cid, uint8_t *buf, size_t cap)
{
    (void)cid;
    MockPeer *m = (MockPeer *)c;
    size_t avail = m->len - m->cursor;
    size_t n = avail < cap ? avail : cap;
    if (m->throttle && n > m->throttle)
        n = m->throttle;
    memcpy(buf, m->data + m->cursor, n);
    m->cursor += n;
    return n;
}
static bool p_closed(void *c, int cid)
{
    (void)cid;
    MockPeer *m = (MockPeer *)c;
    return m->closes && m->cursor >= m->len;
}
static void p_close(void *c, int cid)
{
    (void)c;
    (void)cid;
}
static EdgeFetchTransport peer_transport(MockPeer *m)
{
    EdgeFetchTransport t;
    t.open = p_open;
    t.send = p_send;
    t.read = p_read;
    t.closed = p_closed;
    t.close = p_close;
    t.ctx = m;
    return t;
}
static EdgeMeshStatus run_mesh(EdgeMeshFetch *m, const EdgeFetchTransport *t, uint32_t now)
{
    for (int i = 0; i < 100000 && m->st == EdgeMeshStatus::PENDING; i++)
        edge_mesh_fetch_pump(m, t, now);
    return m->st;
}

static uint8_t g_rbuf[EDGE_MESH_RESP_MAX]; // caller-owned response accumulation buffer for the requester engine

// --- requester engine ----------------------------------------------------------------------------
static void test_requester_hit()
{
    uint8_t frame[EDGE_MESH_ENTRY_MAX];
    size_t fn = 0;
    build_hit_frame(frame, sizeof(frame), &fn, 3);
    uint8_t resp[EDGE_MESH_RESP_MAX];
    size_t rn = edge_mesh_build_response(true, frame, fn, resp, sizeof(resp));

    MockPeer m = {resp, rn, 0, 4, true, 7, true}; // 4 bytes/read -> exercises multi-pump accumulation
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, resp, 8, g_rbuf, sizeof(g_rbuf), 1000); // request ignored by the mock
    TEST_ASSERT_EQUAL(EdgeMeshStatus::HIT, run_mesh(&mf, &t, 1000));
    TEST_ASSERT_EQUAL_UINT(fn, mf.entry_len);

    EdgeEntry got;
    memset(&got, 0, sizeof(got));
    TEST_ASSERT_TRUE(edge_mesh_deserialize_entry(mf.buf + mf.entry_off, mf.entry_len, &got, 2000));
    TEST_ASSERT_EQUAL_MEMORY(frame, mf.buf + mf.entry_off, fn);
    TEST_ASSERT_EQUAL_UINT(7, got.body_len);
    TEST_ASSERT_EQUAL_MEMORY("payload", got.body, 7);
    edge_mesh_fetch_end(&mf, &t);
}

static void test_requester_miss()
{
    uint8_t resp[8];
    size_t rn = edge_mesh_build_response(false, nullptr, 0, resp, sizeof(resp));
    MockPeer m = {resp, rn, 0, 0, true, 7, true};
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, resp, 8, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::MISS, run_mesh(&mf, &t, 1000));
}

static void test_requester_open_fail()
{
    MockPeer m = {(const uint8_t *)"", 0, 0, 0, false, -1, true};
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, (const uint8_t *)"x", 1, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::FAILED, mf.st);
}

static void test_requester_send_fail()
{
    MockPeer m = {(const uint8_t *)"", 0, 0, 0, false, 7, false}; // open ok, send fails
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, (const uint8_t *)"x", 1, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::FAILED, mf.st);
}

static void test_requester_timeout()
{
    // A truncated frame that never completes and the peer never closes -> deadline drives FAILED.
    uint8_t partial[4] = {'E', 'M', EDGE_MESH_VERSION, 1}; // HIT header, no entry
    MockPeer m = {partial, sizeof(partial), 0, 0, false, 7, true};
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, (const uint8_t *)"x", 1, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::PENDING, edge_mesh_fetch_pump(&mf, &t, 1000));
    TEST_ASSERT_EQUAL(EdgeMeshStatus::FAILED, edge_mesh_fetch_pump(&mf, &t, 1000 + DWS_MESH_QUERY_MS + 1));
}

static void test_requester_peer_closed_early()
{
    uint8_t partial[5] = {'E', 'M', EDGE_MESH_VERSION, 1, 0}; // incomplete entry length prefix
    MockPeer m = {partial, sizeof(partial), 0, 0, true, 7, true};
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, (const uint8_t *)"x", 1, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::FAILED, run_mesh(&mf, &t, 1000));
}

static void test_requester_malformed()
{
    uint8_t junk[6] = {'X', 'X', 0, 0, 0, 0};
    MockPeer m = {junk, sizeof(junk), 0, 0, true, 7, true};
    EdgeFetchTransport t = peer_transport(&m);
    EdgeMeshFetch mf;
    edge_mesh_fetch_begin(&mf, &t, "peer", 7645, (const uint8_t *)"x", 1, g_rbuf, sizeof(g_rbuf), 1000);
    TEST_ASSERT_EQUAL(EdgeMeshStatus::FAILED, run_mesh(&mf, &t, 1000));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_request_roundtrip);
    RUN_TEST(test_request_incomplete_then_complete);
    RUN_TEST(test_request_malformed);
    RUN_TEST(test_entry_frame_roundtrip);
    RUN_TEST(test_age_propagation);
    RUN_TEST(test_response_roundtrip);
    RUN_TEST(test_response_malformed);
    RUN_TEST(test_requester_hit);
    RUN_TEST(test_requester_miss);
    RUN_TEST(test_requester_open_fail);
    RUN_TEST(test_requester_send_fail);
    RUN_TEST(test_requester_timeout);
    RUN_TEST(test_requester_peer_closed_early);
    RUN_TEST(test_requester_malformed);
    return UNITY_END();
}
