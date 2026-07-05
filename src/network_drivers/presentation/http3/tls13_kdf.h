// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file tls13_kdf.h
 * @brief TLS 1.3 key schedule (RFC 8446 sec 7.1) for the QUIC handshake.
 *
 * QUIC runs TLS 1.3 as its handshake protocol (RFC 9001), and mbedTLS exposes no QUIC-TLS callback
 * API, so the handshake is hand-rolled here. This module is the key schedule: the chain of
 * HKDF-Extract and Derive-Secret steps (RFC 8446 sec 7.1) that turns the (EC)DHE shared secret and
 * the running handshake transcript hash into the traffic secrets for each encryption level, plus the
 * per-message Finished MAC (sec 4.4.4). It is cipher-suite TLS_AES_128_GCM_SHA256 only, so the hash
 * is SHA-256 throughout and every secret is 32 bytes.
 *
 * The schedule is transcript-hash-driven: each step takes a Transcript-Hash the caller computed over
 * the handshake messages so far, so this module has no dependency on the message wire formats and is
 * host-testable in isolation against the RFC 8448 sec 3 worked trace (which lists every intermediate
 * secret and the (EC)DHE input directly). The QUIC packet-protection keys ({key, iv, hp}) are then
 * derived from these traffic secrets by quic_keys_from_secret() (RFC 9001 sec 5.1).
 *
 * Pure, zero heap, host-tested against RFC 8448 sec 3.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_TLS13_KDF_H
#define DETERMINISTICESPASYNCWEBSERVER_TLS13_KDF_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_HTTP3

#include <stddef.h>
#include <stdint.h>

/** @brief SHA-256 secret length; every TLS 1.3 secret here is 32 bytes. */
#define TLS13_SECRET_LEN 32

/**
 * @brief The running key-schedule state for one handshake (server side).
 *
 * Filled in three steps as the handshake progresses: tls13_ks_early() before any (EC)DHE,
 * tls13_ks_handshake() once ClientHello..ServerHello is hashed and the shared secret is known, and
 * tls13_ks_master() once ClientHello..server Finished is hashed. Each step also derives that level's
 * client and server traffic secrets, from which quic_keys_from_secret() makes the packet keys.
 */
struct Tls13KeySchedule
{
    uint8_t early_secret[TLS13_SECRET_LEN];      ///< HKDF-Extract(0, PSK|0) - no-PSK: Extract(0, 0^32)
    uint8_t handshake_secret[TLS13_SECRET_LEN];  ///< HKDF-Extract(Derive(early,"derived"), (EC)DHE)
    uint8_t master_secret[TLS13_SECRET_LEN];     ///< HKDF-Extract(Derive(handshake,"derived"), 0^32)
    uint8_t client_hs_traffic[TLS13_SECRET_LEN]; ///< Derive-Secret(handshake, "c hs traffic", CH..SH)
    uint8_t server_hs_traffic[TLS13_SECRET_LEN]; ///< Derive-Secret(handshake, "s hs traffic", CH..SH)
    uint8_t client_ap_traffic[TLS13_SECRET_LEN]; ///< Derive-Secret(master, "c ap traffic", CH..SFIN)
    uint8_t server_ap_traffic[TLS13_SECRET_LEN]; ///< Derive-Secret(master, "s ap traffic", CH..SFIN)
};

/**
 * @brief Derive-Secret (RFC 8446 sec 7.1): HKDF-Expand-Label(secret, label, transcript_hash, 32).
 *
 * @param secret           A 32-byte PRK / traffic secret.
 * @param label            Short label without the "tls13 " prefix, e.g. "c hs traffic", "derived".
 * @param transcript_hash  Transcript-Hash of the relevant messages (32 bytes; H("") for "derived").
 * @param out              32-byte derived secret.
 */
void tls13_derive_secret(const uint8_t secret[TLS13_SECRET_LEN], const char *label,
                         const uint8_t transcript_hash[TLS13_SECRET_LEN], uint8_t out[TLS13_SECRET_LEN]);

/** @brief Step 1: early_secret = HKDF-Extract(0, 0^32) (no-PSK; the only mode we support). */
void tls13_ks_early(Tls13KeySchedule *ks);

/**
 * @brief Step 2: handshake_secret and the client/server handshake traffic secrets.
 *
 * handshake_secret = HKDF-Extract(Derive-Secret(early, "derived", H("")), @p ecdhe); the traffic
 * secrets are Derive-Secret(handshake_secret, "c hs traffic"/"s hs traffic", @p ch_sh_hash).
 *
 * @param ecdhe       The 32-byte (EC)DHE shared secret (X25519 output).
 * @param ch_sh_hash  Transcript-Hash of ClientHello..ServerHello.
 */
void tls13_ks_handshake(Tls13KeySchedule *ks, const uint8_t ecdhe[TLS13_SECRET_LEN],
                        const uint8_t ch_sh_hash[TLS13_SECRET_LEN]);

/**
 * @brief Step 3: master_secret and the client/server application traffic secrets.
 *
 * master_secret = HKDF-Extract(Derive-Secret(handshake, "derived", H("")), 0^32); the traffic
 * secrets are Derive-Secret(master_secret, "c ap traffic"/"s ap traffic", @p ch_sfin_hash).
 *
 * @param ch_sfin_hash  Transcript-Hash of ClientHello..server Finished.
 */
void tls13_ks_master(Tls13KeySchedule *ks, const uint8_t ch_sfin_hash[TLS13_SECRET_LEN]);

/**
 * @brief The Finished verify_data (RFC 8446 sec 4.4.4).
 *
 * finished_key = HKDF-Expand-Label(@p base_secret, "finished", "", 32); the output is
 * HMAC-SHA256(finished_key, @p transcript_hash). @p base_secret is the sender's handshake traffic
 * secret (server_hs_traffic for the server's Finished, client_hs_traffic to verify the client's).
 *
 * @param base_secret      The Finished sender's handshake traffic secret.
 * @param transcript_hash  Transcript-Hash of the handshake up to but excluding this Finished.
 * @param out              32-byte verify_data.
 */
void tls13_finished_mac(const uint8_t base_secret[TLS13_SECRET_LEN], const uint8_t transcript_hash[TLS13_SECRET_LEN],
                        uint8_t out[TLS13_SECRET_LEN]);

#endif // DETWS_ENABLE_HTTP3
#endif // DETERMINISTICESPASYNCWEBSERVER_TLS13_KDF_H
