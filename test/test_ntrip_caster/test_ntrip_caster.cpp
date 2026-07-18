// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the NTRIP caster protocol codec (services/gnss/ntrip_caster): rover request parsing
// (mountpoint, NTRIP version, HTTP Basic auth), the stream-accept / error responses, and the RTCM
// source table (STR records + ENDSOURCETABLE, with a self-consistent Content-Length). Pure host tests.

#include "services/gnss/ntrip_caster.h"
#include "shared_primitives/numparse.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_parse_v1_stream_request()
{
    const char *req = "GET /BASE1 HTTP/1.0\r\nUser-Agent: NTRIP testclient/1.0\r\n\r\n";
    NtripRequest r;
    TEST_ASSERT_TRUE(ntrip_request_parse(req, strlen(req), &r));
    TEST_ASSERT_TRUE(r.complete);
    TEST_ASSERT_TRUE(r.is_get);
    TEST_ASSERT_EQUAL(NtripVersion::NTRIP_V1, r.version);
    TEST_ASSERT_EQUAL_STRING("BASE1", r.mountpoint);
    TEST_ASSERT_FALSE(r.want_sourcetable);
    TEST_ASSERT_NULL(r.auth_b64);
}

void test_parse_v2_request_detects_version()
{
    const char *req = "GET /BASE1 HTTP/1.1\r\n"
                      "Host: caster.example\r\n"
                      "Ntrip-Version: Ntrip/2.0\r\n"
                      "User-Agent: NTRIP rover/2\r\n\r\n";
    NtripRequest r;
    TEST_ASSERT_TRUE(ntrip_request_parse(req, strlen(req), &r));
    TEST_ASSERT_EQUAL(NtripVersion::NTRIP_V2, r.version);
    TEST_ASSERT_EQUAL_STRING("BASE1", r.mountpoint);
}

void test_parse_sourcetable_request()
{
    const char *req = "GET / HTTP/1.0\r\n\r\n";
    NtripRequest r;
    TEST_ASSERT_TRUE(ntrip_request_parse(req, strlen(req), &r));
    TEST_ASSERT_TRUE(r.want_sourcetable);
    TEST_ASSERT_EQUAL_STRING("", r.mountpoint);
}

void test_parse_extracts_basic_auth()
{
    // The parser spans the base64 token verbatim (it does not decode it).
    const char *req = "GET /M HTTP/1.0\r\nAuthorization: Basic dXNlcjpwYXNz\r\n\r\n";
    NtripRequest r;
    TEST_ASSERT_TRUE(ntrip_request_parse(req, strlen(req), &r));
    TEST_ASSERT_NOT_NULL(r.auth_b64);
    TEST_ASSERT_EQUAL_UINT16(12, r.auth_b64_len);
    TEST_ASSERT_EQUAL_INT(0, strncmp(r.auth_b64, "dXNlcjpwYXNz", 12));
}

void test_parse_incomplete_needs_more()
{
    const char *req = "GET /BASE1 HTTP/1.0\r\nUser-Agent: x\r\n"; // no blank line yet
    NtripRequest r;
    TEST_ASSERT_FALSE(ntrip_request_parse(req, strlen(req), &r));
}

void test_parse_rejects_non_get()
{
    const char *req = "POST /BASE1 HTTP/1.0\r\n\r\n";
    NtripRequest r;
    TEST_ASSERT_TRUE(ntrip_request_parse(req, strlen(req), &r)); // header block complete...
    TEST_ASSERT_FALSE(r.is_get);                                 // ...but not a GET
}

void test_stream_response_v1_v2()
{
    char buf[256];
    size_t n1 = ntrip_build_stream_response(buf, sizeof(buf), NtripVersion::NTRIP_V1);
    TEST_ASSERT_EQUAL_size_t(strlen("ICY 200 OK\r\n\r\n"), n1);
    TEST_ASSERT_EQUAL_STRING("ICY 200 OK\r\n\r\n", buf);

    size_t n2 = ntrip_build_stream_response(buf, sizeof(buf), NtripVersion::NTRIP_V2);
    TEST_ASSERT_TRUE(n2 > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "HTTP/1.1 200 OK\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Content-Type: gnss/data\r\n"));
    TEST_ASSERT_EQUAL_CHAR('\0', buf[n2]); // NUL-terminated at the returned length
}

void test_str_record_format()
{
    NtripMount m;
    memset(&m, 0, sizeof(m));
    m.mountpoint = "BASE1";
    m.identifier = "Lab roof";
    m.format_details = "1005(1),1006(10)";
    m.nav_system = "GPS";
    m.country = "USA";
    m.generator = "DWS";
    m.lat_deg = 37.77;
    m.lon_deg = -122.42;
    m.nmea_required = false;

    char rec[192];
    size_t n = ntrip_build_str_record(rec, sizeof(rec), &m);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("STR;BASE1;Lab roof;RTCM 3.3;1005(1),1006(10);0;GPS;none;USA;37.77;-122.42;0;0;"
                             "DWS;none;N;N;9600;",
                             rec);
    // A well-formed STR record has 19 fields (18 semicolons).
    int semis = 0;
    for (size_t i = 0; i < n; i++)
        if (rec[i] == ';')
            semis++;
    TEST_ASSERT_EQUAL_INT(18, semis);
}

void test_str_record_defaults_and_negative_small_lon()
{
    NtripMount m;
    memset(&m, 0, sizeof(m));
    m.mountpoint = "M";
    m.lat_deg = 0.0;
    m.lon_deg = -0.05; // exercises the "-0.05" small-negative path
    char rec[192];
    size_t n = ntrip_build_str_record(rec, sizeof(rec), &m);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(rec, ";GPS;"));        // nav-system default
    TEST_ASSERT_NOT_NULL(strstr(rec, ";1005(1);"));    // format-details default
    TEST_ASSERT_NOT_NULL(strstr(rec, ";DWS;"));        // generator default
    TEST_ASSERT_NOT_NULL(strstr(rec, ";0.00;-0.05;")); // lat/lon formatting incl. small negative
}

// Read the Content-Length header value out of a response.
static long content_length_of(const char *resp)
{
    const char *h = strstr(resp, "Content-Length:");
    if (!h)
        return -1;
    const char *end = h;
    return dws_strtol(h + 15, &end);
}

void test_sourcetable_has_records_and_correct_length()
{
    NtripMount mounts[2];
    memset(mounts, 0, sizeof(mounts));
    mounts[0].mountpoint = "BASE1";
    mounts[0].identifier = "Roof";
    mounts[0].lat_deg = 37.77;
    mounts[0].lon_deg = -122.42;
    mounts[1].mountpoint = "BASE2";
    mounts[1].identifier = "Field";
    mounts[1].lat_deg = 40.0;
    mounts[1].lon_deg = -105.0;

    char buf[1024];
    size_t n = ntrip_build_sourcetable(buf, sizeof(buf), NtripVersion::NTRIP_V1, mounts, 2);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "SOURCETABLE 200 OK\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "STR;BASE1;"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "STR;BASE2;"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "ENDSOURCETABLE\r\n"));

    // The advertised Content-Length must equal the actual body (everything after the blank line).
    const char *body = strstr(buf, "\r\n\r\n");
    TEST_ASSERT_NOT_NULL(body);
    body += 4;
    long declared = content_length_of(buf);
    TEST_ASSERT_EQUAL_INT((long)strlen(body), declared);
}

void test_sourcetable_v2_content_type()
{
    NtripMount m;
    memset(&m, 0, sizeof(m));
    m.mountpoint = "BASE1";
    char buf[512];
    size_t n = ntrip_build_sourcetable(buf, sizeof(buf), NtripVersion::NTRIP_V2, &m, 1);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "HTTP/1.1 200 OK\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Content-Type: gnss/sourcetable\r\n"));
}

void test_error_response()
{
    char buf[128];
    size_t n1 = ntrip_build_error_response(buf, sizeof(buf), NtripVersion::NTRIP_V1);
    TEST_ASSERT_TRUE(n1 > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "ERROR"));
    size_t n2 = ntrip_build_error_response(buf, sizeof(buf), NtripVersion::NTRIP_V2);
    TEST_ASSERT_TRUE(n2 > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "400 Bad Request"));
}

void test_unauthorized_response()
{
    char buf[128];
    size_t n1 = ntrip_build_unauthorized_response(buf, sizeof(buf), NtripVersion::NTRIP_V1);
    TEST_ASSERT_TRUE(n1 > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "Bad Password"));
    size_t n2 = ntrip_build_unauthorized_response(buf, sizeof(buf), NtripVersion::NTRIP_V2);
    TEST_ASSERT_TRUE(n2 > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "401 Unauthorized"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "WWW-Authenticate: Basic"));
}

void test_response_overflow_fails_closed()
{
    char tiny[4];
    TEST_ASSERT_EQUAL_size_t(0, ntrip_build_stream_response(tiny, sizeof(tiny), NtripVersion::NTRIP_V2));
    NtripMount m;
    memset(&m, 0, sizeof(m));
    m.mountpoint = "BASE1";
    TEST_ASSERT_EQUAL_size_t(0, ntrip_build_sourcetable(tiny, sizeof(tiny), NtripVersion::NTRIP_V1, &m, 1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_v1_stream_request);
    RUN_TEST(test_parse_v2_request_detects_version);
    RUN_TEST(test_parse_sourcetable_request);
    RUN_TEST(test_parse_extracts_basic_auth);
    RUN_TEST(test_parse_incomplete_needs_more);
    RUN_TEST(test_parse_rejects_non_get);
    RUN_TEST(test_stream_response_v1_v2);
    RUN_TEST(test_str_record_format);
    RUN_TEST(test_str_record_defaults_and_negative_small_lon);
    RUN_TEST(test_sourcetable_has_records_and_correct_length);
    RUN_TEST(test_sourcetable_v2_content_type);
    RUN_TEST(test_error_response);
    RUN_TEST(test_unauthorized_response);
    RUN_TEST(test_response_overflow_fails_closed);
    return UNITY_END();
}
