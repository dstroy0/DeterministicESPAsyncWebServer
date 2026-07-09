// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_date.h
 * @brief Format a Unix epoch as an RFC 7231 IMF-fixdate (one shared copy).
 *
 * The `Date:` header formatter (gmtime_r + strftime to "%a, %d %b %Y %H:%M:%S GMT")
 * was open-coded in the NTP service; the time-source-fed Date header needs the same
 * thing. This header-only inline helper is the single home for it - reentrant
 * (gmtime_r, never the shared static tm buffer, so it is worker-safe) and fail-closed
 * (empty string, length 0) when the epoch is unset or does not fit a broken-down year.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_HTTP_DATE_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_HTTP_DATE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

/**
 * @brief Write @p epoch as an RFC 7231 IMF-fixdate into @p out (always GMT).
 * @return bytes written (excluding the NUL), or 0 with an empty @p out when @p epoch is 0,
 *         @p out is null / @p out_cap is 0, or the time cannot be broken down.
 */
inline size_t detws_http_date(time_t epoch, char *out, size_t out_cap)
{
    if (!out || out_cap == 0)
        return 0;
    if (epoch == 0)
    {
        out[0] = '\0';
        return 0;
    }
    struct tm tmv; // reentrant: gmtime_r, never the shared static buffer (worker-safe)
    if (!gmtime_r(&epoch, &tmv))
    {
        out[0] = '\0';
        return 0;
    }
    return strftime(out, out_cap, "%a, %d %b %Y %H:%M:%S GMT", &tmv);
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_HTTP_DATE_H
