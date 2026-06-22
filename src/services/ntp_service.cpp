// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntp_service.cpp
 * @brief SNTP wall-clock time sync implementation (DETWS_ENABLE_NTP).
 */

#include "ntp_service.h"

// A successful sync moves the clock well past this sentinel (2021-01-01 UTC);
// a cold-booted RTC sits near the Unix epoch.
static const time_t DETWS_NTP_PLAUSIBLE_EPOCH = 1609459200;

#if DETWS_ENABLE_NTP && defined(ARDUINO)

#include <Arduino.h>

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

bool detws_ntp_begin(const char *tz, const char *server1, const char *server2)
{
    (void)tz;
    (void)server1;
    (void)server2;
    return false;
}
bool detws_ntp_synced()
{
    return false;
}
time_t detws_ntp_epoch()
{
    return 0;
}
size_t detws_ntp_http_date(char *out, size_t out_cap)
{
    if (out && out_cap)
        out[0] = '\0';
    return 0;
}

#endif // DETWS_ENABLE_NTP && ARDUINO
