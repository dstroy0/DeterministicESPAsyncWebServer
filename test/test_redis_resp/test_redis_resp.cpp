// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Redis RESP2 codec (services/redis_resp): the command encoder
// and the cursor reply parser. Pure host tests.

#include "services/redis_resp.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A command encodes as an array of bulk strings.
void test_encode_command()
{
    const char *argv[] = {"SET", "foo", "bar"};
    char buf[64];
    size_t n = resp_encode_command(buf, sizeof(buf), argv, nullptr, 3);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING("*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n", buf);
    TEST_ASSERT_EQUAL_size_t(n, strlen(buf));
}

// Explicit per-arg lengths allow binary-safe args (embedded NUL).
void test_encode_binary_safe()
{
    const char a0[] = {'S', 'E', 'T'};
    const char a1[] = {'k'};
    const char a2[] = {'a', '\0', 'b'}; // contains a NUL
    const char *argv[] = {a0, a1, a2};
    const size_t lens[] = {3, 1, 3};
    char buf[64];
    size_t n = resp_encode_command(buf, sizeof(buf), argv, lens, 3);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    const char expect[] = "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$3\r\na\0b\r\n";
    TEST_ASSERT_EQUAL_size_t(sizeof(expect) - 1, n);
    TEST_ASSERT_EQUAL_MEMORY(expect, buf, n);
}

void test_encode_overflow_fails_closed()
{
    const char *argv[] = {"SET", "foo", "bar"};
    char buf[8];
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, sizeof(buf), argv, nullptr, 3));
}

void test_parse_simple_and_error()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"+OK\r\n", 5, &r, &c));
    TEST_ASSERT_EQUAL(RESP_SIMPLE, r.type);
    TEST_ASSERT_EQUAL_MEMORY("OK", r.str, 2);
    TEST_ASSERT_EQUAL_size_t(5, c);

    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"-ERR no\r\n", 9, &r, &c));
    TEST_ASSERT_EQUAL(RESP_ERROR, r.type);
    TEST_ASSERT_EQUAL_MEMORY("ERR no", r.str, 6);
}

void test_parse_integer()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)":1000\r\n", 7, &r, &c));
    TEST_ASSERT_EQUAL(RESP_INTEGER, r.type);
    TEST_ASSERT_EQUAL_INT64(1000, r.ival);
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)":-5\r\n", 5, &r, &c));
    TEST_ASSERT_EQUAL_INT64(-5, r.ival);
}

void test_parse_bulk_and_nil()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"$5\r\nhello\r\n", 11, &r, &c));
    TEST_ASSERT_EQUAL(RESP_BULK, r.type);
    TEST_ASSERT_EQUAL_size_t(5, r.str_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", r.str, 5);
    TEST_ASSERT_EQUAL_size_t(11, c);

    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"$-1\r\n", 5, &r, &c)); // nil bulk
    TEST_ASSERT_EQUAL(RESP_NIL, r.type);
}

// An array is parsed as a header (count); the caller then parses each element
// from the remaining bytes using the reported consumed offset.
void test_parse_array_cursor()
{
    const uint8_t *msg = (const uint8_t *)"*2\r\n$3\r\nfoo\r\n:42\r\n";
    size_t len = strlen((const char *)msg);
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse(msg, len, &r, &c));
    TEST_ASSERT_EQUAL(RESP_ARRAY, r.type);
    TEST_ASSERT_EQUAL_INT64(2, r.count);
    size_t off = c;
    TEST_ASSERT_TRUE(resp_parse(msg + off, len - off, &r, &c)); // element 0
    TEST_ASSERT_EQUAL(RESP_BULK, r.type);
    TEST_ASSERT_EQUAL_MEMORY("foo", r.str, 3);
    off += c;
    TEST_ASSERT_TRUE(resp_parse(msg + off, len - off, &r, &c)); // element 1
    TEST_ASSERT_EQUAL(RESP_INTEGER, r.type);
    TEST_ASSERT_EQUAL_INT64(42, r.ival);
}

void test_parse_incomplete_and_malformed()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"+OK", 3, &r, &c));           // no CRLF yet
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"$5\r\nhel", 7, &r, &c));     // bulk body incomplete
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"?bad\r\n", 6, &r, &c));      // unknown type byte
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)":notanum\r\n", 10, &r, &c)); // non-numeric integer
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_encode_command);
    RUN_TEST(test_encode_binary_safe);
    RUN_TEST(test_encode_overflow_fails_closed);
    RUN_TEST(test_parse_simple_and_error);
    RUN_TEST(test_parse_integer);
    RUN_TEST(test_parse_bulk_and_nil);
    RUN_TEST(test_parse_array_cursor);
    RUN_TEST(test_parse_incomplete_and_malformed);
    return UNITY_END();
}
