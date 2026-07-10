// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the TCP relay / DNAT byte pump (services/relay): bidirectional transfer, the
// backpressure carry (a send that accepts partial writes), independent half-close with shutdown
// propagation, a large multi-step transfer (byte-exact), and a seam error. Two mock sockets stand
// in for the inbound and origin connections.

#include "services/relay/relay.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A mock socket: `in` is what the relay reads from this peer; `out` is what the relay writes to it.
struct MockSock
{
    uint8_t in[4096];
    size_t in_len, in_pos;
    bool in_eof; // recv returns <0 (EOF) once `in` is drained
    uint8_t out[4096];
    size_t out_len;
    size_t send_cap; // max bytes a single send accepts (0 = unlimited) - drives backpressure
    bool fail_send;
    bool shutdown_called;
};

static int msock_recv(void *c, uint8_t *buf, size_t cap)
{
    MockSock *s = (MockSock *)c;
    if (s->in_pos < s->in_len)
    {
        size_t n = s->in_len - s->in_pos;
        if (n > cap)
            n = cap;
        memcpy(buf, s->in + s->in_pos, n);
        s->in_pos += n;
        return (int)n;
    }
    return s->in_eof ? -1 : 0;
}

static int msock_send(void *c, const uint8_t *buf, size_t len)
{
    MockSock *s = (MockSock *)c;
    if (s->fail_send)
        return -1;
    size_t n = len;
    if (s->send_cap && n > s->send_cap)
        n = s->send_cap;
    if (s->out_len + n > sizeof(s->out))
        n = sizeof(s->out) - s->out_len;
    memcpy(s->out + s->out_len, buf, n);
    s->out_len += n;
    return (int)n;
}

static void msock_shutdown(void *c)
{
    ((MockSock *)c)->shutdown_called = true;
}

static void sock_init(MockSock *s, const void *in, size_t in_len, bool eof)
{
    memset(s, 0, sizeof(*s));
    if (in_len)
        memcpy(s->in, in, in_len);
    s->in_len = in_len;
    s->in_eof = eof;
}

static DetRelayEnd end_of(MockSock *s)
{
    DetRelayEnd e;
    e.recv = msock_recv;
    e.send = msock_send;
    e.shutdown = msock_shutdown;
    e.ctx = s;
    return e;
}

static int run_relay(DetRelay *r, int max_steps)
{
    for (int i = 0; i < max_steps; i++)
    {
        int st = det_relay_step(r);
        if (st != DET_RELAY_RUNNING)
            return st;
    }
    return DET_RELAY_RUNNING; // never finished (a bug if it happens)
}

void test_bidirectional()
{
    MockSock a, b;
    sock_init(&a, "hello from client", 17, true);
    sock_init(&b, "hi from origin", 14, true);
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    TEST_ASSERT_EQUAL_INT(DET_RELAY_DONE, run_relay(&r, 64));
    TEST_ASSERT_EQUAL_size_t(17, b.out_len);
    TEST_ASSERT_EQUAL_MEMORY("hello from client", b.out, 17);
    TEST_ASSERT_EQUAL_size_t(14, a.out_len);
    TEST_ASSERT_EQUAL_MEMORY("hi from origin", a.out, 14);
    TEST_ASSERT_EQUAL_UINT32(17, r.bytes_a2b);
    TEST_ASSERT_EQUAL_UINT32(14, r.bytes_b2a);
}

void test_backpressure()
{
    uint8_t data[1000];
    for (int i = 0; i < 1000; i++)
        data[i] = (uint8_t)(i * 37 + 11);
    MockSock a, b;
    sock_init(&a, data, sizeof(data), true);
    sock_init(&b, nullptr, 0, true);
    b.send_cap = 7; // the origin accepts only 7 bytes per write
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    TEST_ASSERT_EQUAL_INT(DET_RELAY_DONE, run_relay(&r, 1000));
    TEST_ASSERT_EQUAL_size_t(1000, b.out_len);
    TEST_ASSERT_EQUAL_MEMORY(data, b.out, 1000); // every byte carried across, in order
}

void test_half_close_shutdown()
{
    uint8_t resp[800];
    for (int i = 0; i < 800; i++)
        resp[i] = (uint8_t)(i ^ 0x3C);
    MockSock a, b;
    sock_init(&a, "req", 3, true);           // client sends a short request then closes
    sock_init(&b, resp, sizeof(resp), true); // origin streams a long response
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    // once a->b finishes (client EOF) the origin's half-close must fire, while b->a is still
    // streaming its long response (proving the two directions close independently)
    int st = DET_RELAY_RUNNING;
    for (int i = 0; i < 10 && !r.b_shut_sent; i++)
        st = det_relay_step(&r);
    TEST_ASSERT_TRUE(r.b_shut_sent); // origin's shutdown fired on the client's FIN
    TEST_ASSERT_TRUE(b.shutdown_called);
    TEST_ASSERT_FALSE(r.b2a_done); // ...while the response direction is still open
    TEST_ASSERT_EQUAL_INT(DET_RELAY_RUNNING, st);

    TEST_ASSERT_EQUAL_INT(DET_RELAY_DONE, run_relay(&r, 64));
    TEST_ASSERT_EQUAL_MEMORY("req", b.out, 3);
    TEST_ASSERT_EQUAL_size_t(800, a.out_len);
    TEST_ASSERT_EQUAL_MEMORY(resp, a.out, 800);
    TEST_ASSERT_TRUE(a.shutdown_called); // the client's write side was closed too, once b EOF'd
}

void test_send_error()
{
    MockSock a, b;
    sock_init(&a, "data", 4, true);
    sock_init(&b, nullptr, 0, true);
    b.fail_send = true; // the origin's send errors
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    int st = DET_RELAY_RUNNING;
    for (int i = 0; i < 8 && st == DET_RELAY_RUNNING; i++)
        st = det_relay_step(&r);
    TEST_ASSERT_EQUAL_INT(DET_RELAY_ERROR, st);
}

void test_one_way_idle_then_close()
{
    // origin never sends; client sends then closes -> relay completes cleanly
    MockSock a, b;
    sock_init(&a, "GET / HTTP/1.0\r\n\r\n", 18, true);
    sock_init(&b, nullptr, 0, true);
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    TEST_ASSERT_EQUAL_INT(DET_RELAY_DONE, run_relay(&r, 32));
    TEST_ASSERT_EQUAL_size_t(18, b.out_len);
    TEST_ASSERT_EQUAL_size_t(0, a.out_len);
}

// A transport that signals close out of band (like det_conn's on_close) rather than via recv < 0:
// the mocks never EOF through recv; det_relay_note_eof() drives the finish.
void test_note_eof_out_of_band()
{
    MockSock a, b;
    sock_init(&a, "hello", 5, false); // in_eof=false: recv returns 0 (not -1) when drained
    sock_init(&b, "world", 5, false);
    DetRelayEnd ea = end_of(&a), eb = end_of(&b);
    DetRelay r;
    det_relay_init(&r, &ea, &eb);

    // one step moves the buffered data each way; without an EOF signal the relay keeps running
    TEST_ASSERT_EQUAL_INT(DET_RELAY_RUNNING, det_relay_step(&r));
    TEST_ASSERT_EQUAL_MEMORY("hello", b.out, 5);
    TEST_ASSERT_EQUAL_MEMORY("world", a.out, 5);

    // both peers close out of band -> the relay finishes and both shutdowns fire
    det_relay_note_eof(&r, false); // inbound closed
    det_relay_note_eof(&r, true);  // origin closed
    TEST_ASSERT_EQUAL_INT(DET_RELAY_DONE, run_relay(&r, 8));
    TEST_ASSERT_TRUE(a.shutdown_called);
    TEST_ASSERT_TRUE(b.shutdown_called);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_bidirectional);
    RUN_TEST(test_backpressure);
    RUN_TEST(test_half_close_shutdown);
    RUN_TEST(test_send_error);
    RUN_TEST(test_one_way_idle_then_close);
    RUN_TEST(test_note_eof_out_of_band);
    return UNITY_END();
}
