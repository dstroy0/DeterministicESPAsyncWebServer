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
