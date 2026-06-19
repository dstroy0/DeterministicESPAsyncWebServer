// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file presentation.cpp
 * @brief Layer 6 (Presentation) — HTTP/1.1 stream parser implementation.
 *
 * **State machine summary**
 * ```
 * PARSE_METHOD  ──space──► PARSE_PATH
 * PARSE_PATH    ──space──► PARSE_VERSION
 * PARSE_PATH    ──'?'───► PARSE_QUERY
 * PARSE_QUERY   ──space──► PARSE_VERSION  (calls parse_query_params first)
 * PARSE_VERSION ──CR─────► PARSE_EXPECT_LF
 * PARSE_EXPECT_LF ──LF──► PARSE_HEADER_KEY
 * PARSE_HEADER_KEY ──':'─► PARSE_HEADER_VAL
 * PARSE_HEADER_KEY ──CR──► PARSE_BODY       (blank line detected)
 * PARSE_HEADER_VAL ──CR──► PARSE_EXPECT_LF  (stores header, parses Content-Length)
 * PARSE_BODY    ──(all bytes read)──► PARSE_COMPLETE
 * Any state + overflow / protocol error ──► PARSE_ERROR
 * ```
 *
 * **Body handling**
 * Body bytes are stored into `HttpReq::body[]` up to `BODY_BUF_SIZE`.
 * Excess bytes are counted in `body_bytes_read` but discarded — the
 * parser still transitions to PARSE_COMPLETE when `body_bytes_read`
 * reaches `content_length`, so large payloads complete cleanly.
 *
 * The LF of the blank line that separates headers from body is consumed
 * by PARSE_BODY and skipped via an early `break` so it does not appear
 * as the first body byte.
 */

#include "presentation.h"

HttpReq http_pool[MAX_CONNS];

void http_reset(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;

    HttpReq *p = &http_pool[slot_id];
    p->slot_id           = slot_id;
    p->parse_state       = PARSE_METHOD;
    memset(p->method, 0, sizeof(p->method));
    memset(p->path,   0, sizeof(p->path));
    p->path_idx          = 0;
    memset(p->query, 0, sizeof(p->query));
    p->query_idx         = 0;
    memset(p->query_params, 0, sizeof(p->query_params));
    p->query_count       = 0;
    p->header_count      = 0;
    p->current_token_idx = 0;
    p->content_length    = 0;
    p->body_bytes_read   = 0;
    memset(p->body, 0, sizeof(p->body));
    p->body_len          = 0;
    memset(p->headers, 0, sizeof(p->headers));
}

/**
 * @brief Split a raw query string into key=value pairs.
 *
 * Operates in-place on `p->query[]`.  Pairs are separated by `&`; key and
 * value are separated by the first `=`.  Both are stored with null
 * terminators inherited from memset in http_reset().
 *
 * Keys or values that exceed their respective capacity limits are silently
 * truncated — no error state is set, because the path itself is valid.
 *
 * @param p HttpReq whose raw query string has already been populated.
 */
static void parse_query_params(HttpReq *p)
{
    const char *qs  = p->query;
    size_t      len = p->query_idx;
    size_t      i   = 0;

    while (i < len && p->query_count < MAX_QUERY_PARAMS)
    {
        QueryParam *qp  = &p->query_params[p->query_count];
        size_t key_idx  = 0;
        size_t val_idx  = 0;
        bool   in_val   = false;

        while (i < len)
        {
            char c = qs[i++];
            if (c == '&')
                break;
            if (c == '=' && !in_val) { in_val = true; continue; }
            if (!in_val && key_idx < QUERY_KEY_LEN - 1)
                qp->key[key_idx++] = c;
            else if (in_val && val_idx < QUERY_VAL_LEN - 1)
                qp->val[val_idx++] = c;
        }

        if (key_idx > 0)
            p->query_count++;
    }
}

void http_parse(uint8_t slot_id)
{
    TcpConn *tcp = &conn_pool[slot_id];
    HttpReq *p   = &http_pool[slot_id];

    while (tcp->rx_tail != tcp->rx_head)
    {
        if (p->parse_state == PARSE_COMPLETE || p->parse_state == PARSE_ERROR)
            break;

        uint8_t c    = tcp->rx_buffer[tcp->rx_tail];
        tcp->rx_tail = (tcp->rx_tail + 1) % RX_BUF_SIZE;

        switch (p->parse_state)
        {

        case PARSE_METHOD:
            if (c == ' ')
            {
                p->parse_state       = PARSE_PATH;
                p->current_token_idx = 0;
            }
            else if (p->current_token_idx < sizeof(p->method) - 1)
            {
                p->method[p->current_token_idx++] = c;
            }
            else
            {
                p->parse_state = PARSE_ERROR;
            }
            break;

        case PARSE_PATH:
            if (c == ' ')
            {
                p->parse_state = PARSE_VERSION;
            }
            else if (c == '?')
            {
                p->parse_state = PARSE_QUERY;
            }
            else if (p->path_idx < MAX_PATH_LEN - 1)
            {
                p->path[p->path_idx++] = c;
            }
            else
            {
                p->parse_state = PARSE_ERROR;
            }
            break;

        case PARSE_QUERY:
            if (c == ' ')
            {
                parse_query_params(p);
                p->parse_state = PARSE_VERSION;
            }
            else if (p->query_idx < MAX_QUERY_LEN - 1)
            {
                p->query[p->query_idx++] = c;
            }
            /* Silently truncate — query excess doesn't invalidate the path. */
            break;

        case PARSE_VERSION:
            if (c == '\r')
                p->parse_state = PARSE_EXPECT_LF;
            break;

        case PARSE_EXPECT_LF:
            if (c == '\n')
            {
                p->parse_state       = PARSE_HEADER_KEY;
                p->current_token_idx = 0;
            }
            else
            {
                p->parse_state = PARSE_ERROR;
            }
            break;

        case PARSE_HEADER_KEY:
            if (c == '\r')
            {
                /* CR on an empty key = blank line = end of headers.
                 * Transition to PARSE_EXPECT_BODY_LF to consume the
                 * trailing LF before entering body parsing, so that
                 * a body starting with '\n' is not silently discarded. */
                p->parse_state = PARSE_EXPECT_BODY_LF;
            }
            else if (c == ':')
            {
                p->parse_state       = PARSE_HEADER_VAL;
                p->current_token_idx = 0;
            }
            else if (p->current_token_idx < MAX_KEY_LEN - 1)
            {
                uint8_t h = p->header_count;
                if (h < MAX_HEADERS)
                    p->headers[h].key[p->current_token_idx++] = c;
            }
            break;

        case PARSE_HEADER_VAL:
            /* Skip leading SP/HT after the colon (RFC 9110 §5.6.3). */
            if (c == ' ' && p->current_token_idx == 0)
                break;
            if (c == '\r')
            {
                uint8_t h = p->header_count;
                if (h < MAX_HEADERS)
                {
                    if (strcasecmp(p->headers[h].key, "Content-Length") == 0)
                        p->content_length = (size_t)atoi(p->headers[h].val);
                    p->header_count++;
                }
                p->parse_state = PARSE_EXPECT_LF;
            }
            else if (p->current_token_idx < MAX_VAL_LEN - 1)
            {
                uint8_t h = p->header_count;
                if (h < MAX_HEADERS)
                    p->headers[h].val[p->current_token_idx++] = c;
            }
            break;

        case PARSE_EXPECT_BODY_LF:
            /* Consume the LF of the blank-line CRLF that terminates headers.
             * Reaching PARSE_BODY cleanly means the first byte it sees is
             * always a real body byte (or the ring buffer is empty). */
            if (c == '\n')
                p->parse_state = PARSE_BODY;
            else
                p->parse_state = PARSE_ERROR;
            break;

        case PARSE_BODY:
            if (p->body_len < BODY_BUF_SIZE)
                p->body[p->body_len++] = c;
            p->body_bytes_read++;
            if (p->body_bytes_read >= p->content_length)
            {
                p->body[p->body_len] = '\0'; // always null-terminate
                p->parse_state = PARSE_COMPLETE;
            }
            break;

        default:
            break;
        }
    }

    /*
     * Requests without a body (GET, HEAD, DELETE, OPTIONS, …) arrive with
     * content_length==0.  They transition from PARSE_BODY to PARSE_COMPLETE
     * here, after the while loop, because there are no body bytes to trigger
     * the in-loop check.
     */
    if (p->parse_state == PARSE_BODY && p->content_length == 0)
    {
        p->body[0]     = '\0';
        p->parse_state = PARSE_COMPLETE;
    }
}

const char *http_get_header(const HttpReq *req, const char *key)
{
    for (uint8_t i = 0; i < req->header_count; i++)
    {
        if (strcasecmp(req->headers[i].key, key) == 0)
            return req->headers[i].val;
    }
    return nullptr;
}

const char *http_get_query(const HttpReq *req, const char *key)
{
    for (uint8_t i = 0; i < req->query_count; i++)
    {
        if (strcmp(req->query_params[i].key, key) == 0)
            return req->query_params[i].val;
    }
    return nullptr;
}
