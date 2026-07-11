// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Streaming file upload (DETWS_ENABLE_UPLOAD): a POST body is streamed straight
// into an FS file via the parser's streaming-body hook. Built with
// BODY_BUF_SIZE=64 so a larger body exercises multi-chunk streaming; the mock FS
// captures the written bytes for verification.

#include "FS.h"
#include "dwserver.h"
#include "services/upload_service/upload_service.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DetWebServer server;
static fs::FS g_fs;

static void push_bytes(uint8_t slot, const char *data, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; i < n; i++)
    {
        c->rx_buffer[c->rx_head] = (uint8_t)data[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}

void setUp()
{
    server = DetWebServer();
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
    fs::mock_fs_reset();
    fs::mock_fs_write_reset();
    tcp_capture_reset();
}

void tearDown()
{
    tcp_capture_disable();
}

void test_upload_streams_body_to_file()
{
    detws_upload_begin(server, "/upload", g_fs, "/dest.bin");

    // 200-byte body (> BODY_BUF_SIZE=64) -> several streamed chunks.
    char body[200];
    for (int i = 0; i < (int)sizeof(body); i++)
        body[i] = (char)('A' + (i % 26));
    size_t blen = sizeof(body);

    char req[400];
    int hn = snprintf(req, sizeof(req), "POST /upload HTTP/1.1\r\nContent-Length: %u\r\n\r\n", (unsigned)blen);
    push_bytes(0, req, (size_t)hn);
    push_bytes(0, body, blen);
    http_parse(0);
    server.handle();

    TEST_ASSERT_EQUAL_UINT(blen, fs::mock_fs_written());
    TEST_ASSERT_EQUAL_MEMORY(body, fs::mock_fs_wdata(), blen);
    TEST_ASSERT_EQUAL_UINT(blen, detws_upload_last_size());

    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "200 OK"));
    char expect[24];
    snprintf(expect, sizeof(expect), "%u bytes", (unsigned)blen);
    TEST_ASSERT_NOT_NULL(strstr(out, expect));
}

void test_small_body_single_chunk()
{
    detws_upload_begin(server, "/upload", g_fs, "/dest.bin");
    const char *body = "tiny";
    char req[128];
    int hn = snprintf(req, sizeof(req), "POST /upload HTTP/1.1\r\nContent-Length: 4\r\n\r\n%s", body);
    push_bytes(0, req, (size_t)hn);
    http_parse(0);
    server.handle();
    TEST_ASSERT_EQUAL_UINT(4, fs::mock_fs_written());
    TEST_ASSERT_EQUAL_MEMORY("tiny", fs::mock_fs_wdata(), 4);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "200 OK"));
}

void test_empty_body_not_streamed()
{
    detws_upload_begin(server, "/upload", g_fs, "/dest.bin");
    char req[128];
    int hn = snprintf(req, sizeof(req), "POST /upload HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    push_bytes(0, req, (size_t)hn);
    http_parse(0);
    server.handle();
    // No body -> not streamed -> handler replies 400, nothing written.
    TEST_ASSERT_EQUAL_UINT(0, fs::mock_fs_written());
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "400"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_upload_streams_body_to_file);
    RUN_TEST(test_small_body_single_chunk);
    RUN_TEST(test_empty_body_not_streamed);
    return UNITY_END();
}
