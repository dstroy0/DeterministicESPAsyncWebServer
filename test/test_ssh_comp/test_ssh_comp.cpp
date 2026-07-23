// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Integration test for SSH server-to-client compression WIRING (network_drivers/presentation/ssh):
// the compression owner (ssh_comp) + its activation + the packet-layer compress path in
// ssh_pkt_send. It drives the packet layer directly (before NEWKEYS, so no cipher) with compression
// activated, then reconstructs the continuous zlib stream from the framed packets and decodes it via
// inflate_raw (validated vs Python zlib in test_inflate) - proving payloads are compressed on the
// wire and the whole session forms one valid, context-takeover zlib stream.

#include "network_drivers/presentation/inflate/inflate.h"
#include "network_drivers/presentation/ssh/auth/ssh_auth.h"         // password verifier for the dispatch path
#include "network_drivers/presentation/ssh/connection/ssh_server.h" // dispatcher compression triggers
#include "network_drivers/presentation/ssh/crypto/ssh_aes256ctr.h"  // native software AES-256-CTR path
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"     // independent KDF verification
#include "network_drivers/presentation/ssh/transport/ssh_comp.h"
#include "network_drivers/presentation/ssh/transport/ssh_dh.h" // DH keygen + RFC 4253 §7.2 key derivation
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include "network_drivers/presentation/ssh/transport/ssh_transport.h" // KEXINIT s2c compression negotiation
#include "network_drivers/session/scratch.h"
#include <string.h>
#include <unity.h>

static uint8_t g_stream[64 * 1024]; // every packet's compressed payload, concatenated
static size_t g_stream_len;
static uint8_t g_orig[64 * 1024]; // every original payload, concatenated
static size_t g_orig_len;
static uint8_t g_decoded[64 * 1024];
static uint8_t g_iscratch[INFLATE_SCRATCH_SIZE];

// Send one payload through ssh_pkt_send (unencrypted framing so the payload is visible), then pull
// the on-wire (compressed) payload out of the packet and accumulate it + the original.
static void send_and_capture(const uint8_t *payload, size_t n)
{
    uint8_t wire[SSH_WIRE_CAP];
    size_t wlen = 0;
    int rc = ssh_pkt_send(0, payload, n, wire, &wlen, sizeof(wire));
    TEST_ASSERT_EQUAL_INT(0, rc);

    // Unencrypted packet: [packet_length(4)] [padding_length(1)] [payload...] [padding].
    size_t pkt_len = ((size_t)wire[0] << 24) | ((size_t)wire[1] << 16) | ((size_t)wire[2] << 8) | wire[3];
    uint8_t pad_len = wire[4];
    TEST_ASSERT_TRUE(pkt_len >= (size_t)(1 + pad_len));
    size_t cpayload_len = pkt_len - 1 - pad_len;

    TEST_ASSERT_TRUE(g_stream_len + cpayload_len <= sizeof(g_stream));
    memcpy(g_stream + g_stream_len, wire + 5, cpayload_len);
    g_stream_len += cpayload_len;
    TEST_ASSERT_TRUE(g_orig_len + n <= sizeof(g_orig));
    memcpy(g_orig + g_orig_len, payload, n);
    g_orig_len += n;
}

// Decode the whole captured stream (minus the 2-byte zlib header) and compare to the originals.
static void verify()
{
    TEST_ASSERT_TRUE(g_stream_len >= 2);
    TEST_ASSERT_EQUAL_HEX8(0x78, g_stream[0]);
    TEST_ASSERT_EQUAL_HEX8(0x9C, g_stream[1]);
    size_t dlen = 0;
    int rc = (int)inflate_raw(g_stream + 2, g_stream_len - 2, g_decoded, sizeof(g_decoded), &dlen, g_iscratch,
                              sizeof(g_iscratch));
    TEST_ASSERT_EQUAL_INT(InflateResult::INFLATE_OK, rc);
    TEST_ASSERT_EQUAL_size_t(g_orig_len, dlen);
    TEST_ASSERT_EQUAL_MEMORY(g_orig, g_decoded, g_orig_len);
}

void setUp()
{
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    g_stream_len = 0;
    g_orig_len = 0;
}
void tearDown()
{
}

// zlib@openssh.com is delayed: not active until USERAUTH_SUCCESS, then active for every packet.
void test_delayed_activation()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0)); // negotiated but not started
    ssh_comp_on_newkeys(0);                    // must NOT start a delayed stream
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0); // now it starts
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
}

// zlib (non-delayed) starts right at NEWKEYS.
void test_immediate_activation()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
}

// "none": never activates, whatever the events.
void test_none_never_activates()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_NONE);
    ssh_comp_on_newkeys(0);
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
}

// The full path: activate, push a realistic terminal session, and prove the framed packets form one
// valid context-takeover zlib stream that decodes back to the originals.
void test_packet_layer_stream_roundtrip()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    const char *prompt = "user@esp32:~$ ";
    send_and_capture((const uint8_t *)prompt, strlen(prompt));
    const char *cmd = "uname -a\r\n";
    send_and_capture((const uint8_t *)cmd, strlen(cmd));
    send_and_capture((const uint8_t *)prompt, strlen(prompt)); // repeat -> cross-packet match
    const char *banner = "DeterministicESPAsyncWebServer 1.0 (xtensa)\r\n";
    send_and_capture((const uint8_t *)banner, strlen(banner));
    send_and_capture((const uint8_t *)"", 0); // empty write
    verify();
}

// A longer session that slides the window, driven through the packet layer.
void test_packet_layer_window_slide()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    ssh_comp_on_auth_success(0);
    uint8_t buf[1000];
    for (int k = 0; k < 30; k++)
    {
        for (int j = 0; j < (int)sizeof(buf); j++)
            buf[j] = (uint8_t)("log: event fired at t="[j % 22]) ^ (uint8_t)((j & 3) ? 0 : k);
        send_and_capture(buf, sizeof(buf));
    }
    verify();
}

// With s2c compression active, an exhausted scratch arena fails the compress-buffer alloc, so the
// send fails closed rather than emitting an uncompressed (desynced) packet.
void test_packet_compress_scratch_exhausted()
{
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB);
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    scratch_reset();
    while (scratch_alloc(8, 1))
        ; // drain the arena
    uint8_t payload[8] = {SSH_MSG_IGNORE, 1, 2, 3, 4, 5, 6, 7};
    uint8_t wire[SSH_WIRE_CAP];
    size_t wlen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_send(0, payload, sizeof(payload), wire, &wlen, sizeof(wire)));
    scratch_reset();
}

// Every entry point rejects an out-of-range slot (the i >= MAX_SSH_CONNS guard branch), and a valid
// but inactive slot cannot compress.
void test_comp_slot_guards()
{
    ssh_comp_reset(MAX_SSH_CONNS); // out-of-range: no-op, no crash
    ssh_comp_set_s2c(MAX_SSH_CONNS, SshCompAlg::SSH_COMP_ZLIB);
    ssh_comp_on_newkeys(MAX_SSH_CONNS);                    // out-of-range: no-op
    ssh_comp_on_auth_success(MAX_SSH_CONNS);               // out-of-range: no-op
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(MAX_SSH_CONNS)); // out-of-range slot is never active

    uint8_t src[8] = {0};
    uint8_t dst[64];
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_comp_s2c(MAX_SSH_CONNS, src, sizeof(src), dst, sizeof(dst), &out_len));
    ssh_comp_reset(0); // slot 0 valid but no stream started
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    TEST_ASSERT_EQUAL_INT(-1, ssh_comp_s2c(0, src, sizeof(src), dst, sizeof(dst), &out_len)); // inactive slot
}

// Activation is idempotent: a second NEWKEYS (zlib) or a second USERAUTH_SUCCESS (delayed) must not
// restart the stream - it drives the `already active` side of the start guard. The wrong event for a
// given algorithm is also a no-op.
void test_comp_activation_idempotent()
{
    // zlib: NEWKEYS starts it; a second NEWKEYS is a no-op (s2c_active already true), and USERAUTH is
    // the wrong event so it does nothing.
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB);
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    ssh_comp_on_newkeys(0); // idempotent
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0); // wrong event for zlib
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    // zlib@openssh.com (delayed): USERAUTH_SUCCESS starts it; a second one is idempotent, and NEWKEYS
    // is the wrong event.
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    ssh_comp_on_newkeys(0); // wrong event for delayed: must NOT start
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0); // idempotent
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
}

// ---- KEXINIT negotiation + the dispatcher's activation trigger -------------

static const uint8_t COMP_ED_SEED[32] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                         0xcc, 0xdd, 0xee, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                         0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

static size_t put_str(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, s, n);
    return 4 + n;
}

// Client KEXINIT carrying the eight negotiable name-lists in @p rows (kex, host key, cipher c2s/s2c,
// mac c2s/s2c, compression c2s/s2c).
static size_t build_client_kexinit(uint8_t *out, const char *const rows[8])
{
    size_t o = 0;
    out[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        out[o++] = (uint8_t)j; // cookie
    for (int j = 0; j < 8; j++)
        o += put_str(out + o, rows[j]);
    o += put_str(out + o, ""); // languages c2s
    o += put_str(out + o, ""); // languages s2c
    out[o++] = 0;              // first_kex_packet_follows
    for (int j = 0; j < 4; j++)
        out[o++] = 0; // reserved
    return o;
}

// With s2c compression built in, KEXINIT picks the client's best offer in server preference order
// (zlib@openssh.com > zlib > none) and refuses a client that offers none of the three.
void test_kexinit_negotiates_s2c_compression()
{
    dws_ssh_hostkey_ed25519_set(COMP_ED_SEED);
    const char *K = "curve25519-sha256";
    const char *H = "ssh-ed25519";
    const char *C = "aes256-ctr";
    const char *M = "hmac-sha2-256";
    uint8_t buf[512];

    // zlib@openssh.com is delayed: negotiated at KEXINIT, started only at USERAUTH_SUCCESS.
    ssh_transport_init(0);
    ssh_comp_reset(0);
    const char *delayed[8] = {K, H, C, C, M, M, "none", "zlib@openssh.com,none"};
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, buf, build_client_kexinit(buf, delayed)));
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    // Plain "zlib" is chosen when the delayed variant is not offered, and starts at NEWKEYS.
    ssh_transport_init(0);
    ssh_comp_reset(0);
    const char *immediate[8] = {K, H, C, C, M, M, "none", "zlib,none"};
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, buf, build_client_kexinit(buf, immediate)));
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    // "none" is still a valid outcome and never starts a stream.
    ssh_transport_init(0);
    ssh_comp_reset(0);
    const char *plain[8] = {K, H, C, C, M, M, "none", "none"};
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, buf, build_client_kexinit(buf, plain)));
    ssh_comp_on_newkeys(0);
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));

    // A client offering nothing we implement for s2c has no mutual algorithm.
    ssh_transport_init(0);
    ssh_comp_reset(0);
    const char *unknown[8] = {K, H, C, C, M, M, "none", "lzo@openssh.com"};
    TEST_ASSERT_EQUAL_INT(-1, ssh_kexinit_parse(0, buf, build_client_kexinit(buf, unknown)));
}

// Before the s2c stream starts, ssh_pkt_send frames the payload verbatim - the compress step is
// skipped entirely, so the bytes on the wire are the payload itself.
void test_packet_send_uncompressed_before_activation()
{
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    const uint8_t payload[6] = {SSH_MSG_IGNORE, 'p', 'l', 'a', 'i', 'n'};
    static uint8_t wire[SSH_WIRE_CAP];
    size_t wlen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_pkt_send(0, payload, sizeof(payload), wire, &wlen, sizeof(wire)));
    TEST_ASSERT_EQUAL_MEMORY(payload, wire + 5, sizeof(payload));
}

// ssh_newkeys_sent turns the outbound direction on and starts a non-delayed "zlib" stream with it;
// zlib@openssh.com is untouched (it waits for USERAUTH_SUCCESS).
void test_newkeys_sent_starts_immediate_stream_only()
{
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB);
    ssh_newkeys_sent(0);
    TEST_ASSERT_TRUE(ssh_pkt[0].enc_out);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    ssh_pkt_init(0);
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    ssh_newkeys_sent(0);
    TEST_ASSERT_TRUE(ssh_pkt[0].enc_out);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));

    ssh_newkeys_sent(MAX_SSH_CONNS); // out-of-range slot: no-op, no crash
    ssh_pkt_init(0);
    ssh_comp_reset(0);
}

// A payload larger than the compressor's maximum input fails the send closed. Emitting it
// uncompressed instead would desync the context-takeover stream for every packet after it.
void test_packet_compress_rejects_oversized_payload()
{
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB);
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    scratch_reset();

    static uint8_t payload[DWS_SSH_ZLIB_MAX_IN + 1];
    memset(payload, 'z', sizeof(payload));
    payload[0] = SSH_MSG_IGNORE;
    static uint8_t wire[SSH_WIRE_CAP];
    size_t wlen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_send(0, payload, sizeof(payload), wire, &wlen, sizeof(wire)));
    ssh_comp_reset(0);
    ssh_pkt_init(0);
}

static uint8_t comp_emt[16];
static int comp_emt_n;
static void comp_rec_emit(uint8_t slot, const uint8_t *p, size_t n)
{
    (void)slot;
    if (comp_emt_n < 16 && n > 0)
        comp_emt[comp_emt_n++] = p[0];
}
static bool comp_pw_cb(const char *u, const char *p)
{
    return strcmp(u, "alice") == 0 && strcmp(p, "s3cret") == 0;
}

// zlib@openssh.com starts its stream on the first packet AFTER USERAUTH_SUCCESS, so the dispatcher
// fires the trigger on a success and leaves it alone on a failure.
void test_dispatch_auth_success_starts_delayed_compression()
{
    ssh_transport_init(0);
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    ssh_comp_set_s2c(0, SshCompAlg::SSH_COMP_ZLIB_DELAYED);
    dws_ssh_auth_set_password_cb(comp_pw_cb);
    dws_ssh_server_set_emit_cb(comp_rec_emit);

    uint8_t pkt[128];
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_str(pkt + n, "alice");
    n += put_str(pkt + n, "ssh-connection");
    n += put_str(pkt + n, "password");
    pkt[n++] = 0;
    size_t base = n;

    // A rejected password leaves the stream stopped.
    n = base + put_str(pkt + base, "wrong");
    ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH;
    comp_emt_n = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_REQUEST, pkt, n));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, comp_emt[0]);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));

    // The accepted password starts it.
    n = base + put_str(pkt + base, "s3cret");
    ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH;
    comp_emt_n = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_REQUEST, pkt, n));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_SUCCESS, comp_emt[0]);
    TEST_ASSERT_TRUE(ssh_sess[0].authed);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
    ssh_comp_reset(0);
}

// ============================================================================
// AES-256-CTR (network_drivers/presentation/ssh/crypto/ssh_aes256ctr.cpp)
// ============================================================================
// This env's own tests only drive ssh_pkt_send before NEWKEYS (no cipher active), so the native
// software AES-256-CTR path is otherwise never called here. Exercised directly below.

static void aes_hex_to_bytes(uint8_t *out, const char *hex, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        unsigned hi = hex[2 * i] >= 'a'   ? hex[2 * i] - 'a' + 10
                      : hex[2 * i] >= 'A' ? hex[2 * i] - 'A' + 10
                                          : hex[2 * i] - '0';
        unsigned lo = hex[2 * i + 1] >= 'a'   ? hex[2 * i + 1] - 'a' + 10
                      : hex[2 * i + 1] >= 'A' ? hex[2 * i + 1] - 'A' + 10
                                              : hex[2 * i + 1] - '0';
        out[i] = (uint8_t)((hi << 4) | lo);
    }
}

// NIST SP 800-38A §F.5.5 (AES-256-CTR), blocks 1-4: encrypt then decrypt must round-trip (CTR is its
// own inverse), and the 64-byte run crosses 4 keystream blocks, driving the big-endian counter
// increment across a byte carry (…feff -> …ff00). Also proves ssh_aes256ctr_wipe() zeroes the context.
void test_aes256ctr_nist_vector_roundtrip_and_wipe(void)
{
    uint8_t key[32], iv[16];
    aes_hex_to_bytes(key, "603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", 32);
    aes_hex_to_bytes(iv, "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff", 16);

    uint8_t pt[64], expected_ct[64];
    aes_hex_to_bytes(pt,
                     "6bc1bee22e409f96e93d7e117393172a"
                     "ae2d8a571e03ac9c9eb76fac45af8e51"
                     "30c81c46a35ce411e5fbc1191a0a52ef"
                     "f69f2445df4f9b17ad2b417be66c3710",
                     64);
    aes_hex_to_bytes(expected_ct,
                     "601ec313775789a5b7a7f504bbf3d228"
                     "f443e3ca4d62b59aca84e990cacaf5c5"
                     "2b0930daa23de94ce87017ba2d84988d"
                     "dfc9c58db67aada613c2dd08457941a6",
                     64);

    SshAesCtrCtx ctx;
    uint8_t ct[64];
    ssh_aes256ctr_init(&ctx, key, iv);
    ssh_aes256ctr_crypt(&ctx, pt, ct, 64);
    TEST_ASSERT_EQUAL_MEMORY(expected_ct, ct, 64);

    uint8_t pt_back[64];
    ssh_aes256ctr_init(&ctx, key, iv); // decrypt = re-init + the same XOR pass over the ciphertext
    ssh_aes256ctr_crypt(&ctx, ct, pt_back, 64);
    TEST_ASSERT_EQUAL_MEMORY(pt, pt_back, 64);

    ssh_aes256ctr_wipe(&ctx);
    uint8_t zeros[sizeof(SshAesCtrCtx)] = {0};
    TEST_ASSERT_EQUAL_MEMORY(zeros, &ctx, sizeof(SshAesCtrCtx));
}

// The big-endian 128-bit counter incrementer carries out of every byte only when the whole counter is
// 0xFF (…ff -> …00 all the way to byte 0) - the one input that runs the increment loop to completion
// without ever taking its early "break" exit.
void test_aes256ctr_counter_full_wraparound(void)
{
    uint8_t key[32] = {0};
    uint8_t iv[16];
    memset(iv, 0xFF, sizeof(iv));

    SshAesCtrCtx ctx;
    ssh_aes256ctr_init(&ctx, key, iv);
    uint8_t pt[16] = {0}, ct[16];
    ssh_aes256ctr_crypt(&ctx, pt, ct, 16); // one block: encrypts under the all-0xFF counter, then wraps it

    uint8_t zero_counter[16] = {0};
    TEST_ASSERT_EQUAL_MEMORY(zero_counter, ctx.counter, 16);
    ssh_aes256ctr_wipe(&ctx);
}

// ============================================================================
// DH-group14-SHA256 KEX (network_drivers/presentation/ssh/transport/ssh_dh.cpp)
// ============================================================================
// No KEX ever runs in this env's own tests, so ssh_dh_generate()/the RFC 4253 §7.2 key derivation
// chain are otherwise never called here. Exercised directly below.

// Out-of-range slot is rejected; a valid slot generates y/f and leaves kex_done false (NEWKEYS has
// not happened yet).
void test_dh_generate_slot_guard_and_state(void)
{
    TEST_ASSERT_EQUAL_INT(-1, ssh_dh_generate(MAX_SSH_CONNS));
    TEST_ASSERT_EQUAL_INT(0, ssh_dh_generate(0));
    TEST_ASSERT_FALSE(ssh_dh[0].kex_done);
    bool f_nonzero = false;
    for (int j = 0; j < SSH_BN_LIMBS; j++)
        if (ssh_dh[0].f.d[j] != 0)
            f_nonzero = true;
    TEST_ASSERT_TRUE(f_nonzero); // f = g^y mod p, g = 2, y != 0 -> f != 0
    ssh_dh_wipe(0);
}

// ssh_dh_derive_keys() is the first-KEX convenience wrapper (session_id == H, aes256-ctr + hmac-sha2-256
// default); ssh_dh_derive_keys_sid() rejects an out-of-range slot as a no-op.
void test_dh_derive_keys_default_wrapper_and_slot_guard(void)
{
    uint8_t K[256];
    memset(K, 0, sizeof(K));
    K[0] = 0x01; // nonzero shared secret
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    for (int j = 0; j < SSH_SHA256_DIGEST_LEN; j++)
        H[j] = (uint8_t)(j + 1);

    ssh_keymat_wipe(0);
    ssh_dh_derive_keys(0, K, H);
    TEST_ASSERT_TRUE(ssh_keys[0].active);
    TEST_ASSERT_EQUAL_UINT8(SSH_CIPHER_AES256CTR, ssh_keys[0].cipher_mode);
    TEST_ASSERT_EQUAL_UINT8(SSH_MAC_HMAC_SHA256, ssh_keys[0].mac_mode);
    ssh_keymat_wipe(0);

    // Out-of-range slot: must not crash and must not touch any real slot's state.
    ssh_dh_derive_keys_sid(MAX_SSH_CONNS, K, H, H, SSH_CIPHER_AES256CTR, SSH_MAC_HMAC_SHA256);
}

// The cipher_alg dispatch inside ssh_dh_derive_keys_sid(): chacha20-poly1305 installs two 512-bit keys
// (no separate MAC key), aes256-gcm installs a stateful AEAD context per direction (no separate MAC key
// either). Both are otherwise-untested branches - only the aes256-ctr default (the "neither" fallthrough)
// is reached via test_dh_derive_keys_default_wrapper_and_slot_guard.
void test_dh_derive_keys_chachapoly_and_gcm_branches(void)
{
    uint8_t K[256];
    memset(K, 0, sizeof(K));
    K[0] = 0x42;
    uint8_t H[SSH_SHA256_DIGEST_LEN], sid[SSH_SHA256_DIGEST_LEN];
    for (int j = 0; j < SSH_SHA256_DIGEST_LEN; j++)
    {
        H[j] = (uint8_t)(0x10 + j);
        sid[j] = (uint8_t)(0x50 + j);
    }

    ssh_keymat_wipe(0);
    ssh_dh_derive_keys_sid(0, K, H, sid, SSH_CIPHER_CHACHA20POLY1305, SSH_MAC_HMAC_SHA256);
    TEST_ASSERT_TRUE(ssh_keys[0].active);
    TEST_ASSERT_EQUAL_UINT8(SSH_CIPHER_CHACHA20POLY1305, ssh_keys[0].cipher_mode);

    ssh_keymat_wipe(0);
    ssh_dh_derive_keys_sid(0, K, H, sid, SSH_CIPHER_AES256GCM, SSH_MAC_HMAC_SHA256);
    TEST_ASSERT_TRUE(ssh_keys[0].active);
    TEST_ASSERT_EQUAL_UINT8(SSH_CIPHER_AES256GCM, ssh_keys[0].cipher_mode);
    ssh_keymat_wipe(0);
}

// Independently replicates hash_mpint_K's RFC 4251 §5 mpint encoding of K plus the K1 = HASH(mpint(K)
// || H || label || session_id) step, so each K encoding edge case below can be checked against a value
// computed a different way than ssh_dh.cpp itself (mirrors test_ssh_transport's KDF verification style).
static void expected_kdf_k1(const uint8_t K[256], const uint8_t H[SSH_SHA256_DIGEST_LEN], char label,
                            const uint8_t *sid, size_t sid_len, uint8_t out[SSH_SHA256_DIGEST_LEN])
{
    size_t off = 0;
    while (off < 256 && K[off] == 0x00u)
        off++;
    SshSha256Ctx c;
    ssh_sha256_init(&c);
    if (off == 256)
    {
        uint8_t len_be[4] = {0, 0, 0, 0};
        ssh_sha256_update(&c, len_be, 4);
    }
    else
    {
        bool pad = (K[off] & 0x80u) != 0;
        uint32_t mlen = (uint32_t)(256 - off) + (pad ? 1u : 0u);
        uint8_t len_be[4] = {(uint8_t)(mlen >> 24), (uint8_t)(mlen >> 16), (uint8_t)(mlen >> 8), (uint8_t)mlen};
        ssh_sha256_update(&c, len_be, 4);
        if (pad)
        {
            uint8_t zero = 0x00u;
            ssh_sha256_update(&c, &zero, 1);
        }
        ssh_sha256_update(&c, K + off, 256 - off);
    }
    ssh_sha256_update(&c, H, SSH_SHA256_DIGEST_LEN);
    uint8_t lbl = (uint8_t)label;
    ssh_sha256_update(&c, &lbl, 1);
    ssh_sha256_update(&c, sid, sid_len);
    ssh_sha256_final(&c, out);
}

// hash_mpint_K has three edge branches a real DH secret never hits on its own, but the public
// ssh_kdf_derive() entry point can be driven through directly with any K: an all-zero K (empty mpint),
// leading zero bytes stripped before a nonzero byte with the MSB clear (no pad), and a leading byte
// with the MSB set (needs the extra 0x00 pad byte so K is not misread as a negative mpint).
void test_kdf_mpint_k_edge_encodings(void)
{
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    for (int j = 0; j < SSH_SHA256_DIGEST_LEN; j++)
        H[j] = (uint8_t)(0x20 + j);

    uint8_t K_zero[256];
    memset(K_zero, 0, sizeof(K_zero));
    uint8_t out_zero[SSH_SHA256_DIGEST_LEN], expected_zero[SSH_SHA256_DIGEST_LEN];
    ssh_kdf_derive(K_zero, H, H, 'A', out_zero, SSH_SHA256_DIGEST_LEN);
    expected_kdf_k1(K_zero, H, 'A', H, SSH_SHA256_DIGEST_LEN, expected_zero);
    TEST_ASSERT_EQUAL_MEMORY(expected_zero, out_zero, SSH_SHA256_DIGEST_LEN);

    uint8_t K_lead_zero[256];
    memset(K_lead_zero, 0, sizeof(K_lead_zero));
    for (int j = 2; j < 256; j++)
        K_lead_zero[j] = (uint8_t)(j & 0x7F); // K_lead_zero[2] != 0 and its MSB is clear -> no pad
    uint8_t out_lz[SSH_SHA256_DIGEST_LEN], expected_lz[SSH_SHA256_DIGEST_LEN];
    ssh_kdf_derive(K_lead_zero, H, H, 'B', out_lz, SSH_SHA256_DIGEST_LEN);
    expected_kdf_k1(K_lead_zero, H, 'B', H, SSH_SHA256_DIGEST_LEN, expected_lz);
    TEST_ASSERT_EQUAL_MEMORY(expected_lz, out_lz, SSH_SHA256_DIGEST_LEN);

    uint8_t K_msb[256];
    memset(K_msb, 0, sizeof(K_msb));
    K_msb[0] = 0x91; // MSB set on the very first byte -> pad byte required
    uint8_t out_msb[SSH_SHA256_DIGEST_LEN], expected_msb[SSH_SHA256_DIGEST_LEN];
    ssh_kdf_derive(K_msb, H, H, 'C', out_msb, SSH_SHA256_DIGEST_LEN);
    expected_kdf_k1(K_msb, H, 'C', H, SSH_SHA256_DIGEST_LEN, expected_msb);
    TEST_ASSERT_EQUAL_MEMORY(expected_msb, out_msb, SSH_SHA256_DIGEST_LEN);
}

// Hybrid KEX (mlkem768x25519-sha256 etc.): k_is_string=true hashes K as a plain SSH string (the last 32
// octets of the buffer), not as a canonical mpint, and must differ from the mpint encoding of the same K.
void test_kdf_string_k_hybrid_branch(void)
{
    uint8_t K[256];
    for (int j = 0; j < 256; j++)
        K[j] = (uint8_t)(j * 7 + 1);
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    for (int j = 0; j < SSH_SHA256_DIGEST_LEN; j++)
        H[j] = (uint8_t)(0x60 + j);

    uint8_t got_string[SSH_SHA256_DIGEST_LEN];
    ssh_kdf_derive(K, H, H, 'C', got_string, SSH_SHA256_DIGEST_LEN, true);

    uint8_t len_be[4] = {0, 0, 0, 32};
    SshSha256Ctx c;
    ssh_sha256_init(&c);
    ssh_sha256_update(&c, len_be, 4);
    ssh_sha256_update(&c, K + (256 - 32), 32);
    ssh_sha256_update(&c, H, SSH_SHA256_DIGEST_LEN);
    uint8_t lbl = 'C';
    ssh_sha256_update(&c, &lbl, 1);
    ssh_sha256_update(&c, H, SSH_SHA256_DIGEST_LEN);
    uint8_t expected[SSH_SHA256_DIGEST_LEN];
    ssh_sha256_final(&c, expected);
    TEST_ASSERT_EQUAL_MEMORY(expected, got_string, SSH_SHA256_DIGEST_LEN);

    uint8_t got_mpint[SSH_SHA256_DIGEST_LEN];
    ssh_kdf_derive(K, H, H, 'C', got_mpint, SSH_SHA256_DIGEST_LEN, false);
    TEST_ASSERT_NOT_EQUAL(0, memcmp(got_string, got_mpint, SSH_SHA256_DIGEST_LEN));
}

// out_len beyond SSH_KDF_MAX clamps rather than overflowing the caller's buffer, and the extension
// chain (K1 || K2 || ...) that produces bytes past the first hash block runs identically whether the
// caller asked for exactly SSH_KDF_MAX or something larger that gets clamped down to it.
void test_kdf_out_len_clamp_matches_exact_max(void)
{
    uint8_t K[256];
    for (int j = 0; j < 256; j++)
        K[j] = (uint8_t)(j ^ 0x5A); // K[0] = 0x5A: nonzero, MSB clear -> no pad, mlen = 256
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    for (int j = 0; j < SSH_SHA256_DIGEST_LEN; j++)
        H[j] = (uint8_t)(0x30 + j);

    uint8_t clamped[SSH_KDF_MAX];
    ssh_kdf_derive(K, H, H, 'X', clamped, SSH_KDF_MAX + 64); // over SSH_KDF_MAX -> clamps

    uint8_t exact[SSH_KDF_MAX];
    ssh_kdf_derive(K, H, H, 'X', exact, SSH_KDF_MAX); // already at the max -> no clamp needed
    TEST_ASSERT_EQUAL_MEMORY(exact, clamped, SSH_KDF_MAX);

    // K1 (the first 32 bytes of the chain) is a direct, independently-computed hash.
    uint8_t expected_k1[SSH_SHA256_DIGEST_LEN];
    expected_kdf_k1(K, H, 'X', H, SSH_SHA256_DIGEST_LEN, expected_k1);
    TEST_ASSERT_EQUAL_MEMORY(expected_k1, clamped, SSH_SHA256_DIGEST_LEN);

    // K2 = HASH(mpint(K) || H || K1) - the chain extension step carries no label/session_id.
    uint8_t len_be[4] = {0, 0, 1, 0};
    SshSha256Ctx c;
    ssh_sha256_init(&c);
    ssh_sha256_update(&c, len_be, 4);
    ssh_sha256_update(&c, K, 256);
    ssh_sha256_update(&c, H, SSH_SHA256_DIGEST_LEN);
    ssh_sha256_update(&c, clamped, SSH_SHA256_DIGEST_LEN); // K1 = acc[0..32)
    uint8_t expected_k2[SSH_SHA256_DIGEST_LEN];
    ssh_sha256_final(&c, expected_k2);
    TEST_ASSERT_EQUAL_MEMORY(expected_k2, clamped + SSH_SHA256_DIGEST_LEN, SSH_SHA256_DIGEST_LEN);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_delayed_activation);
    RUN_TEST(test_immediate_activation);
    RUN_TEST(test_none_never_activates);
    RUN_TEST(test_packet_layer_stream_roundtrip);
    RUN_TEST(test_packet_layer_window_slide);
    RUN_TEST(test_packet_compress_scratch_exhausted);
    RUN_TEST(test_comp_slot_guards);
    RUN_TEST(test_comp_activation_idempotent);
    RUN_TEST(test_kexinit_negotiates_s2c_compression);
    RUN_TEST(test_packet_send_uncompressed_before_activation);
    RUN_TEST(test_newkeys_sent_starts_immediate_stream_only);
    RUN_TEST(test_packet_compress_rejects_oversized_payload);
    RUN_TEST(test_dispatch_auth_success_starts_delayed_compression);
    RUN_TEST(test_aes256ctr_nist_vector_roundtrip_and_wipe);
    RUN_TEST(test_aes256ctr_counter_full_wraparound);
    RUN_TEST(test_dh_generate_slot_guard_and_state);
    RUN_TEST(test_dh_derive_keys_default_wrapper_and_slot_guard);
    RUN_TEST(test_dh_derive_keys_chachapoly_and_gcm_branches);
    RUN_TEST(test_kdf_mpint_k_edge_encodings);
    RUN_TEST(test_kdf_string_k_hybrid_branch);
    RUN_TEST(test_kdf_out_len_clamp_matches_exact_max);
    return UNITY_END();
}
