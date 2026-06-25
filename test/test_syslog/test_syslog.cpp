// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the RFC 5424 syslog line formatter (syslog_format). No sockets.

#include "services/syslog/syslog.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_pri_local0_info()
{
    char buf[256];
    size_t n = syslog_format(buf, sizeof(buf), SYSLOG_FAC_LOCAL0, SYSLOG_INFO, "esp32", "app", "hello");
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    // PRI = 16*8 + 6 = 134
    TEST_ASSERT_EQUAL_STRING("<134>1 - esp32 app - - - hello", buf);
}

void test_pri_computation_varies()
{
    char buf[256];
    // daemon(3)*8 + err(3) = 27
    syslog_format(buf, sizeof(buf), SYSLOG_FAC_DAEMON, SYSLOG_ERR, "h", "a", "m");
    TEST_ASSERT_EQUAL_STRING("<27>1 - h a - - - m", buf);
    // local7(23)*8 + emerg(0) = 184
    syslog_format(buf, sizeof(buf), SYSLOG_FAC_LOCAL7, SYSLOG_EMERG, "h", "a", "m");
    TEST_ASSERT_EQUAL_STRING("<184>1 - h a - - - m", buf);
}

void test_nilvalue_for_empty_fields()
{
    char buf[256];
    syslog_format(buf, sizeof(buf), SYSLOG_FAC_USER, SYSLOG_DEBUG, nullptr, "", "msg");
    // user(1)*8 + debug(7) = 15; empty hostname + appname -> "-"
    TEST_ASSERT_EQUAL_STRING("<15>1 - - - - - - msg", buf);
}

void test_empty_message_ok()
{
    char buf[256];
    size_t n = syslog_format(buf, sizeof(buf), SYSLOG_FAC_LOCAL0, SYSLOG_NOTICE, "h", "a", nullptr);
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    // local0(16)*8 + notice(5) = 133; trailing MSG empty
    TEST_ASSERT_EQUAL_STRING("<133>1 - h a - - - ", buf);
}

void test_overflow_returns_zero()
{
    char buf[16]; // far too small for the header + message
    size_t n = syslog_format(buf, sizeof(buf), SYSLOG_FAC_LOCAL0, SYSLOG_INFO, "esp32", "app", "a long message");
    TEST_ASSERT_EQUAL_UINT(0, n);
}

void test_length_matches_strlen()
{
    char buf[256];
    size_t n = syslog_format(buf, sizeof(buf), SYSLOG_FAC_LOCAL0, SYSLOG_INFO, "host", "app", "payload");
    TEST_ASSERT_EQUAL_UINT(strlen(buf), n);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_pri_local0_info);
    RUN_TEST(test_pri_computation_varies);
    RUN_TEST(test_nilvalue_for_empty_fields);
    RUN_TEST(test_empty_message_ok);
    RUN_TEST(test_overflow_returns_zero);
    RUN_TEST(test_length_matches_strlen);
    return UNITY_END();
}
