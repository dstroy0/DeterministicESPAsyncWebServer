// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Redis RESP2 codec (services/redis_resp): the command encoder
// and the cursor reply parser. Pure host tests.

#include "services/redis_resp.h"
#include <math.h>
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
    // A bulk length near SIZE_MAX must fail closed, not wrap the bounds check (32-bit hardening).
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"$999999999999\r\nhi\r\n", 19, &r, &c));
}

// Every guard sub-condition of resp_encode_command, plus a null arg element and an arg-loop overflow.
void test_encode_guard_subconditions()
{
    char buf[64];
    const char *argv[] = {"GET", "k"};
    size_t alen[] = {3, 1};
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(nullptr, sizeof(buf), argv, alen, 2)); // null buf
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, 0, argv, alen, 2));               // zero cap
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, sizeof(buf), nullptr, alen, 2));  // null args
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, sizeof(buf), argv, alen, 0));     // zero argc
    const char *withnull[] = {"GET", nullptr};
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, sizeof(buf), withnull, nullptr, 2)); // null arg elem
    TEST_ASSERT_EQUAL_size_t(0, resp_encode_command(buf, 8, argv, alen, 2)); // header fits, arg overflows
}

// Parser guard sub-conditions and the integer / bulk terminator / nil-array edges.
void test_parse_guard_subconditions_and_edges()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_FALSE(resp_parse(nullptr, 5, &r, &c));                           // null buf
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"+\r", 2, &r, &c));            // len < 3
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"+OK\r\n", 5, nullptr, &c));   // null out
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"+OK\r\n", 5, &r, nullptr));   // null consumed
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)":\r\n", 3, &r, &c));          // empty integer
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)":-\r\n", 4, &r, &c));         // '-' then no digit
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"$5\r\nhelloAB", 11, &r, &c)); // bulk with wrong terminator
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"*-1\r\n", 5, &r, &c));         // nil array
    TEST_ASSERT_EQUAL_INT(RESP_NIL, r.type);
}

// RESP3: null (_), boolean (#t/#f).
void test_parse_resp3_null_bool()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"_\r\n", 3, &r, &c));
    TEST_ASSERT_EQUAL(RESP_NIL, r.type);
    TEST_ASSERT_EQUAL_size_t(3, c);

    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"#t\r\n", 4, &r, &c));
    TEST_ASSERT_EQUAL(RESP_BOOL, r.type);
    TEST_ASSERT_EQUAL_INT64(1, r.ival);
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"#f\r\n", 4, &r, &c));
    TEST_ASSERT_EQUAL_INT64(0, r.ival);
    TEST_ASSERT_FALSE(resp_parse((const uint8_t *)"#x\r\n", 4, &r, &c)); // invalid boolean
}

// RESP3: double (,) including inf / -inf / nan and an exponent.
void test_parse_resp3_double()
{
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)",3.14\r\n", 7, &r, &c));
    TEST_ASSERT_EQUAL(RESP_DOUBLE, r.type);
    TEST_ASSERT_EQUAL_MEMORY("3.14", r.str, 4);
    TEST_ASSERT_TRUE(fabs(r.dval - 3.14) < 1e-9);

    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)",inf\r\n", 6, &r, &c));
    TEST_ASSERT_TRUE(isinf(r.dval) && r.dval > 0);
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)",-inf\r\n", 7, &r, &c));
    TEST_ASSERT_TRUE(isinf(r.dval) && r.dval < 0);
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)",nan\r\n", 6, &r, &c));
    TEST_ASSERT_TRUE(isnan(r.dval));
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)",1.5e3\r\n", 8, &r, &c));
    TEST_ASSERT_TRUE(fabs(r.dval - 1500.0) < 1e-6);
}

// RESP3: big number ((), bulk error (!), verbatim string (=).
void test_parse_resp3_bignum_bulkerr_verbatim()
{
    RespReply r;
    size_t c;
    const char *big = "(3492890328409238509324850943850943825024385\r\n";
    const char *digits = "3492890328409238509324850943850943825024385";
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)big, strlen(big), &r, &c));
    TEST_ASSERT_EQUAL(RESP_BIG_NUMBER, r.type);
    TEST_ASSERT_EQUAL_size_t(strlen(digits), r.str_len);
    TEST_ASSERT_EQUAL_MEMORY(digits, r.str, r.str_len);

    const char *be = "!21\r\nSYNTAX invalid syntax\r\n";
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)be, strlen(be), &r, &c));
    TEST_ASSERT_EQUAL(RESP_BULK_ERROR, r.type);
    TEST_ASSERT_EQUAL_MEMORY("SYNTAX invalid syntax", r.str, 21);
    TEST_ASSERT_EQUAL_size_t(strlen(be), c);

    const char *vb = "=15\r\ntxt:Some string\r\n";
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)vb, strlen(vb), &r, &c));
    TEST_ASSERT_EQUAL(RESP_VERBATIM, r.type);
    TEST_ASSERT_EQUAL_MEMORY("txt:Some string", r.str, 15); // format prefix kept
}

// RESP3 aggregates: map (%, reports 2*pairs), set (~), push (>).
void test_parse_resp3_map_set_push()
{
    const uint8_t *m = (const uint8_t *)"%2\r\n$1\r\na\r\n:1\r\n$1\r\nb\r\n:2\r\n";
    size_t len = strlen((const char *)m);
    RespReply r;
    size_t c;
    TEST_ASSERT_TRUE(resp_parse(m, len, &r, &c));
    TEST_ASSERT_EQUAL(RESP_MAP, r.type);
    TEST_ASSERT_EQUAL_INT64(4, r.count); // 2 pairs -> 4 children
    size_t off = c;
    int64_t seen_vals = 0;
    for (int i = 0; i < 4; i++)
    {
        TEST_ASSERT_TRUE(resp_parse(m + off, len - off, &r, &c));
        if (i % 2 == 1)
            seen_vals += r.ival; // the values are integers 1 and 2
        off += c;
    }
    TEST_ASSERT_EQUAL_size_t(len, off);
    TEST_ASSERT_EQUAL_INT64(3, seen_vals);

    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)"~2\r\n", 4, &r, &c));
    TEST_ASSERT_EQUAL(RESP_SET, r.type);
    TEST_ASSERT_EQUAL_INT64(2, r.count);
    TEST_ASSERT_TRUE(resp_parse((const uint8_t *)">3\r\n", 4, &r, &c));
    TEST_ASSERT_EQUAL(RESP_PUSH, r.type);
    TEST_ASSERT_EQUAL_INT64(3, r.count);
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
    RUN_TEST(test_encode_guard_subconditions);
    RUN_TEST(test_parse_guard_subconditions_and_edges);
    RUN_TEST(test_parse_resp3_null_bool);
    RUN_TEST(test_parse_resp3_double);
    RUN_TEST(test_parse_resp3_bignum_bulkerr_verbatim);
    RUN_TEST(test_parse_resp3_map_set_push);
    return UNITY_END();
}
