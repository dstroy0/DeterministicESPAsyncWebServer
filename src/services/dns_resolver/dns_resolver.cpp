// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dns_resolver.cpp
 * @brief IPv4 classifier / verifier (pure) + lwIP DNS resolve (ESP32).
 *
 * The resolve marshals dns_gethostbyname into tcpip_thread and polls a done flag
 * with a deadline - the same cross-thread pattern the http_client uses.
 */

#include "services/dns_resolver/dns_resolver.h"

#if DETWS_ENABLE_DNS_RESOLVER

DetwsIpClass detws_dns_classify(uint32_t ip)
{
    if (ip == 0u)
        return DETWS_IP_UNSPECIFIED;
    if (ip == 0xFFFFFFFFu)
        return DETWS_IP_BROADCAST;
    uint8_t a = (uint8_t)((ip >> 24) & 0xFF);
    uint8_t b = (uint8_t)((ip >> 16) & 0xFF);
    if (a == 127)
        return DETWS_IP_LOOPBACK;
    if (a == 10)
        return DETWS_IP_PRIVATE;
    if (a == 172 && b >= 16 && b <= 31)
        return DETWS_IP_PRIVATE;
    if (a == 192 && b == 168)
        return DETWS_IP_PRIVATE;
    if (a == 169 && b == 254)
        return DETWS_IP_LINKLOCAL;
    if (a >= 224 && a <= 239)
        return DETWS_IP_MULTICAST;
    return DETWS_IP_PUBLIC;
}

bool detws_dns_verify(uint32_t ip)
{
    switch (detws_dns_classify(ip))
    {
    case DETWS_IP_UNSPECIFIED: // 0.0.0.0 - blocked / no answer
    case DETWS_IP_BROADCAST:   // 255.255.255.255 - never a host
    case DETWS_IP_LOOPBACK:    // 127.x - DNS-rebinding to localhost
    case DETWS_IP_MULTICAST:   // 224-239 - never an A-record host
        return false;
    default:
        return true; // private / link-local / public are plausible
    }
}

#ifdef ARDUINO

#include "lwip/def.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/priv/tcpip_priv.h"
#include "services/det_clock.h" // detws_millis() - the single pluggable monotonic source
#include <Arduino.h>

namespace
{
ip_addr_t s_addr;
volatile bool s_done = false;
volatile bool s_ok = false;

struct DnsCall
{
    struct tcpip_api_call_data base;
    const char *host;
};

void dns_cb(const char *name, const ip_addr_t *addr, void *arg)
{
    (void)name;
    (void)arg;
    if (addr)
    {
        s_addr = *addr;
        s_ok = true;
    }
    s_done = true;
}

err_t do_dns(struct tcpip_api_call_data *c)
{
    const char *host = ((DnsCall *)c)->host;
    err_t e = dns_gethostbyname(host, &s_addr, dns_cb, nullptr);
    if (e == ERR_OK) // already cached
    {
        s_ok = true;
        s_done = true;
    }
    else if (e != ERR_INPROGRESS) // hard failure
        s_done = true;
    return ERR_OK;
}

uint32_t to_host_order(const ip_addr_t *a)
{
    return lwip_ntohl(ip4_addr_get_u32(ip_2_ip4(a)));
}
} // namespace

bool detws_dns_resolve(const char *host, uint32_t *out_ip)
{
    if (!host || !out_ip)
        return false;

    ip_addr_t literal;
    if (ipaddr_aton(host, &literal)) // dotted-quad fast path, no DNS
    {
        *out_ip = to_host_order(&literal);
        return true;
    }

    s_done = false;
    s_ok = false;
    DnsCall k;
    memset(&k, 0, sizeof(k));
    k.host = host;
    tcpip_api_call(do_dns, &k.base); // resolve in the lwIP thread

    uint32_t deadline = detws_millis() + DETWS_DNS_TIMEOUT_MS;
    while (!s_done && (int32_t)(deadline - detws_millis()) > 0)
        delay(5);

    if (!s_ok)
        return false;
    *out_ip = to_host_order(&s_addr);
    return true;
}

#else // host build - no resolver

bool detws_dns_resolve(const char *, uint32_t *)
{
    return false;
}

#endif // ARDUINO

bool detws_dns_resolve_verified(const char *host, uint32_t *out_ip)
{
    uint32_t ip = 0;
    if (!detws_dns_resolve(host, &ip))
        return false;
    if (!detws_dns_verify(ip))
        return false;
    if (out_ip)
        *out_ip = ip;
    return true;
}

#endif // DETWS_ENABLE_DNS_RESOLVER
