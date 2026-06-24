// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file html.h
 * @brief Layer 7 (Application) - embedded HTML page assets served by built-in services.
 *
 * Static, self-contained HTML/JS pages that ship inside the firmware (no
 * filesystem needed): the captive-portal WiFi form and the WebSocket terminal.
 * Declared here and defined once in html.cpp (so multiple services can serve the
 * same asset without duplicate definitions). On ESP32 these const arrays live in
 * flash (DROM) and are read directly.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_HTML_H
#define DETERMINISTICESPASYNCWEBSERVER_HTML_H

/** @brief Captive-portal form for WiFi credential entry (provisioning service). */
extern const char DETWS_PROV_FORM[];

/** @brief Success page shown after credentials are saved and the device reboots. */
extern const char DETWS_PROV_SAVED_HTML[];

/**
 * @brief Self-contained WebSocket terminal page (green-phosphor CRT theme).
 *
 * Auto-selects `ws://` / `wss://` from its own scheme (TLS-agnostic); includes
 * input history, autoscroll, and reconnect. Served by the web-terminal service.
 */
extern const char DETWS_TERMINAL_PAGE[];

#endif // DETERMINISTICESPASYNCWEBSERVER_HTML_H
