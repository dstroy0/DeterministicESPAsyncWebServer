// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntrip_caster.cpp
 * @brief NTRIP caster protocol codec - request parse + response / source-table build. See ntrip_caster.h.
 */

#include "services/gnss/ntrip_caster.h"

#if DWS_ENABLE_NTRIP_CASTER

#include <stdio.h>
#include <string.h>

namespace
{
char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

// Case-insensitive: does the line at [s,end) begin with prefix?
bool ci_prefix(const char *s, const char *end, const char *prefix)
{
    while (*prefix)
    {
        if (s >= end || lower(*s) != lower(*prefix))
            return false;
        s++;
        prefix++;
    }
    return true;
}

// Skip spaces / tabs.
const char *skip_ws(const char *s, const char *end)
{
    while (s < end && (*s == ' ' || *s == '\t'))
        s++;
    return s;
}

// Format a degree value to 2 decimals with integer math (newlib-nano often stubs %f).
void fmt_deg2(char *out, size_t cap, double v)
{
    int hundredths = (int)(v * 100.0 + (v >= 0.0 ? 0.5 : -0.5));
    int whole = hundredths / 100;
    int frac = hundredths % 100;
    if (frac < 0)
        frac = -frac;
    const char *sign = (v < 0.0 && whole == 0) ? "-" : ""; // preserve "-0.xx"
    snprintf(out, cap, "%s%d.%02d", sign, whole, frac);
}

// Find the request header block terminator (CRLFCRLF, or bare LFLF fallback). On success sets *hend
// just past the blank line and returns true; returns false if the block is incomplete (need more bytes).
bool find_header_end(const char *buf, size_t len, size_t *hend)
{
    for (size_t i = 0; i < len; i++)
    {
        if (i + 3 < len && buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n')
        {
            *hend = i + 4;
            return true;
        }
        if (i + 1 < len && buf[i] == '\n' && buf[i + 1] == '\n')
        {
            *hend = i + 2;
            return true;
        }
    }
    return false;
}

// Scan the header lines in [buf,end) for Ntrip-Version and Authorization: Basic, filling out.
void scan_headers(const char *buf, const char *end, NtripRequest *out)
{
    const char *line = buf;
    while (line < end)
    {
        const char *le = line;
        while (le < end && *le != '\n')
            le++;
        const char *lend = le; // exclusive; trim a trailing '\r'
        if (lend > line && *(lend - 1) == '\r')
            lend--;

        if (ci_prefix(line, lend, "ntrip-version:"))
        {
            const char *v = line + 14;
            while (v + 2 < lend && !(v[0] == '2' && v[1] == '.' && v[2] == '0'))
                v++;
            if (v + 2 < lend) // found "Ntrip/2.0"
                out->version = NtripVersion::NTRIP_V2;
        }
        else if (ci_prefix(line, lend, "authorization:"))
        {
            const char *v = skip_ws(line + 14, lend);
            if (ci_prefix(v, lend, "basic "))
            {
                v = skip_ws(v + 6, lend);
                out->auth_b64 = v;
                out->auth_b64_len = (uint16_t)(lend - v);
            }
        }
        line = le + 1;
    }
}
} // namespace

bool ntrip_request_parse(const char *buf, size_t len, NtripRequest *out)
{
    memset(out, 0, sizeof(*out));
    out->version = NtripVersion::NTRIP_V1;

    // Find the end of the request header block (blank line): CRLFCRLF, or bare LFLF as a fallback.
    size_t hend = 0;
    if (!find_header_end(buf, len, &hend))
        return false; // need more bytes
    out->complete = true;

    const char *end = buf + hend;

    // Request line: "GET <target> HTTP/1.x".
    const char *p = buf;
    if (!ci_prefix(p, end, "GET "))
    {
        out->is_get = false; // malformed / unsupported method
        return true;
    }
    out->is_get = true;
    p = skip_ws(p + 4, end);
    const char *target = p;
    while (p < end && *p != ' ' && *p != '\r' && *p != '\n' && *p != '?')
        p++;
    size_t tlen = (size_t)(p - target);

    if (tlen == 0 || (tlen == 1 && target[0] == '/'))
    {
        out->want_sourcetable = true; // "GET /" -> list the source table
    }
    else
    {
        const char *mp = target;
        size_t mlen = tlen;
        if (mp[0] == '/') // strip the leading slash
        {
            mp++;
            mlen--;
        }
        if (mlen >= sizeof(out->mountpoint))
            mlen = sizeof(out->mountpoint) - 1;
        memcpy(out->mountpoint, mp, mlen);
        out->mountpoint[mlen] = '\0';
    }

    // Scan the header lines for Ntrip-Version and Authorization.
    scan_headers(buf, end, out);
    return true;
}

size_t ntrip_build_stream_response(char *out, size_t cap, NtripVersion version)
{
    int n;
    if (version == NtripVersion::NTRIP_V2)
        n = snprintf(out, cap,
                     "HTTP/1.1 200 OK\r\n"
                     "Ntrip-Version: Ntrip/2.0\r\n"
                     "Server: DWS\r\n"
                     "Content-Type: gnss/data\r\n"
                     "Connection: close\r\n\r\n");
    else
        n = snprintf(out, cap, "ICY 200 OK\r\n\r\n");
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

size_t ntrip_build_error_response(char *out, size_t cap, NtripVersion version)
{
    int n;
    if (version == NtripVersion::NTRIP_V2)
        n = snprintf(out, cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
    else
        n = snprintf(out, cap, "ERROR - Bad Request\r\n");
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

size_t ntrip_build_unauthorized_response(char *out, size_t cap, NtripVersion version)
{
    int n;
    if (version == NtripVersion::NTRIP_V2)
        n = snprintf(out, cap,
                     "HTTP/1.1 401 Unauthorized\r\n"
                     "WWW-Authenticate: Basic realm=\"NTRIP\"\r\n"
                     "Content-Length: 0\r\n"
                     "Connection: close\r\n\r\n");
    else
        n = snprintf(out, cap, "ERROR - Bad Password\r\n");
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

size_t ntrip_build_str_record(char *out, size_t cap, const NtripMount *m)
{
    if (!m || !m->mountpoint)
        return 0;
    char lat[16];
    char lon[16];
    fmt_deg2(lat, sizeof(lat), m->lat_deg);
    fmt_deg2(lon, sizeof(lon), m->lon_deg);
    const char *ident = m->identifier ? m->identifier : "";
    const char *fmtd = m->format_details ? m->format_details : "1005(1)";
    const char *nav = m->nav_system ? m->nav_system : "GPS";
    const char *ctry = m->country ? m->country : "";
    const char *gen = m->generator ? m->generator : "DWS";
    // STR;mount;identifier;format;format-details;carrier;nav;network;country;lat;lon;nmea;solution;
    //     generator;compr;auth;fee;bitrate;misc   (carrier 0 = station reference only, no observations)
    int n = snprintf(out, cap, "STR;%s;%s;RTCM 3.3;%s;0;%s;none;%s;%s;%s;%d;0;%s;none;N;N;9600;", m->mountpoint, ident,
                     fmtd, nav, ctry, lat, lon, m->nmea_required ? 1 : 0, gen);
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

size_t ntrip_build_sourcetable(char *out, size_t cap, NtripVersion version, const NtripMount *mounts,
                               size_t mount_count)
{
    static const char ENDLINE[] = "ENDSOURCETABLE\r\n";

    // Pass 1: compute the body length (records + CRLFs + ENDSOURCETABLE) with a scratch record buffer.
    size_t body_len = 0;
    char rec[192];
    for (size_t i = 0; i < mount_count; i++)
    {
        size_t rn = ntrip_build_str_record(rec, sizeof(rec), &mounts[i]);
        if (rn == 0)
            return 0;
        body_len += rn + 2; // + CRLF
    }
    body_len += sizeof(ENDLINE) - 1;

    // Pass 2: header with the computed length, then the records, then ENDSOURCETABLE.
    int hn;
    if (version == NtripVersion::NTRIP_V2)
        hn = snprintf(out, cap,
                      "HTTP/1.1 200 OK\r\n"
                      "Ntrip-Version: Ntrip/2.0\r\n"
                      "Server: DWS\r\n"
                      "Content-Type: gnss/sourcetable\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      (unsigned)body_len);
    else
        hn = snprintf(out, cap,
                      "SOURCETABLE 200 OK\r\n"
                      "Server: DWS\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: %u\r\n\r\n",
                      (unsigned)body_len);
    if (hn < 0 || (size_t)hn >= cap)
        return 0;

    size_t pos = (size_t)hn;
    for (size_t i = 0; i < mount_count; i++)
    {
        size_t rn = ntrip_build_str_record(out + pos, cap - pos, &mounts[i]);
        if (rn == 0 || pos + rn + 2 >= cap)
            return 0;
        pos += rn;
        out[pos++] = '\r';
        out[pos++] = '\n';
    }
    if (pos + (sizeof(ENDLINE) - 1) >= cap)
        return 0;
    memcpy(out + pos, ENDLINE, sizeof(ENDLINE) - 1);
    pos += sizeof(ENDLINE) - 1;
    out[pos] = '\0';
    return pos;
}

#endif // DWS_ENABLE_NTRIP_CASTER
