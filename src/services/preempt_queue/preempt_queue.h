// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file preempt_queue.h
 * @brief User-configurable preempting work queue + high-priority processing task
 *        (DETWS_ENABLE_PREEMPT_QUEUE) - the v5 real-time ingest primitive.
 *
 * One fixed-capacity queue feeding one dedicated, high-priority, core-pinned task.
 * A producer posts a fixed-size item; the scheduler **preempts** the lower-priority
 * producer the instant the item lands so it is processed immediately instead of on
 * the next tick. Producers post from a task (back or front, with a wait timeout) or
 * from an ISR (interrupt-safe, with an immediate context-switch request). The
 * processing task pops items in order and hands each to a user handler.
 *
 * This is the single normalized pipe for "hardware event -> process now": a
 * DMA-complete / GPIO / bus ISR posts a descriptor, the high-priority task drains
 * it. Zero-heap (static FreeRTOS queue + task storage, compile-time sized),
 * fail-closed on a full queue, no hot-path locks - so latency stays bounded
 * (determinism holds).
 *
 * Sizing is compile-time (static allocation): DETWS_PQ_DEPTH items of
 * DETWS_PQ_ITEM_SIZE bytes, DETWS_PQ_STACK task stack. The task priority and core
 * are set by the caller at detws_pq_start() (users own their task priorities).
 *
 * Host builds have no FreeRTOS task: posts enqueue into the same fixed ring and
 * detws_pq_drain() runs the handler over what is queued, so the logic is
 * host-testable and behaves identically to the device's draining task.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PREEMPT_QUEUE_H
#define DETERMINISTICESPASYNCWEBSERVER_PREEMPT_QUEUE_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_PREEMPT_QUEUE

#include "shared_primitives/shim.h"

/**
 * @brief Handler the processing task invokes for each dequeued item.
 * @param item pointer to DETWS_PQ_ITEM_SIZE bytes (the posted item).
 * @param ctx  the opaque pointer passed to detws_pq_start().
 */
typedef void (*DetwsPqHandler)(const void *item, void *ctx);

/** @brief Processing-task configuration set by the caller at start. */
struct DetwsPqConfig
{
    DetwsPqHandler handler; ///< Called once per dequeued item (required).
    void *ctx;              ///< Opaque, forwarded to @ref handler.
    uint8_t priority;       ///< Processing-task priority (user-set; high = preempts producers).
    uint8_t core;           ///< Core to pin the task to (ESP32; ignored on host).
    const char *name;       ///< Task name (debug); may be nullptr.
};

/**
 * @brief Create the queue and start the high-priority processing task.
 *
 * Idempotent: a second call while running is a no-op returning false. On host no
 * task is started (drive with detws_pq_drain()).
 * @return true if started (or already running on a prior call returns false).
 */
bool detws_pq_start(const DetwsPqConfig *cfg);

/**
 * @brief Post an item to the back of the queue from a task context.
 *
 * Copies DETWS_PQ_ITEM_SIZE bytes from @p item. Blocks up to @p timeout_ticks for
 * space, then fails closed.
 * @return true if queued; false if the queue stayed full (item dropped).
 */
bool detws_pq_post(const void *item, uint32_t timeout_ticks);

/**
 * @brief Post an item to the FRONT of the queue (urgent) from a task context.
 *        Same contract as detws_pq_post() otherwise.
 */
bool detws_pq_post_urgent(const void *item, uint32_t timeout_ticks);

/**
 * @brief Post an item from an ISR (interrupt-safe).
 *
 * Uses the interrupt-safe enqueue and requests an immediate context switch if a
 * higher-priority task (the processing task) was woken, so processing begins right
 * after the ISR returns rather than on the next tick. Never blocks.
 * @return true if queued; false if the queue was full (item dropped).
 */
bool detws_pq_post_from_isr(const void *item);

/**
 * @brief Run the handler over every currently-queued item (host / inline drive).
 *
 * No-op on ARDUINO, where the processing task drains the queue. On host this is how
 * tests and inline code process posted items.
 */
void detws_pq_drain(void);

/** @brief Stop the processing task (no-op on host). */
void detws_pq_stop(void);

/** @brief True while the processing task is running (always false on host). */
bool detws_pq_running(void);

/** @brief Peak number of items ever queued at once (for sizing DETWS_PQ_DEPTH). */
size_t detws_pq_high_water(void);

#endif // DETWS_ENABLE_PREEMPT_QUEUE

#endif // DETERMINISTICESPASYNCWEBSERVER_PREEMPT_QUEUE_H
