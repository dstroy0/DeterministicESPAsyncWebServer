// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the WebSocket web-serial terminal (DWS_ENABLE_WEB_TERMINAL):
// page serving, the WS upgrade + connect tracking, browser->device commands, and
// device->browser broadcast.

#include "services/web_terminal/web_terminal.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DWS server;
static char g_cmd[64];
static uint8_t g_cmd_client;

static void on_cmd(const char *line, uint8_t client_id)
{
    snprintf(g_cmd, sizeof(g_cmd), "%s", line);
    g_cmd_client = client_id;
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

static void push_bytes(uint8_t slot, const uint8_t *data, size_t len)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; i < len; i++)
    {
        size_t next = (c->rx_head + 1) % RX_BUF_SIZE;
        c->rx_buffer[c->rx_head] = data[i];
        c->rx_head = next;
    }
}

// Build a WS frame (mask key all-zero so the stored payload equals the input).
static size_t build_frame(uint8_t *dst, WsOpcode opcode, const uint8_t *payload, uint16_t len)
{
    size_t pos = 0;
    dst[pos++] = 0x80u | (uint8_t)opcode; // FIN + opcode
    dst[pos++] = 0x80u | (uint8_t)len;    // MASK + len (<=125)
    dst[pos++] = 0;
    dst[pos++] = 0;
    dst[pos++] = 0;
    dst[pos++] = 0;
    if (payload && len)
    {
        memcpy(dst + pos, payload, len);
        pos += len;
    }
    return pos;
}

void setUp()
{
    server = DWS();
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
    dws_sse_init();
    tcp_capture_reset();
    g_cmd[0] = '\0';
    g_cmd_client = 0xFF;
    dws_web_terminal_begin(server, "/terminal");
    dws_web_terminal_on_command(on_cmd);
}

void tearDown()
{
    tcp_capture_disable();
}

// Perform the WS upgrade handshake on a slot; return the allocated ws_id.
static uint8_t do_handshake(uint8_t slot)
{
    push_str(slot, "GET /terminal/ws HTTP/1.1\r\nHost: x\r\n"
                   "Upgrade: websocket\r\nConnection: Upgrade\r\n"
                   "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                   "Sec-WebSocket-Version: 13\r\n\r\n");
    http_parse(slot);
    server.handle();
    WsConn *ws = ws_find(slot);
    return ws ? ws->ws_id : 0xFF;
}

// ====================================================================
// TESTS
// ====================================================================

void test_serves_terminal_page()
{
    push_str(0, "GET /terminal HTTP/1.1\r\nHost: x\r\n\r\n");
    http_parse(0);
    server.handle();
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(r, "text/html"));
    TEST_ASSERT_NOT_NULL(strstr(r, "DWS Terminal")); // page title
    TEST_ASSERT_NOT_NULL(strstr(r, "#080c08"));      // docs CRT theme bg
}

void test_ws_upgrade_tracks_client()
{
    TEST_ASSERT_EQUAL_UINT(0, dws_web_terminal_client_count());
    uint8_t wid = do_handshake(0);
    TEST_ASSERT_NOT_EQUAL(0xFF, wid);
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "101 Switching Protocols"));
    TEST_ASSERT_EQUAL_UINT(1, dws_web_terminal_client_count());
}

// RFC 6455 4.2.1: a GET with Upgrade: websocket but no "Upgrade" token in the
// Connection header is not a valid handshake -> 400, no upgrade.
void test_ws_upgrade_requires_connection_token()
{
    push_str(0, "GET /terminal/ws HTTP/1.1\r\nHost: x\r\n"
                "Upgrade: websocket\r\n" // no Connection: Upgrade
                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n");
    http_parse(0);
    server.handle();
    TEST_ASSERT_NULL(ws_find(0));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "400"));
}

// RFC 6455 4.2.1: a Sec-WebSocket-Key that does not base64-decode to 16 bytes is a
// bad handshake -> 400, no upgrade.
void test_ws_upgrade_rejects_bad_key_length()
{
    push_str(0, "GET /terminal/ws HTTP/1.1\r\nHost: x\r\n"
                "Upgrade: websocket\r\nConnection: Upgrade\r\n"
                "Sec-WebSocket-Key: c2hvcnQ=\r\nSec-WebSocket-Version: 13\r\n\r\n"); // "short" -> 5 bytes
    http_parse(0);
    server.handle();
    TEST_ASSERT_NULL(ws_find(0));
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "400"));
}

void test_command_delivered_to_callback()
{
    uint8_t wid = do_handshake(0);
    uint8_t frame[32];
    size_t n = build_frame(frame, WsOpcode::WS_OP_TEXT, (const uint8_t *)"reboot", 6);
    push_bytes(0, frame, n);
    server.handle();
    TEST_ASSERT_EQUAL_STRING("reboot", g_cmd);
    TEST_ASSERT_EQUAL_UINT(wid, g_cmd_client);
}

void test_broadcast_reaches_client()
{
    do_handshake(0);
    tcp_capture_reset(); // isolate the broadcast from the handshake bytes
    dws_web_terminal_print("hello browser");
    const char *r = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(r, "hello browser"));
}

void test_printf_broadcast()
{
    do_handshake(0);
    tcp_capture_reset();
    dws_web_terminal_printf("count=%d", 7);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "count=7"));
}

void test_no_broadcast_without_clients()
{
    // No handshake -> no terminal clients -> print writes nothing.
    tcp_capture_reset();
    dws_web_terminal_print("nobody home");
    TEST_ASSERT_EQUAL_UINT(0, dws_web_terminal_client_count());
    TEST_ASSERT_NULL(strstr(tcp_captured(), "nobody home"));
}

void test_close_clears_client()
{
    do_handshake(0);
    TEST_ASSERT_EQUAL_UINT(1, dws_web_terminal_client_count());
    WsConn *ws = ws_find(0);
    ws->parse_state = WsParseState::WS_CLOSED; // simulate client close
    server.handle();                           // handle() fires the ws_close route callback
    TEST_ASSERT_EQUAL_UINT(0, dws_web_terminal_client_count());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_serves_terminal_page);
    RUN_TEST(test_ws_upgrade_tracks_client);
    RUN_TEST(test_ws_upgrade_requires_connection_token);
    RUN_TEST(test_ws_upgrade_rejects_bad_key_length);
    RUN_TEST(test_command_delivered_to_callback);
    RUN_TEST(test_broadcast_reaches_client);
    RUN_TEST(test_printf_broadcast);
    RUN_TEST(test_no_broadcast_without_clients);
    RUN_TEST(test_close_clears_client);
    return UNITY_END();
}
