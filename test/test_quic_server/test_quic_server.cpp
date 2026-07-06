// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// HTTP/3 server-glue test: the same end-to-end flow as test_h3_e2e (a QUIC client completes the
// TLS 1.3 handshake, sends an HTTP/3 GET, and verifies the 200 + body), but driven THROUGH the
// quic_server module - quic_server_begin / quic_server_ingest / quic_server_poll / quic_server_respond
// with the outbound datagrams captured by the host output sink. It exercises what quic_server adds
// over the raw engines: opening a connection from a client Initial, routing later datagrams by
// Destination Connection ID (odcid for the long-header handshake packets, our chosen SCID for the
// 1-RTT short header), the ingest ring, and the request/response callback seam.

#include "network_drivers/presentation/http3/h3_conn.h"
#include "network_drivers/presentation/http3/h3_frame.h"
#include "network_drivers/presentation/http3/qpack.h"
#include "network_drivers/presentation/http3/quic_crypto.h"
#include "network_drivers/presentation/http3/quic_frame.h"
#include "network_drivers/presentation/http3/quic_packet.h"
#include "network_drivers/presentation/http3/quic_server.h"
#include "network_drivers/presentation/http3/quic_tp.h"
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
static const uint8_t SERVER_SCID[8] = {0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58};

// The server's RNG is driven so the handshake is reproducible: the ephemeral X25519 key must be
// SERVER_PRIV (the client computes the shared secret against x25519_base(SERVER_PRIV)); the SCID is a
// known value so the client can address the 1-RTT short header to it.
static int g_rng_call;
static void test_rng(uint8_t *out, size_t len)
{
    if (g_rng_call == 0)
        memcpy(out, SERVER_PRIV, len);
    else if (g_rng_call == 1)
        memcpy(out, SERVER_RANDOM, len);
    else
        memcpy(out, SERVER_SCID, len); // DETWS_QUIC_SCID_LEN == sizeof(SERVER_SCID)
    g_rng_call++;
}

// Captured outbound datagrams (server -> client) from the current poll.
static uint8_t g_out[16][1500];
static size_t g_out_len[16];
static int g_out_n;
static void out_sink(void *, const uint8_t *dg, size_t len, const char *, uint16_t)
{
    if (g_out_n < 16 && len <= sizeof(g_out[0]))
    {
        memcpy(g_out[g_out_n], dg, len);
        g_out_len[g_out_n] = len;
        g_out_n++;
    }
}

// The application handler: capture the request and answer 200 through the server API.
static char g_method[16], g_path[64];
static void app_request(void *, uint32_t conn_id, uint64_t sid, const char *method, const char *path, const char *,
                        const uint8_t *, size_t)
{
    strncpy(g_method, method, sizeof(g_method) - 1);
    strncpy(g_path, path, sizeof(g_path) - 1);
    quic_server_respond(conn_id, sid, 200, "text/plain", (const uint8_t *)"hello h3", 8);
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
    g_method[0] = g_path[0] = '\0';
    g_rng_call = 0;
    g_out_n = 0;
}

// --- packet helpers (client side), identical construction to the engine's -------------------
static void wr_pn(uint8_t *o, uint64_t pn, uint8_t pn_len)
{
    for (uint8_t i = 0; i < pn_len; i++)
        o[i] = (uint8_t)(pn >> (8 * (pn_len - 1 - i)));
}
static size_t build_long(uint8_t *out, size_t cap, uint8_t type, const uint8_t *dcid, uint8_t dcl, const uint8_t *scid,
                         uint8_t scl, uint64_t pn, const QuicPacketKeys *keys, const uint8_t *frames, size_t frame_len)
{
    uint8_t pn_len = quic_pn_length(pn, -1);
    size_t p = quic_build_long_header(out, cap, type, QUIC_VERSION_1, dcid, dcl, scid, scl, pn_len);
    if (type == QUIC_LP_INITIAL)
        p += quic_varint_encode(out + p, cap - p, 0);
    p += quic_varint_encode(out + p, cap - p, (uint64_t)pn_len + frame_len + 16);
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
static size_t open_long(const uint8_t *dg, size_t len, const QuicPacketKeys *keys, uint8_t *plain, size_t *wire,
                        uint8_t *type)
{
    QuicLongHeader h;
    TEST_ASSERT_TRUE(quic_parse_long_header(dg, len, &h));
    *type = h.type;
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
    *wire = off + (size_t)length;
    static uint8_t work[2048];
    memcpy(work, dg, *wire);
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
static size_t build_client_hello(uint8_t *out, const uint8_t client_pub[32], const uint8_t *tp, size_t tp_len)
{
    size_t p = 0;
    out[p++] = TLS_HS_CLIENT_HELLO;
    size_t hs = p;
    p += 3;
    out[p++] = 0x03;
    out[p++] = 0x03;
    for (int i = 0; i < 32; i++)
        out[p++] = (uint8_t)i;
    out[p++] = 0x00;
    out[p++] = 0x00;
    out[p++] = 0x02;
    out[p++] = 0x13;
    out[p++] = 0x01;
    out[p++] = 0x01;
    out[p++] = 0x00;
    size_t ext = p;
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
    uint16_t el = (uint16_t)(p - ext - 2);
    out[ext] = (uint8_t)(el >> 8);
    out[ext + 1] = (uint8_t)el;
    uint32_t hl = (uint32_t)(p - hs - 3);
    out[hs] = (uint8_t)(hl >> 16);
    out[hs + 1] = (uint8_t)(hl >> 8);
    out[hs + 2] = (uint8_t)hl;
    return p;
}

// Scan every captured datagram for a 1-RTT STREAM frame on stream 0 carrying HTTP/3
// HEADERS(:status 200) + DATA("hello h3"); returns true if found.
static bool response_ok(const QuicPacketKeys *ap_s)
{
    uint8_t plain[2048];
    for (int d = 0; d < g_out_n; d++)
    {
        const uint8_t *dg = g_out[d];
        size_t len = g_out_len[d];
        if (quic_is_long_header(dg[0]))
            continue; // handshake / coalesced long-header packets: not the 1-RTT response
        size_t p2 = open_short(dg, len, sizeof(CLIENT_SCID), ap_s, plain);
        if (p2 == SIZE_MAX)
            continue;
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
            if (!(f.type >= QUIC_FT_STREAM && f.type <= QUIC_FT_STREAM + 7 && f.stream.id == 0))
                continue;
            const uint8_t *sp = f.stream.data;
            size_t so = 0, sn = (size_t)f.stream.length;
            char status[8] = {0};
            bool data_ok = false;
            while (so < sn)
            {
                H3Frame hf;
                if (!h3_frame_parse(sp + so, sn - so, &hf))
                    break;
                const uint8_t *hp = sp + so + hf.header_len;
                if (hf.type == H3_HEADERS)
                {
                    char sc[128];
                    struct E
                    {
                        char *s;
                    } e = {status};
                    qpack_decode(
                        hp, (size_t)hf.length, sc, sizeof(sc),
                        [](void *c, const char *nm, size_t nl, const char *v, size_t vl) -> bool {
                            if (nl == 7 && memcmp(nm, ":status", 7) == 0)
                            {
                                memcpy(((E *)c)->s, v, vl);
                                ((E *)c)->s[vl] = 0;
                            }
                            return true;
                        },
                        &e);
                }
                else if (hf.type == H3_DATA)
                {
                    if (hf.length == 8 && memcmp(hp, "hello h3", 8) == 0)
                        data_ok = true;
                }
                so += hf.header_len + (size_t)hf.length;
            }
            if (strcmp(status, "200") == 0 && data_ok)
                return true;
        }
    }
    return false;
}

void test_quic_server_http3_get()
{
    fill();

    // Start the server: it owns the QuicConn + H3Conn pool and routes by connection ID.
    QuicServerConfig scfg;
    memset(&scfg, 0, sizeof(scfg));
    scfg.cert_der = CERT;
    scfg.cert_len = sizeof(CERT);
    memcpy(scfg.ed25519_seed, SERVER_SEED, 32);
    scfg.rng = test_rng;
    quic_server_set_out_sink(out_sink, nullptr);
    TEST_ASSERT_TRUE(quic_server_begin(443, &scfg, app_request, nullptr));

    QuicInitialSecrets init;
    quic_derive_initial_secrets(ODCID, sizeof(ODCID), &init);

    // Client Initial(ClientHello), padded to the 1200-byte minimum.
    QuicTransportParams ctp;
    quic_tp_defaults(&ctp);
    ctp.initial_max_data = 524288;
    ctp.initial_max_sd_bidi_local = 131072;
    uint8_t ctpe[128];
    size_t ctpl = quic_tp_encode(&ctp, ctpe, sizeof(ctpe));
    uint8_t client_pub[32];
    ssh_x25519_base(client_pub, CLIENT_PRIV);
    uint8_t ch[512];
    size_t chl = build_client_hello(ch, client_pub, ctpe, ctpl);
    uint8_t frames[1200];
    size_t fl = quic_build_crypto(frames, sizeof(frames), 0, ch, chl);
    memset(frames + fl, 0, 1100 - fl);
    fl = 1100;
    uint8_t dg[1500];
    size_t dl = build_long(dg, sizeof(dg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID), 0,
                           &init.client, frames, fl);

    // Deliver it: quic_server opens the connection and produces the server flight.
    g_out_n = 0;
    TEST_ASSERT_TRUE(quic_server_ingest(dg, dl, "192.0.2.10", 40000));
    quic_server_poll();
    TEST_ASSERT_EQUAL_UINT8(1, quic_server_active_conns());
    TEST_ASSERT_GREATER_THAN(0, g_out_n); // the coalesced Initial(SH) + Handshake(EE..Finished) flight

    // Derive the client-side handshake + 1-RTT keys from the captured flight.
    uint8_t plain[2048], sh[512], hsf[1024];
    size_t wire = 0;
    uint8_t ty = 0;
    size_t pt = open_long(g_out[0], g_out_len[0], &init.server, plain, &wire, &ty);
    size_t shl = extract_crypto(plain, pt, sh);
    uint8_t server_pub[32], ecdhe[32];
    ssh_x25519_base(server_pub, SERVER_PRIV);
    ssh_x25519(ecdhe, CLIENT_PRIV, server_pub);
    SshSha256Ctx t;
    uint8_t chsh[32], chsf[32];
    ssh_sha256_init(&t);
    ssh_sha256_update(&t, ch, chl);
    ssh_sha256_update(&t, sh, shl);
    {
        SshSha256Ctx tmp = t;
        ssh_sha256_final(&tmp, chsh);
    }
    Tls13KeySchedule cks;
    tls13_ks_early(&cks);
    tls13_ks_handshake(&cks, ecdhe, chsh);
    QuicPacketKeys hs_s, hs_c, ap_s, ap_c;
    quic_keys_from_secret(cks.server_hs_traffic, &hs_s);
    quic_keys_from_secret(cks.client_hs_traffic, &hs_c);
    size_t hw = 0;
    uint8_t hty = 0;
    size_t hpt = open_long(g_out[0] + wire, g_out_len[0] - wire, &hs_s, plain, &hw, &hty);
    size_t hsfl = extract_crypto(plain, hpt, hsf);
    ssh_sha256_update(&t, hsf, hsfl);
    ssh_sha256_final(&t, chsf);
    tls13_ks_master(&cks, chsf);
    quic_keys_from_secret(cks.server_ap_traffic, &ap_s);
    quic_keys_from_secret(cks.client_ap_traffic, &ap_c);

    // Client Initial(ACK) + Handshake(ACK + Finished) -> server completes the handshake.
    uint8_t ifr[64];
    size_t ifl = quic_build_ack(ifr, sizeof(ifr), 0, 0, 0);
    uint8_t idg[256];
    size_t idl = build_long(idg, sizeof(idg), QUIC_LP_INITIAL, ODCID, sizeof(ODCID), CLIENT_SCID, sizeof(CLIENT_SCID),
                            1, &init.client, ifr, ifl);
    uint8_t cfin[36] = {TLS_HS_FINISHED, 0x00, 0x00, 0x20};
    tls13_finished_mac(cks.client_hs_traffic, chsf, cfin + 4);
    uint8_t hfr[64];
    size_t hfl = quic_build_ack(hfr, sizeof(hfr), 0, 0, 0);
    hfl += quic_build_crypto(hfr + hfl, sizeof(hfr) - hfl, 0, cfin, sizeof(cfin));
    size_t hdl = build_long(idg + idl, sizeof(idg) - idl, QUIC_LP_HANDSHAKE, ODCID, sizeof(ODCID), CLIENT_SCID,
                            sizeof(CLIENT_SCID), 0, &hs_c, hfr, hfl);
    g_out_n = 0;
    TEST_ASSERT_TRUE(quic_server_ingest(idg, idl + hdl, "192.0.2.10", 40000));
    quic_server_poll(); // drains HANDSHAKE_DONE + the server's control/QPACK streams (ignored)

    // Client HTTP/3 GET on request stream 0 (1-RTT). The short-header DCID is the server's SCID.
    uint8_t block[128];
    size_t bp = qpack_encode_prefix(block, sizeof(block));
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "GET", 3);
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/hello", 6);
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":authority", 10, "h3.test", 7);
    uint8_t h3req[256];
    size_t h3l = h3_build_headers(h3req, sizeof(h3req), block, bp);
    uint8_t sfr[300];
    size_t sfrl = quic_build_stream(sfr, sizeof(sfr), 0, 0, h3req, h3l, true);
    uint8_t s1[512];
    size_t s1l = build_short(s1, sizeof(s1), SERVER_SCID, sizeof(SERVER_SCID), 0, &ap_c, sfr, sfrl);

    g_out_n = 0;
    TEST_ASSERT_TRUE(quic_server_ingest(s1, s1l, "192.0.2.10", 40000));
    quic_server_poll(); // dispatches the request to app_request, which responds; flushed here

    // The request reached the application through the server callback...
    TEST_ASSERT_EQUAL_STRING("GET", g_method);
    TEST_ASSERT_EQUAL_STRING("/hello", g_path);
    // ...and the 200 + body came back on the 1-RTT request stream.
    TEST_ASSERT_TRUE(response_ok(&ap_s));

    quic_server_stop();
    TEST_ASSERT_EQUAL_UINT8(0, quic_server_active_conns());
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_quic_server_http3_get);
    return UNITY_END();
}
