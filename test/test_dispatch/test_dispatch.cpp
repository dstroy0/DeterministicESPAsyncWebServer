// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Dispatch-level RFC 7231 compliance:
//   §6.5.2 - unrecognized method → 501 Not Implemented
//   §6.5.5 - known path, wrong method → 405 Method Not Allowed + Allow header

#include "dwserver.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>
#if DETWS_ENABLE_CSRF
#include "services/csrf/csrf.h" // supply a valid token so an unsafe method reaches method dispatch
#endif

static DetWebServer server;
static bool handler_called = false;

static void push_str(uint8_t slot, const char *s)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; s[i]; i++)
    {
        c->rx_buffer[c->rx_head] = (uint8_t)s[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}

static void handle_ok(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    handler_called = true;
    server.send(slot_id, 200, "text/plain", "OK");
}

void setUp()
{
    server = DetWebServer();
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
    tcp_capture_reset();
#if DETWS_ENABLE_CSRF
    // With CSRF compiled in, state-changing methods are gated before dispatch; set a secret so a valid
    // token can be issued (csrf_issue/verify no-op without one), letting the unsafe-method tests below
    // reach the 405/Allow method dispatch as a legitimate token-bearing client would.
    static const uint8_t csrf_key[16] = {0x53, 0x65, 0x63, 0x72, 0x65, 0x74, 0x4b, 0x65,
                                         0x79, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36};
    csrf_set_secret(csrf_key, sizeof(csrf_key));
#endif
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

// Feed an unsafe-method (POST/DELETE/...) request. Under DETWS_ENABLE_CSRF such a method is gated before
// dispatch, so attach a valid token to let the request reach the 405/Allow method dispatch (as a real
// token-bearing client would); with CSRF off the request line is plain.
static void feed_unsafe(uint8_t slot, const char *method, const char *path)
{
    char reqbuf[256];
#if DETWS_ENABLE_CSRF
    char tok[CSRF_TOKEN_BUF];
    csrf_issue(tok, sizeof(tok));
    snprintf(reqbuf, sizeof(reqbuf), "%s %s HTTP/1.1\r\nX-CSRF-Token: %s\r\n\r\n", method, path, tok);
#else
    snprintf(reqbuf, sizeof(reqbuf), "%s %s HTTP/1.1\r\n\r\n", method, path);
#endif
    feed_and_handle(slot, reqbuf);
}

// ---- §6.5.5 405 Method Not Allowed ----------------------------------------

void test_method_mismatch_returns_405()
{
    server.on("/res", HttpMethod::HTTP_POST, handle_ok);
    feed_and_handle(0, "GET /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "405 Method Not Allowed"));
}

void test_405_includes_allow_header()
{
    server.on("/res", HttpMethod::HTTP_POST, handle_ok);
    feed_unsafe(0, "DELETE", "/res");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "Allow: POST"));
}

void test_405_allow_lists_all_methods_for_path()
{
    server.on("/res", HttpMethod::HTTP_POST, handle_ok);
    server.on("/res", HttpMethod::HTTP_DELETE, handle_ok);
    feed_and_handle(0, "GET /res HTTP/1.1\r\n\r\n");
    const char *resp = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(resp, "405"));
    TEST_ASSERT_NOT_NULL(strstr(resp, "POST"));
    TEST_ASSERT_NOT_NULL(strstr(resp, "DELETE"));
}

void test_unknown_path_still_404_not_405()
{
    server.on("/res", HttpMethod::HTTP_POST, handle_ok);
    feed_and_handle(0, "GET /nope HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "404"));
}

// ---- §6.5.2 501 Not Implemented -------------------------------------------

void test_unknown_method_returns_501()
{
    server.on("/res", HttpMethod::HTTP_GET, handle_ok);
    feed_and_handle(0, "FOO /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "501 Not Implemented"));
}

void test_unknown_method_not_treated_as_get()
{
    // A bogus method must NOT run the GET handler (security: no method spoofing).
    server.on("/res", HttpMethod::HTTP_GET, handle_ok);
    feed_and_handle(0, "XGET /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
}

// ---- HEAD handling (RFC 7231 §4.3.2) --------------------------------------

void test_head_runs_get_handler_without_body()
{
    server.on("/res", HttpMethod::HTTP_GET, handle_ok);
    feed_and_handle(0, "HEAD /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called); // GET handler serves HEAD
    const char *resp = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(resp, "200 OK"));
    // Content-Length reflects the would-be GET body ("OK" = 2 bytes)...
    TEST_ASSERT_NOT_NULL(strstr(resp, "Content-Length: 2"));
    // ...but no body follows the header terminator.
    const char *sep = strstr(resp, "\r\n\r\n");
    TEST_ASSERT_NOT_NULL(sep);
    TEST_ASSERT_EQUAL_STRING("\r\n\r\n", sep);
}

void test_get_route_advertises_head_in_allow()
{
    server.on("/res", HttpMethod::HTTP_GET, handle_ok);
    feed_unsafe(0, "POST", "/res");
    const char *resp = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(resp, "405"));
    TEST_ASSERT_NOT_NULL(strstr(resp, "GET"));
    TEST_ASSERT_NOT_NULL(strstr(resp, "HEAD"));
}

void test_head_on_post_only_route_405()
{
    server.on("/res", HttpMethod::HTTP_POST, handle_ok);
    feed_and_handle(0, "HEAD /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_FALSE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "405"));
}

// ---- WebSocket handoff (regression) ---------------------------------------

#if DETWS_ENABLE_WEBSOCKET
// Once a slot has upgraded to WebSocket, http_parse() must not consume its rx
// bytes - they are WS frames the frame parser will drain. The event-queue
// dispatch used to call http_parse() on an upgraded slot and eat the first
// frame's header byte, which dropped the first connection after a reboot
// (the WS parser then read 0x80|len as opcode/RSV and failed the connection).
void test_http_parse_skips_ws_upgraded_slot()
{
    WsConn *ws = ws_alloc(2);
    TEST_ASSERT_NOT_NULL(ws);

    // A masked client TEXT frame: 0x81 0x85 <mask 4> <payload 5>.
    const uint8_t frame[] = {0x81, 0x85, 0x01, 0x02, 0x03, 0x04, 0x41, 0x43, 0x42, 0x45, 0x44};
    TcpConn *c = &conn_pool[2];
    for (size_t i = 0; i < sizeof(frame); i++)
    {
        c->rx_buffer[c->rx_head] = frame[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
    size_t tail_before = c->rx_tail;

    http_parse(2); // must be a no-op on a WS slot

    TEST_ASSERT_EQUAL_size_t(tail_before, c->rx_tail); // every WS byte preserved
    ws_free(2);
}
#endif

// ---- sanity ---------------------------------------------------------------

void test_correct_method_still_dispatches()
{
    server.on("/res", HttpMethod::HTTP_GET, handle_ok);
    feed_and_handle(0, "GET /res HTTP/1.1\r\n\r\n");
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "200 OK"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_method_mismatch_returns_405);
    RUN_TEST(test_405_includes_allow_header);
    RUN_TEST(test_405_allow_lists_all_methods_for_path);
    RUN_TEST(test_unknown_path_still_404_not_405);
    RUN_TEST(test_unknown_method_returns_501);
    RUN_TEST(test_unknown_method_not_treated_as_get);
    RUN_TEST(test_head_runs_get_handler_without_body);
    RUN_TEST(test_get_route_advertises_head_in_allow);
    RUN_TEST(test_head_on_post_only_route_405);
#if DETWS_ENABLE_WEBSOCKET
    RUN_TEST(test_http_parse_skips_ws_upgraded_slot);
#endif
    RUN_TEST(test_correct_method_still_dispatches);
    return UNITY_END();
}
