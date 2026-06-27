// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the guardrails core (services/guardrails): the threshold
// evaluator and the JSON serializer. Sampling the live counters is ESP32-only.

#include "services/guardrails/guardrails.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_eval_all_clear()
{
    DetwsHealth h = {20000, 15000, 10000, 2048};
    TEST_ASSERT_EQUAL_UINT8(DETWS_BREACH_NONE, detws_guardrail_eval(&h, 8192, 4096, 512));
}

void test_eval_heap_breach()
{
    DetwsHealth h = {4000, 3000, 8000, 2048}; // free_heap below 8192
    TEST_ASSERT_EQUAL_UINT8(DETWS_BREACH_HEAP, detws_guardrail_eval(&h, 8192, 4096, 512));
}

void test_eval_frag_and_stack()
{
    DetwsHealth h = {20000, 15000, 1000, 256}; // largest block < 4096, stack < 512
    uint8_t b = detws_guardrail_eval(&h, 8192, 4096, 512);
    TEST_ASSERT_EQUAL_UINT8(DETWS_BREACH_FRAG | DETWS_BREACH_STACK, b);
}

void test_eval_all_breached()
{
    DetwsHealth h = {100, 50, 100, 100};
    uint8_t b = detws_guardrail_eval(&h, 8192, 4096, 512);
    TEST_ASSERT_EQUAL_UINT8(DETWS_BREACH_HEAP | DETWS_BREACH_FRAG | DETWS_BREACH_STACK, b);
}

void test_json()
{
    DetwsHealth h = {20000, 15000, 10000, 2048};
    char buf[128];
    int n = detws_health_json(&h, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING(
        "{\"free_heap\":20000,\"min_free_heap\":15000,\"largest_free_block\":10000,\"stack_free\":2048}", buf);
}

void test_json_small_buffer_fails_closed()
{
    DetwsHealth h = {20000, 15000, 10000, 2048};
    char buf[8];
    TEST_ASSERT_EQUAL_INT(0, detws_health_json(&h, buf, sizeof(buf)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_eval_all_clear);
    RUN_TEST(test_eval_heap_breach);
    RUN_TEST(test_eval_frag_and_stack);
    RUN_TEST(test_eval_all_breached);
    RUN_TEST(test_json);
    RUN_TEST(test_json_small_buffer_fails_closed);
    return UNITY_END();
}
