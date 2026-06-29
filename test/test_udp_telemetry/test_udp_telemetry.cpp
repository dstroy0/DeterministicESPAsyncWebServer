// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the UDP telemetry line builder (services/udp_telemetry): the
// InfluxDB line-protocol formatting and fail-closed overflow. The UDP cast is
// ESP32-only (a no-op on host).

#include "services/udp_telemetry/udp_telemetry.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_int_and_uint_fields()
{
    char buf[128];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "env");
    detws_line_add_int(&l, "rssi", -42);
    detws_line_add_uint(&l, "uptime", 1234u);
    TEST_ASSERT_TRUE(detws_line_ok(&l));
    TEST_ASSERT_EQUAL_STRING("env rssi=-42i,uptime=1234u", buf); // signed 'i' vs unsigned 'u'
    TEST_ASSERT_EQUAL_size_t(strlen("env rssi=-42i,uptime=1234u"), detws_line_len(&l));
}

void test_float_field()
{
    char buf[64];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "weather");
    detws_line_add_float(&l, "temp", 21.5f, 1);
    detws_line_add_float(&l, "hum", 60.25f, 2);
    TEST_ASSERT_TRUE(detws_line_ok(&l));
    TEST_ASSERT_EQUAL_STRING("weather temp=21.5,hum=60.25", buf);
}

void test_no_fields_not_ok()
{
    char buf[32];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "m");
    TEST_ASSERT_FALSE(detws_line_ok(&l)); // measurement only, no fields
    TEST_ASSERT_EQUAL_STRING("m", buf);
}

void test_overflow_fails_closed()
{
    char buf[16];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "measurement_with_a_long_name");
    detws_line_add_int(&l, "x", 1);
    TEST_ASSERT_FALSE(detws_line_ok(&l)); // did not fit
}

// Tags sit between the measurement and the fields (series key); a trailing
// timestamp is space-separated after the fields.
void test_tags_and_timestamp()
{
    char buf[128];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "env");
    detws_line_add_tag(&l, "host", "esp32-1");
    detws_line_add_tag(&l, "room", "lab");
    detws_line_add_int(&l, "rssi", -42);
    detws_line_set_timestamp(&l, 1465839830100400200LL);
    TEST_ASSERT_TRUE(detws_line_ok(&l));
    TEST_ASSERT_EQUAL_STRING("env,host=esp32-1,room=lab rssi=-42i 1465839830100400200", buf);
}

// Tag keys/values escape comma, equals and space (line protocol special chars).
void test_tag_escaping()
{
    char buf[128];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "m");
    detws_line_add_tag(&l, "a b", "x,y=z");
    detws_line_add_int(&l, "f", 1);
    TEST_ASSERT_TRUE(detws_line_ok(&l));
    TEST_ASSERT_EQUAL_STRING("m,a\\ b=x\\,y\\=z f=1i", buf);
}

// A tag added after a field is a misuse -> the line fails closed.
void test_tag_after_field_fails_closed()
{
    char buf[64];
    DetwsLine l;
    detws_line_init(&l, buf, sizeof(buf), "m");
    detws_line_add_int(&l, "f", 1);
    detws_line_add_tag(&l, "late", "nope"); // invalid ordering
    TEST_ASSERT_FALSE(detws_line_ok(&l));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_int_and_uint_fields);
    RUN_TEST(test_float_field);
    RUN_TEST(test_no_fields_not_ok);
    RUN_TEST(test_overflow_fails_closed);
    RUN_TEST(test_tags_and_timestamp);
    RUN_TEST(test_tag_escaping);
    RUN_TEST(test_tag_after_field_fails_closed);
    return UNITY_END();
}
