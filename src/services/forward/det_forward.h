// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_forward.h
 * @brief Interface forwarding plane (DETWS_ENABLE_FORWARD) - the v5 bridge / router.
 *
 * A forwarding plane over the ingest pipeline. You register **interfaces** (Wi-Fi STA /
 * AP, Ethernet, a peripheral bus, a radio), each with an egress **send callback**, then
 * add per-pair **rules** (`src -> dst`, allow or deny, with an optional rate cap). When a
 * frame arrives on an interface you call det_forward_ingress(); the plane evaluates the
 * rules and forwards the bytes to **every allowed destination** by calling that
 * destination's send callback - so the device bridges / routes between its interfaces
 * instead of only terminating traffic.
 *
 * The canonical wiring is DMA-driven: an inbound DMA-complete event (services/dma) is
 * posted onto the FORWARD lane (services/preempt_queue), whose task calls
 * det_forward_ingress(), and each destination's send callback hands the bytes to that
 * interface's egress DMA. The plane itself is decoupled from both - it only knows
 * interfaces, rules, and the send callbacks - so it is pure and host-testable.
 *
 * **Default-deny**: a `(src, dst)` pair is forwarded only when an ALLOW rule matches and
 * no DENY rule does (a DENY always wins). A frame is never reflected to its source
 * interface. **Fail-closed**: an exceeded rate cap or a send callback returning false
 * drops the frame for that destination and is counted - it never blocks. Storage is
 * static (zero heap): DETWS_FWD_MAX_IFACES interfaces, DETWS_FWD_MAX_RULES rules.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_FORWARD_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_FORWARD_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_FORWARD

#include <stddef.h>
#include <stdint.h>

/** @brief Interface kind (informational; the plane treats all interfaces the same). */
enum det_if_kind
{
    DET_IF_OTHER = 0,
    DET_IF_WIFI_STA,
    DET_IF_WIFI_AP,
    DET_IF_ETH,
    DET_IF_BUS,
    DET_IF_RADIO,
};

/** @brief Rule action for a `(src, dst)` interface pair or an ACL entry. */
enum det_fwd_action
{
    DET_FWD_DENY = 0,
    DET_FWD_ALLOW = 1,
};

/** @brief Wildcard source interface for an ACL entry (matches a frame from any source). */
#define DET_FWD_IF_ANY 0xFF

/**
 * @brief Egress: emit @p len bytes on interface @p if_id.
 * @return true if the interface accepted the bytes; false drops (counted as a send fail).
 */
typedef bool (*det_if_send_fn)(uint8_t if_id, const uint8_t *data, uint16_t len, void *ctx);

/** @brief Forwarding counters (monotonic since the last det_forward_reset()). */
struct det_forward_stats
{
    uint32_t frames_in;    ///< ingress calls
    uint32_t forwarded;    ///< destination sends that succeeded
    uint32_t blocked;      ///< destinations refused by a DENY / default-deny
    uint32_t rate_dropped; ///< destinations dropped by a rate cap
    uint32_t send_fail;    ///< destination send callbacks that returned false
    uint32_t acl_denied;   ///< frames dropped at ingress by the access-control list
};

/** @brief Clear all interfaces, rules, and stats (start from empty). */
void det_forward_reset(void);

/**
 * @brief Register an interface and its egress send callback.
 * @return true; false if @p if_id is already registered, @p send is null, or the table
 *         is full (DETWS_FWD_MAX_IFACES).
 */
bool det_forward_add_if(uint8_t if_id, uint8_t kind, det_if_send_fn send, void *ctx);

/**
 * @brief Add a forwarding rule. @p rate_cap_per_sec caps ALLOW rules (0 = unlimited); it
 *        is ignored for DENY rules.
 * @return true; false if the table is full (DETWS_FWD_MAX_RULES).
 */
bool det_forward_add_rule(uint8_t src_if, uint8_t dst_if, uint8_t action, uint16_t rate_cap_per_sec);

/**
 * @brief Set the ACL default action - what happens to a frame that matches no ACL entry.
 *        Default DET_FWD_ALLOW (an empty ACL passes everything, so the ACL is opt-in);
 *        set DET_FWD_DENY for allowlist semantics (only explicitly permitted frames pass).
 */
void det_forward_acl_set_default(uint8_t action);

/**
 * @brief Add an ingress access-control entry (evaluated in add order, first match wins).
 *
 * A frame is matched when it arrived on @p src_if (or DET_FWD_IF_ANY) and its bytes at
 * `[offset, offset + patlen)` equal @p pattern under @p mask (each `byte & mask == pattern`).
 * @p patlen 0 matches any content (interface-only entry); a frame shorter than
 * `offset + patlen` does not match the entry (evaluation continues). The first matching
 * entry's @p action (permit/deny) decides; if none match, the ACL default applies.
 * Denied frames are dropped at ingress before any forwarding rule runs.
 * @return false if the ACL table is full or @p patlen exceeds DETWS_FWD_ACL_PATLEN.
 */
bool det_forward_acl_add(uint8_t src_if, uint16_t offset, const uint8_t *pattern, const uint8_t *mask, uint8_t patlen,
                         uint8_t action);

/**
 * @brief Forward a frame that arrived on @p src_if to every allowed destination.
 *
 * Evaluates the rules against each registered interface (never the source), applies the
 * per-rule rate cap, and calls the destination's send callback. Fail-closed.
 * @return the number of destinations the frame was successfully forwarded to.
 */
uint8_t det_forward_ingress(uint8_t src_if, const uint8_t *data, uint16_t len);

/** @brief Copy the current forwarding counters into @p out. */
void det_forward_get_stats(det_forward_stats *out);

#if !defined(ARDUINO)
/** @brief Host only: set the millisecond clock the rate cap uses (tests drive the window).
 *         On device the plane reads detws_millis(). */
void det_forward_test_set_now(uint32_t ms);
#endif

#endif // DETWS_ENABLE_FORWARD

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_FORWARD_H
