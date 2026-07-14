// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file control.cpp
 * @brief PID control law (pure single-precision float; FPU-accelerated on ESP32 / ESP32-S3).
 */

#include "services/control/control.h"

#if DETWS_ENABLE_CONTROL

// clamp helper is control_clamp() from control.h (inline) - reused here, not re-declared.

void pid_init(Pid *p, float kp, float ki, float kd)
{
    if (!p)
        return;
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
    p->kff = 0.0f;
    p->out_min = -CONTROL_UNBOUNDED;
    p->out_max = CONTROL_UNBOUNDED;
    p->integ_min = -CONTROL_UNBOUNDED;
    p->integ_max = CONTROL_UNBOUNDED;
    p->d_alpha = 0.0f;
    pid_reset(p);
}

void pid_set_output_limits(Pid *p, float lo, float hi)
{
    if (p)
    {
        p->out_min = lo;
        p->out_max = hi;
    }
}

void pid_set_integral_limits(Pid *p, float lo, float hi)
{
    if (p)
    {
        p->integ_min = lo;
        p->integ_max = hi;
    }
}

void pid_set_derivative_filter(Pid *p, float alpha)
{
    if (p)
        p->d_alpha = alpha;
}

void pid_set_feedforward(Pid *p, float kff)
{
    if (p)
        p->kff = kff;
}

void pid_reset(Pid *p)
{
    if (!p)
        return;
    p->integ = 0.0f;
    p->prev_meas = 0.0f;
    p->d_filt = 0.0f;
    p->primed = false;
}

DETWS_CONTROL_HOT float pid_update(Pid *p, float setpoint, float measurement, float dt)
{
    if (!p || dt <= 0.0f)
        return 0.0f;

    float error = setpoint - measurement;

    // Derivative on measurement (no setpoint-change "kick"): d(error)/dt = -d(measurement)/dt when
    // the setpoint is held. Skip the first update (no prev_meas yet), optionally low-pass filter it.
    float deriv = 0.0f;
    if (p->primed)
    {
        deriv = -(measurement - p->prev_meas) / dt;
        p->d_filt = (p->d_alpha > 0.0f) ? p->d_filt + p->d_alpha * (deriv - p->d_filt) : deriv;
    }
    p->prev_meas = measurement;
    p->primed = true;

    // Tentative integration, hard-clamped to the accumulator bounds (a secondary safety limit).
    float integ_next = control_clamp(p->integ + p->ki * error * dt, p->integ_min, p->integ_max);

    // FMA chain -> madd.s on the FPU: kp*error + integ + kd*d_filt + kff*setpoint.
    float unclamped = p->kp * error + integ_next + p->kd * p->d_filt + p->kff * setpoint;
    float out = control_clamp(unclamped, p->out_min, p->out_max);

    // Anti-windup by conditional integration: commit the new integral unless the output is
    // saturated AND integrating further this direction would push it deeper into the rail - then
    // freeze the accumulator instead, so it never winds up past what the actuator can deliver.
    bool worsen_high = (unclamped > p->out_max) && (error > 0.0f);
    bool worsen_low = (unclamped < p->out_min) && (error < 0.0f);
    if (!worsen_high && !worsen_low)
        p->integ = integ_next;

    return out;
}

void pid_update_n(Pid *p, const float *setpoint, const float *measurement, float dt, float *out, uint8_t n)
{
    if (!p || !setpoint || !measurement || !out)
        return;
    for (uint8_t i = 0; i < n; i++)
        out[i] = pid_update(&p[i], setpoint[i], measurement[i], dt);
}

#endif // DETWS_ENABLE_CONTROL
