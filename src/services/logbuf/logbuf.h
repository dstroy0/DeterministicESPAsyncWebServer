// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file logbuf.h
 * @brief Fixed-RAM rotating log buffer with severity traps (DWS_ENABLE_LOGBUF).
 *
 * Keeps the last DWS_LOG_LINES log lines in a fixed ring (the oldest is pruned
 * on overflow - no heap, bounded latency), each line stored as `<L> message`
 * where L is the severity letter. Dump the ring oldest-first for a `/logs`
 * endpoint, and register a trap callback that fires when a line is logged at or
 * above a severity threshold (forward criticals as an SNMP trap / webhook). Pure
 * and fully host-tested - no ESP32 dependency.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_LOGBUF_H
#define DETERMINISTICESPASYNCWEBSERVER_LOGBUF_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_LOGBUF

/** @brief Severity levels (ordered low -> high). Compared (level >= threshold) and passed through the
 *  uint8_t trap-callback ABI, so integer constants in a namespacing struct - cast-free. */
struct DWSLogLevel
{
    static constexpr uint8_t DWS_LOG_DEBUG = 0;
    static constexpr uint8_t DWS_LOG_INFO = 1;
    static constexpr uint8_t DWS_LOG_WARN = 2;
    static constexpr uint8_t DWS_LOG_ERROR = 3;
};

/** @brief Trap callback: fired for a line logged at level >= the threshold. */
typedef void (*dws_log_trap_fn)(uint8_t level, const char *line);

/** @brief Empty the ring (and clear the line count). */
void dws_logbuf_reset(void);

/** @brief Append @p msg at @p level (stored as `<L> msg`, truncated to fit). */
void dws_log(uint8_t level, const char *msg);

/** @brief Number of lines currently held (0 .. DWS_LOG_LINES). */
uint16_t dws_log_count(void);

/** @brief Line @p i (0 = oldest .. count-1 = newest), or nullptr if out of range. */
const char *dws_log_at(uint16_t i);

/**
 * @brief Dump all held lines, oldest-first, newline-separated, into @p out.
 * @return characters written, or 0 if @p cap is too small (fail-closed).
 */
int dws_log_dump(char *out, size_t cap);

/** @brief Install a trap callback that fires when a line is logged at level >= @p threshold. */
void dws_log_set_trap(uint8_t threshold, dws_log_trap_fn cb);

#endif // DWS_ENABLE_LOGBUF
#endif // DETERMINISTICESPASYNCWEBSERVER_LOGBUF_H
