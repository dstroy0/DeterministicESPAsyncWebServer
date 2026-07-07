// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file tls13_msg.h
 * @brief TLS 1.3 handshake messages for the QUIC handshake (RFC 8446 sec 4).
 *
 * The wire formats of the handshake messages a QUIC server exchanges: it parses the client's
 * ClientHello and builds its own flight (ServerHello, then the encrypted EncryptedExtensions,
 * Certificate, CertificateVerify, and Finished). Every message is emitted whole, including the
 * 4-byte handshake header (msg_type + 24-bit length), because that is what both the CRYPTO stream
 * carries and the transcript hash covers.
 *
 * This server is deliberately a single, spec-valid profile: cipher suite TLS_AES_128_GCM_SHA256,
 * key share X25519, and an Ed25519 certificate (the only signature scheme we produce - the in-tree
 * crypto has Ed25519 but no ECDSA P-256 or RSA-PSS). A ClientHello that offers none of these is a
 * handshake failure, decided by the state machine that drives this module. QUIC transport parameters
 * ride in the quic_transport_parameters extension (codepoint 0x39, RFC 9001 sec 8.2).
 *
 * Pure, zero heap, host-tested against the RFC 8448 sec 3 ServerHello / Certificate / Finished bytes
 * and by ClientHello field extraction + an Ed25519 CertificateVerify sign/verify round-trip.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TLS13_MSG_H
#define DETERMINISTICESPASYNCWEBSERVER_TLS13_MSG_H

#include "ServerConfig.h"

#if DETWS_ENABLE_HTTP3

#include <stddef.h>
#include <stdint.h>

/** @brief TLS handshake message types (RFC 8446 sec 4). */
enum
{
    TLS_HS_CLIENT_HELLO = 1,
    TLS_HS_SERVER_HELLO = 2,
    TLS_HS_ENCRYPTED_EXTENSIONS = 8,
    TLS_HS_CERTIFICATE = 11,
    TLS_HS_CERTIFICATE_VERIFY = 15,
    TLS_HS_FINISHED = 20,
};

#define TLS_CIPHER_AES_128_GCM_SHA256 0x1301 ///< the one cipher suite we support
#define TLS_GROUP_X25519 0x001d              ///< the one key-exchange group we support
#define TLS_SIG_ED25519 0x0807               ///< the one signature scheme we produce
#define TLS_VERSION_1_3 0x0304               ///< supported_versions selected value
#define TLS_EXT_QUIC_TRANSPORT_PARAMS 0x0039 ///< quic_transport_parameters (RFC 9001 sec 8.2)

/** @brief What the state machine needs out of a parsed ClientHello (pointers alias the input). */
struct Tls13ClientHello
{
    const uint8_t *session_id; ///< legacy_session_id (echoed back in ServerHello)
    uint8_t session_id_len;
    uint8_t client_x25519[32]; ///< the client's X25519 key_share (valid iff has_key_share)
    bool has_key_share;
    bool offers_tls13;      ///< supported_versions contains 0x0304
    bool offers_x25519;     ///< supported_groups contains x25519
    bool offers_ed25519;    ///< signature_algorithms contains ed25519
    bool offers_h3_alpn;    ///< ALPN contains "h3"
    const uint8_t *quic_tp; ///< raw quic_transport_parameters extension body (or NULL)
    size_t quic_tp_len;
    const uint8_t *sni; ///< first server_name host_name (or NULL), not NUL-terminated
    size_t sni_len;
};

/**
 * @brief Parse a ClientHello handshake message (@p msg includes the 4-byte handshake header).
 * @return false if it is not a well-formed ClientHello. Missing/!supported extensions are reported
 * through the offers_* flags rather than failing the parse, so the caller can send the right alert.
 */
bool tls13_parse_client_hello(const uint8_t *msg, size_t len, Tls13ClientHello *out);

/**
 * @brief Build a ServerHello (RFC 8446 sec 4.1.3) selecting TLS 1.3 / AES-128-GCM-SHA256 / X25519.
 *
 * @param random          32-byte server random.
 * @param session_id      legacy_session_id_echo (the client's, echoed verbatim; may be NULL if len 0).
 * @param session_id_len  echoed session-id length (0..32).
 * @param server_x25519   the server's 32-byte X25519 public key_share.
 * @return bytes written, or 0 on overflow.
 */
size_t tls13_build_server_hello(uint8_t *out, size_t cap, const uint8_t random[32], const uint8_t *session_id,
                                uint8_t session_id_len, const uint8_t server_x25519[32]);

/**
 * @brief Build EncryptedExtensions (RFC 8446 sec 4.3.1) carrying ALPN "h3" and the server's
 * quic_transport_parameters (@p quic_tp, from quic_tp_encode). @return bytes written, or 0.
 */
size_t tls13_build_encrypted_extensions(uint8_t *out, size_t cap, const uint8_t *quic_tp, size_t quic_tp_len);

/**
 * @brief Build a Certificate message (RFC 8446 sec 4.4.2) with an empty request context and one
 * CertificateEntry wrapping @p cert_der (DER X.509) with no entry extensions. @return bytes written.
 */
size_t tls13_build_certificate(uint8_t *out, size_t cap, const uint8_t *cert_der, size_t cert_len);

/**
 * @brief Build a CertificateVerify (RFC 8446 sec 4.4.3) with an Ed25519 signature.
 *
 * Signs the sec 4.4.3 content - 64 * 0x20, the context string "TLS 1.3, server CertificateVerify",
 * a 0x00 separator, then @p transcript_hash (Transcript-Hash of ClientHello..Certificate) - with the
 * Ed25519 key @p seed, and emits algorithm=ed25519, the 64-byte signature.
 *
 * @param transcript_hash  32-byte Transcript-Hash through the Certificate message.
 * @param seed             32-byte Ed25519 private seed.
 * @return bytes written, or 0 on overflow.
 */
size_t tls13_build_cert_verify(uint8_t *out, size_t cap, const uint8_t transcript_hash[32], const uint8_t seed[32]);

/**
 * @brief Build a Finished message (RFC 8446 sec 4.4.4) carrying @p verify_data (from
 * tls13_finished_mac). @return bytes written, or 0 on overflow.
 */
size_t tls13_build_finished(uint8_t *out, size_t cap, const uint8_t verify_data[32]);

/**
 * @brief Assemble the sec 4.4.3 signed content into @p out (64*0x20 || context || 0x00 || hash).
 *
 * Exposed so the state machine can also verify a client's CertificateVerify if client auth is ever
 * added, and so it is directly unit-testable. @p is_server picks the "server"/"client" context word.
 * @return content length written (always 98 + 32), or 0 on overflow.
 */
size_t tls13_cert_verify_content(uint8_t *out, size_t cap, const uint8_t transcript_hash[32], bool is_server);

#endif // DETWS_ENABLE_HTTP3
#endif // DETERMINISTICESPASYNCWEBSERVER_TLS13_MSG_H
