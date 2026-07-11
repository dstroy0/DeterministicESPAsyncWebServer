// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Telnet server test: drives a ConnProto::PROTO_TELNET connection through the real
// conn_pool ring buffer and checks the IAC negotiation, line echo/editing, and
// command dispatch via the tcp_write capture mock.

#include "lwip/tcp.h"
#include "network_drivers/presentation/telnet/telnet.h"
#include "network_drivers/transport/tcp.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

#define IAC 255
#define WILL 251
#define WONT 252
#define DO 253
#define DONT 254
#define OPT_ECHO 1
#define OPT_SGA 3

static char g_last_cmd[128];
static int g_cmd_count;

static void cmd_cb(const char *line, uint8_t id)
{
    (void)id;
    strncpy(g_last_cmd, line, sizeof(g_last_cmd) - 1);
    g_last_cmd[sizeof(g_last_cmd) - 1] = '\0';
    g_cmd_count++;
}

void setUp()
{
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].pcb = &_mock_pcb;
        conn_pool[i].proto = ConnProto::PROTO_TELNET;
    }
    g_last_cmd[0] = '\0';
    g_cmd_count = 0;
    telnet_on_command(cmd_cb);
    tcp_capture_reset();
}

void tearDown()
{
    telnet_close(0);
    tcp_capture_disable();
}

static void push_bytes(uint8_t slot, const uint8_t *d, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; i < n; i++)
    {
        c->rx_buffer[c->rx_head] = d[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}
static void push_str(uint8_t slot, const char *s)
{
    push_bytes(slot, (const uint8_t *)s, strlen(s));
}

// ---------------------------------------------------------------------------

void test_accept_negotiates_echo_and_sga()
{
    telnet_accept(0);
    const uint8_t *out = (const uint8_t *)tcp_captured();
    TEST_ASSERT_TRUE(tcp_captured_len() >= 6);
    // IAC WILL ECHO, IAC WILL SUPPRESS-GO-AHEAD
    const uint8_t expect[6] = {IAC, WILL, OPT_ECHO, IAC, WILL, OPT_SGA};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 6);
    TEST_ASSERT_EQUAL_UINT8(1, telnet_client_count());
}

void test_line_echoed_and_dispatched()
{
    telnet_accept(0);
    tcp_capture_reset();
    push_str(0, "hello\n");
    telnet_rx(0);
    TEST_ASSERT_EQUAL_STRING("hello", g_last_cmd);
    TEST_ASSERT_EQUAL_INT(1, g_cmd_count);
    // Each typed char was echoed back.
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "hello"));
}

void test_backspace_first_line()
{
    telnet_accept(0);
    tcp_capture_reset();
    push_str(0, "ab\x08\n"); // 0x08 backspace removes 'b' -> "a"
    telnet_rx(0);
    TEST_ASSERT_EQUAL_STRING("a", g_last_cmd);
}

void test_iac_will_gets_dont()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[3] = {IAC, WILL, 24}; // client WILL TERMINAL-TYPE
    push_bytes(0, seq, 3);
    telnet_rx(0);
    const uint8_t *out = (const uint8_t *)tcp_captured();
    TEST_ASSERT_EQUAL_UINT(3, tcp_captured_len());
    const uint8_t expect[3] = {IAC, DONT, 24};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 3);
}

void test_iac_do_unsupported_gets_wont()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[3] = {IAC, DO, 31}; // client DO NAWS (window size) - unsupported
    push_bytes(0, seq, 3);
    telnet_rx(0);
    const uint8_t *out = (const uint8_t *)tcp_captured();
    TEST_ASSERT_EQUAL_UINT(3, tcp_captured_len());
    const uint8_t expect[3] = {IAC, WONT, 31};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 3);
}

void test_iac_do_echo_is_silent()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[3] = {IAC, DO, OPT_ECHO}; // already offered WILL ECHO -> no reply
    push_bytes(0, seq, 3);
    telnet_rx(0);
    TEST_ASSERT_EQUAL_UINT(0, tcp_captured_len());
}

void test_iac_stripped_from_data()
{
    telnet_accept(0);
    tcp_capture_reset();
    // "a" IAC-NOP "b" \n  -> line should be "ab" (IAC sequence consumed)
    const uint8_t seq[] = {'a', IAC, 241 /*NOP*/, 'b', '\n'};
    push_bytes(0, seq, sizeof(seq));
    telnet_rx(0);
    TEST_ASSERT_EQUAL_STRING("ab", g_last_cmd);
}

void test_print_broadcast()
{
    telnet_accept(0);
    tcp_capture_reset();
    telnet_println("hi there");
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "hi there"));
    TEST_ASSERT_NOT_NULL(strstr(out, "\r\n"));
}

// telnet_rx / telnet_close on a slot with no Telnet connection are safe no-ops.
void test_unknown_slot_is_noop()
{
    telnet_accept(0);
    tcp_capture_reset();
    telnet_rx(1); // no TelnetConn for slot 1
    telnet_close(1);
    TEST_ASSERT_EQUAL_UINT(0, tcp_captured_len());
    TEST_ASSERT_EQUAL_UINT8(1, telnet_client_count());
}

// A bare CR waits for its LF, and control characters are ignored in a line.
void test_cr_and_control_ignored()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[] = {'a', '\r', 0x01, 'b', '\n'}; // CR held, 0x01 dropped
    push_bytes(0, seq, sizeof(seq));
    telnet_rx(0);
    TEST_ASSERT_EQUAL_STRING("ab", g_last_cmd);
}

// IAC IAC in the data stream is an escaped literal 0xFF added to the line.
void test_iac_escaped_literal()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[] = {'x', IAC, IAC, '\n'};
    push_bytes(0, seq, sizeof(seq));
    telnet_rx(0);
    TEST_ASSERT_EQUAL_UINT8('x', (uint8_t)g_last_cmd[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, (uint8_t)g_last_cmd[1]);
    TEST_ASSERT_EQUAL_UINT8(0, (uint8_t)g_last_cmd[2]);
}

// A subnegotiation (IAC SB ... SE) is consumed; following data resumes normally.
void test_subnegotiation_consumed()
{
    telnet_accept(0);
    tcp_capture_reset();
    const uint8_t seq[] = {IAC, 250 /*SB*/, 24, 'a', 'b', 240 /*SE*/, 'h', 'i', '\n'};
    push_bytes(0, seq, sizeof(seq));
    telnet_rx(0);
    TEST_ASSERT_EQUAL_STRING("hi", g_last_cmd);
}

// Past MAX_TELNET_CONNS the extra connection is dropped, not admitted.
void test_accept_no_capacity()
{
    for (uint8_t s = 0; s < MAX_TELNET_CONNS; s++)
        telnet_accept(s);
    TEST_ASSERT_EQUAL_UINT8(MAX_TELNET_CONNS, telnet_client_count());
    telnet_accept(MAX_TELNET_CONNS); // one past capacity -> dropped
    TEST_ASSERT_EQUAL_UINT8(MAX_TELNET_CONNS, telnet_client_count());
    for (uint8_t s = 0; s < MAX_TELNET_CONNS; s++)
        telnet_close(s);
}

// Application output doubles a literal IAC (RFC 854); printf formats and broadcasts.
void test_output_escaping_and_printf()
{
    telnet_accept(0);
    tcp_capture_reset();
    telnet_print("a\xff"
                 "b");
    const uint8_t *out = (const uint8_t *)tcp_captured();
    const uint8_t expect[] = {'a', 0xFF, 0xFF, 'b'};
    TEST_ASSERT_EQUAL_UINT(4, tcp_captured_len());
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 4);

    tcp_capture_reset();
    telnet_printf("n=%d", 7);
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "n=7"));
}

// An inactive connection (no pcb) swallows both raw and escaped sends.
void test_inactive_conn_sends_nothing()
{
    telnet_accept(0);
    conn_pool[0].pcb = nullptr; // connection went away under us
    tcp_capture_reset();
    telnet_print("\xff"); // send_escaped bails on the inactive conn
    push_str(0, "x\n");   // handle_data -> raw_send bails too
    telnet_rx(0);
    TEST_ASSERT_EQUAL_UINT(0, tcp_captured_len());
    TEST_ASSERT_EQUAL_STRING("x", g_last_cmd); // line still dispatched
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_accept_negotiates_echo_and_sga);
    RUN_TEST(test_line_echoed_and_dispatched);
    RUN_TEST(test_backspace_first_line);
    RUN_TEST(test_iac_will_gets_dont);
    RUN_TEST(test_iac_do_unsupported_gets_wont);
    RUN_TEST(test_iac_do_echo_is_silent);
    RUN_TEST(test_iac_stripped_from_data);
    RUN_TEST(test_print_broadcast);
    RUN_TEST(test_unknown_slot_is_noop);
    RUN_TEST(test_cr_and_control_ignored);
    RUN_TEST(test_iac_escaped_literal);
    RUN_TEST(test_subnegotiation_consumed);
    RUN_TEST(test_accept_no_capacity);
    RUN_TEST(test_output_escaping_and_printf);
    RUN_TEST(test_inactive_conn_sends_nothing);
    return UNITY_END();
}
