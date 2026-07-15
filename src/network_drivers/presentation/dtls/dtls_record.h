// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dtls_record.h
 * @brief DTLS 1.3 record layer (RFC 9147 §4).
 *
 * The datagram counterpart to the TLS 1.3 record layer: it protects and unprotects individual
 * UDP-carried records. This is the transport-specific half of DTLS 1.3; the handshake it carries
 * reuses the TLS 1.3 crypto that already backs HTTP/3 (tls13_*, quic_hkdf, quic_aead).
 *
 * Two record shapes (RFC 9147 §4):
 *   - **DTLSPlaintext** - the classic 13-byte header (type, legacy_version, epoch, 48-bit sequence
 *     number, length, fragment). Used unencrypted for the first handshake flight and for alerts
 *     sent in epoch 0.
 *   - **DTLSCiphertext** - the compact "unified header" plus an AEAD-sealed body, used once record
 *     keys exist. The record's sequence number is itself encrypted (RFC 9147 §4.2.3), and the AEAD
 *     nonce is the TLS 1.3 construction over the full 64-bit sequence number (§4.2.2, epoch excluded).
 *
 * ─ Reuse ─
 *   AEAD (AEAD_AES_128_GCM) and the AES-128 block used for sequence-number encryption come from
 *   quic_aead; key/iv/sn derivation from quic_hkdf (HKDF-Expand-Label). Phase 1 supports the one
 *   cipher suite the whole hand-rolled TLS 1.3 stack uses: TLS_AES_128_GCM_SHA256.
 *
 * Pure, zero heap, host-tested. Not the mbedTLS TCP-TLS engine (network_drivers/tls) - this is the
 * self-contained datagram record layer.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DTLS_RECORD_H
#define DETERMINISTICESPASYNCWEBSERVER_DTLS_RECORD_H

#include "ServerConfig.h"

#if DETWS_ENABLE_DTLS

#include <stddef.h>
#include <stdint.h>

/** @name Record content types (RFC 8446 §5 / RFC 9147 §4).
 *  Shared by the DTLSPlaintext `type` field and the DTLSInnerPlaintext trailing content type. */
///@{
#define DTLS_CT_CHANGE_CIPHER_SPEC 20
#define DTLS_CT_ALERT 21
#define DTLS_CT_HANDSHAKE 22
#define DTLS_CT_APPLICATION_DATA 23
#define DTLS_CT_ACK 26 ///< DTLS 1.3 acknowledgement (RFC 9147 §7)
///@}

/** @brief DTLSPlaintext legacy_version on the wire: DTLS 1.2 (RFC 9147 §4). */
#define DTLS_LEGACY_VERSION 0xFEFD

/** @brief DTLSPlaintext header length: type(1) + version(2) + epoch(2) + seq(6) + length(2). */
#define DTLS_PLAINTEXT_HDR_LEN 13

/** @brief AEAD tag length (all supported suites: 16 bytes). */
#define DTLS_TAG_LEN 16

/** @brief Record-layer AEAD suites (phase 1: AEAD_AES_128_GCM with SHA-256). */
enum class DtlsCipher : uint8_t
{
    AES_128_GCM_SHA256 = 0
};

/**
 * @brief One direction's record-protection keys for one epoch (RFC 9147 §4).
 *
 * Derived from a TLS 1.3 traffic secret; holds the AEAD key + write IV plus the separate
 * sequence-number-encryption key. One instance per (epoch, direction).
 */
struct DtlsRecordKeys
{
    DtlsCipher cipher;  ///< negotiated AEAD (phase 1: AES-128-GCM)
    uint16_t epoch;     ///< this epoch number; its low 2 bits appear in the unified header
    uint8_t key[16];    ///< AEAD key
    uint8_t iv[12];     ///< AEAD write IV (per-record nonce = iv XOR sequence_number)
    uint8_t sn_key[16]; ///< sequence-number-encryption key
};

/**
 * @brief Derive the directional record keys from a 32-byte TLS 1.3 traffic secret.
 *
 * RFC 8446 §7.3 + RFC 9147 §4.2.3: key = HKDF-Expand-Label(secret,"key",""), iv = "iv", and the
 * sequence-number key = HKDF-Expand-Label(secret,"sn","") (all with the "tls13 " prefix).
 */
void dtls_record_keys_derive(DtlsRecordKeys *out, DtlsCipher cipher, uint16_t epoch, const uint8_t secret[32]);

// ---------------------------------------------------------------------------
// DTLSPlaintext (RFC 9147 §4): unencrypted record (initial handshake flight, alerts)
// ---------------------------------------------------------------------------

/**
 * @brief Build a DTLSPlaintext record.
 * @return total bytes written (DTLS_PLAINTEXT_HDR_LEN + @p frag_len), or 0 on overflow.
 */
size_t dtls_plaintext_build(uint8_t content_type, uint16_t epoch, uint64_t seq, const uint8_t *fragment,
                            size_t frag_len, uint8_t *out, size_t out_cap);

/** @brief Parsed view of a DTLSPlaintext record (fields point into the caller's buffer). */
struct DtlsPlaintext
{
    uint8_t content_type;
    uint16_t epoch;
    uint64_t seq;            ///< 48-bit record sequence number
    const uint8_t *fragment; ///< into the input buffer
    size_t frag_len;
};

/**
 * @brief Parse a DTLSPlaintext record, validating legacy_version and the length field.
 * @return total record length consumed (13 + length), or 0 if malformed / truncated.
 */
size_t dtls_plaintext_parse(const uint8_t *rec, size_t rec_len, DtlsPlaintext *out);

// ---------------------------------------------------------------------------
// DTLSCiphertext (RFC 9147 §4): AEAD-protected record with the unified header
// ---------------------------------------------------------------------------

/**
 * @brief Protect one record (RFC 9147 §4.2).
 *
 * Seals @p plaintext (whose inner content type is @p content_type) under @p keys at record sequence
 * number @p seq, writing a complete DTLSCiphertext to @p out: the unified header (C=0 no connection
 * id, S=1 16-bit sequence number, L=1 length present, low 2 epoch bits), the AEAD-sealed body, and
 * the encrypted sequence number. The AEAD nonce is iv XOR seq; the associated data is the unified
 * header carrying the *plaintext* sequence number (before §4.2.3 encryption).
 *
 * @return bytes written, or 0 on overflow / unsupported cipher.
 */
size_t dtls_ciphertext_protect(const DtlsRecordKeys *keys, uint64_t seq, uint8_t content_type, const uint8_t *plaintext,
                               size_t pt_len, uint8_t *out, size_t out_cap);

/** @brief Result of a successful @ref dtls_ciphertext_unprotect. */
struct DtlsCiphertext
{
    uint8_t content_type; ///< recovered inner content type (last non-zero byte of the inner plaintext)
    uint16_t epoch;       ///< epoch of @p keys (its low 2 bits matched the header)
    uint64_t seq;         ///< reconstructed full sequence number
    size_t pt_len;        ///< plaintext bytes written to @p out
};

/**
 * @brief Unprotect one received DTLSCiphertext record (RFC 9147 §4.2).
 *
 * Decrypts the sequence number, reconstructs the full sequence number from @p next_seq (the expected
 * next value for this epoch, RFC 9147 §4.2.2 / RFC 9000 Appendix A.3), opens the AEAD, and strips the
 * inner content type and any zero padding. @p keys must be the epoch whose low 2 bits match the
 * record header (verified). Connection-ID records (header C bit set) are rejected - CID is not
 * negotiated in this phase.
 *
 * @return true on success (@p out / @p info filled); false on a malformed header, an epoch-bit
 *         mismatch, a connection-id record, a failed AEAD tag, or an output overflow.
 */
bool dtls_ciphertext_unprotect(const DtlsRecordKeys *keys, uint64_t next_seq, const uint8_t *rec, size_t rec_len,
                               uint8_t *out, size_t out_cap, DtlsCiphertext *info);

// ---------------------------------------------------------------------------
// Anti-replay sliding window (RFC 9147 §4.5.1)
// ---------------------------------------------------------------------------

/** @brief 64-record sliding replay window over the highest sequence number accepted in an epoch. */
struct DtlsReplayWindow
{
    uint64_t highest; ///< highest accepted sequence number (bit 0 of @ref bitmap)
    uint64_t bitmap;  ///< bit i set => (highest - i) has been accepted
    bool seeded;      ///< false until the first record is accepted
};

/** @brief Reset a replay window to empty. */
void dtls_replay_init(DtlsReplayWindow *w);

/**
 * @brief Test whether @p seq may be accepted (new and within the window).
 * @return true if @p seq is new and in-window; false if it is a replay or older than the window.
 */
bool dtls_replay_check(const DtlsReplayWindow *w, uint64_t seq);

/** @brief Record @p seq as accepted, advancing the window. Call only after a successful deprotect. */
void dtls_replay_mark(DtlsReplayWindow *w, uint64_t seq);

#endif // DETWS_ENABLE_DTLS
#endif // DETERMINISTICESPASYNCWEBSERVER_DTLS_RECORD_H
