// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the WAMP codec (services/wamp): the message builders (JSON arrays over
// JsonWriter) and the positional array parser. Pure host tests.

#include "services/wamp/wamp.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_build_hello()
{
    char buf[128];
    size_t n = wamp_build_hello(buf, sizeof(buf), "realm1", "{\"roles\":{}}");
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING("[1,\"realm1\",{\"roles\":{}}]", buf);
}

void test_build_subscribe_default_options()
{
    char buf[128];
    size_t n = wamp_build_subscribe(buf, sizeof(buf), 713845233, "com.myapp.topic", nullptr);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING("[32,713845233,{},\"com.myapp.topic\"]", buf);
}

void test_build_publish_with_args()
{
    char buf[160];
    size_t n = wamp_build_publish(buf, sizeof(buf), 239714735, "com.myapp.temp", nullptr, "[24.5]", "{\"unit\":\"C\"}");
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING("[16,239714735,{},\"com.myapp.temp\",[24.5],{\"unit\":\"C\"}]", buf);
}

// kwargs without args still emits an empty positional Arguments list.
void test_build_publish_kwargs_only()
{
    char buf[160];
    size_t n = wamp_build_publish(buf, sizeof(buf), 1, "t", nullptr, nullptr, "{\"k\":1}");
    TEST_ASSERT_EQUAL_STRING("[16,1,{},\"t\",[],{\"k\":1}]", buf);
}

void test_build_call_and_register_and_yield()
{
    char buf[160];
    TEST_ASSERT_GREATER_THAN(
        0, (int)wamp_build_call(buf, sizeof(buf), 7814135, "com.myapp.add", nullptr, "[2,3]", nullptr));
    TEST_ASSERT_EQUAL_STRING("[48,7814135,{},\"com.myapp.add\",[2,3]]", buf);

    TEST_ASSERT_GREATER_THAN(0, (int)wamp_build_register(buf, sizeof(buf), 25349185, "com.myapp.add", nullptr));
    TEST_ASSERT_EQUAL_STRING("[64,25349185,{},\"com.myapp.add\"]", buf);

    TEST_ASSERT_GREATER_THAN(0, (int)wamp_build_yield(buf, sizeof(buf), 6131533, nullptr, "[5]", nullptr));
    TEST_ASSERT_EQUAL_STRING("[70,6131533,{},[5]]", buf);
}

void test_build_unsubscribe_and_goodbye()
{
    char buf[128];
    TEST_ASSERT_GREATER_THAN(0, (int)wamp_build_unsubscribe(buf, sizeof(buf), 85346237, 5512315355ULL));
    TEST_ASSERT_EQUAL_STRING("[34,85346237,5512315355]", buf);
    TEST_ASSERT_GREATER_THAN(
        0, (int)wamp_build_goodbye(buf, sizeof(buf), "wamp.close.goodbye_and_out", "{\"message\":\"bye\"}"));
    TEST_ASSERT_EQUAL_STRING("[6,{\"message\":\"bye\"},\"wamp.close.goodbye_and_out\"]", buf);
}

void test_build_overflow_fails_closed()
{
    char buf[8];
    TEST_ASSERT_EQUAL_size_t(0, wamp_build_subscribe(buf, sizeof(buf), 1, "com.myapp.topic", nullptr));
}

// Parse a WELCOME: [2, SessionId, Details].
void test_parse_type_and_id()
{
    const char *welcome = "[2, 9129137332, {\"roles\":{\"broker\":{}}}]";
    int type;
    TEST_ASSERT_TRUE(wamp_get_type(welcome, &type));
    TEST_ASSERT_EQUAL_INT(WAMP_WELCOME, type);
    uint64_t session;
    TEST_ASSERT_TRUE(wamp_get_uint(welcome, 1, &session));
    TEST_ASSERT_EQUAL_UINT64(9129137332ULL, session);
}

// Parse an EVENT: [36, SubscriptionId, PublicationId, Details, Arguments].
void test_parse_event_positions()
{
    const char *event = "[36,5512315355,4429313566,{},[\"hello\"]]";
    int type;
    uint64_t sub, pub;
    TEST_ASSERT_TRUE(wamp_get_type(event, &type));
    TEST_ASSERT_EQUAL_INT(WAMP_EVENT, type);
    TEST_ASSERT_TRUE(wamp_get_uint(event, 1, &sub));
    TEST_ASSERT_TRUE(wamp_get_uint(event, 2, &pub));
    TEST_ASSERT_EQUAL_UINT64(5512315355ULL, sub);
    TEST_ASSERT_EQUAL_UINT64(4429313566ULL, pub);
    // element 4 is the Arguments list, returned raw.
    const char *s;
    size_t n;
    TEST_ASSERT_TRUE(wamp_element(event, 4, &s, &n));
    TEST_ASSERT_EQUAL_size_t(9, n);
    TEST_ASSERT_EQUAL_MEMORY("[\"hello\"]", s, 9);
}

// The positional scanner must skip nested strings/objects with commas and brackets.
void test_parse_get_uri_and_nesting()
{
    const char *subscribed = "[33, 713845233, 5512315355]";
    uint64_t request, subscription;
    TEST_ASSERT_TRUE(wamp_get_uint(subscribed, 1, &request));
    TEST_ASSERT_TRUE(wamp_get_uint(subscribed, 2, &subscription));
    TEST_ASSERT_EQUAL_UINT64(713845233ULL, request);

    // A CALL with a tricky Options dict (commas, brackets, an escaped quote in a string).
    const char *call = "[48,1,{\"a\":[1,2],\"b\":\"x,]\\\"y\"},\"com.myapp.proc\",[7]]";
    int type;
    TEST_ASSERT_TRUE(wamp_get_type(call, &type));
    TEST_ASSERT_EQUAL_INT(WAMP_CALL, type);
    char uri[32];
    TEST_ASSERT_TRUE(wamp_get_uri(call, 3, uri, sizeof(uri)));
    TEST_ASSERT_EQUAL_STRING("com.myapp.proc", uri);
    uint64_t arg;
    const char *s;
    size_t n;
    TEST_ASSERT_TRUE(wamp_element(call, 4, &s, &n));
    TEST_ASSERT_EQUAL_MEMORY("[7]", s, 3);
    (void)arg;
}

void test_parse_malformed()
{
    int type;
    uint64_t v;
    TEST_ASSERT_FALSE(wamp_get_type("not an array", &type));
    TEST_ASSERT_FALSE(wamp_get_uint("[2,9129137332]", 5, &v)); // index past the end
    TEST_ASSERT_FALSE(wamp_get_uint("[2,\"notanumber\"]", 1, &v));
    char uri[8];
    TEST_ASSERT_FALSE(wamp_get_uri("[2,123]", 1, uri, sizeof(uri))); // not a quoted string
}

// A URI that does not fit the destination fails closed without touching the buffer;
// one that exactly fills it (body + NUL) succeeds.
void test_get_uri_dest_bounds()
{
    const char *call = "[48,1,{},\"com.myapp.long.procedure.name\",[]]";
    char small[8];
    memset(small, '#', sizeof(small));
    TEST_ASSERT_FALSE(wamp_get_uri(call, 3, small, sizeof(small))); // 26-char URI > 7
    TEST_ASSERT_EQUAL_CHAR('#', small[0]);                          // buffer untouched on failure

    // Exact fit: a 3-char URI needs 4 bytes (3 + NUL).
    char exact[4];
    TEST_ASSERT_TRUE(wamp_get_uri("[2,\"abc\"]", 1, exact, sizeof(exact)));
    TEST_ASSERT_EQUAL_STRING("abc", exact);

    // One byte short of exact fails closed.
    char tight[3];
    TEST_ASSERT_FALSE(wamp_get_uri("[2,\"abc\"]", 1, tight, sizeof(tight)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_hello);
    RUN_TEST(test_build_subscribe_default_options);
    RUN_TEST(test_build_publish_with_args);
    RUN_TEST(test_build_publish_kwargs_only);
    RUN_TEST(test_build_call_and_register_and_yield);
    RUN_TEST(test_build_unsubscribe_and_goodbye);
    RUN_TEST(test_build_overflow_fails_closed);
    RUN_TEST(test_parse_type_and_id);
    RUN_TEST(test_parse_event_positions);
    RUN_TEST(test_parse_get_uri_and_nesting);
    RUN_TEST(test_parse_malformed);
    RUN_TEST(test_get_uri_dest_bounds);
    return UNITY_END();
}
