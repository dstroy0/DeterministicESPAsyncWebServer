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
// Common preempt-queue state (both host + device), owned by one instance (internal linkage):
// the per-lane handler, its context, and the high-water mark. The backend-specific state (the
// FreeRTOS queue/task on device, the ring on host) lives in its own owner where its types are
// in scope. One named owner, unreachable from any other translation unit.
struct PqCtx
{
    DetwsPqHandler handler[DETWS_PQ_LANE_COUNT] = {};
    void *ctx[DETWS_PQ_LANE_COUNT] = {};
    size_t high_water[DETWS_PQ_LANE_COUNT] = {}; // peak items queued at once (sizing aid)
};
PqCtx s_pq;

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
// All FreeRTOS-backend state, owned by one instance (internal linkage): the static queue
// storage/control blocks, the queue + task handles, and the per-lane run flag. One named
// owner, unreachable from any other translation unit.
struct PqQueueCtx
{
    StaticQueue_t q_struct[DETWS_PQ_LANE_COUNT];
    uint8_t q_storage[DETWS_PQ_LANE_COUNT][DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
    QueueHandle_t q[DETWS_PQ_LANE_COUNT] = {};
    TaskHandle_t task[DETWS_PQ_LANE_COUNT] = {};
    volatile bool run[DETWS_PQ_LANE_COUNT] = {};
};
PqQueueCtx s_pqq;

void note_depth(detws_pq_lane lane, UBaseType_t waiting)
{
    if ((size_t)waiting > s_pq.high_water[lane])
        s_pq.high_water[lane] = (size_t)waiting;
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
        if (xQueueReceive(s_pqq.q[lane], item, portMAX_DELAY) == pdTRUE && s_pq.handler[lane])
            s_pq.handler[lane](item, s_pq.ctx[lane]);
    }
}
} // namespace

bool detws_pq_start_lane(detws_pq_lane lane, const DetwsPqConfig *cfg)
{
    if (!lane_ok(lane) || s_pqq.run[lane] || !cfg || !cfg->handler)
        return false;
    s_pq.handler[lane] = cfg->handler;
    s_pq.ctx[lane] = cfg->ctx;
    s_pq.high_water[lane] = 0;
    if (!s_pqq.q[lane])
        s_pqq.q[lane] =
            xQueueCreateStatic(DETWS_PQ_DEPTH, DETWS_PQ_ITEM_SIZE, s_pqq.q_storage[lane], &s_pqq.q_struct[lane]);
    if (!s_pqq.q[lane])
        return false;
    s_pqq.run[lane] = true;
    uint8_t prio = cfg->priority ? cfg->priority : detws_pq_lane_priority(lane);
    int core = cfg->core % portNUM_PROCESSORS;
    if (xTaskCreatePinnedToCore(pq_task, cfg->name ? cfg->name : lane_name(lane), DETWS_PQ_STACK,
                                (void *)(uintptr_t)lane, prio, &s_pqq.task[lane], core) != pdPASS)
    {
        s_pqq.run[lane] = false;
        return false;
    }
    return true;
}

bool detws_pq_post_lane(detws_pq_lane lane, const void *item, uint32_t timeout_ticks)
{
    if (!lane_ok(lane) || !s_pqq.q[lane] || !item)
        return false;
    if (xQueueSendToBack(s_pqq.q[lane], item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaiting(s_pqq.q[lane]));
    return true;
}

bool detws_pq_post_lane_urgent(detws_pq_lane lane, const void *item, uint32_t timeout_ticks)
{
    if (!lane_ok(lane) || !s_pqq.q[lane] || !item)
        return false;
    if (xQueueSendToFront(s_pqq.q[lane], item, (TickType_t)timeout_ticks) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaiting(s_pqq.q[lane]));
    return true;
}

bool detws_pq_post_lane_from_isr(detws_pq_lane lane, const void *item)
{
    if (!lane_ok(lane) || !s_pqq.q[lane] || !item)
        return false;
    BaseType_t woke = pdFALSE;
    if (xQueueSendToBackFromISR(s_pqq.q[lane], item, &woke) != pdTRUE)
        return false;
    note_depth(lane, uxQueueMessagesWaitingFromISR(s_pqq.q[lane]));
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
    s_pqq.run[lane] = false;
    if (s_pqq.task[lane]) // the task blocks on the queue (portMAX_DELAY), so delete it directly
    {
        vTaskDelete(s_pqq.task[lane]);
        s_pqq.task[lane] = nullptr;
    }
}

bool detws_pq_running_lane(detws_pq_lane lane)
{
    return lane_ok(lane) && s_pqq.run[lane];
}

size_t detws_pq_high_water_lane(detws_pq_lane lane)
{
    return lane_ok(lane) ? s_pq.high_water[lane] : 0;
}

#else // host build - fixed per-lane rings, no tasks; detws_pq_drain_lane() runs the handler

namespace
{
// All host-backend state, owned by one instance (internal linkage): the per-lane ring buffer,
// its head/tail/count cursors, and the started flag. One named owner, unreachable cross-TU.
struct PqRingCtx
{
    uint8_t buf[DETWS_PQ_LANE_COUNT][DETWS_PQ_DEPTH * DETWS_PQ_ITEM_SIZE];
    size_t head[DETWS_PQ_LANE_COUNT] = {}; // next write slot
    size_t tail[DETWS_PQ_LANE_COUNT] = {}; // next read slot
    size_t count[DETWS_PQ_LANE_COUNT] = {};
    bool started[DETWS_PQ_LANE_COUNT] = {};
};
PqRingCtx s_pqr;

void note_count(detws_pq_lane lane)
{
    if (s_pqr.count[lane] > s_pq.high_water[lane])
        s_pq.high_water[lane] = s_pqr.count[lane];
}
} // namespace

bool detws_pq_start_lane(detws_pq_lane lane, const DetwsPqConfig *cfg)
{
    if (!lane_ok(lane) || s_pqr.started[lane] || !cfg || !cfg->handler)
        return false;
    s_pq.handler[lane] = cfg->handler;
    s_pq.ctx[lane] = cfg->ctx;
    s_pqr.head[lane] = 0;
    s_pqr.tail[lane] = 0;
    s_pqr.count[lane] = 0;
    s_pq.high_water[lane] = 0;
    s_pqr.started[lane] = true;
    return true;
}

bool detws_pq_post_lane(detws_pq_lane lane, const void *item, uint32_t)
{
    if (!lane_ok(lane) || !item || s_pqr.count[lane] >= DETWS_PQ_DEPTH)
        return false; // fail closed when full
    memcpy(s_pqr.buf[lane] + s_pqr.head[lane] * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_pqr.head[lane] = (s_pqr.head[lane] + 1) % DETWS_PQ_DEPTH;
    s_pqr.count[lane]++;
    note_count(lane);
    return true;
}

bool detws_pq_post_lane_urgent(detws_pq_lane lane, const void *item, uint32_t)
{
    if (!lane_ok(lane) || !item || s_pqr.count[lane] >= DETWS_PQ_DEPTH)
        return false;
    s_pqr.tail[lane] =
        (s_pqr.tail[lane] + DETWS_PQ_DEPTH - 1) % DETWS_PQ_DEPTH; // step the read cursor back, write there
    memcpy(s_pqr.buf[lane] + s_pqr.tail[lane] * DETWS_PQ_ITEM_SIZE, item, DETWS_PQ_ITEM_SIZE);
    s_pqr.count[lane]++;
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
    while (s_pqr.count[lane] > 0)
    {
        if (s_pq.handler[lane])
            s_pq.handler[lane](s_pqr.buf[lane] + s_pqr.tail[lane] * DETWS_PQ_ITEM_SIZE, s_pq.ctx[lane]);
        s_pqr.tail[lane] = (s_pqr.tail[lane] + 1) % DETWS_PQ_DEPTH;
        s_pqr.count[lane]--;
    }
}

void detws_pq_stop_lane(detws_pq_lane lane)
{
    if (lane_ok(lane))
        s_pqr.started[lane] = false;
}

bool detws_pq_running_lane(detws_pq_lane lane)
{
    return lane_ok(lane) && s_pqr.started[lane];
}

size_t detws_pq_high_water_lane(detws_pq_lane lane)
{
    return lane_ok(lane) ? s_pq.high_water[lane] : 0;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_PREEMPT_QUEUE
