// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dtls_record.cpp
 * @brief DTLS 1.3 record layer (RFC 9147 §4). See dtls_record.h.
 */

#include "network_drivers/presentation/dtls/dtls_record.h"

#if DETWS_ENABLE_DTLS

#include "network_drivers/presentation/http3/quic_aead.h"
#include "network_drivers/presentation/http3/quic_hkdf.h"
#include <string.h>

namespace
{
// Unified-header first-byte fixed pattern and flag bits (RFC 9147 §4, Figure 3): 0 0 1 C S L E E.
const uint8_t DTLS_UH_FIXED = 0x20; // 001x xxxx
const uint8_t DTLS_UH_FIXED_MASK = 0xE0;
const uint8_t DTLS_UH_CID = 0x10;    // C: connection id present
const uint8_t DTLS_UH_SEQ16 = 0x08;  // S: 16-bit (vs 8-bit) sequence number
const uint8_t DTLS_UH_LENGTH = 0x04; // L: length present
const uint8_t DTLS_UH_EPOCH_MASK = 0x03;

// Build the AEAD nonce: the 64-bit sequence number, right-aligned in the 12-byte IV, XOR the IV
// (RFC 9147 §4.2.2 / RFC 8446 §5.3; the epoch is NOT mixed in). Same construction as QUIC.
void build_nonce(const uint8_t iv[12], uint64_t seq, uint8_t nonce[12])
{
    memcpy(nonce, iv, 12);
    for (int i = 0; i < 8; i++)
        nonce[11 - i] ^= (uint8_t)(seq >> (8 * i));
}

// Reconstruct the full sequence number from its truncated on-wire bits (RFC 9147 §4.2.2, using the
// RFC 9000 Appendix A.3 packet-number decoding: the candidate closest to `expected`).
uint64_t seq_decode(uint64_t expected, uint64_t truncated, unsigned bits)
{
    if (bits == 0 || bits >= 64)
        return truncated;
    uint64_t win = (uint64_t)1 << bits;
    uint64_t hwin = win >> 1;
    uint64_t mask = win - 1;
    uint64_t candidate = (expected & ~mask) | (truncated & mask);
    if (candidate + hwin <= expected && candidate + win > candidate)
        return candidate + win;
    if (candidate > expected + hwin && candidate >= win)
        return candidate - win;
    return candidate;
}
} // namespace

void dtls_record_keys_derive(DtlsRecordKeys *out, DtlsCipher cipher, uint16_t epoch, const uint8_t secret[32])
{
    out->cipher = cipher;
    out->epoch = epoch;
    // AEAD_AES_128_GCM: 16-byte key, 12-byte IV, 16-byte sequence-number key.
    quic_hkdf_expand_label(secret, "key", out->key, sizeof(out->key));
    quic_hkdf_expand_label(secret, "iv", out->iv, sizeof(out->iv));
    quic_hkdf_expand_label(secret, "sn", out->sn_key, sizeof(out->sn_key));
}

// ---------------------------------------------------------------------------
// DTLSPlaintext
// ---------------------------------------------------------------------------

size_t dtls_plaintext_build(uint8_t content_type, uint16_t epoch, uint64_t seq, const uint8_t *fragment,
                            size_t frag_len, uint8_t *out, size_t out_cap)
{
    size_t total = DTLS_PLAINTEXT_HDR_LEN + frag_len;
    if (total > out_cap || frag_len > 0xFFFF)
        return 0;
    out[0] = content_type;
    out[1] = (uint8_t)(DTLS_LEGACY_VERSION >> 8);
    out[2] = (uint8_t)DTLS_LEGACY_VERSION;
    out[3] = (uint8_t)(epoch >> 8);
    out[4] = (uint8_t)epoch;
    out[5] = (uint8_t)(seq >> 40); // 48-bit sequence number, big-endian
    out[6] = (uint8_t)(seq >> 32);
    out[7] = (uint8_t)(seq >> 24);
    out[8] = (uint8_t)(seq >> 16);
    out[9] = (uint8_t)(seq >> 8);
    out[10] = (uint8_t)seq;
    out[11] = (uint8_t)(frag_len >> 8);
    out[12] = (uint8_t)frag_len;
    if (frag_len)
        memcpy(out + DTLS_PLAINTEXT_HDR_LEN, fragment, frag_len);
    return total;
}

size_t dtls_plaintext_parse(const uint8_t *rec, size_t rec_len, DtlsPlaintext *out)
{
    if (rec_len < DTLS_PLAINTEXT_HDR_LEN)
        return 0;
    if (rec[1] != (uint8_t)(DTLS_LEGACY_VERSION >> 8) || rec[2] != (uint8_t)DTLS_LEGACY_VERSION)
        return 0; // wrong legacy_version
    out->content_type = rec[0];
    out->epoch = (uint16_t)(((uint16_t)rec[3] << 8) | rec[4]);
    out->seq = ((uint64_t)rec[5] << 40) | ((uint64_t)rec[6] << 32) | ((uint64_t)rec[7] << 24) |
               ((uint64_t)rec[8] << 16) | ((uint64_t)rec[9] << 8) | (uint64_t)rec[10];
    size_t length = ((size_t)rec[11] << 8) | rec[12];
    if (DTLS_PLAINTEXT_HDR_LEN + length > rec_len)
        return 0;
    out->fragment = rec + DTLS_PLAINTEXT_HDR_LEN;
    out->frag_len = length;
    return DTLS_PLAINTEXT_HDR_LEN + length;
}

// ---------------------------------------------------------------------------
// DTLSCiphertext
// ---------------------------------------------------------------------------

size_t dtls_ciphertext_protect(const DtlsRecordKeys *keys, uint64_t seq, uint8_t content_type, const uint8_t *plaintext,
                               size_t pt_len, uint8_t *out, size_t out_cap)
{
    if (keys->cipher != DtlsCipher::AES_128_GCM_SHA256)
        return 0;
    // Unified header: C=0, S=1 (16-bit seq), L=1 (length present). hdr = byte0 || seq16 || length16.
    const size_t hdr_len = 1 + 2 + 2;
    size_t inner_len = pt_len + 1;             // DTLSInnerPlaintext = plaintext || content_type
    size_t enc_len = inner_len + DTLS_TAG_LEN; // AEAD ciphertext || tag
    size_t total = hdr_len + enc_len;
    if (total > out_cap)
        return 0;

    out[0] = (uint8_t)(DTLS_UH_FIXED | DTLS_UH_SEQ16 | DTLS_UH_LENGTH | (keys->epoch & DTLS_UH_EPOCH_MASK));
    out[1] = (uint8_t)(seq >> 8); // plaintext sequence number (this header form is the AEAD AAD)
    out[2] = (uint8_t)seq;
    out[3] = (uint8_t)(enc_len >> 8);
    out[4] = (uint8_t)enc_len;

    // Assemble the inner plaintext where it will be sealed (seal permits out == pt).
    memcpy(out + hdr_len, plaintext, pt_len);
    out[hdr_len + pt_len] = content_type;

    uint8_t nonce[12];
    build_nonce(keys->iv, seq, nonce);
    // AAD = the unified header carrying the plaintext sequence number (before §4.2.3 encryption).
    quic_aes128_gcm_seal(keys->key, nonce, out, hdr_len, out + hdr_len, inner_len, out + hdr_len);

    // Encrypt the sequence number (RFC 9147 §4.2.3): mask = AES-ECB(sn_key, ciphertext[0..15]).
    // enc_len = inner_len + 16 >= 17, so the 16-byte sample is always available.
    QuicAes128 sn;
    quic_aes128_init(&sn, keys->sn_key);
    uint8_t mask[16];
    quic_aes128_encrypt_block(&sn, out + hdr_len, mask);
    quic_aes128_wipe(&sn);
    out[1] ^= mask[0];
    out[2] ^= mask[1];
    return total;
}

bool dtls_ciphertext_unprotect(const DtlsRecordKeys *keys, uint64_t next_seq, const uint8_t *rec, size_t rec_len,
                               uint8_t *out, size_t out_cap, DtlsCiphertext *info)
{
    if (keys->cipher != DtlsCipher::AES_128_GCM_SHA256 || rec_len < 1)
        return false;
    uint8_t b0 = rec[0];
    if ((b0 & DTLS_UH_FIXED_MASK) != DTLS_UH_FIXED)
        return false; // top 3 bits must be 001
    if (b0 & DTLS_UH_CID)
        return false; // connection-id records are not negotiated in this phase
    if ((b0 & DTLS_UH_EPOCH_MASK) != (keys->epoch & DTLS_UH_EPOCH_MASK))
        return false; // wrong epoch keys for this record

    size_t seq_len = (b0 & DTLS_UH_SEQ16) ? 2 : 1;
    size_t off = 1;
    if (off + seq_len > rec_len)
        return false;
    size_t seq_off = off;
    off += seq_len;

    size_t enc_len;
    if (b0 & DTLS_UH_LENGTH)
    {
        if (off + 2 > rec_len)
            return false;
        enc_len = ((size_t)rec[off] << 8) | rec[off + 1];
        off += 2;
    }
    else
    {
        enc_len = rec_len - off; // to end of datagram
    }
    if (off + enc_len > rec_len || enc_len < 16 || enc_len < DTLS_TAG_LEN + 1)
        return false; // need >= 16 bytes for the SN sample and >= tag + one inner byte

    const uint8_t *enc = rec + off;
    size_t hdr_len = off; // unified header length (C=0, so <= 5)

    // Copy the header so we can write the decrypted sequence number into the AEAD AAD form.
    uint8_t hdr[5];
    memcpy(hdr, rec, hdr_len);

    // Decrypt the sequence number (RFC 9147 §4.2.3).
    QuicAes128 sn;
    quic_aes128_init(&sn, keys->sn_key);
    uint8_t mask[16];
    quic_aes128_encrypt_block(&sn, enc, mask);
    quic_aes128_wipe(&sn);
    uint64_t trunc = 0;
    for (size_t i = 0; i < seq_len; i++)
    {
        hdr[seq_off + i] ^= mask[i];
        trunc = (trunc << 8) | hdr[seq_off + i];
    }
    uint64_t full_seq = seq_decode(next_seq, trunc, (unsigned)(seq_len * 8));

    size_t inner_len = enc_len - DTLS_TAG_LEN;
    if (inner_len > out_cap)
        return false;

    uint8_t nonce[12];
    build_nonce(keys->iv, full_seq, nonce);
    if (!quic_aes128_gcm_open(keys->key, nonce, hdr, hdr_len, enc, enc_len, out))
        return false;

    // Strip zero padding: the last non-zero byte of the inner plaintext is the content type (RFC 8446 §5.2).
    size_t n = inner_len;
    while (n > 0 && out[n - 1] == 0)
        n--;
    if (n == 0)
        return false; // no content type -> invalid record
    info->content_type = out[n - 1];
    info->pt_len = n - 1;
    info->seq = full_seq;
    info->epoch = keys->epoch;
    return true;
}

// ---------------------------------------------------------------------------
// Anti-replay sliding window (RFC 9147 §4.5.1)
// ---------------------------------------------------------------------------

void dtls_replay_init(DtlsReplayWindow *w)
{
    w->highest = 0;
    w->bitmap = 0;
    w->seeded = false;
}

bool dtls_replay_check(const DtlsReplayWindow *w, uint64_t seq)
{
    if (!w->seeded || seq > w->highest)
        return true; // first record, or ahead of the window
    uint64_t diff = w->highest - seq;
    if (diff >= 64)
        return false;                       // older than the window
    return ((w->bitmap >> diff) & 1u) == 0; // set bit => already seen (replay)
}

void dtls_replay_mark(DtlsReplayWindow *w, uint64_t seq)
{
    if (!w->seeded)
    {
        w->seeded = true;
        w->highest = seq;
        w->bitmap = 1; // bit 0 = highest
        return;
    }
    if (seq > w->highest)
    {
        uint64_t shift = seq - w->highest;
        w->bitmap = (shift >= 64) ? 1u : ((w->bitmap << shift) | 1u);
        w->highest = seq;
        return;
    }
    uint64_t diff = w->highest - seq;
    if (diff < 64)
        w->bitmap |= ((uint64_t)1 << diff);
}

#endif // DETWS_ENABLE_DTLS
