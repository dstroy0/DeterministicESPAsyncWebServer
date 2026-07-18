// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for dws_prov_form_field(): the x-www-form-urlencoded field
// extractor + URL-decoder used by the WiFi provisioning captive portal. The
// rest of the provisioning module is ESP32-only (softAP / lwIP UDP / NVS).

#include "services/provisioning_service/provisioning_service.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Plain fields are extracted by name.
void test_plain_fields()
{
    char v[64];
    TEST_ASSERT_TRUE(dws_prov_form_field("ssid=MyAP&psk=secret", "ssid", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("MyAP", v);
    TEST_ASSERT_TRUE(dws_prov_form_field("ssid=MyAP&psk=secret", "psk", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("secret", v);
}

// '+' decodes to space and %XX to the byte; value may be the last field.
void test_url_decoding()
{
    char v[64];
    TEST_ASSERT_TRUE(dws_prov_form_field("ssid=My+AP&psk=p%40ss%21", "ssid", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("My AP", v);
    TEST_ASSERT_TRUE(dws_prov_form_field("ssid=My+AP&psk=p%40ss%21", "psk", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("p@ss!", v);
}

// A missing field returns false and an empty string.
void test_missing_field()
{
    char v[64];
    TEST_ASSERT_FALSE(dws_prov_form_field("ssid=x", "psk", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("", v);
}

// The key matches only whole field names, not substrings of another field.
void test_no_substring_match()
{
    char v[64];
    TEST_ASSERT_TRUE(dws_prov_form_field("myssid=wrong&ssid=right", "ssid", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("right", v);
}

// The output is bounded by cap and always null-terminated.
void test_capacity_bound()
{
    char v[4];
    TEST_ASSERT_TRUE(dws_prov_form_field("ssid=abcdef", "ssid", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("abc", v); // 3 chars + null
}

void test_form_field_null_guards()
{
    // Any null argument (or zero cap) fails closed and leaves a writable out empty.
    char v[8] = "x";
    TEST_ASSERT_FALSE(dws_prov_form_field(nullptr, "ssid", v, sizeof(v)));
    TEST_ASSERT_EQUAL_STRING("", v);
    TEST_ASSERT_FALSE(dws_prov_form_field("ssid=x", nullptr, v, sizeof(v)));
    TEST_ASSERT_FALSE(dws_prov_form_field("ssid=x", "ssid", nullptr, sizeof(v)));
    TEST_ASSERT_FALSE(dws_prov_form_field("ssid=x", "ssid", v, 0));
}

void test_host_provisioning_stubs()
{
    // On host there is no NVS/WiFi: load reports no stored creds and clears the buffers; clear no-ops.
    char ssid[8] = "x", psk[8] = "y";
    TEST_ASSERT_FALSE(dws_provisioning_load(ssid, sizeof(ssid), psk, sizeof(psk)));
    TEST_ASSERT_EQUAL_STRING("", ssid);
    TEST_ASSERT_EQUAL_STRING("", psk);
    dws_provisioning_clear(); // no-op, must not crash
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_plain_fields);
    RUN_TEST(test_url_decoding);
    RUN_TEST(test_missing_field);
    RUN_TEST(test_no_substring_match);
    RUN_TEST(test_capacity_bound);
    RUN_TEST(test_form_field_null_guards);
    RUN_TEST(test_host_provisioning_stubs);
    return UNITY_END();
}
