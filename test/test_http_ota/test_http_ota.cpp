// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Tests the parser's streaming-body hook (DETWS_ENABLE_OTA): a body larger than
// BODY_BUF_SIZE is streamed to a sink in chunks and reaches PARSE_COMPLETE
// (bypassing the 413 cap), while the default 413 behavior is preserved when no
// hook matches. Uses a mock sink (no ESP32 Update dependency).

#include "network_drivers/presentation/http_parser/http_parser.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static size_t g_total;
static int g_chunks;
static uint8_t g_capture[8192];

static bool begin_cb(HttpReq *req)
{
    return strcmp(req->method, "POST") == 0 && strcmp(req->path, "/update") == 0;
}
static void data_cb(HttpReq *req, const uint8_t *d, size_t n)
{
    (void)req;
    if (g_total + n <= sizeof(g_capture))
        memcpy(g_capture + g_total, d, n);
    g_total += n;
    g_chunks++;
}

static void feed(HttpReq *r, const char *s)
{
    for (size_t i = 0; s[i]; i++)
        http_parser_feed(r, (uint8_t)s[i]);
}

void setUp()
{
    g_total = 0;
    g_chunks = 0;
    http_parser_set_stream_hooks(nullptr, nullptr);
}
void tearDown()
{
}

// A >BODY_BUF_SIZE body on the matched route streams to completion in chunks.
void test_large_body_streams_to_completion()
{
    http_parser_set_stream_hooks(begin_cb, data_cb);
    HttpReq r;
    r.slot_id = 0;
    http_parser_reset(&r);

    const size_t N = 4096; // >> BODY_BUF_SIZE (256)
    char hdr[128];
    snprintf(hdr, sizeof(hdr), "POST /update HTTP/1.1\r\nHost: x\r\nContent-Length: %u\r\n\r\n", (unsigned)N);
    feed(&r, hdr);
    for (size_t i = 0; i < N; i++)
        http_parser_feed(&r, (uint8_t)('A' + (i % 26)));

    TEST_ASSERT_EQUAL(PARSE_COMPLETE, r.parse_state);
    TEST_ASSERT_TRUE(r.body_streaming);
    TEST_ASSERT_EQUAL_UINT(N, (unsigned)g_total); // every byte delivered
    TEST_ASSERT_GREATER_THAN(1, g_chunks);        // multiple chunks → cap bypassed
    for (size_t i = 0; i < N; i++)                // content intact
        TEST_ASSERT_EQUAL_UINT8('A' + (i % 26), g_capture[i]);
}

// Without hooks, a >BODY_BUF_SIZE body still yields 413 (default unchanged).
void test_no_hooks_large_body_is_413()
{
    HttpReq r;
    r.slot_id = 0;
    http_parser_reset(&r);
    feed(&r, "POST /update HTTP/1.1\r\nHost: x\r\nContent-Length: 4096\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, r.parse_state);
}

// A non-matching path is not streamed, so the large body still 413s.
void test_nonmatching_path_not_streamed()
{
    http_parser_set_stream_hooks(begin_cb, data_cb);
    HttpReq r;
    r.slot_id = 0;
    http_parser_reset(&r);
    feed(&r, "POST /other HTTP/1.1\r\nHost: x\r\nContent-Length: 4096\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, r.parse_state);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)g_total);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_large_body_streams_to_completion);
    RUN_TEST(test_no_hooks_large_body_is_413);
    RUN_TEST(test_nonmatching_path_not_streamed);
    return UNITY_END();
}
