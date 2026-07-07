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
 * behind DETWS_ENABLE_IPV6; the TCP/UDP listeners already bind IPADDR_TYPE_ANY, so the server
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
enum DetIpFamily
{
    DET_IP_NONE = 0, ///< empty / unparsed
    DET_IP_V4 = 4,   ///< IPv4 (bytes[0..3])
    DET_IP_V6 = 6,   ///< IPv6 (bytes[0..15])
};

/** @brief Address scope, in rough order of reachability (used for allow/deny policy + logging). */
enum DetIpScope
{
    DET_IP_SCOPE_UNSPECIFIED = 0, ///< 0.0.0.0 / ::
    DET_IP_SCOPE_LOOPBACK,        ///< 127.0.0.0/8 / ::1
    DET_IP_SCOPE_LINK_LOCAL,      ///< 169.254.0.0/16 / fe80::/10
    DET_IP_SCOPE_PRIVATE,         ///< RFC1918 (10/8, 172.16/12, 192.168/16) / ULA fc00::/7
    DET_IP_SCOPE_MULTICAST,       ///< 224.0.0.0/4 / ff00::/8
    DET_IP_SCOPE_GLOBAL,          ///< globally routable unicast
};

/** @brief A v4 or v6 address in network (big-endian) byte order. */
struct DetIp
{
    uint8_t family;    ///< DetIpFamily
    uint8_t bytes[16]; ///< network order; v4 uses the first 4
};

/** @brief Longest text an ::det_ip_format can produce, including the NUL (RFC 5952 v4-mapped). */
#define DET_IP_STR_MAX 46

/**
 * @brief Parse an IPv4 or IPv6 textual address (RFC 4291 §2.2) into @p out.
 * @return true on success (@p out->family set to DET_IP_V4/V6), false if @p s is malformed.
 */
bool det_ip_parse(const char *s, DetIp *out);

/**
 * @brief Format @p ip into @p out as its RFC 5952 canonical text.
 * @return the length written (excluding the NUL), or 0 if @p ip is empty or @p cap is too small
 *         (need up to ::DET_IP_STR_MAX).
 */
size_t det_ip_format(const DetIp *ip, char *out, size_t cap);

/** @brief Classify @p ip into a ::DetIpScope. */
DetIpScope det_ip_classify(const DetIp *ip);

/** @brief True if @p a and @p b are the same family and address. */
bool det_ip_equal(const DetIp *a, const DetIp *b);

/** @brief True if @p ip is an IPv4-mapped IPv6 address (::ffff:a.b.c.d, RFC 4291 §2.5.5.2). */
bool det_ip_is_v4_mapped(const DetIp *ip);

/**
 * @brief Build a v4 ::DetIp from four octets (a.b.c.d).
 */
DetIp det_ip_from_v4_octets(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

/**
 * @brief Build a v6 ::DetIp from 16 address bytes in network (big-endian) order.
 */
DetIp det_ip_from_v6_bytes(const uint8_t bytes[16]);

/**
 * @brief The v4 address as a big-endian (network-order) uint32 (a<<24 | b<<16 | c<<8 | d).
 * @return 0 if @p ip is not a v4 (or v4-mapped) address.
 */
uint32_t det_ip_to_v4_be(const DetIp *ip);

/** @brief True if @p ip is empty (DET_IP_NONE) or the all-zero unspecified address (0.0.0.0 / ::). */
bool det_ip_is_unspecified(const DetIp *ip);

/**
 * @brief CIDR containment: is @p addr inside the @p net / @p prefix_len block?
 *
 * The two must be the same family. @p prefix_len is 0..32 for v4, 0..128 for v6; the top
 * @p prefix_len bits of the address bytes must match @p net (a prefix of 0 matches everything).
 * This is the standard v4/v6 allowlist match.
 * @return true if @p addr is covered; false on a family mismatch or an out-of-range prefix.
 */
bool det_ip_prefix_match(const DetIp *addr, const DetIp *net, uint8_t prefix_len);

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_IP_H
