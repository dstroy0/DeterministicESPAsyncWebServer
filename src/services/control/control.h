// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file control.h
 * @brief Closed-loop control law (DETWS_ENABLE_CONTROL) - a zero-heap PID controller plus a
 *        handful of inline control-law primitives, for driving an actuator toward a setpoint.
 *
 * The PID is the textbook parallel form with the corrections that matter on real hardware:
 *   - derivative-on-measurement (the derivative acts on the measurement, not the error, so a
 *     step change in the setpoint does not produce a "derivative kick"),
 *   - an optional single-pole low-pass on the derivative (measurement noise otherwise dominates
 *     the D term),
 *   - output clamping to the actuator's range, and
 *   - anti-windup by back-calculation (the integrator is pulled back by exactly the amount the
 *     output was clamped, so it never winds up past what the actuator can deliver), plus a hard
 *     integral clamp as a secondary bound,
 *   - a feed-forward term (kff * setpoint) for the part of the command known in advance.
 *
 * Pure float arithmetic - no heap, no <math.h>. Pair it with a plant it can command (a CiA 402
 * drive via `services/cia402`, a dshot ESC, a heater PWM) and a sensor it can read back.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CONTROL_H
#define DETERMINISTICESPASYNCWEBSERVER_CONTROL_H

#include "ServerConfig.h"

#if DETWS_ENABLE_CONTROL

#include <stddef.h>
#include <stdint.h>

// Hardware acceleration: the math is single-precision float end to end so it runs on the ESP32 /
// ESP32-S3 FPU (single-cycle add/mul + `madd.s`/`msub.s` fused multiply-add), never the soft-float
// double path. The update is written as an FMA chain so `-ffp-contract=fast` (on at -O2) folds it
// into madd.s. Define DETWS_CONTROL_IRAM=1 to place the hot pid_update() in IRAM so a real-time
// loop never stalls on a flash-cache miss. On an FPU-less target (e.g. ESP32-C3) it still works,
// just via soft-float.
#if defined(DETWS_CONTROL_IRAM) && DETWS_CONTROL_IRAM && (defined(ARDUINO) || defined(ESP_PLATFORM))
#include <esp_attr.h>
#define DETWS_CONTROL_HOT IRAM_ATTR
#else
#define DETWS_CONTROL_HOT
#endif

#define CONTROL_UNBOUNDED 1e30f ///< sentinel for "no clamp" (well outside any real actuator range)

// PID-run log for offline tuning with tools/pid_tune.py, in two interchangeable formats:
//
//  1. CSV (human / serial friendly) - one row per control step, these columns:
#define CONTROL_LOG_HEADER "t_s,setpoint,measurement,output,dt_s"
//
//  2. Dense binary (little-endian) for high-rate loops - self-describing (the header carries the
//     gains, limits, and sample period the run used, so the tuner needs no flags) and 16 B/sample:
//       header (PID_LOG_HEADER_LEN): "DPID" | ver u8=1 | flags u8 | reserved u16 |
//         dt_s f32 | kp f32 | ki f32 | kd f32 | kff f32 | out_min f32 | out_max f32
//       record (PID_LOG_RECORD_LEN): setpoint f32 | measurement f32 | output f32 |
//         status u32 (bit 0 = output was saturated this step, so the tuner can drop it from the
//         plant identification fit).
#define PID_LOG_MAGIC "DPID"
#define PID_LOG_VERSION 1
#define PID_LOG_HEADER_LEN 36
#define PID_LOG_RECORD_LEN 16
#define PID_LOG_STATUS_SATURATED 0x1u

/**
 * @brief A single-loop PID controller. Zero its bytes or call pid_init() before first use; the
 *        runtime-state fields below the gains are owned by pid_update() / pid_reset().
 */
struct Pid
{
    // gains
    float kp;  ///< proportional gain
    float ki;  ///< integral gain (per second)
    float kd;  ///< derivative gain (seconds)
    float kff; ///< feed-forward gain applied to the setpoint
    // limits
    float out_min;   ///< output lower clamp
    float out_max;   ///< output upper clamp
    float integ_min; ///< integral accumulator lower clamp
    float integ_max; ///< integral accumulator upper clamp
    float d_alpha;   ///< derivative low-pass smoothing in [0,1); 0 = raw (unfiltered) derivative
    // runtime state (owned by pid_update / pid_reset)
    float integ;     ///< integral accumulator
    float prev_meas; ///< previous measurement (for derivative-on-measurement)
    float d_filt;    ///< filtered derivative
    bool primed;     ///< false until the first update supplies prev_meas (no derivative on step 1)
};

/// Initialize with the three gains; feed-forward 0, no derivative filter, output/integral
/// unbounded (CONTROL_UNBOUNDED). Clears the runtime state.
void pid_init(Pid *p, float kp, float ki, float kd);

/// Clamp the controller output to [lo, hi] (the actuator's range).
void pid_set_output_limits(Pid *p, float lo, float hi);

/// Hard clamp the integral accumulator to [lo, hi] (a secondary anti-windup bound).
void pid_set_integral_limits(Pid *p, float lo, float hi);

/// Set the derivative low-pass smoothing factor in [0,1); 0 disables the filter.
void pid_set_derivative_filter(Pid *p, float alpha);

/// Set the feed-forward gain (the command output includes kff * setpoint).
void pid_set_feedforward(Pid *p, float kff);

/// Clear the integrator, derivative memory, and prime flag (e.g. when re-enabling the loop).
void pid_reset(Pid *p);

/// Advance the loop one step: returns the (clamped) control output for @p setpoint given the
/// measured process value @p measurement over the elapsed time @p dt seconds. dt <= 0 returns 0.
/// FPU-accelerated (single-precision, FMA); place in IRAM with DETWS_CONTROL_IRAM=1.
DETWS_CONTROL_HOT float pid_update(Pid *p, float setpoint, float measurement, float dt);

/// Batched multi-axis update: run @p n loops from contiguous arrays in one tight, FPU-bound,
/// SIMD-friendly pass (motion masters run several drives off one control tick). Each
/// out[i] = pid_update(&p[i], setpoint[i], measurement[i], dt).
void pid_update_n(Pid *p, const float *setpoint, const float *measurement, float dt, float *out, uint8_t n);

/// Write the self-describing 36-octet dense-binary log header from @p p's gains + limits and the
/// sample period @p dt (see the PID_LOG_* format above). Returns PID_LOG_HEADER_LEN, or 0 if cap
/// is too small. Emit once, then a pid_log_record() per control step.
size_t pid_log_header(uint8_t *buf, size_t cap, const Pid *p, float dt);

/// Write one 16-octet dense-binary log record. Returns PID_LOG_RECORD_LEN, or 0 if cap too small.
size_t pid_log_record(uint8_t *buf, size_t cap, float setpoint, float measurement, float output, bool saturated);

// --- inline control-law primitives (dedup of the arithmetic every loop reaches for) ---

/// Clamp @p v to [lo, hi].
static inline float control_clamp(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

/// Deadband: return 0 within +/- @p band, else @p v shifted toward 0 by @p band (continuous).
static inline float control_deadband(float v, float band)
{
    if (v > band)
        return v - band;
    if (v < -band)
        return v + band;
    return 0.0f;
}

/// Slew-rate limit: move @p current toward @p target by at most @p max_step this call.
static inline float control_slew(float target, float current, float max_step)
{
    float d = target - current;
    if (d > max_step)
        return current + max_step;
    if (d < -max_step)
        return current - max_step;
    return target;
}

/// One step of a single-pole low-pass: blend @p sample into @p prev by @p alpha in [0,1].
static inline float control_lpf(float prev, float sample, float alpha)
{
    return prev + alpha * (sample - prev);
}

#endif // DETWS_ENABLE_CONTROL

#endif // DETERMINISTICESPASYNCWEBSERVER_CONTROL_H
