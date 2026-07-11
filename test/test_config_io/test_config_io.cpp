// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for schema-driven config export/restore (services/config_io) over
// the host in-memory config store: export formatting, round-trip, unknown-key
// skipping, and fail-closed overflow.

#include "services/config_io/config_io.h"
#include "services/config_store/config_store.h"
#include <string.h>
#include <unity.h>

static const DetwsCfgField SCHEMA[] = {
    {"ssid", DetwsCfgType::DETWS_CFG_STR},
    {"port", DetwsCfgType::DETWS_CFG_U32},
    {"name", DetwsCfgType::DETWS_CFG_STR},
};
static const size_t N = sizeof(SCHEMA) / sizeof(SCHEMA[0]);

void setUp()
{
    detws_config_begin("t");
    detws_config_clear();
}
void tearDown()
{
}

void test_export_format()
{
    detws_config_begin("t");
    detws_config_set_str("ssid", "myssid");
    detws_config_set_u32("port", 8080);
    detws_config_set_str("name", "node1");
    char buf[256];
    int n = detws_config_export("t", SCHEMA, N, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("ssid=myssid\nport=8080\nname=node1\n", buf);
}

void test_round_trip()
{
    detws_config_begin("t");
    detws_config_set_str("ssid", "abc");
    detws_config_set_u32("port", 1234);
    detws_config_set_str("name", "x");
    char buf[256];
    detws_config_export("t", SCHEMA, N, buf, sizeof(buf));

    detws_config_clear(); // wipe
    TEST_ASSERT_EQUAL_UINT32(0, detws_config_get_u32("port", 0));

    int c = detws_config_import("t", SCHEMA, N, buf, strlen(buf));
    TEST_ASSERT_EQUAL_INT(3, c);
    char s[32];
    detws_config_get_str("ssid", s, sizeof(s), "");
    TEST_ASSERT_EQUAL_STRING("abc", s);
    TEST_ASSERT_EQUAL_UINT32(1234, detws_config_get_u32("port", 0));
}

void test_import_skips_unknown_keys()
{
    detws_config_begin("t");
    const char *txt = "port=42\nbogus=99\n";
    int c = detws_config_import("t", SCHEMA, N, txt, strlen(txt));
    TEST_ASSERT_EQUAL_INT(1, c); // only "port" is in the schema
    TEST_ASSERT_EQUAL_UINT32(42, detws_config_get_u32("port", 0));
}

void test_export_overflow_fails_closed()
{
    detws_config_begin("t");
    detws_config_set_str("ssid", "value");
    char buf[4];
    TEST_ASSERT_EQUAL_INT(0, detws_config_export("t", SCHEMA, N, buf, sizeof(buf)));
}

void test_export_import_null_guards()
{
    char out[128];
    DetwsCfgField fields[1] = {};
    TEST_ASSERT_EQUAL_INT(0, detws_config_export("ns", fields, 1, nullptr, sizeof(out))); // null out
    TEST_ASSERT_EQUAL_INT(0, detws_config_export("ns", nullptr, 1, out, sizeof(out)));    // null fields
    TEST_ASSERT_EQUAL_INT(0, detws_config_import("ns", nullptr, 1, "text", 4));           // null fields
    TEST_ASSERT_EQUAL_INT(0, detws_config_import("ns", fields, 1, nullptr, 0));           // null text
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_export_format);
    RUN_TEST(test_round_trip);
    RUN_TEST(test_import_skips_unknown_keys);
    RUN_TEST(test_export_overflow_fails_closed);
    RUN_TEST(test_export_import_null_guards);
    return UNITY_END();
}
