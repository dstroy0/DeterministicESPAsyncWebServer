// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntp_service.h
 * @brief Optional SNTP wall-clock time sync (DETWS_ENABLE_NTP).
 *
 * Thin wrapper over the ESP-IDF SNTP client (`configTzTime`). Gives a
 * clock-less device wall-clock time for log timestamps, the HTTP `Date` header,
 * and future TLS certificate-validity checks. No-op stub when DETWS_ENABLE_NTP
 * is 0 or on non-Arduino builds.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_NTP_SERVICE_H
#define DETERMINISTICESPASYNCWEBSERVER_NTP_SERVICE_H

#include "shared_primitives/shim.h"

/**
 * @brief Start the SNTP client.
 *
 * Returns immediately; the first sync arrives asynchronously (poll
 * detws_ntp_synced()). Call once after the WiFi link is up.
 *
 * @param tz       POSIX TZ string (e.g. "UTC0", "EST5EDT,M3.2.0,M11.1.0").
 *                 Pass nullptr for UTC.
 * @param server1  Primary NTP server (default "pool.ntp.org").
 * @param server2  Secondary NTP server (default "time.nist.gov").
 * @return true if the client was started; false if disabled at compile time.
 */
bool detws_ntp_begin(const char *tz = nullptr, const char *server1 = "pool.ntp.org",
                     const char *server2 = "time.nist.gov");

/**
 * @brief True once a plausible wall-clock time has been obtained from SNTP.
 *
 * Checks that the system clock has advanced past 2021-01-01, which only happens
 * after a successful sync (the RTC starts at the epoch on a cold boot).
 */
bool detws_ntp_synced();

/**
 * @brief Current Unix epoch seconds, or 0 if not yet synced (or disabled).
 */
time_t detws_ntp_epoch();

/**
 * @brief Format the current time as an RFC 7231 IMF-fixdate (HTTP `Date`).
 *
 * Writes e.g. "Sun, 06 Nov 1994 08:49:37 GMT" into @p out. Always GMT.
 *
 * @param out      Destination buffer (>= 30 bytes recommended).
 * @param out_cap  Capacity of @p out.
 * @return Number of characters written (excluding the null), or 0 if time is
 *         not yet available / disabled.
 */
size_t detws_ntp_http_date(char *out, size_t out_cap);

#if !defined(ARDUINO)
/**
 * @brief Host-only test seam: inject a wall-clock epoch so time-dependent paths
 *        (e.g. the optional HTTP Date header) are exercisable off-device. 0 = none.
 */
void detws_ntp_set_test_epoch(time_t epoch);
#endif

#endif // DETERMINISTICESPASYNCWEBSERVER_NTP_SERVICE_H
