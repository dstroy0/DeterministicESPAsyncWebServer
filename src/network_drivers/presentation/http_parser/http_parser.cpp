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

#include "network_drivers/presentation/http_parser/http_parser.h"

HttpReq http_pool[MAX_CONNS];

#if DETWS_ENABLE_STREAM_BODY
// Streaming-body hooks (OTA / file upload). Null unless the application installs them.
static HttpStreamBeginCb g_stream_begin = nullptr;
static HttpStreamDataCb g_stream_data = nullptr;
static HttpStreamAbortCb g_stream_abort = nullptr;

void http_parser_set_stream_hooks(HttpStreamBeginCb begin, HttpStreamDataCb data, HttpStreamAbortCb abort)
{
    g_stream_begin = begin;
    g_stream_data = data;
    g_stream_abort = abort;
}
#endif // DETWS_ENABLE_STREAM_BODY

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
#if DETWS_ENABLE_STREAM_BODY
    // A streamed body that never reached PARSE_COMPLETE is being torn down (peer
    // reset / timeout / error): let the sink release its resource before we wipe
    // the state. The normal-completion reset runs while parse_state==PARSE_COMPLETE
    // (the handler already finished the sink), so this fires only on abort.
    if (req->body_streaming && req->parse_state != PARSE_COMPLETE && g_stream_abort)
        g_stream_abort(req);
#endif
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
#if DETWS_CAPTURE_AUTH_HEADER
            // The Authorization value (Digest / JWT bearer) exceeds MAX_VAL_LEN,
            // so capture it whole into a dedicated buffer independent of scratch.
            p->cur_is_auth = (strcasecmp(p->cur_key, "Authorization") == 0);
            if (p->cur_is_auth)
                p->auth_idx = 0;
#endif
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
            // An over-long key is silently capped (a capacity limit, not an
            // error): the scratch/stored key is already full and the excess is
            // ignored. A truncated key cannot match the short Host/Content-Length
            // names, and not failing the request keeps long but valid header names
            // (CORS, Sec-WebSocket-Extensions, ...) working. Mirrors the value path.
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
#if DETWS_CAPTURE_AUTH_HEADER
            if (p->cur_is_auth)
            {
                p->authorization[p->auth_idx] = '\0';
                p->cur_is_auth = false;
            }
#endif

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

            // RFC 9112 §6.1/§6.3: this server does not decode chunked request bodies,
            // and a Transfer-Encoding present with (or instead of) Content-Length is a
            // request-smuggling vector - the chunked octets would otherwise be left in
            // the buffer and reparsed as the next request. Reject any request bearing
            // Transfer-Encoding (fail closed).
            if (strcasecmp(p->cur_key, "Transfer-Encoding") == 0)
            {
                p->parse_state = PARSE_ERROR;
                break;
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
        else
        {
#if DETWS_CAPTURE_AUTH_HEADER
            // Capture the full Authorization value (Digest / JWT) past MAX_VAL_LEN.
            if (p->cur_is_auth && p->auth_idx < DETWS_AUTH_HDR_CAP - 1)
                p->authorization[p->auth_idx++] = c;
#endif
            if (p->current_token_idx < MAX_VAL_LEN - 1)
            {
                // Always capture into the scratch value; also store into the
                // header slot when one is still available.
                uint8_t h = p->header_count;
                p->cur_val[p->current_token_idx] = c;
                if (h < MAX_HEADERS)
                    p->headers[h].val[p->current_token_idx] = c;
                p->current_token_idx++;
            }
            // Silently truncate the scratch/stored value - capacity limit, not an error.
        }
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
#if DETWS_ENABLE_STREAM_BODY
            // Streaming sink (OTA / upload): all headers are parsed here, so the
            // hook can match method/path/Authorization and begin a sink (Update
            // or a file). If it accepts, the body streams in chunks and the size
            // cap is bypassed; the matching route handler still runs at COMPLETE.
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
#if DETWS_ENABLE_STREAM_BODY
        if (p->body_streaming)
        {
            // Reuse body[] as a flush buffer: fill it, then hand whole chunks to
            // the sink. No BODY_BUF_SIZE cap on the total - the body never lives
            // in RAM all at once.
            p->body[p->body_len++] = byte;
            if (p->body_len == BODY_BUF_SIZE)
            {
                if (g_stream_data)
                    g_stream_data(p, p->body, p->body_len);
                p->body_len = 0;
            }
            p->body_bytes_read++;
            if (p->body_bytes_read >= p->content_length)
            {
                if (p->body_len && g_stream_data)
                    g_stream_data(p, p->body, p->body_len); // flush the tail
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

bool http_get_cookie(const HttpReq *req, const char *name, char *out, size_t out_size)
{
    if (out == nullptr || out_size == 0)
        return false;
    out[0] = '\0';
    if (req == nullptr || name == nullptr || name[0] == '\0')
        return false;

    // RFC 6265 4.2.1: the request "Cookie" header is "name1=value1; name2=value2".
    // Names are case-sensitive; a value may be DQUOTE-wrapped.
    const char *c = http_get_header(req, "Cookie");
    if (c == nullptr)
        return false;
    size_t nlen = strlen(name);

    const char *p = c;
    while (*p != '\0')
    {
        while (*p == ' ' || *p == '\t' || *p == ';') // skip inter-pair separators/spaces
            p++;
        if (*p == '\0')
            break;
        const char *eq = p;
        while (*eq != '\0' && *eq != '=' && *eq != ';') // cookie-name runs up to '='
            eq++;
        if (*eq == '=' && (size_t)(eq - p) == nlen && strncmp(p, name, nlen) == 0)
        {
            const char *v = eq + 1;
            const char *end = v;
            while (*end != '\0' && *end != ';') // value runs up to the next ';'
                end++;
            while (end > v && (end[-1] == ' ' || end[-1] == '\t')) // trim trailing OWS
                end--;
            size_t vlen = (size_t)(end - v);
            if (vlen >= 2 && v[0] == '"' && v[vlen - 1] == '"') // strip a quoted cookie-value
            {
                v++;
                vlen -= 2;
            }
            if (vlen >= out_size)
                vlen = out_size - 1;
            memcpy(out, v, vlen);
            out[vlen] = '\0';
            return true;
        }
        p = eq;
        while (*p != '\0' && *p != ';') // advance past this pair
            p++;
    }
    return false;
}

// Copy an IPv4 token (strip surrounding quotes, IPv6 brackets are rejected, drop a
// trailing :port) from [s, s+n) into out. Returns true if it looks like dotted IPv4.
static bool fwd_copy_ipv4(const char *s, size_t n, char *out, size_t cap)
{
    // Trim leading/trailing OWS and a wrapping DQUOTE.
    while (n > 0 && (*s == ' ' || *s == '\t'))
    {
        s++;
        n--;
    }
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
        n--;
    if (n >= 2 && s[0] == '"' && s[n - 1] == '"')
    {
        s++;
        n -= 2;
    }
    if (n == 0 || s[0] == '[')
        return false; // empty, or IPv6 ("[...]") which this IPv4 stack does not key on
    // Drop a trailing ":port" (only one colon, IPv4 has none).
    size_t ip_len = 0;
    int dots = 0, digits = 0;
    for (; ip_len < n; ip_len++)
    {
        char c = s[ip_len];
        if (c == ':')
            break;
        if (c == '.')
            dots++;
        else if (c >= '0' && c <= '9')
            digits++;
        else
            return false; // not a bare IPv4 (could be "unknown" / "_obf")
    }
    if (dots != 3 || digits == 0 || ip_len + 1 > cap)
        return false;
    memcpy(out, s, ip_len);
    out[ip_len] = '\0';
    return true;
}

bool http_forwarded_client(const HttpReq *req, char *ip_out, size_t ip_cap, bool *is_https)
{
    if (is_https)
        *is_https = false;
    if (!ip_out || ip_cap == 0 || !req)
        return false;
    ip_out[0] = '\0';

    // Prefer RFC 7239 "Forwarded" (the leftmost element is the original client):
    //   Forwarded: for=192.0.2.60;proto=https, for=198.51.100.1
    const char *fwd = http_get_header(req, "Forwarded");
    if (fwd)
    {
        // First element = up to the first ','. Within it, find for= and proto=.
        const char *elem_end = strchr(fwd, ',');
        size_t elen = elem_end ? (size_t)(elem_end - fwd) : strlen(fwd);
        // proto=
        if (is_https)
        {
            // Only the first element's proto= matters, so this is a single check.
            const char *hit = strstr(fwd, "proto=");
            if (hit && (size_t)(hit - fwd) < elen)
                *is_https = (strncasecmp(hit + 6, "https", 5) == 0);
        }
        // for=
        const char *f = strstr(fwd, "for=");
        if (f && (size_t)(f - fwd) < elen)
        {
            const char *fv = f + 4;
            const char *fend = fv;
            size_t lim = elen - (size_t)(fv - fwd);
            size_t k = 0;
            while (k < lim && fend[k] != ';' && fend[k] != ',')
                k++;
            if (fwd_copy_ipv4(fv, k, ip_out, ip_cap))
                return true;
        }
    }

    // De-facto X-Forwarded-For (comma list; leftmost = original client) + X-Forwarded-Proto.
    if (is_https)
    {
        const char *xfp = http_get_header(req, "X-Forwarded-Proto");
        if (xfp && strncasecmp(xfp, "https", 5) == 0)
            *is_https = true;
    }
    const char *xff = http_get_header(req, "X-Forwarded-For");
    if (xff)
    {
        const char *end = strchr(xff, ',');
        size_t len = end ? (size_t)(end - xff) : strlen(xff);
        if (fwd_copy_ipv4(xff, len, ip_out, ip_cap))
            return true;
    }
    return false;
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

bool http_get_form(const HttpReq *req, const char *key, char *out, size_t out_size)
{
    if (out == nullptr || out_size == 0)
        return false;
    out[0] = '\0';
    if (req == nullptr || key == nullptr)
        return false;

    // Only urlencoded bodies (allow a trailing "; charset=..." suffix).
    const char *ct = http_get_header(req, "Content-Type");
    if (ct == nullptr || strncasecmp(ct, "application/x-www-form-urlencoded", 33) != 0)
        return false;

    const char *body = (const char *)req->body;
    size_t len = req->body_len;
    size_t key_len = strlen(key);
    size_t i = 0;

    while (i < len)
    {
        size_t ks = i;
        while (i < len && body[i] != '=' && body[i] != '&')
            i++;
        bool key_matches = (i - ks == key_len) && (strncmp(body + ks, key, key_len) == 0);

        size_t vs = i, ve = i;
        if (i < len && body[i] == '=')
        {
            vs = ++i;
            while (i < len && body[i] != '&')
                i++;
            ve = i;
        }
        if (i < len && body[i] == '&')
            i++;

        if (key_matches)
        {
            size_t vlen = ve - vs;
            if (vlen > out_size - 1)
                vlen = out_size - 1;
            memcpy(out, body + vs, vlen);
            out[vlen] = '\0';
            return true;
        }
    }
    return false;
}

const char *http_get_param(const HttpReq *req, const char *key)
{
    if (req == nullptr || key == nullptr)
        return nullptr;
    for (uint8_t i = 0; i < req->path_param_count; i++)
    {
        if (strcmp(req->path_params[i].key, key) == 0)
            return req->path_params[i].val;
    }
    return nullptr;
}
