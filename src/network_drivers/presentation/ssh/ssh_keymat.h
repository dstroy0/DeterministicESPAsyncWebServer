// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_keymat.h
 * @brief SSH session key material - types, pools, and security model.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY MODEL - READ BEFORE MODIFYING ANYTHING IN THIS FILE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The fundamental threat this layout defends against is **buffer-overflow
 * key extraction**: an attacker who can cause an out-of-bounds read or write
 * in the packet receive path (ssh_pool[].pkt_buf) should not be able to
 * reach AES session keys or HMAC keys in the same memory operation.
 *
 * DEFENSE 1 - Physical BSS separation
 * ─────────────────────────────────────
 * All three pools are SEPARATE static symbols:
 *
 *   ssh_pool[MAX_SSH_CONNS]   - packet assembly buffers, protocol state
 *   ssh_keys[MAX_SSH_CONNS]   - AES-256 key schedules + HMAC keys
 *   ssh_dh[MAX_SSH_CONNS]     - ephemeral DH scalar (y), server public (f), K
 *
 * The linker places these as independent objects.  An overflow inside
 * pkt_buf must cross the entire ssh_pool[] symbol, then any objects the
 * linker placed between it and ssh_keys[], before reaching key material.
 * On ESP32 with the default linker script this is a different RAM region
 * than a linear overflow from pkt_buf would reach.
 *
 * An attacker relying on a single linear write cannot bridge both gaps in
 * one step.  This is not mitigation-by-obscurity - it is the same "separate
 * key store" principle used by HSMs, but implemented in software via linker
 * symbol separation.
 *
 * DEFENSE 2 - RSA host private key is NEVER stored in static memory
 * ──────────────────────────────────────────────────────────────────
 * The RSA-2048 private key (d, p, q, dp, dq, qInv) is loaded from NVS
 * (encrypted flash on ESP32) into a LOCAL STACK FRAME inside
 * ssh_rsa_sign().  It is explicitly zeroed (via volatile memset, which
 * the compiler cannot elide) before ssh_rsa_sign() returns.
 *
 * Consequences:
 *   - A static memory scan never finds the private key.
 *   - Cold-boot attacks recover only the public key from BSS.
 *   - The exposure window is the duration of a single mbedtls_rsa_pkcs1_sign
 *     call, typically < 1 ms.
 *
 * DEFENSE 3 - DH ephemeral scalar zeroing
 * ─────────────────────────────────────────
 * y (2048-bit private DH scalar) lives in ssh_dh[slot].y.  After
 * ssh_dh_finish() derives K it calls ssh_wipe() on the entire SshDhState
 * struct, which uses a volatile loop to zero all 801 bytes including y,
 * f, and K.  K must be zeroed after session keys are derived from it
 * (RFC 4253 §7.2 makes no requirement, but it reduces long-term exposure).
 *
 * DEFENSE 4 - crypto_work scratch buffer zeroing
 * ────────────────────────────────────────────────
 * crypto_work[SSH_CRYPTO_WORK_SIZE] is used for Montgomery multiplication
 * temporaries (up to 516 bytes of intermediate products that contain
 * combinations of y, K, and d fragments).  It is zeroed via ssh_wipe()
 * immediately after every call to bn_expmod_group14() or ssh_rsa_sign().
 *
 * DEFENSE 5 - MAC-verify-before-use (all packet input)
 * ──────────────────────────────────────────────────────
 * After key exchange, every inbound SSH binary packet is:
 *   1. Received into pkt_buf.
 *   2. Decrypted in place (AES-256-CTR).
 *   3. HMAC-SHA2-256 verified over (seq_num_be32 || plaintext_packet).
 *   4. ONLY THEN forwarded to the protocol handler.
 *
 * If HMAC verification fails the connection is closed immediately.  No
 * plaintext bytes are acted upon before the MAC is confirmed valid.
 * This prevents padding oracle and chosen-ciphertext attacks.
 *
 * DEFENSE 6 - Sequence number overflow guard
 * ────────────────────────────────────────────
 * RFC 4253 §9.3.4 requires rekeying before the sequence number wraps.
 * If seq_c2s or seq_s2c reaches 0xFFFFFFFF the connection is closed.
 * (Rekeying is not yet implemented; close-on-wrap is the safe fallback.)
 *
 * WHAT THIS DOES NOT PROTECT AGAINST
 * ────────────────────────────────────
 *   - An attacker with arbitrary-read in the process can still read
 *     ssh_keys[] - physical separation only raises the bar, not a hard wall.
 *   - Timing side-channels in the software Montgomery/AES paths are not
 *     addressed.  On ESP32 the hardware AES path (mbedtls) has
 *     implementation-level timing properties that are mbedtls's concern.
 *   - Cold-boot attacks on SRAM after power-loss are partially mitigated by
 *     the zeroing policies above, but ESP32 SRAM may retain data briefly.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_KEYMAT_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_KEYMAT_H

#include "DetWebServerConfig.h"
#include "ssh_aes256ctr.h"
#include "ssh_bignum.h"
#include "ssh_chachapoly.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/** @brief Negotiated bulk cipher for a session. */
enum
{
    SSH_CIPHER_AES256CTR = 0,        ///< aes256-ctr + a separate HMAC (the fallback)
    SSH_CIPHER_CHACHA20POLY1305 = 1, ///< chacha20-poly1305@openssh.com (AEAD; no separate MAC)
};

// ---------------------------------------------------------------------------
// Secure wipe
// ---------------------------------------------------------------------------

/**
 * @brief Zero @p len bytes at @p ptr using a volatile loop the compiler
 *        cannot optimize away.
 *
 * Use this (not memset) for any buffer that contains key material.
 * C compilers are permitted to elide a plain memset() whose result is
 * "not observed" - a common optimization that silently skips zeroing of
 * local key buffers before they go out of scope.  The volatile write
 * forces the store to happen even if the memory is never read again.
 *
 * @param ptr  Buffer to wipe.
 * @param len  Number of bytes to zero.
 */
static inline void ssh_wipe(void *ptr, size_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0; i < len; i++)
        p[i] = 0;
}

// ---------------------------------------------------------------------------
// Session key material  (one entry per SSH connection)
// ---------------------------------------------------------------------------

/**
 * @brief AES-256-CTR + HMAC-SHA2-256 session keys for one SSH connection.
 *
 * This struct occupies a separate BSS symbol (ssh_keys[]) from the packet
 * receive buffer (ssh_pool[].pkt_buf).  See the security model at the top
 * of this file for why that separation matters.
 *
 * Key derivation follows RFC 4253 §7.2.  After the DH exchange hash H is
 * known and K is available, six values are derived:
 *
 *   IV_c2s  = SHA256(K || H || "A" || session_id)   [16 bytes]
 *   IV_s2c  = SHA256(K || H || "B" || session_id)   [16 bytes]
 *   key_c2s = SHA256(K || H || "C" || session_id)   [32 bytes]
 *   key_s2c = SHA256(K || H || "D" || session_id)   [32 bytes]
 *   mac_c2s = SHA256(K || H || "E" || session_id)   [32 bytes]
 *   mac_s2c = SHA256(K || H || "F" || session_id)   [32 bytes]
 *
 * c2s_ctx is initialized with key_c2s + IV_c2s (client-to-server, server decrypts).
 * s2c_ctx is initialized with key_s2c + IV_s2c (server-to-client, server encrypts).
 */
struct SshKeyMat
{
    SshAesCtrCtx c2s_ctx; ///< Client→server cipher (AES-256-CTR); server decrypts inbound with it.
    SshAesCtrCtx s2c_ctx; ///< Server→client cipher (AES-256-CTR); server encrypts outbound with it.

    uint8_t mac_key_c2s[32]; ///< HMAC-SHA2-256 key, client-to-server direction (aes256-ctr mode).
    uint8_t mac_key_s2c[32]; ///< HMAC-SHA2-256 key, server-to-client direction (aes256-ctr mode).

    uint8_t cipher_mode; ///< SSH_CIPHER_* selected for this session (0 = aes256-ctr).
    // chacha20-poly1305@openssh.com: 512-bit key per direction (K_main || K_header); no IV, no MAC key.
    uint8_t chacha_key_c2s[SSH_CHACHAPOLY_KEY_LEN]; ///< client-to-server, used only in chacha mode.
    uint8_t chacha_key_s2c[SSH_CHACHAPOLY_KEY_LEN]; ///< server-to-client, used only in chacha mode.

    bool active; ///< True once keys are installed after successful KEX.
};

/**
 * @brief Pool of session key material, one entry per MAX_SSH_CONNS.
 *
 * Separate BSS symbol from ssh_pool[] - see security model.
 * Zeroed on connection close by ssh_keymat_wipe(slot).
 */
extern SshKeyMat ssh_keys[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// DH ephemeral state  (one entry per SSH connection, zeroed after KEX)
// ---------------------------------------------------------------------------

/**
 * @brief Ephemeral Diffie-Hellman state for one SSH connection.
 *
 * The three SshBigNum fields (y, f, K) together hold 768 bytes of sensitive
 * material.  The entire struct is wiped by ssh_dh_wipe() immediately after
 * session keys are derived from K.
 *
 * FIELD LIFETIME:
 *   y  - generated by ssh_dh_generate(); zeroed in ssh_dh_wipe().
 *   f  - computed in ssh_dh_generate() as g^y mod p; sent in KEXDH_REPLY;
 *         zeroed in ssh_dh_wipe().
 *   K  - computed in ssh_dh_finish() as e^y mod p; used for key derivation;
 *         zeroed in ssh_dh_wipe() AFTER keys are installed.
 *   H  - the exchange hash; becomes the session_id for the connection's
 *         lifetime (RFC 4253 §7.2); stored in H[], NOT zeroed (it is a
 *         commitment to the handshake, and is not secret).
 */
struct SshDhState
{
    SshBigNum y; ///< Server ephemeral private DH scalar (SENSITIVE - wiped after KEX).
    SshBigNum f; ///< Server DH public value = g^y mod p (sent to client).
    SshBigNum K; ///< Shared DH secret = e^y mod p (SENSITIVE - wiped after key derivation).

    uint8_t H[32]; ///< SHA-256 exchange hash; doubles as session_id after first KEX.
    bool kex_done; ///< True once NEWKEYS has been sent and received.
};

/** @brief Pool of ephemeral DH state, one entry per MAX_SSH_CONNS. */
extern SshDhState ssh_dh[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// Wipe helpers
// ---------------------------------------------------------------------------

/** @brief Zero all key material for slot @p i on disconnect or KEX failure. */
static inline void ssh_keymat_wipe(uint8_t i)
{
    if (i < MAX_SSH_CONNS)
        ssh_wipe(&ssh_keys[i], sizeof(SshKeyMat));
}

/** @brief Zero the ephemeral DH state for slot @p i after keys are derived. */
static inline void ssh_dh_wipe(uint8_t i)
{
    if (i < MAX_SSH_CONNS)
        ssh_wipe(&ssh_dh[i], sizeof(SshDhState));
}

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_KEYMAT_H
