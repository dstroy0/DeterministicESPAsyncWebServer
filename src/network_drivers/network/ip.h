// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ip.h
 * @brief Layer 3 (Network) - a family-tagged IP address (IPv4 or IPv6) with RFC-faithful
 *        text parsing, canonical formatting, and scope classification.
 *
 * One representation for both address families so the rest of the stack can carry a peer
 * address without caring whether it is v4 or v6. The address bytes are stored in network
 * (big-endian, left-to-right) order: a v4 address uses bytes[0..3], a v6 address bytes[0..15].
 *
 * Pure and host-testable - no lwIP, no Arduino, no heap, no stdlib parsing. The parser
 * implements RFC 4291 §2.2 text forms (dotted-quad v4; v6 with `::` zero-compression and the
 * embedded-v4 `::ffff:a.b.c.d` tail); the formatter emits the RFC 5952 canonical form
 * (lower-case, no leading zeros, the longest zero run compressed to `::`, v4-mapped shown as
 * dotted). ESP32 dual-stack bring-up (enabling IPv6 on the netif) lives in the physical layer
 * behind DWS_ENABLE_IPV6; the TCP/UDP listeners already bind IPADDR_TYPE_ANY, so the server
 * accepts v6 connections the moment the interface has a v6 address.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_IP_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_IP_H

#include <stddef.h>
#include <stdint.h>

/** @brief Address family tag. */
enum class DWSIpFamily : uint8_t
{
    DWS_IP_NONE = 0, ///< empty / unparsed
    DWS_IP_V4 = 4,   ///< IPv4 (bytes[0..3])
    DWS_IP_V6 = 6,   ///< IPv6 (bytes[0..15])
};

/** @brief Address scope, in rough order of reachability (used for allow/deny policy + logging). */
enum class DWSIpScope : uint8_t
{
    DWS_IP_SCOPE_UNSPECIFIED = 0, ///< 0.0.0.0 / ::
    DWS_IP_SCOPE_LOOPBACK,        ///< 127.0.0.0/8 / ::1
    DWS_IP_SCOPE_LINK_LOCAL,      ///< 169.254.0.0/16 / fe80::/10
    DWS_IP_SCOPE_PRIVATE,         ///< RFC1918 (10/8, 172.16/12, 192.168/16) / ULA fc00::/7
    DWS_IP_SCOPE_MULTICAST,       ///< 224.0.0.0/4 / ff00::/8
    DWS_IP_SCOPE_GLOBAL,          ///< globally routable unicast
};

/** @brief A v4 or v6 address in network (big-endian) byte order. */
struct DWSIp
{
    DWSIpFamily family; ///< address family tag
    uint8_t bytes[16];  ///< network order; v4 uses the first 4
};

/** @brief Longest text an ::dws_ip_format can produce, including the NUL (RFC 5952 v4-mapped). */
#define DWS_IP_STR_MAX 46

/**
 * @brief Parse an IPv4 or IPv6 textual address (RFC 4291 §2.2) into @p out.
 * @return true on success (@p out->family set to DWSIpFamily::DWS_IP_V4/V6), false if @p s is malformed.
 */
bool dws_ip_parse(const char *s, DWSIp *out);

/**
 * @brief Format @p ip into @p out as its RFC 5952 canonical text.
 * @return the length written (excluding the NUL), or 0 if @p ip is empty or @p cap is too small
 *         (need up to ::DWS_IP_STR_MAX).
 */
size_t dws_ip_format(const DWSIp *ip, char *out, size_t cap);

/** @brief Classify @p ip into a ::DWSIpScope. */
DWSIpScope dws_ip_classify(const DWSIp *ip);

/** @brief True if @p a and @p b are the same family and address. */
bool dws_ip_equal(const DWSIp *a, const DWSIp *b);

/** @brief True if @p ip is an IPv4-mapped IPv6 address (::ffff:a.b.c.d, RFC 4291 §2.5.5.2). */
bool dws_ip_is_v4_mapped(const DWSIp *ip);

/**
 * @brief Build a v4 ::DWSIp from four octets (a.b.c.d).
 */
DWSIp dws_ip_from_v4_octets(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

/**
 * @brief Build a v6 ::DWSIp from 16 address bytes in network (big-endian) order.
 */
DWSIp dws_ip_from_v6_bytes(const uint8_t bytes[16]);

/**
 * @brief The v4 address as a big-endian (network-order) uint32 (a<<24 | b<<16 | c<<8 | d).
 * @return 0 if @p ip is not a v4 (or v4-mapped) address.
 */
uint32_t dws_ip_to_v4_be(const DWSIp *ip);

/** @brief True if @p ip is empty (DWSIpFamily::DWS_IP_NONE) or the all-zero unspecified address (0.0.0.0 / ::). */
bool dws_ip_is_unspecified(const DWSIp *ip);

/**
 * @brief CIDR containment: is @p addr inside the @p net / @p prefix_len block?
 *
 * The two must be the same family. @p prefix_len is 0..32 for v4, 0..128 for v6; the top
 * @p prefix_len bits of the address bytes must match @p net (a prefix of 0 matches everything).
 * This is the standard v4/v6 allowlist match.
 * @return true if @p addr is covered; false on a family mismatch or an out-of-range prefix.
 */
bool dws_ip_prefix_match(const DWSIp *addr, const DWSIp *net, uint8_t prefix_len);

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_IP_H
