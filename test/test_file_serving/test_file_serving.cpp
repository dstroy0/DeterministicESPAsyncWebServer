// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for serve_file().
//
// Tests verify that:
//   - Missing file → 404
//   - Existing file → 200 with correct Content-Type and Content-Length
//   - File body is streamed to tcp_write
//   - Content-Length matches file size exactly
//   - Multiple content types are handled correctly
//   - Empty file → 200 with Content-Length: 0

#include "FS.h"
#include "dwserver.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DWS server;
static bool handler_called = false;

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

static void handle_html(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    handler_called = true;
    fs::FS fs;
    server.serve_file(slot_id, fs, "/index.html", "text/html");
}

static void handle_js(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    handler_called = true;
    fs::FS fs;
    server.serve_file(slot_id, fs, "/app.js", "application/javascript");
}

static void handle_missing(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    handler_called = true;
    fs::FS fs;
    server.serve_file(slot_id, fs, "/missing.txt", "text/plain");
}

void setUp()
{
    server = DWS();
    handler_called = false;

    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = ConnState::CONN_ACTIVE;
        conn_pool[i].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    sse_init();

    fs::mock_fs_clear();
    tcp_capture_reset();
}

void tearDown()
{
    tcp_capture_disable();
    fs::mock_fs_clear();
}

// ---------------------------------------------------------------------------
// Helper: feed a complete HTTP request and drive handle()
// ---------------------------------------------------------------------------
static void feed_and_handle(uint8_t slot, const char *req_str)
{
    push_str(slot, req_str);
    http_parse(slot);
    server.handle();
}

// ====================================================================
// UNIT TESTS
// ====================================================================

void test_missing_file_returns_404()
{
    server.on("/page", HttpMethod::HTTP_GET, handle_missing);
    fs::mock_fs_clear(); // no file set
    feed_and_handle(0, "GET /page HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "404"));
}

void test_existing_file_returns_200()
{
    server.on("/page", HttpMethod::HTTP_GET, handle_html);
    fs::mock_fs_set("<html><body>Hello</body></html>");
    feed_and_handle(0, "GET /page HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "200 OK"));
}

void test_response_includes_content_type_html()
{
    server.on("/page", HttpMethod::HTTP_GET, handle_html);
    fs::mock_fs_set("<html></html>");
    feed_and_handle(0, "GET /page HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Content-Type: text/html"));
}

void test_response_includes_content_type_js()
{
    server.on("/app", HttpMethod::HTTP_GET, handle_js);
    fs::mock_fs_set("console.log('hello');");
    feed_and_handle(0, "GET /app HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Content-Type: application/javascript"));
}

void test_content_length_matches_file_size()
{
    server.on("/page", HttpMethod::HTTP_GET, handle_html);
    const char *body = "Hello, World!";
    fs::mock_fs_set(body);
    size_t expected_len = strlen(body);

    feed_and_handle(0, "GET /page HTTP/1.1\r\n\r\n");

    char expected_cl[64];
    snprintf(expected_cl, sizeof(expected_cl), "Content-Length: %u", (unsigned)expected_len);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), expected_cl));
}

void test_file_body_is_sent()
{
    server.on("/page", HttpMethod::HTTP_GET, handle_html);
    const char *body = "<h1>Test Page</h1>";
    fs::mock_fs_set(body);
    feed_and_handle(0, "GET /page HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), body));
}

void test_empty_file_returns_200_with_zero_length()
{
    server.on("/empty", HttpMethod::HTTP_GET, [](uint8_t slot_id, HttpReq *req) {
        (void)req;
        fs::FS fs;
        server.serve_file(slot_id, fs, "/empty.txt", "text/plain");
    });
    uint8_t zero_data[] = {};
    fs::mock_fs_set(zero_data, 0);

    feed_and_handle(0, "GET /empty HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Content-Length: 0"));
}

void test_large_file_body_fully_sent()
{
    // A body far larger than one send-buffer window: the cross-loop file pump must
    // deliver every byte, not truncate at the window. (The host mock never returns
    // ERR_MEM, so this guards the pump's body-length accounting; the real TCP
    // send-window paging is verified on hardware.)
    static const size_t N = 16000;
    static uint8_t big[N];
    for (size_t i = 0; i < N; i++)
        big[i] = (uint8_t)('A' + (i % 26)); // printable, no NUL, position-dependent

    server.on("/big", HttpMethod::HTTP_GET, [](uint8_t slot_id, HttpReq *req) {
        (void)req;
        fs::FS fs;
        server.serve_file(slot_id, fs, "/big.bin", "application/octet-stream");
    });
    fs::mock_fs_set(big, N);

    feed_and_handle(0, "GET /big HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "200 OK"));

    char expected_cl[64];
    snprintf(expected_cl, sizeof(expected_cl), "Content-Length: %u", (unsigned)N);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), expected_cl));

    // The whole body must be present after the header boundary, byte-exact.
    const char *cap = tcp_captured();
    const char *body = strstr(cap, "\r\n\r\n");
    TEST_ASSERT_NOT_NULL(body);
    body += 4;
    size_t body_len = tcp_captured_len() - (size_t)(body - cap);
    TEST_ASSERT_EQUAL_size_t(N, body_len); // no truncation
    for (size_t i = 0; i < N; i++)
        TEST_ASSERT_EQUAL_UINT8((uint8_t)('A' + (i % 26)), (uint8_t)body[i]);
}

void test_serve_file_does_not_affect_other_routes()
{
    static bool other_called = false;
    server.on("/other", HttpMethod::HTTP_GET, [](uint8_t slot_id, HttpReq *req) {
        (void)req;
        other_called = true;
        server.send(slot_id, 200, "text/plain", "other");
    });
    server.on("/file", HttpMethod::HTTP_GET, handle_html);

    fs::mock_fs_set("<html/>");
    feed_and_handle(0, "GET /other HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(other_called);
    TEST_ASSERT_FALSE(handler_called);
}

void test_multiple_content_types()
{
    static const struct
    {
        const char *path;
        const char *ctype;
        const char *body;
    } cases[] = {
        {"/page.html", "text/html", "<html/>"},
        {"/style.css", "text/css", "body{}"},
        {"/data.json", "application/json", "{}"},
        {"/app.js", "text/javascript", "var x=1;"},
    };

    static const char *cur_ctype = nullptr;
    static const char *cur_path = nullptr;

    for (size_t i = 0; i < 4; i++)
    {
        cur_ctype = cases[i].ctype;
        cur_path = cases[i].path;

        server = DWS();
        conn_pool[0] = {};
        conn_pool[0].id = 0;
        conn_pool[0].state = ConnState::CONN_ACTIVE;
        conn_pool[0].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[0].pcb = &_mock_pcb;
        http_reset(0);
        tcp_capture_reset();

        server.on(cur_path, HttpMethod::HTTP_GET, [](uint8_t slot_id, HttpReq *req) {
            (void)req;
            fs::FS fs;
            server.serve_file(slot_id, fs, cur_path, cur_ctype);
        });

        fs::mock_fs_set(cases[i].body);
        char req_str[128];
        snprintf(req_str, sizeof(req_str), "GET %s HTTP/1.1\r\n\r\n", cases[i].path);
        feed_and_handle(0, req_str);

        TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), "200 OK"), "expected 200 OK");
        TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), cases[i].ctype), "expected content-type in response");
    }
}

// ====================================================================
// STRESS TESTS
// ====================================================================

void stress_serve_file_50_requests()
{
    const char *body = "stress body";
    fs::mock_fs_set(body);
    server.on("/f", HttpMethod::HTTP_GET, handle_html);

    for (int i = 0; i < 50; i++)
    {
        uint8_t slot = (uint8_t)(i % MAX_CONNS);
        conn_pool[slot] = {};
        conn_pool[slot].id = slot;
        conn_pool[slot].state = ConnState::CONN_ACTIVE;
        conn_pool[slot].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[slot].pcb = &_mock_pcb;
        http_reset(slot);
        tcp_capture_reset();
        handler_called = false;

        push_str(slot, "GET /f HTTP/1.1\r\n\r\n");
        http_parse(slot);
        server.handle();

        TEST_ASSERT_TRUE_MESSAGE(handler_called, "handler not called");
        TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), "200 OK"), "not 200");
        TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), body), "body missing");
    }
}

void stress_alternate_missing_and_found()
{
    server.on("/f", HttpMethod::HTTP_GET, [](uint8_t slot_id, HttpReq *req) {
        (void)req;
        fs::FS fs;
        server.serve_file(slot_id, fs, "/f.txt", "text/plain");
    });

    for (int i = 0; i < 40; i++)
    {
        uint8_t slot = (uint8_t)(i % MAX_CONNS);
        conn_pool[slot] = {};
        conn_pool[slot].id = slot;
        conn_pool[slot].state = ConnState::CONN_ACTIVE;
        conn_pool[slot].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[slot].pcb = &_mock_pcb;
        http_reset(slot);
        tcp_capture_reset();

        if (i % 2 == 0)
            fs::mock_fs_set("content");
        else
            fs::mock_fs_clear();

        push_str(slot, "GET /f HTTP/1.1\r\n\r\n");
        http_parse(slot);
        server.handle();

        if (i % 2 == 0)
            TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), "200"), "expected 200");
        else
            TEST_ASSERT_NOT_NULL_MESSAGE(strstr(tcp_captured(), "404"), "expected 404");
    }
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_missing_file_returns_404);
    RUN_TEST(test_existing_file_returns_200);
    RUN_TEST(test_response_includes_content_type_html);
    RUN_TEST(test_response_includes_content_type_js);
    RUN_TEST(test_content_length_matches_file_size);
    RUN_TEST(test_file_body_is_sent);
    RUN_TEST(test_empty_file_returns_200_with_zero_length);
    RUN_TEST(test_large_file_body_fully_sent);
    RUN_TEST(test_serve_file_does_not_affect_other_routes);
    RUN_TEST(test_multiple_content_types);
    RUN_TEST(stress_serve_file_50_requests);
    RUN_TEST(stress_alternate_missing_and_found);

    return UNITY_END();
}
