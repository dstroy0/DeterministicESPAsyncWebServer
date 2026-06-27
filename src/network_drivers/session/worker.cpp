// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file worker.cpp
 * @brief Server worker identity - implementation.
 *
 * The id lives in thread-local storage so each worker task resolves its own
 * per-worker state with no lock and no shared lookup. The block is part of task
 * creation (no heap after begin()); an unbound context reads the zero default.
 */

#include "network_drivers/session/worker.h"
#include <atomic>

namespace
{
// Per-task worker id. Default 0: the user loop(), the lwIP thread, and unit tests
// all read worker 0, which is the sole worker in the default build.
thread_local int t_worker_id = 0;
} // namespace

int detws_worker_count(void)
{
    return DETWS_WORKER_COUNT;
}

int detws_worker_self(void)
{
    return t_worker_id;
}

void detws_worker_set_self(int id)
{
    t_worker_id = id;
}

// ---------------------------------------------------------------------------
// Worker tasks
// ---------------------------------------------------------------------------

#ifdef ARDUINO

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace
{
detws_worker_pump_fn s_pump = nullptr;
TaskHandle_t s_tasks[DETWS_WORKER_COUNT] = {nullptr};
std::atomic<bool> s_run{false}; // release on start publishes s_pump; acquire in the task

// Each worker binds its id, then pumps until asked to stop. A 1-tick yield keeps
// the lower-priority idle/loop tasks alive without adding latency past one tick.
void worker_task(void *arg)
{
    int id = (int)(intptr_t)arg;
    detws_worker_set_self(id);
    while (s_run.load(std::memory_order_acquire))
    {
        if (s_pump)
            s_pump(id);
        vTaskDelay(DETWS_WORKER_POLL_TICKS); // default 1 = 1000 Hz (tested cadence)
    }
    s_tasks[id] = nullptr;
    vTaskDelete(nullptr);
}
} // namespace

// Per-worker deferred-callback queues: app code on any task hands a {fn, arg} to
// the owning worker, which runs it in its own context (race-free push path).
namespace
{
struct DeferCmd
{
    detws_deferred_fn fn;
    void *arg;
};
StaticQueue_t s_dq_struct[DETWS_WORKER_COUNT];
uint8_t s_dq_storage[DETWS_WORKER_COUNT][DETWS_DEFER_QUEUE_DEPTH * sizeof(DeferCmd)];
QueueHandle_t s_dq[DETWS_WORKER_COUNT] = {nullptr};
} // namespace

void detws_workers_start(detws_worker_pump_fn pump)
{
    if (s_run.load(std::memory_order_acquire))
        return; // already running
    s_pump = pump;
    for (int i = 0; i < DETWS_WORKER_COUNT; i++)
        if (!s_dq[i])
            s_dq[i] = xQueueCreateStatic(DETWS_DEFER_QUEUE_DEPTH, sizeof(DeferCmd), s_dq_storage[i], &s_dq_struct[i]);
    s_run.store(true, std::memory_order_release);
    for (int i = 0; i < DETWS_WORKER_COUNT; i++)
    {
        int core = (DETWS_WORKER_CORE + i) % portNUM_PROCESSORS;
        xTaskCreatePinnedToCore(worker_task, "detws_worker", DETWS_WORKER_TASK_STACK, (void *)(intptr_t)i,
                                DETWS_WORKER_TASK_PRIORITY, &s_tasks[i], core);
    }
}

bool detws_defer(int worker_id, detws_deferred_fn fn, void *arg)
{
    if (!fn)
        return false;
    if (worker_id < 0 || worker_id >= DETWS_WORKER_COUNT || !s_dq[worker_id])
        return false;
    DeferCmd cmd = {fn, arg};
    return xQueueSend(s_dq[worker_id], &cmd, 0) == pdTRUE;
}

void detws_worker_run_deferred(int worker_id)
{
    if (worker_id < 0 || worker_id >= DETWS_WORKER_COUNT || !s_dq[worker_id])
        return;
    DeferCmd cmd;
    while (xQueueReceive(s_dq[worker_id], &cmd, 0) == pdTRUE)
        if (cmd.fn)
            cmd.fn(cmd.arg);
}

void detws_workers_stop(void)
{
    if (!s_run.load(std::memory_order_acquire))
        return;
    s_run.store(false, std::memory_order_release);
    // Tasks self-delete on their next iteration; give them a few ticks to exit
    // before the caller tears down the slots they were servicing.
    vTaskDelay(3);
}

bool detws_workers_running(void)
{
    return s_run.load(std::memory_order_acquire);
}

#else // host build - no tasks; handle()/tests drive the pipeline inline

void detws_workers_start(detws_worker_pump_fn)
{
}
void detws_workers_stop(void)
{
}
bool detws_workers_running(void)
{
    return false;
}

// No worker task on host: the caller and the pipeline are the same thread, so a
// deferred callback can run inline immediately (same observable effect, race-free).
bool detws_defer(int, detws_deferred_fn fn, void *arg)
{
    if (!fn)
        return false;
    fn(arg);
    return true;
}
void detws_worker_run_deferred(int)
{
}

#endif // ARDUINO
