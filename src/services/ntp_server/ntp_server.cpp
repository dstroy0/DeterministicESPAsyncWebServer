// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dws_ntp_server.cpp
 * @brief NTP server (RFC 5905 server mode) - implementation. See dws_ntp_server.h.
 */

#include "services/ntp_server/ntp_server.h"
#include "ServerConfig.h"

#if DWS_ENABLE_NTP_SERVER

#include <string.h> // memset, memcpy

namespace
{
void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}
} // namespace

size_t dws_ntp_server_build_response(const uint8_t *req, size_t req_len, uint8_t stratum, uint32_t refid,
                                     uint32_t dws_ntp_secs, uint32_t dws_ntp_frac, uint8_t *out, size_t out_cap)
{
    if (!req || !out || req_len < NTP_PACKET_LEN || out_cap < NTP_PACKET_LEN)
        return 0;

    // LI (2 bits) | VN (3 bits) | Mode (3 bits). Echo the client's version; reply as server (4).
    uint8_t vn = (uint8_t)((req[0] >> 3) & 0x7);
    memset(out, 0, NTP_PACKET_LEN);
    out[0] = (uint8_t)((0u << 6) | (vn << 3) | 4u); // LI = 0 (in sync), VN echoed, Mode = 4 (server)
    out[1] = stratum;
    out[2] = req[2] ? req[2] : 6; // poll interval: echo the client's, else 2^6 s
    out[3] = (uint8_t)(-6);       // precision: ~2^-6 s (16 ms), the clock's granularity
    // Root delay (4..7) stays 0; root dispersion (8..11) ~ 1 s to advertise a coarse clock.
    put_be32(out + 8, 0x00010000u);
    put_be32(out + 12, refid);
    put_be32(out + 16, dws_ntp_secs); // reference timestamp (when our clock was last good = now)
    put_be32(out + 20, dws_ntp_frac);
    memcpy(out + 24, req + 40, 8);    // origin timestamp = the client's transmit timestamp
    put_be32(out + 32, dws_ntp_secs); // receive timestamp
    put_be32(out + 36, dws_ntp_frac);
    put_be32(out + 40, dws_ntp_secs); // transmit timestamp
    put_be32(out + 44, dws_ntp_frac);
    return NTP_PACKET_LEN;
}

#if defined(ARDUINO)

#include "network_drivers/transport/udp.h"
#include "services/clock.h"
#include "services/time_source/time_source.h"

namespace
{
// All NTP-server binding state, owned by one instance (internal linkage): the advertised
// stratum and reference id, grouped so it is one named owner, unreachable cross-TU.
struct NtpServerCtx
{
    uint8_t stratum = DWS_NTP_SERVER_STRATUM;
    uint32_t refid = NTP_REFID_LOCL;
};
NtpServerCtx s_ntp;

// UDP handler: answer each request from the current time (silent if we have none).
void dws_ntp_server_udp_handler(const uint8_t *data, size_t len, struct DWSUdpPeer *peer, void *ctx)
{
    (void)ctx;
    uint32_t unix_secs = dws_time_now();
    if (unix_secs == 0) // no valid time - do not serve a wrong clock
        return;
    // Sub-second fraction from the monotonic ms clock (best-effort; not phase-locked to the
    // 1 Hz second boundary, so the sub-second component is approximate on this class of clock).
    uint32_t frac = (uint32_t)(((uint64_t)(dws_millis() % 1000u) << 32) / 1000u);

    uint8_t resp[NTP_PACKET_LEN];
    size_t n = dws_ntp_server_build_response(data, len, s_ntp.stratum, s_ntp.refid, unix_secs + NTP_UNIX_OFFSET, frac,
                                             resp, sizeof(resp));
    if (n)
        dws_udp_send(peer, resp, n);
}
} // namespace

bool dws_ntp_server_begin(uint8_t stratum, uint32_t refid)
{
    s_ntp.stratum = stratum;
    s_ntp.refid = refid;
    return dws_udp_listen(123, dws_ntp_server_udp_handler, nullptr);
}

#else // host build: no lwIP. The codec above is host-tested; the binding is a stub.

bool dws_ntp_server_begin(uint8_t, uint32_t)
{
    return false;
}

#endif // ARDUINO

#endif // DWS_ENABLE_NTP_SERVER
