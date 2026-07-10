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
 * Shipped: the NEGOTIATE exchange; the NTLM crypto (smb_md / ntlm / ntlmssp); the SPNEGO wrapping
 * (spnego); the SESSION_SETUP request/response framing that carries those tokens; and the
 * TREE_CONNECT / CREATE / CLOSE / READ / WRITE file commands - the full read/write-a-file-on-a-share
 * client. Roadmap (later options): SMB 3.1.1 negotiate contexts + preauth integrity, SMB2 signing.
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

/** @brief SESSION_SETUP response SessionFlags (MS-SMB2 §2.2.6). */
enum
{
    SMB2_SESSION_FLAG_IS_GUEST = 0x0001,
    SMB2_SESSION_FLAG_IS_NULL = 0x0002,
    SMB2_SESSION_FLAG_ENCRYPT_DATA = 0x0004,
};

/** @brief NT status values seen in the SMB2 header during the SESSION_SETUP exchange. */
enum
{
    SMB2_STATUS_SUCCESS = 0x00000000,
    SMB2_STATUS_MORE_PROCESSING_REQUIRED = 0xC0000016, ///< server wants the next SESSION_SETUP round
};

/** @brief TREE_CONNECT response ShareType (MS-SMB2 §2.2.10). */
enum
{
    SMB2_SHARE_TYPE_DISK = 0x01,
    SMB2_SHARE_TYPE_PIPE = 0x02,
    SMB2_SHARE_TYPE_PRINT = 0x03,
};

/** @brief CREATE DesiredAccess masks (MS-DTYP ACCESS_MASK; the common file rights). */
enum
{
    SMB2_FILE_READ_DATA = 0x00000001,
    SMB2_FILE_WRITE_DATA = 0x00000002,
    SMB2_FILE_APPEND_DATA = 0x00000004,
    SMB2_FILE_READ_ATTRIBUTES = 0x00000080,
    SMB2_FILE_GENERIC_READ = 0x00120089,  ///< READ_CONTROL|SYNCHRONIZE|READ_ATTRIBUTES|READ_EA|READ_DATA
    SMB2_FILE_GENERIC_WRITE = 0x00120116, ///< READ_CONTROL|SYNCHRONIZE|WRITE_ATTRIBUTES|WRITE_EA|APPEND|WRITE_DATA
};

/** @brief CREATE ShareAccess (MS-SMB2 §2.2.13). */
enum
{
    SMB2_FILE_SHARE_READ = 0x01,
    SMB2_FILE_SHARE_WRITE = 0x02,
    SMB2_FILE_SHARE_DELETE = 0x04,
};

/** @brief CREATE CreateDisposition (MS-SMB2 §2.2.13). */
enum
{
    SMB2_FILE_SUPERSEDE = 0,
    SMB2_FILE_OPEN = 1,      ///< open an existing file, fail if absent
    SMB2_FILE_CREATE = 2,    ///< create, fail if it exists
    SMB2_FILE_OPEN_IF = 3,   ///< open, create if absent
    SMB2_FILE_OVERWRITE = 4, ///< open + truncate, fail if absent
    SMB2_FILE_OVERWRITE_IF = 5,
};

/** @brief CREATE CreateOptions (MS-SMB2 §2.2.13; the two we set). */
enum
{
    SMB2_FILE_DIRECTORY_FILE = 0x00000001,
    SMB2_FILE_NON_DIRECTORY_FILE = 0x00000040,
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

/** @brief Parsed SESSION_SETUP response (MS-SMB2 §2.2.6). */
struct Smb2SessionSetupResp
{
    uint16_t session_flags;
    const uint8_t *sec_buf; ///< the server's SPNEGO/NTLM token (points into @p msg), or nullptr
    uint16_t sec_buf_len;
};

/**
 * @brief Build a SESSION_SETUP request (header + §2.2.5 body) carrying a security token.
 *
 * The token is the SPNEGO/NTLMSSP blob for this round of the handshake: the InitialContextToken on
 * the first request, and (echoing the SessionId the server returned) the AUTHENTICATE NegTokenResp
 * on the second. Capabilities / Channel / PreviousSessionId are 0 (a plain new session).
 *
 * @param message_id    the SMB2 MessageId (increments across the exchange).
 * @param session_id    0 on the first round; the server-assigned SessionId on the second.
 * @param security_mode SMB2_NEGOTIATE_SIGNING_ENABLED and/or _REQUIRED (one byte on the wire).
 * @return total message bytes (no transport prefix), or 0 on overflow / empty token.
 */
size_t smb2_build_session_setup(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                                uint8_t security_mode, const uint8_t *sec_buf, size_t sec_len);

/**
 * @brief Parse a SESSION_SETUP response message (the SMB2 header + §2.2.6 body).
 *
 * The caller reads the SessionId (to echo on the next round) and the NT status
 * (SMB2_STATUS_MORE_PROCESSING_REQUIRED vs SMB2_STATUS_SUCCESS) from smb2_parse_header on the same
 * @p msg; this extracts the SessionFlags and the server security buffer.
 *
 * @param msg the SMB2 message (starting at the sync header, transport prefix already stripped).
 * @return true on a well-formed response (header valid, command == SESSION_SETUP, StructureSize == 9,
 *         security buffer within bounds); false otherwise. On success @p out->sec_buf points into
 *         @p msg (or is nullptr when SecurityBufferLength is 0).
 */
bool smb2_parse_session_setup_response(const uint8_t *msg, size_t len, Smb2SessionSetupResp *out);

/**
 * @brief Build a TREE_CONNECT request (header + §2.2.9 body) for a share path.
 * @param path_utf16 the UNC path `\\server\share` in UTF-16LE (no NUL); @p path_len its byte length.
 * @return total message bytes (no transport prefix), or 0 on overflow / empty path.
 */
size_t smb2_build_tree_connect(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                               const uint8_t *path_utf16, size_t path_len);

/** @brief Parsed TREE_CONNECT response (MS-SMB2 §2.2.10). The TreeId is in the response header. */
struct Smb2TreeConnectResp
{
    uint8_t share_type;
    uint32_t share_flags;
    uint32_t capabilities;
    uint32_t maximal_access;
};

/**
 * @brief Parse a TREE_CONNECT response message (validates command + StructureSize 16).
 * @return true on a well-formed response; the caller reads the TreeId from smb2_parse_header.
 */
bool smb2_parse_tree_connect_response(const uint8_t *msg, size_t len, Smb2TreeConnectResp *out);

/**
 * @brief Build a CREATE request (header + §2.2.13 body) to open/create a file on the tree.
 * @param name_utf16 the file name relative to the share root in UTF-16LE (no leading backslash, no
 *                   NUL); @p name_len its byte length (must be > 0).
 * @param desired_access     e.g. SMB2_FILE_GENERIC_READ / _WRITE.
 * @param share_access       SMB2_FILE_SHARE_* bitmask.
 * @param create_disposition SMB2_FILE_OPEN / _CREATE / _OPEN_IF / ...
 * @param create_options     SMB2_FILE_NON_DIRECTORY_FILE for a regular file.
 * @return total message bytes (no transport prefix), or 0 on overflow.
 */
size_t smb2_build_create(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                         uint32_t desired_access, uint32_t share_access, uint32_t create_disposition,
                         uint32_t create_options, const uint8_t *name_utf16, size_t name_len);

/** @brief Parsed CREATE response (MS-SMB2 §2.2.14). */
struct Smb2CreateResp
{
    uint8_t file_id[16]; ///< the open handle (persistent 8 + volatile 8), for READ/WRITE/CLOSE
    uint64_t end_of_file;
    uint32_t create_action;
    uint32_t file_attributes;
};

/**
 * @brief Parse a CREATE response message (validates command + StructureSize 89, FileId in bounds).
 * @return true on a well-formed response.
 */
bool smb2_parse_create_response(const uint8_t *msg, size_t len, Smb2CreateResp *out);

/**
 * @brief Build a CLOSE request (header + §2.2.15 body) for an open FileId.
 * @return total message bytes (no transport prefix), or 0 on overflow.
 */
size_t smb2_build_close(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                        const uint8_t file_id[16]);

/** @brief Parsed CLOSE response (MS-SMB2 §2.2.16). */
struct Smb2CloseResp
{
    uint64_t end_of_file;
    uint32_t file_attributes;
};

/**
 * @brief Parse a CLOSE response message (validates command + StructureSize 60).
 * @return true on a well-formed response.
 */
bool smb2_parse_close_response(const uint8_t *msg, size_t len, Smb2CloseResp *out);

/**
 * @brief Build a READ request (header + §2.2.19 body) for @p length bytes at @p offset of an open file.
 * @return total message bytes (no transport prefix), or 0 on overflow.
 */
size_t smb2_build_read(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                       const uint8_t file_id[16], uint32_t length, uint64_t offset);

/** @brief Parsed READ response (MS-SMB2 §2.2.20). */
struct Smb2ReadResp
{
    const uint8_t *data; ///< the file bytes read (points into @p msg), or nullptr when DataLength is 0
    uint32_t data_len;
};

/**
 * @brief Parse a READ response message (validates command + StructureSize 17, data within bounds).
 * @return true on a well-formed response.
 */
bool smb2_parse_read_response(const uint8_t *msg, size_t len, Smb2ReadResp *out);

/**
 * @brief Build a WRITE request (header + §2.2.21 body) writing @p data at @p offset of an open file.
 * @return total message bytes (no transport prefix), or 0 on overflow / empty data.
 */
size_t smb2_build_write(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                        const uint8_t file_id[16], const uint8_t *data, size_t data_len, uint64_t offset);

/** @brief Parsed WRITE response (MS-SMB2 §2.2.22). */
struct Smb2WriteResp
{
    uint32_t count; ///< bytes actually written
};

/**
 * @brief Parse a WRITE response message (validates command + StructureSize 17).
 * @return true on a well-formed response.
 */
bool smb2_parse_write_response(const uint8_t *msg, size_t len, Smb2WriteResp *out);

#endif // DETWS_ENABLE_SMB

#endif // DETERMINISTICESPASYNCWEBSERVER_SMB2_H
