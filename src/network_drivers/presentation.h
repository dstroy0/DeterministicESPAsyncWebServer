// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.h
 * @brief Layer 6 (Presentation) — HTTP/1.1 stream parser types and helpers.
 *
 * Declares the byte-by-byte state machine that converts raw TCP bytes
 * (stored in the Layer 4 ring buffers) into a structured HttpReq.
 *
 * **Memory model**
 * `http_pool[MAX_CONNS]` is a statically allocated pool of parser contexts,
 * one per connection slot.  No heap allocation occurs at any point.
 *
 * **Thread / task safety**
 * http_parse() is called from the Arduino main-loop task via server_tick().
 * http_reset() may be called from the same context.  Neither function is
 * safe to call from an lwIP callback.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PRESENTATION_H
#define DETERMINISTICESPASYNCWEBSERVER_PRESENTATION_H

#include <Arduino.h>
#include "transport.h"

// ---------------------------------------------------------------------------
// Capacity constants
// ---------------------------------------------------------------------------

/** @brief Maximum number of HTTP headers stored per request. */
#define MAX_HEADERS       8

/** @brief Maximum URL path length including the leading `/`. */
#define MAX_PATH_LEN      64

/** @brief Maximum header key length (e.g. `"Content-Type"`). */
#define MAX_KEY_LEN       24

/** @brief Maximum header value length. */
#define MAX_VAL_LEN       48

/** @brief Maximum raw query string length (everything after `?`, before space). */
#define MAX_QUERY_LEN     128

/** @brief Maximum number of key=value query parameters parsed and stored. */
#define MAX_QUERY_PARAMS  8

/** @brief Maximum query parameter key length. */
#define QUERY_KEY_LEN     24

/** @brief Maximum query parameter value length. */
#define QUERY_VAL_LEN     48

/**
 * @brief Maximum body bytes stored in HttpReq::body.
 *
 * Bodies larger than this are received and counted (content_length is still
 * tracked correctly) but the excess bytes are silently discarded.
 * Set BODY_BUF_SIZE to 0 if body storage is not needed.
 */
#define BODY_BUF_SIZE     256

// ---------------------------------------------------------------------------
// Parser state machine
// ---------------------------------------------------------------------------

/**
 * @brief States of the HTTP/1.1 request parser.
 *
 * The state machine advances one byte at a time inside http_parse().
 * Any unrecoverable format error transitions to PARSE_ERROR; the framework
 * will respond with 400 Bad Request automatically.
 */
enum ParseState
{
    PARSE_METHOD,          ///< Reading the HTTP method (GET, POST, …).
    PARSE_PATH,            ///< Reading the request URL path component.
    PARSE_QUERY,           ///< Reading the query string (after `?`).
    PARSE_VERSION,         ///< Reading `HTTP/1.x` — contents discarded.
    PARSE_HEADER_KEY,      ///< Reading a header field name.
    PARSE_HEADER_VAL,      ///< Reading a header field value.
    PARSE_EXPECT_LF,       ///< Consuming the LF of a header-line CRLF pair.
    PARSE_EXPECT_BODY_LF,  ///< Consuming the LF of the blank-line CRLF (end of headers).
    PARSE_BODY,            ///< Reading the request body.
    PARSE_COMPLETE,        ///< Full request parsed; ready for dispatch.
    PARSE_ERROR            ///< Unrecoverable parse failure.
};

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------

/**
 * @brief A single HTTP header field (key: value).
 */
struct Header
{
    char key[MAX_KEY_LEN]; ///< Field name, null-terminated (e.g. `"Content-Type"`).
    char val[MAX_VAL_LEN]; ///< Field value, null-terminated (e.g. `"application/json"`).
};

/**
 * @brief A single parsed query string parameter.
 *
 * Produced by the internal `parse_query_params()` helper from the raw
 * query string accumulated in HttpReq::query.
 */
struct QueryParam
{
    char key[QUERY_KEY_LEN]; ///< Parameter name, null-terminated.
    char val[QUERY_VAL_LEN]; ///< Parameter value, null-terminated (empty string if absent).
};

/**
 * @brief Fully-parsed HTTP/1.1 request.
 *
 * One instance lives in `http_pool[slot_id]`.  Populated incrementally by
 * http_parse() as bytes arrive in the ring buffer; valid for the application
 * layer only when `parse_state == PARSE_COMPLETE`.
 *
 * Call http_reset() to recycle the slot after a request has been handled.
 */
struct HttpReq
{
    uint8_t    slot_id;         ///< Index into conn_pool (mirrors transport slot).
    ParseState parse_state;     ///< Current parser state.

    char   method[8];           ///< HTTP method string, null-terminated (max 7 chars: OPTIONS).
    char   path[MAX_PATH_LEN];  ///< URL path, null-terminated; never includes query string.
    size_t path_idx;            ///< Write cursor into path[].

    char       query[MAX_QUERY_LEN]; ///< Raw query string (after `?`), null-terminated.
    size_t     query_idx;            ///< Write cursor into query[].
    QueryParam query_params[MAX_QUERY_PARAMS]; ///< Parsed key=value pairs.
    uint8_t    query_count;          ///< Number of valid entries in query_params[].

    Header  headers[MAX_HEADERS]; ///< Captured header fields.
    uint8_t header_count;         ///< Number of valid entries in headers[].
    size_t  current_token_idx;    ///< Write cursor shared by key and value sub-states.

    size_t content_length;        ///< Value of the Content-Length header (0 if absent).
    size_t body_bytes_read;       ///< Total body bytes received (may exceed BODY_BUF_SIZE).

    uint8_t body[BODY_BUF_SIZE + 1]; ///< Stored body bytes, always null-terminated.
    size_t  body_len;                ///< Bytes stored in body[] (≤ BODY_BUF_SIZE).
};

/** @brief Pool of parser contexts, one per connection slot. */
extern HttpReq http_pool[MAX_CONNS];

// ---------------------------------------------------------------------------
// Functions
// ---------------------------------------------------------------------------

/**
 * @brief Reset (recycle) a parser slot to the initial state.
 *
 * Zeroes all fields and sets `parse_state = PARSE_METHOD`.  Must be called:
 * - Before the first connection is accepted (via DetWebServer::begin()).
 * - After every request, once the response has been sent.
 * - On EVT_CONNECT, EVT_DISCONNECT, and EVT_ERROR events.
 *
 * @param slot_id Index of the slot to reset (0 … MAX_CONNS-1).
 *                Out-of-range values are silently ignored.
 */
void http_reset(uint8_t slot_id);

/**
 * @brief Advance the HTTP parser for a connection slot.
 *
 * Drains as many bytes as are available in `conn_pool[slot_id].rx_buffer`
 * and advances the state machine.  Returns immediately if `parse_state` is
 * already `PARSE_COMPLETE` or `PARSE_ERROR`.
 *
 * Called by server_tick() on every `EVT_DATA` event.
 *
 * @param slot_id Connection slot to parse (0 … MAX_CONNS-1).
 *
 * @note A request with no body (GET, HEAD, DELETE, …) transitions directly
 *       from `PARSE_BODY` to `PARSE_COMPLETE` once the blank line is seen.
 */
void http_parse(uint8_t slot_id);

/**
 * @brief Look up a header value by name (case-insensitive).
 *
 * Performs a linear scan over the `headers[]` array.  Comparison uses
 * `strcasecmp` so `"content-type"` matches `"Content-Type"`.
 *
 * @param req HTTP request to search.
 * @param key Header field name to find.
 * @return Pointer to the null-terminated value string, or `nullptr` if not found.
 *
 * @note The returned pointer is into `req->headers[i].val` — valid only while
 *       the slot is not reset.
 */
const char *http_get_header(const HttpReq *req, const char *key);

/**
 * @brief Look up a query parameter value by name (case-sensitive).
 *
 * Performs a linear scan over `query_params[]`.  Query strings are parsed
 * on first sight of a space after the `?` section; the raw string is also
 * retained in `HttpReq::query`.
 *
 * @param req HTTP request to search.
 * @param key Query parameter name.
 * @return Pointer to the null-terminated value string, or `nullptr` if not found.
 *
 * @note An empty value (`key=`) returns a pointer to an empty string `""`,
 *       not `nullptr`.
 */
const char *http_get_query(const HttpReq *req, const char *key);

#endif
