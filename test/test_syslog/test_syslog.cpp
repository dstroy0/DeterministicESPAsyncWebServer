// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the RFC 5424 syslog client (dws_syslog_format formatter + dws_syslog_init /
// dws_syslog_log over the udp_transport host capture seam).

#include "network_drivers/transport/udp.h"
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
    size_t n = dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_INFO,
                                 "esp32", "app", "hello");
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    // PRI = 16*8 + 6 = 134
    TEST_ASSERT_EQUAL_STRING("<134>1 - esp32 app - - - hello", buf);
}

void test_pri_computation_varies()
{
    char buf[256];
    // daemon(3)*8 + err(3) = 27
    dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_DAEMON, SyslogSeverity::SYSLOG_ERR, "h", "a", "m");
    TEST_ASSERT_EQUAL_STRING("<27>1 - h a - - - m", buf);
    // local7(23)*8 + emerg(0) = 184
    dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_LOCAL7, SyslogSeverity::SYSLOG_EMERG, "h", "a", "m");
    TEST_ASSERT_EQUAL_STRING("<184>1 - h a - - - m", buf);
}

void test_nilvalue_for_empty_fields()
{
    char buf[256];
    dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_USER, SyslogSeverity::SYSLOG_DEBUG, nullptr, "",
                      "msg");
    // user(1)*8 + debug(7) = 15; empty hostname + appname -> "-"
    TEST_ASSERT_EQUAL_STRING("<15>1 - - - - - - msg", buf);
}

void test_empty_message_ok()
{
    char buf[256];
    size_t n = dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_NOTICE,
                                 "h", "a", nullptr);
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    // local0(16)*8 + notice(5) = 133; trailing MSG empty
    TEST_ASSERT_EQUAL_STRING("<133>1 - h a - - - ", buf);
}

void test_overflow_returns_zero()
{
    char buf[16]; // far too small for the header + message
    size_t n = dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_INFO,
                                 "esp32", "app", "a long message");
    TEST_ASSERT_EQUAL_UINT(0, n);
}

void test_length_matches_strlen()
{
    char buf[256];
    size_t n = dws_syslog_format(buf, sizeof(buf), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_INFO,
                                 "host", "app", "payload");
    TEST_ASSERT_EQUAL_UINT(strlen(buf), n);
}

// dws_syslog_init + dws_syslog_log format the record and hand it to dws_udp_sendto.
void test_init_and_log_captured()
{
    dws_udp_capture_enable();
    dws_udp_capture_reset();
    dws_syslog_init("192.168.1.1", 514, "host1", "myapp", SyslogFacility::SYSLOG_FAC_LOCAL0);
    TEST_ASSERT_TRUE(dws_syslog_log(SyslogSeverity::SYSLOG_INFO, "hello"));
    const char *expect = "<134>1 - host1 myapp - - - hello";
    TEST_ASSERT_EQUAL_UINT(strlen(expect), dws_udp_captured_len());
    TEST_ASSERT_EQUAL_MEMORY(expect, dws_udp_captured(), dws_udp_captured_len());
}

// With no server configured the client is not ready and sends nothing.
void test_log_not_ready_when_no_server()
{
    dws_udp_capture_enable();
    dws_udp_capture_reset();
    dws_syslog_init(nullptr, 514, "h", "a", SyslogFacility::SYSLOG_FAC_USER); // null server_ip -> copy_field empties it
    TEST_ASSERT_FALSE(dws_syslog_log(SyslogSeverity::SYSLOG_INFO, "x"));
    TEST_ASSERT_EQUAL_UINT(0, dws_udp_captured_len());
}

// A null output, a zero cap, and out-of-range PRI values are handled defensively.
void test_format_null_and_pri_clamp()
{
    char buf[64];
    // Guard clauses return 0.
    TEST_ASSERT_EQUAL_UINT(
        0, dws_syslog_format(nullptr, sizeof(buf), (SyslogFacility)0, (SyslogSeverity)0, "h", "a", "m"));
    TEST_ASSERT_EQUAL_UINT(0, dws_syslog_format(buf, 0, (SyslogFacility)0, (SyslogSeverity)0, "h", "a", "m"));
    // facility/severity are unsigned enums, so PRI is never negative; an over-range value cast in at the
    // boundary clamps to 191.
    dws_syslog_format(buf, sizeof(buf), (SyslogFacility)25, (SyslogSeverity)0, "h", "a", "m"); // 200 clamps to 191
    TEST_ASSERT_EQUAL_STRING("<191>1 - h a - - - m", buf);
}

// An over-long hostname is truncated to DWS_SYSLOG_FIELD_MAX - 1 characters.
void test_init_truncates_long_fields()
{
    dws_udp_capture_enable();
    dws_udp_capture_reset();
    char longname[DWS_SYSLOG_FIELD_MAX + 16];
    memset(longname, 'H', sizeof(longname) - 1);
    longname[sizeof(longname) - 1] = '\0';
    dws_syslog_init("10.0.0.1", 514, longname, "a", SyslogFacility::SYSLOG_FAC_LOCAL0);
    TEST_ASSERT_TRUE(dws_syslog_log(SyslogSeverity::SYSLOG_INFO, "m"));
    const char *sent = (const char *)dws_udp_captured();
    const char *host = strstr(sent, "1 - ") + 4; // start of the HOSTNAME field
    size_t hcount = 0;
    while (host[hcount] == 'H')
        hcount++;
    TEST_ASSERT_EQUAL_UINT((size_t)(DWS_SYSLOG_FIELD_MAX - 1), hcount);
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
    RUN_TEST(test_init_and_log_captured);
    RUN_TEST(test_log_not_ready_when_no_server);
    RUN_TEST(test_format_null_and_pri_clamp);
    RUN_TEST(test_init_truncates_long_fields);
    return UNITY_END();
}
