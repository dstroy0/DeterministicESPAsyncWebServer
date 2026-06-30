// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file redis_resp.cpp
 * @brief Redis RESP2 command encoder + reply parser (pure, host-tested).
 */

#include "services/redis_resp.h"

#if DETWS_ENABLE_REDIS

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

    case '$': {
        int64_t blen;
        if (!parse_int(buf, header_from, header_to, &blen))
            return false;
        if (blen < 0) // $-1 = nil
        {
            out->type = RESP_NIL;
            *consumed = after_header;
            return true;
        }
        // Bound the bulk length against the remaining capacity without adding it (a 32-bit
        // size_t would wrap if we computed after_header + blen + 2 first).
        if (after_header + 2 > len || (uint64_t)blen > (uint64_t)(len - after_header - 2))
            return false;                              // bulk body + trailing CRLF not fully buffered
        size_t need = after_header + (size_t)blen + 2; // body + trailing CRLF
        if (buf[after_header + (size_t)blen] != '\r' || buf[after_header + (size_t)blen + 1] != '\n')
            return false; // malformed terminator
        out->type = RESP_BULK;
        out->str = (const char *)(buf + after_header);
        out->str_len = (size_t)blen;
        *consumed = need;
        return true;
    }

    case '*': {
        int64_t n;
        if (!parse_int(buf, header_from, header_to, &n))
            return false;
        if (n < 0) // *-1 = nil array
        {
            out->type = RESP_NIL;
            *consumed = after_header;
            return true;
        }
        out->type = RESP_ARRAY;
        out->ival = n;
        out->count = n;
        *consumed = after_header; // header only; the caller parses each element next
        return true;
    }

    default:
        return false; // unknown type byte
    }
}

#endif // DETWS_ENABLE_REDIS
