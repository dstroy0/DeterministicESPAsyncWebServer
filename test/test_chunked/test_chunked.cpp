// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for send_chunked() / ChunkedResponse streaming responses.

#include "dwserver.h"
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

// ---- Chunk sources (pull generators) ---------------------------------------
// Each returns the next body piece (one chunk per call) and 0 to end. A single
// global cursor is enough here: tests are sequential on slot 0, reset in setUp().

static int g_step;

static size_t src_hello(uint8_t *buf, size_t cap, void *ctx)
{
    (void)cap;
    (void)ctx;
    if (g_step++ == 0)
    {
        memcpy(buf, "hello", 5);
        return 5;
    }
    return 0;
}
static size_t src_multi(uint8_t *buf, size_t cap, void *ctx)
{
    (void)cap;
    (void)ctx;
    if (g_step == 0)
    {
        g_step = 1;
        memcpy(buf, "ab", 2);
        return 2;
    }
    if (g_step == 1)
    {
        g_step = 2;
        memcpy(buf, "cdef", 4);
        return 4;
    }
    return 0;
}
static size_t src_printf(uint8_t *buf, size_t cap, void *ctx)
{
    (void)ctx;
    if (g_step++ == 0)
        return (size_t)snprintf((char *)buf, cap, "x=%d", 42);
    return 0;
}
static size_t src_ok(uint8_t *buf, size_t cap, void *ctx)
{
    (void)cap;
    (void)ctx;
    if (g_step++ == 0)
    {
        memcpy(buf, "ok", 2);
        return 2;
    }
    return 0;
}
static size_t src_empty(uint8_t *buf, size_t cap, void *ctx)
{
    (void)buf;
    (void)cap;
    (void)ctx;
    return 0; // empty body: just the terminating chunk
}
static size_t src_two5(uint8_t *buf, size_t cap, void *ctx)
{
    (void)cap;
    (void)ctx;
    if (g_step == 0)
    {
        g_step = 1;
        memcpy(buf, "hello", 5);
        return 5;
    }
    if (g_step == 1)
    {
        g_step = 2;
        memcpy(buf, "world", 5);
        return 5;
    }
    return 0;
}
// Large body: emit BIG_TOTAL bytes of a position-dependent pattern, cap bytes per
// call, to exercise paging a body far past one send window without truncation.
static const int BIG_TOTAL = 16000;
static size_t src_big(uint8_t *buf, size_t cap, void *ctx)
{
    (void)ctx;
    size_t produced = (size_t)g_step;
    if (produced >= (size_t)BIG_TOTAL)
        return 0;
    size_t n = (size_t)BIG_TOTAL - produced;
    if (n > cap)
        n = cap;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint8_t)('A' + ((produced + i) % 26));
    g_step += (int)n;
    return n;
}

// ---- Handlers --------------------------------------------------------------

static void h_hello(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_hello);
}
static void h_multi(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_multi);
}
static void h_printf(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_printf);
}
static void h_ok(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_ok);
}
static void h_empty(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_empty);
}
static void h_two5(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "text/plain", src_two5);
}
static void h_big(uint8_t s, HttpReq *r)
{
    (void)r;
    server.send_chunked(s, 200, "application/octet-stream", src_big);
}
static void h_with_hdr(uint8_t s, HttpReq *r)
{
    (void)r;
    server.add_response_header(s, "X-Stream", "1");
    server.send_chunked(s, 200, "text/plain", src_hello);
}

void setUp()
{
    server = DetWebServer();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].proto = PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    sse_init();
    tcp_capture_reset();
    g_log_status = 0;
    g_log_len = -1;
    g_step = 0;
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

void test_single_piece_then_terminator()
{
    server.on("/c", HTTP_GET, h_ok);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    // One piece "ok" then the terminator; nothing extra.
    TEST_ASSERT_NOT_NULL(strstr(r, "2\r\nok\r\n0\r\n\r\n"));
}

void test_empty_body_is_just_terminator()
{
    server.on("/c", HTTP_GET, h_empty);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    const char *body = strstr(r, "\r\n\r\n"); // end of headers
    TEST_ASSERT_NOT_NULL(body);
    // A source that returns 0 immediately yields only the terminating chunk.
    TEST_ASSERT_EQUAL_STRING("\r\n\r\n0\r\n\r\n", body);
}

void test_large_chunked_body_not_truncated()
{
    server.on_request_log(log_cb);
    server.on("/c", HTTP_GET, h_big);
    feed_and_handle(0, "GET /c HTTP/1.1\r\n\r\n");
    const char *r = tcp_captured();
    // The whole 16000-byte body must page out (paced across the window) and finish
    // with the terminator; the log hook reports the full body length, no truncation.
    TEST_ASSERT_NOT_NULL(strstr(r, "0\r\n\r\n"));
    TEST_ASSERT_EQUAL_INT(200, g_log_status);
    TEST_ASSERT_EQUAL_INT(BIG_TOTAL, g_log_len);
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

// RFC 7230 3.3.1: chunked must not be sent to an HTTP/1.0 client. The same
// streaming handler falls back to a close-delimited body: no Transfer-Encoding,
// Connection: close, and the raw bytes with no chunk framing or terminator.
void test_http10_falls_back_to_close_delimited()
{
    server.on("/c", HTTP_GET, h_multi);
    feed_and_handle(0, "GET /c HTTP/1.0\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK"));
    TEST_ASSERT_NULL(strstr(r, "Transfer-Encoding")); // never chunked to a 1.0 client
    TEST_ASSERT_NOT_NULL(strstr(r, "Connection: close\r\n"));
    // Body bytes verbatim, no "2\r\n.."/"0\r\n\r\n" framing.
    const char *body = strstr(r, "\r\n\r\n");
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_STRING("\r\n\r\nabcdef", body);
}

// The large-body pager must also page a close-delimited HTTP/1.0 stream in full.
void test_http10_large_body_not_truncated()
{
    server.on_request_log(log_cb);
    server.on("/c", HTTP_GET, h_big);
    feed_and_handle(0, "GET /c HTTP/1.0\r\n\r\n");
    TEST_ASSERT_NULL(strstr(tcp_captured(), "Transfer-Encoding"));
    TEST_ASSERT_EQUAL_INT(200, g_log_status);
    TEST_ASSERT_EQUAL_INT(BIG_TOTAL, g_log_len); // full body paged out, none dropped
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_headers_announce_chunked_no_content_length);
    RUN_TEST(test_single_chunk_framing);
    RUN_TEST(test_multiple_chunks_in_order);
    RUN_TEST(test_printf_chunk);
    RUN_TEST(test_single_piece_then_terminator);
    RUN_TEST(test_empty_body_is_just_terminator);
    RUN_TEST(test_large_chunked_body_not_truncated);
    RUN_TEST(test_head_sends_headers_only);
    RUN_TEST(test_custom_header_injected_into_chunked);
    RUN_TEST(test_log_hook_reports_total_body_length);
    RUN_TEST(test_http10_falls_back_to_close_delimited);
    RUN_TEST(test_http10_large_body_not_truncated);
    return UNITY_END();
}
