// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the webhook builders (services/webhook): IFTTT URL + payload
// formatting, JSON escaping, omitted values, and fail-closed overflow. Firing
// goes through the http_client (ESP32) and is not exercised here.

#include "services/webhook/webhook.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_ifttt_url()
{
    char buf[160];
    int n = dws_ifttt_url("button_pressed", "abc123", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("https://maker.ifttt.com/trigger/button_pressed/with/key/abc123", buf);
}

void test_payload_three_values()
{
    char buf[128];
    int n = dws_ifttt_payload("a", "b", "c", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("{\"value1\":\"a\",\"value2\":\"b\",\"value3\":\"c\"}", buf);
}

void test_payload_omits_nulls()
{
    char buf[128];
    dws_ifttt_payload("only1", nullptr, nullptr, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"value1\":\"only1\"}", buf);

    dws_ifttt_payload(nullptr, "mid", nullptr, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"value2\":\"mid\"}", buf);

    dws_ifttt_payload(nullptr, nullptr, nullptr, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{}", buf);
}

void test_payload_escapes_json()
{
    char buf[128];
    dws_ifttt_payload("he said \"hi\"", "a\\b", nullptr, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"value1\":\"he said \\\"hi\\\"\",\"value2\":\"a\\\\b\"}", buf);
}

void test_overflow_fails_closed()
{
    char buf[8];
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_url("event", "key", buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_payload("aaaa", "bbbb", "cccc", buf, sizeof(buf)));
}

void test_ifttt_trigger_and_post_stub()
{
    // Host build (no HTTP client): webhook_post is a -1 stub; ifttt_trigger builds url+payload then posts.
    TEST_ASSERT_EQUAL_INT(-1, dws_webhook_post("http://x/y", "{}"));
    TEST_ASSERT_EQUAL_INT(-1, dws_ifttt_trigger("evt", "key", "1", "2", "3"));
}

// Null / zero-cap argument guards on the pure builders: fail closed, clearing the output
// buffer when one is provided.
void test_builder_arg_guards()
{
    char buf[64];
    buf[0] = 'x';
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_url(nullptr, "k", buf, sizeof(buf)));         // null event
    TEST_ASSERT_EQUAL_STRING("", buf);                                               // cleared
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_url("e", nullptr, buf, sizeof(buf)));         // null key
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_url("e", "k", nullptr, 10));                  // null out (no clear)
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_url("e", "k", buf, 0));                       // zero cap
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_payload("a", nullptr, nullptr, nullptr, 64)); // null out
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_payload("a", nullptr, nullptr, buf, 0));      // zero cap
}

// put_escaped fails closed when the escaped value would overrun the buffer, both on a
// plain character and on an escape sequence landing at the boundary.
void test_payload_escape_overflow_fails_closed()
{
    char buf[16];
    // "{\"value1\":\"" is 11 chars; a 10-char plain value overruns mid-escape-loop.
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_payload("aaaaaaaaaa", nullptr, nullptr, buf, sizeof(buf)));
    // A value whose escape ('"' -> two bytes) lands with < 2 bytes left also fails closed.
    TEST_ASSERT_EQUAL_INT(0, dws_ifttt_payload("aaa\"", nullptr, nullptr, buf, sizeof(buf)));
}

// dws_ifttt_trigger returns -1 when the url or payload cannot be built (too long for its
// fixed stack buffer), before any post is attempted.
void test_trigger_build_failures()
{
    char bigev[200];
    memset(bigev, 'e', 190);
    bigev[190] = '\0';
    TEST_ASSERT_EQUAL_INT(-1, dws_ifttt_trigger(bigev, "k", "1", nullptr, nullptr)); // url overflows url[160]

    char bigval[400];
    memset(bigval, 'v', 350);
    bigval[350] = '\0';
    TEST_ASSERT_EQUAL_INT(-1, dws_ifttt_trigger("e", "k", bigval, nullptr, nullptr)); // payload overflows body[256]
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ifttt_url);
    RUN_TEST(test_payload_three_values);
    RUN_TEST(test_payload_omits_nulls);
    RUN_TEST(test_payload_escapes_json);
    RUN_TEST(test_overflow_fails_closed);
    RUN_TEST(test_ifttt_trigger_and_post_stub);
    RUN_TEST(test_builder_arg_guards);
    RUN_TEST(test_payload_escape_overflow_fails_closed);
    RUN_TEST(test_trigger_build_failures);
    return UNITY_END();
}
