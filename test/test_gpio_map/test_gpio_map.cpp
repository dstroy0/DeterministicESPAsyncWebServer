// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the GPIO pin-mapper core (services/gpio_map): the direction
// names, the JSON serializer, the control-POST parser, and the output guard. The
// digital read / write are ESP32-only and no-ops on the host.

#include "services/gpio_map/gpio_map.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_dir_name()
{
    TEST_ASSERT_EQUAL_STRING("in", dws_gpio_dir_name(DWSGpioDir::DWS_GPIO_IN));
    TEST_ASSERT_EQUAL_STRING("in_pullup", dws_gpio_dir_name(DWSGpioDir::DWS_GPIO_IN_PULLUP));
    TEST_ASSERT_EQUAL_STRING("in_pulldown", dws_gpio_dir_name(DWSGpioDir::DWS_GPIO_IN_PULLDOWN));
    TEST_ASSERT_EQUAL_STRING("out", dws_gpio_dir_name(DWSGpioDir::DWS_GPIO_OUT));
    TEST_ASSERT_EQUAL_STRING("in", dws_gpio_dir_name((DWSGpioDir)99)); // unknown -> in
}

void test_json()
{
    DWSGpioPin pins[2] = {
        {2, "LED", DWSGpioDir::DWS_GPIO_OUT, 1},
        {0, "BOOT", DWSGpioDir::DWS_GPIO_IN_PULLUP, 0},
    };
    char buf[256];
    int n = dws_gpio_json(pins, 2, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("{\"pins\":[{\"pin\":2,\"label\":\"LED\",\"dir\":\"out\",\"level\":1},"
                             "{\"pin\":0,\"label\":\"BOOT\",\"dir\":\"in_pullup\",\"level\":0}]}",
                             buf);
}

void test_json_empty()
{
    char buf[64];
    int n = dws_gpio_json(nullptr, 0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(0, n); // null table -> empty string, 0
}

void test_json_small_buffer_fails_closed()
{
    DWSGpioPin pins[1] = {{2, "LED", DWSGpioDir::DWS_GPIO_OUT, 1}};
    char buf[8];
    TEST_ASSERT_EQUAL_INT(0, dws_gpio_json(pins, 1, buf, sizeof(buf)));
}

void test_parse_set()
{
    uint8_t pin = 0, level = 0;
    const char *b = "pin=2&level=1";
    TEST_ASSERT_TRUE(dws_gpio_parse_set(b, strlen(b), &pin, &level));
    TEST_ASSERT_EQUAL_UINT8(2, pin);
    TEST_ASSERT_EQUAL_UINT8(1, level);

    const char *b2 = "level=0&pin=13"; // order independent
    TEST_ASSERT_TRUE(dws_gpio_parse_set(b2, strlen(b2), &pin, &level));
    TEST_ASSERT_EQUAL_UINT8(13, pin);
    TEST_ASSERT_EQUAL_UINT8(0, level);

    const char *b3 = "pin=2&level=7"; // any nonzero level -> 1
    TEST_ASSERT_TRUE(dws_gpio_parse_set(b3, strlen(b3), &pin, &level));
    TEST_ASSERT_EQUAL_UINT8(1, level);
}

void test_parse_set_rejects_partial()
{
    uint8_t pin = 0, level = 0;
    const char *b = "pin=2"; // missing level
    TEST_ASSERT_FALSE(dws_gpio_parse_set(b, strlen(b), &pin, &level));
    const char *b2 = "level=1"; // missing pin
    TEST_ASSERT_FALSE(dws_gpio_parse_set(b2, strlen(b2), &pin, &level));
    const char *b3 = "pin=&level=1"; // no digits for pin
    TEST_ASSERT_FALSE(dws_gpio_parse_set(b3, strlen(b3), &pin, &level));
}

void test_parse_set_no_prefix_match()
{
    // "spin=2" must not satisfy the "pin" field (field-boundary check).
    uint8_t pin = 0, level = 0;
    const char *b = "spin=2&level=1";
    TEST_ASSERT_FALSE(dws_gpio_parse_set(b, strlen(b), &pin, &level));
}

void test_is_output()
{
    DWSGpioPin pins[2] = {
        {2, "LED", DWSGpioDir::DWS_GPIO_OUT, 0},
        {0, "BOOT", DWSGpioDir::DWS_GPIO_IN_PULLUP, 0},
    };
    TEST_ASSERT_TRUE(dws_gpio_is_output(pins, 2, 2));
    TEST_ASSERT_FALSE(dws_gpio_is_output(pins, 2, 0));  // input pin
    TEST_ASSERT_FALSE(dws_gpio_is_output(pins, 2, 99)); // not in table
}

void test_host_gpio_stubs()
{
    // Host build: the GPIO bind functions are no-ops (no digitalRead/Write).
    DWSGpioPin pins[1] = {};
    dws_gpio_begin_pins(pins, 1);
    dws_gpio_read(pins, 1);
    dws_gpio_write(0, 1);
    TEST_PASS();
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_dir_name);
    RUN_TEST(test_json);
    RUN_TEST(test_json_empty);
    RUN_TEST(test_json_small_buffer_fails_closed);
    RUN_TEST(test_parse_set);
    RUN_TEST(test_parse_set_rejects_partial);
    RUN_TEST(test_parse_set_no_prefix_match);
    RUN_TEST(test_is_output);
    RUN_TEST(test_host_gpio_stubs);
    return UNITY_END();
}
