// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dws_ntp_service.cpp
 * @brief SNTP wall-clock time sync implementation (DWS_ENABLE_NTP).
 */

#include "ntp_service.h"
#include "shared_primitives/http_date.h" // dws_http_date() - the shared IMF-fixdate formatter

#if DWS_ENABLE_NTP && defined(ARDUINO)

#include <Arduino.h>

// A successful sync moves the clock well past this sentinel (2021-01-01 UTC);
// a cold-booted RTC sits near the Unix epoch.
static const time_t DWS_NTP_PLAUSIBLE_EPOCH = 1609459200;

bool dws_ntp_begin(const char *tz, const char *server1, const char *server2)
{
    // configTzTime applies the POSIX TZ and starts the SNTP client (async).
    configTzTime(tz ? tz : "UTC0", server1, server2);
    return true;
}

bool dws_ntp_synced()
{
    return time(nullptr) > DWS_NTP_PLAUSIBLE_EPOCH;
}

time_t dws_ntp_epoch()
{
    time_t now = time(nullptr);
    return (now > DWS_NTP_PLAUSIBLE_EPOCH) ? now : 0;
}

size_t dws_ntp_http_date(char *out, size_t out_cap)
{
    return dws_http_date(dws_ntp_epoch(), out, out_cap);
}

#else

// Host build: no SNTP. A test seam lets a unit test inject a wall-clock epoch so
// the Date-header path (and any time-dependent code) is exercisable off-device.
#include <time.h>
// All host NTP test-seam state, owned by one instance (internal linkage): the injected
// wall-clock epoch, so it is one named owner, unreachable from any other translation unit.
namespace
{
struct NtpSvcCtx
{
    time_t host_test_epoch = 0;
};
NtpSvcCtx s_ntp_svc;
} // namespace
void dws_ntp_set_test_epoch(time_t epoch)
{
    s_ntp_svc.host_test_epoch = epoch;
}

bool dws_ntp_begin(const char *tz, const char *server1, const char *server2)
{
    (void)tz;
    (void)server1;
    (void)server2;
    return false;
}
bool dws_ntp_synced()
{
    return s_ntp_svc.host_test_epoch != 0;
}
time_t dws_ntp_epoch()
{
    return s_ntp_svc.host_test_epoch;
}
size_t dws_ntp_http_date(char *out, size_t out_cap)
{
    return dws_http_date(dws_ntp_epoch(), out, out_cap);
}

#endif // DWS_ENABLE_NTP && ARDUINO

// NTP as a registry time source (defined for both the device and host builds; dws_ntp_epoch is 0
// until synced / when no test epoch is injected). Register it with dws_time_source_add() so the
// aggregated dws_time_now() - and the HTTP Date header - can be fed by NTP alongside an RTC / GPS.
uint32_t dws_ntp_time_source(void)
{
    return (uint32_t)dws_ntp_epoch();
}
