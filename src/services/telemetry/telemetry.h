// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telemetry.h
 * @brief Zero-heap telemetry math helpers (DETWS_ENABLE_TELEMETRY).
 *
 * Pure-computation building blocks for turning a stream of sensor samples into
 * dashboard figures, alert triggers, and odometer-style totals - no heap, no
 * Arduino dependency, all state in caller-supplied storage or small POD structs,
 * so the whole cluster unit-tests on the host:
 *
 *   - DetwsWindow    moving-window statistics (mean / variance / stddev / min /
 *                    max) over a caller-provided ring buffer, O(1) mean/variance
 *                    via running sums.
 *   - DetwsRate      derivative / rate-of-change between successive samples
 *                    (units per second), for slope alerts.
 *   - DetwsTotalizer trapezoidal integration of a rate over time (a running
 *                    total / odometer).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H
#define DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H

#include "DetWebServerConfig.h"
#include "shared_primitives/shim.h"

#if DETWS_ENABLE_TELEMETRY

// ---------------------------------------------------------------------------
// Moving-window statistics
// ---------------------------------------------------------------------------

/**
 * @brief Moving-window stats accumulator over a caller-provided ring buffer.
 *
 * The caller owns the `float` storage (no heap). Mean and variance are kept O(1)
 * via running sums; min/max are an O(window) scan.
 */
struct DetwsWindow
{
    float *buf;     ///< caller-provided sample storage (>= cap floats).
    uint16_t cap;   ///< window capacity (samples).
    uint16_t count; ///< samples currently held (<= cap).
    uint16_t head;  ///< next write index (oldest sample when full).
    double sum;     ///< running sum of held samples.
    double sum_sq;  ///< running sum of squares of held samples.
};

/** @brief Bind @p w to @p buf (capacity @p cap samples) and reset it to empty. */
void detws_window_init(DetwsWindow *w, float *buf, uint16_t cap);

/** @brief Add @p sample, evicting the oldest once the window is full. */
void detws_window_push(DetwsWindow *w, float sample);

/** @brief Number of samples currently in the window. */
uint16_t detws_window_count(const DetwsWindow *w);

/** @brief Arithmetic mean of the window (0 when empty). */
float detws_window_mean(const DetwsWindow *w);

/** @brief Population variance of the window (0 when empty). */
float detws_window_variance(const DetwsWindow *w);

/** @brief Population standard deviation of the window (0 when empty). */
float detws_window_stddev(const DetwsWindow *w);

/** @brief Smallest sample in the window (0 when empty). */
float detws_window_min(const DetwsWindow *w);

/** @brief Largest sample in the window (0 when empty). */
float detws_window_max(const DetwsWindow *w);

// ---------------------------------------------------------------------------
// Rate of change (first derivative)
// ---------------------------------------------------------------------------

/** @brief Derivative / rate-of-change tracker between successive samples. */
struct DetwsRate
{
    float last_value; ///< previous sample value.
    uint32_t last_ms; ///< millis() of the previous sample.
    bool primed;      ///< false until the first sample is seen.
};

/** @brief Reset @p r so the next sample is treated as the first. */
void detws_rate_init(DetwsRate *r);

/**
 * @brief Feed a sample; returns the rate of change in units per second since the
 *        previous sample.
 *
 * Returns 0 on the first sample (nothing to differentiate) and when the elapsed
 * time is 0. The elapsed-time math is unsigned, so it survives a millis()
 * rollover.
 */
float detws_rate_update(DetwsRate *r, float value, uint32_t now_ms);

// ---------------------------------------------------------------------------
// Totalizer (run-time integral / odometer)
// ---------------------------------------------------------------------------

/** @brief Running total from trapezoidal integration of a rate over time. */
struct DetwsTotalizer
{
    double total;     ///< accumulated total (in rate-units * seconds).
    float last_rate;  ///< previous rate sample.
    uint32_t last_ms; ///< millis() of the previous rate sample.
    bool primed;      ///< false until the first rate sample is seen.
};

/** @brief Reset @p t to a zero total with no prior sample. */
void detws_totalizer_init(DetwsTotalizer *t);

/**
 * @brief Integrate @p rate (units per second) over the time since the last call
 *        (trapezoidal rule) and return the running total.
 *
 * The first call only seeds the baseline (total stays 0). Unsigned elapsed-time
 * math survives a millis() rollover.
 */
double detws_totalizer_add(DetwsTotalizer *t, float rate, uint32_t now_ms);

/** @brief Current running total. */
double detws_totalizer_total(const DetwsTotalizer *t);

/** @brief Reset the running total to 0 and drop the prior sample. */
void detws_totalizer_reset(DetwsTotalizer *t);

#endif // DETWS_ENABLE_TELEMETRY
#endif // DETERMINISTICESPASYNCWEBSERVER_TELEMETRY_H
