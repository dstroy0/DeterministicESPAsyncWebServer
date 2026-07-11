// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file guardrails.cpp
 * @brief Heap/stack guardrail evaluator + JSON (pure) and the ESP32 sampler.
 *
 * The evaluator and serializer are host-tested; the sample reads the live esp_* /
 * FreeRTOS counters on ESP32 and returns zeros on host.
 */

#include "services/guardrails/guardrails.h"

#if DETWS_ENABLE_GUARDRAILS

#include <stdio.h>

uint8_t detws_guardrail_eval(const DetwsHealth *h, uint32_t heap_min, uint32_t frag_min_block, uint32_t stack_min)
{
    uint8_t b = DetwsBreach::DETWS_BREACH_NONE;
    if (!h)
        return b;
    if (h->free_heap < heap_min)
        b |= DetwsBreach::DETWS_BREACH_HEAP;
    if (h->largest_free_block < frag_min_block)
        b |= DetwsBreach::DETWS_BREACH_FRAG;
    if (h->stack_free < stack_min)
        b |= DetwsBreach::DETWS_BREACH_STACK;
    return b;
}

int detws_health_json(const DetwsHealth *h, char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    out[0] = '\0';
    if (!h)
        return 0;
    int w = snprintf(out, cap, "{\"free_heap\":%u,\"min_free_heap\":%u,\"largest_free_block\":%u,\"stack_free\":%u}",
                     (unsigned)h->free_heap, (unsigned)h->min_free_heap, (unsigned)h->largest_free_block,
                     (unsigned)h->stack_free);
    if (w < 0 || (size_t)w >= cap)
    {
        out[0] = '\0';
        return 0;
    }
    return w;
}

#ifdef ARDUINO

#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace
{
// All guardrails sampler state, owned by one instance (internal linkage): the breach
// callback, so it is one named owner, unreachable from any other translation unit.
struct GuardrailsCtx
{
    detws_breach_fn cb = nullptr;
};
GuardrailsCtx s_gr;
} // namespace

void detws_guardrails_sample(DetwsHealth *h)
{
    if (!h)
        return;
    h->free_heap = esp_get_free_heap_size();
    h->min_free_heap = esp_get_minimum_free_heap_size();
    h->largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    h->stack_free = (uint32_t)uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t);
}

void detws_guardrails_begin(detws_breach_fn cb)
{
    s_gr.cb = cb;
}

uint8_t detws_guardrails_check(void)
{
    DetwsHealth h;
    detws_guardrails_sample(&h);
    uint8_t b =
        detws_guardrail_eval(&h, DETWS_GUARDRAIL_HEAP_MIN, DETWS_GUARDRAIL_FRAG_MIN_BLOCK, DETWS_GUARDRAIL_STACK_MIN);
    if (b != DetwsBreach::DETWS_BREACH_NONE && s_gr.cb)
        s_gr.cb(b, &h);
    return b;
}

#else // host build - no live counters

void detws_guardrails_sample(DetwsHealth *h)
{
    if (h)
        *h = DetwsHealth{};
}
void detws_guardrails_begin(detws_breach_fn)
{
}
uint8_t detws_guardrails_check(void)
{
    return DetwsBreach::DETWS_BREACH_NONE;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_GUARDRAILS
