// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb2.cpp
 * @brief SMB2 client wire codec implementation (see smb2.h). All fields little-endian.
 */

#include "smb2.h"

#if DWS_ENABLE_SMB

#include <string.h>

#include "crypto/sha512.h" // dws_sha512 for the SMB 3.1.1 preauth-integrity chain
#include "shared_primitives/endian.h"
#include "smb_md.h" // dws_hmac_sha256 for message signing

static const uint8_t SMB2_PROTOCOL_ID[4] = {0xFE, 'S', 'M', 'B'};

size_t dws_smb2_transport_frame(uint8_t *out, size_t cap, const uint8_t *msg, size_t msg_len)
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

uint32_t dws_smb2_transport_len(const uint8_t *buf, size_t len)
{
    if (!buf || len < 4 || buf[0] != 0x00)
        return 0;
    return ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

size_t dws_smb2_build_header(uint8_t *buf, size_t cap, Smb2Command command, uint16_t credit_request,
                             uint64_t message_id, uint32_t tree_id, uint64_t session_id)
{
    if (!buf || cap < SMB2_HEADER_SIZE)
        return 0;
    memset(buf, 0, SMB2_HEADER_SIZE);
    memcpy(buf + 0, SMB2_PROTOCOL_ID, 4); // ProtocolId
    dws_wr16le(buf + 4, 64);              // StructureSize
    // bytes 6 CreditCharge and 8 Status/ChannelSequence stay zero
    dws_wr16le(buf + 12, (uint16_t)command); // Command
    dws_wr16le(buf + 14, credit_request);    // CreditRequest
    // byte 16 Flags stays zero for a client request; byte 20 NextCommand stays zero
    dws_wr64le(buf + 24, message_id); // MessageId
    // byte 32 Reserved stays zero
    dws_wr32le(buf + 36, tree_id);    // TreeId
    dws_wr64le(buf + 40, session_id); // SessionId
    // bytes 48 through 63 Signature stay zero
    return SMB2_HEADER_SIZE;
}

bool dws_smb2_parse_header(const uint8_t *buf, size_t len, Smb2Header *out)
{
    if (!buf || !out || len < SMB2_HEADER_SIZE)
        return false;
    if (memcmp(buf, SMB2_PROTOCOL_ID, 4) != 0 || dws_rd16le(buf + 4) != 64)
        return false;
    out->status = dws_rd32le(buf + 8);
    out->command = (Smb2Command)dws_rd16le(buf + 12);
    out->credit_response = dws_rd16le(buf + 14);
    out->flags = dws_rd32le(buf + 16);
    out->message_id = dws_rd64le(buf + 24);
    out->tree_id = dws_rd32le(buf + 36);
    out->session_id = dws_rd64le(buf + 40);
    return true;
}

size_t dws_smb2_build_negotiate(uint8_t *buf, size_t cap, const uint8_t client_guid[16], uint16_t security_mode)
{
    static const Smb2Dialect dialects[] = {Smb2Dialect::SMB2_DIALECT_0202, Smb2Dialect::SMB2_DIALECT_0210,
                                           Smb2Dialect::SMB2_DIALECT_0300, Smb2Dialect::SMB2_DIALECT_0302};
    const uint16_t ndialects = (uint16_t)(sizeof(dialects) / sizeof(dialects[0]));
    const size_t total = SMB2_HEADER_SIZE + 36 + (size_t)ndialects * 2; // header + fixed body + dialects
    if (!buf || !client_guid || cap < total)
        return 0;

    // GCOVR_EXCL_START  cap >= total (64 + 36 + 8) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_NEGOTIATE, 1, 0, 0, 0) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE; // NEGOTIATE request body
    memset(b, 0, 36);
    dws_wr16le(b + 0, 36);            // StructureSize
    dws_wr16le(b + 2, ndialects);     // DialectCount
    dws_wr16le(b + 4, security_mode); // SecurityMode
    // byte 6 Reserved and byte 8 Capabilities stay zero
    memcpy(b + 12, client_guid, 16); // ClientGuid
    // byte 28 ClientStartTime stays zero; only 3.1.1 reinterprets these 8 bytes as negotiate-context fields
    for (uint16_t i = 0; i < ndialects; i++)
        dws_wr16le(b + 36 + i * 2, (uint16_t)dialects[i]);
    return total;
}

bool dws_smb2_parse_negotiate_response(const uint8_t *msg, size_t len, Smb2NegotiateResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_NEGOTIATE)
        return false;
    // The fixed response body is 64 bytes (StructureSize .. NegotiateContextOffset), Buffer follows.
    if (len < SMB2_HEADER_SIZE + 64)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 65) // StructureSize
        return false;

    out->security_mode = dws_rd16le(b + 2);
    out->dialect = dws_rd16le(b + 4);
    memcpy(out->server_guid, b + 8, 16);
    out->capabilities = dws_rd32le(b + 24);
    out->max_transact = dws_rd32le(b + 28);
    out->max_read = dws_rd32le(b + 32);
    out->max_write = dws_rd32le(b + 36);

    uint16_t sec_off = dws_rd16le(b + 56); // SecurityBufferOffset - from the start of the SMB2 header (msg)
    uint16_t sec_len = dws_rd16le(b + 58); // SecurityBufferLength
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

size_t dws_smb2_build_negotiate_311(uint8_t *buf, size_t cap, const uint8_t client_guid[16], uint16_t security_mode,
                                    const uint8_t *salt, size_t salt_len)
{
    static const Smb2Dialect dialects[] = {Smb2Dialect::SMB2_DIALECT_0202, Smb2Dialect::SMB2_DIALECT_0210,
                                           Smb2Dialect::SMB2_DIALECT_0300, Smb2Dialect::SMB2_DIALECT_0302,
                                           Smb2Dialect::SMB2_DIALECT_0311};
    const uint16_t ndialects = (uint16_t)(sizeof(dialects) / sizeof(dialects[0]));
    if (!buf || !client_guid || !salt || salt_len == 0 || salt_len > 0xFFFF)
        return 0;

    // header(64) + fixed body(36) + dialects(2*n), padded to 8, then the negotiate-context list. Each
    // context is ContextType(2) + DataLength(2) + Reserved(4) + Data, 8-byte aligned (MS-SMB2 §2.2.3.1).
    const size_t body_end = SMB2_HEADER_SIZE + 36 + (size_t)ndialects * 2;
    const size_t ctx_start = (body_end + 7) & ~(size_t)7; // NegotiateContextOffset (from msg start)
    const size_t preauth_data = 6 + salt_len;             // HashAlgorithmCount + SaltLength + 1 hash + Salt
    const size_t preauth_ctx = 8 + preauth_data;          // context header + data
    const size_t after_preauth = ctx_start + preauth_ctx;
    const size_t preauth_pad = ((after_preauth + 7) & ~(size_t)7) - after_preauth; // align the next context
    const size_t sign_ctx = 8 + 4; // header + SigningAlgorithmCount + 1 algorithm
    const size_t total = ctx_start + preauth_ctx + preauth_pad + sign_ctx;
    if (cap < total)
        return 0;

    // GCOVR_EXCL_START  cap >= total >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_NEGOTIATE, 1, 0, 0, 0) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, ctx_start - SMB2_HEADER_SIZE); // fixed body + dialects + alignment pad
    dws_wr16le(b + 0, 36);                      // StructureSize
    dws_wr16le(b + 2, ndialects);               // DialectCount
    dws_wr16le(b + 4, security_mode);           // SecurityMode
    memcpy(b + 12, client_guid, 16);            // ClientGuid
    dws_wr32le(b + 28, (uint32_t)ctx_start);    // NegotiateContextOffset (overlays ClientStartTime)
    dws_wr16le(b + 32, 2);                      // NegotiateContextCount (preauth + signing)
    for (uint16_t i = 0; i < ndialects; i++)
        dws_wr16le(b + 36 + i * 2, (uint16_t)dialects[i]);

    // Context 1 - PREAUTH_INTEGRITY_CAPABILITIES (mandatory once 0x0311 is offered), §2.2.3.1.1.
    uint8_t *c = buf + ctx_start;
    dws_wr16le(c + 0, Smb2NegotiateContextType::SMB2_PREAUTH_INTEGRITY_CAPABILITIES);
    dws_wr16le(c + 2, (uint16_t)preauth_data);
    dws_wr32le(c + 4, 0);                                                 // Reserved
    dws_wr16le(c + 8, 1);                                                 // HashAlgorithmCount
    dws_wr16le(c + 10, (uint16_t)salt_len);                               // SaltLength
    dws_wr16le(c + 12, Smb2HashAlgorithm::SMB2_PREAUTH_INTEGRITY_SHA512); // HashAlgorithms[0]
    memcpy(c + 14, salt, salt_len);                                       // Salt

    // Context 2 - SIGNING_CAPABILITIES advertising HMAC-SHA256 (the algorithm this client signs with).
    uint8_t *c2 = c + preauth_ctx + preauth_pad;
    if (preauth_pad)
        memset(c + preauth_ctx, 0, preauth_pad);
    dws_wr16le(c2 + 0, Smb2NegotiateContextType::SMB2_SIGNING_CAPABILITIES);
    dws_wr16le(c2 + 2, 4);                                               // DataLength
    dws_wr32le(c2 + 4, 0);                                               // Reserved
    dws_wr16le(c2 + 8, 1);                                               // SigningAlgorithmCount
    dws_wr16le(c2 + 10, Smb2SigningAlgorithm::SMB2_SIGNING_HMAC_SHA256); // SigningAlgorithms[0]
    return total;
}

bool dws_smb2_parse_negotiate_contexts(const uint8_t *msg, size_t len, Smb2NegotiateContexts *out)
{
    if (!msg || !out)
        return false;
    memset(out, 0, sizeof(*out));
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_NEGOTIATE)
        return false;
    if (len < SMB2_HEADER_SIZE + 64)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 65) // StructureSize
        return false;
    uint16_t count = dws_rd16le(b + 6); // NegotiateContextCount (reserved for < 3.1.1)
    uint32_t off = dws_rd32le(b + 60);  // NegotiateContextOffset (from the SMB2 header start)
    if (count == 0 || off < SMB2_HEADER_SIZE)
        return false; // not a 3.1.1 response carrying a context list

    size_t p = off;
    for (uint16_t i = 0; i < count; i++)
    {
        p = (p + 7) & ~(size_t)7; // every context is 8-byte aligned
        if (p + 8 > len)
            return false;
        uint16_t ctype = dws_rd16le(msg + p);
        uint16_t dlen = dws_rd16le(msg + p + 2); // Reserved (4) follows at p+4
        size_t data = p + 8;
        if (data + dlen > len)
            return false;
        const uint8_t *d = msg + data;
        if (ctype == Smb2NegotiateContextType::SMB2_PREAUTH_INTEGRITY_CAPABILITIES && dlen >= 6)
        {
            uint16_t hcount = dws_rd16le(d + 0); // HashAlgorithmCount
            uint16_t slen = dws_rd16le(d + 2);   // SaltLength
            if (hcount >= 1 && (size_t)dlen >= 4 + (size_t)hcount * 2 + slen)
            {
                out->have_preauth = true;
                out->hash_algorithm = dws_rd16le(d + 4); // HashAlgorithms[0]
                out->salt_len = slen;
                out->salt = slen ? d + 4 + (size_t)hcount * 2 : nullptr;
            }
        }
        else if (ctype == Smb2NegotiateContextType::SMB2_SIGNING_CAPABILITIES && dlen >= 4)
        {
            uint16_t scount = dws_rd16le(d + 0); // SigningAlgorithmCount
            if (scount >= 1 && (size_t)dlen >= 2 + (size_t)scount * 2)
            {
                out->have_signing = true;
                out->signing_algorithm = dws_rd16le(d + 2); // SigningAlgorithms[0]
            }
        }
        else if (ctype == Smb2NegotiateContextType::SMB2_ENCRYPTION_CAPABILITIES && dlen >= 4)
        {
            uint16_t ccount = dws_rd16le(d + 0); // CipherCount
            if (ccount >= 1 && (size_t)dlen >= 2 + (size_t)ccount * 2)
            {
                out->have_encryption = true;
                out->cipher = dws_rd16le(d + 2); // Ciphers[0]
            }
        }
        p = data + dlen;
    }
    return true;
}

void dws_smb_preauth_init(SmbPreauth *p)
{
    if (p)
        memset(p->hash, 0, sizeof(p->hash)); // the preauth hash starts as 64 zero bytes (MS-SMB2 §3.1.5.2)
}

void dws_smb_preauth_update(SmbPreauth *p, const uint8_t *msg, size_t len)
{
    if (!p || (!msg && len))
        return;
    // hash = SHA-512(previous hash || message); chain the current value with the next handshake message.
    // Snapshot the previous hash so the digest input never aliases its output buffer.
    uint8_t prev[SMB2_PREAUTH_HASH_LEN];
    memcpy(prev, p->hash, sizeof(prev));
    DwsSha512Ctx c;
    dws_sha512_init(&c);
    dws_sha512_update(&c, prev, sizeof(prev));
    dws_sha512_update(&c, msg, len);
    dws_sha512_final(&c, p->hash);
}

size_t dws_smb2_build_session_setup(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                                    uint8_t security_mode, const uint8_t *sec_buf, size_t sec_len)
{
    const size_t body = 24; // fixed SESSION_SETUP request body (§2.2.5)
    const size_t total = SMB2_HEADER_SIZE + body + sec_len;
    if (!buf || !sec_buf || sec_len == 0 || sec_len > 0xFFFF || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 24 + sec_len) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_SESSION_SETUP, 1, message_id, 0, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    dws_wr16le(b + 0, 25); // StructureSize (fixed 24 + 1 for the variable buffer)
    b[2] = 0;              // Flags (SMB2_SESSION_FLAG_BINDING only for 3.x channel binding)
    b[3] = security_mode;  // SecurityMode (one byte here)
    // byte 4 Capabilities and byte 8 Channel stay zero
    dws_wr16le(b + 12, (uint16_t)(SMB2_HEADER_SIZE + body)); // SecurityBufferOffset (from the header start)
    dws_wr16le(b + 14, (uint16_t)sec_len);                   // SecurityBufferLength
    // byte 16 PreviousSessionId stays zero for a fresh session
    memcpy(b + body, sec_buf, sec_len);
    return total;
}

bool dws_smb2_parse_session_setup_response(const uint8_t *msg, size_t len, Smb2SessionSetupResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_SESSION_SETUP)
        return false;
    // The fixed response body is 8 bytes (StructureSize .. SecurityBufferLength), Buffer follows.
    if (len < SMB2_HEADER_SIZE + 8)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 9) // StructureSize
        return false;

    out->session_flags = dws_rd16le(b + 2);
    uint16_t sec_off = dws_rd16le(b + 4); // SecurityBufferOffset - from the start of the SMB2 header (msg)
    uint16_t sec_len = dws_rd16le(b + 6); // SecurityBufferLength
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

size_t dws_smb2_build_tree_connect(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id,
                                   const uint8_t *path_utf16, size_t path_len)
{
    const size_t body = 8; // fixed TREE_CONNECT request body (§2.2.9)
    const size_t total = SMB2_HEADER_SIZE + body + path_len;
    if (!buf || !path_utf16 || path_len == 0 || path_len > 0xFFFF || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 8 + path_len) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_TREE_CONNECT, 1, message_id, 0, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    dws_wr16le(b + 0, 9); // StructureSize
    // byte 2 Flags/Reserved stays zero
    dws_wr16le(b + 4, (uint16_t)(SMB2_HEADER_SIZE + body)); // PathOffset (from the header start) = 72
    dws_wr16le(b + 6, (uint16_t)path_len);                  // PathLength
    memcpy(b + body, path_utf16, path_len);                 // the \\server\share path (UTF-16LE)
    return total;
}

bool dws_smb2_parse_tree_connect_response(const uint8_t *msg, size_t len, Smb2TreeConnectResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_TREE_CONNECT)
        return false;
    if (len < SMB2_HEADER_SIZE + 16) // fixed 16-byte body, no variable buffer
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 16) // StructureSize
        return false;
    out->share_type = b[2];
    out->share_flags = dws_rd32le(b + 4);
    out->capabilities = dws_rd32le(b + 8);
    out->maximal_access = dws_rd32le(b + 12);
    return true;
}

size_t dws_smb2_build_create(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                             uint32_t desired_access, uint32_t share_access, uint32_t create_disposition,
                             uint32_t create_options, const uint8_t *name_utf16, size_t name_len)
{
    const size_t body = 56; // fixed CREATE request body (§2.2.13)
    const size_t total = SMB2_HEADER_SIZE + body + name_len;
    if (!buf || !name_utf16 || name_len == 0 || name_len > 0xFFFF || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 56 + name_len) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_CREATE, 1, message_id, tree_id, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    dws_wr16le(b + 0, 57); // StructureSize (fixed 56 + 1 for the variable buffer)
    // byte 2 SecurityFlags and byte 3 RequestedOplockLevel stay zero (SMB2_OPLOCK_LEVEL_NONE)
    dws_wr32le(b + 4, 2); // ImpersonationLevel = Impersonation
    // byte 8 SmbCreateFlags and byte 16 Reserved stay zero
    dws_wr32le(b + 24, desired_access);                      // DesiredAccess
    dws_wr32le(b + 28, 0);                                   // FileAttributes = 0
    dws_wr32le(b + 32, share_access);                        // ShareAccess
    dws_wr32le(b + 36, create_disposition);                  // CreateDisposition
    dws_wr32le(b + 40, create_options);                      // CreateOptions
    dws_wr16le(b + 44, (uint16_t)(SMB2_HEADER_SIZE + body)); // NameOffset (from the header start) = 120
    dws_wr16le(b + 46, (uint16_t)name_len);                  // NameLength
    // bytes 48 CreateContextsOffset and 52 CreateContextsLength stay zero
    memcpy(b + body, name_utf16, name_len);
    return total;
}

bool dws_smb2_parse_create_response(const uint8_t *msg, size_t len, Smb2CreateResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_CREATE)
        return false;
    if (len < SMB2_HEADER_SIZE + 88) // fixed 88-byte body (StructureSize .. CreateContextsLength)
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 89) // StructureSize
        return false;
    out->create_action = dws_rd32le(b + 4);
    out->end_of_file = dws_rd64le(b + 48);
    out->file_attributes = dws_rd32le(b + 56);
    memcpy(out->file_id, b + 64, 16); // FileId (persistent 8 + volatile 8)
    return true;
}

size_t dws_smb2_build_close(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                            const uint8_t file_id[16])
{
    const size_t body = 24; // fixed CLOSE request body (§2.2.15), no variable buffer
    const size_t total = SMB2_HEADER_SIZE + body;
    if (!buf || !file_id || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 24) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_CLOSE, 1, message_id, tree_id, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    dws_wr16le(b + 0, 24); // StructureSize
    // byte 2 Flags stays zero (no POSTQUERY_ATTRIB); byte 4 Reserved stays zero
    memcpy(b + 8, file_id, 16); // FileId
    return total;
}

bool dws_smb2_parse_close_response(const uint8_t *msg, size_t len, Smb2CloseResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_CLOSE)
        return false;
    if (len < SMB2_HEADER_SIZE + 60) // fixed 60-byte body
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 60) // StructureSize
        return false;
    out->end_of_file = dws_rd64le(b + 48);
    out->file_attributes = dws_rd32le(b + 56);
    return true;
}

size_t dws_smb2_build_read(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                           const uint8_t file_id[16], uint32_t length, uint64_t offset)
{
    const size_t body = 48;                           // fixed READ request body (§2.2.19)
    const size_t total = SMB2_HEADER_SIZE + body + 1; // + a 1-byte buffer (StructureSize 49 convention)
    if (!buf || !file_id || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 48 + 1) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_READ, 1, message_id, tree_id, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body + 1);
    dws_wr16le(b + 0, 49);                   // StructureSize
    b[2] = (uint8_t)(SMB2_HEADER_SIZE + 16); // Padding: requested data offset in the response (header + 16-byte body)
    // byte 3 Flags stays zero
    dws_wr32le(b + 4, length);   // Length
    dws_wr64le(b + 8, offset);   // Offset
    memcpy(b + 16, file_id, 16); // FileId
    dws_wr32le(b + 32, 1);       // MinimumCount = 1 (fail if the server returns nothing)
    // bytes 36 Channel, 40 RemainingBytes and 44/46 ReadChannelInfoOffset/Length stay zero
    // the one-byte Buffer at b+48 stays zero (already zeroed)
    return total;
}

bool dws_smb2_parse_read_response(const uint8_t *msg, size_t len, Smb2ReadResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_READ)
        return false;
    if (len < SMB2_HEADER_SIZE + 16) // fixed 16-byte body (StructureSize .. Reserved2), Buffer follows
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 17) // StructureSize
        return false;

    uint8_t data_off = b[2];               // DataOffset - from the start of the SMB2 header (msg)
    uint32_t data_len = dws_rd32le(b + 4); // DataLength
    if (data_len == 0)
    {
        out->data = nullptr;
        out->data_len = 0;
        return true;
    }
    if (data_off < SMB2_HEADER_SIZE || (size_t)data_off + data_len > len)
        return false; // data out of bounds - fail closed
    out->data = msg + data_off;
    out->data_len = data_len;
    return true;
}

size_t dws_smb2_build_write(uint8_t *buf, size_t cap, uint64_t message_id, uint64_t session_id, uint32_t tree_id,
                            const uint8_t file_id[16], const uint8_t *data, size_t data_len, uint64_t offset)
{
    const size_t body = 48; // fixed WRITE request body (§2.2.21)
    const size_t total = SMB2_HEADER_SIZE + body + data_len;
    if (!buf || !file_id || !data || data_len == 0 || data_len > 0xFFFFFFFF || cap < total)
        return 0;
    // GCOVR_EXCL_START  cap >= total (64 + 48 + data_len) >= SMB2_HEADER_SIZE was checked above, so this cannot fail
    if (dws_smb2_build_header(buf, cap, Smb2Command::SMB2_WRITE, 1, message_id, tree_id, session_id) == 0)
        return 0;
    // GCOVR_EXCL_STOP

    uint8_t *b = buf + SMB2_HEADER_SIZE;
    memset(b, 0, body);
    dws_wr16le(b + 0, 49);                                  // StructureSize
    dws_wr16le(b + 2, (uint16_t)(SMB2_HEADER_SIZE + body)); // DataOffset (from the header start) = 112
    dws_wr32le(b + 4, (uint32_t)data_len);                  // Length
    dws_wr64le(b + 8, offset);                              // Offset
    memcpy(b + 16, file_id, 16);                            // FileId
    // bytes 32 Channel, 36 RemainingBytes, 40/42 WriteChannelInfoOffset/Length and 44 Flags stay zero
    memcpy(b + body, data, data_len); // the data to write
    return total;
}

bool dws_smb2_parse_write_response(const uint8_t *msg, size_t len, Smb2WriteResp *out)
{
    if (!msg || !out)
        return false;
    Smb2Header h;
    if (!dws_smb2_parse_header(msg, len, &h) || h.command != Smb2Command::SMB2_WRITE)
        return false;
    if (len < SMB2_HEADER_SIZE + 16) // fixed 16-byte body
        return false;
    const uint8_t *b = msg + SMB2_HEADER_SIZE;
    if (dws_rd16le(b + 0) != 17) // StructureSize
        return false;
    out->count = dws_rd32le(b + 4); // Count (bytes written)
    return true;
}

// --- Message signing (MS-SMB2 §3.1.4.1 / §3.1.5.1) -------------------------
// The SMB2 sync header places the Flags field at offset 16 (LE u32) and the 16-byte Signature at
// offset 48. The MAC covers the whole message with the Signature zeroed; SMB 2.x uses HMAC-SHA256 and
// the on-wire Signature is its first 16 octets.
static constexpr size_t SMB2_FLAGS_OFF = 16;
static constexpr size_t SMB2_SIGNATURE_OFF = 48;
static constexpr size_t SMB2_SIGNATURE_LEN = 16;

void dws_smb2_sign(const uint8_t key[16], uint8_t *msg, size_t msg_len)
{
    if (!key || !msg || msg_len < SMB2_HEADER_SIZE)
        return;
    dws_wr32le(msg + SMB2_FLAGS_OFF, dws_rd32le(msg + SMB2_FLAGS_OFF) | Smb2HeaderFlags::SMB2_FLAGS_SIGNED);
    memset(msg + SMB2_SIGNATURE_OFF, 0, SMB2_SIGNATURE_LEN); // zero the Signature before hashing
    uint8_t mac[32];
    dws_hmac_sha256(key, 16, msg, msg_len, mac);
    memcpy(msg + SMB2_SIGNATURE_OFF, mac, SMB2_SIGNATURE_LEN); // Signature = first 16 octets of the MAC
}

bool dws_smb2_verify(const uint8_t key[16], uint8_t *msg, size_t msg_len)
{
    if (!key || !msg || msg_len < SMB2_HEADER_SIZE)
        return false;
    uint8_t received[SMB2_SIGNATURE_LEN];
    memcpy(received, msg + SMB2_SIGNATURE_OFF, SMB2_SIGNATURE_LEN);
    memset(msg + SMB2_SIGNATURE_OFF, 0, SMB2_SIGNATURE_LEN);
    uint8_t mac[32];
    dws_hmac_sha256(key, 16, msg, msg_len, mac);
    memcpy(msg + SMB2_SIGNATURE_OFF, received, SMB2_SIGNATURE_LEN); // restore; the message is unchanged
    uint8_t diff = 0;
    for (size_t i = 0; i < SMB2_SIGNATURE_LEN; i++)
        diff |= (uint8_t)(mac[i] ^ received[i]); // constant-time compare (no early exit)
    return diff == 0;
}

#endif // DWS_ENABLE_SMB
