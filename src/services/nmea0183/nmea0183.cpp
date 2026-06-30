// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea0183.cpp
 * @brief NMEA 0183 sentence codec (pure, host-tested).
 */

#include "services/nmea0183/nmea0183.h"

#if DETWS_ENABLE_NMEA0183

uint8_t nmea0183_checksum(const char *s, size_t len)
{
    uint8_t cs = 0;
    for (size_t i = 0; i < len; i++)
        cs ^= (uint8_t)s[i];
    return cs;
}

static char hex_digit(uint8_t v)
{
    v &= 0x0Fu;
    return (char)(v < 10 ? ('0' + v) : ('A' + v - 10));
}

static int hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

size_t nmea0183_build(char *buf, size_t cap, const char *body)
{
    if (!buf || !body)
        return 0;
    size_t blen = strlen(body);
    size_t total = 1 + blen + 1 + 2 + 2; // '$' + body + '*' + HH + CRLF
    if (cap < total + 1)                 // + NUL
        return 0;
    uint8_t cs = nmea0183_checksum(body, blen);
    size_t p = 0;
    buf[p++] = '$';
    memcpy(buf + p, body, blen);
    p += blen;
    buf[p++] = '*';
    buf[p++] = hex_digit((uint8_t)(cs >> 4));
    buf[p++] = hex_digit(cs);
    buf[p++] = '\r';
    buf[p++] = '\n';
    buf[p] = '\0';
    return p;
}

bool nmea0183_parse(const char *s, size_t len, Nmea0183 *out)
{
    if (!s || !out || len < 4 || (s[0] != '$' && s[0] != '!'))
        return false;

    // Find the '*' that introduces the checksum, stopping at any CR/LF.
    size_t star = 0;
    bool found = false;
    for (size_t i = 1; i < len; i++)
    {
        if (s[i] == '*')
        {
            star = i;
            found = true;
            break;
        }
        if (s[i] == '\r' || s[i] == '\n')
            break;
    }
    if (!found || star + 2 >= len) // need two checksum hex digits after '*'
        return false;

    int hi = hex_val(s[star + 1]);
    int lo = hex_val(s[star + 2]);
    if (hi < 0 || lo < 0)
        return false;
    uint8_t expect = (uint8_t)((hi << 4) | lo);
    if (nmea0183_checksum(s + 1, star - 1) != expect)
        return false;

    // Split the payload s[1..star-1] on commas (field 0 is the address).
    uint8_t fc = 0;
    size_t fstart = 1;
    for (size_t i = 1; i <= star; i++)
    {
        if (i == star || s[i] == ',')
        {
            if (fc < DETWS_NMEA0183_MAX_FIELDS)
            {
                out->fields[fc] = s + fstart;
                out->field_len[fc] = (uint8_t)(i - fstart);
                fc++;
            }
            fstart = i + 1;
        }
    }
    out->field_count = fc;

    // Derive talker / type from the address field (field 0).
    memset(out->talker, 0, sizeof(out->talker));
    memset(out->type, 0, sizeof(out->type));
    if (fc > 0)
    {
        uint8_t al = out->field_len[0];
        const char *a = out->fields[0];
        for (uint8_t i = 0; i < 2 && i < al; i++)
            out->talker[i] = a[i];
        for (uint8_t i = 0; i < 3 && (uint8_t)(2 + i) < al; i++)
            out->type[i] = a[2 + i];
    }
    return true;
}

bool nmea0183_field_float(const Nmea0183 *m, uint8_t idx, float *out)
{
    if (!m || !out || idx >= m->field_count || m->field_len[idx] == 0)
        return false;
    const char *end = m->fields[idx];
    // The field is delimited by a ',' or '*' in the source, so det_strtof stops at the field end.
    float v = det_strtof(m->fields[idx], &end);
    if (end == m->fields[idx])
        return false;
    *out = v;
    return true;
}

bool nmea0183_field_int(const Nmea0183 *m, uint8_t idx, long *out)
{
    if (!m || !out || idx >= m->field_count || m->field_len[idx] == 0)
        return false;
    const char *end = m->fields[idx];
    long v = det_strtol(m->fields[idx], &end);
    if (end == m->fields[idx])
        return false;
    *out = v;
    return true;
}

#endif // DETWS_ENABLE_NMEA0183
