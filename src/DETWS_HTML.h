// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DETWS_HTML.h
 * @brief HTML page templates and static string constants.
 *
 * Contains pre-defined HTML response strings for built-in services
 * (such as WiFi provisioning) to keep implementation files clean.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_HTML_H
#define DETERMINISTICESPASYNCWEBSERVER_HTML_H

/**
 * @brief HTML captive portal form for WiFi credentials entry.
 */
static const char DETWS_PROV_FORM[] =
    "<!DOCTYPE html><html><head><meta name=viewport content='width=device-width'>"
    "<title>WiFi setup</title></head><body><h2>WiFi setup</h2>"
    "<form method=POST action=/save>"
    "SSID:<br><input name=ssid maxlength=32><br>"
    "Password:<br><input name=psk type=password maxlength=63><br><br>"
    "<input type=submit value=Save></form></body></html>";

/**
 * @brief HTML success page served when credentials are saved and device is rebooting.
 */
static const char DETWS_PROV_SAVED_HTML[] =
    "<html><body>Saved. Rebooting...</body></html>";

#endif // DETERMINISTICESPASYNCWEBSERVER_HTML_H
