// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb2.cpp
 * @brief SMB2 client wire codec implementation (see smb2.h). All fields little-endian.
 */

#include "smb2.h"

#if DETWS_ENABLE_SMB

#include <string.h>

// Little-endian primitives (SMB2 is little-endian on the wire).
static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}
static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static uint64_t rd64(const uint8_t *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}
static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
static void wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}
static void wr64(uint8_t *p, uint64_t v)
{
    wr32(p, (uint32_t)v);
    wr32(p + 4, (uint32_t)(v >> 32));
}

static const uint8_t SMB2_PROTOCOL_ID[4] = {0xFE, 'S', 'M', 'B'};

size_t smb2_transport_frame(uint8_t *out, size_t cap, const uint8_t *msg, size_t msg_len)
{
    if (!out || !msg || msg_len > 0x00FFFFFF || 4 + msg_len > cap)
        return 0;
    out[0] = 0x00; // Direct TCP: first byte MUST be zero
    out[1] = (uint8_t)(msg_len >> 16);
    out[2] = (uint8_t)(msg_len >> 8);
    out[3] = (uint8_t)(msg_len);
    memcpy(out + 4, msg, msg_len);
    return 4 + msg_len;
}

uint32_t smb2_transport_len(const uint8_t *buf, size_t len)
{
    if (!buf || len < 4 || buf[0] != 0x00)
        return 0;
    return ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

size_t smb2_build_header(uint8_t *buf, size_t cap, uint16_t command, uint16_t credit_request, uint64_t message_id,
                         uint32_t tree_id, uint64_t session_id)
{
    if (!buf || cap < SMB2_HEADER_SIZE)
        return 0;
    memset(buf, 0, SMB2_HEADER_SIZE);
    memcpy(buf + 0, SMB2_PROTOCOL_ID, 4); // ProtocolId
    wr16(buf + 4, 64);                    // StructureSize
    // CreditCharge (6), Status/ChannelSequence (8) left 0
    wr16(buf + 12, command);        // Command
    wr16(buf + 14, credit_request); // CreditRequest
    // Flags (16) = 0 (client request), NextCommand (20) = 0
    wr64(buf + 24, message_id); // MessageId
    // Reserved (32) = 0
    wr32(buf + 36, tree_id);    // TreeId
    wr64(buf + 40, session_id); // SessionId
    // Signature (48..63) = 0
    return SMB2_HEADER_SIZE;
}

bool smb2_parse_header(const uint8_t *buf, size_t len, Smb2Header *out)
{
    if (!buf || !out || len < SMB2_HEADER_SIZE)
        return false;
    if (memcmp(buf, SMB2_PROTOCOL_ID, 4) != 0 || rd16(buf + 4) != 64)
        return false;
    out->status = rd32(buf + 8);
    out->command = rd16(buf + 12);
    out->credit_response = rd16(buf + 14);
    out->flags = rd32(buf + 16);
    out->message_id = rd64(buf + 24);
    out->tree_id = rd32(buf + 36);
    out->session_id = rd64(buf + 40);
    return true;
}

size_t smb2_build_negotiate(uint8_t *buf, size_t cap, const uint8_t client_guid[16], uint16_t security_mode)
{
    static const uint16_t dialects[] = {SMB2_DIALECT_0202, SMB2_DIALECT_0210, SMB2_DIALECT_0300, SMB2_DIALECT_0302};
    const uint16_t ndialects = (uint16_t)(sizeof(dialects) / sizeof(dialects[0]));
    const size_t total = SMB2_HEADER_SIZE + 36 + (size_t)ndialects * 2; // header + fixed body + dialects
    if (!buf || !client_guid || cap < total)
        return 0;

    if (smb2_build_header(buf, cap, SMB2_NEGOTIATE, 1, 0, 0, 0) == 0)
        return 0;

    uint8_t *b = buf + SMB2_HEADER_SIZE; // NEGOTIATE request body
    memset(b, 0, 36);
    wr16(b + 0, 36);            // StructureSize
    wr16(b + 2, ndialects);     // DialectCount
    wr16(b + 4, security_mode); // SecurityMode
    // Reserved (6) = 0, Capabilities (8) = 0
    memcpy(b + 12, client_guid, 16); // ClientGuid
    // ClientStartTime (28) = 0 (only 3.1.1 reinterprets these 8 bytes as negotiate-context fields)
    for (uint16_t i = 0; i < ndialects; i++)
        wr16(b + 36 + i * 2, dialects[i]);
    return total;
}

bool smb2_parse_negotiate_response(const uint8_t *msg, size_t len, Smb2NegotiateResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!smb2_parse_header(msg, len, &h) || h.command != SMB2_NEGOTIATE)
        return false;
    // The fixed response body is 64 bytes (StructureSize .. NegotiateContextOffset), Buffer follows.
    if (len < SMB2_HEADER_SIZE + 64)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (rd16(b + 0) != 65) // StructureSize
        return false;

    out->security_mode = rd16(b + 2);
    out->dialect = rd16(b + 4);
    memcpy(out->server_guid, b + 8, 16);
    out->capabilities = rd32(b + 24);
    out->max_transact = rd32(b + 28);
    out->max_read = rd32(b + 32);
    out->max_write = rd32(b + 36);

    uint16_t sec_off = rd16(b + 56); // SecurityBufferOffset - from the start of the SMB2 header (msg)
    uint16_t sec_len = rd16(b + 58); // SecurityBufferLength
    if (sec_len == 0)
    {
        out->sec_buf = nullptr;
        out->sec_buf_len = 0;
        return true;
    }
    if ((size_t)sec_off + sec_len > len || sec_off < SMB2_HEADER_SIZE)
        return false; // security buffer out of bounds - fail closed
    out->sec_buf = msg + sec_off;
    out->sec_buf_len = sec_len;
    return true;
}

size_t smb2_build_session_setup(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                                uint8_t security_mode, const uint8_t *sec_buf, size_t sec_len)
{
    const size_t body = 24; // fixed SESSION_SETUP request body (§2.2.5)
    const size_t total = SMB2_HEADER_SIZE + body + sec_len;
    if (!buf || !sec_buf || sec_len == 0 || sec_len > 0xFFFF || cap < total)
        return 0;
    if (smb2_build_header(buf, cap, SMB2_SESSION_SETUP, 1, message_id, 0, session_id) == 0)
        return 0;

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    wr16(b + 0, 25);      // StructureSize (fixed 24 + 1 for the variable buffer)
    b[2] = 0;             // Flags (SMB2_SESSION_FLAG_BINDING only for 3.x channel binding)
    b[3] = security_mode; // SecurityMode (one byte here)
    // Capabilities (4) = 0, Channel (8) = 0
    wr16(b + 12, (uint16_t)(SMB2_HEADER_SIZE + body)); // SecurityBufferOffset (from the header start)
    wr16(b + 14, (uint16_t)sec_len);                   // SecurityBufferLength
    // PreviousSessionId (16) = 0 (a fresh session)
    memcpy(b + body, sec_buf, sec_len);
    return total;
}

bool smb2_parse_session_setup_response(const uint8_t *msg, size_t len, Smb2SessionSetupResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!smb2_parse_header(msg, len, &h) || h.command != SMB2_SESSION_SETUP)
        return false;
    // The fixed response body is 8 bytes (StructureSize .. SecurityBufferLength), Buffer follows.
    if (len < SMB2_HEADER_SIZE + 8)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (rd16(b + 0) != 9) // StructureSize
        return false;

    out->session_flags = rd16(b + 2);
    uint16_t sec_off = rd16(b + 4); // SecurityBufferOffset - from the start of the SMB2 header (msg)
    uint16_t sec_len = rd16(b + 6); // SecurityBufferLength
    if (sec_len == 0)
    {
        out->sec_buf = nullptr;
        out->sec_buf_len = 0;
        return true;
    }
    if ((size_t)sec_off + sec_len > len || sec_off < SMB2_HEADER_SIZE)
        return false; // security buffer out of bounds - fail closed
    out->sec_buf = msg + sec_off;
    out->sec_buf_len = sec_len;
    return true;
}

size_t smb2_build_tree_connect(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                               const uint8_t *path_utf16, size_t path_len)
{
    const size_t body = 8; // fixed TREE_CONNECT request body (§2.2.9)
    const size_t total = SMB2_HEADER_SIZE + body + path_len;
    if (!buf || !path_utf16 || path_len == 0 || path_len > 0xFFFF || cap < total)
        return 0;
    if (smb2_build_header(buf, cap, SMB2_TREE_CONNECT, 1, message_id, 0, session_id) == 0)
        return 0;

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    wr16(b + 0, 9); // StructureSize
    // Flags/Reserved (2) = 0
    wr16(b + 4, (uint16_t)(SMB2_HEADER_SIZE + body)); // PathOffset (from the header start) = 72
    wr16(b + 6, (uint16_t)path_len);                  // PathLength
    memcpy(b + body, path_utf16, path_len);           // the \\server\share path (UTF-16LE)
    return total;
}

bool smb2_parse_tree_connect_response(const uint8_t *msg, size_t len, Smb2TreeConnectResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!smb2_parse_header(msg, len, &h) || h.command != SMB2_TREE_CONNECT)
        return false;
    if (len < SMB2_HEADER_SIZE + 16) // fixed 16-byte body, no variable buffer
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (rd16(b + 0) != 16) // StructureSize
        return false;
    out->share_type = b[2];
    out->share_flags = rd32(b + 4);
    out->capabilities = rd32(b + 8);
    out->maximal_access = rd32(b + 12);
    return true;
}

size_t smb2_build_create(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                         uint32_t desired_access, uint32_t share_access, uint32_t create_disposition,
                         uint32_t create_options, const uint8_t *name_utf16, size_t name_len)
{
    const size_t body = 56; // fixed CREATE request body (§2.2.13)
    const size_t total = SMB2_HEADER_SIZE + body + name_len;
    if (!buf || !name_utf16 || name_len == 0 || name_len > 0xFFFF || cap < total)
        return 0;
    if (smb2_build_header(buf, cap, SMB2_CREATE, 1, message_id, tree_id, session_id) == 0)
        return 0;

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    wr16(b + 0, 57); // StructureSize (fixed 56 + 1 for the variable buffer)
    // SecurityFlags (2) = 0, RequestedOplockLevel (3) = 0 (SMB2_OPLOCK_LEVEL_NONE)
    wr32(b + 4, 2); // ImpersonationLevel = Impersonation
    // SmbCreateFlags (8) = 0, Reserved (16) = 0
    wr32(b + 24, desired_access);                      // DesiredAccess
    wr32(b + 28, 0);                                   // FileAttributes = 0
    wr32(b + 32, share_access);                        // ShareAccess
    wr32(b + 36, create_disposition);                  // CreateDisposition
    wr32(b + 40, create_options);                      // CreateOptions
    wr16(b + 44, (uint16_t)(SMB2_HEADER_SIZE + body)); // NameOffset (from the header start) = 120
    wr16(b + 46, (uint16_t)name_len);                  // NameLength
    // CreateContextsOffset (48) = 0, CreateContextsLength (52) = 0
    memcpy(b + body, name_utf16, name_len);
    return total;
}

bool smb2_parse_create_response(const uint8_t *msg, size_t len, Smb2CreateResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!smb2_parse_header(msg, len, &h) || h.command != SMB2_CREATE)
        return false;
    if (len < SMB2_HEADER_SIZE + 88) // fixed 88-byte body (StructureSize .. CreateContextsLength)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (rd16(b + 0) != 89) // StructureSize
        return false;
    out->create_action = rd32(b + 4);
    out->end_of_file = rd64(b + 48);
    out->file_attributes = rd32(b + 56);
    memcpy(out->file_id, b + 64, 16); // FileId (persistent 8 + volatile 8)
    return true;
}

size_t smb2_build_close(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                        const uint8_t file_id[16])
{
    const size_t body = 24; // fixed CLOSE request body (§2.2.15), no variable buffer
    const size_t total = SMB2_HEADER_SIZE + body;
    if (!buf || !file_id || cap < total)
        return 0;
    if (smb2_build_header(buf, cap, SMB2_CLOSE, 1, message_id, tree_id, session_id) == 0)
        return 0;

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    wr16(b + 0, 24); // StructureSize
    // Flags (2) = 0 (no POSTQUERY_ATTRIB), Reserved (4) = 0
    memcpy(b + 8, file_id, 16); // FileId
    return total;
}

bool smb2_parse_close_response(const uint8_t *msg, size_t len, Smb2CloseResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!smb2_parse_header(msg, len, &h) || h.command != SMB2_CLOSE)
        return false;
    if (len < SMB2_HEADER_SIZE + 60) // fixed 60-byte body
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (rd16(b + 0) != 60) // StructureSize
        return false;
    out->end_of_file = rd64(b + 48);
    out->file_attributes = rd32(b + 56);
    return true;
}

#endif // DETWS_ENABLE_SMB
