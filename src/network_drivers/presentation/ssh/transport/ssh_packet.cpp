// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_packet.cpp
 * @brief SSH binary packet framing, encryption, MAC, and receive reassembly.
 */

#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include "network_drivers/session/scratch.h"
#include "network_drivers/presentation/ssh/crypto/ssh_chachapoly.h"
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha512.h"
#include "network_drivers/presentation/ssh/transport/ssh_keymat.h"
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h> // esp_fill_random()
#else
#include <Arduino.h> // mock
#endif

// ---------------------------------------------------------------------------
// BSS allocation
// ---------------------------------------------------------------------------

SshPacketState ssh_pkt[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static inline uint32_t read_u32_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static inline void write_u32_be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

// Compute the padding needed so that (5 + payload_len + padding) is a
// multiple of 16 (AES block size).  Minimum padding = 4 bytes (RFC 4253 §6).
static size_t compute_padding(size_t payload_len)
{
    size_t total = 5 + payload_len; // 4-byte length + 1-byte padding_len + payload
    size_t remainder = total % 16;
    size_t padding = (remainder == 0) ? 0 : (16 - remainder);
    if (padding < 4)
        padding += 16;
    return padding;
}

// Compute the MAC over 4-byte seq_no || buf using the HMAC named by mac_mode. For E&M the buf is
// the plaintext packet; for ETM it is the length field || ciphertext. Writes ssh_mac_len() bytes.
static void compute_mac_mode(uint8_t mac_mode, const uint8_t *mac_key, uint32_t seq_no, const uint8_t *buf,
                             size_t buf_len, uint8_t *mac_out)
{
    uint8_t seq_be[4];
    write_u32_be(seq_be, seq_no);
    if (mac_mode == SSH_MAC_HMAC_SHA512 || mac_mode == SSH_MAC_HMAC_SHA512_ETM)
    {
        SshHmacSha512Ctx ctx;
        ssh_hmac_sha512_init(&ctx, mac_key, 64);
        ssh_hmac_sha512_update(&ctx, seq_be, 4);
        ssh_hmac_sha512_update(&ctx, buf, buf_len);
        ssh_hmac_sha512_final(&ctx, mac_out);
    }
    else
    {
        SshHmacCtx ctx;
        ssh_hmac_sha256_init(&ctx, mac_key, 32);
        ssh_hmac_sha256_update(&ctx, seq_be, 4);
        ssh_hmac_sha256_update(&ctx, buf, buf_len);
        ssh_hmac_sha256_final(&ctx, mac_out);
    }
}

// Constant-time 32-byte comparison to prevent timing oracles on MAC verify.
static int ct_memcmp(const uint8_t *a, const uint8_t *b, size_t n)
{
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++)
        diff |= a[i] ^ b[i];
    return (int)diff;
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------

void ssh_pkt_init(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    SshPacketState *s = &ssh_pkt[i];
    memset(s, 0, sizeof(*s));
    s->kex_active = true;
    s->encrypted = false;
}

// ---------------------------------------------------------------------------
// Send
// ---------------------------------------------------------------------------

int ssh_pkt_send(uint8_t i, const uint8_t *payload, size_t payload_len, uint8_t *out, size_t *out_len, size_t out_cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshPacketState *s = &ssh_pkt[i];
    SshKeyMat *km = &ssh_keys[i];

    // Sequence overflow guard.
    if (s->seq_no_send >= SSH_SEQ_CLOSE_THRESHOLD)
        return -1;

    // Padding block size and base differ by mode. chacha (AEAD) and aes-ETM exclude the 4-byte
    // length from the block-alignment (it is AAD / sent in clear); plain aes-E&M includes it.
    //   chacha    : block 8,  base = padding_length + payload
    //   aes ETM   : block 16, base = padding_length + payload
    //   aes E&M / plaintext : block 16, base = length + padding_length + payload  (compute_padding)
    bool chacha = s->encrypted && km->cipher_mode == SSH_CIPHER_CHACHA20POLY1305;
    bool etm = s->encrypted && km->cipher_mode == SSH_CIPHER_AES256CTR && ssh_mac_is_etm(km->mac_mode);
    size_t pad_len, tag_len;
    if (chacha)
    {
        size_t base = 1 + payload_len;
        pad_len = 8 - (base % 8);
        if (pad_len < 4)
            pad_len += 8;
        tag_len = SSH_CHACHAPOLY_TAG_LEN;
    }
    else if (etm)
    {
        size_t base = 1 + payload_len;
        pad_len = 16 - (base % 16);
        if (pad_len < 4)
            pad_len += 16;
        tag_len = ssh_mac_len(km->mac_mode);
    }
    else
    {
        pad_len = compute_padding(payload_len);
        tag_len = s->encrypted ? ssh_mac_len(km->mac_mode) : 0;
    }
    size_t pkt_len = 1 + payload_len + pad_len; // padding_length + payload + padding
    size_t wire_len = 4 + pkt_len + tag_len;

    if (wire_len > out_cap)
        return -1;

    // Assemble the plaintext packet into out[].
    write_u32_be(out, (uint32_t)pkt_len);            // packet_length
    out[4] = (uint8_t)pad_len;                       // padding_length
    memcpy(out + 5, payload, payload_len);           // payload
    esp_fill_random(out + 5 + payload_len, pad_len); // random padding

    if (chacha)
    {
        // Encrypt length (header key) + payload (main key) and append the Poly1305 tag.
        ssh_chachapoly_encrypt(km->chacha_key_s2c, s->seq_no_send, out, out, (uint32_t)pkt_len);
    }
    else if (etm)
    {
        // Encrypt-then-MAC: length stays in clear; encrypt the payload, then MAC over (length||ct).
        ssh_aes256ctr_crypt(&km->s2c_ctx, out + 4, out + 4, pkt_len);
        compute_mac_mode(km->mac_mode, km->mac_key_s2c, s->seq_no_send, out, 4 + pkt_len, out + 4 + pkt_len);
    }
    else if (s->encrypted)
    {
        // Encrypt-and-MAC: MAC over plaintext (seq || unencrypted packet), then AES-256-CTR.
        uint8_t mac[64];
        compute_mac_mode(km->mac_mode, km->mac_key_s2c, s->seq_no_send, out, 4 + pkt_len, mac);
        ssh_aes256ctr_crypt(&km->s2c_ctx, out, out, 4 + pkt_len);
        memcpy(out + 4 + pkt_len, mac, tag_len);
        ssh_wipe(mac, sizeof(mac));
    }

    *out_len = wire_len;
    s->seq_no_send++;
    return 0;
}

// ---------------------------------------------------------------------------
// Receive
// ---------------------------------------------------------------------------

int ssh_pkt_recv(uint8_t i, const uint8_t *data, size_t len, ssh_msg_handler_t handler)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshPacketState *s = &ssh_pkt[i];
    SshKeyMat *km = &ssh_keys[i];

    // Append to receive buffer.
    size_t space = SSH_PKT_BUF_SIZE - s->rx_len;
    if (len > space)
    {
        // Buffer overflow - discard and disconnect.
        ssh_wipe(s->rx_buf, s->rx_len);
        s->rx_len = 0;
        return -1;
    }
    memcpy(s->rx_buf + s->rx_len, data, len);
    s->rx_len += len;

    // Extract complete packets.
    while (s->rx_len >= 4)
    {
        if (s->encrypted && km->cipher_mode == SSH_CIPHER_CHACHA20POLY1305)
        {
            // chacha20-poly1305@openssh.com. Keyed by the sequence number, so decrypting the
            // length is stateless/repeatable - no cipher-state peek/restore is needed.
            if (s->rx_len < 4)
                break; // need the encrypted length field

            uint32_t pkt_len = ssh_chachapoly_get_length(km->chacha_key_c2s, s->seq_no_recv, s->rx_buf);
            if (pkt_len < 1 || pkt_len > SSH_PKT_BUF_SIZE - 4 - SSH_CHACHAPOLY_TAG_LEN)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }
            size_t wire_need = 4 + pkt_len + SSH_CHACHAPOLY_TAG_LEN;
            if (s->rx_len < wire_need)
                break; // incomplete packet

            const size_t scratch_sz = 4 + pkt_len; // plaintext = length(4) || (pad_len||payload||pad)
            ScratchScope scratch_scope;
            uint8_t *scratch = (uint8_t *)scratch_alloc(scratch_sz, 16);
            if (!scratch)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }

            // Verify the Poly1305 tag over the ciphertext, then decrypt. No plaintext on failure.
            if (!ssh_chachapoly_decrypt(km->chacha_key_c2s, s->seq_no_recv, scratch, s->rx_buf, pkt_len))
            {
                ssh_wipe(scratch, scratch_sz);
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1; // caller must close connection
            }

            if (s->seq_no_recv >= SSH_SEQ_CLOSE_THRESHOLD)
            {
                ssh_wipe(scratch, scratch_sz);
                return -1;
            }
            s->seq_no_recv++;

            uint8_t pad_len_byte = scratch[4];
            if (pad_len_byte < 4 || pad_len_byte >= pkt_len)
            {
                ssh_wipe(scratch, scratch_sz);
                return -1;
            }
            size_t payload_len = pkt_len - 1 - pad_len_byte;
            uint8_t msg_type = scratch[5];
            handler(i, msg_type, scratch + 5, payload_len);

            size_t consumed = wire_need;
            memmove(s->rx_buf, s->rx_buf + consumed, s->rx_len - consumed);
            s->rx_len -= consumed;
            ssh_wipe(scratch, scratch_sz);
        }
        else if (s->encrypted && ssh_mac_is_etm(km->mac_mode))
        {
            // aes256-ctr + encrypt-then-MAC: the 4-byte packet_length is sent in the clear, and the
            // MAC is verified over (length || ciphertext) BEFORE anything is decrypted.
            if (s->rx_len < 4)
                break;
            uint32_t pkt_len = read_u32_be(s->rx_buf);
            size_t mac_tag = ssh_mac_len(km->mac_mode);
            // The encrypted portion (pkt_len) must be a positive whole number of AES blocks.
            if (pkt_len < 1 || pkt_len > SSH_PKT_BUF_SIZE - 4 || (pkt_len % 16) != 0)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }
            size_t wire_need = 4 + pkt_len + mac_tag;
            if (s->rx_len < wire_need)
                break; // incomplete packet

            uint8_t expected_mac[64];
            compute_mac_mode(km->mac_mode, km->mac_key_c2s, s->seq_no_recv, s->rx_buf, 4 + pkt_len, expected_mac);
            if (ct_memcmp(expected_mac, s->rx_buf + 4 + pkt_len, mac_tag) != 0)
            {
                ssh_wipe(expected_mac, sizeof(expected_mac));
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1; // caller must close connection
            }
            ssh_wipe(expected_mac, sizeof(expected_mac));

            if (s->seq_no_recv >= SSH_SEQ_CLOSE_THRESHOLD)
                return -1;
            s->seq_no_recv++;

            // MAC verified -> decrypt the payload (advances c2s_ctx by exactly pkt_len/16 blocks).
            const size_t scratch_sz = SSH_PKT_BUF_SIZE;
            ScratchScope scratch_scope;
            uint8_t *scratch = (uint8_t *)scratch_alloc(scratch_sz, 16);
            if (!scratch)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }
            memcpy(scratch, s->rx_buf + 4, pkt_len);
            ssh_aes256ctr_crypt(&km->c2s_ctx, scratch, scratch, pkt_len);

            // scratch = padding_length || payload || padding.
            uint8_t pad_len_byte = scratch[0];
            if (pad_len_byte < 4 || pad_len_byte >= pkt_len)
            {
                ssh_wipe(scratch, scratch_sz);
                return -1;
            }
            size_t payload_len = pkt_len - 1 - pad_len_byte;
            uint8_t msg_type = scratch[1];
            handler(i, msg_type, scratch + 1, payload_len);

            size_t consumed = wire_need;
            memmove(s->rx_buf, s->rx_buf + consumed, s->rx_len - consumed);
            s->rx_len -= consumed;
            ssh_wipe(scratch, scratch_sz);
        }
        else if (s->encrypted)
        {
            // aes256-ctr + encrypt-and-MAC. We need the first cipher block (16 bytes) for the length.
            if (s->rx_len < 16)
                break; // wait for more data

            // --- Peek packet_length WITHOUT permanently advancing the cipher ---
            // AES-CTR is stateful: decrypting bytes advances the counter.  To
            // read the length without committing, snapshot the CTR streaming
            // state (counter/keystream/pos - the key schedule is invariant and
            // may hold internal pointers on mbedtls, so we never copy it),
            // decrypt the first block, then restore.  Nothing is consumed yet.
            uint8_t saved_counter[16];
            uint8_t saved_keystream[16];
            uint8_t saved_pos;
            memcpy(saved_counter, km->c2s_ctx.counter, 16);
            memcpy(saved_keystream, km->c2s_ctx.keystream, 16);
            saved_pos = km->c2s_ctx.pos;

            uint8_t len_block[16];
            memcpy(len_block, s->rx_buf, 16);
            ssh_aes256ctr_crypt(&km->c2s_ctx, len_block, len_block, 16);
            uint32_t pkt_len = read_u32_be(len_block);
            ssh_wipe(len_block, sizeof(len_block));

            // Restore the cipher to the packet boundary (un-peek).
            memcpy(km->c2s_ctx.counter, saved_counter, 16);
            memcpy(km->c2s_ctx.keystream, saved_keystream, 16);
            km->c2s_ctx.pos = saved_pos;
            ssh_wipe(saved_keystream, sizeof(saved_keystream));

            // Validate length.  The encrypted portion (4 + pkt_len) must be a
            // whole number of AES blocks (RFC 4253 §6 padding guarantees this).
            size_t enc_len = 4 + pkt_len;
            if (pkt_len < 1 || pkt_len > SSH_PKT_BUF_SIZE - 4 || (enc_len % 16) != 0)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }

            size_t mac_tag = ssh_mac_len(km->mac_mode);
            size_t wire_need = enc_len + mac_tag;
            if (s->rx_len < wire_need)
                break; // incomplete packet; cipher state already restored

            // Borrow this packet's plaintext scratch from the shared arena. The
            // scope guard reclaims it on every exit path, so multiple packets in
            // one call reuse the same space instead of accumulating; an exhausted
            // arena fails closed (discard + disconnect).
            const size_t scratch_sz = SSH_PKT_BUF_SIZE + 64;
            ScratchScope scratch_scope;
            uint8_t *scratch = (uint8_t *)scratch_alloc(scratch_sz, 16);
            if (!scratch)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }

            // Full packet present.  Decrypt EXACTLY the encrypted portion,
            // which advances c2s_ctx by exactly enc_len/16 blocks and leaves
            // the cipher aligned on the next packet boundary.
            memcpy(scratch, s->rx_buf, enc_len);
            ssh_aes256ctr_crypt(&km->c2s_ctx, scratch, scratch, enc_len);

            // Verify MAC over seq_no || plaintext(scratch[0..enc_len)).
            const uint8_t *rx_mac = s->rx_buf + enc_len; // MAC is sent in clear
            uint8_t expected_mac[64];
            compute_mac_mode(km->mac_mode, km->mac_key_c2s, s->seq_no_recv, scratch, enc_len, expected_mac);

            if (ct_memcmp(expected_mac, rx_mac, mac_tag) != 0)
            {
                // MAC failure: zero everything and disconnect.
                ssh_wipe(scratch, scratch_sz);
                ssh_wipe(expected_mac, sizeof(expected_mac));
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1; // caller must close connection
            }
            ssh_wipe(expected_mac, sizeof(expected_mac));

            // MAC verified.  Sequence overflow guard.
            if (s->seq_no_recv >= SSH_SEQ_CLOSE_THRESHOLD)
            {
                ssh_wipe(scratch, scratch_sz);
                return -1;
            }
            s->seq_no_recv++;

            // Extract payload: scratch[5 .. 5 + payload_len - 1]
            uint8_t pad_len_byte = scratch[4];
            // RFC 4253 6: there MUST be at least 4 bytes of padding, and it cannot
            // exceed the packet (which would underflow payload_len).
            if (pad_len_byte < 4 || pad_len_byte >= pkt_len)
            {
                ssh_wipe(scratch, scratch_sz);
                return -1;
            }
            size_t payload_len = pkt_len - 1 - pad_len_byte;
            uint8_t msg_type = scratch[5];
            handler(i, msg_type, scratch + 5, payload_len);

            // Consume from rx_buf.
            size_t consumed = wire_need;
            memmove(s->rx_buf, s->rx_buf + consumed, s->rx_len - consumed);
            s->rx_len -= consumed;
            ssh_wipe(scratch, scratch_sz);
        }
        else
        {
            // Unencrypted path (during initial handshake / before NEWKEYS).
            uint32_t pkt_len = read_u32_be(s->rx_buf);
            if (pkt_len < 1 || pkt_len > SSH_PKT_BUF_SIZE - 4)
            {
                ssh_wipe(s->rx_buf, s->rx_len);
                s->rx_len = 0;
                return -1;
            }
            size_t wire_need = 4 + pkt_len; // no MAC before NEWKEYS
            if (s->rx_len < wire_need)
                break;

            if (s->seq_no_recv >= SSH_SEQ_CLOSE_THRESHOLD)
                return -1;
            s->seq_no_recv++;

            uint8_t pad_len_byte = s->rx_buf[4];
            if (pad_len_byte >= pkt_len)
                return -1;
            size_t payload_len = pkt_len - 1 - pad_len_byte;
            uint8_t msg_type = s->rx_buf[5];
            handler(i, msg_type, s->rx_buf + 5, payload_len);

            size_t consumed = wire_need;
            memmove(s->rx_buf, s->rx_buf + consumed, s->rx_len - consumed);
            s->rx_len -= consumed;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Disconnect
// ---------------------------------------------------------------------------

int ssh_pkt_disconnect(uint8_t i, uint32_t reason_code, uint8_t *out, size_t *out_len, size_t out_cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;

    // Build SSH_MSG_DISCONNECT payload (RFC 4253 §11.1):
    //   byte    SSH_MSG_DISCONNECT
    //   uint32  reason code
    //   string  description (empty)
    //   string  language tag (empty)
    uint8_t payload[13];
    payload[0] = SSH_MSG_DISCONNECT;
    payload[1] = (uint8_t)(reason_code >> 24);
    payload[2] = (uint8_t)(reason_code >> 16);
    payload[3] = (uint8_t)(reason_code >> 8);
    payload[4] = (uint8_t)(reason_code);
    payload[5] = 0;
    payload[6] = 0;
    payload[7] = 0;
    payload[8] = 0; // empty description
    payload[9] = 0;
    payload[10] = 0;
    payload[11] = 0;
    payload[12] = 0; // empty language

    int rc = ssh_pkt_send(i, payload, sizeof(payload), out, out_len, out_cap);

    // Zero packet state and key material regardless of send success.
    ssh_pkt_init(i);
    ssh_keymat_wipe(i);
    ssh_dh_wipe(i);

    return rc;
}
