// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mdns_adaptive.h
 * @brief Adaptive mDNS beacon scheduling: RF-aware backoff, TTL refresher, auto-sleep beacon
 *        (DETWS_ENABLE_MDNS_ADAPTIVE).
 *
 * The mDNS service (shipped) announces records with a TTL; caches on the network evict a record when its
 * TTL lapses, so a device must re-announce to stay discoverable. Two pressures shape *when* to announce:
 *
 *  - **Crowded RF**: on a busy 2.4 GHz channel, hammering announces just adds collisions. So back the
 *    announce interval off (toward a ceiling) when contention is high, and recover it toward the nominal
 *    cadence when the air is quiet.
 *  - **A continuous refresher**: re-announce at ~half the record TTL (RFC 6762 cache eviction is at TTL)
 *    so caches never lapse in steady state.
 *  - **Auto-sleep beacons**: before entering a sleep window that would run past the next refresh, announce
 *    *now* so the record survives the sleep instead of lapsing while the radio is off.
 *
 * These are the pure scheduling decisions - what interval, and is an announce due (incl. before a sleep).
 * The app owns the actual mDNS transmit. Wrap-safe time math, no heap, no stdlib, host-testable.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MDNS_ADAPTIVE_H
#define DETERMINISTICESPASYNCWEBSERVER_MDNS_ADAPTIVE_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_MDNS_ADAPTIVE

/** @brief Adaptive beacon state. */
struct MdnsBeacon
{
    uint32_t base_ms;   ///< nominal refresh cadence (e.g. TTL/2), and the backoff floor.
    uint32_t max_ms;    ///< backoff ceiling under heavy contention.
    uint32_t cur_ms;    ///< current adaptive interval.
    uint16_t hi_thresh; ///< contention count at/above which the interval backs off.
};

/** @brief The continuous-refresher cadence for a record TTL: half the TTL, in milliseconds. */
uint32_t det_mdns_refresh_interval(uint32_t ttl_s);

/** @brief Initialize a beacon. @p cur_ms starts at @p base_ms. */
void det_mdns_beacon_init(MdnsBeacon *b, uint32_t base_ms, uint32_t max_ms, uint16_t hi_thresh);

/**
 * @brief Adapt the interval to observed RF contention (announces/collisions seen in the last window).
 *        contention >= hi_thresh doubles the interval (capped at max_ms); contention == 0 halves it back
 *        toward base_ms; moderate contention holds.
 * @return the new interval (ms).
 */
uint32_t det_mdns_beacon_adapt(MdnsBeacon *b, uint16_t contention);

/** @brief Is an announce due now? (wrap-safe: elapsed since @p last_ms >= the current interval). */
bool det_mdns_beacon_due(const MdnsBeacon *b, uint32_t last_ms, uint32_t now_ms);

/**
 * @brief Auto-sleep beacon: should we announce *before* sleeping for @p sleep_ms?
 *        True when the elapsed-since-last plus the sleep would meet/exceed the interval - i.e. the record
 *        would lapse mid-sleep - so we refresh proactively before the radio goes off.
 */
bool det_mdns_beacon_presleep_due(const MdnsBeacon *b, uint32_t last_ms, uint32_t now_ms, uint32_t sleep_ms);

#endif // DETWS_ENABLE_MDNS_ADAPTIVE
#endif // DETERMINISTICESPASYNCWEBSERVER_MDNS_ADAPTIVE_H
