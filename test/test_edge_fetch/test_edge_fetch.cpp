// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// Host tests for the CDN edge-cache async origin-fetch engine (services/edge_cache/edge_fetch): the
// completion detector and the state machine, driven over a scripted mock transport.

#include "services/edge_cache/edge_fetch.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// --- scripted mock origin transport --------------------------------------------------------------

struct MockOrigin
{
    const uint8_t *data;
    size_t len;
    size_t cursor;
    size_t throttle; // max bytes per read (0 = as much as fits)
    bool closes;     // origin closes its side once everything is delivered
    int open_ret;    // cid to hand back (or < 0 to fail the open)
};

static int m_open(void *c, const char *h, uint16_t p, uint32_t t)
{
    (void)h;
    (void)p;
    (void)t;
    return ((MockOrigin *)c)->open_ret;
}
static bool m_send(void *c, int cid, const void *d, size_t l)
{
    (void)c;
    (void)cid;
    (void)d;
    (void)l;
    return true;
}
static size_t m_read(void *c, int cid, uint8_t *buf, size_t cap)
{
    (void)cid;
    MockOrigin *m = (MockOrigin *)c;
    size_t avail = m->len - m->cursor;
    size_t n = avail < cap ? avail : cap;
    if (m->throttle && n > m->throttle)
        n = m->throttle;
    memcpy(buf, m->data + m->cursor, n);
    m->cursor += n;
    return n;
}
static bool m_closed(void *c, int cid)
{
    (void)cid;
    MockOrigin *m = (MockOrigin *)c;
    return m->closes && m->cursor >= m->len;
}
static void m_close(void *c, int cid)
{
    (void)c;
    (void)cid;
}

static EdgeFetchTransport make_transport(MockOrigin *m)
{
    EdgeFetchTransport t;
    t.open = m_open;
    t.send = m_send;
    t.read = m_read;
    t.closed = m_closed;
    t.close = m_close;
    t.ctx = m;
    return t;
}

// Pump to a terminal state (bounded iterations); returns the final status.
static EdgeFetchStatus run_fetch(EdgeFetch *f, const EdgeFetchTransport *t, uint32_t now)
{
    for (int i = 0; i < 100000 && f->st == EdgeFetchStatus::PENDING; i++)
        edge_fetch_pump(f, t, now);
    return f->st;
}

// --- tests ---------------------------------------------------------------------------------------

static void test_fetch_content_length()
{
    static const char *R = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nhello";
    MockOrigin m = {(const uint8_t *)R, strlen(R), 0, 4, true, 7}; // 4 bytes/read (multi-pump)
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::DONE, run_fetch(&f, &t, 1000));
    TEST_ASSERT_EQUAL_INT(200, f.status);
    TEST_ASSERT_EQUAL_UINT(5, f.body_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", f.buf + f.body_off, 5);
}

static void test_fetch_chunked()
{
    static const char *R =
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    MockOrigin m = {(const uint8_t *)R, strlen(R), 0, 3, true, 9};
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::DONE, run_fetch(&f, &t, 1000));
    TEST_ASSERT_EQUAL_INT(200, f.status);
    TEST_ASSERT_EQUAL_UINT(11, f.body_len);
    TEST_ASSERT_EQUAL_MEMORY("hello world", f.buf + f.body_off, 11);
}

static void test_fetch_close_delimited()
{
    static const char *R = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nbody-till-close";
    MockOrigin m = {(const uint8_t *)R, strlen(R), 0, 0, true, 5}; // no CL / chunked -> ends on close
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::DONE, run_fetch(&f, &t, 1000));
    TEST_ASSERT_EQUAL_INT(200, f.status);
    TEST_ASSERT_EQUAL_MEMORY("body-till-close", f.buf + f.body_off, 15);
}

static void test_fetch_oversize()
{
    static uint8_t big[DETWS_EDGE_FETCH_BUF + 1024];
    const char *head = "HTTP/1.1 200 OK\r\nContent-Length: 6000\r\n\r\n";
    size_t hl = strlen(head);
    memcpy(big, head, hl);
    memset(big + hl, 'x', sizeof(big) - hl); // body larger than the fetch buffer
    MockOrigin m = {big, sizeof(big), 0, 0, false, 3};
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::OVERSIZE, run_fetch(&f, &t, 1000));
}

static void test_fetch_timeout()
{
    static const char *R = "HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\npartial"; // never completes
    MockOrigin m = {(const uint8_t *)R, strlen(R), 0, 0, false, 2};
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::PENDING, edge_fetch_pump(&f, &t, 1000)); // drained, still waiting
    TEST_ASSERT_EQUAL(EdgeFetchStatus::FAILED, edge_fetch_pump(&f, &t, 1000 + DETWS_EDGE_FETCH_TIMEOUT_MS));
}

static void test_fetch_open_fail()
{
    MockOrigin m = {(const uint8_t *)"", 0, 0, 0, false, -1}; // open returns < 0
    EdgeFetchTransport t = make_transport(&m);
    EdgeFetch f;
    edge_fetch_begin(&f, &t, "h", 80, "GET / HTTP/1.1\r\n\r\n", 18, 1000);
    TEST_ASSERT_EQUAL(EdgeFetchStatus::FAILED, f.st);
}

static void test_resp_complete_unit()
{
    size_t hl = 0;
    const char *partial = "HTTP/1.1 200 OK\r\nContent-Len"; // header block not terminated
    TEST_ASSERT_FALSE(edge_resp_complete((const uint8_t *)partial, strlen(partial), false, &hl));
    TEST_ASSERT_EQUAL_UINT(0, hl);

    const char *cl = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nabc";
    TEST_ASSERT_TRUE(edge_resp_complete((const uint8_t *)cl, strlen(cl), false, &hl));
    const char *cl_short = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nab";
    TEST_ASSERT_FALSE(edge_resp_complete((const uint8_t *)cl_short, strlen(cl_short), false, &hl));

    const char *ch_inc = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n"; // no 0-chunk
    TEST_ASSERT_FALSE(edge_resp_complete((const uint8_t *)ch_inc, strlen(ch_inc), false, &hl));
    const char *ch_ok = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n";
    TEST_ASSERT_TRUE(edge_resp_complete((const uint8_t *)ch_ok, strlen(ch_ok), false, &hl));

    // close-delimited: incomplete until the peer closes
    const char *cd = "HTTP/1.1 200 OK\r\n\r\nsome body";
    TEST_ASSERT_FALSE(edge_resp_complete((const uint8_t *)cd, strlen(cd), false, &hl));
    TEST_ASSERT_TRUE(edge_resp_complete((const uint8_t *)cd, strlen(cd), true, &hl));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fetch_content_length);
    RUN_TEST(test_fetch_chunked);
    RUN_TEST(test_fetch_close_delimited);
    RUN_TEST(test_fetch_oversize);
    RUN_TEST(test_fetch_timeout);
    RUN_TEST(test_fetch_open_fail);
    RUN_TEST(test_resp_complete_unit);
    return UNITY_END();
}
