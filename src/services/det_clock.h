// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_clock.h
 * @brief Pluggable monotonic clock for all library timing.
 *
 * The library's internal timing runs at **1000 Hz** - one tick is one millisecond,
 * the cadence the test suite asserts and every timeout / poll is expressed in.
 * `detws_millis()` is that single time source; by default it is the platform
 * `millis()`.
 *
 * To drive the library from your own clock (a hardware timer, an external RTC, a
 * simulation clock), call:
 *
 *     detws_set_clock(my_clock_fn, my_ticks_per_second);
 *
 * Your clock reports a free-running tick count at `ticks_per_second`. The library
 * **divides it down to its internal 1000 Hz**, so timeouts and polling keep the
 * exact 1 ms granularity the tests verify regardless of how fast your clock runs.
 * Pass a rate >= 1000, ideally a multiple of 1000 for exact division (e.g. a
 * 1 MHz timer -> ticks_per_second = 1000000, divided by 1000). Pass `nullptr` to
 * revert to the platform default. One source covers everything - swap it once and
 * every subsystem follows.
 *
 * The worker poll cadence is fixed at 1000 Hz (the tested default); a build can
 * trade latency for idle power with DETWS_WORKER_POLL_TICKS - see DetWebServerConfig.h.
 *
 * Header-only (the override state lives in inline-function-local statics, a single
 * instance across the whole program), so there is nothing extra to compile or link.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_CLOCK_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_CLOCK_H

#include <Arduino.h> // platform millis()
#include <stdint.h>

/** @brief User clock: returns a free-running monotonic tick count. */
typedef uint32_t (*detws_clock_fn)(void);

// Override state. Inline functions with local statics resolve to a single shared
// instance across all translation units (ODR), so a set from anywhere is seen
// everywhere - no .cpp, no globals to define.
inline detws_clock_fn &_detws_clock_fn_ref()
{
    static detws_clock_fn fn = nullptr;
    return fn;
}
inline uint32_t &_detws_clock_div_ref()
{
    static uint32_t div = 1;
    return div;
}

/**
 * @brief Install a custom clock running at @p ticks_per_second; the library divides
 *        it down to its internal 1000 Hz. Pass (nullptr, 0) to revert to the
 *        platform default.
 */
inline void detws_set_clock(detws_clock_fn fn, uint32_t ticks_per_second)
{
    _detws_clock_fn_ref() = fn;
    _detws_clock_div_ref() = (ticks_per_second >= 1000) ? (ticks_per_second / 1000) : 1;
}

/** @brief The library's monotonic time at 1000 Hz (milliseconds). */
inline uint32_t detws_millis(void)
{
    detws_clock_fn fn = _detws_clock_fn_ref();
    if (fn)
        return fn() / _detws_clock_div_ref();
    return millis();
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_CLOCK_H
