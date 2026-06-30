// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file telemetry.cpp
 * @brief Telemetry math helpers implementation (DETWS_ENABLE_TELEMETRY).
 */

#include "telemetry.h"

#if DETWS_ENABLE_TELEMETRY

#include "shared_primitives/shim.h"

void detws_window_init(DetwsWindow *w, float *buf, uint16_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->count = 0;
    w->head = 0;
    w->sum = 0.0;
    w->sum_sq = 0.0;
}

void detws_window_push(DetwsWindow *w, float sample)
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

uint16_t detws_window_count(const DetwsWindow *w)
{
    return w->count;
}

float detws_window_mean(const DetwsWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    return (float)(w->sum / (double)w->count);
}

float detws_window_variance(const DetwsWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    double mean = w->sum / (double)w->count;
    double var = w->sum_sq / (double)w->count - mean * mean;
    return var < 0.0 ? 0.0f : (float)var; // clamp tiny negatives from rounding
}

float detws_window_stddev(const DetwsWindow *w)
{
    return sqrtf(detws_window_variance(w));
}

float detws_window_min(const DetwsWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    float m = w->buf[0];
    for (uint16_t i = 1; i < w->count; i++)
        if (w->buf[i] < m)
            m = w->buf[i];
    return m;
}

float detws_window_max(const DetwsWindow *w)
{
    if (w->count == 0)
        return 0.0f;
    float m = w->buf[0];
    for (uint16_t i = 1; i < w->count; i++)
        if (w->buf[i] > m)
            m = w->buf[i];
    return m;
}

void detws_rate_init(DetwsRate *r)
{
    r->last_value = 0.0f;
    r->last_ms = 0;
    r->primed = false;
}

float detws_rate_update(DetwsRate *r, float value, uint32_t now_ms)
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

void detws_totalizer_init(DetwsTotalizer *t)
{
    t->total = 0.0;
    t->last_rate = 0.0f;
    t->last_ms = 0;
    t->primed = false;
}

double detws_totalizer_add(DetwsTotalizer *t, float rate, uint32_t now_ms)
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

double detws_totalizer_total(const DetwsTotalizer *t)
{
    return t->total;
}

void detws_totalizer_reset(DetwsTotalizer *t)
{
    t->total = 0.0;
    t->last_rate = 0.0f;
    t->last_ms = 0;
    t->primed = false;
}

#endif // DETWS_ENABLE_TELEMETRY
