// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file pcap.h
 * @brief libpcap file framing - the classic global + per-record headers, link-type agnostic.
 *
 * One owner for the PCAP framing shared by every capture feature (Wi-Fi promiscuous capture,
 * CAN / bus listen-only capture, ...): each writes its frames with the matching DLT link type so
 * the forwarded stream is a valid `.pcap` a wired Wireshark / tcpdump opens directly. Header-only
 * and pure (little-endian byte writes, no heap, no stdlib), host-identical.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_PCAP_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_PCAP_H

#include <stddef.h>
#include <stdint.h>

/** @brief libpcap header sizes. */
#define DWS_PCAP_GLOBAL_HDR_LEN 24
#define DWS_PCAP_REC_HDR_LEN 16

/** @brief Common libpcap DLT link-layer types. */
#define DWS_DLT_IEEE802_11 105         ///< raw 802.11 (Wi-Fi promiscuous capture)
#define DWS_DLT_CAN_SOCKETCAN 227      ///< Linux SocketCAN classic/FD frames
#define DWS_DLT_ETHERNET 1             ///< IEEE 802.3 Ethernet
#define DWS_DLT_IEEE802_15_4_NOFCS 230 ///< raw 802.15.4 MAC frame, no FCS
#define DWS_DLT_IEEE802_15_4_TAP 283   ///< 802.15.4 with a TAP pseudo-header (RSSI / channel TLVs)

namespace detpcap_detail
{
inline void wr32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}
inline void wr16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
} // namespace detpcap_detail

/**
 * @brief Write the 24-byte libpcap global header (little-endian, microsecond timestamps).
 * @param linktype the DLT_* link type of the frames that follow (e.g. ::DWS_DLT_IEEE802_11).
 * @return ::DWS_PCAP_GLOBAL_HDR_LEN, or 0 if @p cap is too small.
 */
inline size_t dws_pcap_global_header(uint8_t *out, size_t cap, uint32_t linktype)
{
    if (!out || cap < DWS_PCAP_GLOBAL_HDR_LEN)
        return 0;
    detpcap_detail::wr32le(out + 0, 0xa1b2c3d4); // magic: usec timestamps, little-endian
    detpcap_detail::wr16le(out + 4, 2);          // version major
    detpcap_detail::wr16le(out + 6, 4);          // version minor
    detpcap_detail::wr32le(out + 8, 0);          // thiszone (GMT)
    detpcap_detail::wr32le(out + 12, 0);         // sigfigs
    detpcap_detail::wr32le(out + 16, 65535);     // snaplen
    detpcap_detail::wr32le(out + 20, linktype);  // network / DLT
    return DWS_PCAP_GLOBAL_HDR_LEN;
}

/**
 * @brief Write a 16-byte libpcap record header for one captured frame.
 * @return ::DWS_PCAP_REC_HDR_LEN, or 0 if @p cap is too small.
 */
inline size_t dws_pcap_record_header(uint8_t *out, size_t cap, uint32_t ts_sec, uint32_t ts_usec, uint32_t caplen,
                                     uint32_t origlen)
{
    if (!out || cap < DWS_PCAP_REC_HDR_LEN)
        return 0;
    detpcap_detail::wr32le(out + 0, ts_sec);
    detpcap_detail::wr32le(out + 4, ts_usec);
    detpcap_detail::wr32le(out + 8, caplen);
    detpcap_detail::wr32le(out + 12, origlen);
    return DWS_PCAP_REC_HDR_LEN;
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_PCAP_H
