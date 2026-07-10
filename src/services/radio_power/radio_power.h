// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file radio_power.h
 * @brief WiFi radio power controls (DETWS_ENABLE_RADIO_POWER).
 *
 * Applies the WiFi modem-sleep mode (DETWS_RADIO_WIFI_PS) and an optional max-TX
 * cap (DETWS_RADIO_MAX_TX_DBM) in one call - trade throughput/latency for lower
 * average power on a battery device. The mode names are pure/host-tested; the
 * apply + readback use esp_wifi on ESP32 (no-ops on host).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_RADIO_POWER_H
#define DETERMINISTICESPASYNCWEBSERVER_RADIO_POWER_H

#include "ServerConfig.h"
#include <stdint.h>

#if DETWS_ENABLE_RADIO_POWER

/** @brief Modem-sleep modes (match DETWS_RADIO_WIFI_PS). */
enum
{
    DETWS_PS_NONE = 0,      ///< no modem sleep (max performance).
    DETWS_PS_MIN_MODEM = 1, ///< wake at every DTIM (balanced).
    DETWS_PS_MAX_MODEM = 2, ///< wake at a listen interval (lowest power, higher latency).
};

/** @brief Name for a modem-sleep mode ("none" / "min_modem" / "max_modem"). */
const char *detws_radio_ps_name(uint8_t mode);

/** @brief Apply DETWS_RADIO_WIFI_PS (+ TX cap) to the radio. No-op on host. */
void detws_radio_power_apply(void);

/** @brief Current modem-sleep mode read back from the radio (DETWS_PS_* ; 0 on host). */
uint8_t detws_radio_ps_get(void);

/**
 * @brief Hold the radio awake for the duration of a bulk transfer (reference-counted).
 *
 * The first hold forces modem sleep off (WIFI_PS_NONE) so a long transfer is not interrupted by DTIM
 * wakeups; the matching release, once the count returns to zero, restores the configured
 * DETWS_RADIO_WIFI_PS mode. Balance every @ref detws_radio_busy_hold with exactly one
 * @ref detws_radio_busy_release. The relay/DNAT listener holds one while any bridge is active; other
 * bulk paths (large file serves, streaming PUT) can do the same. No-op on host.
 */
void detws_radio_busy_hold(void);

/** @brief Release a bulk-transfer hold; restores the configured modem-sleep mode at zero. No-op on host. */
void detws_radio_busy_release(void);

#endif // DETWS_ENABLE_RADIO_POWER
#endif // DETERMINISTICESPASYNCWEBSERVER_RADIO_POWER_H
