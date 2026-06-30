// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_dh.cpp
 * @brief DH-group14-SHA256 key exchange implementation.
 */

#include "network_drivers/presentation/ssh/ssh_dh.h"

// ---------------------------------------------------------------------------
// RNG
// ---------------------------------------------------------------------------

void ssh_rng_fill(uint8_t *buf, size_t len)
{
    esp_fill_random(buf, len);
}

// ---------------------------------------------------------------------------
// DH key generation
// ---------------------------------------------------------------------------

int ssh_dh_generate(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshDhState *dh = &ssh_dh[i];

    // Generate a random 2048-bit private scalar y.
    // RFC 4253 §8 does not specify a minimum bit-length for y beyond requiring
    // it to be in [1, p-1].  Common practice is a full 2048-bit random value,
    // which ensures the discrete-log is as hard as the group order.
    ssh_rng_fill((uint8_t *)dh->y.d, sizeof(SshBigNum));

    // Ensure y < p by clearing the two MSBs (conservative; not strictly
    // required since rejection sampling would also work, but a single mask
    // is sufficient because p ≈ 2^2048 and clearing 2 bits keeps y in range).
    dh->y.d[SSH_BN_LIMBS - 1] &= 0x3FFFFFFFu;
    // Also ensure y > 1 (set bit 1 of the LSB limb to avoid pathological y=0,1).
    dh->y.d[0] |= 0x00000002u;

    // f = g^y mod p  (g = 2 for group-14)
    bn_expmod_group14(&dh->f, &group14_g, &dh->y);

    dh->kex_done = false;
    return 0;
}

// ---------------------------------------------------------------------------
// DH finish: compute K, build exchange hash, derive session keys
// ---------------------------------------------------------------------------

int ssh_dh_finish(uint8_t i, const uint8_t e_be[256], const uint8_t *hash_input, size_t hi_len)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshDhState *dh = &ssh_dh[i];

    // Parse and validate client public value e.
    SshBigNum e;
    bn_from_bytes(&e, e_be, 256);
    if (bn_dh_validate(&e) != 0)
        return -1; // close connection

    // K = e^y mod p
    bn_expmod_group14(&dh->K, &e, &dh->y);

    // Build exchange hash H = SHA256(hash_input).
    // hash_input is assembled by the SSH packet layer and contains:
    //   mpint(K) || byte[32](H fields) from RFC 4253 §8 ordering.
    // The full pre-image (V_C, V_S, I_C, I_S, K_S, e, f, K) is encoded
    // by the caller into hash_input before calling here.
    ssh_sha256(hash_input, hi_len, dh->H);

    // Derive session keys (AES + HMAC) from K and H.
    uint8_t K_be[256];
    bn_to_bytes(K_be, &dh->K);
    ssh_dh_derive_keys(i, K_be, dh->H);

    // Zero sensitive material: y (private scalar), K (shared secret),
    // and the K_be stack buffer.
    ssh_wipe(dh->y.d, sizeof(SshBigNum));
    ssh_wipe(dh->K.d, sizeof(SshBigNum));
    ssh_wipe(K_be, 256);
    // f and H are retained: f was already sent; H is the session_id.

    dh->kex_done = true;
    return 0;
}

// ---------------------------------------------------------------------------
// Session key derivation (RFC 4253 §7.2)
// ---------------------------------------------------------------------------

// Hash the shared secret K as an SSH mpint into @p ctx (RFC 4251 §5 / RFC 4253
// §7.2): big-endian, 4-byte length prefix, a leading 0x00 only if the MSB is set,
// and all UNNECESSARY leading 0x00 bytes stripped (canonical form). The exchange
// hash encodes K the same way (hash_mpint), so the KDF must too: if K has any
// high-order zero bytes (~1/256 of handshakes) a spec-compliant peer strips them
// and would otherwise derive different keys.
static void hash_mpint_K(SshSha256Ctx *ctx, const uint8_t K_be[256])
{
    size_t off = 0;
    while (off < 256 && K_be[off] == 0x00u)
        off++;
    if (off == 256) // K == 0: empty mpint (not reachable for a real DH secret)
    {
        uint8_t len_be[4] = {0, 0, 0, 0};
        ssh_sha256_update(ctx, len_be, 4);
        return;
    }
    bool pad = (K_be[off] & 0x80u) != 0;
    uint32_t mlen = (uint32_t)(256 - off) + (pad ? 1u : 0u);
    uint8_t len_be[4] = {(uint8_t)(mlen >> 24), (uint8_t)(mlen >> 16), (uint8_t)(mlen >> 8), (uint8_t)mlen};
    ssh_sha256_update(ctx, len_be, 4);
    if (pad)
    {
        uint8_t zero = 0x00u;
        ssh_sha256_update(ctx, &zero, 1);
    }
    ssh_sha256_update(ctx, K_be + off, 256 - off);
}

// RFC 4253 §7.2 key derivation extended to any length:
//   K1 = HASH(mpint(K) || H || X || session_id)   (X = label byte)
//   K2 = HASH(mpint(K) || H || K1)
//   K3 = HASH(mpint(K) || H || K1 || K2)  ...   key = K1 || K2 || K3 || ...
// @p out_len up to SSH_KDF_MAX. For the first KEX session_id == H; on a re-key it
// is the H from the first KEX.
void ssh_kdf_derive(const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN],
                    const uint8_t session_id[SSH_SHA256_DIGEST_LEN], char label, uint8_t *out, size_t out_len)
{
    if (out_len > SSH_KDF_MAX)
        out_len = SSH_KDF_MAX; // bounded: every negotiated algorithm needs <= 32 B today
    uint8_t acc[SSH_KDF_MAX];  // K1 || K2 || ... accumulated for the chain hash
    size_t have = 0;

    // K1 = HASH(mpint(K) || H || label || session_id)
    SshSha256Ctx ctx;
    ssh_sha256_init(&ctx);
    hash_mpint_K(&ctx, K_be);
    ssh_sha256_update(&ctx, H, SSH_SHA256_DIGEST_LEN);
    uint8_t lbl = (uint8_t)label;
    ssh_sha256_update(&ctx, &lbl, 1);
    ssh_sha256_update(&ctx, session_id, SSH_SHA256_DIGEST_LEN);
    ssh_sha256_final(&ctx, acc); // acc[0..31] = K1
    have = SSH_SHA256_DIGEST_LEN;

    // Ki+1 = HASH(mpint(K) || H || K1..Ki) until enough material.
    while (have < out_len && have + SSH_SHA256_DIGEST_LEN <= SSH_KDF_MAX)
    {
        ssh_sha256_init(&ctx);
        hash_mpint_K(&ctx, K_be);
        ssh_sha256_update(&ctx, H, SSH_SHA256_DIGEST_LEN);
        ssh_sha256_update(&ctx, acc, have); // all prior blocks
        ssh_sha256_final(&ctx, acc + have);
        have += SSH_SHA256_DIGEST_LEN;
    }
    memcpy(out, acc, out_len);
}

// One 32-byte derived value (the only size any negotiated algorithm needs today).
static void derive_key(const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN],
                       const uint8_t session_id[SSH_SHA256_DIGEST_LEN], char label, uint8_t out[SSH_SHA256_DIGEST_LEN])
{
    ssh_kdf_derive(K_be, H, session_id, label, out, SSH_SHA256_DIGEST_LEN);
}

void ssh_dh_derive_keys_sid(uint8_t i, const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN],
                            const uint8_t session_id[SSH_SHA256_DIGEST_LEN])
{
    if (i >= MAX_SSH_CONNS)
        return;
    SshKeyMat *km = &ssh_keys[i];

    // RFC 4253 §7.2 derives six values, each keyed by a label byte 'A'..'F'.
    // The AES contexts need both key and IV at init time, so derive all six
    // values first, then initialize the contexts once.
    uint8_t iv_c2s[SSH_SHA256_DIGEST_LEN], iv_s2c[SSH_SHA256_DIGEST_LEN];
    uint8_t key_c2s[32], key_s2c[32];

    derive_key(K_be, H, session_id, 'A', iv_c2s);          // IV  C→S (first 16 bytes used)
    derive_key(K_be, H, session_id, 'B', iv_s2c);          // IV  S→C
    derive_key(K_be, H, session_id, 'C', key_c2s);         // cipher key C→S
    derive_key(K_be, H, session_id, 'D', key_s2c);         // cipher key S→C
    derive_key(K_be, H, session_id, 'E', km->mac_key_c2s); // MAC key C→S
    derive_key(K_be, H, session_id, 'F', km->mac_key_s2c); // MAC key S→C

    ssh_aes256ctr_init(&km->c2s_ctx, key_c2s, iv_c2s);
    ssh_aes256ctr_init(&km->s2c_ctx, key_s2c, iv_s2c);

    // Wipe stack temporaries (key material).
    ssh_wipe(key_c2s, sizeof(key_c2s));
    ssh_wipe(key_s2c, sizeof(key_s2c));
    ssh_wipe(iv_c2s, sizeof(iv_c2s));
    ssh_wipe(iv_s2c, sizeof(iv_s2c));

    km->active = true;
}

void ssh_dh_derive_keys(uint8_t i, const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN])
{
    // First-KEX convenience: the session id equals H.
    ssh_dh_derive_keys_sid(i, K_be, H, H);
}
