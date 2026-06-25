// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file syslog.cpp
 * @brief RFC 5424 syslog client: line formatter + UDP send.
 */

#include "services/syslog/syslog.h"

#if DETWS_ENABLE_SYSLOG

#include "network_drivers/transport/udp_transport.h"
#include <stdio.h>
#include <string.h>

// Configuration + format scratch - all in BSS, no heap.
static char g_server_ip[16]; // "255.255.255.255" + NUL
static uint16_t g_port = 514;
static char g_hostname[DETWS_SYSLOG_FIELD_MAX];
static char g_appname[DETWS_SYSLOG_FIELD_MAX];
static int g_facility = SYSLOG_FAC_LOCAL0;
static bool g_ready = false;
static char g_buf[DETWS_SYSLOG_MSG_MAX];

static void copy_field(char *dst, size_t cap, const char *src)
{
    if (!src || !src[0])
    {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

void syslog_init(const char *server_ip, uint16_t port, const char *hostname, const char *appname, int facility)
{
    copy_field(g_server_ip, sizeof(g_server_ip), server_ip);
    g_port = port;
    copy_field(g_hostname, sizeof(g_hostname), hostname);
    copy_field(g_appname, sizeof(g_appname), appname);
    g_facility = facility;
    g_ready = (g_server_ip[0] != '\0');
}

size_t syslog_format(char *out, size_t cap, int facility, int severity, const char *hostname, const char *appname,
                     const char *msg)
{
    if (!out || cap == 0)
        return 0;
    int pri = facility * 8 + severity;
    const char *h = (hostname && hostname[0]) ? hostname : "-";
    const char *a = (appname && appname[0]) ? appname : "-";
    // <PRI>VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP SD SP MSG
    // TIMESTAMP/PROCID/MSGID/STRUCTURED-DATA are NILVALUE ("-").
    int n = snprintf(out, cap, "<%d>1 - %s %s - - - %s", pri, h, a, msg ? msg : "");
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

bool syslog_log(int severity, const char *msg)
{
    if (!g_ready)
        return false;
    size_t n = syslog_format(g_buf, sizeof(g_buf), g_facility, severity, g_hostname, g_appname, msg);
    if (n == 0)
        return false;
    return det_udp_sendto(g_server_ip, g_port, (const uint8_t *)g_buf, n);
}

#endif // DETWS_ENABLE_SYSLOG
