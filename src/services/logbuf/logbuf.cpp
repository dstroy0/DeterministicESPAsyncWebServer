// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file logbuf.cpp
 * @brief Fixed-RAM rotating log ring + severity trap - implementation (pure).
 */

#include "services/logbuf/logbuf.h"

#if DETWS_ENABLE_LOGBUF
namespace
{
char s_lines[DETWS_LOG_LINES][DETWS_LOG_LINE_LEN]; // ring storage (BSS)
uint8_t s_level[DETWS_LOG_LINES];                  // per-line severity
uint16_t s_head = 0;                               // index of the oldest line
uint16_t s_count = 0;                              // lines currently held
uint8_t s_trap_threshold = 0xFF;                   // 0xFF = trap disabled
detws_log_trap_fn s_trap = nullptr;

char level_letter(uint8_t level)
{
    switch (level)
    {
    case DETWS_LOG_ERROR:
        return 'E';
    case DETWS_LOG_WARN:
        return 'W';
    case DETWS_LOG_INFO:
        return 'I';
    default:
        return 'D';
    }
}
} // namespace

void detws_logbuf_reset(void)
{
    s_head = 0;
    s_count = 0;
}

void detws_log(uint8_t level, const char *msg)
{
    uint16_t slot;
    if (s_count < DETWS_LOG_LINES)
    {
        slot = (uint16_t)((s_head + s_count) % DETWS_LOG_LINES);
        s_count++;
    }
    else // full: overwrite the oldest and advance head
    {
        slot = s_head;
        s_head = (uint16_t)((s_head + 1) % DETWS_LOG_LINES);
    }
    snprintf(s_lines[slot], DETWS_LOG_LINE_LEN, "%c %s", level_letter(level), msg ? msg : "");
    s_level[slot] = level;

    if (s_trap && level >= s_trap_threshold)
        s_trap(level, s_lines[slot]);
}

uint16_t detws_log_count(void)
{
    return s_count;
}

const char *detws_log_at(uint16_t i)
{
    if (i >= s_count)
        return nullptr;
    return s_lines[(s_head + i) % DETWS_LOG_LINES];
}

int detws_log_dump(char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    out[0] = '\0';
    size_t pos = 0;
    for (uint16_t i = 0; i < s_count; i++)
    {
        const char *line = s_lines[(s_head + i) % DETWS_LOG_LINES];
        size_t n = strlen(line);
        size_t need = n + (i + 1 < s_count ? 1 : 0); // +1 for the '\n' separator
        if (pos + need >= cap)                       // keep room for the null terminator
        {
            out[0] = '\0';
            return 0;
        }
        memcpy(out + pos, line, n);
        pos += n;
        if (i + 1 < s_count)
            out[pos++] = '\n';
    }
    out[pos] = '\0';
    return (int)pos;
}

void detws_log_set_trap(uint8_t threshold, detws_log_trap_fn cb)
{
    s_trap_threshold = threshold;
    s_trap = cb;
}

#endif // DETWS_ENABLE_LOGBUF
