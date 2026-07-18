// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file guardrails.h
 * @brief Runtime heap/stack guardrails (DWS_ENABLE_GUARDRAILS).
 *
 * Samples the live health of the device - free heap, the heap low-water mark, the
 * largest free block (a fragmentation signal), and the calling task's remaining
 * stack - and trips a guardrail (callback) when any value crosses its configured
 * floor. A proactive fail-safe hook on top of the passive numbers in /metrics: an
 * app can shed load, drop to a safe state, or reboot before exhaustion bites.
 *
 * The threshold evaluator and the JSON serializer are pure and host-tested; the
 * sample reads `esp_get_free_heap_size` / `heap_caps_get_largest_free_block` /
 * `uxTaskGetStackHighWaterMark` on ESP32 (zeros on host).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_GUARDRAILS_H
#define DETERMINISTICESPASYNCWEBSERVER_GUARDRAILS_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_GUARDRAILS

/** @brief A health snapshot. */
struct DetwsHealth
{
    uint32_t free_heap;          ///< current free heap (bytes).
    uint32_t min_free_heap;      ///< lowest free heap since boot (bytes).
    uint32_t largest_free_block; ///< largest allocatable block (fragmentation, bytes).
    uint32_t stack_free;         ///< calling task's remaining stack (bytes).
};

/** @brief Guardrail breach flags: a bitmask OR'd together, so integer constants in a namespacing
 *  struct (cast-free at every | / &). */
struct DetwsBreach
{
    static constexpr uint8_t DWS_BREACH_NONE = 0;
    static constexpr uint8_t DWS_BREACH_HEAP = 1;  ///< free heap below DWS_GUARDRAIL_HEAP_MIN.
    static constexpr uint8_t DWS_BREACH_FRAG = 2;  ///< largest block below DWS_GUARDRAIL_FRAG_MIN_BLOCK.
    static constexpr uint8_t DWS_BREACH_STACK = 4; ///< task stack remaining below DWS_GUARDRAIL_STACK_MIN.
};

// ---------------------------------------------------------------------------
// Host-testable core
// ---------------------------------------------------------------------------

/** @brief Evaluate @p h against the floors; returns a DWS_BREACH_* bitmask. */
uint8_t dws_guardrail_eval(const DetwsHealth *h, uint32_t heap_min, uint32_t frag_min_block, uint32_t stack_min);

/**
 * @brief Serialize a health snapshot as JSON into @p out.
 * @return characters written, or 0 if @p cap is too small (fail-closed).
 */
int dws_health_json(const DetwsHealth *h, char *out, size_t cap);

// ---------------------------------------------------------------------------
// Sampling + guardrail check (ESP32; zeros / no-op on host)
// ---------------------------------------------------------------------------

/** @brief Fill @p h from the live esp_* / FreeRTOS counters (zeros on host). */
void dws_guardrails_sample(DetwsHealth *h);

/** @brief Breach callback: @p breaches is a DWS_BREACH_* bitmask, @p h the snapshot. */
typedef void (*dws_breach_fn)(uint8_t breaches, const DetwsHealth *h);

/** @brief Install the breach callback (thresholds come from DWS_GUARDRAIL_*). */
void dws_guardrails_begin(dws_breach_fn cb);

/**
 * @brief Sample, evaluate, and fire the callback if any guardrail is breached.
 * @return the DWS_BREACH_* bitmask (0 = all clear).
 */
uint8_t dws_guardrails_check(void);

#endif // DWS_ENABLE_GUARDRAILS
#endif // DETERMINISTICESPASYNCWEBSERVER_GUARDRAILS_H
