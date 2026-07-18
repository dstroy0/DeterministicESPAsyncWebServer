// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the NATS client protocol codec (services/nats): the CONNECT/PUB/SUB/UNSUB/
// PING/PONG builders and the inbound MSG/INFO/PING/+OK/-ERR parser. Pure host tests.

#include "services/nats/nats.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_build_connect()
{
    char buf[64];
    size_t n = dws_nats_build_connect(buf, sizeof(buf), "{\"verbose\":false}");
    TEST_ASSERT_EQUAL_STRING("CONNECT {\"verbose\":false}\r\n", buf);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
}

void test_build_pub()
{
    char buf[64];
    size_t n = dws_nats_build_pub(buf, sizeof(buf), "foo", nullptr, (const uint8_t *)"hello", 5);
    TEST_ASSERT_EQUAL_STRING("PUB foo 5\r\nhello\r\n", buf);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
}

void test_build_pub_with_reply()
{
    char buf[64];
    size_t n = dws_nats_build_pub(buf, sizeof(buf), "req", "_INBOX.1", (const uint8_t *)"hi", 2);
    TEST_ASSERT_EQUAL_STRING("PUB req _INBOX.1 2\r\nhi\r\n", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
}

void test_build_pub_empty_payload()
{
    char buf[32];
    size_t n = dws_nats_build_pub(buf, sizeof(buf), "foo", nullptr, nullptr, 0);
    TEST_ASSERT_EQUAL_STRING("PUB foo 0\r\n\r\n", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
}

void test_build_sub_and_unsub()
{
    char buf[32];
    TEST_ASSERT_GREATER_THAN(0, (int)dws_nats_build_sub(buf, sizeof(buf), "foo", nullptr, "1"));
    TEST_ASSERT_EQUAL_STRING("SUB foo 1\r\n", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)dws_nats_build_sub(buf, sizeof(buf), "foo", "workers", "9"));
    TEST_ASSERT_EQUAL_STRING("SUB foo workers 9\r\n", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)dws_nats_build_unsub(buf, sizeof(buf), "1", 5, true));
    TEST_ASSERT_EQUAL_STRING("UNSUB 1 5\r\n", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)dws_nats_build_unsub(buf, sizeof(buf), "1", 0, false));
    TEST_ASSERT_EQUAL_STRING("UNSUB 1\r\n", buf);
}

void test_parse_msg()
{
    const char *raw = "MSG foo 1 5\r\nhello\r\nMSG bar 2 3\r\nbye\r\n";
    size_t len = strlen(raw);
    NatsMsg m;
    size_t c;
    TEST_ASSERT_TRUE(dws_nats_parse(raw, len, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_MSG, m.type);
    TEST_ASSERT_EQUAL_MEMORY("foo", m.subject, m.subject_len);
    TEST_ASSERT_EQUAL_MEMORY("1", m.sid, m.sid_len);
    TEST_ASSERT_EQUAL_size_t(0, m.reply_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", m.payload, 5);
    // The second message follows at the consumed offset.
    size_t off = c;
    TEST_ASSERT_TRUE(dws_nats_parse(raw + off, len - off, &m, &c));
    TEST_ASSERT_EQUAL_MEMORY("bar", m.subject, 3);
    TEST_ASSERT_EQUAL_MEMORY("bye", m.payload, 3);
}

void test_parse_msg_with_reply()
{
    const char *raw = "MSG foo 1 _INBOX.7 5\r\nhello\r\n";
    NatsMsg m;
    size_t c;
    TEST_ASSERT_TRUE(dws_nats_parse(raw, strlen(raw), &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_MSG, m.type);
    TEST_ASSERT_EQUAL_MEMORY("_INBOX.7", m.reply, m.reply_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", m.payload, 5);
}

void test_parse_control_lines()
{
    NatsMsg m;
    size_t c;
    TEST_ASSERT_TRUE(dws_nats_parse("PING\r\n", 6, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_PING, m.type);
    TEST_ASSERT_TRUE(dws_nats_parse("PONG\r\n", 6, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_PONG, m.type);
    TEST_ASSERT_TRUE(dws_nats_parse("+OK\r\n", 5, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_OK, m.type);
    TEST_ASSERT_TRUE(dws_nats_parse("-ERR 'Unknown Protocol Operation'\r\n", 35, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_ERR, m.type);
    TEST_ASSERT_EQUAL_MEMORY("'Unknown Protocol Operation'", m.arg, m.arg_len);
    TEST_ASSERT_TRUE(dws_nats_parse("INFO {\"server_id\":\"x\"}\r\n", 24, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_INFO, m.type);
    TEST_ASSERT_EQUAL_MEMORY("{\"server_id\":\"x\"}", m.arg, m.arg_len);
}

void test_parse_incomplete()
{
    NatsMsg m;
    size_t c;
    TEST_ASSERT_FALSE(dws_nats_parse("PING", 4, &m, &c));                // no CRLF yet
    TEST_ASSERT_FALSE(dws_nats_parse("MSG foo 1 5\r\nhel", 16, &m, &c)); // payload short
    // A byte count near SIZE_MAX must fail closed, not wrap the bounds check (32-bit hardening).
    TEST_ASSERT_FALSE(dws_nats_parse("MSG foo 1 999999999999\r\nhi\r\n", 28, &m, &c));
}

void test_build_overflow_fails_closed()
{
    char small[8];
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(small, sizeof(small), "foo", nullptr, (const uint8_t *)"hello", 5));
}

void test_build_ping_pong()
{
    char buf[16];
    TEST_ASSERT_EQUAL_size_t(6, dws_nats_build_ping(buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("PING\r\n", buf);
    TEST_ASSERT_EQUAL_size_t(6, dws_nats_build_pong(buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("PONG\r\n", buf);
}

void test_build_null_args()
{
    char buf[64];
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_connect(nullptr, 64, "{}"));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_connect(buf, sizeof(buf), nullptr));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(nullptr, 64, "s", nullptr, nullptr, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(buf, sizeof(buf), nullptr, nullptr, nullptr, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(buf, sizeof(buf), "s", nullptr, nullptr, 3)); // len && !payload
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_sub(nullptr, 64, "s", nullptr, "1"));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_sub(buf, sizeof(buf), nullptr, nullptr, "1"));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_sub(buf, sizeof(buf), "s", nullptr, nullptr));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_unsub(nullptr, 64, "1", 0, false));
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_unsub(buf, sizeof(buf), nullptr, 0, false));
}

void test_build_overflow_put_ch()
{
    char buf[16];
    // cap 6: "PUB " fits, "foo" overflows in put_str -> ok=false, then put_ch bails.
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(buf, 6, "foo", nullptr, (const uint8_t *)"hi", 2));
    // cap 7: "PUB foo" fits exactly, the following put_ch(' ') is the overflow.
    TEST_ASSERT_EQUAL_size_t(0, dws_nats_build_pub(buf, 7, "foo", nullptr, (const uint8_t *)"hi", 2));
}

void test_parse_edges()
{
    NatsMsg m;
    size_t c;
    TEST_ASSERT_FALSE(dws_nats_parse(nullptr, 5, &m, &c));
    TEST_ASSERT_FALSE(dws_nats_parse("PING\r\n", 6, nullptr, &c));
    TEST_ASSERT_FALSE(dws_nats_parse("PING\r\n", 6, &m, nullptr));

    // MSG with too few tokens, and with a non-numeric byte count.
    TEST_ASSERT_FALSE(dws_nats_parse("MSG foo\r\n", 9, &m, &c));
    TEST_ASSERT_FALSE(dws_nats_parse("MSG a b xyz\r\n", 13, &m, &c));

    // MSG line with trailing whitespace before the CRLF still parses.
    const char *raw = "MSG a b 3 \r\nXXX\r\n";
    TEST_ASSERT_TRUE(dws_nats_parse(raw, strlen(raw), &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_MSG, m.type);
    TEST_ASSERT_EQUAL_size_t(3, m.payload_len);

    // An unrecognized verb parses as UNKNOWN (consumes the line).
    TEST_ASSERT_TRUE(dws_nats_parse("ZZZ whatever\r\n", 14, &m, &c));
    TEST_ASSERT_EQUAL(NatsMsgType::NATS_UNKNOWN, m.type);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_connect);
    RUN_TEST(test_build_ping_pong);
    RUN_TEST(test_build_null_args);
    RUN_TEST(test_build_overflow_put_ch);
    RUN_TEST(test_parse_edges);
    RUN_TEST(test_build_pub);
    RUN_TEST(test_build_pub_with_reply);
    RUN_TEST(test_build_pub_empty_payload);
    RUN_TEST(test_build_sub_and_unsub);
    RUN_TEST(test_parse_msg);
    RUN_TEST(test_parse_msg_with_reply);
    RUN_TEST(test_parse_control_lines);
    RUN_TEST(test_parse_incomplete);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
