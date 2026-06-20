#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PROGMEM
#define PROGMEM
#endif

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

// Use a function-local static so the variable is shared across all TUs
// (inline functions with static locals are merged by the linker per C++11 §3.2).
// A plain `static` variable in a header would give each TU its own copy,
// causing set_millis() in test files to not affect millis() in transport.cpp.
inline uint32_t &_millis_ref()
{
    static uint32_t v = 0;
    return v;
}
inline uint32_t millis()
{
    return _millis_ref();
}
inline void set_millis(uint32_t v)
{
    _millis_ref() = v;
}
