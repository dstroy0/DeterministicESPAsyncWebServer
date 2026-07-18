// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file syslog.cpp
 * @brief RFC 5424 syslog client: line formatter + UDP send.
 */

#include "services/syslog/syslog.h"

#if DWS_ENABLE_SYSLOG

#include "network_drivers/transport/udp.h"
#include <stdio.h>
#include <string.h>

// All syslog client state, owned by one instance (internal linkage): the collector endpoint,
// the host/app identity, the facility, the ready flag, and the format scratch (all BSS, no
// heap), grouped so it is one named owner, unreachable from any other translation unit.
struct SyslogCtx
{
    char server_ip[16] = {0}; // "255.255.255.255" + NUL
    uint16_t port = DWS_SYSLOG_DEFAULT_PORT;
    char hostname[DWS_SYSLOG_FIELD_MAX] = {0};
    char appname[DWS_SYSLOG_FIELD_MAX] = {0};
    SyslogFacility facility = SyslogFacility::SYSLOG_FAC_LOCAL0;
    bool ready = false;
    char buf[DWS_SYSLOG_MSG_MAX];
};
static SyslogCtx s_syslog;

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

void syslog_init(const char *server_ip, uint16_t port, const char *hostname, const char *appname,
                 SyslogFacility facility)
{
    copy_field(s_syslog.server_ip, sizeof(s_syslog.server_ip), server_ip);
    s_syslog.port = port;
    copy_field(s_syslog.hostname, sizeof(s_syslog.hostname), hostname);
    copy_field(s_syslog.appname, sizeof(s_syslog.appname), appname);
    s_syslog.facility = facility;
    s_syslog.ready = (s_syslog.server_ip[0] != '\0');
}

size_t syslog_format(char *out, size_t cap, SyslogFacility facility, SyslogSeverity severity, const char *hostname,
                     const char *appname, const char *msg)
{
    if (!out || cap == 0)
        return 0;
    // RFC 5424 6.2.1: PRIVAL = facility*8 + severity, range 0..191. The numeric priority is the wire
    // encoding of the level pair, so cast to int here (the boundary). facility/severity are unsigned
    // enums, so PRI is never negative; clamp only the top so an over-range value cast in by the caller
    // can never emit a malformed PRI (e.g. <400>).
    int pri = (int)facility * 8 + (int)severity;
    if (pri > 191)
        pri = 191;
    const char *h = (hostname && hostname[0]) ? hostname : "-";
    const char *a = (appname && appname[0]) ? appname : "-";
    // <PRI>VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID SP SD SP MSG
    // TIMESTAMP/PROCID/MSGID/STRUCTURED-DATA are NILVALUE ("-").
    int n = snprintf(out, cap, "<%d>1 - %s %s - - - %s", pri, h, a, msg ? msg : "");
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

bool syslog_log(SyslogSeverity severity, const char *msg)
{
    if (!s_syslog.ready)
        return false;
    size_t n = syslog_format(s_syslog.buf, sizeof(s_syslog.buf), s_syslog.facility, severity, s_syslog.hostname,
                             s_syslog.appname, msg);
    if (n == 0)
        return false;
    return dws_udp_sendto(s_syslog.server_ip, s_syslog.port, (const uint8_t *)s_syslog.buf, n);
}

#endif // DWS_ENABLE_SYSLOG
