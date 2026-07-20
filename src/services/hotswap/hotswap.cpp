// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hotswap.cpp
 * @brief Removable-storage state machine + its binding (see hotswap.h).
 */

#include "services/hotswap/hotswap.h"

#if DWS_ENABLE_HOTSWAP

#include "services/clock.h" // dws_millis
#include <stdio.h>

// ---------------------------------------------------------------------------
// Pure core
// ---------------------------------------------------------------------------

void dws_hotswap_core_init(HotswapCore *c, uint8_t fail_threshold, uint32_t probe_interval_ms, uint32_t now)
{
    if (!c)
        return;
    c->state = StorageState::ABSENT;
    c->fail_run = 0;
    // A zero threshold would fault the volume before any failure had been seen.
    c->fail_threshold = fail_threshold ? fail_threshold : 1;
    c->probe_interval_ms = probe_interval_ms;
    // Back-date the first probe so a mount is attempted immediately rather than one interval late.
    c->last_probe_ms = now - probe_interval_ms;
    c->mounts = 0;
    c->faults = 0;
}

bool dws_hotswap_core_io(HotswapCore *c, bool ok)
{
    if (!c || c->state != StorageState::READY)
        return false; // not mounted: the caller should not have been touching it
    if (ok)
    {
        c->fail_run = 0; // any success proves the medium is still there
        return false;
    }
    if (c->fail_run < 0xFF)
        c->fail_run++;
    if (c->fail_run < c->fail_threshold)
        return false; // one bad write is not a removal
    c->state = StorageState::FAULTED;
    c->faults++;
    return true;
}

bool dws_hotswap_core_due(const HotswapCore *c, uint32_t now)
{
    if (!c || c->state == StorageState::READY)
        return false;
    // Unsigned delta, so this is correct across a millis() rollover.
    return (now - c->last_probe_ms) >= c->probe_interval_ms;
}

bool dws_hotswap_core_probe(HotswapCore *c, bool present, bool mounted, uint32_t now)
{
    if (!c)
        return false;
    c->last_probe_ms = now;
    StorageState was = c->state;
    if (present && mounted)
    {
        c->state = StorageState::READY;
        c->fail_run = 0;
        if (was != StorageState::READY)
            c->mounts++;
    }
    else
    {
        // Present but unmountable is not storage, so it reads the same as absent.
        c->state = StorageState::ABSENT;
        c->fail_run = 0;
    }
    return c->state != was;
}

// ---------------------------------------------------------------------------
// Binding
// ---------------------------------------------------------------------------

/** @brief Owned state: the core plus the app's callbacks. */
struct HotswapCtx
{
    HotswapCore core;
    DWSHotswapMount mount;
    DWSHotswapUnmount unmount;
    DWSHotswapPresent present;
    DWSHotswapEvent event;
    void *ctx;
    bool begun;
};
static HotswapCtx s_hs = {{StorageState::ABSENT, 0, 1, 0, 0, 0, 0}, nullptr, nullptr, nullptr, nullptr, nullptr, false};

static void hs_notify(StorageState from, StorageState to)
{
    if (s_hs.event && from != to)
        s_hs.event(from, to, s_hs.ctx);
}

void dws_hotswap_begin(DWSHotswapMount mount, DWSHotswapUnmount unmount, DWSHotswapPresent present, void *ctx)
{
    s_hs.mount = mount;
    s_hs.unmount = unmount;
    s_hs.present = present;
    s_hs.ctx = ctx;
    s_hs.begun = true;
    dws_hotswap_core_init(&s_hs.core, DWS_HOTSWAP_FAIL_THRESHOLD, DWS_HOTSWAP_PROBE_MS, dws_millis());
}

void dws_hotswap_set_event_cb(DWSHotswapEvent cb)
{
    s_hs.event = cb;
}

void dws_hotswap_poll_at(uint32_t now)
{
    if (!s_hs.begun || !dws_hotswap_core_due(&s_hs.core, now))
        return;

    // A volume that faulted is still mounted as far as the driver knows. Drop it before retrying,
    // so the remount starts from a clean state instead of reusing handles to a card that left.
    if (s_hs.core.state == StorageState::FAULTED && s_hs.unmount)
        s_hs.unmount(s_hs.ctx);

    bool present = s_hs.present ? s_hs.present(s_hs.ctx) : true;
    bool mounted = false;
    if (present && s_hs.mount)
        mounted = s_hs.mount(s_hs.ctx);

    StorageState was = s_hs.core.state;
    if (dws_hotswap_core_probe(&s_hs.core, present, mounted, now))
        hs_notify(was, s_hs.core.state);
}

void dws_hotswap_poll(void)
{
    dws_hotswap_poll_at(dws_millis());
}

bool dws_hotswap_ready(void)
{
    return s_hs.core.state == StorageState::READY;
}

void dws_hotswap_io(bool ok)
{
    StorageState was = s_hs.core.state;
    if (!dws_hotswap_core_io(&s_hs.core, ok))
        return;
    // Just faulted: drop the mount now rather than at the next poll, so nothing else can write
    // through a handle to a card that is no longer there.
    if (s_hs.unmount)
        s_hs.unmount(s_hs.ctx);
    hs_notify(was, s_hs.core.state);
}

StorageState dws_hotswap_state(void)
{
    return s_hs.core.state;
}

const char *dws_hotswap_state_name(StorageState s)
{
    switch (s)
    {
    case StorageState::READY:
        return "ready";
    case StorageState::FAULTED:
        return "faulted";
    case StorageState::ABSENT:
    default:
        return "absent";
    }
}

size_t dws_hotswap_json(char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    int n = snprintf(out, cap, "{\"storage\":\"%s\",\"mounts\":%u,\"faults\":%u}",
                     dws_hotswap_state_name(s_hs.core.state), (unsigned)s_hs.core.mounts, (unsigned)s_hs.core.faults);
    if (n < 0 || (size_t)n >= cap)
    {
        out[0] = '\0';
        return 0; // fail closed rather than emit a truncated object
    }
    return (size_t)n;
}

#endif // DWS_ENABLE_HOTSWAP
