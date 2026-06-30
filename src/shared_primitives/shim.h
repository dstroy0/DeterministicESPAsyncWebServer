// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file shim.h
 * @brief One place for the portable C standard-library (and Arduino) headers the library uses.
 *
 * The same handful of `<stdint.h>` / `<stddef.h>` / `<string.h>` (and friends) were included
 * file by file across the whole codebase. This shim pulls them in once, with a comment on each
 * naming the symbols this codebase actually uses from it, so the dependency surface is visible
 * in one place. Portable, host-safe source files include `shared_primitives/shim.h` instead of
 * re-listing the standard headers.
 *
 * Scope: only the portable C/C++ standard headers plus Arduino.h (guarded by `ARDUINO`, so a
 * host / native codec build never pulls it in). Layer-specific headers - lwIP, FreeRTOS,
 * ESP-IDF (`esp_*`), and mbedTLS - are deliberately NOT here: they belong to the transport /
 * TLS / network layers, the codec test environments do not have their include paths, and
 * routing them through a global shim would couple every pure codec to those stacks. Per
 * project policy this shim never includes `<stdlib.h>` (parse by hand; see det_numparse.h).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SHIM_H
#define DETERMINISTICESPASYNCWEBSERVER_SHIM_H

#include <stdint.h>  // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <stddef.h>  // size_t, ptrdiff_t, NULL
#include <string.h>  // memcpy, memmove, memset, memcmp, memchr, strlen, strnlen, strcmp, strncmp,
                     // strcasecmp, strncasecmp, strchr, strrchr, strstr, strncpy
#include <stdio.h>   // snprintf, vsnprintf, sscanf (format into / scan from fixed buffers)
#include <stdarg.h>  // va_list, va_start, va_end (the log / format varargs helpers)
#include <time.h>    // time_t, struct tm, gmtime_r, strftime (HTTP-date / clock formatting)
#include <math.h>    // floating-point math for the telemetry helpers (e.g. sqrtf for std-dev)
#include <assert.h>  // assert (host-side invariant checks; compiled out with NDEBUG)

#if defined(ARDUINO)
#include <Arduino.h> // millis, micros, delay, Serial, pinMode/digitalWrite, String (ESP32 only)
#endif

#endif // DETERMINISTICESPASYNCWEBSERVER_SHIM_H
