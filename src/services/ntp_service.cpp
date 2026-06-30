// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntp_service.cpp
 * @brief SNTP wall-clock time sync implementation (DETWS_ENABLE_NTP).
 */

#include "ntp_service.h"

#if DETWS_ENABLE_NTP && defined(ARDUINO)

#include <Arduino.h>

// A successful sync moves the clock well past this sentinel (2021-01-01 UTC);
// a cold-booted RTC sits near the Unix epoch.
static const time_t DETWS_NTP_PLAUSIBLE_EPOCH = 1609459200;

bool detws_ntp_begin(const char *tz, const char *server1, const char *server2)
{
    // configTzTime applies the POSIX TZ and starts the SNTP client (async).
    configTzTime(tz ? tz : "UTC0", server1, server2);
    return true;
}

bool detws_ntp_synced()
{
    return time(nullptr) > DETWS_NTP_PLAUSIBLE_EPOCH;
}

time_t detws_ntp_epoch()
{
    time_t now = time(nullptr);
    return (now > DETWS_NTP_PLAUSIBLE_EPOCH) ? now : 0;
}

size_t detws_ntp_http_date(char *out, size_t out_cap)
{
    if (!out || out_cap == 0)
        return 0;
    time_t now = detws_ntp_epoch();
    if (now == 0)
    {
        out[0] = '\0';
        return 0;
    }
    struct tm gmt;
    gmtime_r(&now, &gmt);
    // RFC 7231 §7.1.1.1 IMF-fixdate, always GMT.
    return strftime(out, out_cap, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
}

#else

// Host build: no SNTP. A test seam lets a unit test inject a wall-clock epoch so
// the Date-header path (and any time-dependent code) is exercisable off-device.
#include <time.h>
static time_t s_host_test_epoch = 0;
void detws_ntp_set_test_epoch(time_t epoch)
{
    s_host_test_epoch = epoch;
}

bool detws_ntp_begin(const char *tz, const char *server1, const char *server2)
{
    (void)tz;
    (void)server1;
    (void)server2;
    return false;
}
bool detws_ntp_synced()
{
    return s_host_test_epoch != 0;
}
time_t detws_ntp_epoch()
{
    return s_host_test_epoch;
}
size_t detws_ntp_http_date(char *out, size_t out_cap)
{
    if (!out || out_cap == 0)
        return 0;
    if (s_host_test_epoch == 0)
    {
        out[0] = '\0';
        return 0;
    }
    struct tm tmv; // reentrant: gmtime_r, never the shared static buffer (worker-safe)
    if (!gmtime_r(&s_host_test_epoch, &tmv))
    {
        out[0] = '\0';
        return 0;
    }
    return strftime(out, out_cap, "%a, %d %b %Y %H:%M:%S GMT", &tmv); // RFC 7231 IMF-fixdate
}

#endif // DETWS_ENABLE_NTP && ARDUINO
