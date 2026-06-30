// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file coap.h
 * @brief Zero-heap CoAP server (RFC 7252): message codec + a fixed resource table.
 *
 * The server is split into a pure, host-testable core and an ESP32-only UDP
 * transport (mirroring the SNMP agent's split):
 *
 *  - coap_server_process() takes a complete request datagram and produces a
 *    complete response datagram in a caller buffer - no sockets, no heap. It is
 *    unit-tested on the host (env:native_coap).
 *  - coap_server_begin_udp() binds the transport-layer UDP service on :5683
 *    (Arduino only) and feeds received datagrams through coap_server_process().
 *
 * Only the message layer's piggybacked-response model is implemented: a CON
 * request is answered with a piggybacked ACK, a NON request with a NON response.
 * Separate responses, retransmission/deduplication and /.well-known/core
 * discovery are out of scope (a deterministic, constrained server typically
 * replies in-line). The codec understands the Uri-Path, Uri-Query and
 * Content-Format options; other options are skipped. Block-wise transfer
 * (RFC 7959, the Block1/Block2 options) is available under DETWS_ENABLE_COAP_BLOCK;
 * resource observation (RFC 7641) under DETWS_ENABLE_COAP_OBSERVE.
 *
 * The resource table is a fixed BSS array of DETWS_COAP_MAX_RESOURCES entries.
 * Register handlers with coap_server_add_resource(); the path string is
 * referenced by pointer and must outlive the server (point it at flash/static
 * data, like the rest of the library's strings).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_COAP_H
#define DETERMINISTICESPASYNCWEBSERVER_COAP_H

#include "shared_primitives/shim.h"

#if DETWS_ENABLE_COAP

// CoAP message types (RFC 7252 §3, the 2-bit T field).
enum CoapType
{
    COAP_TYPE_CON = 0, ///< Confirmable (answered with a piggybacked ACK).
    COAP_TYPE_NON = 1, ///< Non-confirmable (answered with a NON response).
    COAP_TYPE_ACK = 2, ///< Acknowledgement.
    COAP_TYPE_RST = 3, ///< Reset (rejects a message; sent for a malformed/empty CON).
};

// CoAP request method codes (class 0; the on-wire Code byte's detail field).
enum CoapMethod
{
    COAP_GET = 1,
    COAP_POST = 2,
    COAP_PUT = 3,
    COAP_DELETE = 4,
};

// Allowed-methods bitmask helpers for coap_server_add_resource() (bit per method).
enum CoapMethodMask
{
    COAP_ALLOW_GET = 1u << COAP_GET,       ///< 0x02
    COAP_ALLOW_POST = 1u << COAP_POST,     ///< 0x04
    COAP_ALLOW_PUT = 1u << COAP_PUT,       ///< 0x08
    COAP_ALLOW_DELETE = 1u << COAP_DELETE, ///< 0x10
};

/** @brief Build a CoAP response Code byte from its class and detail (e.g. COAP_CODE(2,5) = 2.05). */
#define COAP_CODE(c, dd) ((uint8_t)(((c) << 5) | ((dd) & 0x1F)))

// Common CoAP response codes (RFC 7252 §5.9; 2.31 / 4.08 / 4.13 from RFC 7959).
enum CoapResponseCode
{
    COAP_RSP_CREATED = COAP_CODE(2, 1),            ///< 2.01
    COAP_RSP_DELETED = COAP_CODE(2, 2),            ///< 2.02
    COAP_RSP_VALID = COAP_CODE(2, 3),              ///< 2.03
    COAP_RSP_CHANGED = COAP_CODE(2, 4),            ///< 2.04
    COAP_RSP_CONTENT = COAP_CODE(2, 5),            ///< 2.05
    COAP_RSP_CONTINUE = COAP_CODE(2, 31),          ///< 2.31 (block-wise: more Block1 blocks expected)
    COAP_RSP_BAD_REQUEST = COAP_CODE(4, 0),        ///< 4.00
    COAP_RSP_BAD_OPTION = COAP_CODE(4, 2),         ///< 4.02
    COAP_RSP_NOT_FOUND = COAP_CODE(4, 4),          ///< 4.04
    COAP_RSP_METHOD_NOT_ALLOWED = COAP_CODE(4, 5), ///< 4.05
    COAP_RSP_NOT_ACCEPTABLE = COAP_CODE(4, 6),     ///< 4.06
    COAP_RSP_REQUEST_INCOMPLETE = COAP_CODE(4, 8), ///< 4.08 (block-wise: out-of-order / lost Block1)
    COAP_RSP_REQUEST_TOO_LARGE = COAP_CODE(4, 13), ///< 4.13 (block-wise: reassembly buffer exceeded)
    COAP_RSP_INTERNAL_ERROR = COAP_CODE(5, 0),     ///< 5.00
    COAP_RSP_NOT_IMPLEMENTED = COAP_CODE(5, 1),    ///< 5.01
};

// CoAP Content-Format identifiers (RFC 7252 §12.3). COAP_CF_NONE means "absent".
enum CoapContentFormat
{
    COAP_CF_TEXT = 0,      ///< text/plain;charset=utf-8
    COAP_CF_LINK = 40,     ///< application/link-format
    COAP_CF_XML = 41,      ///< application/xml
    COAP_CF_OCTET = 42,    ///< application/octet-stream
    COAP_CF_JSON = 50,     ///< application/json
    COAP_CF_CBOR = 60,     ///< application/cbor
    COAP_CF_NONE = 0xFFFF, ///< no Content-Format option present / emitted
};

/**
 * @brief A decoded CoAP request handed to a resource handler.
 *
 * All pointers reference transport- or library-owned scratch valid only for the
 * duration of the handler call; copy out anything you need to keep.
 */
struct CoapRequest
{
    uint8_t method;          ///< COAP_GET / COAP_POST / COAP_PUT / COAP_DELETE.
    const char *path;        ///< reconstructed Uri-Path, e.g. "/temp" (always begins with '/').
    const char *query;       ///< reconstructed Uri-Query (segments joined by '&'), or "" if none.
    const uint8_t *payload;  ///< request payload bytes (may be nullptr if payload_len == 0).
    size_t payload_len;      ///< request payload length in bytes.
    uint16_t content_format; ///< request Content-Format, or COAP_CF_NONE if absent.
};

/**
 * @brief A response a resource handler fills in.
 *
 * @p code defaults to 2.05 Content; set it to another CoapResponseCode as
 * appropriate. Write the response body into @p payload (capacity @p payload_cap)
 * and set @p payload_len. Set @p content_format to describe the body, or leave it
 * COAP_CF_NONE for an empty/typeless response.
 */
struct CoapResponse
{
    uint8_t code;            ///< response Code byte (see CoapResponseCode); defaults to COAP_RSP_CONTENT.
    uint16_t content_format; ///< COAP_CF_* describing the body, or COAP_CF_NONE.
    uint8_t *payload;        ///< caller-provided buffer to write the response body into.
    size_t payload_cap;      ///< capacity of @p payload in bytes.
    size_t payload_len;      ///< bytes written by the handler (0 = empty body).
};

/** @brief Resource handler: read @p req, fill @p resp. */
typedef void (*CoapHandler)(const CoapRequest *req, CoapResponse *resp);

// ---------------------------------------------------------------------------
// Server configuration / resource registration
// ---------------------------------------------------------------------------

/** @brief Reset the server and clear the resource table. Call before registering resources. */
void coap_server_init();

/**
 * @brief Register a resource at @p path served by @p handler.
 *
 * @param path     resource path beginning with '/' (referenced by pointer, not copied).
 * @param methods  allowed-methods bitmask (e.g. COAP_ALLOW_GET | COAP_ALLOW_PUT); a request
 *                 using a method not in the mask is answered 4.05 Method Not Allowed.
 * @param handler  invoked for an allowed method on a matching path.
 * @return false if the table is full.
 */
bool coap_server_add_resource(const char *path, uint8_t methods, CoapHandler handler);

// ---------------------------------------------------------------------------
// Core processing (host-testable; no sockets, no heap)
// ---------------------------------------------------------------------------

/**
 * @brief Process one CoAP request datagram and build the response datagram.
 *
 * Parses the message, reconstructs the Uri-Path/Uri-Query, dispatches against the
 * resource table, and encodes a piggybacked response (ACK for CON, NON for NON).
 * A malformed or unsupported-version CON is answered with an RST; a malformed NON
 * (or any ACK/RST received) yields no response.
 *
 * @param req      request datagram bytes.
 * @param req_len  number of bytes in @p req.
 * @param resp     destination buffer for the response datagram.
 * @param resp_cap capacity of @p resp.
 * @return number of response bytes written, or 0 to send nothing.
 */
size_t coap_server_process(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap);

/**
 * @brief Like coap_server_process(), but include an Observe option (RFC 7641) in
 *        a successful (2.xx) response carrying the notification sequence
 *        @p observe_seq (a value < 0 omits it). Used by the Observe transport.
 */
size_t coap_server_process_ex(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap, int32_t observe_seq);

// ---------------------------------------------------------------------------
// UDP transport (binds via the transport-layer UDP service; no-op on host)
// ---------------------------------------------------------------------------

/**
 * @brief Bind the server to UDP @p port (default 5683) via the transport-layer UDP service.
 *
 * Callback-driven (no per-loop servicing). Call after WiFi is up. On non-Arduino
 * builds det_udp_listen() is a stub, so the core remains host-testable.
 */
void coap_server_begin_udp(uint16_t port = 5683);

#if DETWS_ENABLE_COAP_OBSERVE
/**
 * @brief Push a notification to every observer of @p path (RFC 7641).
 *
 * Re-renders the resource (invokes its GET handler) and sends the current
 * representation as a CoAP notification - from the bound server port, carrying
 * each observer's token and an increasing Observe sequence. Call this whenever the
 * resource's state changes. A send failure drops that observer. No-op on a host
 * build. A client registers by sending a GET with the Observe option (0); it
 * deregisters with Observe (1), a Reset, or by going away.
 */
void coap_notify(const char *path);
#endif

#endif // DETWS_ENABLE_COAP

#endif // DETERMINISTICESPASYNCWEBSERVER_COAP_H
