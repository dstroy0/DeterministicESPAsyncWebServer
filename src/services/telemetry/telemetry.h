// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telemetry.h
 * @brief Zero-heap telemetry math helpers (DWS_ENABLE_TELEMETRY).
 *
 * Pure-computation building blocks for turning a stream of sensor samples into
 * dashboard figures, alert triggers, and odometer-style totals - no heap, no
 * Arduino dependency, all state in caller-supplied storage or small POD structs,
 * so the whole cluster unit-tests on the host:
 *
 *   - DWSWindow    moving-window statistics (mean / variance / stddev / min /
 *                    max) over a caller-provided ring buffer, O(1) mean/variance
 *                    via running sums.
 *   - DWSRate      derivative / rate-of-change between successive samples
 *                    (units per second), for slope alerts.
 *   - DWSTotalizer trapezoidal integration of a rate over time (a running
 *                    total / odometer).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H
#define DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H

#include "ServerConfig.h"
#include <stdint.h>

#if DWS_ENABLE_TELEMETRY

// ---------------------------------------------------------------------------
// Moving-window statistics
// ---------------------------------------------------------------------------

/**
 * @brief Moving-window stats accumulator over a caller-provided ring buffer.
 *
 * The caller owns the `float` storage (no heap). Mean and variance are kept O(1)
 * via running sums; min/max are an O(window) scan.
 */
struct DWSWindow
{
    float *buf;     ///< caller-provided sample storage (>= cap floats).
    uint16_t cap;   ///< window capacity (samples).
    uint16_t count; ///< samples currently held (<= cap).
    uint16_t head;  ///< next write index (oldest sample when full).
    double sum;     ///< running sum of held samples.
    double sum_sq;  ///< running sum of squares of held samples.
};

/** @brief Bind @p w to @p buf (capacity @p cap samples) and reset it to empty. */
void dws_window_init(DWSWindow *w, float *buf, uint16_t cap);

/** @brief Add @p sample, evicting the oldest once the window is full. */
void dws_window_push(DWSWindow *w, float sample);

/** @brief Number of samples currently in the window. */
uint16_t dws_window_count(const DWSWindow *w);

/** @brief Arithmetic mean of the window (0 when empty). */
float dws_window_mean(const DWSWindow *w);

/** @brief Population variance of the window (0 when empty). */
float dws_window_variance(const DWSWindow *w);

/** @brief Population standard deviation of the window (0 when empty). */
float dws_window_stddev(const DWSWindow *w);

/** @brief Smallest sample in the window (0 when empty). */
float dws_window_min(const DWSWindow *w);

/** @brief Largest sample in the window (0 when empty). */
float dws_window_max(const DWSWindow *w);

// ---------------------------------------------------------------------------
// Rate of change (first derivative)
// ---------------------------------------------------------------------------

/** @brief Derivative / rate-of-change tracker between successive samples. */
struct DWSRate
{
    float last_value; ///< previous sample value.
    uint32_t last_ms; ///< millis() of the previous sample.
    bool primed;      ///< false until the first sample is seen.
};

/** @brief Reset @p r so the next sample is treated as the first. */
void dws_rate_init(DWSRate *r);

/**
 * @brief Feed a sample; returns the rate of change in units per second since the
 *        previous sample.
 *
 * Returns 0 on the first sample (nothing to differentiate) and when the elapsed
 * time is 0. The elapsed-time math is unsigned, so it survives a millis()
 * rollover.
 */
float dws_rate_update(DWSRate *r, float value, uint32_t now_ms);

// ---------------------------------------------------------------------------
// Totalizer (run-time integral / odometer)
// ---------------------------------------------------------------------------

/** @brief Running total from trapezoidal integration of a rate over time. */
struct DWSTotalizer
{
    double total;     ///< accumulated total (in rate-units * seconds).
    float last_rate;  ///< previous rate sample.
    uint32_t last_ms; ///< millis() of the previous rate sample.
    bool primed;      ///< false until the first rate sample is seen.
};

/** @brief Reset @p t to a zero total with no prior sample. */
void dws_totalizer_init(DWSTotalizer *t);

/**
 * @brief Integrate @p rate (units per second) over the time since the last call
 *        (trapezoidal rule) and return the running total.
 *
 * The first call only seeds the baseline (total stays 0). Unsigned elapsed-time
 * math survives a millis() rollover.
 */
double dws_totalizer_add(DWSTotalizer *t, float rate, uint32_t now_ms);

/** @brief Current running total. */
double dws_totalizer_total(const DWSTotalizer *t);

/** @brief Reset the running total to 0 and drop the prior sample. */
void dws_totalizer_reset(DWSTotalizer *t);

#endif // DWS_ENABLE_TELEMETRY
#endif // DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H
