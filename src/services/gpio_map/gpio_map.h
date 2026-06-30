// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file gpio_map.h
 * @brief Browser GPIO pin-mapper / diagnostics (DETWS_ENABLE_GPIO_MAP).
 *
 * Exposes a compile-time table of GPIO pins (number, label, configured direction,
 * live level) as JSON so a browser diag panel can show the pin map and toggle
 * outputs. The live read (digitalRead) and write (pinMode / digitalWrite) use the
 * Arduino API on ESP32; the JSON serializer and the control-POST parser are pure
 * and host-tested. No allocation: the pin table is caller-owned and the JSON is
 * written into a caller buffer.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_GPIO_MAP_H
#define DETERMINISTICESPASYNCWEBSERVER_GPIO_MAP_H

#include "shared_primitives/shim.h"

#if DETWS_ENABLE_GPIO_MAP

class DetWebServer;

/** @brief Configured direction of a mapped pin (how the panel renders / drives it). */
enum DetwsGpioDir
{
    DETWS_GPIO_IN = 0,      ///< read-only input.
    DETWS_GPIO_IN_PULLUP,   ///< input with internal pull-up.
    DETWS_GPIO_IN_PULLDOWN, ///< input with internal pull-down.
    DETWS_GPIO_OUT,         ///< output (drivable from the panel).
};

/** @brief One mapped GPIO pin. */
struct DetwsGpioPin
{
    uint8_t pin;       ///< GPIO number.
    const char *label; ///< human label (null-terminated, caller-owned).
    uint8_t dir;       ///< DetwsGpioDir.
    uint8_t level;     ///< live level (0 / 1); filled by detws_gpio_read.
};

// ---------------------------------------------------------------------------
// Host-testable core
// ---------------------------------------------------------------------------

/** @brief Short name for a direction ("in", "in_pullup", "in_pulldown", "out"). */
const char *detws_gpio_dir_name(uint8_t dir);

/**
 * @brief Serialize a pin array as JSON `{"pins":[...]}` into @p out.
 * @return characters written, or 0 if @p cap is too small (fail-closed).
 */
int detws_gpio_json(const DetwsGpioPin *pins, uint8_t count, char *out, size_t cap);

/**
 * @brief Parse a control body of the form `pin=<n>&level=<0|1>` (form-encoded).
 * @return true if both fields parsed into @p pin / @p level.
 */
bool detws_gpio_parse_set(const char *body, size_t len, uint8_t *pin, uint8_t *level);

/** @brief True if @p pin is a drivable output in the table (guards a control POST). */
bool detws_gpio_is_output(const DetwsGpioPin *pins, uint8_t count, uint8_t pin);

// ---------------------------------------------------------------------------
// ESP32 integration (no-ops on host builds)
// ---------------------------------------------------------------------------

/** @brief Apply pinMode() for every entry per its direction (call once at setup). */
void detws_gpio_begin_pins(const DetwsGpioPin *pins, uint8_t count);

/** @brief Refresh each pin's live @c level via digitalRead (no-op on host). */
void detws_gpio_read(DetwsGpioPin *pins, uint8_t count);

/** @brief Drive an output @p pin to @p level via digitalWrite (no-op on host). */
void detws_gpio_write(uint8_t pin, uint8_t level);

/**
 * @brief Serve the GPIO map at @p path: GET returns the JSON, POST drives an
 *        output (body `pin=<n>&level=<0|1>`, only pins marked DETWS_GPIO_OUT).
 *        The pin table is caller-owned and must outlive the server.
 */
void detws_gpio_map_begin(DetWebServer &server, const char *path, DetwsGpioPin *pins, uint8_t count);

#endif // DETWS_ENABLE_GPIO_MAP
#endif // DETERMINISTICESPASYNCWEBSERVER_GPIO_MAP_H
