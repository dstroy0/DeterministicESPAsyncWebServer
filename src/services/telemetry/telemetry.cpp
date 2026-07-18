// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telemetry.cpp
 * @brief Telemetry math helpers implementation (DWS_ENABLE_TELEMETRY).
 */

#include "telemetry.h"

#if DWS_ENABLE_TELEMETRY

#include <math.h>

void dws_window_init(DWSWindow *w, float *buf, uint16_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->count = 0;
    w->head = 0;
    w->sum = 0.0;
    w->sum_sq = 0.0;
}

void dws_window_push(DWSWindow *w, float sample)
{
    if (!w->buf || w->cap == 0)
        return;
    if (w->count == w->cap)
    {
        // Full: evict the oldest sample (at head) from the running sums.
        float old = w->buf[w->head];
        w->sum -= (double)old;
        w->sum_sq -= (double)old * (double)old;
    }
    else
    {
        w->count++;
    }
    w->buf[w->head] = sample;
    w->sum += (double)sample;
    w->sum_sq += (double)sample * (double)sample;
    w->head = (uint16_t)((w->head + 1) % w->cap);
}

uint16_t dws_window_count(const DWSWindow *w)
{
    return w->count;
}

float dws_window_mean(const DWSWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    return (float)(w->sum / (double)w->count);
}

float dws_window_variance(const DWSWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    double mean = w->sum / (double)w->count;
    double var = w->sum_sq / (double)w->count - mean * mean;
    return var < 0.0 ? 0.0f : (float)var; // clamp tiny negatives from rounding
}

float dws_window_stddev(const DWSWindow *w)
{
    return sqrtf(dws_window_variance(w));
}

float dws_window_min(const DWSWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    float m = w->buf[0];
    for (uint16_t i = 1; i < w->count; i++)
        if (w->buf[i] < m)
            m = w->buf[i];
    return m;
}

float dws_window_max(const DWSWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    float m = w->buf[0];
    for (uint16_t i = 1; i < w->count; i++)
        if (w->buf[i] > m)
            m = w->buf[i];
    return m;
}

void dws_rate_init(DWSRate *r)
{
    r->last_value = 0.0f;
    r->last_ms = 0;
    r->primed = false;
}

float dws_rate_update(DWSRate *r, float value, uint32_t now_ms)
{
    if (!r->primed)
    {
        r->last_value = value;
        r->last_ms = now_ms;
        r->primed = true;
        return 0.0f;
    }
    uint32_t dt_ms = (uint32_t)(now_ms - r->last_ms); // wraps correctly
    float rate = 0.0f;
    if (dt_ms != 0)
        rate = (value - r->last_value) * 1000.0f / (float)dt_ms;
    r->last_value = value;
    r->last_ms = now_ms;
    return rate;
}

void dws_totalizer_init(DWSTotalizer *t)
{
    t->total = 0.0;
    t->last_rate = 0.0f;
    t->last_ms = 0;
    t->primed = false;
}

double dws_totalizer_add(DWSTotalizer *t, float rate, uint32_t now_ms)
{
    if (!t->primed)
    {
        t->last_rate = rate;
        t->last_ms = now_ms;
        t->primed = true;
        return t->total;
    }
    uint32_t dt_ms = (uint32_t)(now_ms - t->last_ms); // wraps correctly
    double dt_s = (double)dt_ms / 1000.0;
    // Trapezoidal: average of the two rate endpoints over the interval.
    t->total += ((double)t->last_rate + (double)rate) * 0.5 * dt_s;
    t->last_rate = rate;
    t->last_ms = now_ms;
    return t->total;
}

double dws_totalizer_total(const DWSTotalizer *t)
{
    return t->total;
}

void dws_totalizer_reset(DWSTotalizer *t)
{
    t->total = 0.0;
    t->last_rate = 0.0f;
    t->last_ms = 0;
    t->primed = false;
}

#endif // DWS_ENABLE_TELEMETRY
