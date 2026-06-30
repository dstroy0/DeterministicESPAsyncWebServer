// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the GraphQL query subset (services/graphql): selection shaping,
// nesting, arguments collected along the path, scalar types + JSON escaping, the
// `query`/anonymous forms, and the fail-closed paths (parse error, unsupported
// operation, depth/overflow limits).

#include "services/graphql/graphql.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

// Demo schema: a flat handful of fields + a nested `device` object and an
// argument-driven `sensor`/`greet`.
static bool resolver(const char *path, const DetwsGqlArgs *args, DetwsGqlValue *out)
{
    if (!strcmp(path, "name"))
    {
        out->type = DETWS_GQL_STR;
        out->s = "esp32";
        return true;
    }
    if (!strcmp(path, "uptime"))
    {
        out->type = DETWS_GQL_INT;
        out->i = 12345;
        return true;
    }
    if (!strcmp(path, "temp"))
    {
        out->type = DETWS_GQL_FLOAT;
        out->f = 21.5;
        return true;
    }
    if (!strcmp(path, "online"))
    {
        out->type = DETWS_GQL_BOOL;
        out->b = true;
        return true;
    }
    if (!strcmp(path, "device.name"))
    {
        out->type = DETWS_GQL_STR;
        out->s = "dev1";
        return true;
    }
    if (!strcmp(path, "device.uptime"))
    {
        out->type = DETWS_GQL_INT;
        out->i = 99;
        return true;
    }
    if (!strcmp(path, "sensor.value"))
    {
        long long id = 0;
        out->type = DETWS_GQL_INT;
        out->i = detws_gql_arg_int(args, "id", &id) ? id * 10 : -1;
        return true;
    }
    if (!strcmp(path, "greet"))
    {
        const char *who = "?";
        detws_gql_arg_str(args, "name", &who);
        static char b[64];
        snprintf(b, sizeof(b), "hi %s", who);
        out->type = DETWS_GQL_STR;
        out->s = b;
        return true;
    }
    if (!strcmp(path, "flag")) // reads a bool argument
    {
        bool on = false;
        detws_gql_arg_bool(args, "on", &on);
        out->type = DETWS_GQL_BOOL;
        out->b = on;
        return true;
    }
    if (!strcmp(path, "ctrl")) // a string holding a control char (forces \uXXXX)
    {
        static const char c[] = {'a', 0x01, 'b', '\0'};
        out->type = DETWS_GQL_STR;
        out->s = c;
        return true;
    }
    return false; // -> null
}

static char out[1024];

static const char *run(const char *q)
{
    int rc = detws_graphql_execute(q, strlen(q), resolver, out, sizeof(out));
    (void)rc;
    return out;
}

static int run_rc(const char *q)
{
    return detws_graphql_execute(q, strlen(q), resolver, out, sizeof(out));
}

void setUp()
{
    memset(out, 0, sizeof(out));
}
void tearDown()
{
}

void test_flat_selection()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"name\":\"esp32\",\"uptime\":12345}}", run("{ name uptime }"));
}

void test_selection_is_honored()
{
    // Only the requested field appears.
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"name\":\"esp32\"}}", run("{ name }"));
}

void test_nested_object()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"device\":{\"name\":\"dev1\",\"uptime\":99}}}",
                             run("{ device { name uptime } }"));
}

void test_args_collected_along_path()
{
    // `id` is on the object `sensor`; the leaf resolver `sensor.value` reads it.
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"sensor\":{\"value\":70}}}", run("{ sensor(id: 7) { value } }"));
}

void test_scalar_types()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"temp\":21.5,\"online\":true}}", run("{ temp online }"));
}

void test_string_arg_and_escaping()
{
    // String arg is decoded, and the resolver's output string is JSON-escaped.
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"greet\":\"hi a\\\"b\"}}", run("{ greet(name: \"a\\\"b\") }"));
}

void test_unresolved_field_is_null()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"mystery\":null}}", run("{ mystery }"));
}

void test_query_keyword_and_name()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"online\":true}}", run("query Status { online }"));
}

void test_comments_and_commas()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"name\":\"esp32\",\"uptime\":12345}}", run("{ name, # a comment\n uptime }"));
}

void test_parse_error_reports_errors()
{
    int rc = detws_graphql_execute("{ name ", 7, resolver, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, rc);
    TEST_ASSERT_NOT_NULL(strstr(out, "\"errors\""));
}

void test_mutation_rejected()
{
    int rc = detws_graphql_execute("mutation { x }", 14, resolver, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, rc);
}

void test_depth_limit()
{
    int rc = detws_graphql_execute("{a{b{c{d{e{f{g}}}}}}}", 21, resolver, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_LIMIT, rc);
}

void test_overflow_fails_closed()
{
    char tiny[8];
    int rc = detws_graphql_execute("{ name uptime }", 15, resolver, tiny, sizeof(tiny));
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_OVERFLOW, rc);
}

void test_string_escapes_decoded()
{
    // \n \t \r \\ \/ and an unknown escape (\z) are all decoded by the arg lexer.
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_OK, run_rc("{ greet(name: \"a\\nb\\tc\\rd\\\\e\\/f\\z\") }"));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"greet\":\"hi a"));
}

void test_number_arg_variants_parse()
{
    // float, exponent, signed-exponent and negative-int argument values all parse
    // (the resolver only reads `id`, but every number branch is exercised).
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_OK, run_rc("{ sensor(id: 7, x: 1.5, y: 2e3, z: -4, w: 1.2e-2) { value } }"));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"value\":70"));
}

void test_bool_args()
{
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"flag\":true}}", run("{ flag(on: true) }"));
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"flag\":false}}", run("{ flag(on: false) }"));
}

void test_null_arg_value()
{
    // `null` parses; greet's name arg is then not a string, so it stays "?".
    TEST_ASSERT_EQUAL_STRING("{\"data\":{\"greet\":\"hi ?\"}}", run("{ greet(name: null) }"));
}

void test_control_char_is_unicode_escaped()
{
    TEST_ASSERT_NOT_NULL(strstr(run("{ ctrl }"), "\\u0001"));
}

void test_unterminated_string_arg_fails()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, run_rc("{ greet(name: \"oops) }"));
}

void test_arg_missing_colon_fails()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, run_rc("{ sensor(id 7) { value } }"));
}

void test_bad_arg_value_fails()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, run_rc("{ greet(name: @) }"));
}

void test_trailing_junk_fails()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, run_rc("{ name } junk"));
}

void test_long_field_name_hits_limit()
{
    char q[256];
    int n = 0;
    q[n++] = '{';
    q[n++] = ' ';
    for (int i = 0; i < 200; i++)
        q[n++] = 'a';
    q[n++] = ' ';
    q[n++] = '}';
    q[n] = '\0';
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_LIMIT, detws_graphql_execute(q, strlen(q), resolver, out, sizeof(out)));
}

void test_null_inputs_fail_closed()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, detws_graphql_execute(0, 0, resolver, out, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, detws_graphql_execute("{ name }", 8, resolver, 0, 0));
}

void test_unknown_operation_keyword_fails()
{
    TEST_ASSERT_EQUAL_INT(DETWS_GQL_ERR_PARSE, run_rc("subscription { name }"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_flat_selection);
    RUN_TEST(test_string_escapes_decoded);
    RUN_TEST(test_number_arg_variants_parse);
    RUN_TEST(test_bool_args);
    RUN_TEST(test_null_arg_value);
    RUN_TEST(test_control_char_is_unicode_escaped);
    RUN_TEST(test_unterminated_string_arg_fails);
    RUN_TEST(test_arg_missing_colon_fails);
    RUN_TEST(test_bad_arg_value_fails);
    RUN_TEST(test_trailing_junk_fails);
    RUN_TEST(test_long_field_name_hits_limit);
    RUN_TEST(test_null_inputs_fail_closed);
    RUN_TEST(test_unknown_operation_keyword_fails);
    RUN_TEST(test_selection_is_honored);
    RUN_TEST(test_nested_object);
    RUN_TEST(test_args_collected_along_path);
    RUN_TEST(test_scalar_types);
    RUN_TEST(test_string_arg_and_escaping);
    RUN_TEST(test_unresolved_field_is_null);
    RUN_TEST(test_query_keyword_and_name);
    RUN_TEST(test_comments_and_commas);
    RUN_TEST(test_parse_error_reports_errors);
    RUN_TEST(test_mutation_rejected);
    RUN_TEST(test_depth_limit);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
