// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file preempt_queue.cpp
 * @brief Preempting work queue + high-priority processing task - implementation.
 */

#include "services/preempt_queue/preempt_queue.h"

#if DETWS_ENABLE_PREEMPT_QUEUE

#include "shared_primitives/shim.h"

namespace
{
DetwsPqHandler s_handler = nullptr;
void *s_ctx = nullptr;
size_t s_high_water = 0; // peak items queued at once (sizing aid)
} // namespace

#ifdef ARDUINO

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace
{
StaticQueue_t s_q_struct;
uint8_t s_q_storage[DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
QueueHandle_t s_q = nullptr;
TaskHandle_t s_task = nullptr;
volatile bool s_run = false;

void note_depth(UBaseType_t waiting)
{
    if ((size_t)waiting > s_high_water)
        s_high_water = (size_t)waiting;
}

// The dedicated processing task: block until an item lands (so a post preempts
// straight into here), then run the handler for each item in order. It blocks
// forever between items (zero idle wakeups); detws_pq_stop() deletes it.
void pq_task(void *)
{
    uint8_t item[DETWS_PQ_ITEM_SIZE];
    for (;;)
    {
        if (xQueueReceive(s_q, item, portMAX_DELAY) == pdTRUE && s_handler)
            s_handler(item, s_ctx);
    }
}
} // namespace

bool detws_pq_start(const DetwsPqConfig *cfg)
{
    if (s_run || !cfg || !cfg->handler)
        return false;
    s_handler = cfg->handler;
    s_ctx = cfg->ctx;
    s_high_water = 0;
    if (!s_q)
        s_q = xQueueCreateStatic(DETWS_PQ_DEPTH, DETWS_PQ_ITEM_SIZE, s_q_storage, &s_q_struct);
    if (!s_q)
        return false;
    s_run = true;
    int core = cfg->core % portNUM_PROCESSORS;
    if (xTaskCreatePinnedToCore(pq_task, cfg->name ? cfg->name : "detws_pq", DETWS_PQ_STACK, nullptr, cfg->priority,
                                &s_task, core) != pdPASS)
    {
        s_run = false;
        return false;
    }
    return true;
}

bool detws_pq_post(const void *item, uint32_t timeout_ticks)
{
    if (!s_q || !item)
        return false;
    if (xQueueSendToBack(s_q, item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(uxQueueMessagesWaiting(s_q));
    return true;
}

bool detws_pq_post_urgent(const void *item, uint32_t timeout_ticks)
{
    if (!s_q || !item)
        return false;
    if (xQueueSendToFront(s_q, item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(uxQueueMessagesWaiting(s_q));
    return true;
}

bool detws_pq_post_from_isr(const void *item)
{
    if (!s_q || !item)
        return false;
    BaseType_t woke = pdFALSE;
    if (xQueueSendToBackFromISR(s_q, item, &woke) != pdTRUE)
        return false;
    note_depth(uxQueueMessagesWaitingFromISR(s_q));
    portYIELD_FROM_ISR(woke); // switch to the processing task now if it outranks us
    return true;
}

void detws_pq_drain(void)
{
    // The processing task drains on device; nothing to do here.
}

void detws_pq_stop(void)
{
    s_run = false;
    if (s_task) // the task blocks on the queue (portMAX_DELAY), so delete it directly
    {
        vTaskDelete(s_task);
        s_task = nullptr;
    }
}

bool detws_pq_running(void)
{
    return s_run;
}

size_t detws_pq_high_water(void)
{
    return s_high_water;
}

#else // host build - fixed ring, no task; detws_pq_drain() runs the handler

namespace
{
uint8_t s_buf[DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
size_t s_head = 0; // next write slot
size_t s_tail = 0; // next read slot
size_t s_count = 0;
bool s_started = false;

void note_count()
{
    if (s_count > s_high_water)
        s_high_water = s_count;
}
} // namespace

bool detws_pq_start(const DetwsPqConfig *cfg)
{
    if (s_started || !cfg || !cfg->handler)
        return false;
    s_handler = cfg->handler;
    s_ctx = cfg->ctx;
    s_head = s_tail = s_count = 0;
    s_high_water = 0;
    s_started = true;
    return true;
}

bool detws_pq_post(const void *item, uint32_t)
{
    if (!item || s_count >= DETWS_PQ_DEPTH)
        return false; // fail closed when full
    memcpy(s_buf + s_head * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_head = (s_head + 1) % DETWS_PQ_DEPTH;
    s_count++;
    note_count();
    return true;
}

bool detws_pq_post_urgent(const void *item, uint32_t)
{
    if (!item || s_count >= DETWS_PQ_DEPTH)
        return false;
    s_tail = (s_tail + DETWS_PQ_DEPTH - 1) % DETWS_PQ_DEPTH; // step the read cursor back, write there
    memcpy(s_buf + s_tail * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_count++;
    note_count();
    return true;
}

bool detws_pq_post_from_isr(const void *item)
{
    return detws_pq_post(item, 0); // no ISRs on host
}

void detws_pq_drain(void)
{
    while (s_count > 0)
    {
        if (s_handler)
            s_handler(s_buf + s_tail * DETWS_PQ_ITEM_SIZE, s_ctx);
        s_tail = (s_tail + 1) % DETWS_PQ_DEPTH;
        s_count--;
    }
}

void detws_pq_stop(void)
{
    s_started = false;
}

bool detws_pq_running(void)
{
    return s_started;
}

size_t detws_pq_high_water(void)
{
    return s_high_water;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_PREEMPT_QUEUE
