// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for bounded regex routes (DetWebServer::on_regex()).

#include "DeterministicESPAsyncWebServer.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DetWebServer server;
static bool g_called;

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

static void h_ok(uint8_t slot, HttpReq *req)
{
    (void)req;
    g_called = true;
    server.send(slot, 200, "text/plain", "ok");
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
    g_called = false;
}

void tearDown()
{
    tcp_capture_disable();
}

// Dispatch one request on a freshly armed slot 0; return whether the handler ran.
static bool hit(const char *method, const char *path)
{
    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[0].pcb = &_mock_pcb;
    http_reset(0);
    tcp_capture_reset();
    g_called = false;
    char req[160];
    snprintf(req, sizeof(req), "%s %s HTTP/1.1\r\n\r\n", method, path);
    push_str(0, req);
    http_parse(0);
    server.handle();
    return g_called;
}

// ====================================================================
// TESTS
// ====================================================================

void test_numeric_class_plus()
{
    server.on_regex("/sensor/[0-9]+", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/sensor/42"));
    TEST_ASSERT_TRUE(hit("GET", "/sensor/7"));
    TEST_ASSERT_FALSE(hit("GET", "/sensor/abc")); // non-digit
    TEST_ASSERT_FALSE(hit("GET", "/sensor/"));    // needs >=1 digit
    TEST_ASSERT_FALSE(hit("GET", "/sensor/4a"));  // trailing junk -> no full match
}

void test_dot_star_matches_rest()
{
    server.on_regex("/files/.*", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/files/")); // .* matches empty
    TEST_ASSERT_TRUE(hit("GET", "/files/a"));
    TEST_ASSERT_TRUE(hit("GET", "/files/deep/a/b")); // '.' matches '/'
    TEST_ASSERT_FALSE(hit("GET", "/other"));
}

void test_escaped_dot_extension()
{
    server.on_regex("/img/.+\\.png", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/img/cat.png"));
    TEST_ASSERT_FALSE(hit("GET", "/img/cat.jpg"));
    TEST_ASSERT_FALSE(hit("GET", "/img/.png")); // .+ needs >=1 char before ".png"
}

void test_optional_quantifier()
{
    server.on_regex("/colou?r", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/color"));
    TEST_ASSERT_TRUE(hit("GET", "/colour"));
    TEST_ASSERT_FALSE(hit("GET", "/colouur"));
}

void test_range_class_only()
{
    server.on_regex("/[a-z]+", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/abc"));
    TEST_ASSERT_FALSE(hit("GET", "/ABC"));
    TEST_ASSERT_FALSE(hit("GET", "/a1"));
}

void test_negated_class()
{
    server.on_regex("/x[^/]+", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/xabc"));
    TEST_ASSERT_FALSE(hit("GET", "/x/")); // '/' excluded
}

void test_anchored_full_match()
{
    server.on_regex("/api/v[12]", HTTP_GET, h_ok);
    TEST_ASSERT_TRUE(hit("GET", "/api/v1"));
    TEST_ASSERT_TRUE(hit("GET", "/api/v2"));
    TEST_ASSERT_FALSE(hit("GET", "/api/v3"));
    TEST_ASSERT_FALSE(hit("GET", "/api/v12")); // extra char -> no full match
    TEST_ASSERT_FALSE(hit("GET", "/api/v1/x"));
}

void test_method_still_enforced()
{
    server.on_regex("/sensor/[0-9]+", HTTP_GET, h_ok);
    // Path matches but method differs -> 405, handler not called.
    TEST_ASSERT_FALSE(hit("POST", "/sensor/42"));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "405"));
}

void test_pathological_pattern_terminates_no_match()
{
    // Catastrophic-looking pattern with no possible match: must return (not hang)
    // and report no match, thanks to the RE_MAX_STEPS budget.
    server.on_regex("/a*a*a*a*a*b", HTTP_GET, h_ok);
    TEST_ASSERT_FALSE(hit("GET", "/aaaaaaaaaaaaaaaaaaaaaaaac"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_numeric_class_plus);
    RUN_TEST(test_dot_star_matches_rest);
    RUN_TEST(test_escaped_dot_extension);
    RUN_TEST(test_optional_quantifier);
    RUN_TEST(test_range_class_only);
    RUN_TEST(test_negated_class);
    RUN_TEST(test_anchored_full_match);
    RUN_TEST(test_method_still_enforced);
    RUN_TEST(test_pathological_pattern_terminates_no_match);
    return UNITY_END();
}
