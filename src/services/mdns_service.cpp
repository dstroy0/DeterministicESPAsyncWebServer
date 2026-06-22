// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mdns_service.cpp
 * @brief mDNS / DNS-SD advertisement implementation (DETWS_ENABLE_MDNS).
 */

#include "mdns_service.h"

#if DETWS_ENABLE_MDNS && defined(ARDUINO)

#include <ESPmDNS.h>

bool detws_mdns_begin(const char *hostname, uint16_t http_port)
{
    if (!hostname || hostname[0] == '\0')
        return false;
    if (!MDNS.begin(hostname))
        return false;
    MDNS.addService("http", "tcp", http_port);
    return true;
}

#else

bool detws_mdns_begin(const char *hostname, uint16_t http_port)
{
    (void)hostname;
    (void)http_port;
    return false; // mDNS disabled at compile time (or non-Arduino build)
}

#endif // DETWS_ENABLE_MDNS && ARDUINO
