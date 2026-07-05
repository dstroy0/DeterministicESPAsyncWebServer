// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_chachapoly.cpp
 * @brief chacha20-poly1305@openssh.com - implementation. See ssh_chachapoly.h.
 */

#include "network_drivers/presentation/ssh/crypto/ssh_chachapoly.h"
#include "network_drivers/presentation/ssh/crypto/ssh_chacha20.h"
#include "network_drivers/presentation/ssh/crypto/ssh_poly1305.h"
#include <string.h>

namespace
{
// The 8-byte ChaCha nonce is the sequence number as a big-endian uint64 (POKE_U64 in OpenSSH); a
// 32-bit SSH seqnr leaves the high 4 bytes zero.
void seq_nonce(uint32_t seqnr, uint8_t iv[8])
{
    iv[0] = iv[1] = iv[2] = iv[3] = 0;
    iv[4] = (uint8_t)(seqnr >> 24);
    iv[5] = (uint8_t)(seqnr >> 16);
    iv[6] = (uint8_t)(seqnr >> 8);
    iv[7] = (uint8_t)seqnr;
}

// Constant-time equality of two 16-byte tags.
bool ct_eq16(const uint8_t *a, const uint8_t *b)
{
    uint8_t d = 0;
    for (int i = 0; i < 16; i++)
        d |= (uint8_t)(a[i] ^ b[i]);
    return d == 0;
}
} // namespace

uint32_t ssh_chachapoly_get_length(const uint8_t key[SSH_CHACHAPOLY_KEY_LEN], uint32_t seqnr,
                                   const uint8_t enc_len[SSH_CHACHAPOLY_AAD_LEN])
{
    uint8_t iv[8];
    seq_nonce(seqnr, iv);
    uint8_t len[4];
    ssh_chacha20_xor(key + 32, iv, 0, enc_len, len, 4); // header key, counter 0
    return ((uint32_t)len[0] << 24) | ((uint32_t)len[1] << 16) | ((uint32_t)len[2] << 8) | len[3];
}

void ssh_chachapoly_encrypt(const uint8_t key[SSH_CHACHAPOLY_KEY_LEN], uint32_t seqnr, uint8_t *dest,
                            const uint8_t *src, uint32_t payload_len)
{
    uint8_t iv[8];
    seq_nonce(seqnr, iv);
    uint8_t poly_key[32];
    ssh_chacha20_xor(key, iv, 0, nullptr, poly_key, 32);          // Poly1305 key = K_main block 0
    ssh_chacha20_xor(key + 32, iv, 0, src, dest, 4);              // length field: K_header, counter 0
    ssh_chacha20_xor(key, iv, 1, src + 4, dest + 4, payload_len); // payload: K_main, counter 1
    ssh_poly1305(dest + 4 + payload_len, dest, 4 + payload_len, poly_key);
}

bool ssh_chachapoly_decrypt(const uint8_t key[SSH_CHACHAPOLY_KEY_LEN], uint32_t seqnr, uint8_t *dest,
                            const uint8_t *src, uint32_t payload_len)
{
    uint8_t iv[8];
    seq_nonce(seqnr, iv);
    uint8_t poly_key[32];
    ssh_chacha20_xor(key, iv, 0, nullptr, poly_key, 32);
    uint8_t tag[16];
    ssh_poly1305(tag, src, 4 + payload_len, poly_key); // MAC over the ciphertext (length || payload)
    if (!ct_eq16(tag, src + 4 + payload_len))
        return false; // authentication failed - produce no plaintext
    ssh_chacha20_xor(key + 32, iv, 0, src, dest, 4);
    ssh_chacha20_xor(key, iv, 1, src + 4, dest + 4, payload_len);
    return true;
}
