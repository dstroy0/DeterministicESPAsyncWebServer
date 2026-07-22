// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file time_compat.h
 * @brief Reentrant UTC broken-down time, portable across the host and target toolchains.
 *
 * The library needs the *reentrant* conversion (never the shared static `tm` that `gmtime()`
 * returns) because responses are formatted from worker threads. ESP32 newlib and glibc both spell
 * that `gmtime_r`; the Windows CRT does not have it at all and spells its own reentrant form
 * `gmtime_s`, with the arguments the other way round. Calling `gmtime_r` directly therefore made
 * every env that formats an HTTP date unbuildable on a Windows host - twelve native test envs
 * (native_app, native_keepalive, native_range, native_upload, native_webdav_handler,
 * native_ssh_sftp, ...) failed to compile, so they could not be run or coverage-measured there at
 * all. This is the one seam that hides that difference.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_TIME_COMPAT_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_TIME_COMPAT_H

#include <time.h>

/**
 * @brief Convert @p epoch to broken-down UTC in caller storage (reentrant).
 * @param epoch  Seconds since the Unix epoch.
 * @param out    Destination `struct tm` (must be non-null).
 * @return @p out on success, or NULL if @p epoch cannot be represented.
 */
inline struct tm *dws_gmtime_r(const time_t *epoch, struct tm *out)
{
#if defined(_WIN32)
    // MS runtime: gmtime_s(tm, time) - arguments reversed vs POSIX, returns errno_t (0 == ok).
    return gmtime_s(out, epoch) == 0 ? out : NULL;
#else
    return gmtime_r(epoch, out);
#endif
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_TIME_COMPAT_H
