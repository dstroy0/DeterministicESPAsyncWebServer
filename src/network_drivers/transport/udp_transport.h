// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file udp_transport.h
 * @brief Layer 4 (Transport) - connectionless UDP datagram service.
 *
 * The transport layer's UDP counterpart to the TCP connection pool (see
 * transport.h / listener.h). It owns *all* lwIP UDP plumbing (`udp_pcb`,
 * `pbuf`); higher layers (application services such as SNMP and the captive
 * portal's DNS responder) bind a port and exchange datagrams through this API
 * without ever touching lwIP. This keeps the OSI layering honest: transport
 * concerns stay in the transport layer.
 *
 * Datagram delivery is callback-driven from the lwIP thread (no per-loop
 * polling). The handler receives a contiguous payload and an opaque peer token
 * valid only for the duration of the call; reply synchronously with
 * det_udp_send(). On non-Arduino (host) builds the functions are no-op stubs so
 * the services that use them stay host-compilable.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_UDP_TRANSPORT_H
#define DETERMINISTICESPASYNCWEBSERVER_UDP_TRANSPORT_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque sender of a received datagram.
 *
 * Wraps the lwIP source address / port / PCB. Valid only inside the
 * DetUdpHandler call; pass it to det_udp_send() to reply. The concrete layout
 * lives in udp_transport.cpp so no lwIP type escapes the transport layer.
 */
struct DetUdpPeer;

/**
 * @brief Datagram handler invoked for each received UDP packet.
 *
 * @param data  contiguous datagram payload (transport-owned scratch).
 * @param len   payload length in bytes.
 * @param peer  reply token (valid only during this call).
 * @param ctx   the opaque context passed to det_udp_listen().
 */
typedef void (*DetUdpHandler)(const uint8_t *data, size_t len, struct DetUdpPeer *peer, void *ctx);

/**
 * @brief Bind a UDP port and route incoming datagrams to @p handler.
 *
 * @param port     UDP port to bind (e.g. 161 for SNMP, 53 for captive DNS).
 * @param handler  callback for each datagram.
 * @param ctx      opaque pointer forwarded to @p handler.
 * @return true on success; false if no listener slot is free, the bind fails,
 *         or UDP is unavailable (host builds). ESP32 only; a host build returns false.
 */
bool det_udp_listen(uint16_t port, DetUdpHandler handler, void *ctx);

/**
 * @brief Send a datagram back to the peer captured during the handler call.
 *
 * @param peer  the token handed to the DetUdpHandler.
 * @param data  payload bytes.
 * @param len   payload length.
 * @return true if the datagram was queued for transmission.
 */
bool det_udp_send(struct DetUdpPeer *peer, const uint8_t *data, size_t len);

/**
 * @brief Send a UDP datagram to an arbitrary destination (outbound client).
 *
 * Unlike det_udp_send() - which replies to the peer of a received datagram -
 * this sends to a host given as a dotted-quad IPv4 string and port, using a
 * single shared outbound PCB. Fire-and-forget; for clients such as the syslog
 * sender. ESP32 only; a host build returns false.
 *
 * @param dst_ip   destination IPv4 address (e.g. "192.168.1.10").
 * @param dst_port destination UDP port.
 * @param data     payload bytes.
 * @param len      payload length.
 * @return true if the datagram was queued; false on a bad address, allocation
 *         failure, or a host build.
 */
bool det_udp_sendto(const char *dst_ip, uint16_t dst_port, const uint8_t *data, size_t len);

#endif // DETERMINISTICESPASYNCWEBSERVER_UDP_TRANSPORT_H
