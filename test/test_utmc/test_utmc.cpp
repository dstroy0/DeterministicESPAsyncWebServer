// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/utmc: the UTMC common-database request/response codec.

#include "services/utmc/utmc.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static bool has(const char *hay, const char *needle)
{
    return strstr(hay, needle) != nullptr;
}

void test_request(void)
{
    char buf[128];
    size_t n = detws_utmc_request("DET_042", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?><UTMCRequest><object id=\"DET_042\"/></UTMCRequest>", buf);
}

void test_response(void)
{
    char buf[256];
    detws_utmc_response("SIGN_7", "SLOW", Utmc::UTMC_QUALITY_GOOD, "2026-07-06T12:00:00Z", buf, sizeof(buf));
    TEST_ASSERT_TRUE(has(buf, "<UTMCResponse><object id=\"SIGN_7\""));
    TEST_ASSERT_TRUE(has(buf, "value=\"SLOW\""));
    TEST_ASSERT_TRUE(has(buf, "quality=\"0\""));
    TEST_ASSERT_TRUE(has(buf, "timestamp=\"2026-07-06T12:00:00Z\""));
}

void test_response_escapes(void)
{
    char buf[256];
    detws_utmc_response("A&B", "x<y", Utmc::UTMC_QUALITY_SUSPECT, "t", buf, sizeof(buf));
    TEST_ASSERT_TRUE(has(buf, "id=\"A&amp;B\""));
    TEST_ASSERT_TRUE(has(buf, "value=\"x&lt;y\""));
    TEST_ASSERT_TRUE(has(buf, "quality=\"1\""));
}

void test_parse_request(void)
{
    const char *req = "<?xml version=\"1.0\"?><UTMCRequest><object id=\"DET_042\"/></UTMCRequest>";
    char id[32];
    size_t n = detws_utmc_parse_request(req, strlen(req), id, sizeof(id));
    TEST_ASSERT_EQUAL_size_t(7, n);
    TEST_ASSERT_EQUAL_STRING("DET_042", id);
    // No id -> 0.
    const char *noid = "<UTMCRequest><object/></UTMCRequest>";
    TEST_ASSERT_EQUAL_size_t(0, detws_utmc_parse_request(noid, strlen(noid), id, sizeof(id)));
}

void test_overflow(void)
{
    char buf[16];
    TEST_ASSERT_EQUAL_size_t(0, detws_utmc_request("a-very-long-object-id-here", buf, sizeof(buf)));
}

void test_parse_request_guards()
{
    char out[64];
    TEST_ASSERT_EQUAL_size_t(0, detws_utmc_parse_request(nullptr, 10, out, sizeof(out))); // null xml
    const char *xml = "<x id=\"ABCDEFGHIJ\"/>";
    TEST_ASSERT_EQUAL_size_t(0, detws_utmc_parse_request(xml, strlen(xml), out, 4)); // id overflows out
    const char *unterm = "<x id=\"ABC";
    TEST_ASSERT_EQUAL_size_t(0, detws_utmc_parse_request(unterm, strlen(unterm), out, sizeof(out))); // unterminated
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_request);
    RUN_TEST(test_response);
    RUN_TEST(test_response_escapes);
    RUN_TEST(test_parse_request);
    RUN_TEST(test_overflow);
    RUN_TEST(test_parse_request_guards);
    return UNITY_END();
}
