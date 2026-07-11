// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the CloudEvents v1.0 envelope (services/cloudevents): the
// structured-JSON builder and the binary-mode ce-* header reader. Pure host tests.

#include "network_drivers/presentation/http_parser/http_parser.h"
#include "services/cloudevents/cloudevents.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void feed_request(uint8_t slot, const char *raw)
{
    http_parser_reset(&http_pool[slot]);
    for (const char *p = raw; *p; p++)
        http_parser_feed(&http_pool[slot], (uint8_t)*p);
}

// A minimal event carries the three required attributes and specversion 1.0.
void test_build_minimal()
{
    CloudEvent ce = {};
    ce.id = "1001";
    ce.source = "/devices/esp32-1";
    ce.type = "com.example.sensor.reading";
    char buf[256];
    size_t n = cloudevents_build_json(buf, sizeof(buf), &ce);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"specversion\":\"1.0\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"id\":\"1001\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"source\":\"/devices/esp32-1\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"type\":\"com.example.sensor.reading\""));
    TEST_ASSERT_NULL(strstr(buf, "\"data\"")); // no data when none supplied
}

// A missing required attribute fails closed (returns 0).
void test_build_requires_id_source_type()
{
    char buf[256];
    CloudEvent a = {};
    a.source = "/s";
    a.type = "t"; // no id
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, sizeof(buf), &a));
    CloudEvent b = {};
    b.id = "1";
    b.type = "t"; // no source
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, sizeof(buf), &b));
    CloudEvent c = {};
    c.id = "1";
    c.source = "/s"; // no type
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, sizeof(buf), &c));
}

// data_json is emitted verbatim (a JSON value), with datacontenttype defaulting.
void test_build_with_json_data()
{
    CloudEvent ce = {};
    ce.id = "7";
    ce.source = "/s";
    ce.type = "t";
    ce.subject = "temp";
    ce.data_json = "{\"celsius\":23.5}";
    char buf[256];
    size_t n = cloudevents_build_json(buf, sizeof(buf), &ce);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"subject\":\"temp\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"datacontenttype\":\"application/json\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"data\":{\"celsius\":23.5}")); // verbatim, not a quoted string
}

// data_str is emitted as a JSON string (escaped).
void test_build_with_string_data()
{
    CloudEvent ce = {};
    ce.id = "8";
    ce.source = "/s";
    ce.type = "t";
    ce.datacontenttype = "text/plain";
    ce.data_str = "hi \"there\"";
    char buf[256];
    TEST_ASSERT_GREATER_THAN(0, (int)cloudevents_build_json(buf, sizeof(buf), &ce));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"datacontenttype\":\"text/plain\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"data\":\"hi \\\"there\\\"\"")); // quoted + escaped
}

// A too-small buffer fails closed (returns 0).
void test_build_overflow_fails_closed()
{
    CloudEvent ce = {};
    ce.id = "1";
    ce.source = "/s";
    ce.type = "t";
    char buf[16]; // far too small for the envelope
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, sizeof(buf), &ce));
}

// Binary mode: read an inbound event's ce-* headers.
void test_from_headers_binary_mode()
{
    feed_request(0, "POST /events HTTP/1.1\r\nHost: x\r\n"
                    "ce-id: abc-1\r\nce-source: /producer\r\nce-type: com.example.test\r\n"
                    "ce-subject: s1\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}");
    CloudEvent ce;
    TEST_ASSERT_TRUE(cloudevents_from_headers(&http_pool[0], &ce));
    TEST_ASSERT_EQUAL_STRING("abc-1", ce.id);
    TEST_ASSERT_EQUAL_STRING("/producer", ce.source);
    TEST_ASSERT_EQUAL_STRING("com.example.test", ce.type);
    TEST_ASSERT_EQUAL_STRING("s1", ce.subject);
    TEST_ASSERT_EQUAL_STRING("application/json", ce.datacontenttype);
}

// Missing a required ce-* header -> not a valid binary CloudEvent.
void test_from_headers_missing_required()
{
    feed_request(0, "POST /events HTTP/1.1\r\nHost: x\r\nce-id: abc-1\r\nce-source: /p\r\n\r\n"); // no ce-type
    CloudEvent ce;
    TEST_ASSERT_FALSE(cloudevents_from_headers(&http_pool[0], &ce));
}

// build_json argument guards, the datacontenttype-without-data branch, and the
// from_headers null guard.
void test_guards_and_datacontenttype_only()
{
    char buf[256];
    CloudEvent ce = {};
    ce.id = "1";
    ce.source = "/s";
    ce.type = "t";
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(nullptr, sizeof(buf), &ce)); // null buf
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, 0, &ce));               // zero cap
    TEST_ASSERT_EQUAL_size_t(0, cloudevents_build_json(buf, sizeof(buf), nullptr)); // null event

    // datacontenttype set but no data_json/data_str -> the third data branch emits only the type.
    ce.datacontenttype = "application/json";
    size_t n = cloudevents_build_json(buf, sizeof(buf), &ce);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"datacontenttype\":\"application/json\""));
    TEST_ASSERT_NULL(strstr(buf, "\"data\":")); // no data value emitted

    CloudEvent out;
    TEST_ASSERT_FALSE(cloudevents_from_headers(nullptr, &out)); // null request
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_minimal);
    RUN_TEST(test_build_requires_id_source_type);
    RUN_TEST(test_build_with_json_data);
    RUN_TEST(test_build_with_string_data);
    RUN_TEST(test_build_overflow_fails_closed);
    RUN_TEST(test_from_headers_binary_mode);
    RUN_TEST(test_from_headers_missing_required);
    RUN_TEST(test_guards_and_datacontenttype_only);
    return UNITY_END();
}
