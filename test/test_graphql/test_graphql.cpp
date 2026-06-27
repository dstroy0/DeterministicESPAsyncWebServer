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
    return false; // -> null
}

static char out[1024];

static const char *run(const char *q)
{
    int rc = detws_graphql_execute(q, strlen(q), resolver, out, sizeof(out));
    (void)rc;
    return out;
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

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_flat_selection);
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
