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
    TEST_ASSERT_EQUAL_STRING("env rssi=-42i,uptime=1234i", buf);
    TEST_ASSERT_EQUAL_size_t(strlen("env rssi=-42i,uptime=1234i"), detws_line_len(&l));
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

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_int_and_uint_fields);
    RUN_TEST(test_float_field);
    RUN_TEST(test_no_fields_not_ok);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
