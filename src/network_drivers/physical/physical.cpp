// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file physical.cpp
 * @brief Layer 1 (Physical) - WiFi hardware initialization implementation.
 *
 * `init_wifi_physical()` kicks off the association process; it returns
 * immediately because the Arduino WiFi driver handles association
 * asynchronously.  Poll `wifi_ready()` in your setup() loop until it
 * returns true before calling `DetWebServer::begin()`.
 */

#include "physical.h"
#include <WiFi.h>

bool init_wifi_physical(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    return true;
}

bool wifi_ready()
{
    return WiFi.isConnected();
}
