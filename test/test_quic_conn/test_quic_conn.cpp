// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the QUIC v1 server connection engine (network_drivers/presentation/http3/quic_conn;
// RFC 9000 / RFC 9001). The test acts as a QUIC *client*: it builds real Initial / Handshake / 1-RTT
// packets, protects and opens them with the same crypto the engine uses, and drives a server QuicConn
// through a full handshake and a 1-RTT request/response stream exchange - proving the engine's packet
// parsing, per-level AEAD, CRYPTO reassembly, ACK generation, coalescing, HANDSHAKE_DONE, and stream
// plumbing all interoperate with an independent implementation.

#include "network_drivers/presentation/http3/quic_conn.h"
#include "network_drivers/presentation/http3/quic_crypto.h"
#include "network_drivers/presentation/http3/quic_frame.h"
#include "network_drivers/presentation/http3/quic_packet.h"
#include "network_drivers/presentation/http3/quic_varint.h"
#include "network_drivers/presentation/http3/tls13_kdf.h"
#include "network_drivers/presentation/http3/tls13_msg.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static const uint8_t CERT[48] = {0x30, 0x2e, 0x02, 0x01, 0x02};
static uint8_t SERVER_PRIV[32], SERVER_SEED[32], SERVER_RANDOM[32], CLIENT_PRIV[32];
static const uint8_t ODCID[8] = {0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8};
static const uint8_t CLIENT_SCID[4] = {0xc1, 0xc2, 0xc3, 0xc4};
static const uint8_t SERVER_SCID[4] = {0x51, 0x52, 0x53, 0x54};

// Callback capture.
static bool g_hs_done;
static uint8_t g_stream_data[256];
static size_t g_stream_len;
static uint64_t g_stream_id;
static bool g_stream_fin;

static void on_hs_done(void *, QuicConn *)
{
    g_hs_done = true;
}
static void on_stream_data(void *, QuicConn *, uint64_t id, const uint8_t *data, size_t len, bool fin)
{
    g_stream_id = id;
    if (len && g_stream_len + len <= sizeof(g_stream_data))
    {
        memcpy(g_stream_data + g_stream_len, data, len);
        g_stream_len += len;
    }
    g_stream_fin = fin;
}

static void fill()
{
    for (int i = 0; i < 32; i++)
    {
        SERVER_PRIV[i] = (uint8_t)(0x40 + i);
        SERVER_SEED[i] = (uint8_t)(0x80 + i);
        SERVER_RANDOM[i] = (uint8_t)(0xA0 + i);
        CLIENT_PRIV[i] = (uint8_t)(0x01 + i);
    }
    g_hs_done = false;
    g_stream_len = 0;
    g_stream_id = 0;
    g_stream_fin = false;
}

static size_t build_client_hello(uint8_t *out, const uint8_t client_pub[32], const uint8_t *tp, size_t tp_len)
{
    size_t p = 0;
    out[p++] = TLS_HS_CLIENT_HELLO;
    size_t hs_len_at = p;
    p += 3;
    out[p++] = 0x03;
    out[p++] = 0x03;
    for (int i = 0; i < 32; i++)
        out[p++] = (uint8_t)i;
    out[p++] = 0x00; // session id
    out[p++] = 0x00;
    out[p++] = 0x02;
    out[p++] = 0x13;
    out[p++] = 0x01; // cipher suites
    out[p++] = 0x01;
    out[p++] = 0x00; // compression
    size_t ext_at = p;
    p += 2;
    static const uint8_t sv[] = {0x00, 0x2b, 0x00, 0x03, 0x02, 0x03, 0x04};
    static const uint8_t sg[] = {0x00, 0x0a, 0x00, 0x04, 0x00, 0x02, 0x00, 0x1d};
    static const uint8_t sa[] = {0x00, 0x0d, 0x00, 0x04, 0x00, 0x02, 0x08, 0x07};
    static const uint8_t ks[] = {0x00, 0x33, 0x00, 0x26, 0x00, 0x24, 0x00, 0x1d, 0x00, 0x20};
    static const uint8_t al[] = {0x00, 0x10, 0x00, 0x05, 0x00, 0x03, 0x02, 'h', '3'};
    memcpy(out + p, sv, sizeof(sv));
    p += sizeof(sv);
    memcpy(out + p, sg, sizeof(sg));
    p += sizeof(sg);
    memcpy(out + p, sa, sizeof(sa));
    p += sizeof(sa);
    memcpy(out + p, ks, sizeof(ks));
    p += sizeof(ks);
    memcpy(out + p, client_pub, 32);
    p += 32;
    memcpy(out + p, al, sizeof(al));
    p += sizeof(al);
    out[p++] = 0x00;
    out[p++] = 0x39;
    out[p++] = (uint8_t)(tp_len >> 8);
    out[p++] = (uint8_t)tp_len;
    memcpy(out + p, tp, tp_len);
    p += tp_len;
    uint16_t el = (uint16_t)(p - ext_at - 2);
    out[ext_at] = (uint8_t)(el >> 8);
    out[ext_at + 1] = (uint8_t)el;
    uint32_t hl = (uint32_t)(p - hs_len_at - 3);
    out[hs_len_at] = (uint8_t)(hl >> 16);
    out[hs_len_at + 1] = (uint8_t)(hl >> 8);
    out[hs_len_at + 2] = (uint8_t)hl;
    return p;
}

static void wr_pn(uint8_t *o, uint64_t pn, uint8_t pn_len)
{
    for (uint8_t i = 0; i < pn_len; i++)
        o[i] = (uint8_t)(pn >> (8 * (pn_len - 1 - i)));
}

// Build a protected long-header packet; returns total length.
static size_t build_long(uint8_t *out, size_t cap, uint8_t type, const uint8_t *dcid, uint8_t dcl, const uint8_t *scid,
                         uint8_t scl, uint64_t pn, const QuicPacketKeys *keys, const uint8_t *frames, size_t frame_len)
{
    uint8_t pn_len = quic_pn_length(pn, -1);
    size_t p = quic_build_long_header(out, cap, type, QUIC_VERSION_1, dcid, dcl, scid, scl, pn_len);
    if (type == QUIC_LP_INITIAL)
        p += quic_varint_encode(out + p, cap - p, 0);
    uint64_t length = (uint64_t)pn_len + frame_len + 16;
    p += quic_varint_encode(out + p, cap - p, length);
    size_t pn_off = p;
    wr_pn(out + p, pn, pn_len);
    p += pn_len;
    memcpy(out + p, frames, frame_len);
    return quic_packet_protect(out, cap, pn_off, pn_len, pn, frame_len, keys, true);
}

static size_t build_short(uint8_t *out, size_t cap, const uint8_t *dcid, uint8_t dcl, uint64_t pn,
                          const QuicPacketKeys *keys, const uint8_t *frames, size_t frame_len)
{
    uint8_t pn_len = quic_pn_length(pn, -1);
    out[0] = (uint8_t)(0x40 | (pn_len - 1));
    memcpy(out + 1, dcid, dcl);
    size_t pn_off = 1 + dcl;
    wr_pn(out + pn_off, pn, pn_len);
    memcpy(out + pn_off + pn_len, frames, frame_len);
    return quic_packet_protect(out, cap, pn_off, pn_len, pn, frame_len, keys, false);
}

// Open a long-header packet at dg; returns plaintext length (or SIZE_MAX) and the on-wire length.
static size_t open_long(const uint8_t *dg, size_t len, const QuicPacketKeys *keys, uint8_t *plain, size_t *wire_len,
                        uint8_t *type_out)
{
    QuicLongHeader h;
    TEST_ASSERT_TRUE(quic_parse_long_header(dg, len, &h));
    *type_out = h.type;
    size_t off = h.hdr_len;
    if (h.type == QUIC_LP_INITIAL)
    {
        uint64_t tl = 0;
        size_t c = 0;
        quic_varint_decode(dg + off, len - off, &tl, &c);
        off += c + (size_t)tl;
    }
    uint64_t length = 0;
    size_t c = 0;
    quic_varint_decode(dg + off, len - off, &length, &c);
    off += c;
    *wire_len = off + (size_t)length;
    static uint8_t work[2048];
    memcpy(work, dg, *wire_len);
    uint64_t pn = 0;
    return quic_packet_unprotect(work, off, (size_t)length, 0, keys, true, plain, &pn);
}

static size_t open_short(const uint8_t *dg, size_t len, uint8_t dcl, const QuicPacketKeys *keys, uint8_t *plain)
{
    static uint8_t work[2048];
    memcpy(work, dg, len);
    uint64_t pn = 0;
    return quic_packet_unprotect(work, 1 + dcl, len - (1 + dcl), 0, keys, false, plain, &pn);
}

// Scan a decrypted payload for a CRYPTO frame and copy its data out; returns bytes copied.
static size_t extract_crypto(const uint8_t *p, size_t len, uint8_t *out)
{
    size_t off = 0, got = 0;
    while (off < len)
    {
        if (p[off] == QUIC_FT_PADDING)
        {
            off++;
            continue;
        }
        QuicFrame f;
        size_t n = quic_frame_parse(p + off, len - off, &f);
        if (!n)
            break;
        off += n;
        if (f.type == QUIC_FT_CRYPTO)
        {
            memcpy(out + got, f.crypto.data, (size_t)f.crypto.length);
            got += (size_t)f.crypto.length;
        }
    }
    return got;
}

static bool has_frame(const uint8_t *p, size_t len, uint64_t want)
{
    size_t off = 0;
    while (off < len)
    {
        if (p[off] == QUIC_FT_PADDING)
        {
            off++;
            continue;
        }
        QuicFrame f;
        size_t n = quic_frame_parse(p + off, len - off, &f);
        if (!n)
            break;
        off += n;
        if (f.type == want)
            return true;
    }
    return false;
}

static void make_cfg(QuicTlsConfig *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->cert_der = CERT;
    cfg->cert_len = sizeof(CERT);
    memcpy(cfg->ed25519_seed, SERVER_SEED, 32);
    memcpy(cfg->ephemeral_priv, SERVER_PRIV, 32);
    memcpy(cfg->random, SERVER_RANDOM, 32);
    quic_tp_defaults(&cfg->params);
    cfg->params.initial_max_data = 1048576;
    cfg->params.initial_max_sd_bidi_remote = 262144;
    cfg->params.initial_max_streams_bidi = 8;
}

void test_full_handshake_and_stream()
{
    fill();
    QuicTlsConfig cfg;
    make_cfg(&cfg);
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    quic_conn_init(&qc, &cfg, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), SERVER_SCID, sizeof(SERVER_SCID),
                   &cb);

    // Client Initial keys from the same DCID the server used.
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);

    // --- Client -> server: Initial(CRYPTO ClientHello), padded ---
    QuicTransportParams ctp;
    quic_tp_defaults(&ctp);
    ctp.initial_max_data = 524288;
    ctp.initial_max_sd_bidi_local = 131072;
    uint8_t ctp_enc[128];
    size_t ctp_len = quic_tp_encode(&ctp, ctp_enc, sizeof(ctp_enc));
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_PRIV);
    uint8_t ch[512];
    size_t ch_len = build_client_hello(ch, client_pub, ctp_enc, ctp_len);

    uint8_t frames[1200];
    size_t fl = quic_build_crypto(frames, sizeof(frames), 0, ch, ch_len);
    memset(frames + fl, 0, 1100 - fl); // PADDING to give the server a comfortable amp budget
    fl = 1100;
    uint8_t dg[1500];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, frames, fl);
    TEST_ASSERT_TRUE(quic_conn_recv(&qc, dg, dl));

    // --- Server -> client: Initial(SH) + Handshake(EE..Finished) ---
    uint8_t sdg[1500];
    size_t sl = quic_conn_send(&qc, sdg, sizeof(sdg));
    TEST_ASSERT_TRUE(sl > 0);

    uint8_t plain[2048], sh[512], hsflight[1024];
    size_t wire = 0;
    uint8_t type = 0;
    size_t pt = open_long(sdg, sl, &init.server, plain, &wire, &type);
    TEST_ASSERT_EQUAL_UINT8(QUIC_LP_INITIAL, type);
    size_t sh_len = extract_crypto(plain, pt, sh);
    TEST_ASSERT_EQUAL_UINT8(TLS_HS_SERVER_HELLO, sh[0]);

    // Derive the handshake secrets (client side) and open the Handshake packet.
    uint8_t server_pub[32], ecdhe[32];
    ssh_x25519_base(server_pub, SERVER_PRIV);
    ssh_x25519(ecdhe, CLIENT_PRIV, server_pub);
    SshSha256Ctx t;
    uint8_t ch_sh[32], ch_sf[32];
    ssh_sha256_init(&t);
    ssh_sha256_update(&t, ch, ch_len);
    ssh_sha256_update(&t, sh, sh_len);
    {
        SshSha256Ctx tmp = t;
        ssh_sha256_final(&tmp, ch_sh);
    }
    Tls13KeySchedule cks;
    tls13_ks_early(&cks);
    tls13_ks_handshake(&cks, ecdhe, ch_sh);
    QuicPacketKeys hs_server_keys, hs_client_keys;
    quic_keys_from_secret(cks.server_hs_traffic, &hs_server_keys);
    quic_keys_from_secret(cks.client_hs_traffic, &hs_client_keys);

    size_t hswire = 0;
    uint8_t hstype = 0;
    size_t hpt = open_long(sdg + wire, sl - wire, &hs_server_keys, plain, &hswire, &hstype);
    TEST_ASSERT_EQUAL_UINT8(QUIC_LP_HANDSHAKE, hstype);
    size_t hsflen = extract_crypto(plain, hpt, hsflight);
    TEST_ASSERT_EQUAL_UINT8(TLS_HS_ENCRYPTED_EXTENSIONS, hsflight[0]);

    // Full transcript -> application keys.
    ssh_sha256_update(&t, hsflight, hsflen);
    ssh_sha256_final(&t, ch_sf);
    tls13_ks_master(&cks, ch_sf);
    QuicPacketKeys ap_server_keys, ap_client_keys;
    quic_keys_from_secret(cks.server_ap_traffic, &ap_server_keys);
    quic_keys_from_secret(cks.client_ap_traffic, &ap_client_keys);
    // Diagnostic: the client-derived keys must equal the server's.
    TEST_ASSERT_EQUAL_UINT8_ARRAY(qc.tls.hs_server.key, hs_server_keys.key, 16);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(qc.tls.ap_server.key, ap_server_keys.key, 16);

    // --- Client -> server: Initial(ACK) + Handshake(ACK + CRYPTO client Finished) ---
    uint8_t ifr[64];
    size_t ifl = quic_build_ack(ifr, sizeof(ifr), 0, 0, 0);
    uint8_t idg[256];
    size_t idl = build_long(idg, sizeof(idg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID),
                            1, &init.client, ifr, ifl);

    uint8_t cfin[36] = {TLS_HS_FINISHED, 0x00, 0x00, 0x20};
    tls13_finished_mac(cks.client_hs_traffic, ch_sf, cfin + 4);
    uint8_t hfr[64];
    size_t hfl = quic_build_ack(hfr, sizeof(hfr), 0, 0, 0);
    hfl += quic_build_crypto(hfr + hfl, sizeof(hfr) - hfl, 0, cfin, sizeof(cfin));
    size_t hdl = build_long(idg + idl, sizeof(idg) - idl, QUIC_LP_HANDSHAKE, ODCID, sizeof(ODCID), CLIENT_SCID,
                            sizeof(CLIENT_SCID), 0, &hs_client_keys, hfr, hfl);
    TEST_ASSERT_TRUE(quic_conn_recv(&qc, idg, idl + hdl));

    TEST_ASSERT_TRUE(quic_conn_established(&qc));
    TEST_ASSERT_TRUE(g_hs_done);

    // Server's next datagram carries HANDSHAKE_DONE at 1-RTT.
    sl = quic_conn_send(&qc, sdg, sizeof(sdg));
    TEST_ASSERT_TRUE(sl > 0);
    bool saw_hs_done = false;
    size_t off = 0;
    while (off < sl)
    {
        if (quic_is_long_header(sdg[off]))
        {
            size_t w = 0;
            uint8_t tp2 = 0;
            open_long(sdg + off, sl - off, &hs_server_keys, plain, &w, &tp2);
            off += w;
        }
        else
        {
            size_t p2 = open_short(sdg + off, sl - off, sizeof(SERVER_SCID), &ap_server_keys, plain);
            TEST_ASSERT_NOT_EQUAL(SIZE_MAX, p2);
            if (has_frame(plain, p2, QUIC_FT_HANDSHAKE_DONE))
                saw_hs_done = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(saw_hs_done);

    // --- 1-RTT stream: client sends "GET" (fin) on stream 0; server echoes "OK" ---
    uint8_t sfr[64];
    size_t sfl = quic_build_stream(sfr, sizeof(sfr), 0, 0, (const uint8_t *)"GET", 3, true);
    uint8_t s1[256];
    size_t s1l = build_short(s1, sizeof(s1), SERVER_SCID, sizeof(SERVER_SCID), 0, &ap_client_keys, sfr, sfl);
    TEST_ASSERT_TRUE(quic_conn_recv(&qc, s1, s1l));
    TEST_ASSERT_EQUAL_UINT64(0, g_stream_id);
    TEST_ASSERT_EQUAL_UINT(3, g_stream_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("GET", g_stream_data, 3);
    TEST_ASSERT_TRUE(g_stream_fin);

    quic_conn_stream_send(&qc, 0, (const uint8_t *)"OK", 2, true);
    sl = quic_conn_send(&qc, sdg, sizeof(sdg));
    TEST_ASSERT_TRUE(sl > 0);
    // The 1-RTT packet is the last (short-header) packet in the datagram.
    off = 0;
    bool got_resp = false;
    while (off < sl)
    {
        if (quic_is_long_header(sdg[off]))
        {
            size_t w = 0;
            uint8_t tp2 = 0;
            open_long(sdg + off, sl - off, &hs_server_keys, plain, &w, &tp2);
            off += w;
            continue;
        }
        size_t p2 = open_short(sdg + off, sl - off, sizeof(SERVER_SCID), &ap_server_keys, plain);
        TEST_ASSERT_NOT_EQUAL(SIZE_MAX, p2);
        // Find the STREAM frame carrying "OK".
        size_t fo = 0;
        while (fo < p2)
        {
            if (plain[fo] == QUIC_FT_PADDING)
            {
                fo++;
                continue;
            }
            QuicFrame f;
            size_t n = quic_frame_parse(plain + fo, p2 - fo, &f);
            if (!n)
                break;
            fo += n;
            if (f.type >= QUIC_FT_STREAM && f.type <= QUIC_FT_STREAM + 7)
            {
                TEST_ASSERT_EQUAL_UINT(2, (size_t)f.stream.length);
                TEST_ASSERT_EQUAL_UINT8_ARRAY("OK", f.stream.data, 2);
                got_resp = true;
            }
        }
        break;
    }
    TEST_ASSERT_TRUE(got_resp);

    // --- Loss recovery: the client never ACKed the 1-RTT response, so a PTO retransmits it ---
    quic_conn_on_timeout(&qc, 5000);                         // arm (APP space has unacknowledged stream data)
    quic_conn_on_timeout(&qc, 5000 + DETWS_QUIC_PTO_MS + 1); // fire -> rewind the response stream
    sl = quic_conn_send(&qc, sdg, sizeof(sdg));
    TEST_ASSERT_TRUE(sl > 0);
    bool resent = false;
    off = 0;
    while (off < sl)
    {
        if (quic_is_long_header(sdg[off]))
        {
            size_t w = 0;
            uint8_t tp2 = 0;
            open_long(sdg + off, sl - off, &hs_server_keys, plain, &w, &tp2);
            off += w;
            continue;
        }
        size_t p2 = open_short(sdg + off, sl - off, sizeof(SERVER_SCID), &ap_server_keys, plain);
        TEST_ASSERT_NOT_EQUAL(SIZE_MAX, p2);
        size_t fo = 0;
        while (fo < p2)
        {
            if (plain[fo] == QUIC_FT_PADDING)
            {
                fo++;
                continue;
            }
            QuicFrame f;
            size_t n = quic_frame_parse(plain + fo, p2 - fo, &f);
            if (!n)
                break;
            fo += n;
            if (f.type >= QUIC_FT_STREAM && f.type <= QUIC_FT_STREAM + 7 && f.stream.id == 0 && f.stream.length == 2 &&
                memcmp(f.stream.data, "OK", 2) == 0)
                resent = true;
        }
        break;
    }
    TEST_ASSERT_TRUE(resent); // the response was retransmitted
}

// A lost server flight is retransmitted on the Probe Timeout, and stops once acknowledged (RFC 9002).
void test_pto_retransmits_flight()
{
    fill();
    QuicTlsConfig cfg;
    make_cfg(&cfg);
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    quic_conn_init(&qc, &cfg, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), SERVER_SCID, sizeof(SERVER_SCID),
                   &cb);

    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);

    // Client Initial(ClientHello), padded for the amplification budget.
    QuicTransportParams ctp;
    quic_tp_defaults(&ctp);
    uint8_t ctp_enc[128];
    size_t ctp_len = quic_tp_encode(&ctp, ctp_enc, sizeof(ctp_enc));
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_PRIV);
    uint8_t ch[512];
    size_t ch_len = build_client_hello(ch, client_pub, ctp_enc, ctp_len);
    uint8_t frames[1200];
    size_t fl = quic_build_crypto(frames, sizeof(frames), 0, ch, ch_len);
    memset(frames + fl, 0, 1100 - fl);
    fl = 1100;
    uint8_t dg[1500];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, frames, fl);
    TEST_ASSERT_TRUE(quic_conn_recv(&qc, dg, dl));

    // Server's first flight (pretend it is lost: capture it and never ACK it).
    uint8_t sdg[1500];
    TEST_ASSERT_TRUE(quic_conn_send(&qc, sdg, sizeof(sdg)) > 0);

    // With the flight already sent and no PTO fired, there is nothing to send.
    TEST_ASSERT_EQUAL_UINT(0, quic_conn_send(&qc, sdg, sizeof(sdg)));
    quic_conn_on_timeout(&qc, 1000);                                  // arm
    TEST_ASSERT_EQUAL_UINT(0, quic_conn_send(&qc, sdg, sizeof(sdg))); // still nothing (not fired)

    // Advance past the PTO: the flight is marked for retransmission and re-sent.
    quic_conn_on_timeout(&qc, 1000 + DETWS_QUIC_PTO_MS + 1);
    uint8_t sdg2[1500];
    size_t sl2 = quic_conn_send(&qc, sdg2, sizeof(sdg2));
    TEST_ASSERT_TRUE(sl2 > 0);
    uint8_t plain[2048], sh[512];
    size_t wire = 0;
    uint8_t type = 0;
    size_t pt = open_long(sdg2, sl2, &init.server, plain, &wire, &type);
    TEST_ASSERT_EQUAL_UINT8(QUIC_LP_INITIAL, type);
    size_t sh_len = extract_crypto(plain, pt, sh);
    TEST_ASSERT_TRUE(sh_len > 0);
    TEST_ASSERT_EQUAL_UINT8(TLS_HS_SERVER_HELLO, sh[0]); // the ServerHello was retransmitted

    // Once the flight is acknowledged (Initial discarded as the client moves on; Handshake ACKed up
    // to the last packet we sent), the PTO disarms and does not retransmit again.
    qc.space[QUIC_ENC_INITIAL].discarded = true;
    qc.space[QUIC_ENC_HANDSHAKE].largest_acked = qc.space[QUIC_ENC_HANDSHAKE].last_ae_pn;
    quic_conn_on_timeout(&qc, 1000 + 10 * DETWS_QUIC_PTO_MS);
    TEST_ASSERT_FALSE(qc.pto_armed);
    TEST_ASSERT_EQUAL_UINT(0, quic_conn_send(&qc, sdg2, sizeof(sdg2)));
}

// Init a server conn and feed it a padded client Initial (ClientHello); the server flight is left
// pending. Returns the client's Initial secrets and the ClientHello bytes (for deriving later keys).
static void feed_client_initial(QuicConn *qc, QuicConnCallbacks *cb, QuicInitialSecrets *init, uint8_t *ch,
                                size_t *ch_len)
{
    QuicTlsConfig cfg;
    make_cfg(&cfg);
    quic_conn_init(qc, &cfg, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), SERVER_SCID, sizeof(SERVER_SCID),
                   cb);
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), init);
    QuicTransportParams ctp;
    quic_tp_defaults(&ctp);
    uint8_t ctp_enc[128];
    size_t ctp_len = quic_tp_encode(&ctp, ctp_enc, sizeof(ctp_enc));
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_PRIV);
    *ch_len = build_client_hello(ch, client_pub, ctp_enc, ctp_len);
    uint8_t frames[1200];
    size_t fl = quic_build_crypto(frames, sizeof(frames), 0, ch, *ch_len);
    memset(frames + fl, 0, 1100 - fl);
    fl = 1100;
    uint8_t dg[1500];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init->client, frames, fl);
    TEST_ASSERT_TRUE(quic_conn_recv(qc, dg, dl));
}

// The public initiate-close API queues a transport CONNECTION_CLOSE the next send emits, after which
// the connection is closed and sends nothing more.
void test_connection_close_api()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    QuicInitialSecrets init;
    uint8_t ch[512];
    size_t ch_len = 0;
    feed_client_initial(&qc, &cb, &init, ch, &ch_len);

    quic_conn_close(&qc, QUIC_ERR_NO_ERROR);
    uint8_t cdg[512];
    TEST_ASSERT_TRUE(quic_conn_send(&qc, cdg, sizeof(cdg)) > 0);
    TEST_ASSERT_TRUE(quic_conn_is_closed(&qc));
    TEST_ASSERT_EQUAL_UINT(0, quic_conn_send(&qc, cdg, sizeof(cdg))); // nothing more after the close
}

// A fatal transport error (an undecodable frame) makes the server report a CONNECTION_CLOSE with the
// right error code instead of leaving the peer to time out (RFC 9000 sec 10.2 / 19.19).
void test_connection_close_on_malformed_frame()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    QuicInitialSecrets init;
    uint8_t ch[512];
    size_t ch_len = 0;
    feed_client_initial(&qc, &cb, &init, ch, &ch_len);

    // Capture the server flight and derive the client-side Handshake keys (to send at + read that level).
    uint8_t sdg[1500];
    size_t sl = quic_conn_send(&qc, sdg, sizeof(sdg));
    TEST_ASSERT_TRUE(sl > 0);
    uint8_t plain[2048], sh[512];
    size_t wire = 0;
    uint8_t type = 0;
    size_t pt = open_long(sdg, sl, &init.server, plain, &wire, &type);
    size_t sh_len = extract_crypto(plain, pt, sh);
    uint8_t server_pub[32], ecdhe[32];
    ssh_x25519_base(server_pub, SERVER_PRIV);
    ssh_x25519(ecdhe, CLIENT_PRIV, server_pub);
    SshSha256Ctx tctx;
    uint8_t ch_sh[32];
    ssh_sha256_init(&tctx);
    ssh_sha256_update(&tctx, ch, ch_len);
    ssh_sha256_update(&tctx, sh, sh_len);
    ssh_sha256_final(&tctx, ch_sh);
    Tls13KeySchedule cks;
    tls13_ks_early(&cks);
    tls13_ks_handshake(&cks, ecdhe, ch_sh);
    QuicPacketKeys hs_server_keys, hs_client_keys;
    quic_keys_from_secret(cks.server_hs_traffic, &hs_server_keys);
    quic_keys_from_secret(cks.client_hs_traffic, &hs_client_keys);

    // Client -> server: a Handshake packet whose only frame is a malformed CRYPTO (its declared length
    // dwarfs the packet), which the server cannot decode.
    uint8_t bad[4] = {QUIC_FT_CRYPTO, 0x00, 0x7f, 0xff}; // CRYPTO off=0 len=16383, no data present
    uint8_t bdg[256];
    size_t bl = build_long(bdg, sizeof(bdg), QUIC_LP_HANDSHAKE, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID),
                           0, &hs_client_keys, bad, sizeof(bad));
    quic_conn_recv(&qc, bdg, bl); // fatal -> queues the close

    // The next server datagram is a CONNECTION_CLOSE (FRAME_ENCODING) at the Handshake level.
    uint8_t cdg[512];
    size_t cl = quic_conn_send(&qc, cdg, sizeof(cdg));
    TEST_ASSERT_TRUE(cl > 0);
    size_t cw = 0;
    uint8_t ctype = 0;
    size_t cpt = open_long(cdg, cl, &hs_server_keys, plain, &cw, &ctype);
    TEST_ASSERT_NOT_EQUAL(SIZE_MAX, cpt);
    bool saw = false;
    size_t fo = 0;
    while (fo < cpt)
    {
        if (plain[fo] == QUIC_FT_PADDING)
        {
            fo++;
            continue;
        }
        QuicFrame f;
        size_t n = quic_frame_parse(plain + fo, cpt - fo, &f);
        if (!n)
            break;
        fo += n;
        if (f.type == QUIC_FT_CONNECTION_CLOSE)
        {
            saw = true;
            TEST_ASSERT_EQUAL_UINT64(QUIC_ERR_FRAME_ENCODING, f.close.error_code);
        }
    }
    TEST_ASSERT_TRUE(saw);
    TEST_ASSERT_TRUE(quic_conn_is_closed(&qc));
    TEST_ASSERT_EQUAL_UINT(0, quic_conn_send(&qc, cdg, sizeof(cdg))); // nothing more after the close
}

// Init a bare server conn (Initial keys ready, handshake not started). cb is owned
// by the caller (quic_conn_init keeps the pointer).
static void init_conn(QuicConn *qc, QuicConnCallbacks *cb)
{
    QuicTlsConfig cfg;
    make_cfg(&cfg);
    quic_conn_init(qc, &cfg, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), SERVER_SCID, sizeof(SERVER_SCID),
                   cb);
}

// A received CONNECTION_CLOSE closes/drains the server connection.
void test_quic_recv_connection_close()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);

    uint8_t fr[32];
    size_t fl = quic_build_connection_close(fr, sizeof(fr), QUIC_ERR_NO_ERROR, 0, nullptr, 0);
    uint8_t dg[256];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, fr, fl);
    quic_conn_recv(&qc, dg, dl);
    TEST_ASSERT_TRUE(quic_conn_is_closed(&qc));
    // A datagram arriving after the connection is closed is rejected outright.
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, dl));
}

// PING and MAX_DATA are accepted with no per-frame state kept.
void test_quic_recv_ping_and_max_data()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);

    uint8_t fr[16];
    size_t fl = quic_build_ping(fr, sizeof(fr));
    fl += quic_build_max_data(fr + fl, sizeof(fr) - fl, 1000000);
    uint8_t dg[256];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, fr, fl);
    TEST_ASSERT_TRUE(quic_conn_recv(&qc, dg, dl));
    TEST_ASSERT_FALSE(quic_conn_is_closed(&qc));
}

// A long header with an unknown version is dropped (Version Negotiation is a
// client concern).
void test_quic_recv_bad_version()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);
    uint8_t fr[8] = {QUIC_FT_PING};
    uint8_t dg[256];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, fr, 1);
    dg[1] = dg[2] = dg[3] = dg[4] = 0xAA; // clobber the version (not header-protected)
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, dl));
}

// A long header of an unsupported type (0-RTT) is dropped before decryption.
void test_quic_recv_unsupported_long_type()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);
    uint8_t fr[8] = {QUIC_FT_PING};
    uint8_t dg[256];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_0RTT, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, fr, 1);
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, dl));
}

// A short-header (1-RTT) packet arriving before the app keys exist is dropped.
void test_quic_recv_short_before_app_keys()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);
    uint8_t fr[8] = {QUIC_FT_PING};
    uint8_t dg[256];
    size_t dl = build_short(dg, sizeof(dg), SERVER_SCID, sizeof(SERVER_SCID), 0, &init.client, fr, 1);
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, dl)); // open_keys(APP) is null -> dropped
}

// A short-header datagram too small to hold the DCID + a packet number is dropped.
void test_quic_recv_short_too_short()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    uint8_t dg[1] = {0x40}; // short header, no room for DCID/PN
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, sizeof(dg)));
}

// A packet whose AEAD tag fails to verify is consumed-and-dropped, not fatal.
void test_quic_recv_unprotect_failure()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);
    uint8_t fr[8] = {QUIC_FT_PING};
    uint8_t dg[256];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, fr, 1);
    dg[dl - 1] ^= 0xFF; // corrupt the auth tag
    quic_conn_recv(&qc, dg, dl);
    TEST_ASSERT_FALSE(quic_conn_is_closed(&qc)); // dropped, no effect
    TEST_ASSERT_FALSE(quic_conn_established(&qc));
}

// A truncated long header (too short to parse) is dropped.
void test_quic_recv_truncated_long_header()
{
    fill();
    QuicConn qc;
    QuicConnCallbacks cb = {on_stream_data, on_hs_done, nullptr};
    init_conn(&qc, &cb);
    uint8_t dg[4] = {0xC0, 0x00, 0x00, 0x00}; // long header flag, then a truncated version
    TEST_ASSERT_FALSE(quic_conn_recv(&qc, dg, sizeof(dg)));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_full_handshake_and_stream);
    RUN_TEST(test_pto_retransmits_flight);
    RUN_TEST(test_connection_close_api);
    RUN_TEST(test_connection_close_on_malformed_frame);
    RUN_TEST(test_quic_recv_connection_close);
    RUN_TEST(test_quic_recv_ping_and_max_data);
    RUN_TEST(test_quic_recv_bad_version);
    RUN_TEST(test_quic_recv_unsupported_long_type);
    RUN_TEST(test_quic_recv_short_before_app_keys);
    RUN_TEST(test_quic_recv_short_too_short);
    RUN_TEST(test_quic_recv_unprotect_failure);
    RUN_TEST(test_quic_recv_truncated_long_header);
    return UNITY_END();
}
