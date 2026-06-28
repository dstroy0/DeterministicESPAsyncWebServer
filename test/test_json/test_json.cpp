// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the zero-heap JSON helper: JsonWriter (serialization) and the
// json_get_* top-level object readers.

#include "network_drivers/presentation/json/json.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ====================================================================
// Writer
// ====================================================================

void test_writer_simple_object()
{
    char buf[64];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.kv_str("status", "ok");
    w.kv_int("count", 3);
    w.end_object();
    TEST_ASSERT_TRUE(w.ok());
    TEST_ASSERT_EQUAL_STRING("{\"status\":\"ok\",\"count\":3}", w.c_str());
}

void test_writer_nested_and_array()
{
    char buf[96];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.key("a");
    w.begin_array();
    w.integer(1);
    w.integer(2);
    w.end_array();
    w.kv_bool("b", true);
    w.key("c");
    w.begin_object();
    w.kv_null("n");
    w.end_object();
    w.end_object();
    TEST_ASSERT_TRUE(w.ok());
    TEST_ASSERT_EQUAL_STRING("{\"a\":[1,2],\"b\":true,\"c\":{\"n\":null}}", w.c_str());
}

void test_writer_value_types()
{
    char buf[96];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.kv_int("i", -7);
    w.kv_uint("u", 42u);
    w.kv_bool("t", true);
    w.kv_bool("f", false);
    w.kv_null("z");
    w.kv_raw("r", "3.14");
    w.end_object();
    TEST_ASSERT_TRUE(w.ok());
    TEST_ASSERT_EQUAL_STRING("{\"i\":-7,\"u\":42,\"t\":true,\"f\":false,\"z\":null,\"r\":3.14}", w.c_str());
}

void test_writer_escapes_strings()
{
    char buf[64];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.kv_str("k", "a\"b\nc\t\\d");
    w.end_object();
    TEST_ASSERT_TRUE(w.ok());
    // a " b \n c \t \ d  ->  a\"b\nc\t\\d
    TEST_ASSERT_EQUAL_STRING("{\"k\":\"a\\\"b\\nc\\t\\\\d\"}", w.c_str());
}

void test_writer_control_char_unicode_escape()
{
    char buf[48];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.kv_str("c", "\x01"); // SOH -> 
    w.end_object();
    TEST_ASSERT_TRUE(w.ok());
    TEST_ASSERT_EQUAL_STRING("{\"c\":\"\\u0001\"}", w.c_str());
}

void test_writer_overflow_sets_not_ok_and_stays_terminated()
{
    char buf[8];
    JsonWriter w(buf, sizeof(buf));
    w.begin_object();
    w.kv_str("aaaa", "bbbb"); // cannot fit in 8 bytes
    w.end_object();
    TEST_ASSERT_FALSE(w.ok());
    TEST_ASSERT_TRUE(strlen(w.c_str()) < sizeof(buf)); // never overran the buffer
}

void test_writer_depth_overflow_sets_not_ok()
{
    char buf[64];
    JsonWriter w(buf, sizeof(buf));
    for (int i = 0; i < JSON_MAX_DEPTH + 1; i++)
        w.begin_object();
    TEST_ASSERT_FALSE(w.ok()); // nesting past JSON_MAX_DEPTH
}

// ====================================================================
// Reader
// ====================================================================

static const char *kBody = "{\"name\":\"ada\",\"port\":8080,\"on\":true,"
                           "\"nested\":{\"x\":\"deep\"},\"x\":\"shallow\"}";

void test_reader_get_string()
{
    char out[16];
    TEST_ASSERT_TRUE(json_get_str(kBody, "name", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("ada", out);
}

void test_reader_get_int()
{
    long v = 0;
    TEST_ASSERT_TRUE(json_get_int(kBody, "port", &v));
    TEST_ASSERT_EQUAL_INT(8080, v);
}

void test_reader_get_bool()
{
    bool b = false;
    TEST_ASSERT_TRUE(json_get_bool(kBody, "on", &b));
    TEST_ASSERT_TRUE(b);
}

void test_reader_only_matches_top_level_key()
{
    // "x" exists both nested and at top level; the top-level one must win.
    char out[16];
    TEST_ASSERT_TRUE(json_get_str(kBody, "x", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("shallow", out);
}

void test_reader_missing_key()
{
    char out[16];
    long v;
    bool b;
    TEST_ASSERT_FALSE(json_get_str(kBody, "nope", out, sizeof(out)));
    TEST_ASSERT_FALSE(json_get_int(kBody, "nope", &v));
    TEST_ASSERT_FALSE(json_get_bool(kBody, "nope", &b));
}

void test_reader_type_mismatch()
{
    long v;
    bool b;
    // "name" is a string, not an int or bool.
    TEST_ASSERT_FALSE(json_get_int(kBody, "name", &v));
    TEST_ASSERT_FALSE(json_get_bool(kBody, "name", &b));
}

void test_reader_unescapes_value()
{
    const char *body = "{\"msg\":\"line1\\nline2\",\"q\":\"a\\\"b\"}";
    char out[32];
    TEST_ASSERT_TRUE(json_get_str(body, "msg", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("line1\nline2", out);
    TEST_ASSERT_TRUE(json_get_str(body, "q", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("a\"b", out);
}

void test_reader_unicode_escape_to_byte()
{
    const char *body = "{\"u\":\"A\\u0042C\"}"; // B == 'B'
    char out[16];
    TEST_ASSERT_TRUE(json_get_str(body, "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("ABC", out);
}

void test_reader_truncates_to_capacity()
{
    char out[4]; // room for 3 chars + NUL
    TEST_ASSERT_TRUE(json_get_str(kBody, "name", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("ada", out); // "ada" is exactly 3
    char small[3];                        // room for 2 chars + NUL
    TEST_ASSERT_TRUE(json_get_str(kBody, "name", small, sizeof(small)));
    TEST_ASSERT_EQUAL_STRING("ad", small); // truncated, still terminated
}

void test_reader_negative_int()
{
    const char *body = "{\"t\":-12345}";
    long v = 0;
    TEST_ASSERT_TRUE(json_get_int(body, "t", &v));
    TEST_ASSERT_EQUAL_INT(-12345, v);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_writer_simple_object);
    RUN_TEST(test_writer_nested_and_array);
    RUN_TEST(test_writer_value_types);
    RUN_TEST(test_writer_escapes_strings);
    RUN_TEST(test_writer_control_char_unicode_escape);
    RUN_TEST(test_writer_overflow_sets_not_ok_and_stays_terminated);
    RUN_TEST(test_writer_depth_overflow_sets_not_ok);
    RUN_TEST(test_reader_get_string);
    RUN_TEST(test_reader_get_int);
    RUN_TEST(test_reader_get_bool);
    RUN_TEST(test_reader_only_matches_top_level_key);
    RUN_TEST(test_reader_missing_key);
    RUN_TEST(test_reader_type_mismatch);
    RUN_TEST(test_reader_unescapes_value);
    RUN_TEST(test_reader_unicode_escape_to_byte);
    RUN_TEST(test_reader_truncates_to_capacity);
    RUN_TEST(test_reader_negative_int);
    return UNITY_END();
}
