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

// Writer: null strings are no-ops, the CR/BS/FF escapes, and an unbalanced close
// clears ok().
void test_writer_null_and_remaining_escapes()
{
    char buf[32];
    JsonWriter w(buf, sizeof(buf));
    w.begin_array();
    w.str(nullptr); // put_escaped(nullptr) is a no-op
    w.raw(nullptr); // put_raw(nullptr) is a no-op
    w.end_array();
    TEST_ASSERT_TRUE(w.ok()); // no overrun / crash

    char b2[16];
    JsonWriter e(b2, sizeof(b2));
    e.str("\r\b\f");
    TEST_ASSERT_TRUE(e.ok());
    TEST_ASSERT_EQUAL_STRING("\"\\r\\b\\f\"", e.c_str());

    char b3[16];
    JsonWriter u(b3, sizeof(b3));
    u.end_object(); // nothing open -> unbalanced
    TEST_ASSERT_FALSE(u.ok());
}

// Reader guards: null out / null json / zero capacity are rejected.
void test_reader_null_guards()
{
    char out[8];
    long v = 0;
    bool b = false;
    TEST_ASSERT_FALSE(json_get_str(kBody, "name", nullptr, 8));
    TEST_ASSERT_FALSE(json_get_str(kBody, "name", out, 0));
    TEST_ASSERT_FALSE(json_get_int(kBody, "port", nullptr));
    TEST_ASSERT_FALSE(json_get_bool(kBody, "on", nullptr));
    TEST_ASSERT_FALSE(json_get_str(nullptr, "name", out, 8));
    TEST_ASSERT_FALSE(json_get_int(nullptr, "port", &v));
    TEST_ASSERT_FALSE(json_get_bool(nullptr, "on", &b));
}

// Reader decodes every backslash escape, treating an unknown escape literally.
void test_reader_all_escapes()
{
    const char *body = "{\"s\":\"\\t\\r\\b\\f\\\\\\/\\q\"}"; // \t \r \b \f \\ \/ \q
    char out[16];
    TEST_ASSERT_TRUE(json_get_str(body, "s", out, sizeof(out)));
    const char expect[] = {'\t', '\r', '\b', '\f', '\\', '/', 'q', '\0'};
    TEST_ASSERT_EQUAL_STRING(expect, out);
}

// \uXXXX with lower- and upper-case hex letters each decode to their UTF-8 encoding.
void test_reader_unicode_hex_case()
{
    const char *body = "{\"a\":\"\\u00ab\\u00CD\"}"; // U+00AB, U+00CD -> two 2-byte UTF-8 sequences
    char out[8];
    TEST_ASSERT_TRUE(json_get_str(body, "a", out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xC2, (unsigned char)out[0]); // U+00AB
    TEST_ASSERT_EQUAL_HEX8(0xAB, (unsigned char)out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xC3, (unsigned char)out[2]); // U+00CD
    TEST_ASSERT_EQUAL_HEX8(0x8D, (unsigned char)out[3]);
    TEST_ASSERT_EQUAL_UINT8(0, (unsigned char)out[4]);
}

// \uXXXX above the BMP one-byte range emits multi-byte UTF-8, and a UTF-16 surrogate pair
// joins into one 4-byte code point.
void test_reader_unicode_utf8_multibyte()
{
    char out[16];
    // U+20AC EURO SIGN -> 3-byte UTF-8 E2 82 AC.
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\u20AC\"}", "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xE2, (unsigned char)out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x82, (unsigned char)out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xAC, (unsigned char)out[2]);
    TEST_ASSERT_EQUAL_UINT8(0, (unsigned char)out[3]);
    // U+1F600 GRINNING FACE as the surrogate pair D83D DE00 -> 4-byte UTF-8 F0 9F 98 80.
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\uD83D\\uDE00\"}", "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xF0, (unsigned char)out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x9F, (unsigned char)out[1]);
    TEST_ASSERT_EQUAL_HEX8(0x98, (unsigned char)out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x80, (unsigned char)out[3]);
    TEST_ASSERT_EQUAL_UINT8(0, (unsigned char)out[4]);
}

// An unpaired surrogate (high with no low, or a lone low) becomes U+FFFD (EF BF BD); the
// full UTF-8 sequence must fit or the string truncates cleanly at the boundary.
void test_reader_unicode_surrogate_edges()
{
    char out[16];
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\uD800Z\"}", "u", out, sizeof(out))); // high, no low
    TEST_ASSERT_EQUAL_HEX8(0xEF, (unsigned char)out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBF, (unsigned char)out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xBD, (unsigned char)out[2]);
    TEST_ASSERT_EQUAL_HEX8('Z', (unsigned char)out[3]);
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\uDC00\"}", "u", out, sizeof(out))); // lone low
    TEST_ASSERT_EQUAL_HEX8(0xEF, (unsigned char)out[0]);
    // A 4-byte sequence that will not fit truncates before it rather than writing a partial char.
    char tiny[3];
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\uD83D\\uDE00\"}", "u", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_UINT8(0, (unsigned char)tiny[0]); // nothing fit -> empty, still NUL-terminated
}

// json_get_bool decodes false as well as true.
void test_reader_false_bool()
{
    bool b = true;
    TEST_ASSERT_TRUE(json_get_bool("{\"f\":false}", "f", &b));
    TEST_ASSERT_FALSE(b);
}

// Malformed top-level objects are rejected without over-reading.
void test_reader_malformed()
{
    char out[8];
    long v = 0;
    TEST_ASSERT_FALSE(json_get_str("{\"name", "x", out, sizeof(out)));   // unterminated key
    TEST_ASSERT_FALSE(json_get_int("{\"a\":{\"b\":1", "z", &v));         // unterminated value object
    TEST_ASSERT_FALSE(json_get_str("{\"a\" 5}", "a", out, sizeof(out))); // key with no ':'
}

// A non-object top level, an empty object, and a non-string member name each yield no match.
void test_reader_non_object_and_bad_member()
{
    char out[16];
    TEST_ASSERT_FALSE(json_get_str("[1,2,3]", "k", out, sizeof(out))); // top level is not '{'
    TEST_ASSERT_FALSE(json_get_str("{}", "k", out, sizeof(out)));      // empty object -> '}'
    TEST_ASSERT_FALSE(json_get_str("{42:1}", "k", out, sizeof(out)));  // member name is not a string
}

// json_get_int rejects a quoted (string) value and a value with no digits.
void test_reader_int_rejects_string_and_nondigits()
{
    long v = 0;
    TEST_ASSERT_FALSE(json_get_int("{\"n\":\"42\"}", "n", &v)); // "42" is a string, not a bare number
    TEST_ASSERT_FALSE(json_get_int("{\"n\":xyz}", "n", &v));    // no digits parse
}

// A valid \uXXXX above 0xFF now emits UTF-8; malformed / short hex still collapses to '?'.
void test_reader_unicode_escape_invalid_and_wide()
{
    char out[16];
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\u01F6\"}", "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0xC7, (unsigned char)out[0]); // U+01F6 -> 2-byte UTF-8 C7 B6
    TEST_ASSERT_EQUAL_HEX8(0xB6, (unsigned char)out[1]);
    TEST_ASSERT_EQUAL_UINT8(0, (unsigned char)out[2]);
    // Invalid / short \u emits '?' without consuming the bad chars, so they trail as literals.
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\uZZZZ\"}", "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("?ZZZZ", out); // invalid hex digit
    TEST_ASSERT_TRUE(json_get_str("{\"u\":\"\\u0A\"}", "u", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("?0A", out); // short \u
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_reader_non_object_and_bad_member);
    RUN_TEST(test_reader_int_rejects_string_and_nondigits);
    RUN_TEST(test_reader_unicode_escape_invalid_and_wide);
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
    RUN_TEST(test_writer_null_and_remaining_escapes);
    RUN_TEST(test_reader_null_guards);
    RUN_TEST(test_reader_all_escapes);
    RUN_TEST(test_reader_unicode_hex_case);
    RUN_TEST(test_reader_unicode_utf8_multibyte);
    RUN_TEST(test_reader_unicode_surrogate_edges);
    RUN_TEST(test_reader_false_bool);
    RUN_TEST(test_reader_malformed);
    return UNITY_END();
}
