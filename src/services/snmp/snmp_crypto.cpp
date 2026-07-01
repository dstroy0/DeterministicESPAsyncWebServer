// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_crypto.cpp
 * @brief USM key localization (SHA-256) + AES-128-CFB implementation.
 */

#include "services/snmp/snmp_crypto.h"

#if DETWS_ENABLE_SNMP_V3

#include "network_drivers/presentation/ssh/ssh_sha256.h"
#include <string.h>

// Zero key material with a volatile loop the compiler cannot optimize away. A
// plain memset() whose result is never observed (the buffer dies at return) may
// be elided as a dead store, leaving secrets on the stack. Same idiom as ssh_wipe.
static inline void snmp_wipe(void *p, size_t n)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (n--)
        *v++ = 0;
}

// ---------------------------------------------------------------------------
// RFC 3414 §2.6 key localization (SHA-256)
// ---------------------------------------------------------------------------

void snmp_usm_localize_key(const char *password, const uint8_t *engine_id, size_t engine_id_len,
                           uint8_t key_out[SNMP_USM_KEY_LEN])
{
    size_t pwlen = password ? strlen(password) : 0;
    if (pwlen == 0)
    {
        memset(key_out, 0, SNMP_USM_KEY_LEN);
        return;
    }

    // Ku = SHA-256( password repeated to exactly 1 048 576 bytes ).
    SshSha256Ctx ctx;
    ssh_sha256_init(&ctx);
    uint8_t block[64];
    size_t pw_index = 0;
    uint32_t count = 0;
    while (count < 1048576u)
    {
        for (int i = 0; i < 64; i++)
        {
            block[i] = (uint8_t)password[pw_index];
            pw_index = (pw_index + 1) % pwlen;
        }
        ssh_sha256_update(&ctx, block, 64);
        count += 64;
    }
    uint8_t ku[SNMP_USM_KEY_LEN];
    ssh_sha256_final(&ctx, ku);

    // Kul = SHA-256( Ku || engineID || Ku ).
    ssh_sha256_init(&ctx);
    ssh_sha256_update(&ctx, ku, SNMP_USM_KEY_LEN);
    ssh_sha256_update(&ctx, engine_id, engine_id_len);
    ssh_sha256_update(&ctx, ku, SNMP_USM_KEY_LEN);
    ssh_sha256_final(&ctx, key_out);

    snmp_wipe(ku, sizeof(ku));
    snmp_wipe(block, sizeof(block));
}

// ---------------------------------------------------------------------------
// AES-128 (FIPS-197) block encrypt + CFB-128 mode
// ---------------------------------------------------------------------------

static const uint8_t kSBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9,
    0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f,
    0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07,
    0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3,
    0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58,
    0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3,
    0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f,
    0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac,
    0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a,
    0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70,
    0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11,
    0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42,
    0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

static const uint8_t kRcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

// Expand a 128-bit key into 11 round keys (44 words).
static void aes128_key_schedule(const uint8_t key[16], uint8_t rk[176])
{
    memcpy(rk, key, 16);
    for (int i = 4; i < 44; i++)
    {
        uint8_t t[4];
        memcpy(t, rk + (i - 1) * 4, 4);
        if (i % 4 == 0)
        {
            uint8_t tmp = t[0]; // RotWord
            t[0] = kSBox[t[1]];
            t[1] = kSBox[t[2]];
            t[2] = kSBox[t[3]];
            t[3] = kSBox[tmp];
            t[0] ^= kRcon[i / 4 - 1];
        }
        for (int j = 0; j < 4; j++)
            rk[i * 4 + j] = rk[(i - 4) * 4 + j] ^ t[j];
    }
}

static uint8_t xtime(uint8_t x)
{
    return (uint8_t)((x << 1) ^ ((x >> 7) * 0x1b));
}

static void aes128_encrypt_block(const uint8_t rk[176], const uint8_t in[16], uint8_t out[16])
{
    uint8_t s[16];
    for (int i = 0; i < 16; i++)
        s[i] = in[i] ^ rk[i];

    for (int round = 1; round <= 10; round++)
    {
        // SubBytes
        for (int i = 0; i < 16; i++)
            s[i] = kSBox[s[i]];

        // ShiftRows (state is column-major: s[r + 4c])
        uint8_t t[16];
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                t[r + 4 * c] = s[r + 4 * ((c + r) % 4)];
        memcpy(s, t, 16);

        // MixColumns (skip in the final round)
        if (round != 10)
        {
            for (int c = 0; c < 4; c++)
            {
                uint8_t *col = s + 4 * c;
                uint8_t a0 = col[0], a1 = col[1], a2 = col[2], a3 = col[3];
                col[0] = (uint8_t)(xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3);
                col[1] = (uint8_t)(a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3);
                col[2] = (uint8_t)(a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3));
                col[3] = (uint8_t)((xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3));
            }
        }

        // AddRoundKey
        for (int i = 0; i < 16; i++)
            s[i] ^= rk[round * 16 + i];
    }
    memcpy(out, s, 16);
}

void snmp_aes128_cfb(const uint8_t key[16], const uint8_t iv[16], const uint8_t *in, uint8_t *out, size_t len,
                     bool encrypt)
{
    uint8_t rk[176];
    aes128_key_schedule(key, rk);
    uint8_t fb[16];
    memcpy(fb, iv, 16);
    uint8_t ks[16];

    size_t off = 0;
    while (off < len)
    {
        aes128_encrypt_block(rk, fb, ks);
        size_t bl = len - off;
        if (bl > 16)
            bl = 16;
        // Ciphertext feedback must be captured before an in-place XOR overwrites
        // the input (matters when out == in).
        uint8_t cipher[16];
        if (encrypt)
        {
            for (size_t j = 0; j < bl; j++)
            {
                out[off + j] = in[off + j] ^ ks[j];
                cipher[j] = out[off + j];
            }
        }
        else
        {
            for (size_t j = 0; j < bl; j++)
            {
                cipher[j] = in[off + j];
                out[off + j] = in[off + j] ^ ks[j];
            }
        }
        if (bl == 16)
            memcpy(fb, cipher, 16);
        off += bl;
    }

    snmp_wipe(rk, sizeof(rk));
    snmp_wipe(ks, sizeof(ks));
    snmp_wipe(fb, sizeof(fb));
}

#endif // DETWS_ENABLE_SNMP_V3
