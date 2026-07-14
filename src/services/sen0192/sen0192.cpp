// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sen0192.cpp
 * @brief SEN0192 microwave motion sensor - debounced presence tracker + GPIO binding. See sen0192.h.
 */

#include "services/sen0192/sen0192.h"

#if DETWS_ENABLE_SEN0192

// ---------------------------------------------------------------------------
// Pure presence state machine (host-tested).
// ---------------------------------------------------------------------------

void sen0192_motion_init(Sen0192Motion *m, uint32_t hold_ms, bool active_high)
{
    m->present = false;
    m->seeded = false;
    m->active_high = active_high;
    m->hold_ms = hold_ms;
    m->last_active_ms = 0;
    m->motion_events = 0;
}

bool sen0192_motion_update(Sen0192Motion *m, bool level_high, uint32_t now_ms)
{
    bool active = (level_high == m->active_high);
    if (active)
    {
        m->last_active_ms = now_ms;
        m->seeded = true;
        if (!m->present)
        {
            m->present = true;
            m->motion_events++;
            return true; // clear -> present edge
        }
        return false;
    }
    sen0192_motion_tick(m, now_ms); // inactive sample: presence may age out
    return false;
}

bool sen0192_motion_tick(Sen0192Motion *m, uint32_t now_ms)
{
    if (m->present && m->seeded && (uint32_t)(now_ms - m->last_active_ms) > m->hold_ms)
        m->present = false;
    return m->present;
}

bool sen0192_motion_present(const Sen0192Motion *m)
{
    return m->present;
}

uint32_t sen0192_motion_events(const Sen0192Motion *m)
{
    return m->motion_events;
}

uint32_t sen0192_motion_active_age_ms(const Sen0192Motion *m, uint32_t now_ms)
{
    return m->seeded ? (uint32_t)(now_ms - m->last_active_ms) : 0;
}

// ---------------------------------------------------------------------------
// ESP32 GPIO binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "services/clock.h" // detws_millis()
#include <Arduino.h>

namespace
{
// The SEN0192 binding state, owned by one instance (internal linkage): the presence tracker and the pin.
struct Sen0192Ctx
{
    Sen0192Motion motion;
    int pin = -1;
};
Sen0192Ctx s_sen;
} // namespace

bool sen0192_begin(void)
{
    s_sen.pin = DETWS_SEN0192_PIN;
    pinMode(s_sen.pin, INPUT);
    sen0192_motion_init(&s_sen.motion, DETWS_SEN0192_HOLD_MS, DETWS_SEN0192_ACTIVE_HIGH != 0);
    return true;
}

bool sen0192_poll(void)
{
    if (s_sen.pin < 0)
        return false;
    bool level = digitalRead(s_sen.pin) != 0;
    return sen0192_motion_update(&s_sen.motion, level, detws_millis());
}

bool sen0192_present(void)
{
    sen0192_motion_tick(&s_sen.motion, detws_millis()); // age presence out even between poll()s
    return sen0192_motion_present(&s_sen.motion);
}

uint32_t sen0192_motion_count(void)
{
    return sen0192_motion_events(&s_sen.motion);
}

#else // host build: no GPIO. The presence state machine above is host-tested.

bool sen0192_begin(void)
{
    return false;
}
bool sen0192_poll(void)
{
    return false;
}
bool sen0192_present(void)
{
    return false;
}
uint32_t sen0192_motion_count(void)
{
    return 0;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_SEN0192
