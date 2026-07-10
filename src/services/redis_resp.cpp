// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file redis_resp.cpp
 * @brief Redis RESP2 command encoder + reply parser (pure, host-tested).
 */

#include "services/redis_resp.h"

#if DETWS_ENABLE_REDIS

#include <stdio.h> // snprintf for the decimal length prefixes
#include <string.h>

size_t resp_encode_command(char *buf, size_t cap, const char *const *args, const size_t *arg_lens, size_t argc)
{
    if (!buf || cap == 0 || !args || argc == 0)
        return 0;
    size_t pos = 0;
    char hdr[24];

    int n = snprintf(hdr, sizeof(hdr), "*%zu\r\n", argc);
    if (n < 0 || pos + (size_t)n >= cap)
        return 0;
    memcpy(buf + pos, hdr, (size_t)n);
    pos += (size_t)n;

    for (size_t i = 0; i < argc; i++)
    {
        if (!args[i])
            return 0;
        size_t alen = arg_lens ? arg_lens[i] : strlen(args[i]);
        n = snprintf(hdr, sizeof(hdr), "$%zu\r\n", alen);
        if (n < 0 || pos + (size_t)n + alen + 2 >= cap) // header + arg + trailing CRLF + NUL room
            return 0;
        memcpy(buf + pos, hdr, (size_t)n);
        pos += (size_t)n;
        memcpy(buf + pos, args[i], alen);
        pos += alen;
        buf[pos++] = '\r';
        buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    return pos;
}

// Find the CRLF that ends the line starting at buf[from]; returns the index of the
// '\r', or len if not found (incomplete line).
static size_t find_crlf(const uint8_t *buf, size_t len, size_t from)
{
    for (size_t i = from; i + 1 < len; i++)
        if (buf[i] == '\r' && buf[i + 1] == '\n')
            return i;
    return len;
}

// Parse a base-10 (optionally negative) integer from [buf+from, buf+end).
static bool parse_int(const uint8_t *buf, size_t from, size_t end, int64_t *out)
{
    if (from >= end)
        return false;
    bool neg = false;
    size_t i = from;
    if (buf[i] == '-')
    {
        neg = true;
        i++;
    }
    if (i >= end)
        return false;
    int64_t v = 0;
    for (; i < end; i++)
    {
        if (buf[i] < '0' || buf[i] > '9')
            return false;
        v = v * 10 + (buf[i] - '0');
    }
    *out = neg ? -v : v;
    return true;
}

// Case-insensitively compare the slice [buf+from, buf+end) to the C string s.
static bool slice_ieq(const uint8_t *buf, size_t from, size_t end, const char *s)
{
    for (size_t i = from; i < end; i++, s++)
    {
        if (*s == '\0')
            return false;
        uint8_t a = buf[i];
        if (a >= 'A' && a <= 'Z')
            a = (uint8_t)(a + 32);
        char b = *s;
        if (b >= 'A' && b <= 'Z')
            b = (char)(b + 32);
        if (a != (uint8_t)b)
            return false;
    }
    return *s == '\0';
}

// Best-effort decimal-to-double for a RESP3 double line: inf / -inf / nan, or
// [sign] int [.frac] [(e|E)[sign]exp]. The raw text stays authoritative in str.
static bool parse_double(const uint8_t *buf, size_t from, size_t end, double *out)
{
    if (slice_ieq(buf, from, end, "inf") || slice_ieq(buf, from, end, "+inf"))
    {
        *out = 1e308 * 10.0;
        return true;
    }
    if (slice_ieq(buf, from, end, "-inf"))
    {
        *out = -1e308 * 10.0;
        return true;
    }
    if (slice_ieq(buf, from, end, "nan"))
    {
        double z = 0.0;
        *out = z / z;
        return true;
    }
    size_t i = from;
    bool neg = false;
    if (i < end && (buf[i] == '+' || buf[i] == '-'))
    {
        neg = (buf[i] == '-');
        i++;
    }
    double mant = 0.0;
    bool any = false;
    for (; i < end && buf[i] >= '0' && buf[i] <= '9'; i++)
    {
        mant = mant * 10.0 + (buf[i] - '0');
        any = true;
    }
    if (i < end && buf[i] == '.')
    {
        i++;
        double scale = 0.1;
        for (; i < end && buf[i] >= '0' && buf[i] <= '9'; i++)
        {
            mant += (buf[i] - '0') * scale;
            scale *= 0.1;
            any = true;
        }
    }
    if (!any)
        return false;
    int exp = 0;
    if (i < end && (buf[i] == 'e' || buf[i] == 'E'))
    {
        i++;
        bool eneg = false;
        if (i < end && (buf[i] == '+' || buf[i] == '-'))
        {
            eneg = (buf[i] == '-');
            i++;
        }
        bool edig = false;
        for (; i < end && buf[i] >= '0' && buf[i] <= '9'; i++)
        {
            exp = exp * 10 + (buf[i] - '0');
            edig = true;
        }
        if (!edig)
            return false;
        if (eneg)
            exp = -exp;
    }
    if (i != end)
        return false; // trailing garbage
    double scale = 1.0;
    int e = exp < 0 ? -exp : exp;
    for (int k = 0; k < e; k++)
        scale *= 10.0;
    mant = (exp < 0) ? (mant / scale) : (mant * scale);
    *out = neg ? -mant : mant;
    return true;
}

bool resp_parse(const uint8_t *buf, size_t len, RespReply *out, size_t *consumed)
{
    if (!buf || len < 3 || !out || !consumed) // shortest value is "+\r\n" style; need a type + CRLF
        return false;

    size_t crlf = find_crlf(buf, len, 1);
    if (crlf == len)
        return false; // header line incomplete
    size_t header_from = 1, header_to = crlf;
    size_t after_header = crlf + 2; // past \r\n

    out->str = nullptr;
    out->str_len = 0;
    out->ival = 0;
    out->dval = 0;
    out->count = 0;

    switch (buf[0])
    {
    case '+':
    case '-':
        out->type = (buf[0] == '+') ? RESP_SIMPLE : RESP_ERROR;
        out->str = (const char *)(buf + header_from);
        out->str_len = header_to - header_from;
        *consumed = after_header;
        return true;

    case ':': {
        int64_t v;
        if (!parse_int(buf, header_from, header_to, &v))
            return false;
        out->type = RESP_INTEGER;
        out->ival = v;
        *consumed = after_header;
        return true;
    }

    // Length-prefixed bodies: bulk string ($), bulk error (!), verbatim string (=).
    case '$':
    case '!':
    case '=': {
        int64_t blen;
        if (!parse_int(buf, header_from, header_to, &blen))
            return false;
        if (buf[0] == '$' && blen < 0) // $-1 = nil
        {
            out->type = RESP_NIL;
            *consumed = after_header;
            return true;
        }
        if (blen < 0)
            return false;
        // Bound the length against the remaining capacity without adding it (a 32-bit
        // size_t would wrap if we computed after_header + blen + 2 first).
        if (after_header + 2 > len || (uint64_t)blen > (uint64_t)(len - after_header - 2))
            return false;                              // body + trailing CRLF not fully buffered
        size_t need = after_header + (size_t)blen + 2; // body + trailing CRLF
        if (buf[after_header + (size_t)blen] != '\r' || buf[after_header + (size_t)blen + 1] != '\n')
            return false; // malformed terminator
        out->type = (buf[0] == '$') ? RESP_BULK : (buf[0] == '!') ? RESP_BULK_ERROR : RESP_VERBATIM;
        out->str = (const char *)(buf + after_header);
        out->str_len = (size_t)blen;
        *consumed = need;
        return true;
    }

    // Aggregates whose children follow: array (*), set (~), push (>).
    case '*':
    case '~':
    case '>': {
        int64_t n;
        if (!parse_int(buf, header_from, header_to, &n))
            return false;
        if (buf[0] == '*' && n < 0) // *-1 = nil array
        {
            out->type = RESP_NIL;
            *consumed = after_header;
            return true;
        }
        if (n < 0)
            return false;
        out->type = (buf[0] == '*') ? RESP_ARRAY : (buf[0] == '~') ? RESP_SET : RESP_PUSH;
        out->ival = n;
        out->count = n;
        *consumed = after_header; // header only; the caller parses each element next
        return true;
    }

    case '%': { // map: N pairs -> 2N following child values
        int64_t n;
        if (!parse_int(buf, header_from, header_to, &n) || n < 0)
            return false;
        out->type = RESP_MAP;
        out->ival = n * 2;
        out->count = n * 2;
        *consumed = after_header;
        return true;
    }

    case '_': // RESP3 null
        out->type = RESP_NIL;
        *consumed = after_header;
        return true;

    case '#': { // boolean
        if (header_to - header_from != 1 || (buf[header_from] != 't' && buf[header_from] != 'f'))
            return false;
        out->type = RESP_BOOL;
        out->ival = (buf[header_from] == 't') ? 1 : 0;
        *consumed = after_header;
        return true;
    }

    case ',': // double (text authoritative; dval best-effort)
        out->type = RESP_DOUBLE;
        out->str = (const char *)(buf + header_from);
        out->str_len = header_to - header_from;
        parse_double(buf, header_from, header_to, &out->dval);
        *consumed = after_header;
        return true;

    case '(': // big number (digits kept as text)
        out->type = RESP_BIG_NUMBER;
        out->str = (const char *)(buf + header_from);
        out->str_len = header_to - header_from;
        *consumed = after_header;
        return true;

    default:
        return false; // unknown type byte
    }
}

#endif // DETWS_ENABLE_REDIS
