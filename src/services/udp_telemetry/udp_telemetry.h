// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_telemetry.h
 * @brief Fire-and-forget UDP telemetry cast (DWS_ENABLE_UDP_TELEMETRY).
 *
 * Builds a metric line in InfluxDB line protocol -
 * `measurement,tag=v field=val,field2=val2 timestamp` (optional tags + trailing
 * timestamp; integer fields carry the `i` suffix, unsigned `u`, floats are plain)
 * - into a caller buffer, then casts it to a configured
 * collector over UDP (dws_udp_sendto), zero-heap and fire-and-forget (no ACK, no
 * retry). The line builder is pure and host-tested; only the send touches the
 * network (ESP32; a no-op on host builds).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_UDP_TELEMETRY_H
#define DETERMINISTICESPASYNCWEBSERVER_UDP_TELEMETRY_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_UDP_TELEMETRY

// ---------------------------------------------------------------------------
// Host-testable line builder (InfluxDB line protocol)
// ---------------------------------------------------------------------------

/** @brief Builder for one telemetry line over a caller buffer. */
struct DWSLine
{
    char *buf;        ///< destination buffer.
    size_t cap;       ///< buffer capacity in bytes.
    size_t pos;       ///< bytes written so far (excludes the null terminator).
    bool overflow;    ///< true once a write did not fit (line is then unusable).
    bool have_fields; ///< true once at least one field is present (comma control).
};

/** @brief Start a line for @p measurement (bound to @p buf / @p cap). */
void dws_line_init(DWSLine *l, char *buf, size_t cap, const char *measurement);

/**
 * @brief Append a `,key=value` tag (InfluxDB tag set, part of the series key).
 *
 * Tags MUST be added before any field (they sit between the measurement and the
 * fields); adding one after a field fails the line closed. Key and value are
 * escaped per line protocol (comma / equals / space backslash-escaped).
 */
void dws_line_add_tag(DWSLine *l, const char *key, const char *val);

/**
 * @brief Append the trailing ` <timestamp>` (line protocol; nanoseconds by default
 *        on InfluxDB). Call after all fields; a line with no field fails closed.
 */
void dws_line_set_timestamp(DWSLine *l, int64_t timestamp);

/** @brief Append `field=<v>i` (integer field). */
void dws_line_add_int(DWSLine *l, const char *field, int64_t v);

/** @brief Append `field=<v>i` (unsigned integer field). */
void dws_line_add_uint(DWSLine *l, const char *field, uint64_t v);

/** @brief Append `field=<v>` (float field, @p decimals places). */
void dws_line_add_float(DWSLine *l, const char *field, float v, uint8_t decimals);

/** @brief Encoded length (bytes), excluding the null terminator. */
size_t dws_line_len(const DWSLine *l);

/** @brief True if every field fit and the line has at least one field. */
bool dws_line_ok(const DWSLine *l);

// ---------------------------------------------------------------------------
// Cast (ESP32; no-op on host)
// ---------------------------------------------------------------------------

/** @brief Set the collector endpoint (dotted-quad IPv4 + UDP port). */
void dws_udp_telemetry_begin(const char *collector_ip, uint16_t port);

/** @brief Cast @p len raw bytes to the collector. @return false if not begun / host. */
bool dws_udp_telemetry_send(const char *data, size_t len);

/** @brief Cast a built line to the collector (no-op if the line overflowed). */
bool dws_udp_telemetry_cast(const DWSLine *l);

#endif // DWS_ENABLE_UDP_TELEMETRY
#endif // DETERMINISTICESPASYNCWEBSERVER_UDP_TELEMETRY_H
