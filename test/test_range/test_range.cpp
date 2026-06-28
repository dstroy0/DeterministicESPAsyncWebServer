// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// HTTP Range requests / 206 Partial Content (DETWS_ENABLE_RANGE). Each test
// serves a known 20-byte file via serve_file() with a Range header and checks
// the status line, Content-Range / Content-Length headers, and the exact bytes
// returned (via the tcp_write capture mock).

#include "DeterministicESPAsyncWebServer.h"
#include "FS.h"
#include <string.h>
#include <unity.h>

static DetWebServer server;
static const char FILE_DATA[] = "0123456789ABCDEFGHIJ"; // 20 bytes

static void push_str(uint8_t slot, const char *s)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; s[i]; i++)
    {
        c->rx_buffer[c->rx_head] = (uint8_t)s[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}

static void serve_data(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    fs::FS fs;
    server.serve_file(slot_id, fs, "/data.bin", "application/octet-stream");
}

void setUp()
{
    server = DetWebServer();
    server.on("/data", HTTP_GET, serve_data);
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
    fs::mock_fs_reset();
    fs::mock_fs_set(FILE_DATA); // 20-byte file at any path
    tcp_capture_reset();
}

void tearDown()
{
    tcp_capture_disable();
}

// Return a pointer to the response body (after the header terminator), or nullptr.
static const char *body_ptr()
{
    const char *sep = strstr(tcp_captured(), "\r\n\r\n");
    return sep ? sep + 4 : nullptr;
}

static size_t body_len()
{
    const char *b = body_ptr();
    if (!b)
        return 0;
    return tcp_captured_len() - (size_t)(b - tcp_captured());
}

static void request(const char *range_hdr)
{
    char req[128];
    if (range_hdr)
        snprintf(req, sizeof(req), "GET /data HTTP/1.1\r\nRange: %s\r\n\r\n", range_hdr);
    else
        snprintf(req, sizeof(req), "GET /data HTTP/1.1\r\n\r\n");
    push_str(0, req);
    http_parse(0);
    server.handle();
}

// ---------------------------------------------------------------------------

void test_no_range_full_200()
{
    request(nullptr);
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Accept-Ranges: bytes"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Length: 20"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Connection: close")); // header must carry the Connection line
    TEST_ASSERT_NULL(strstr(r, "Content-Range"));
    TEST_ASSERT_EQUAL_UINT(20, body_len());
    TEST_ASSERT_EQUAL_MEMORY(FILE_DATA, body_ptr(), 20);
}

void test_range_prefix()
{
    request("bytes=0-3");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes 0-3/20"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Length: 4"));
    TEST_ASSERT_EQUAL_UINT(4, body_len());
    TEST_ASSERT_EQUAL_MEMORY("0123", body_ptr(), 4);
}

void test_range_open_ended()
{
    request("bytes=5-");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes 5-19/20"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Length: 15"));
    TEST_ASSERT_EQUAL_UINT(15, body_len());
    TEST_ASSERT_EQUAL_MEMORY("56789ABCDEFGHIJ", body_ptr(), 15);
}

void test_range_suffix()
{
    request("bytes=-4");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes 16-19/20"));
    TEST_ASSERT_EQUAL_UINT(4, body_len());
    TEST_ASSERT_EQUAL_MEMORY("GHIJ", body_ptr(), 4);
}

void test_range_single_byte()
{
    request("bytes=2-2");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Content-Range: bytes 2-2/20"));
    TEST_ASSERT_EQUAL_UINT(1, body_len());
    TEST_ASSERT_EQUAL_MEMORY("2", body_ptr(), 1);
}

void test_range_clamped_to_eof()
{
    request("bytes=10-999");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Content-Range: bytes 10-19/20"));
    TEST_ASSERT_EQUAL_UINT(10, body_len());
    TEST_ASSERT_EQUAL_MEMORY("ABCDEFGHIJ", body_ptr(), 10);
}

void test_range_unsatisfiable_416()
{
    request("bytes=100-200");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "416 Range Not Satisfiable"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes */20"));
    TEST_ASSERT_NULL(strstr(r, "206"));
}

void test_malformed_range_ignored()
{
    request("bytes=abc");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK")); // malformed -> full response
    TEST_ASSERT_EQUAL_UINT(20, body_len());
}

void test_multirange_falls_back_to_200()
{
    request("bytes=0-1,5-6");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK")); // multi-range unsupported -> full 200
    TEST_ASSERT_NULL(strstr(r, "206"));
    TEST_ASSERT_EQUAL_UINT(20, body_len());
}

// A start that overflows size_t must not wrap to a small in-range value -> 416, never
// a corrupt 206. (Saturating accumulator in parse_byte_range.)
void test_range_overflow_start_unsatisfiable()
{
    request("bytes=99999999999999999999999-");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "416 Range Not Satisfiable"));
    TEST_ASSERT_NULL(strstr(r, "206"));
}

// An overflowing end clamps to the last byte (full 206), not a wrapped window.
void test_range_overflow_end_clamps()
{
    request("bytes=0-99999999999999999999999");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes 0-19/20"));
}

// Suffix "bytes=-0" requests the last zero bytes -> unsatisfiable (416).
void test_range_suffix_zero_unsatisfiable()
{
    request("bytes=-0");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "416 Range Not Satisfiable"));
    TEST_ASSERT_NULL(strstr(r, "206"));
}

void test_head_with_range_no_body()
{
    push_str(0, "HEAD /data HTTP/1.1\r\nRange: bytes=0-3\r\n\r\n");
    http_parse(0);
    server.handle();
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "206 Partial Content"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Range: bytes 0-3/20"));
    TEST_ASSERT_NOT_NULL(strstr(r, "Content-Length: 4"));
    TEST_ASSERT_EQUAL_UINT(0, body_len()); // HEAD: headers only
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_no_range_full_200);
    RUN_TEST(test_range_prefix);
    RUN_TEST(test_range_open_ended);
    RUN_TEST(test_range_suffix);
    RUN_TEST(test_range_single_byte);
    RUN_TEST(test_range_clamped_to_eof);
    RUN_TEST(test_range_unsatisfiable_416);
    RUN_TEST(test_malformed_range_ignored);
    RUN_TEST(test_range_overflow_start_unsatisfiable);
    RUN_TEST(test_range_overflow_end_clamps);
    RUN_TEST(test_range_suffix_zero_unsatisfiable);
    RUN_TEST(test_multirange_falls_back_to_200);
    RUN_TEST(test_head_with_range_no_body);
    return UNITY_END();
}
