// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb2.h
 * @brief SMB2 client wire codec (MS-SMB2), DETWS_ENABLE_SMB - increment 1: the transport
 *        frame, the 64-byte sync packet header, and the NEGOTIATE exchange.
 *
 * Windows-share program storage is a common CNC file path (Fanuc / Haas / Mazak / Heidenhain
 * expose one), so a device can read/write `.nc` files over SMB2. This is the pure wire layer:
 * build the little-endian SMB2 messages and parse the responses; the TCP socket is the
 * application's. All fields are little-endian (SMB2 is a little-endian protocol).
 *
 * A client speaks SMB2 over Direct TCP (port 445): each message is prefixed by a 4-byte transport
 * header (`0x00` + a 24-bit big-endian length), then the 64-byte SMB2 sync header (MS-SMB2
 * §2.2.1.2), then the per-command body. The exchange begins with NEGOTIATE (§2.2.3 request /
 * §2.2.4 response): the client offers a dialect list, the server picks one and returns the SPNEGO
 * security token that seeds authentication.
 *
 * Roadmap (later increments): NTLM crypto (MD4/MD5/HMAC-MD5) + SESSION_SETUP, then TREE_CONNECT /
 * CREATE / READ / WRITE / CLOSE. Increment 1 needs no crypto.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SMB2_H
#define DETERMINISTICESPASYNCWEBSERVER_SMB2_H

#include "ServerConfig.h"

#if DETWS_ENABLE_SMB

#include <stddef.h>
#include <stdint.h>

/** @brief SMB2 command codes (MS-SMB2 §2.2.1.2). */
enum Smb2Command
{
    SMB2_NEGOTIATE = 0x0000,
    SMB2_SESSION_SETUP = 0x0001,
    SMB2_LOGOFF = 0x0002,
    SMB2_TREE_CONNECT = 0x0003,
    SMB2_TREE_DISCONNECT = 0x0004,
    SMB2_CREATE = 0x0005,
    SMB2_CLOSE = 0x0006,
    SMB2_READ = 0x0008,
    SMB2_WRITE = 0x0009,
};

/** @brief SMB2 dialect revision numbers (MS-SMB2 §2.2.4). */
enum Smb2Dialect
{
    SMB2_DIALECT_0202 = 0x0202, ///< SMB 2.0.2
    SMB2_DIALECT_0210 = 0x0210, ///< SMB 2.1
    SMB2_DIALECT_0300 = 0x0300, ///< SMB 3.0
    SMB2_DIALECT_0302 = 0x0302, ///< SMB 3.0.2
    SMB2_DIALECT_0311 = 0x0311, ///< SMB 3.1.1
};

enum
{
    SMB2_NEGOTIATE_SIGNING_ENABLED = 0x0001,
    SMB2_NEGOTIATE_SIGNING_REQUIRED = 0x0002,
    SMB2_FLAGS_SERVER_TO_REDIR = 0x00000001, ///< set on a response (server -> client)
    SMB2_HEADER_SIZE = 64,
};

/** @brief Parsed SMB2 sync header. */
struct Smb2Header
{
    uint16_t command;
    uint32_t status; ///< NT status (response); 0 = STATUS_SUCCESS
    uint32_t flags;
    uint64_t message_id;
    uint32_t tree_id;
    uint64_t session_id;
    uint16_t credit_response;
};

/** @brief Parsed NEGOTIATE response (MS-SMB2 §2.2.4). */
struct Smb2NegotiateResp
{
    uint16_t security_mode;
    uint16_t dialect; ///< the DialectRevision the server chose
    uint8_t server_guid[16];
    uint32_t capabilities;
    uint32_t max_transact;
    uint32_t max_read;
    uint32_t max_write;
    const uint8_t *sec_buf; ///< SPNEGO/NTLM security token (points into @p msg), or nullptr
    uint16_t sec_buf_len;
};

/**
 * @brief Prefix an SMB2 message with the 4-byte Direct-TCP transport header (`0x00` + a 24-bit
 *        big-endian length) into @p out.
 * @return total bytes written (4 + @p msg_len), or 0 on overflow / a length that does not fit 24 bits.
 */
size_t smb2_transport_frame(uint8_t *out, size_t cap, const uint8_t *msg, size_t msg_len);

/**
 * @brief Read the Direct-TCP transport length prefix.
 * @return the SMB2 message length that follows the 4-byte prefix, or 0 if @p len < 4 or the first
 *         byte is non-zero (an invalid Direct-TCP frame).
 */
uint32_t smb2_transport_len(const uint8_t *buf, size_t len);

/**
 * @brief Build a 64-byte SMB2 sync header into @p buf.
 * @return SMB2_HEADER_SIZE, or 0 if @p cap < 64.
 */
size_t smb2_build_header(uint8_t *buf, size_t cap, uint16_t command, uint16_t credit_request, uint64_t message_id,
                         uint32_t tree_id, uint64_t session_id);

/**
 * @brief Parse a 64-byte SMB2 sync header (validates ProtocolId + StructureSize).
 * @return true on a valid header; false if @p len < 64, ProtocolId != `FE 53 4D 42`, or
 *         StructureSize != 64.
 */
bool smb2_parse_header(const uint8_t *buf, size_t len, Smb2Header *out);

/**
 * @brief Build a NEGOTIATE request (header + body) offering SMB 2.0.2 / 2.1 / 3.0 / 3.0.2.
 * @param client_guid   the 16-byte client GUID.
 * @param security_mode SMB2_NEGOTIATE_SIGNING_ENABLED and/or _REQUIRED.
 * @return total message bytes (no transport prefix), or 0 on overflow.
 */
size_t smb2_build_negotiate(uint8_t *buf, size_t cap, const uint8_t client_guid[16], uint16_t security_mode);

/**
 * @brief Parse a NEGOTIATE response message (the SMB2 header + §2.2.4 body).
 *
 * @param msg the SMB2 message (starting at the sync header, transport prefix already stripped).
 * @return true on a well-formed response (header valid, command == NEGOTIATE, StructureSize == 65,
 *         and the security buffer within bounds); false otherwise. On success @p out->sec_buf points
 *         into @p msg (or is nullptr when SecurityBufferLength is 0).
 */
bool smb2_parse_negotiate_response(const uint8_t *msg, size_t len, Smb2NegotiateResp *out);

#endif // DETWS_ENABLE_SMB

#endif // DETERMINISTICESPASYNCWEBSERVER_SMB2_H
