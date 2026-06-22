// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Comprehensive unit tests for the standalone HTTP/1.1 parser.
//
// Tests call http_parser_feed() directly - no ring buffer, no transport layer.
// This verifies the parser in isolation: state transitions, field extraction,
// body handling, 413 detection, error cases, and boundary values.
//
// Sections:
//   RESET      - http_parser_reset() invariants
//   FEED API   - terminal-state guard, byte ordering
//   METHOD     - all supported methods, overflow → PARSE_ERROR
//   PATH       - extraction, truncation → PARSE_ERROR
//   QUERY      - single/multiple params, key/value split
//   HEADERS    - extraction, case-insensitive lookup, multi-header
//   BODY       - GET (no body), POST/PUT with body, boundary values
//   413        - Content-Length > BODY_BUF_SIZE → PARSE_ENTITY_TOO_LARGE
//   HELPERS    - http_get_header, http_get_query edge cases
//   STRESS     - large query, many headers, incremental feeds

#include "network_drivers/presentation/http_parser.h"
#include <unity.h>

// ---- Helpers -----------------------------------------------------------

static void feed_str(HttpReq *req, const char *s)
{
    for (; *s; s++)
        http_parser_feed(req, (uint8_t)*s);
}

static void feed_request(uint8_t slot, const char *raw)
{
    http_parser_reset(&http_pool[slot]);
    feed_str(&http_pool[slot], raw);
}

void setUp()
{
    for (int i = 0; i < MAX_CONNS; i++)
    {
        http_pool[i] = {};
        http_pool[i].slot_id = (uint8_t)i;
        http_parser_reset(&http_pool[i]);
    }
}

void tearDown()
{
}

// ====================================================================
// RESET TESTS
// ====================================================================

void test_reset_sets_parse_method_state()
{
    http_pool[0].parse_state = PARSE_COMPLETE;
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL(PARSE_METHOD, http_pool[0].parse_state);
}

void test_reset_preserves_slot_id()
{
    http_pool[2].slot_id = 2;
    http_parser_reset(&http_pool[2]);
    TEST_ASSERT_EQUAL(2, (int)http_pool[2].slot_id);
}

void test_reset_clears_method()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL('\0', http_pool[0].method[0]);
}

void test_reset_clears_path()
{
    feed_request(0, "GET /api/data HTTP/1.1\r\n\r\n");
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL('\0', http_pool[0].path[0]);
}

void test_reset_clears_header_count()
{
    feed_request(0, "GET / HTTP/1.1\r\nX-Foo: bar\r\n\r\n");
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].header_count);
}

void test_reset_clears_body()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 3\r\n\r\nabc");
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].body_len);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].content_length);
}

void test_reset_clears_query_count()
{
    feed_request(0, "GET /?a=1&b=2 HTTP/1.1\r\n\r\n");
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].query_count);
}

// ====================================================================
// FEED API - terminal-state guard
// ====================================================================

void test_feed_after_complete_does_not_change_state()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    http_parser_feed(&http_pool[0], 'X');
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

void test_feed_after_error_does_not_change_state()
{
    http_pool[0].parse_state = PARSE_ERROR;
    http_parser_feed(&http_pool[0], 'X');
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_feed_after_entity_too_large_does_not_change_state()
{
    http_pool[0].parse_state = PARSE_ENTITY_TOO_LARGE;
    http_parser_feed(&http_pool[0], 'X');
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state);
}

// ====================================================================
// METHOD TESTS
// ====================================================================

void test_method_get()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("GET", http_pool[0].method);
}

void test_method_post()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("POST", http_pool[0].method);
}

void test_method_put()
{
    feed_request(0, "PUT /r HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("PUT", http_pool[0].method);
}

void test_method_delete()
{
    feed_request(0, "DELETE /r HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("DELETE", http_pool[0].method);
}

void test_method_patch()
{
    feed_request(0, "PATCH /r HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("PATCH", http_pool[0].method);
}

void test_method_head()
{
    feed_request(0, "HEAD / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("HEAD", http_pool[0].method);
}

void test_method_options()
{
    feed_request(0, "OPTIONS / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("OPTIONS", http_pool[0].method);
}

void test_method_overflow_is_error()
{
    // More than 7 chars (sizeof method - 1) before a space → PARSE_ERROR
    feed_request(0, "TOOLONGM /path HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

// ====================================================================
// PATH TESTS
// ====================================================================

void test_path_root()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL_STRING("/", http_pool[0].path);
}

void test_path_segments()
{
    feed_request(0, "GET /api/users/42 HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL_STRING("/api/users/42", http_pool[0].path);
}

void test_path_without_query()
{
    feed_request(0, "GET /p?k=v HTTP/1.1\r\n\r\n");
    // path should NOT contain the query string
    TEST_ASSERT_EQUAL_STRING("/p", http_pool[0].path);
}

void test_path_overflow_is_414()
{
    // Build a path longer than MAX_PATH_LEN
    char req[MAX_PATH_LEN + 64];
    int idx = 0;
    memcpy(req + idx, "GET /", 5);
    idx += 5;
    for (int i = 0; i < MAX_PATH_LEN; i++)
        req[idx++] = 'a';
    memcpy(req + idx, " HTTP/1.1\r\n\r\n", 13);
    idx += 13;
    req[idx] = '\0';
    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);
}

// ====================================================================
// QUERY STRING TESTS
// ====================================================================

void test_single_query_param()
{
    feed_request(0, "GET /p?id=42 HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(1, (int)http_pool[0].query_count);
    const char *v = http_get_query(&http_pool[0], "id");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("42", v);
}

void test_two_query_params()
{
    feed_request(0, "GET /p?a=1&b=2 HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(2, (int)http_pool[0].query_count);
    TEST_ASSERT_EQUAL_STRING("1", http_get_query(&http_pool[0], "a"));
    TEST_ASSERT_EQUAL_STRING("2", http_get_query(&http_pool[0], "b"));
}

void test_query_key_not_found_returns_null()
{
    feed_request(0, "GET /p?a=1 HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NULL(http_get_query(&http_pool[0], "z"));
}

void test_query_empty_value()
{
    feed_request(0, "GET /p?key= HTTP/1.1\r\n\r\n");
    const char *v = http_get_query(&http_pool[0], "key");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("", v);
}

// ====================================================================
// HEADER TESTS
// ====================================================================

void test_single_header_stored()
{
    feed_request(0, "GET / HTTP/1.1\r\nX-Custom: hello\r\n\r\n");
    TEST_ASSERT_EQUAL(1, (int)http_pool[0].header_count);
    const char *v = http_get_header(&http_pool[0], "X-Custom");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("hello", v);
}

void test_header_lookup_case_insensitive()
{
    feed_request(0, "GET / HTTP/1.1\r\nContent-Type: application/json\r\n\r\n");
    TEST_ASSERT_NOT_NULL(http_get_header(&http_pool[0], "content-type"));
    TEST_ASSERT_NOT_NULL(http_get_header(&http_pool[0], "CONTENT-TYPE"));
    TEST_ASSERT_EQUAL_STRING("application/json", http_get_header(&http_pool[0], "Content-Type"));
}

void test_header_leading_space_stripped()
{
    feed_request(0, "GET / HTTP/1.1\r\nX-Val:   trimmed\r\n\r\n");
    const char *v = http_get_header(&http_pool[0], "X-Val");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("trimmed", v);
}

void test_content_length_header_parsed()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
    TEST_ASSERT_EQUAL(5, (int)http_pool[0].content_length);
}

void test_content_length_in_headers_array()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 3\r\n\r\nabc");
    const char *cl = http_get_header(&http_pool[0], "Content-Length");
    TEST_ASSERT_NOT_NULL(cl);
    TEST_ASSERT_EQUAL_STRING("3", cl);
}

void test_multiple_headers_stored()
{
    feed_request(0, "GET / HTTP/1.1\r\n"
                    "X-A: one\r\n"
                    "X-B: two\r\n"
                    "X-C: three\r\n"
                    "\r\n");
    TEST_ASSERT_EQUAL(3, (int)http_pool[0].header_count);
    TEST_ASSERT_EQUAL_STRING("one", http_get_header(&http_pool[0], "X-A"));
    TEST_ASSERT_EQUAL_STRING("two", http_get_header(&http_pool[0], "X-B"));
    TEST_ASSERT_EQUAL_STRING("three", http_get_header(&http_pool[0], "X-C"));
}

void test_missing_header_returns_null()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_NULL(http_get_header(&http_pool[0], "X-Missing"));
}

// ====================================================================
// BODY TESTS
// ====================================================================

void test_get_no_body_completes()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].body_len);
    TEST_ASSERT_EQUAL('\0', (char)http_pool[0].body[0]);
}

void test_post_with_body()
{
    feed_request(0, "POST /r HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(5, (int)http_pool[0].body_len);
    TEST_ASSERT_EQUAL_STRING("hello", (const char *)http_pool[0].body);
}

void test_put_with_body()
{
    feed_request(0, "PUT /r HTTP/1.1\r\nContent-Length: 7\r\n\r\nupdated");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(7, (int)http_pool[0].body_len);
    TEST_ASSERT_EQUAL_STRING("updated", (const char *)http_pool[0].body);
}

void test_body_starting_with_newline()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 5\r\n\r\n\nabcd");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(5, (int)http_pool[0].body_len);
    TEST_ASSERT_EQUAL('\n', (char)http_pool[0].body[0]);
    TEST_ASSERT_EQUAL_STRING("\nabcd", (const char *)http_pool[0].body);
}

void test_post_content_length_zero()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].body_len);
}

void test_body_exactly_at_buffer_limit()
{
    // Body of exactly BODY_BUF_SIZE bytes - should succeed
    char req[32 + BODY_BUF_SIZE + 64];
    int off = 0;
    off +=
        snprintf(req + off, sizeof(req) - (size_t)off, "POST / HTTP/1.1\r\nContent-Length: %d\r\n\r\n", BODY_BUF_SIZE);
    memset(req + off, 'X', BODY_BUF_SIZE);
    off += BODY_BUF_SIZE;
    req[off] = '\0';

    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(BODY_BUF_SIZE, (int)http_pool[0].body_len);
}

void test_body_null_terminated_after_complete()
{
    feed_request(0, "POST / HTTP/1.1\r\nContent-Length: 3\r\n\r\nabc");
    TEST_ASSERT_EQUAL('\0', (char)http_pool[0].body[3]);
}

// ====================================================================
// 413 - PARSE_ENTITY_TOO_LARGE
// ====================================================================

void test_body_one_over_limit_is_413()
{
    // Content-Length == BODY_BUF_SIZE + 1 → PARSE_ENTITY_TOO_LARGE
    char req[128];
    snprintf(req, sizeof(req), "POST / HTTP/1.1\r\nContent-Length: %d\r\n\r\n", BODY_BUF_SIZE + 1);
    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state);
}

void test_body_far_over_limit_is_413()
{
    char req[128];
    snprintf(req, sizeof(req), "POST / HTTP/1.1\r\nContent-Length: 65535\r\n\r\n");
    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state);
}

void test_413_no_body_bytes_fed()
{
    // Even though we detected 413, no body bytes should have been stored
    char req[128];
    snprintf(req, sizeof(req), "POST / HTTP/1.1\r\nContent-Length: %d\r\n\r\n", BODY_BUF_SIZE + 1);
    feed_request(0, req);
    TEST_ASSERT_EQUAL(0, (int)http_pool[0].body_len);
}

void test_413_header_still_stored()
{
    // Headers before the blank line must be accessible even when 413
    char req[128];
    snprintf(req, sizeof(req), "POST / HTTP/1.1\r\nX-Tag: test\r\nContent-Length: %d\r\n\r\n", BODY_BUF_SIZE + 1);
    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("test", http_get_header(&http_pool[0], "X-Tag"));
}

void test_body_exactly_at_limit_is_not_413()
{
    // BODY_BUF_SIZE is the max that fits - should NOT trigger 413
    char req[128];
    snprintf(req, sizeof(req), "POST / HTTP/1.1\r\nContent-Length: %d\r\n\r\n", BODY_BUF_SIZE);
    feed_request(0, req);
    // Parser enters PARSE_BODY, not PARSE_ENTITY_TOO_LARGE
    // (we don't send the body bytes here - just check it didn't go 413)
    TEST_ASSERT_NOT_EQUAL(PARSE_ENTITY_TOO_LARGE, http_pool[0].parse_state);
}

// ====================================================================
// 414 - PARSE_URI_TOO_LONG
// ====================================================================

void test_path_overflow_stops_feeding()
{
    // Bytes fed after URI_TOO_LONG are ignored - state must not change
    char req[MAX_PATH_LEN + 64];
    int idx = 0;
    memcpy(req + idx, "GET /", 5);
    idx += 5;
    for (int i = 0; i < MAX_PATH_LEN; i++)
        req[idx++] = 'a';
    req[idx] = '\0';
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], req);
    TEST_ASSERT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);
    // Feed more bytes - state must stay PARSE_URI_TOO_LONG
    http_parser_feed(&http_pool[0], 'X');
    TEST_ASSERT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);
}

void test_414_path_filled_to_capacity()
{
    // Buffer fills to MAX_PATH_LEN-1 chars before overflow is detected
    char req[MAX_PATH_LEN + 64];
    int idx = 0;
    memcpy(req + idx, "GET /api", 8);
    idx += 8;
    for (int i = 0; i < MAX_PATH_LEN; i++)
        req[idx++] = 'x';
    req[idx] = '\0';
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], req);
    TEST_ASSERT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);
    // Prefix intact; buffer filled to exactly MAX_PATH_LEN-1 chars
    TEST_ASSERT_EQUAL('/', http_pool[0].path[0]);
    TEST_ASSERT_EQUAL('a', http_pool[0].path[1]);
    TEST_ASSERT_EQUAL(MAX_PATH_LEN - 1, (int)strlen(http_pool[0].path));
}

// ====================================================================
// RFC 7230 COMPLIANCE - method (tchar only)
// ====================================================================

void test_method_nul_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    http_parser_feed(&http_pool[0], 0x00);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_method_control_char_is_error()
{
    http_parser_reset(&http_pool[0]);
    http_parser_feed(&http_pool[0], 0x01);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_method_del_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    http_parser_feed(&http_pool[0], 0x7F);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_method_non_tchar_symbol_is_error()
{
    // '(' is VCHAR but not tchar
    http_parser_reset(&http_pool[0]);
    http_parser_feed(&http_pool[0], (uint8_t)'(');
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_method_tchar_symbols_accepted()
{
    // '-' is a valid tchar; a custom method like "X-CMD" is valid per RFC 7230
    feed_request(0, "X-CMD / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("X-CMD", http_pool[0].method);
}

// ====================================================================
// RFC 7230 COMPLIANCE - path / query (VCHAR only)
// ====================================================================

void test_path_nul_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET /");
    http_parser_feed(&http_pool[0], 0x00); // NUL in path
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_path_control_char_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET /");
    http_parser_feed(&http_pool[0], 0x01);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_path_del_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET /");
    http_parser_feed(&http_pool[0], 0x7F);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_query_nul_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET /p?k=");
    http_parser_feed(&http_pool[0], 0x00);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_query_control_char_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET /p?k=");
    http_parser_feed(&http_pool[0], 0x02);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

// ====================================================================
// RFC 7230 COMPLIANCE - header field-name (tchar only)
// ====================================================================

void test_header_key_space_is_error()
{
    // Space in a field-name is not a valid tchar
    feed_request(0, "GET / HTTP/1.1\r\nX Bad: v\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_key_nul_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-");
    http_parser_feed(&http_pool[0], 0x00);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_key_control_char_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-");
    http_parser_feed(&http_pool[0], 0x01);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_key_mid_cr_is_error()
{
    // CR in the middle of a key name must be PARSE_ERROR, not blank-line detection
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-Foo");
    http_parser_feed(&http_pool[0], '\r'); // CR mid-key
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_key_colon_at_start_skips_header()
{
    // Empty key name (colon immediately after CRLF): transition to val with empty key
    // This is unusual but not explicitly rejected - the header just has an empty key name
    feed_request(0, "GET / HTTP/1.1\r\n: empty-key\r\n\r\n");
    // Parser enters header-val with empty key - should complete without error
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

// ====================================================================
// RFC 7230 COMPLIANCE - header field-value (field-value chars only)
// ====================================================================

void test_header_val_nul_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-A: ");
    http_parser_feed(&http_pool[0], 0x00);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_val_control_char_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-A: ");
    http_parser_feed(&http_pool[0], 0x01);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_val_del_byte_is_error()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-A: ");
    http_parser_feed(&http_pool[0], 0x7F);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_header_val_htab_mid_value_allowed()
{
    // HTAB is valid mid-value (RFC 7230 §3.2)
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-A: foo");
    http_parser_feed(&http_pool[0], '\t');
    feed_str(&http_pool[0], "bar\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    const char *v = http_get_header(&http_pool[0], "X-A");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL('f', v[0]); // value starts with 'f', not tab
}

void test_header_val_leading_htab_stripped()
{
    // Leading HTAB (OWS) is stripped just like leading SP
    feed_request(0, "GET / HTTP/1.1\r\nX-B:\tvalue\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    const char *v = http_get_header(&http_pool[0], "X-B");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_STRING("value", v);
}

void test_header_val_obs_text_allowed()
{
    // obs-text bytes (%x80-FF) are allowed for legacy compatibility (RFC 7230 §3.2.6)
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\nX-C: ");
    http_parser_feed(&http_pool[0], 0x80); // obs-text
    http_parser_feed(&http_pool[0], 0xFF); // obs-text
    feed_str(&http_pool[0], "\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

// ====================================================================
// VERSION VALIDATION
// ====================================================================

void test_version_http11_recognized()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(HTTP_11, http_pool[0].version);
}

void test_version_http10_recognized()
{
    feed_request(0, "GET / HTTP/1.0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(HTTP_10, http_pool[0].version);
}

void test_version_unknown_is_http_unknown()
{
    feed_request(0, "GET / HTTP/2.0\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(HTTP_UNKNOWN, http_pool[0].version);
}

void test_version_reset_to_unknown()
{
    feed_request(0, "GET / HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(HTTP_11, http_pool[0].version);
    http_parser_reset(&http_pool[0]);
    TEST_ASSERT_EQUAL(HTTP_UNKNOWN, http_pool[0].version);
}

// ====================================================================
// PARSE_ERROR CASES
// ====================================================================

void test_bad_expect_lf_is_error()
{
    // CRLF in version line replaced by CR + X (no LF)
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\rX"); // CR then non-LF
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

void test_blank_line_non_lf_is_error()
{
    // Header block ends with CR + non-LF in the blank line
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "GET / HTTP/1.1\r\n\rX");
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

// ====================================================================
// MULTI-SLOT INDEPENDENCE
// ====================================================================

void test_slots_are_independent()
{
    feed_request(0, "GET /slot0 HTTP/1.1\r\n\r\n");
    feed_request(1, "POST /slot1 HTTP/1.1\r\nContent-Length: 4\r\n\r\ndata");

    TEST_ASSERT_EQUAL_STRING("GET", http_pool[0].method);
    TEST_ASSERT_EQUAL_STRING("/slot0", http_pool[0].path);
    TEST_ASSERT_EQUAL_STRING("POST", http_pool[1].method);
    TEST_ASSERT_EQUAL_STRING("/slot1", http_pool[1].path);
    TEST_ASSERT_EQUAL_STRING("data", (const char *)http_pool[1].body);
}

// ====================================================================
// INCREMENTAL FEED
// ====================================================================

void test_incremental_byte_by_byte()
{
    const char *raw = "GET /inc HTTP/1.1\r\n\r\n";
    http_parser_reset(&http_pool[0]);
    for (; *raw; raw++)
        http_parser_feed(&http_pool[0], (uint8_t)*raw);
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("/inc", http_pool[0].path);
}

void test_incremental_two_chunks()
{
    http_parser_reset(&http_pool[0]);
    feed_str(&http_pool[0], "POST /c HTTP/1.1\r\nContent-Length: 4\r\n\r\n");
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    feed_str(&http_pool[0], "body");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL_STRING("body", (const char *)http_pool[0].body);
}

// ====================================================================
// STRESS
// ====================================================================

void stress_many_requests_same_slot()
{
    for (int i = 0; i < 100; i++)
    {
        feed_request(0, "GET /stress HTTP/1.1\r\n\r\n");
        TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    }
}

void stress_max_headers()
{
    // Build a request with MAX_HEADERS header lines
    char req[MAX_HEADERS * 32 + 64];
    int off = 0;
    off += snprintf(req + off, sizeof(req) - (size_t)off, "GET / HTTP/1.1\r\n");
    for (int i = 0; i < MAX_HEADERS; i++)
        off += snprintf(req + off, sizeof(req) - (size_t)off, "H%d: v%d\r\n", i, i);
    off += snprintf(req + off, sizeof(req) - (size_t)off, "\r\n");
    req[off] = '\0';

    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(MAX_HEADERS, (int)http_pool[0].header_count);
}

void stress_max_query_params()
{
    // Build a query string with MAX_QUERY_PARAMS parameters
    char req[MAX_QUERY_PARAMS * 16 + 64];
    int off = 0;
    off += snprintf(req + off, sizeof(req) - (size_t)off, "GET /p?");
    for (int i = 0; i < MAX_QUERY_PARAMS; i++)
    {
        if (i > 0)
            req[off++] = '&';
        off += snprintf(req + off, sizeof(req) - (size_t)off, "k%d=%d", i, i);
    }
    off += snprintf(req + off, sizeof(req) - (size_t)off, " HTTP/1.1\r\n\r\n");
    req[off] = '\0';

    feed_request(0, req);
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
    TEST_ASSERT_EQUAL(MAX_QUERY_PARAMS, (int)http_pool[0].query_count);
}

int main()
{
    UNITY_BEGIN();

    // Reset invariants
    RUN_TEST(test_reset_sets_parse_method_state);
    RUN_TEST(test_reset_preserves_slot_id);
    RUN_TEST(test_reset_clears_method);
    RUN_TEST(test_reset_clears_path);
    RUN_TEST(test_reset_clears_header_count);
    RUN_TEST(test_reset_clears_body);
    RUN_TEST(test_reset_clears_query_count);

    // Terminal-state guard
    RUN_TEST(test_feed_after_complete_does_not_change_state);
    RUN_TEST(test_feed_after_error_does_not_change_state);
    RUN_TEST(test_feed_after_entity_too_large_does_not_change_state);

    // Method
    RUN_TEST(test_method_get);
    RUN_TEST(test_method_post);
    RUN_TEST(test_method_put);
    RUN_TEST(test_method_delete);
    RUN_TEST(test_method_patch);
    RUN_TEST(test_method_head);
    RUN_TEST(test_method_options);
    RUN_TEST(test_method_overflow_is_error);

    // Path
    RUN_TEST(test_path_root);
    RUN_TEST(test_path_segments);
    RUN_TEST(test_path_without_query);
    RUN_TEST(test_path_overflow_is_414);

    // Query
    RUN_TEST(test_single_query_param);
    RUN_TEST(test_two_query_params);
    RUN_TEST(test_query_key_not_found_returns_null);
    RUN_TEST(test_query_empty_value);

    // Headers
    RUN_TEST(test_single_header_stored);
    RUN_TEST(test_header_lookup_case_insensitive);
    RUN_TEST(test_header_leading_space_stripped);
    RUN_TEST(test_content_length_header_parsed);
    RUN_TEST(test_content_length_in_headers_array);
    RUN_TEST(test_multiple_headers_stored);
    RUN_TEST(test_missing_header_returns_null);

    // Body
    RUN_TEST(test_get_no_body_completes);
    RUN_TEST(test_post_with_body);
    RUN_TEST(test_put_with_body);
    RUN_TEST(test_body_starting_with_newline);
    RUN_TEST(test_post_content_length_zero);
    RUN_TEST(test_body_exactly_at_buffer_limit);
    RUN_TEST(test_body_null_terminated_after_complete);

    // 413
    RUN_TEST(test_body_one_over_limit_is_413);
    RUN_TEST(test_body_far_over_limit_is_413);
    RUN_TEST(test_413_no_body_bytes_fed);
    RUN_TEST(test_413_header_still_stored);
    RUN_TEST(test_body_exactly_at_limit_is_not_413);

    // 414
    RUN_TEST(test_path_overflow_stops_feeding);
    RUN_TEST(test_414_path_filled_to_capacity);

    // RFC 7230 - method (tchar)
    RUN_TEST(test_method_nul_byte_is_error);
    RUN_TEST(test_method_control_char_is_error);
    RUN_TEST(test_method_del_byte_is_error);
    RUN_TEST(test_method_non_tchar_symbol_is_error);
    RUN_TEST(test_method_tchar_symbols_accepted);

    // RFC 7230 - path / query (VCHAR)
    RUN_TEST(test_path_nul_byte_is_error);
    RUN_TEST(test_path_control_char_is_error);
    RUN_TEST(test_path_del_byte_is_error);
    RUN_TEST(test_query_nul_byte_is_error);
    RUN_TEST(test_query_control_char_is_error);

    // RFC 7230 - header field-name (tchar)
    RUN_TEST(test_header_key_space_is_error);
    RUN_TEST(test_header_key_nul_byte_is_error);
    RUN_TEST(test_header_key_control_char_is_error);
    RUN_TEST(test_header_key_mid_cr_is_error);
    RUN_TEST(test_header_key_colon_at_start_skips_header);

    // RFC 7230 - header field-value
    RUN_TEST(test_header_val_nul_byte_is_error);
    RUN_TEST(test_header_val_control_char_is_error);
    RUN_TEST(test_header_val_del_byte_is_error);
    RUN_TEST(test_header_val_htab_mid_value_allowed);
    RUN_TEST(test_header_val_leading_htab_stripped);
    RUN_TEST(test_header_val_obs_text_allowed);

    // Version
    RUN_TEST(test_version_http11_recognized);
    RUN_TEST(test_version_http10_recognized);
    RUN_TEST(test_version_unknown_is_http_unknown);
    RUN_TEST(test_version_reset_to_unknown);

    // Errors
    RUN_TEST(test_bad_expect_lf_is_error);
    RUN_TEST(test_blank_line_non_lf_is_error);

    // Multi-slot
    RUN_TEST(test_slots_are_independent);

    // Incremental
    RUN_TEST(test_incremental_byte_by_byte);
    RUN_TEST(test_incremental_two_chunks);

    // Stress
    RUN_TEST(stress_many_requests_same_slot);
    RUN_TEST(stress_max_headers);
    RUN_TEST(stress_max_query_params);

    return UNITY_END();
}
