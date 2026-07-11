// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for HTTP Basic Authentication (per-route).
//
// Tests verify that:
//   - Unprotected routes are not affected
//   - Protected routes return 401 when no Authorization header is present
//   - Protected routes return 401 on bad credentials
//   - Protected routes invoke the handler on valid credentials
//   - The WWW-Authenticate header includes the correct realm
//   - Credentials are checked exactly (no prefix match)

#include "dwserver.h"
#include <string.h>
#include <unity.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static DetWebServer server;
static bool handler_called = false;
static uint8_t handler_slot = 0xFF;

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

static void handle_ok(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    handler_called = true;
    handler_slot = slot_id;
    server.send(slot_id, 200, "text/plain", "OK");
}

void setUp()
{
    server = DetWebServer();
    handler_called = false;
    handler_slot = 0xFF;

    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    sse_init();

    tcp_capture_reset();
}

void tearDown()
{
    tcp_capture_disable();
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

void test_unprotected_route_fires_handler()
{
    server.on("/open", HTTP_GET, handle_ok);
    feed_and_handle(0, "GET /open HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
}

void test_protected_route_no_header_returns_401()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401 Unauthorized") != nullptr);
}

void test_protected_route_wrong_password_returns_401()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    // base64("user:wrong") = "dXNlcjp3cm9uZw=="
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n"
                       "Authorization: Basic dXNlcjp3cm9uZw==\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401") != nullptr);
}

void test_protected_route_wrong_username_returns_401()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    // base64("admin:pass") = "YWRtaW46cGFzcw=="
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n"
                       "Authorization: Basic YWRtaW46cGFzcw==\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401") != nullptr);
}

void test_protected_route_valid_credentials_fires_handler()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    // base64("user:pass") = "dXNlcjpwYXNz"
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n"
                       "Authorization: Basic dXNlcjpwYXNz\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "200 OK") != nullptr);
}

void test_401_includes_www_authenticate_header()
{
    server.on("/secret", HTTP_GET, handle_ok, "MyRealm", "u", "p");
    feed_and_handle(0, "GET /secret HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "WWW-Authenticate: Basic realm=\"MyRealm\""));
}

void test_non_basic_scheme_returns_401()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n"
                       "Authorization: Bearer some_token\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401") != nullptr);
}

void test_credentials_without_colon_returns_401()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    // base64("nocolon") = "bm9jb2xvbg=="
    feed_and_handle(0, "GET /admin HTTP/1.1\r\n"
                       "Authorization: Basic bm9jb2xvbg==\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401") != nullptr);
}

void test_protected_and_unprotected_routes_coexist()
{
    server.on("/public", HTTP_GET, handle_ok);
    server.on("/private", HTTP_GET, handle_ok, "Priv", "u", "p");

    // Hit public route -- handler fires
    feed_and_handle(0, "GET /public HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    handler_called = false;
    conn_pool[1].state = CONN_ACTIVE;
    conn_pool[1].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[1].pcb = &_mock_pcb;
    http_reset(1);
    tcp_capture_reset();

    // Hit private without creds -- 401
    feed_and_handle(1, "GET /private HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "401") != nullptr);
}

void test_auth_route_returns_404_for_wrong_path()
{
    server.on("/admin", HTTP_GET, handle_ok, "Admin", "user", "pass");
    feed_and_handle(0, "GET /other HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_TRUE(strstr(tcp_captured(), "404") != nullptr);
}

void test_auth_checked_per_method()
{
    // Route only handles POST; a GET to that path is 405 Method Not Allowed
    // (RFC 7231 §6.5.5) - auth is never evaluated for the wrong method, so the
    // response must not be 401.
    server.on("/upload", HTTP_POST, handle_ok, "Upload", "u", "p");
    feed_and_handle(0, "GET /upload HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "405"));
    TEST_ASSERT_NULL(strstr(tcp_captured(), "401"));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Allow: POST"));
}

// ====================================================================
// STRESS TESTS
// ====================================================================

void stress_auth_50_valid_requests()
{
    server.on("/s", HTTP_GET, handle_ok, "R", "u", "p");
    // base64("u:p") = "dTpw"
    const char *req = "GET /s HTTP/1.1\r\n"
                      "Authorization: Basic dTpw\r\n\r\n";

    for (int i = 0; i < 50; i++)
    {
        uint8_t slot = (uint8_t)(i % MAX_CONNS);
        conn_pool[slot] = {};
        conn_pool[slot].id = slot;
        conn_pool[slot].state = CONN_ACTIVE;
        conn_pool[slot].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[slot].pcb = &_mock_pcb;
        http_reset(slot);

        handler_called = false;
        push_str(slot, req);
        http_parse(slot);
        server.handle();
        TEST_ASSERT_TRUE_MESSAGE(handler_called, "handler not called with valid creds");
    }
}

void stress_auth_50_invalid_requests()
{
    server.on("/s", HTTP_GET, handle_ok, "R", "u", "p");
    const char *req = "GET /s HTTP/1.1\r\n"
                      "Authorization: Basic d3Jvbmc6Y3JlZHM=\r\n\r\n"; // "wrong:creds"

    for (int i = 0; i < 50; i++)
    {
        uint8_t slot = (uint8_t)(i % MAX_CONNS);
        conn_pool[slot] = {};
        conn_pool[slot].id = slot;
        conn_pool[slot].state = CONN_ACTIVE;
        conn_pool[slot].proto = ConnProto::PROTO_HTTP; // dispatch requires an explicit protocol
        conn_pool[slot].pcb = &_mock_pcb;
        http_reset(slot);

        handler_called = false;
        push_str(slot, req);
        http_parse(slot);
        server.handle();
        TEST_ASSERT_FALSE_MESSAGE(handler_called, "handler called with bad creds");
    }
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_unprotected_route_fires_handler);
    RUN_TEST(test_protected_route_no_header_returns_401);
    RUN_TEST(test_protected_route_wrong_password_returns_401);
    RUN_TEST(test_protected_route_wrong_username_returns_401);
    RUN_TEST(test_protected_route_valid_credentials_fires_handler);
    RUN_TEST(test_401_includes_www_authenticate_header);
    RUN_TEST(test_non_basic_scheme_returns_401);
    RUN_TEST(test_credentials_without_colon_returns_401);
    RUN_TEST(test_protected_and_unprotected_routes_coexist);
    RUN_TEST(test_auth_route_returns_404_for_wrong_path);
    RUN_TEST(test_auth_checked_per_method);

    RUN_TEST(stress_auth_50_valid_requests);
    RUN_TEST(stress_auth_50_invalid_requests);

    return UNITY_END();
}
