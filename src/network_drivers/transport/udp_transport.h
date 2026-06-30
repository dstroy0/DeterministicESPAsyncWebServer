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
#include "shared_primitives/shim.h"

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

/**
 * @brief Copy a received peer's source IPv4 address (dotted-quad) and port out.
 *
 * The DetUdpPeer token is valid only inside the handler; a service that wants to
 * message the peer later (e.g. CoAP Observe notifications) captures its address
 * here and sends with det_udp_listener_sendto(). @return false on a host build or
 * a too-small buffer.
 */
bool det_udp_peer_addr(const struct DetUdpPeer *peer, char *ip_out, size_t ip_cap, uint16_t *port_out);

/**
 * @brief Send a datagram from the listener bound on @p listen_port to an address.
 *
 * Unlike det_udp_sendto() (a shared ephemeral source port), this uses the bound
 * listener's PCB, so the datagram's source is @p listen_port - required when a
 * peer matches replies by the server endpoint (CoAP Observe notifications come
 * from :5683). @return false if no listener is bound on @p listen_port.
 */
bool det_udp_listener_sendto(uint16_t listen_port, const char *dst_ip, uint16_t dst_port, const uint8_t *data,
                             size_t len);

#if !defined(ARDUINO)
// Host-only test hooks: capture the last datagram passed to det_udp_sendto() so a
// unit test can inspect what an outbound UDP service (SNMP trap/inform, syslog,
// telemetry) actually built. Off by default; enable per test. Mirrors the
// tcp_capture() seam in the lwIP mock.
void det_udp_capture_enable();
void det_udp_capture_reset();
const uint8_t *det_udp_captured(); ///< bytes of the last captured datagram (nullptr if none)
size_t det_udp_captured_len();     ///< length of the last captured datagram
#endif

#endif // DETERMINISTICESPASYNCWEBSERVER_UDP_TRANSPORT_H
