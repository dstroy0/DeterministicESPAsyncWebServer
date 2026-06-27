// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file worker.h
 * @brief Layer 5 (Session) - server worker identity.
 *
 * The server pipeline runs in one or more dedicated worker tasks (see
 * DETWS_WORKER_COUNT). Each worker owns a disjoint partition of connection slots
 * (slot i -> worker i % count) and its own scratch arena, so per-worker state
 * (the arena, work buffers) is selected by the caller's worker id. This header is
 * the single source of that id.
 *
 * The id is per-task/per-thread: a worker binds itself once at task entry via
 * detws_worker_set_self(); any context that has not bound an id (the user's
 * loop(), a unit test, the lwIP thread) reads 0, which is also the only valid id
 * in the default single-worker build, so DETWS_WORKER_COUNT == 1 is byte-for-byte
 * the original single-pipeline behavior.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_WORKER_H
#define DETERMINISTICESPASYNCWEBSERVER_WORKER_H

#include "DetWebServerConfig.h"

/** @brief Number of server worker tasks (DETWS_WORKER_COUNT). */
int detws_worker_count(void);

/** @brief Worker id [0, count) of the calling task; 0 by default / single-worker. */
int detws_worker_self(void);

/** @brief Bind the calling task/thread to worker id @p id (worker entry / tests). */
void detws_worker_set_self(int id);

// ---------------------------------------------------------------------------
// Worker tasks (ESP32)
// ---------------------------------------------------------------------------
//
// On ESP32 the server runs in dedicated FreeRTOS worker tasks instead of the
// user's loop(): detws_workers_start() spawns DETWS_WORKER_COUNT tasks, each
// pinned to a core, each binding its worker id and repeatedly invoking the
// app-supplied pump (so this layer stays free of any app dependency). On host
// builds there are no tasks - the pipeline is driven inline by handle() / tests -
// so these are no-ops and detws_workers_running() is false.

/** @brief Pump callback run by each worker task with its worker id. */
typedef void (*detws_worker_pump_fn)(int worker_id);

/** @brief Spawn the worker task(s) and start them running @p pump. No-op on host. */
void detws_workers_start(detws_worker_pump_fn pump);

/** @brief Signal the worker task(s) to exit and wait briefly for them. No-op on host. */
void detws_workers_stop(void);

/** @brief True while worker task(s) are running (always false on host). */
bool detws_workers_running(void);

#endif // DETERMINISTICESPASYNCWEBSERVER_WORKER_H
