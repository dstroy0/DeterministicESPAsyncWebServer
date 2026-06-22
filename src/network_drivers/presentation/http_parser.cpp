// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_parser.cpp
 * @brief Standalone HTTP/1.1 request parser - implementation.
 *
 * No dependency on transport, session, or lwIP.  Consumes one byte at a
 * time via http_parser_feed(); the presentation layer is responsible for
 * pulling bytes out of whatever transport buffer it uses.
 */

#include "http_parser.h"

HttpReq http_pool[MAX_CONNS];

#if DETWS_ENABLE_OTA
// Streaming-body hooks (OTA). Null unless the application installs them.
static HttpStreamBeginCb g_stream_begin = nullptr;
static HttpStreamDataCb g_stream_data = nullptr;

void http_parser_set_stream_hooks(HttpStreamBeginCb begin, HttpStreamDataCb data)
{
    g_stream_begin = begin;
    g_stream_data = data;
}
#endif // DETWS_ENABLE_OTA

// ---------------------------------------------------------------------------
// FNV-1a hash constants for HTTP version validation
// ---------------------------------------------------------------------------
// Precomputed at compile time via constexpr; zero runtime cost.
// The hash of the 8-byte version token ("HTTP/1.0" or "HTTP/1.1") is
// compared against the accumulated _version_hash when CR terminates the
// version field.

static constexpr uint32_t FNV_OFFSET = 2166136261u;
static constexpr uint32_t FNV_PRIME = 16777619u;

static constexpr uint32_t fnv1a(const char *s, uint32_t h = FNV_OFFSET)
{
    return *s ? fnv1a(s + 1, (h ^ (uint8_t)*s) * FNV_PRIME) : h;
}

static constexpr uint32_t HASH_HTTP10 = fnv1a("HTTP/1.0");
static constexpr uint32_t HASH_HTTP11 = fnv1a("HTTP/1.1");

// ---------------------------------------------------------------------------
// RFC 7230 character-class helpers
// ---------------------------------------------------------------------------

/**
 * @brief True for bytes that are valid HTTP "token" characters (RFC 7230 §3.2.6).
 *
 * token = 1*tchar
 * tchar = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /
 *         "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
 *
 * Used to validate method bytes and header field-name bytes.
 */
static inline bool is_tchar(uint8_t b)
{
    if (b >= 'A' && b <= 'Z')
        return true;
    if (b >= 'a' && b <= 'z')
        return true;
    if (b >= '0' && b <= '9')
        return true;
    switch (b)
    {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '_':
    case '`':
    case '|':
    case '~':
        return true;
    default:
        return false;
    }
}

/**
 * @brief True for visible ASCII characters (RFC 5234 VCHAR = %x21-7E).
 *
 * Used to validate request-target (path, query) bytes.  Excludes NUL,
 * control characters, SP (which is the request-line field delimiter),
 * and DEL (0x7F).
 */
static inline bool is_vchar(uint8_t b)
{
    return b >= 0x21 && b <= 0x7E;
}

/**
 * @brief True for bytes permitted in an HTTP header field-value (RFC 7230 §3.2).
 *
 * field-value = *( field-vchar / SP / HTAB )
 * field-vchar = VCHAR / obs-text (%x80-FF)
 *
 * Excludes all control characters (%x00-08, %x0A-1F, %x7F).
 * SP (0x20) and HTAB (0x09) are allowed as internal whitespace.
 * obs-text (%x80-FF) is kept for legacy compatibility.
 */
static inline bool is_field_value_char(uint8_t b)
{
    return b == 0x09                   // HTAB
           || (b >= 0x20 && b <= 0x7E) // SP through '~', no DEL
           || b >= 0x80;               // obs-text
}

/**
 * @brief Split a raw query string into key=value pairs.
 *
 * Operates in-place on `req->query[]`.  Pairs are `&`-separated; key and
 * value are split on the first `=`.  Keys or values longer than their
 * respective limits are silently truncated - the path itself remains valid.
 */
static void parse_query_params(HttpReq *req)
{
    const char *qs = req->query;
    size_t len = req->query_idx;
    size_t i = 0;

    while (i < len && req->query_count < MAX_QUERY_PARAMS)
    {
        QueryParam *qp = &req->query_params[req->query_count];
        size_t key_idx = 0;
        size_t val_idx = 0;
        bool in_val = false;

        while (i < len)
        {
            char c = qs[i++];
            if (c == '&')
                break;
            if (c == '=' && !in_val)
            {
                in_val = true;
                continue;
            }
            if (!in_val && key_idx < QUERY_KEY_LEN - 1)
                qp->key[key_idx++] = c;
            else if (in_val && val_idx < QUERY_VAL_LEN - 1)
                qp->val[val_idx++] = c;
        }

        if (key_idx > 0)
            req->query_count++;
    }
}

void http_parser_reset(HttpReq *req)
{
    uint8_t id = req->slot_id;
    *req = {};         // zero all fields
    req->slot_id = id; // restore slot identity
    req->parse_state = PARSE_METHOD;
    req->_version_hash = FNV_OFFSET; // seed the FNV-1a accumulator
}

void http_parser_feed(HttpReq *p, uint8_t byte)
{
    // Terminal states - do nothing
    switch (p->parse_state)
    {
    case PARSE_COMPLETE:
    case PARSE_ERROR:
    case PARSE_ENTITY_TOO_LARGE:
    case PARSE_URI_TOO_LONG:
        return;
    default:
        break;
    }

    char c = (char)byte;

    switch (p->parse_state)
    {

    case PARSE_METHOD:
        if (c == ' ')
        {
            p->parse_state = PARSE_PATH;
            p->current_token_idx = 0;
        }
        else if (!is_tchar(byte))
        {
            // RFC 7230 §3.1.1: method = token; any non-tchar is malformed
            p->parse_state = PARSE_ERROR;
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
        else if (!is_vchar(byte))
        {
            // RFC 3986 §3.3: path chars must be visible ASCII (or pct-encoded)
            p->parse_state = PARSE_ERROR;
        }
        else if (p->path_idx < MAX_PATH_LEN - 1)
        {
            p->path[p->path_idx++] = c;
        }
        else
        {
            p->parse_state = PARSE_URI_TOO_LONG;
        }
        break;

    case PARSE_QUERY:
        if (c == ' ')
        {
            parse_query_params(p);
            p->parse_state = PARSE_VERSION;
        }
        else if (!is_vchar(byte))
        {
            // Control chars and NUL are not valid query-string bytes
            p->parse_state = PARSE_ERROR;
        }
        else if (p->query_idx < MAX_QUERY_LEN - 1)
        {
            p->query[p->query_idx++] = c;
        }
        // Silently truncate - query overflow is a capacity limit, not a protocol error
        break;

    case PARSE_VERSION:
        if (c == '\r')
        {
            if (p->_version_hash == HASH_HTTP11)
                p->version = HTTP_11;
            else if (p->_version_hash == HASH_HTTP10)
                p->version = HTTP_10;
            else
                p->version = HTTP_UNKNOWN;
            p->parse_state = PARSE_EXPECT_LF;
        }
        else
        {
            p->_version_hash = (p->_version_hash ^ byte) * FNV_PRIME;
        }
        break;

    case PARSE_EXPECT_LF:
        if (c == '\n')
        {
            p->parse_state = PARSE_HEADER_KEY;
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
            if (p->current_token_idx == 0)
            {
                // Blank line - end of headers
                p->parse_state = PARSE_EXPECT_BODY_LF;
            }
            else
            {
                // CR mid-key: malformed (RFC 7230 §3.2 requires CRLF after value)
                p->parse_state = PARSE_ERROR;
            }
        }
        else if (c == ':')
        {
            // Terminate the scratch key so Host / Content-Length detection works
            // regardless of whether this header is stored (header_count < MAX).
            size_t k = p->current_token_idx < MAX_KEY_LEN ? p->current_token_idx : MAX_KEY_LEN - 1;
            p->cur_key[k] = '\0';
            p->parse_state = PARSE_HEADER_VAL;
            p->current_token_idx = 0;
        }
        else if (!is_tchar(byte))
        {
            // RFC 7230 §3.2: field-name = token; any non-tchar is malformed
            p->parse_state = PARSE_ERROR;
        }
        else
        {
            uint8_t h = p->header_count;
            if (p->current_token_idx < MAX_KEY_LEN - 1)
            {
                // Always capture into the scratch key; also store into the
                // header slot when one is still available.
                p->cur_key[p->current_token_idx] = c;
                if (h < MAX_HEADERS)
                    p->headers[h].key[p->current_token_idx] = c;
                p->current_token_idx++;
            }
            else if (h < MAX_HEADERS)
            {
                p->parse_state = PARSE_ERROR; // key too long for stored header
            }
            // Past MAX_HEADERS with an over-long key: the scratch key is already
            // capped; ignore the excess (it cannot match Host/Content-Length).
        }
        break;

    case PARSE_HEADER_VAL:
        // Strip leading OWS (SP or HTAB) after the colon - RFC 9110 §5.6.3
        if ((c == ' ' || c == '\t') && p->current_token_idx == 0)
            break;
        if (c == '\r')
        {
            uint8_t h = p->header_count;

            // Terminate the scratch value so detection sees a clean C string.
            size_t vlen = p->current_token_idx < MAX_VAL_LEN ? p->current_token_idx : MAX_VAL_LEN - 1;
            p->cur_val[vlen] = '\0';

            // Host / Content-Length detection works off the scratch copies, so
            // it is correct even for headers past MAX_HEADERS (RFC 7230 §5.4,
            // §3.3.2).
            if (strcasecmp(p->cur_key, "Host") == 0)
                p->host_count++;

            if (strcasecmp(p->cur_key, "Content-Length") == 0)
            {
                // RFC 7230 §3.3.2: Content-Length = 1*DIGIT.
                size_t cl = 0;
                bool valid = (p->cur_val[0] != '\0');
                for (const char *q = p->cur_val; *q; q++)
                {
                    if (*q < '0' || *q > '9')
                    {
                        valid = false;
                        break;
                    }
                    cl = cl * 10 + (size_t)(*q - '0');
                }
                // A non-numeric value, or a second Content-Length whose value
                // disagrees with the first, is a fatal framing error (request
                // smuggling vector) → 400.
                if (!valid || (p->content_length_count > 0 && cl != p->content_length))
                {
                    p->parse_state = PARSE_ERROR;
                    break;
                }
                p->content_length = cl;
                p->content_length_count++;
            }

            if (h < MAX_HEADERS)
                p->header_count++;

            p->parse_state = PARSE_EXPECT_LF;
            p->current_token_idx = 0;
        }
        else if (!is_field_value_char(byte))
        {
            // RFC 7230 §3.2: control chars and NUL are not valid in field values
            p->parse_state = PARSE_ERROR;
        }
        else if (p->current_token_idx < MAX_VAL_LEN - 1)
        {
            // Always capture into the scratch value; also store into the header
            // slot when one is still available.
            uint8_t h = p->header_count;
            p->cur_val[p->current_token_idx] = c;
            if (h < MAX_HEADERS)
                p->headers[h].val[p->current_token_idx] = c;
            p->current_token_idx++;
        }
        // Silently truncate - value overflow is a capacity limit, not a protocol error
        break;

    case PARSE_EXPECT_BODY_LF:
        /*
         * Consumes the LF of the blank-line CRLF that ends the header block.
         * Decides the next state based on Content-Length:
         *   > BODY_BUF_SIZE → 413 Payload Too Large
         *   == 0            → PARSE_COMPLETE (no body)
         *   else            → PARSE_BODY
         */
        if (c == '\n')
        {
            // RFC 7230 §5.4: a request MUST NOT carry more than one Host header
            // (always enforced); an HTTP/1.1 request MUST carry exactly one Host
            // header (enforced only when DETWS_ENFORCE_HOST_HEADER is set).
            bool host_violation = (p->host_count > 1);
#if DETWS_ENFORCE_HOST_HEADER
            if (p->version == HTTP_11 && p->host_count == 0)
                host_violation = true;
#endif
            if (host_violation)
                p->parse_state = PARSE_ERROR;
#if DETWS_ENABLE_OTA
            // Streaming sink (OTA): all headers are parsed here, so the hook can
            // match method/path/Authorization and begin a sink (e.g. Update).
            // If it accepts, the body streams in chunks and the size cap is
            // bypassed; the matching route handler still runs at PARSE_COMPLETE.
            else if (p->content_length > 0 && g_stream_begin && g_stream_begin(p))
            {
                p->body_streaming = true;
                p->parse_state = PARSE_BODY;
            }
#endif
            else if (p->content_length > BODY_BUF_SIZE)
                p->parse_state = PARSE_ENTITY_TOO_LARGE;
            else if (p->content_length == 0)
            {
                p->body[0] = '\0';
                p->parse_state = PARSE_COMPLETE;
            }
            else
            {
                p->parse_state = PARSE_BODY;
            }
        }
        else
        {
            p->parse_state = PARSE_ERROR;
        }
        break;

    case PARSE_BODY:
        // Body is opaque data - no character validation.
#if DETWS_ENABLE_OTA
        if (p->body_streaming)
        {
            // Reuse body[] as a flush buffer: fill it, then hand whole chunks to
            // the sink. No BODY_BUF_SIZE cap on the total - the body never lives
            // in RAM all at once.
            p->body[p->body_len++] = byte;
            if (p->body_len == BODY_BUF_SIZE)
            {
                if (g_stream_data)
                    g_stream_data(p->body, p->body_len);
                p->body_len = 0;
            }
            p->body_bytes_read++;
            if (p->body_bytes_read >= p->content_length)
            {
                if (p->body_len && g_stream_data)
                    g_stream_data(p->body, p->body_len); // flush the tail
                p->body_len = 0;
                p->body[0] = '\0';
                p->parse_state = PARSE_COMPLETE;
            }
            break;
        }
#endif
        if (p->body_len < BODY_BUF_SIZE)
            p->body[p->body_len++] = byte;
        p->body_bytes_read++;
        if (p->body_bytes_read >= p->content_length)
        {
            p->body[p->body_len] = '\0';
            p->parse_state = PARSE_COMPLETE;
        }
        break;

    default:
        break;
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
