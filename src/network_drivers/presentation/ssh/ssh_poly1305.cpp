// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_poly1305.cpp
 * @brief Poly1305 (RFC 8439) - implementation. See ssh_poly1305.h.
 *
 * poly1305-donna 32-bit: the accumulator h and key part r are held as five 26-bit limbs; each
 * 16-byte block adds the message limb (with the 2^128 high bit), multiplies by r, and reduces
 * modulo 2^130 - 5. The final value is fully reduced, conditionally has p subtracted in constant
 * time, and s is added modulo 2^128.
 */

#include "ssh_poly1305.h"

namespace
{
uint32_t rd_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
void wr_le32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

// Absorb one 16-byte block into h: h = (h + block) * r mod (2^130 - 5). hibit is 2^24 for a full
// block (the implicit high 1 bit at position 128) or 0 for the padded final block.
void poly_block(uint32_t h[5], const uint32_t r[5], const uint32_t sr[5], const uint8_t blk[16], uint32_t hibit)
{
    uint32_t t0 = rd_le32(blk + 0), t1 = rd_le32(blk + 4), t2 = rd_le32(blk + 8), t3 = rd_le32(blk + 12);
    h[0] += t0 & 0x3ffffff;
    h[1] += ((t0 >> 26) | (t1 << 6)) & 0x3ffffff;
    h[2] += ((t1 >> 20) | (t2 << 12)) & 0x3ffffff;
    h[3] += ((t2 >> 14) | (t3 << 18)) & 0x3ffffff;
    h[4] += (t3 >> 8) | hibit;

    uint64_t d0 = (uint64_t)h[0] * r[0] + (uint64_t)h[1] * sr[4] + (uint64_t)h[2] * sr[3] + (uint64_t)h[3] * sr[2] +
                  (uint64_t)h[4] * sr[1];
    uint64_t d1 = (uint64_t)h[0] * r[1] + (uint64_t)h[1] * r[0] + (uint64_t)h[2] * sr[4] + (uint64_t)h[3] * sr[3] +
                  (uint64_t)h[4] * sr[2];
    uint64_t d2 = (uint64_t)h[0] * r[2] + (uint64_t)h[1] * r[1] + (uint64_t)h[2] * r[0] + (uint64_t)h[3] * sr[4] +
                  (uint64_t)h[4] * sr[3];
    uint64_t d3 = (uint64_t)h[0] * r[3] + (uint64_t)h[1] * r[2] + (uint64_t)h[2] * r[1] + (uint64_t)h[3] * r[0] +
                  (uint64_t)h[4] * sr[4];
    uint64_t d4 = (uint64_t)h[0] * r[4] + (uint64_t)h[1] * r[3] + (uint64_t)h[2] * r[2] + (uint64_t)h[3] * r[1] +
                  (uint64_t)h[4] * r[0];

    uint32_t c;
    c = (uint32_t)(d0 >> 26);
    h[0] = (uint32_t)d0 & 0x3ffffff;
    d1 += c;
    c = (uint32_t)(d1 >> 26);
    h[1] = (uint32_t)d1 & 0x3ffffff;
    d2 += c;
    c = (uint32_t)(d2 >> 26);
    h[2] = (uint32_t)d2 & 0x3ffffff;
    d3 += c;
    c = (uint32_t)(d3 >> 26);
    h[3] = (uint32_t)d3 & 0x3ffffff;
    d4 += c;
    c = (uint32_t)(d4 >> 26);
    h[4] = (uint32_t)d4 & 0x3ffffff;
    h[0] += c * 5;
    c = h[0] >> 26;
    h[0] &= 0x3ffffff;
    h[1] += c;
}
} // namespace

void ssh_poly1305(uint8_t tag[SSH_POLY1305_TAG_LEN], const uint8_t *msg, size_t len,
                  const uint8_t key[SSH_POLY1305_KEY_LEN])
{
    uint32_t t0 = rd_le32(key + 0), t1 = rd_le32(key + 4), t2 = rd_le32(key + 8), t3 = rd_le32(key + 12);
    // Clamp r (RFC 8439 sec 2.5) folded into the limb split.
    uint32_t r[5];
    r[0] = t0 & 0x3ffffff;
    r[1] = ((t0 >> 26) | (t1 << 6)) & 0x3ffff03;
    r[2] = ((t1 >> 20) | (t2 << 12)) & 0x3ffc0ff;
    r[3] = ((t2 >> 14) | (t3 << 18)) & 0x3f03fff;
    r[4] = (t3 >> 8) & 0x00fffff;
    uint32_t sr[5] = {0, r[1] * 5, r[2] * 5, r[3] * 5, r[4] * 5};
    uint32_t h[5] = {0, 0, 0, 0, 0};

    while (len >= 16)
    {
        poly_block(h, r, sr, msg, 1u << 24);
        msg += 16;
        len -= 16;
    }
    if (len)
    {
        uint8_t buf[16] = {0};
        for (size_t i = 0; i < len; i++)
            buf[i] = msg[i];
        buf[len] = 1; // the message-terminating high bit for the partial block
        poly_block(h, r, sr, buf, 0);
    }

    // Fully carry h.
    uint32_t c;
    c = h[1] >> 26;
    h[1] &= 0x3ffffff;
    h[2] += c;
    c = h[2] >> 26;
    h[2] &= 0x3ffffff;
    h[3] += c;
    c = h[3] >> 26;
    h[3] &= 0x3ffffff;
    h[4] += c;
    c = h[4] >> 26;
    h[4] &= 0x3ffffff;
    h[0] += c * 5;
    c = h[0] >> 26;
    h[0] &= 0x3ffffff;
    h[1] += c;

    // Compute h + -p (i.e. h - (2^130 - 5)).
    uint32_t g0 = h[0] + 5;
    c = g0 >> 26;
    g0 &= 0x3ffffff;
    uint32_t g1 = h[1] + c;
    c = g1 >> 26;
    g1 &= 0x3ffffff;
    uint32_t g2 = h[2] + c;
    c = g2 >> 26;
    g2 &= 0x3ffffff;
    uint32_t g3 = h[3] + c;
    c = g3 >> 26;
    g3 &= 0x3ffffff;
    uint32_t g4 = h[4] + c - (1u << 26);

    // Select h if h < p, else h + -p; branch-free.
    uint32_t mask = (g4 >> 31) - 1; // all-ones when g4 has no borrow (h >= p) -> pick g
    g0 &= mask;
    g1 &= mask;
    g2 &= mask;
    g3 &= mask;
    g4 &= mask;
    mask = ~mask;
    h[0] = (h[0] & mask) | g0;
    h[1] = (h[1] & mask) | g1;
    h[2] = (h[2] & mask) | g2;
    h[3] = (h[3] & mask) | g3;
    h[4] = (h[4] & mask) | g4;

    // Reassemble h into four 32-bit words (h mod 2^128).
    uint32_t f0 = (h[0]) | (h[1] << 26);
    uint32_t f1 = (h[1] >> 6) | (h[2] << 20);
    uint32_t f2 = (h[2] >> 12) | (h[3] << 14);
    uint32_t f3 = (h[3] >> 18) | (h[4] << 8);

    // tag = (h + s) mod 2^128, where s = key[16..32].
    uint64_t f = (uint64_t)f0 + rd_le32(key + 16);
    f0 = (uint32_t)f;
    f = (uint64_t)f1 + rd_le32(key + 20) + (f >> 32);
    f1 = (uint32_t)f;
    f = (uint64_t)f2 + rd_le32(key + 24) + (f >> 32);
    f2 = (uint32_t)f;
    f = (uint64_t)f3 + rd_le32(key + 28) + (f >> 32);
    f3 = (uint32_t)f;

    wr_le32(tag + 0, f0);
    wr_le32(tag + 4, f1);
    wr_le32(tag + 8, f2);
    wr_le32(tag + 12, f3);
}
