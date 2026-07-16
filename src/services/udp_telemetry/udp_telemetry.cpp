// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_telemetry.cpp
 * @brief InfluxDB line-protocol builder (pure) + UDP cast to a collector.
 *
 * The builder is host-tested; the cast uses det_udp_sendto on ESP32 and is a
 * no-op on host builds (no transport dependency pulled into the unit test).
 */

#include "services/udp_telemetry/udp_telemetry.h"

#if DETWS_ENABLE_UDP_TELEMETRY

#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Line builder (pure)
// ---------------------------------------------------------------------------

static void line_append(DetwsLine *l, const char *s)
{
    if (l->overflow)
        return;
    size_t n = strnlen(s, l->cap + 1);
    if (l->pos + n >= l->cap) // keep room for the null terminator
    {
        l->overflow = true;
        return;
    }
    memcpy(l->buf + l->pos, s, n);
    l->pos += n;
    l->buf[l->pos] = '\0';
}

// Separator before a field: a space before the first, a comma after.
static void line_sep(DetwsLine *l)
{
    line_append(l, l->have_fields ? "," : " ");
    l->have_fields = true;
}

void detws_line_init(DetwsLine *l, char *buf, size_t cap, const char *measurement)
{
    l->buf = buf;
    l->cap = cap;
    l->pos = 0;
    l->overflow = false;
    l->have_fields = false;
    if (cap)
        buf[0] = '\0';
    line_append(l, measurement ? measurement : "");
}

// Append a string with InfluxDB tag/key escaping: comma, equals and space are
// backslash-escaped (line protocol, "Special characters").
static void line_append_escaped(DetwsLine *l, const char *s)
{
    if (!s)
        return;
    for (const char *p = s; *p; p++)
    {
        if (*p == ',' || *p == '=' || *p == ' ')
        {
            char esc[3] = {'\\', *p, '\0'};
            line_append(l, esc);
        }
        else
        {
            char one[2] = {*p, '\0'};
            line_append(l, one);
        }
    }
}

void detws_line_add_tag(DetwsLine *l, const char *key, const char *val)
{
    // Tags are part of the series key: they come right after the measurement,
    // comma-separated, BEFORE the space-separated fields. Adding one after a field
    // is a misuse -> fail the line closed.
    if (l->have_fields)
    {
        l->overflow = true;
        return;
    }
    line_append(l, ",");
    line_append_escaped(l, key);
    line_append(l, "=");
    line_append_escaped(l, val);
}

void detws_line_set_timestamp(DetwsLine *l, int64_t timestamp)
{
    if (!l->have_fields) // a line needs at least one field before the timestamp
    {
        l->overflow = true;
        return;
    }
    char num[24];
    snprintf(num, sizeof(num), " %lld", (long long)timestamp); // space-separated trailing timestamp
    line_append(l, num);
}

void detws_line_add_int(DetwsLine *l, const char *field, int64_t v)
{
    char num[24];
    snprintf(num, sizeof(num), "%lldi", (long long)v); // InfluxDB integer suffix
    line_sep(l);
    line_append(l, field);
    line_append(l, "=");
    line_append(l, num);
}

void detws_line_add_uint(DetwsLine *l, const char *field, uint64_t v)
{
    char num[24];
    snprintf(num, sizeof(num), "%lluu", (unsigned long long)v); // InfluxDB UInteger suffix 'u' (not signed 'i')
    line_sep(l);
    line_append(l, field);
    line_append(l, "=");
    line_append(l, num);
}

void detws_line_add_float(DetwsLine *l, const char *field, float v, uint8_t decimals)
{
    char num[32];
    snprintf(num, sizeof(num), "%.*f", (int)decimals, (double)v);
    line_sep(l);
    line_append(l, field);
    line_append(l, "=");
    line_append(l, num);
}

size_t detws_line_len(const DetwsLine *l)
{
    return l->pos;
}

bool detws_line_ok(const DetwsLine *l)
{
    return !l->overflow && l->have_fields;
}

// ---------------------------------------------------------------------------
// Cast
// ---------------------------------------------------------------------------

#ifdef ARDUINO

#include "network_drivers/transport/udp.h"

namespace
{
// All UDP-telemetry cast state, owned by one instance (internal linkage): the collector
// endpoint and the begun flag, grouped so it is one named owner, unreachable cross-TU.
struct UdpTelemetryCtx
{
    char ip[16] = {0};
    uint16_t port = 0;
    bool begun = false;
};
UdpTelemetryCtx s_ut;
} // namespace

void detws_udp_telemetry_begin(const char *collector_ip, uint16_t port)
{
    strncpy(s_ut.ip, collector_ip ? collector_ip : "", sizeof(s_ut.ip) - 1);
    s_ut.ip[sizeof(s_ut.ip) - 1] = '\0';
    s_ut.port = port;
    s_ut.begun = true;
}

bool detws_udp_telemetry_send(const char *data, size_t len)
{
    if (!s_ut.begun || !data)
        return false;
    return det_udp_sendto(s_ut.ip, s_ut.port, (const uint8_t *)data, len);
}

#else // host build - no network

void detws_udp_telemetry_begin(const char *, uint16_t)
{
}

bool detws_udp_telemetry_send(const char *, size_t)
{
    return false;
}

#endif // ARDUINO

bool detws_udp_telemetry_cast(const DetwsLine *l)
{
    if (!detws_line_ok(l))
        return false;
    return detws_udp_telemetry_send(l->buf, l->pos);
}

#endif // DETWS_ENABLE_UDP_TELEMETRY
