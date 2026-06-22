// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file physical.h
 * @brief Layer 1 (Physical) - WiFi radio initialization and link-state query.
 *
 * Wraps the Arduino WiFi library into the OSI-layered interface used by
 * this library.  On ESP32, the "physical" link is the 802.11 radio managed
 * by the Espressif WiFi stack (not a discrete PHY chip).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H
#define DETERMINISTICESPASYNCWEBSERVER_PHYSICAL_H

#include <Arduino.h>

/**
 * @brief Connect to a WiFi access point.
 *
 * Calls `WiFi.begin()` and returns immediately; it does not block waiting
 * for the association to complete.  Poll wifi_ready() to check link status.
 *
 * @param ssid     Network SSID (null-terminated).
 * @param password WPA2 passphrase (null-terminated).
 * @return Always returns `true` (WiFi.begin() is fire-and-forget).
 */
bool init_wifi_physical(const char *ssid, const char *password);

/**
 * @brief Query whether the WiFi link is up.
 *
 * @return `true` if associated with an AP and an IP address is assigned.
 */
bool wifi_ready();

#endif
