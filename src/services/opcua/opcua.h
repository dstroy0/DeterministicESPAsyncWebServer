// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file opcua.h
 * @brief OPC UA Binary codec + UA-TCP transport + SecureChannel + Session (DETWS_ENABLE_OPCUA).
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
 * gets through the handshake. Session and the Read service are later increments.
 * SecurityPolicy is None (no encryption) for now.
 *
 * **Increment 2** adds the SecureChannel: NodeId / ExtensionObject / DateTime
 * encoding on top of the increment-1 codec, then parsing an `OpenSecureChannel`
 * (OPN) request and answering with an `OpenSecureChannelResponse` - the server
 * assigns a SecureChannelId + security token (SecurityPolicy None, OPC UA Part 6
 * §7.1.3 / Part 4 §5.5.2).
 *
 * **Increment 3** adds the Session: `MSG` (secure conversation) service calls
 * dispatched by their body TypeId - `CreateSession` (the server assigns a SessionId
 * and AuthenticationToken) and `ActivateSession` (OPC UA Part 4 §5.6).
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
// NodeId / ExtensionObject / DateTime (OPC UA Part 6 §5.2.2)
// ---------------------------------------------------------------------------

/** @brief Numeric NodeId (the only kind the SecureChannel service needs). */
struct UaNodeId
{
    uint16_t ns;  ///< NamespaceIndex.
    uint32_t id;  ///< Numeric identifier.
    bool numeric; ///< false for a String/Guid/ByteString id (value skipped on read).
};

/** @brief Encode a numeric NodeId, picking the smallest of the TwoByte/FourByte/Numeric forms. */
void ua_w_nodeid_numeric(UaWriter *w, uint16_t ns, uint32_t id);

/**
 * @brief Decode a NodeId. Numeric forms fill @p out; String/Guid/ByteString ids
 *        are skipped (out->numeric=false). Latches @c err on an unknown form.
 */
bool ua_r_nodeid(UaReader *r, UaNodeId *out);

/** @brief Convert a Unix epoch (seconds) to an OPC UA DateTime (100 ns ticks since 1601), 0 for <= 0. */
int64_t opcua_filetime_from_unix(int64_t unix_seconds);

/** @brief Numeric NodeIds (namespace 0) the SecureChannel service uses (binary encoding ids). */
#define OPCUA_ID_OPEN_REQ 446  ///< OpenSecureChannelRequest_Encoding_DefaultBinary.
#define OPCUA_ID_OPEN_RESP 449 ///< OpenSecureChannelResponse_Encoding_DefaultBinary.

/** @brief SecurityPolicy "None" URI (no signing/encryption). */
#define OPCUA_POLICY_NONE_URI "http://opcfoundation.org/UA/SecurityPolicy#None"

// ---------------------------------------------------------------------------
// SecureChannel - OpenSecureChannel (OPN), SecurityPolicy None
// ---------------------------------------------------------------------------

/** @brief Fields extracted from an OpenSecureChannelRequest we need to reply. */
struct OpcUaOpenChannel
{
    uint32_t secure_channel_id;           ///< 0 on a fresh issue; non-zero on renew.
    uint32_t sequence_number;             ///< Client SequenceHeader SequenceNumber.
    uint32_t request_id;                  ///< Client SequenceHeader RequestId (echoed).
    uint32_t request_handle;              ///< RequestHeader RequestHandle (echoed).
    uint32_t client_protocol_version;     ///< ClientProtocolVersion.
    uint32_t security_token_request_type; ///< 0 = Issue, 1 = Renew.
    uint32_t message_security_mode;       ///< 1 = None, 2 = Sign, 3 = SignAndEncrypt.
    uint32_t requested_lifetime;          ///< RequestedLifetime (ms).
};

/** @brief Parse a complete `OPN` message (SecurityPolicy None). @return true if valid. */
bool opcua_parse_open(const uint8_t *msg, size_t len, OpcUaOpenChannel *out);

/**
 * @brief Build the `OPN` OpenSecureChannelResponse to a parsed request.
 * @param req        the parsed request (RequestId/RequestHandle are echoed).
 * @param channel_id SecureChannelId the server assigns/keeps.
 * @param token_id   security TokenId the server issues.
 * @param seq_number server SequenceNumber for this message.
 * @param now_ft     OPC UA DateTime for the response/token timestamps (0 = unset).
 * @param lifetime   RevisedLifetime (ms) granted to the token.
 * @return total OPN message bytes written, or 0 if it does not fit @p cap.
 */
size_t opcua_build_open_response(const OpcUaOpenChannel *req, uint32_t channel_id, uint32_t token_id,
                                 uint32_t seq_number, int64_t now_ft, uint32_t lifetime, uint8_t *out, size_t cap);

// ---------------------------------------------------------------------------
// Session - CreateSession / ActivateSession (MSG service calls, SecurityPolicy None)
// ---------------------------------------------------------------------------

/** @brief Numeric NodeIds (namespace 0) for the Session services (binary encoding ids). */
#define OPCUA_ID_CREATE_SESSION_REQ 461    ///< CreateSessionRequest_Encoding_DefaultBinary.
#define OPCUA_ID_CREATE_SESSION_RESP 464   ///< CreateSessionResponse_Encoding_DefaultBinary.
#define OPCUA_ID_ACTIVATE_SESSION_REQ 467  ///< ActivateSessionRequest_Encoding_DefaultBinary.
#define OPCUA_ID_ACTIVATE_SESSION_RESP 470 ///< ActivateSessionResponse_Encoding_DefaultBinary.

/** @brief Common fields of a `MSG` (secure conversation) service request. */
struct OpcUaMsg
{
    uint32_t token_id;        ///< SymmetricSecurityHeader TokenId.
    uint32_t sequence_number; ///< SequenceHeader SequenceNumber.
    uint32_t request_id;      ///< SequenceHeader RequestId (echoed in the response).
    uint32_t type_id;         ///< Body TypeId NodeId (numeric id; 0 if non-numeric).
    uint32_t request_handle;  ///< RequestHeader RequestHandle (echoed in the response).
};

/**
 * @brief Parse a `MSG` envelope: security + sequence headers, body TypeId, and the
 *        leading RequestHeader (every service request starts with one).
 * @return true if valid. @p out->type_id selects the service to dispatch.
 */
bool opcua_parse_msg(const uint8_t *msg, size_t len, OpcUaMsg *out);

/**
 * @brief Build a `MSG` CreateSessionResponse (SecurityPolicy None): assign a SessionId
 *        and AuthenticationToken, empty endpoint/certificate/signature fields.
 * @param req             the parsed request (TokenId/RequestId/RequestHandle echoed).
 * @param session_id      numeric SessionId identifier the server assigns (namespace 1).
 * @param auth_token      numeric AuthenticationToken the server assigns (namespace 1).
 * @param revised_timeout RevisedSessionTimeout (ms).
 * @param seq             server SequenceNumber for this message.
 * @param now_ft          OPC UA DateTime for the response timestamp (0 = unset).
 * @return total MSG bytes written, or 0 if it does not fit @p cap.
 */
size_t opcua_build_create_session_response(const OpcUaMsg *req, uint32_t session_id, uint32_t auth_token,
                                           double revised_timeout, uint32_t seq, int64_t now_ft, uint8_t *out,
                                           size_t cap);

/**
 * @brief Build a `MSG` ActivateSessionResponse (SecurityPolicy None): ServiceResult Good,
 *        empty ServerNonce / Results / DiagnosticInfos.
 * @return total MSG bytes written, or 0 if it does not fit @p cap.
 */
size_t opcua_build_activate_session_response(const OpcUaMsg *req, uint32_t seq, int64_t now_ft, uint8_t *out,
                                             size_t cap);

// ---------------------------------------------------------------------------
// ESP32 TCP server (PROTO_OPCUA data handler)
// ---------------------------------------------------------------------------

/**
 * @brief PROTO_OPCUA data handler: drive the UA-TCP handshake, SecureChannel, Session.
 *
 * Dispatched by the session layer for connections accepted on an OPC UA listener
 * (`listen(4840, PROTO_OPCUA)`). Drains framed messages from the slot's rx ring:
 * `HEL` -> negotiated `ACK`, `OPN` -> OpenSecureChannelResponse (SecurityPolicy
 * None), `MSG` CreateSession/ActivateSession -> their responses, `CLO` -> close.
 * The Read service is a later increment.
 */
void opcua_rx(uint8_t slot);

#endif // DETWS_ENABLE_OPCUA
#endif // DETERMINISTICESPASYNCWEBSERVER_OPCUA_H
