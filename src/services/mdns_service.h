// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mdns_service.h
 * @brief Optional mDNS / DNS-SD advertisement (DETWS_ENABLE_MDNS).
 *
 * Thin wrapper over the ESP32 ESPmDNS library so a headless device is reachable
 * at `<hostname>.local` and advertises an `_http._tcp` service for browsers and
 * service-discovery tools. Compiled to a no-op stub when DETWS_ENABLE_MDNS is 0
 * or on non-Arduino builds, so it costs nothing unless enabled.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MDNS_SERVICE_H
#define DETERMINISTICESPASYNCWEBSERVER_MDNS_SERVICE_H

#include "DetWebServerConfig.h"
#include <stdint.h>

/**
 * @brief Start mDNS responder and advertise an HTTP service.
 *
 * Call once after the WiFi link is up (wifi_ready()) and after begin(). The
 * device becomes reachable at `<hostname>.local` and advertises
 * `_http._tcp` on @p http_port.
 *
 * @param hostname   Host label without the `.local` suffix (e.g. "mydevice").
 * @param http_port  TCP port the HTTP server listens on (default 80).
 * @return true if the responder started; false if disabled at compile time,
 *         not on Arduino, or ESPmDNS failed to start.
 */
bool detws_mdns_begin(const char *hostname, uint16_t http_port = 80);

#endif // DETERMINISTICESPASYNCWEBSERVER_MDNS_SERVICE_H
