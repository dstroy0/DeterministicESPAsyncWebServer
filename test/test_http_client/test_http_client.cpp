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

// A chunk size that overflows size_t (16 hex digits = 0xFFFFFFFFFFFFFFFF) must be clamped to
// the bytes actually present, not trusted. The pre-fix bound `in + csz > len` wrapped and
// skipped the clamp, leaving a gigantic memmove (this crashes the old code even on a 64-bit
// host). The wrap-safe `csz > len - in` clamps it to the buffered bytes.
void test_parse_chunked_oversize_size_clamped()
{
    char resp[] = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFF\r\nAB";
    size_t off, blen;
    int st = http_client_parse_response((uint8_t *)resp, strlen(resp), &off, &blen);
    TEST_ASSERT_EQUAL_INT(200, st);
    TEST_ASSERT_EQUAL_UINT(2, blen); // clamped to the 2 bytes actually buffered ("AB")
    TEST_ASSERT_EQUAL_MEMORY("AB", resp + off, 2);
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

void test_url_edge_rejections()
{
    bool https = false;
    char host[64], path[128];
    uint16_t port = 0;
    TEST_ASSERT_FALSE(http_client_parse_url(nullptr, &https, host, 64, &port, path, 128));      // null url
    TEST_ASSERT_FALSE(http_client_parse_url("http://h/", nullptr, host, 64, &port, path, 128)); // null out
    TEST_ASSERT_FALSE(http_client_parse_url("http:///p", &https, host, 64, &port, path, 128));  // empty host
    TEST_ASSERT_FALSE(
        http_client_parse_url("http://longhost.example/", &https, host, 6, &port, path, 128));   // host too long
    TEST_ASSERT_FALSE(http_client_parse_url("http://h:/x", &https, host, 64, &port, path, 128)); // ':' then no digit
    TEST_ASSERT_FALSE(http_client_parse_url("http://h:99999/", &https, host, 64, &port, path, 128)); // port > 65535
    TEST_ASSERT_FALSE(http_client_parse_url("http://h/longpath", &https, host, 64, &port, path, 4)); // path too long
    TEST_ASSERT_FALSE(http_client_parse_url("http://h", &https, host, 64, &port, path, 1));          // no room for "/"

    TEST_ASSERT_TRUE(http_client_parse_url("http://h:8080/api", &https, host, 64, &port, path, 128));
    TEST_ASSERT_EQUAL_UINT16(8080, port);
    TEST_ASSERT_EQUAL_STRING("/api", path);
}

void test_build_edge_rejections()
{
    char out[256];
    const uint8_t body[] = "data";
    TEST_ASSERT_EQUAL_size_t(0,
                             http_client_build_request(nullptr, "h", 80, "/", nullptr, nullptr, 0, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, http_client_build_request("GET", "h", 80, "/", nullptr, nullptr, 0, out, 0));
    TEST_ASSERT_EQUAL_size_t(
        0, http_client_build_request("POST", "h", 80, "/", "text/plain", body, 4, out, 20)); // body overflow
    TEST_ASSERT_EQUAL_size_t(
        0, http_client_build_request("GET", "h", 80, "/", nullptr, nullptr, 0, out, 10)); // hdr overflow

    size_t n = http_client_build_request("GET", "h", 8080, "/", nullptr, nullptr, 0, out, sizeof(out));
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_NOT_NULL(strstr(out, "Host: h:8080\r\n")); // non-default port in the Host header
}

void test_response_edge_rejections()
{
    size_t boff = 0, blen = 0;
    char r2[] = "HTTP/1.1XXXXXXXX"; // no space after the version
    TEST_ASSERT_EQUAL_INT(HTTP_CLIENT_ERR_RESPONSE,
                          http_client_parse_response((uint8_t *)r2, sizeof(r2) - 1, &boff, &blen));
    char r3[] = "HTTP/1.1 999 Z\r\n\r\n"; // status outside 100..599
    TEST_ASSERT_EQUAL_INT(HTTP_CLIENT_ERR_RESPONSE,
                          http_client_parse_response((uint8_t *)r3, sizeof(r3) - 1, &boff, &blen));
    char r4[] = "HTTP/1.1 200 OK\r\nHost: x\r\n"; // no blank-line terminator
    TEST_ASSERT_EQUAL_INT(HTTP_CLIENT_ERR_RESPONSE,
                          http_client_parse_response((uint8_t *)r4, sizeof(r4) - 1, &boff, &blen));

    char r5[] =
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\na\r\n0123456789\r\n0\r\n\r\n"; // lower-case hex size
    TEST_ASSERT_EQUAL_INT(200, http_client_parse_response((uint8_t *)r5, sizeof(r5) - 1, &boff, &blen));
    TEST_ASSERT_EQUAL_size_t(10, blen);
    TEST_ASSERT_EQUAL_MEMORY("0123456789", r5 + boff, 10);

    char r6[] = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n;ext\r\nx"; // junk size line -> empty body
    TEST_ASSERT_EQUAL_INT(200, http_client_parse_response((uint8_t *)r6, sizeof(r6) - 1, &boff, &blen));
    TEST_ASSERT_EQUAL_size_t(0, blen);

    char r7[] = "HTTP/1.1 200 OK\r\nContent-Length: -5\r\n\r\nbody"; // negative length clamps to 0
    TEST_ASSERT_EQUAL_INT(200, http_client_parse_response((uint8_t *)r7, sizeof(r7) - 1, &boff, &blen));
    TEST_ASSERT_EQUAL_size_t(0, blen);
}

// On a host build the transport entry points are stubs that report a connect error.
void test_host_transport_stubs()
{
    HttpClientResult res;
    TEST_ASSERT_EQUAL_INT(HTTP_CLIENT_ERR_CONNECT, http_get("http://x/", &res));
    const uint8_t b[] = "x";
    TEST_ASSERT_EQUAL_INT(HTTP_CLIENT_ERR_CONNECT, http_post("http://x/", "text/plain", b, 1, &res));
    http_client_set_ca(nullptr, 0);
    http_client_set_pin(nullptr);
    http_client_clear_verify();
    TEST_PASS();
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_url_edge_rejections);
    RUN_TEST(test_build_edge_rejections);
    RUN_TEST(test_response_edge_rejections);
    RUN_TEST(test_host_transport_stubs);
    RUN_TEST(test_url_http_default);
    RUN_TEST(test_url_https_port_nopath);
    RUN_TEST(test_url_bad_scheme);
    RUN_TEST(test_build_get);
    RUN_TEST(test_build_post_with_body_and_port);
    RUN_TEST(test_parse_content_length);
    RUN_TEST(test_parse_status_404);
    RUN_TEST(test_parse_chunked);
    RUN_TEST(test_parse_chunked_oversize_size_clamped);
    RUN_TEST(test_parse_connection_close_body);
    RUN_TEST(test_parse_malformed);
    return UNITY_END();
}
