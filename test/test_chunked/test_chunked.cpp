// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for send_chunked() / ChunkedResponse streaming responses.

#include "DeterministicESPAsyncWebServer.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DetWebServer server;

static int g_log_status;
static int g_log_len;
static void log_cb(const char *method, const char *path, int status, int body_len)
{
    (void)method;
    (void)path;
    g_log_status = status;
    g_log_len = body_len;
}

static void push_str(uint8_t slot, const char *s)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; s[i]; i++)
    {
        size_t next = (c->rx_head + 1) % RX_BUF_SIZE;
        c->rx_buffer[c->rx_head] = (uint8_t)s[i];
        c->rx_head = next;
    }
}

// ---- Chunk fillers ---------------------------------------------------------

static void f_hello(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("hello");
}
static void f_multi(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("ab");
    res.write("cdef");
}
static void f_printf(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.printf("x=%d", 42);
}
static void f_empty(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("");         // no-op
    res.write(nullptr, 0); // no-op
    res.write("ok");
}
static void f_two5(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("hello");
    res.write("world");
}

// ---- Handlers --------------------------------------------------------------

static void h_hello(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", f_hello);
}
static void h_multi(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", f_multi);
}
static void h_printf(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", f_printf);
}
static void h_empty(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", f_empty);
}
static void h_two5(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", f_two5);
}
static void h_with_hdr(uint8_t s, HttpReq *r)
{
    (void)r;
    server.add_response_header(s, "X-Stream", "1");
    server.send_chunked(s, 200, "text/plain", f_hello);
}

void setUp()
{
    server = DetWebServer();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    sse_init();
    tcp_capture_reset();
    g_log_status = 0;
    g_log_len = -1;
}

void tearDown()
{
    tcp_capture_disable();
}

static void feed_and_handle(uint8_t slot, const char *req_str)
{
    push_str(slot, req_str);
    http_parse(slot);
    server.handle();
}

// ====================================================================
// TESTS
// ====================================================================

void test_headers_announce_chunked_no_content_length()
{
    server.on("/c", HTTP_GET, h_hello);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Transfer-Encoding: chunked\r\n"));
    TEST_ASSERT_NULL(strstr(r, "Content-Length")); // mutually exclusive with chunked
}

void test_single_chunk_framing()
{
    server.on("/c", HTTP_GET, h_hello);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    // "hello" = 5 bytes -> "5\r\nhello\r\n" then the terminating "0\r\n\r\n".
    TEST_ASSERT_NOT_NULL(strstr(r, "5\r\nhello\r\n0\r\n\r\n"));
}

void test_multiple_chunks_in_order()
{
    server.on("/c", HTTP_GET, h_multi);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "2\r\nab\r\n4\r\ncdef\r\n0\r\n\r\n"));
}

void test_printf_chunk()
{
    server.on("/c", HTTP_GET, h_printf);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "4\r\nx=42\r\n0\r\n\r\n"));
}

void test_empty_writes_do_not_terminate_early()
{
    server.on("/c", HTTP_GET, h_empty);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    // The empty / null writes must emit nothing; only "ok" then the terminator.
    TEST_ASSERT_NOT_NULL(strstr(r, "2\r\nok\r\n0\r\n\r\n"));
}

void test_head_sends_headers_only()
{
    server.on("/c", HTTP_GET, h_hello);
    feed_and_handle(0, "HEAD /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "Transfer-Encoding: chunked\r\n"));
    TEST_ASSERT_NULL(strstr(r, "hello"));     // no body
    TEST_ASSERT_NULL(strstr(r, "0\r\n\r\n")); // no terminating chunk
}

void test_custom_header_injected_into_chunked()
{
    server.on("/c", HTTP_GET, h_with_hdr);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "X-Stream: 1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(r, "5\r\nhello\r\n"));
}

void test_log_hook_reports_total_body_length()
{
    server.on_request_log(log_cb);
    server.on("/c", HTTP_GET, h_two5);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL_INT(200, g_log_status);
    TEST_ASSERT_EQUAL_INT(10, g_log_len); // "hello" + "world", framing excluded
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_headers_announce_chunked_no_content_length);
    RUN_TEST(test_single_chunk_framing);
    RUN_TEST(test_multiple_chunks_in_order);
    RUN_TEST(test_printf_chunk);
    RUN_TEST(test_empty_writes_do_not_terminate_early);
    RUN_TEST(test_head_sends_headers_only);
    RUN_TEST(test_custom_header_injected_into_chunked);
    RUN_TEST(test_log_hook_reports_total_body_length);
    return UNITY_END();
}
