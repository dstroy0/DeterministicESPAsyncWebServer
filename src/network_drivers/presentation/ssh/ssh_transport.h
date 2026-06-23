// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_transport.h
 * @brief SSH transport-layer protocol state machine (RFC 4253).
 *
 * Sits on top of the binary packet layer (ssh_packet.*) and the crypto
 * primitives (ssh_dh, ssh_rsa, ssh_aes256ctr, ssh_hmac_sha256). Drives the
 * handshake: identification-string (banner) exchange → algorithm negotiation
 * (KEXINIT) → Diffie-Hellman key exchange (KEXDH) → NEWKEYS → key install,
 * then hands off to the user-auth layer (ssh_auth.*).
 *
 * ── Supported algorithms (single choice per category) ──────────────────────
 *   kex            : diffie-hellman-group14-sha256   (RFC 8268)
 *   host key / sig : rsa-sha2-256                     (RFC 8332)
 *   cipher (both)  : aes256-ctr                       (RFC 4344)
 *   MAC (both)     : hmac-sha2-256                    (RFC 6668)
 *   compression    : none
 *
 * Negotiation accepts the connection only if the client offers each of these.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_TRANSPORT_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_TRANSPORT_H

#include "DetWebServerConfig.h"
#include "ssh_keymat.h"
#include "ssh_sha256.h"
#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Sizing
// ---------------------------------------------------------------------------

/** @brief Max stored length of an SSH identification string (RFC 4253 §4.2: 255). */
#define SSH_VERSION_MAX 256

/** @brief Max stored size of a KEXINIT payload (for exchange-hash reconstruction). */
#define SSH_KEXINIT_MAX 512

/** @brief Server identification string (no CR LF; appended on the wire). */
#define SSH_SERVER_VERSION "SSH-2.0-DetWS_1.0"

/**
 * @brief Re-key when either packet sequence number reaches this value.
 *
 * RFC 4253 §9 recommends re-keying after ~1 GB or one hour. A packet-count
 * proxy is used here; the default is well below SSH_SEQ_CLOSE_THRESHOLD so a
 * re-key always happens before the sequence number can wrap.
 */
#ifndef SSH_REKEY_PACKET_THRESHOLD
#define SSH_REKEY_PACKET_THRESHOLD 0x40000000u
#endif

// ---------------------------------------------------------------------------
// Handshake phase
// ---------------------------------------------------------------------------

/** @brief SSH connection lifecycle phase. */
enum SshPhase
{
    SSH_PHASE_BANNER,  ///< Awaiting the client identification string.
    SSH_PHASE_KEXINIT, ///< Awaiting the client KEXINIT.
    SSH_PHASE_DH_INIT, ///< Awaiting SSH_MSG_KEXDH_INIT.
    SSH_PHASE_NEWKEYS, ///< Awaiting SSH_MSG_NEWKEYS.
    SSH_PHASE_SERVICE, ///< Awaiting SERVICE_REQUEST ("ssh-userauth").
    SSH_PHASE_AUTH,    ///< User authentication in progress (RFC 4252).
    SSH_PHASE_OPEN     ///< Authenticated; connection/channel protocol active.
};

// ---------------------------------------------------------------------------
// Per-connection transport state
// ---------------------------------------------------------------------------

/**
 * @brief SSH transport/session state for one connection (BSS pool).
 *
 * Holds the handshake phase plus the few values that must persist across
 * messages to compute the exchange hash H: the client and server
 * identification strings (V_C, V_S) and the two KEXINIT payloads (I_C, I_S).
 * The exchange hash from the first KEX is retained as the session id, which
 * is required for key derivation and for every later re-key.
 */
struct SshSession
{
    SshPhase phase; ///< Current handshake phase.

    char v_c[SSH_VERSION_MAX]; ///< Client identification string (no CR LF).
    uint16_t v_c_len;          ///< Length of v_c.

    uint8_t banner_buf[SSH_VERSION_MAX]; ///< Accumulator for the inbound banner.
    uint16_t banner_len;                 ///< Bytes buffered in banner_buf.

    uint8_t i_c[SSH_KEXINIT_MAX]; ///< Client KEXINIT payload (for H).
    uint16_t i_c_len;             ///< Length of i_c.
    uint8_t i_s[SSH_KEXINIT_MAX]; ///< Server KEXINIT payload (for H).
    uint16_t i_s_len;             ///< Length of i_s.

    uint8_t session_id[SSH_SHA256_DIGEST_LEN]; ///< H from the first KEX (RFC 4253 §7.2).
    bool have_session_id;                      ///< True once the first KEX completes.

    bool authed;           ///< True after successful user authentication.
    uint8_t auth_failures; ///< Failed USERAUTH_REQUESTs (brute-force limit, RFC 4252 §4).
};

/** @brief Static pool of SSH session state (BSS), one per SSH slot. */
extern SshSession ssh_sess[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

/**
 * @brief Reset transport state for slot @p i to the start of a handshake.
 */
void ssh_transport_init(uint8_t i);

/**
 * @brief Write the server identification string ("SSH-2.0-…\r\n") to @p out.
 *
 * Sent verbatim (not inside a binary packet) at connection start, before any
 * KEXINIT. The CR LF is included on the wire but excluded from V_S used in H.
 *
 * @return 0 on success, -1 if @p cap is too small.
 */
int ssh_transport_server_banner(uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Feed raw bytes while awaiting the client identification string.
 *
 * Accumulates bytes until a CR LF (or bare LF) terminates a line beginning
 * with "SSH-". Earlier non-SSH lines (allowed by RFC 4253 §4.2) are skipped.
 * On completion the client version (without CR LF) is stored in v_c.
 *
 * @param[in]  i         SSH slot.
 * @param[in]  data      Inbound bytes.
 * @param[in]  len       Number of bytes in @p data.
 * @param[out] consumed  Bytes consumed from @p data (the banner may be followed
 *                       immediately by binary packets).
 * @return 1 when the banner is complete, 0 if more data is needed, -1 on error.
 */
int ssh_transport_recv_banner(uint8_t i, const uint8_t *data, size_t len, size_t *consumed);

/**
 * @brief Build the server KEXINIT payload for slot @p i (RFC 4253 §7.1).
 *
 * Stores a copy in ssh_sess[i].i_s for later exchange-hash computation.
 *
 * @return 0 on success, -1 on buffer overflow.
 */
int ssh_kexinit_build(uint8_t i, uint8_t *payload, size_t *len, size_t cap);

/**
 * @brief Parse and negotiate the client KEXINIT payload (RFC 4253 §7.1).
 *
 * Stores a copy in ssh_sess[i].i_c. Verifies the client offers every algorithm
 * this server supports (one per category).
 *
 * @return 0 if negotiation succeeds, -1 if any required algorithm is absent or
 *         the payload is malformed.
 */
int ssh_kexinit_parse(uint8_t i, const uint8_t *payload, size_t len);

/**
 * @brief Compute the SSH exchange hash H (RFC 4253 §8).
 *
 *   H = SHA256( string(V_C) || string(V_S) || string(I_C) || string(I_S)
 *               || string(K_S) || mpint(e) || mpint(f) || mpint(K) )
 *
 * V_C/V_S/I_C/I_S are taken from ssh_sess[i]; the rest are supplied. The
 * 256-byte big-endian integers are re-encoded as SSH mpints (minimal length,
 * leading 0x00 when the high bit is set).
 *
 * @param[in]  i      SSH slot.
 * @param[in]  e_be   Client DH public value e (256-byte big-endian).
 * @param[in]  f_be   Server DH public value f (256-byte big-endian).
 * @param[in]  k_be   Shared secret K (256-byte big-endian).
 * @param[in]  ks     Server host-key blob K_S.
 * @param[in]  ks_len Length of @p ks.
 * @param[out] out    32-byte exchange hash.
 * @return 0 on success, -1 on bad slot.
 */
int ssh_kex_exchange_hash(uint8_t i, const uint8_t *e_be, const uint8_t *f_be, const uint8_t *k_be, const uint8_t *ks,
                          size_t ks_len, uint8_t out[SSH_SHA256_DIGEST_LEN]);

/**
 * @brief Parse SSH_MSG_KEXDH_INIT, extracting the client DH value e.
 *
 * Payload: byte(30) || mpint(e). The mpint is normalized into a fixed 256-byte
 * big-endian buffer (leading sign/zero bytes stripped, right-aligned).
 *
 * @param[in]  payload  KEXDH_INIT payload.
 * @param[in]  len      Payload length.
 * @param[out] e_be     256-byte big-endian client public value.
 * @return 0 on success, -1 if malformed or e exceeds 2048 bits.
 */
int ssh_kexdh_parse_init(const uint8_t *payload, size_t len, uint8_t e_be[256]);

/**
 * @brief Build SSH_MSG_KEXDH_REPLY (RFC 4253 §8, RFC 8332 §3).
 *
 * Payload: byte(31) || string(K_S) || mpint(f) || string(signature), where the
 * signature blob is string("rsa-sha2-256") || string(@p sig).
 *
 * @param[in]  ks       Server host-key blob K_S.
 * @param[in]  ks_len   Length of @p ks.
 * @param[in]  f_be     Server DH public value f (256-byte big-endian).
 * @param[in]  sig      Raw RSA signature over the exchange hash H.
 * @param[in]  sig_len  Length of @p sig (256 for RSA-2048).
 * @param[out] out      Output payload buffer.
 * @param[out] out_len  Bytes written.
 * @param[in]  cap      Capacity of @p out.
 * @return 0 on success, -1 on overflow.
 */
int ssh_kexdh_build_reply(const uint8_t *ks, size_t ks_len, const uint8_t *f_be, const uint8_t *sig, size_t sig_len,
                          uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Handle SSH_MSG_KEXDH_INIT end-to-end and produce the reply payload.
 *
 * Validates e, computes the shared secret K = e^y mod p, builds the exchange
 * hash H, signs H with the RSA host key, assembles SSH_MSG_KEXDH_REPLY, and
 * derives the six session keys (installed into ssh_keys[i]; encryption is not
 * activated until NEWKEYS - see ssh_newkeys_complete()). On the first KEX the
 * exchange hash is saved as the session id. K is wiped from the stack before
 * returning.
 *
 * Requires ssh_dh_generate(i) and ssh_rsa_load_pubkey() to have been called.
 *
 * @param[in]  i          SSH slot.
 * @param[in]  payload    KEXDH_INIT payload.
 * @param[in]  len        Payload length.
 * @param[out] reply_out  KEXDH_REPLY payload buffer.
 * @param[out] reply_len  Bytes written to @p reply_out.
 * @param[in]  cap        Capacity of @p reply_out.
 * @return 0 on success, -1 on validation/crypto/buffer error.
 */
int ssh_kexdh_handle(uint8_t i, const uint8_t *payload, size_t len, uint8_t *reply_out, size_t *reply_len, size_t cap);

/**
 * @brief Complete the NEWKEYS exchange: activate encryption and advance phase.
 *
 * Called once the client's SSH_MSG_NEWKEYS has been received (the server having
 * already sent its own). Flips ssh_pkt[i] into encrypted mode and moves to
 * SSH_PHASE_SERVICE (awaiting the ssh-userauth service request).
 */
void ssh_newkeys_complete(uint8_t i);

/**
 * @brief True if slot @p i has reached the re-key threshold (RFC 4253 §9).
 *
 * Checks both packet sequence numbers against SSH_REKEY_PACKET_THRESHOLD.
 */
bool ssh_rekey_needed(uint8_t i);

/**
 * @brief Begin a server-initiated re-key by emitting a fresh KEXINIT.
 *
 * Generates a new ephemeral DH key pair, builds and stores a new server
 * KEXINIT (I_S), and returns the transport to SSH_PHASE_KEXINIT. The session
 * id and authentication state are preserved, so once the re-key completes the
 * connection resumes in its prior (authenticated) phase.
 *
 * @param[in]  i        Connection slot index.
 * @param[out] out      KEXINIT payload to send.
 * @param[out] out_len  Bytes written.
 * @param[in]  cap      Capacity of @p out.
 * @return 0 on success, -1 on error.
 */
int ssh_transport_begin_rekey(uint8_t i, uint8_t *out, size_t *out_len, size_t cap);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_TRANSPORT_H
