// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file safety_scl.cpp
 * @brief IEC 61784-3 black-channel SCL shared primitives - implementation. See safety_scl.h.
 *
 * The whole module is one small state machine, and every transition out of RUNNING is one-way. The
 * elapsed-time test is an unsigned difference (`now - stamp >= limit`), which is what makes the
 * watchdog wrap-safe: at a millis() rollover the subtraction wraps with it and still yields the true
 * interval.
 */

#include "services/safety_scl/safety_scl.h"

#if DWS_ENABLE_SAFETY_SCL

namespace
{
// Latch the first fault: once fail-safe, later failures do not overwrite the diagnostically
// interesting one that actually broke the connection.
void trip(SclConn *c, SclFault why)
{
    if (c->state != SclState::FAILSAFE)
    {
        c->state = SclState::FAILSAFE;
        c->fault = why;
    }
}

uint32_t wrap(uint32_t v, uint32_t mod)
{
    return mod ? (v % mod) : v;
}
} // namespace

void dws_scl_init(SclConn *c, uint32_t first_counter, uint32_t counter_mod, uint32_t watchdog_ms, uint32_t now)
{
    if (!c)
        return;
    c->state = SclState::INIT;
    c->fault = SclFault::NONE;
    c->counter_mod = counter_mod;
    c->expected = wrap(first_counter, counter_mod);
    c->watchdog_ms = watchdog_ms;
    c->last_ok_ms = now;
    c->accepted = 0;
    c->rejected = 0;
}

bool dws_scl_on_frame(SclConn *c, bool signature_ok, uint32_t counter, uint32_t now)
{
    if (!c)
        return false;
    if (c->state == SclState::FAILSAFE)
    {
        c->rejected++; // refused, but the original fault stands
        return false;
    }

    // Corruption is checked first: a frame whose signature failed cannot be trusted to carry a
    // meaningful counter either, so reporting SIGNATURE is the honest diagnosis.
    if (!signature_ok)
    {
        c->rejected++;
        trip(c, SclFault::SIGNATURE);
        return false;
    }
    if (wrap(counter, c->counter_mod) != c->expected)
    {
        c->rejected++;
        trip(c, SclFault::COUNTER);
        return false;
    }

    c->expected = dws_scl_next_counter(c->expected, c->counter_mod);
    c->last_ok_ms = now;
    c->accepted++;
    c->state = SclState::RUNNING;
    return true;
}

bool dws_scl_poll(SclConn *c, uint32_t now)
{
    if (!c)
        return false;
    if (c->state == SclState::FAILSAFE)
        return false;
    // Only a connection that has actually run can time out; one still in INIT is starting up, not
    // silent. A zero watchdog disables the check entirely.
    if (c->state == SclState::RUNNING && c->watchdog_ms && (uint32_t)(now - c->last_ok_ms) >= c->watchdog_ms)
    {
        trip(c, SclFault::TIMEOUT);
        return false;
    }
    return true;
}

void dws_scl_reset(SclConn *c, uint32_t first_counter, uint32_t now)
{
    if (!c)
        return;
    c->state = SclState::INIT;
    c->fault = SclFault::NONE;
    c->expected = wrap(first_counter, c->counter_mod);
    c->last_ok_ms = now;
    // accepted / rejected deliberately preserved: they tally the whole session, not one connection
    // attempt, so a flapping link is visible rather than reset away.
}

bool dws_scl_ok(const SclConn *c)
{
    return c && c->state != SclState::FAILSAFE;
}

SclState dws_scl_state(const SclConn *c)
{
    return c ? c->state : SclState::FAILSAFE; // a missing connection is not a usable one
}

SclFault dws_scl_fault(const SclConn *c)
{
    return c ? c->fault : SclFault::NONE;
}

uint32_t dws_scl_next_counter(uint32_t counter, uint32_t counter_mod)
{
    uint32_t n = counter + 1u; // wraps naturally at 2^32 when no modulus is set
    return counter_mod ? (n % counter_mod) : n;
}

#endif // DWS_ENABLE_SAFETY_SCL
