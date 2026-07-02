// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file preempt_queue.cpp
 * @brief Preempting work queues + high-priority processing tasks - implementation.
 *
 * One queue + one task per lane (DETWS_PQ_LANE_COUNT lanes). The no-lane detws_pq_* API
 * lives in the header and forwards to the USER lane. Internal lanes default to a higher
 * priority than the user lane so internal ingest preempts user work.
 */

#include "services/preempt_queue/preempt_queue.h"

#if DETWS_ENABLE_PREEMPT_QUEUE

#include <string.h>

namespace
{
DetwsPqHandler s_handler[DETWS_PQ_LANE_COUNT] = {};
void *s_ctx[DETWS_PQ_LANE_COUNT] = {};
size_t s_high_water[DETWS_PQ_LANE_COUNT] = {}; // peak items queued at once (sizing aid)

const char *lane_name(detws_pq_lane lane)
{
    switch (lane)
    {
    case DETWS_PQ_LANE_DMA:
        return "detws_pq_dma";
    case DETWS_PQ_LANE_FORWARD:
        return "detws_pq_fwd";
    case DETWS_PQ_LANE_DEVICE:
        return "detws_pq_dev";
    default:
        return "detws_pq_user";
    }
}

bool lane_ok(detws_pq_lane lane)
{
    return (unsigned)lane < (unsigned)DETWS_PQ_LANE_COUNT;
}
} // namespace

// Default task priority per lane: internal lanes rank above the user lane (DMA highest),
// staying below the lwIP tcpip (18) / WiFi tasks so networking is never starved.
uint8_t detws_pq_lane_priority(detws_pq_lane lane)
{
    switch (lane)
    {
    case DETWS_PQ_LANE_DMA:
        return (uint8_t)(DETWS_PQ_INTERNAL_PRIORITY + 2);
    case DETWS_PQ_LANE_FORWARD:
        return (uint8_t)(DETWS_PQ_INTERNAL_PRIORITY + 1);
    case DETWS_PQ_LANE_DEVICE:
        return (uint8_t)(DETWS_PQ_INTERNAL_PRIORITY);
    case DETWS_PQ_LANE_USER:
    default:
        return 5; // used only when a config passes priority 0; kept below the internal lanes
    }
}

#ifdef ARDUINO

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace
{
StaticQueue_t s_q_struct[DETWS_PQ_LANE_COUNT];
uint8_t s_q_storage[DETWS_PQ_LANE_COUNT][DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
QueueHandle_t s_q[DETWS_PQ_LANE_COUNT] = {};
TaskHandle_t s_task[DETWS_PQ_LANE_COUNT] = {};
volatile bool s_run[DETWS_PQ_LANE_COUNT] = {};

void note_depth(detws_pq_lane lane, UBaseType_t waiting)
{
    if ((size_t)waiting > s_high_water[lane])
        s_high_water[lane] = (size_t)waiting;
}

// The dedicated processing task for one lane (its id is the task parameter): block until
// an item lands (so a post preempts straight into here), then run the handler for each
// item in order. It blocks forever between items (zero idle wakeups).
void pq_task(void *arg)
{
    detws_pq_lane lane = (detws_pq_lane)(uintptr_t)arg;
    uint8_t item[DETWS_PQ_ITEM_SIZE];
    for (;;)
    {
        if (xQueueReceive(s_q[lane], item, portMAX_DELAY) == pdTRUE && s_handler[lane])
            s_handler[lane](item, s_ctx[lane]);
    }
}
} // namespace

bool detws_pq_start_lane(detws_pq_lane lane, const DetwsPqConfig *cfg)
{
    if (!lane_ok(lane) || s_run[lane] || !cfg || !cfg->handler)
        return false;
    s_handler[lane] = cfg->handler;
    s_ctx[lane] = cfg->ctx;
    s_high_water[lane] = 0;
    if (!s_q[lane])
        s_q[lane] = xQueueCreateStatic(DETWS_PQ_DEPTH, DETWS_PQ_ITEM_SIZE, s_q_storage[lane], &s_q_struct[lane]);
    if (!s_q[lane])
        return false;
    s_run[lane] = true;
    uint8_t prio = cfg->priority ? cfg->priority : detws_pq_lane_priority(lane);
    int core = cfg->core % portNUM_PROCESSORS;
    if (xTaskCreatePinnedToCore(pq_task, cfg->name ? cfg->name : lane_name(lane), DETWS_PQ_STACK,
                                (void *)(uintptr_t)lane, prio, &s_task[lane], core) != pdPASS)
    {
        s_run[lane] = false;
        return false;
    }
    return true;
}

bool detws_pq_post_lane(detws_pq_lane lane, const void *item, uint32_t timeout_ticks)
{
    if (!lane_ok(lane) || !s_q[lane] || !item)
        return false;
    if (xQueueSendToBack(s_q[lane], item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaiting(s_q[lane]));
    return true;
}

bool detws_pq_post_lane_urgent(detws_pq_lane lane, const void *item, uint32_t timeout_ticks)
{
    if (!lane_ok(lane) || !s_q[lane] || !item)
        return false;
    if (xQueueSendToFront(s_q[lane], item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaiting(s_q[lane]));
    return true;
}

bool detws_pq_post_lane_from_isr(detws_pq_lane lane, const void *item)
{
    if (!lane_ok(lane) || !s_q[lane] || !item)
        return false;
    BaseType_t woke = pdFALSE;
    if (xQueueSendToBackFromISR(s_q[lane], item, &woke) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaitingFromISR(s_q[lane]));
    portYIELD_FROM_ISR(woke); // switch to the processing task now if it outranks us
    return true;
}

void detws_pq_drain_lane(detws_pq_lane)
{
    // The lane's task drains on device; nothing to do here.
}

void detws_pq_stop_lane(detws_pq_lane lane)
{
    if (!lane_ok(lane))
        return;
    s_run[lane] = false;
    if (s_task[lane]) // the task blocks on the queue (portMAX_DELAY), so delete it directly
    {
        vTaskDelete(s_task[lane]);
        s_task[lane] = nullptr;
    }
}

bool detws_pq_running_lane(detws_pq_lane lane)
{
    return lane_ok(lane) && s_run[lane];
}

size_t detws_pq_high_water_lane(detws_pq_lane lane)
{
    return lane_ok(lane) ? s_high_water[lane] : 0;
}

#else // host build - fixed per-lane rings, no tasks; detws_pq_drain_lane() runs the handler

namespace
{
uint8_t s_buf[DETWS_PQ_LANE_COUNT][DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
size_t s_head[DETWS_PQ_LANE_COUNT] = {}; // next write slot
size_t s_tail[DETWS_PQ_LANE_COUNT] = {}; // next read slot
size_t s_count[DETWS_PQ_LANE_COUNT] = {};
bool s_started[DETWS_PQ_LANE_COUNT] = {};

void note_count(detws_pq_lane lane)
{
    if (s_count[lane] > s_high_water[lane])
        s_high_water[lane] = s_count[lane];
}
} // namespace

bool detws_pq_start_lane(detws_pq_lane lane, const DetwsPqConfig *cfg)
{
    if (!lane_ok(lane) || s_started[lane] || !cfg || !cfg->handler)
        return false;
    s_handler[lane] = cfg->handler;
    s_ctx[lane] = cfg->ctx;
    s_head[lane] = s_tail[lane] = s_count[lane] = 0;
    s_high_water[lane] = 0;
    s_started[lane] = true;
    return true;
}

bool detws_pq_post_lane(detws_pq_lane lane, const void *item, uint32_t)
{
    if (!lane_ok(lane) || !item || s_count[lane] >= DETWS_PQ_DEPTH)
        return false; // fail closed when full
    memcpy(s_buf[lane] + s_head[lane] * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_head[lane] = (s_head[lane] + 1) % DETWS_PQ_DEPTH;
    s_count[lane]++;
    note_count(lane);
    return true;
}

bool detws_pq_post_lane_urgent(detws_pq_lane lane, const void *item, uint32_t)
{
    if (!lane_ok(lane) || !item || s_count[lane] >= DETWS_PQ_DEPTH)
        return false;
    s_tail[lane] = (s_tail[lane] + DETWS_PQ_DEPTH - 1) % DETWS_PQ_DEPTH; // step the read cursor back, write there
    memcpy(s_buf[lane] + s_tail[lane] * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_count[lane]++;
    note_count(lane);
    return true;
}

bool detws_pq_post_lane_from_isr(detws_pq_lane lane, const void *item)
{
    return detws_pq_post_lane(lane, item, 0); // no ISRs on host
}

void detws_pq_drain_lane(detws_pq_lane lane)
{
    if (!lane_ok(lane))
        return;
    while (s_count[lane] > 0)
    {
        if (s_handler[lane])
            s_handler[lane](s_buf[lane] + s_tail[lane] * DETWS_PQ_ITEM_SIZE, s_ctx[lane]);
        s_tail[lane] = (s_tail[lane] + 1) % DETWS_PQ_DEPTH;
        s_count[lane]--;
    }
}

void detws_pq_stop_lane(detws_pq_lane lane)
{
    if (lane_ok(lane))
        s_started[lane] = false;
}

bool detws_pq_running_lane(detws_pq_lane lane)
{
    return lane_ok(lane) && s_started[lane];
}

size_t detws_pq_high_water_lane(detws_pq_lane lane)
{
    return lane_ok(lane) ? s_high_water[lane] : 0;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_PREEMPT_QUEUE
