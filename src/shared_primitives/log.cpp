// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file log.cpp
 * @brief The emitted half of the DWS_LOG* macros (see log.h).
 *
 * Nothing here is compiled when DWS_LOG_LEVEL is NONE - not even the sink pointer - so a build that
 * logs nothing links no logging code and spends no BSS on it.
 */

#include "shared_primitives/log.h"

#if DWS_LOG_LEVEL < DWS_LOG_LEVEL_NONE

#include <stdarg.h>
#include <stdio.h>

#if DWS_ENABLE_LOGBUF
#include "services/logbuf/logbuf.h"
#endif

/** @brief Owned state: just the sink the formatted line is handed to. */
struct LogCtx
{
    dws_log_sink_fn sink;
};
static LogCtx s_log = {nullptr};

void dws_log_set_sink(dws_log_sink_fn cb)
{
    s_log.sink = cb;
}

void dws_log_printf(uint8_t level, const char *fmt, ...)
{
    if (!fmt)
        return;

    // One line's worth of stack, matching what the ring can store - a message longer than a stored
    // line would be truncated there anyway, so it is truncated once, here.
    char line[DWS_LOG_LINE_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);

#if DWS_ENABLE_LOGBUF
    dws_log(level, line);
#endif
    if (s_log.sink)
        s_log.sink(level, line);
}

#endif // DWS_LOG_LEVEL < DWS_LOG_LEVEL_NONE
