// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the outbound HTTP client's pure core: URL parsing, request
// building, and response parsing (incl. chunked decode). No sockets.

#include "services/http_client/http_client.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ---- URL parsing ----------------------------------------------------------

void test_url_http_default()
{
    bool https;
    char host[64], path[64];
    uint16_t port;
    TEST_ASSERT_TRUE(
        http_client_parse_url("http://example.com/api/v1", &https, host, sizeof(host), &port, path, sizeof(path)));
    TEST_ASSERT_FALSE(https);
    TEST_ASSERT_EQUAL_STRING("example.com", host);
    TEST_ASSERT_EQUAL_UINT(80, port);
    TEST_ASSERT_EQUAL_STRING("/api/v1", path);
}

void test_url_https_port_nopath()
{
    bool https;
    char host[64], path[64];
    uint16_t port;
    TEST_ASSERT_TRUE(
        http_client_parse_url("https://10.0.0.5:8443", &https, host, sizeof(host), &port, path, sizeof(path)));
    TEST_ASSERT_TRUE(https);
    TEST_ASSERT_EQUAL_STRING("10.0.0.5", host);
    TEST_ASSERT_EQUAL_UINT(8443, port);
    TEST_ASSERT_EQUAL_STRING("/", path); // default path
}

void test_url_bad_scheme()
{
    bool https;
    char host[64], path[64];
    uint16_t port;
    TEST_ASSERT_FALSE(http_client_parse_url("ftp://x/y", &https, host, sizeof(host), &port, path, sizeof(path)));
    TEST_ASSERT_FALSE(http_client_parse_url("example.com", &https, host, sizeof(host), &port, path, sizeof(path)));
}

// ---- request building -----------------------------------------------------

void test_build_get()
{
    char buf[256];
    size_t n = http_client_build_request("GET", "example.com", 80, "/path", nullptr, nullptr, 0, buf, sizeof(buf));
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "GET /path HTTP/1.1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Host: example.com\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Connection: close\r\n\r\n"));
    TEST_ASSERT_NULL(strstr(buf, "Content-Length"));
}

void test_build_post_with_body_and_port()
{
    char buf[256];
    const char *body = "{\"k\":1}";
    size_t n = http_client_build_request("POST", "host", 8080, "/ingest", "application/json", (const uint8_t *)body,
                                         strlen(body), buf, sizeof(buf));
    TEST_ASSERT_GREATER_THAN_UINT(0, n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "POST /ingest HTTP/1.1\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Host: host:8080\r\n")); // non-default port in Host
    TEST_ASSERT_NOT_NULL(strstr(buf, "Content-Type: application/json\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Content-Length: 7\r\n"));
    TEST_ASSERT_EQUAL_MEMORY(body, buf + n - 7, 7); // body appended verbatim
}

// ---- response parsing -----------------------------------------------------

void test_parse_content_length()
{
    char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_EQUAL_INT(200, st);
    TEST_ASSERT_EQUAL_UINT(5, blen);
    TEST_ASSERT_EQUAL_MEMORY("hello", resp + off, 5);
}

void test_parse_status_404()
{
    char resp[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_EQUAL_INT(404, st);
    TEST_ASSERT_EQUAL_UINT(0, blen);
}

void test_parse_chunked()
{
    // two chunks "Wiki" (4) + "pedia" (5) -> "Wikipedia"
    char resp[] = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_EQUAL_INT(200, st);
    TEST_ASSERT_EQUAL_UINT(9, blen);
    TEST_ASSERT_EQUAL_MEMORY("Wikipedia", resp + off, 9);
}

void test_parse_connection_close_body()
{
    // No Content-Length / chunked: body is everything after the headers.
    char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nstreamed body";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_EQUAL_INT(200, st);
    TEST_ASSERT_EQUAL_UINT(13, blen);
    TEST_ASSERT_EQUAL_MEMORY("streamed body", resp + off, 13);
}

void test_parse_malformed()
{
    char resp[] = "not http at all";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_LESS_THAN_INT(0, st);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_url_http_default);
    RUN_TEST(test_url_https_port_nopath);
    RUN_TEST(test_url_bad_scheme);
    RUN_TEST(test_build_get);
    RUN_TEST(test_build_post_with_body_and_port);
    RUN_TEST(test_parse_content_length);
    RUN_TEST(test_parse_status_404);
    RUN_TEST(test_parse_chunked);
    RUN_TEST(test_parse_connection_close_body);
    RUN_TEST(test_parse_malformed);
    return UNITY_END();
}
