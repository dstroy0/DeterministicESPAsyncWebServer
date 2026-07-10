// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ftp.cpp
 * @brief FTP client wire codec implementation (see ftp.h).
 */

#include "ftp.h"

#if DETWS_ENABLE_FTP

#include <string.h>

static const size_t FTP_SENT = (size_t)-1; // "overflowed" sentinel threaded through the emitters

// Append raw bytes; propagates the overflow sentinel.
static size_t ftp_emit(char *buf, size_t cap, size_t n, const char *s, size_t slen)
{
    if (n == FTP_SENT || n + slen > cap)
        return FTP_SENT;
    memcpy(buf + n, s, slen);
    return n + slen;
}

// Append an unsigned decimal; propagates the overflow sentinel.
static size_t ftp_emit_uint(char *buf, size_t cap, size_t n, unsigned v)
{
    if (n == FTP_SENT)
        return FTP_SENT;
    char rev[10];
    int ri = 0;
    if (v == 0)
        rev[ri++] = '0';
    else
        while (v)
        {
            rev[ri++] = (char)('0' + (v % 10));
            v /= 10;
        }
    if (n + (size_t)ri > cap)
        return FTP_SENT;
    for (int k = 0; k < ri; k++)
        buf[n + k] = rev[ri - 1 - k];
    return n + (size_t)ri;
}

// Finish: on no overflow and room for the NUL, terminate and return the length; else 0.
static size_t ftp_finish(char *buf, size_t cap, size_t n)
{
    if (n == FTP_SENT || n + 1 > cap)
        return 0;
    buf[n] = 0;
    return n;
}

size_t ftp_build_command(char *buf, size_t cap, const char *verb, const char *arg)
{
    if (!buf || !verb || !verb[0])
        return 0;
    size_t n = 0;
    n = ftp_emit(buf, cap, n, verb, strlen(verb));
    if (arg && arg[0])
    {
        n = ftp_emit(buf, cap, n, " ", 1);
        n = ftp_emit(buf, cap, n, arg, strlen(arg));
    }
    n = ftp_emit(buf, cap, n, "\r\n", 2);
    return ftp_finish(buf, cap, n);
}

size_t ftp_build_port(char *buf, size_t cap, const uint8_t ip[4], uint16_t port)
{
    if (!buf || !ip)
        return 0;
    size_t n = 0;
    n = ftp_emit(buf, cap, n, "PORT ", 5);
    for (int i = 0; i < 4; i++)
    {
        n = ftp_emit_uint(buf, cap, n, ip[i]);
        n = ftp_emit(buf, cap, n, ",", 1);
    }
    n = ftp_emit_uint(buf, cap, n, (unsigned)(port >> 8));
    n = ftp_emit(buf, cap, n, ",", 1);
    n = ftp_emit_uint(buf, cap, n, (unsigned)(port & 0xFF));
    n = ftp_emit(buf, cap, n, "\r\n", 2);
    return ftp_finish(buf, cap, n);
}

size_t ftp_build_eprt(char *buf, size_t cap, const char *ip_str, bool ipv6, uint16_t port)
{
    if (!buf || !ip_str || !ip_str[0])
        return 0;
    size_t n = 0;
    n = ftp_emit(buf, cap, n, "EPRT |", 6);
    n = ftp_emit(buf, cap, n, ipv6 ? "2" : "1", 1);
    n = ftp_emit(buf, cap, n, "|", 1);
    n = ftp_emit(buf, cap, n, ip_str, strlen(ip_str));
    n = ftp_emit(buf, cap, n, "|", 1);
    n = ftp_emit_uint(buf, cap, n, port);
    n = ftp_emit(buf, cap, n, "|\r\n", 3);
    return ftp_finish(buf, cap, n);
}

static bool ftp_is_3digit(const char *p)
{
    return p[0] >= '0' && p[0] <= '9' && p[1] >= '0' && p[1] <= '9' && p[2] >= '0' && p[2] <= '9';
}

static int ftp_code3(const char *p)
{
    return (p[0] - '0') * 100 + (p[1] - '0') * 10 + (p[2] - '0');
}

// Index just past the LF of the line starting at @p start, or 0 if the line is not yet complete.
static size_t ftp_line_end(const char *buf, size_t len, size_t start)
{
    for (size_t i = start; i < len; i++)
        if (buf[i] == '\n')
            return i + 1;
    return 0;
}

bool ftp_parse_reply(const char *buf, size_t len, int *code, size_t *consumed)
{
    if (!buf || len < 4 || !ftp_is_3digit(buf))
        return false;
    int first = ftp_code3(buf);
    char sep = buf[3];

    if (sep == ' ')
    {
        size_t eol = ftp_line_end(buf, len, 0);
        if (!eol)
            return false; // line not fully received
        *code = first;
        *consumed = eol;
        return true;
    }
    if (sep != '-')
        return false; // malformed: the separator must be SP or '-'

    // Multiline: end at the first line that begins with the same code followed by a space.
    size_t pos = ftp_line_end(buf, len, 0);
    if (!pos)
        return false;
    while (pos < len)
    {
        if (len - pos >= 4 && ftp_is_3digit(buf + pos) && ftp_code3(buf + pos) == first && buf[pos + 3] == ' ')
        {
            size_t eol = ftp_line_end(buf, len, pos);
            if (!eol)
                return false; // terminator line not fully received
            *code = first;
            *consumed = eol;
            return true;
        }
        size_t eol = ftp_line_end(buf, len, pos);
        if (!eol)
            return false; // partial continuation line; need more
        pos = eol;
    }
    return false; // no terminator yet
}

bool ftp_parse_pasv(const char *buf, size_t len, uint8_t ip[4], uint16_t *port)
{
    if (!buf || !ip || !port)
        return false;
    size_t i = 0;
    while (i < len && buf[i] != '(')
        i++;
    if (i >= len)
        return false;
    i++; // past '('

    unsigned nums[6];
    for (int ni = 0; ni < 6; ni++)
    {
        if (i >= len || buf[i] < '0' || buf[i] > '9')
            return false; // the guard above guarantees at least one digit in this field
        unsigned v = 0;
        while (i < len && buf[i] >= '0' && buf[i] <= '9')
        {
            v = v * 10 + (unsigned)(buf[i] - '0');
            if (v > 255)
                return false;
            i++;
        }
        nums[ni] = v;
        if (ni < 5)
        {
            if (i >= len || buf[i] != ',')
                return false;
            i++;
        }
    }
    ip[0] = (uint8_t)nums[0];
    ip[1] = (uint8_t)nums[1];
    ip[2] = (uint8_t)nums[2];
    ip[3] = (uint8_t)nums[3];
    *port = (uint16_t)(nums[4] * 256 + nums[5]);
    return true;
}

bool ftp_parse_epsv(const char *buf, size_t len, uint16_t *port)
{
    if (!buf || !port)
        return false;
    size_t i = 0;
    while (i < len && buf[i] != '(')
        i++;
    if (i >= len)
        return false;
    i++; // past '('
    if (i >= len)
        return false;
    char d = buf[i]; // the delimiter (RFC 2428 recommends '|')

    // Skip the 3 leading delimiters (empty net-prt + net-addr fields) to reach the port field.
    int seen = 0;
    while (i < len && seen < 3)
    {
        if (buf[i] == d)
            seen++;
        i++;
    }
    if (seen < 3)
        return false;

    if (i >= len || buf[i] < '0' || buf[i] > '9')
        return false; // the guard above guarantees at least one port digit follows
    unsigned v = 0;
    while (i < len && buf[i] >= '0' && buf[i] <= '9')
    {
        v = v * 10 + (unsigned)(buf[i] - '0');
        if (v > 65535)
            return false;
        i++;
    }
    *port = (uint16_t)v;
    return true;
}

#endif // DETWS_ENABLE_FTP
