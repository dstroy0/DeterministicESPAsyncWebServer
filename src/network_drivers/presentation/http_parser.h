// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_parser.h
 * @brief Standalone HTTP/1.1 request parser - no transport dependency.
 *
 * The parser is a pure byte-stream state machine.  It has no knowledge of
 * ring buffers, TCP PCBs, or FreeRTOS.  Feed it bytes one at a time via
 * `http_parser_feed()` and inspect `HttpReq::parse_state` to know when the
 * request is ready.
 *
 * **State machine**
 * ```
 * PARSE_METHOD       ──space──────► PARSE_PATH
 * PARSE_PATH         ──space──────► PARSE_VERSION
 * PARSE_PATH         ──'?'────────► PARSE_QUERY
 * PARSE_QUERY        ──space──────► PARSE_VERSION  (calls parse_query_params)
 * PARSE_VERSION      ──CR─────────► PARSE_EXPECT_LF
 * PARSE_EXPECT_LF    ──LF─────────► PARSE_HEADER_KEY
 * PARSE_HEADER_KEY   ──':'────────► PARSE_HEADER_VAL
 * PARSE_HEADER_KEY   ──CR─────────► PARSE_EXPECT_BODY_LF  (blank line)
 * PARSE_HEADER_VAL   ──CR─────────► PARSE_EXPECT_LF  (stores header)
 * PARSE_EXPECT_BODY_LF ──LF (CL=0)──► PARSE_COMPLETE
 * PARSE_EXPECT_BODY_LF ──LF (CL>BUF)► PARSE_ENTITY_TOO_LARGE  (→ 413)
 * PARSE_EXPECT_BODY_LF ──LF (else)──► PARSE_BODY
 * PARSE_BODY         ──(all read)──► PARSE_COMPLETE
 * PARSE_PATH (overflow) ───────────► PARSE_URI_TOO_LONG       (→ 414)
 * Any state + protocol error ──────► PARSE_ERROR             (→ 400)
 * ```
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_HTTP_PARSER_H
#define DETERMINISTICESPASYNCWEBSERVER_HTTP_PARSER_H

#include "DetWebServerConfig.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Parser state enumeration
// ---------------------------------------------------------------------------

/**
 * @brief States of the HTTP/1.1 request parser.
 *
 * Advance via http_parser_feed().  The application layer inspects this
 * after each feed call or after draining a complete chunk.
 */
enum ParseState
{
    PARSE_METHOD,           ///< Reading the HTTP method (GET, POST, …).
    PARSE_PATH,             ///< Reading the URL path component.
    PARSE_QUERY,            ///< Reading the raw query string (after `?`).
    PARSE_VERSION,          ///< Accumulating `HTTP/1.x` - hashed for validation.
    PARSE_HEADER_KEY,       ///< Reading a header field name.
    PARSE_HEADER_VAL,       ///< Reading a header field value.
    PARSE_EXPECT_LF,        ///< Consuming the LF of a header-line CRLF pair.
    PARSE_EXPECT_BODY_LF,   ///< Consuming the LF of the blank-line CRLF.
    PARSE_BODY,             ///< Reading the request body.
    PARSE_COMPLETE,         ///< Full request parsed; ready for dispatch.
    PARSE_ERROR,            ///< Unrecoverable parse failure → 400.
    PARSE_ENTITY_TOO_LARGE, ///< Content-Length > BODY_BUF_SIZE → 413.
    PARSE_URI_TOO_LONG      ///< Path exceeds MAX_PATH_LEN → 414.
};

/**
 * @brief Parsed HTTP protocol version.
 *
 * Populated from the request line (`HTTP/1.0` or `HTTP/1.1`) using an FNV-1a
 * hash accumulated during `PARSE_VERSION`.  The application layer may use
 * this to drive keep-alive semantics: HTTP/1.1 defaults to persistent
 * connections; HTTP/1.0 defaults to close.
 */
enum HttpVersion
{
    HTTP_UNKNOWN = 0, ///< Version string did not match any known token.
    HTTP_10,          ///< HTTP/1.0 - close semantics by default.
    HTTP_11           ///< HTTP/1.1 - persistent connection by default.
};

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------

/** @brief A single HTTP header field (key: value). */
struct Header
{
    char key[MAX_KEY_LEN]; ///< Field name, null-terminated.
    char val[MAX_VAL_LEN]; ///< Field value, null-terminated.
};

/** @brief A single parsed query-string parameter. */
struct QueryParam
{
    char key[QUERY_KEY_LEN]; ///< Parameter name, null-terminated.
    char val[QUERY_VAL_LEN]; ///< Parameter value (empty string if absent).
};

/**
 * @brief Fully-parsed HTTP/1.1 request.
 *
 * Populated incrementally by http_parser_feed().  Valid for dispatch
 * only when `parse_state == PARSE_COMPLETE`.
 *
 * Call http_parser_reset() to recycle this struct for the next request.
 */
struct HttpReq
{
    uint8_t slot_id;        ///< Transport slot index (set by presentation layer).
    ParseState parse_state; ///< Current parser state.
    HttpVersion version;    ///< Protocol version parsed from the request line.
    uint32_t _version_hash; ///< FNV-1a accumulator for version validation (internal).

    char method[8];          ///< HTTP method, null-terminated (max 7: OPTIONS).
    char path[MAX_PATH_LEN]; ///< URL path, null-terminated; no query string.
    size_t path_idx;         ///< Write cursor into path[].

    char query[MAX_QUERY_LEN];                 ///< Raw query string (after `?`).
    size_t query_idx;                          ///< Write cursor into query[].
    QueryParam query_params[MAX_QUERY_PARAMS]; ///< Parsed key=value pairs.
    uint8_t query_count;                       ///< Valid entries in query_params[].

    Header headers[MAX_HEADERS]; ///< Captured header fields.
    uint8_t header_count;        ///< Valid entries in headers[].
    size_t current_token_idx;    ///< Write cursor shared by key/value sub-states.

    // Scratch copies of the header currently being parsed, populated even for
    // headers beyond MAX_HEADERS so that Host / Content-Length detection and
    // counting are independent of the storage cap (RFC 7230 §5.4, §3.3.2).
    char cur_key[MAX_KEY_LEN]; ///< Field-name of the in-progress header.
    char cur_val[MAX_VAL_LEN]; ///< Field-value of the in-progress header.

    size_t content_length;        ///< Value of Content-Length header (0 if absent).
    uint8_t content_length_count; ///< Number of Content-Length fields seen (RFC 7230 §3.3.2).
    uint8_t host_count;           ///< Number of Host fields seen (RFC 7230 §5.4).
    size_t body_bytes_read;       ///< Body bytes received (may exceed BODY_BUF_SIZE).

    uint8_t body[BODY_BUF_SIZE + 1]; ///< Stored body bytes, always null-terminated.
    size_t body_len;                 ///< Bytes stored in body[] (≤ BODY_BUF_SIZE).
};

/** @brief Pool of parser contexts, one per transport slot. */
extern HttpReq http_pool[MAX_CONNS];

// ---------------------------------------------------------------------------
// Parser API
// ---------------------------------------------------------------------------

/**
 * @brief Reset a parser context to the initial (PARSE_METHOD) state.
 *
 * Zeroes all fields and sets `parse_state = PARSE_METHOD`.  Call before the
 * first use, after each completed or failed request, and on connection events.
 *
 * @param req  Parser context to reset.  Must not be null.
 */
void http_parser_reset(HttpReq *req);

/**
 * @brief Feed one byte to the parser state machine.
 *
 * Returns immediately without modifying state when `parse_state` is already
 * `PARSE_COMPLETE`, `PARSE_ERROR`, `PARSE_ENTITY_TOO_LARGE`, or `PARSE_URI_TOO_LONG`.
 *
 * @param req  Parser context for this request.
 * @param byte Next byte from the HTTP stream.
 */
void http_parser_feed(HttpReq *req, uint8_t byte);

/**
 * @brief Look up a header value by name (case-insensitive).
 *
 * @param req  Parsed request.
 * @param key  Header field name (e.g. `"Content-Type"`).
 * @return Pointer to the null-terminated value, or `nullptr` if not found.
 */
const char *http_get_header(const HttpReq *req, const char *key);

/**
 * @brief Look up a query parameter value by name (case-sensitive).
 *
 * @param req  Parsed request.
 * @param key  Parameter name.
 * @return Pointer to the null-terminated value (empty string if `key=` with
 *         no value), or `nullptr` if the key is absent.
 */
const char *http_get_query(const HttpReq *req, const char *key);

#endif
