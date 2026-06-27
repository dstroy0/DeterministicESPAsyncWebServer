// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file opcua.h
 * @brief OPC UA Binary codec + UA-TCP transport, increment 1 (DETWS_ENABLE_OPCUA).
 *
 * OPC UA (IEC 62541) is large; this is built in increments. **Increment 1** is the
 * foundation every OPC UA server needs:
 *   - the OPC UA Binary built-in-type codec (little-endian writer/reader for
 *     Boolean, integers, Float/Double, String, ByteString - bounds-checked, the
 *     basis for every later structure/service),
 *   - the UA-TCP connection protocol (UACP) message framing
 *     (`MessageType` + chunk byte + `MessageSize`), and
 *   - the **Hello / Acknowledge** handshake (OPC UA Part 6 §7.1.2): parse a
 *     client `HEL`, negotiate buffer sizes, emit an `ACK`.
 *
 * The codec + framing + handshake are pure and host-tested; opcua_rx() is the
 * PROTO_OPCUA TCP data handler (ESP32) - `listen(4840, PROTO_OPCUA)` and a client
 * gets through the handshake. SecureChannel (OPN), Session, and the Read service
 * are later increments. SecurityPolicy is None (no encryption) for now.
 *
 * No heap, no stdlib.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_OPCUA_H
#define DETERMINISTICESPASYNCWEBSERVER_OPCUA_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_OPCUA

/** @brief Server's advertised buffer / max-message size for the handshake. */
#ifndef DETWS_OPCUA_BUF
#define DETWS_OPCUA_BUF 8192
#endif

// ---------------------------------------------------------------------------
// OPC UA Binary built-in type codec (little-endian; OPC UA Part 6 §5.2)
// ---------------------------------------------------------------------------

/** @brief Bounds-checked little-endian writer. */
struct UaWriter
{
    uint8_t *o;
    size_t cap, n;
    bool ok;
};
void ua_w_u8(UaWriter *w, uint8_t v);
void ua_w_u16(UaWriter *w, uint16_t v);
void ua_w_u32(UaWriter *w, uint32_t v);
void ua_w_u64(UaWriter *w, uint64_t v);
void ua_w_i32(UaWriter *w, int32_t v);
void ua_w_f32(UaWriter *w, float v);
void ua_w_f64(UaWriter *w, double v);
void ua_w_bool(UaWriter *w, bool v);
/** @brief Encode a String/ByteString: int32 length (-1 = null) then the bytes. */
void ua_w_string(UaWriter *w, const char *s, int32_t len);

/** @brief Bounds-checked little-endian reader; @c err latches on underrun. */
struct UaReader
{
    const uint8_t *p;
    size_t len, off;
    bool err;
};
uint8_t ua_r_u8(UaReader *r);
uint16_t ua_r_u16(UaReader *r);
uint32_t ua_r_u32(UaReader *r);
uint64_t ua_r_u64(UaReader *r);
int32_t ua_r_i32(UaReader *r);
float ua_r_f32(UaReader *r);
double ua_r_f64(UaReader *r);
bool ua_r_bool(UaReader *r);
/**
 * @brief Decode a String/ByteString into @p out (NUL-terminated, bounded).
 * @param out_len  set to the decoded length (or -1 for a null string).
 * @return false on underrun or if the value does not fit @p cap.
 */
bool ua_r_string(UaReader *r, char *out, size_t cap, int32_t *out_len);

// ---------------------------------------------------------------------------
// UA-TCP (UACP) message framing
// ---------------------------------------------------------------------------

/** @brief Parsed UACP message header (8 bytes). */
struct UaMsgHeader
{
    char type[3];  ///< "HEL" / "ACK" / "ERR" / "OPN" / "MSG" / "CLO".
    char chunk;    ///< 'F' final, 'C' intermediate, 'A' abort.
    uint32_t size; ///< total message size including this 8-byte header.
};

/** @brief Parse the 8-byte UACP header from @p buf (need >= 8 bytes). */
bool opcua_parse_header(const uint8_t *buf, size_t len, UaMsgHeader *h);

// ---------------------------------------------------------------------------
// Hello / Acknowledge handshake (OPC UA Part 6 §7.1.2)
// ---------------------------------------------------------------------------

/** @brief Decoded Hello message body. */
struct OpcUaHello
{
    uint32_t protocol_version;
    uint32_t recv_buf_size;
    uint32_t send_buf_size;
    uint32_t max_msg_size;
    uint32_t max_chunk_count;
};

/** @brief Parse a complete `HEL` message (header + body). @return true if valid. */
bool opcua_parse_hello(const uint8_t *msg, size_t len, OpcUaHello *out);

/**
 * @brief Build the `ACK` reply to a parsed Hello, negotiating buffer sizes down
 *        to the server's DETWS_OPCUA_BUF limit.
 * @return total ACK message bytes written to @p out, or 0 if it does not fit.
 */
size_t opcua_build_ack(const OpcUaHello *hello, uint8_t *out, size_t cap);

// ---------------------------------------------------------------------------
// ESP32 TCP server (PROTO_OPCUA data handler)
// ---------------------------------------------------------------------------

/**
 * @brief PROTO_OPCUA data handler: complete the Hello/Acknowledge handshake.
 *
 * Dispatched by the session layer for connections accepted on an OPC UA listener
 * (`listen(4840, PROTO_OPCUA)`). Reads a framed `HEL` from the slot's rx ring and
 * sends the negotiated `ACK`.
 */
void opcua_rx(uint8_t slot);

#endif // DETWS_ENABLE_OPCUA
#endif // DETERMINISTICESPASYNCWEBSERVER_OPCUA_H
