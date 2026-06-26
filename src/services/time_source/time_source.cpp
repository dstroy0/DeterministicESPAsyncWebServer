// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file time_source.cpp
 * @brief Multi-source time fallback matrix - implementation.
 *
 * A fixed BSS table of sources queried in ascending priority. See time_source.h.
 */

#include "time_source.h"

#if DETWS_ENABLE_TIME_SOURCE

namespace
{
struct Src
{
    const char *name;
    TimeSourceFn fn;
    uint8_t priority;
    bool used;
};

Src s_sources[DETWS_TIME_SOURCE_MAX];
const char *s_active;
} // namespace

bool detws_time_source_add(const char *name, uint8_t priority, TimeSourceFn fn)
{
    if (!fn)
        return false;
    for (int i = 0; i < DETWS_TIME_SOURCE_MAX; i++)
    {
        if (!s_sources[i].used)
        {
            s_sources[i].name = name;
            s_sources[i].fn = fn;
            s_sources[i].priority = priority;
            s_sources[i].used = true;
            return true;
        }
    }
    return false; // table full
}

uint32_t detws_time_now(void)
{
    s_active = nullptr;

    // Query sources in ascending priority (lowest value first); stop at the first
    // that returns a nonzero epoch. A selection scan avoids sorting and, more
    // importantly, does not invoke lower-priority callbacks once a higher-priority
    // source has answered (reading an RTC / GPS can be costly).
    uint32_t queried = 0; // bitmask of sources already tried
    for (int pass = 0; pass < DETWS_TIME_SOURCE_MAX; pass++)
    {
        int sel = -1;
        for (int i = 0; i < DETWS_TIME_SOURCE_MAX; i++)
        {
            if (!s_sources[i].used || (queried & (1u << i)))
                continue;
            if (sel < 0 || s_sources[i].priority < s_sources[sel].priority)
                sel = i;
        }
        if (sel < 0)
            break; // no more sources

        queried |= (1u << sel);
        uint32_t epoch = s_sources[sel].fn();
        if (epoch != 0)
        {
            s_active = s_sources[sel].name;
            return epoch;
        }
    }
    return 0;
}

const char *detws_time_source_active(void)
{
    return s_active;
}

void detws_time_source_reset(void)
{
    for (int i = 0; i < DETWS_TIME_SOURCE_MAX; i++)
        s_sources[i] = Src{};
    s_active = nullptr;
}

#else // DETWS_ENABLE_TIME_SOURCE == 0 -> no-op stubs

bool detws_time_source_add(const char *, uint8_t, TimeSourceFn)
{
    return false;
}
uint32_t detws_time_now(void)
{
    return 0;
}
const char *detws_time_source_active(void)
{
    return nullptr;
}
void detws_time_source_reset(void)
{
}

#endif // DETWS_ENABLE_TIME_SOURCE
