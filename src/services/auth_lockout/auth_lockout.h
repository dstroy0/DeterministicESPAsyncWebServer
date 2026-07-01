// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file auth_lockout.h
 * @brief Per-IP brute-force lockout for HTTP auth (DETWS_ENABLE_AUTH_LOCKOUT).
 *
 * Tracks consecutive failed authentications per source IPv4 in a fixed BSS table
 * (no heap). After DETWS_AUTH_LOCKOUT_THRESHOLD consecutive failures an address
 * is locked out for DETWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure
 * up to DETWS_AUTH_LOCKOUT_MAX_MS; a successful auth clears the address. Compiled
 * only when DETWS_ENABLE_AUTH_LOCKOUT is set (the host unit tests enable it and
 * drive it with a synthetic millisecond clock). Addresses are identity keys only
 * (byte order does not matter); 0 is the empty-bucket sentinel and is treated as
 * untrackable (never locked).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_AUTH_LOCKOUT_H
#define DETERMINISTICESPASYNCWEBSERVER_AUTH_LOCKOUT_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_AUTH_LOCKOUT

#include "shared_primitives/shim.h"

/**
 * @brief Remaining lockout time for @p ip at @p now_ms, in milliseconds.
 *
 * @return 0 if the address is not currently locked out (or is untracked / zero);
 *         otherwise the milliseconds until the lockout expires. The window math
 *         is unsigned so it survives a millis() rollover.
 */
uint32_t auth_lockout_remaining_ms(uint32_t ip, uint32_t now_ms);

/** @brief Record a failed authentication from @p ip at @p now_ms (may start or escalate a lockout). */
void auth_lockout_fail(uint32_t ip, uint32_t now_ms);

/** @brief Clear @p ip's failure / lockout state after a successful authentication. */
void auth_lockout_succeed(uint32_t ip);

/** @brief Reset the whole lockout table (e.g. between tests). */
void auth_lockout_reset(void);

#endif // DETWS_ENABLE_AUTH_LOCKOUT

#endif // DETERMINISTICESPASYNCWEBSERVER_AUTH_LOCKOUT_H
