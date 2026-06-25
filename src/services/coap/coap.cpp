// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file coap.cpp
 * @brief CoAP server (RFC 7252): resource table, message codec, and the UDP binding.
 */

#include "services/coap/coap.h"

#if DETWS_ENABLE_COAP

#include "network_drivers/transport/udp_transport.h"
#include <string.h>

// CoAP option numbers we understand (RFC 7252 §5.10). Others are skipped.
enum
{
    COAP_OPT_URI_PATH = 11,
    COAP_OPT_CONTENT_FORMAT = 12,
    COAP_OPT_URI_QUERY = 15,
};

static const uint8_t COAP_PAYLOAD_MARKER = 0xFF;
static const size_t COAP_MAX_TOKEN = 8;

// ---------------------------------------------------------------------------
// Resource table + reconstruction scratch (all in BSS - no heap)
// ---------------------------------------------------------------------------

struct CoapResource
{
    const char *path;
    uint8_t methods;
    CoapHandler handler;
};

static CoapResource g_res[DETWS_COAP_MAX_RESOURCES];
static size_t g_res_count = 0;

static char g_path[DETWS_COAP_MAX_PATH];     // reconstructed Uri-Path
static char g_query[DETWS_COAP_MAX_QUERY];   // reconstructed Uri-Query
static uint8_t g_pl[DETWS_COAP_MAX_PAYLOAD]; // handler response-body scratch

void coap_server_init()
{
    g_res_count = 0;
    memset(g_res, 0, sizeof(g_res));
}

bool coap_server_add_resource(const char *path, uint8_t methods, CoapHandler handler)
{
    if (g_res_count >= DETWS_COAP_MAX_RESOURCES || !path || !handler)
        return false;
    g_res[g_res_count].path = path;
    g_res[g_res_count].methods = methods;
    g_res[g_res_count].handler = handler;
    g_res_count++;
    return true;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Minimal big-endian unsigned decode of an option value (used for Content-Format).
static uint32_t opt_uint(const uint8_t *v, size_t n)
{
    uint32_t r = 0;
    for (size_t i = 0; i < n; i++)
        r = (r << 8) | v[i];
    return r;
}

// Append one path/query segment to a reconstruction buffer, prefixed by @p sep
// (pass '\0' for no separator, e.g. the first Uri-Query segment). Returns false
// (and leaves the buffer NUL-terminated) if it would overflow.
static bool seg_append(char *buf, size_t cap, size_t *len, char sep, const uint8_t *seg, size_t seglen)
{
    size_t need = (sep ? 1u : 0u) + seglen + 1u; // optional separator + segment + NUL
    if (*len + need > cap)
        return false;
    if (sep)
        buf[(*len)++] = sep;
    memcpy(buf + *len, seg, seglen);
    *len += seglen;
    buf[*len] = '\0';
    return true;
}

static const CoapResource *find_resource(const char *path)
{
    for (size_t i = 0; i < g_res_count; i++)
        if (strcmp(g_res[i].path, path) == 0)
            return &g_res[i];
    return nullptr;
}

// Emit a bare message: 4-byte header + token, no options/payload. Used for RST and
// for error responses (4.04 / 4.05 / 5.01) that carry no diagnostic payload.
static size_t emit_header(uint8_t *out, size_t cap, uint8_t type, uint8_t code, uint16_t mid, const uint8_t *token,
                          uint8_t tkl)
{
    if (cap < (size_t)(4 + tkl))
        return 0;
    out[0] = (uint8_t)((1 << 6) | (type << 4) | tkl); // Ver=1
    out[1] = code;
    out[2] = (uint8_t)(mid >> 8);
    out[3] = (uint8_t)(mid & 0xFF);
    if (tkl)
        memcpy(out + 4, token, tkl);
    return (size_t)(4 + tkl);
}

// ---------------------------------------------------------------------------
// Core processing (no sockets, no heap)
// ---------------------------------------------------------------------------

size_t coap_server_process(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap)
{
    if (req_len < 4)
        return 0; // too short for a header

    uint8_t ver = (req[0] >> 6) & 0x03;
    uint8_t type = (req[0] >> 4) & 0x03;
    uint8_t tkl = req[0] & 0x0F;
    uint8_t code = req[1];
    uint16_t mid = (uint16_t)((req[2] << 8) | req[3]);

    // Reply type for a request: piggybacked ACK for CON, NON for NON. ACK/RST we
    // receive are not requests - ignore them.
    if (type != COAP_TYPE_CON && type != COAP_TYPE_NON)
        return 0;
    uint8_t rsp_type = (type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON;

    // A malformed message: bad version or reserved TKL (9..15). RFC 7252 §4.2:
    // reject a CON with an RST; stay silent for a NON.
    if (ver != 1 || tkl > COAP_MAX_TOKEN)
        return (type == COAP_TYPE_CON) ? emit_header(resp, resp_cap, COAP_TYPE_RST, 0, mid, nullptr, 0) : 0;

    const uint8_t *p = req + 4;
    const uint8_t *end = req + req_len;
    if (p + tkl > end)
        return (type == COAP_TYPE_CON) ? emit_header(resp, resp_cap, COAP_TYPE_RST, 0, mid, nullptr, 0) : 0;
    const uint8_t *token = p;
    p += tkl;

    // An empty message (Code 0.00): CON is a ping -> RST; anything else -> ignore.
    if (code == 0)
        return (type == COAP_TYPE_CON) ? emit_header(resp, resp_cap, COAP_TYPE_RST, 0, mid, nullptr, 0) : 0;

    // Decode options, reconstructing Uri-Path / Uri-Query and reading Content-Format.
    size_t path_len = 0, query_len = 0;
    g_path[0] = '\0';
    g_query[0] = '\0';
    uint16_t req_cf = COAP_CF_NONE;
    const uint8_t *payload = nullptr;
    size_t payload_len = 0;
    uint32_t opt_num = 0;
    bool bad = false;

    while (p < end)
    {
        uint8_t b = *p++;
        if (b == COAP_PAYLOAD_MARKER)
        {
            payload = p;
            payload_len = (size_t)(end - p);
            break;
        }
        uint32_t delta = b >> 4;
        uint32_t olen = b & 0x0F;
        if (delta == 15 || olen == 15) // 15 is reserved outside the payload marker
        {
            bad = true;
            break;
        }
        // Extended delta / length (13 -> +1 byte; 14 -> +2 bytes).
        if (delta == 13)
        {
            if (p >= end)
            {
                bad = true;
                break;
            }
            delta = (uint32_t)(*p++) + 13;
        }
        else if (delta == 14)
        {
            if (p + 2 > end)
            {
                bad = true;
                break;
            }
            delta = (uint32_t)((p[0] << 8) | p[1]) + 269;
            p += 2;
        }
        if (olen == 13)
        {
            if (p >= end)
            {
                bad = true;
                break;
            }
            olen = (uint32_t)(*p++) + 13;
        }
        else if (olen == 14)
        {
            if (p + 2 > end)
            {
                bad = true;
                break;
            }
            olen = (uint32_t)((p[0] << 8) | p[1]) + 269;
            p += 2;
        }
        if (p + olen > end)
        {
            bad = true;
            break;
        }
        opt_num += delta;
        const uint8_t *val = p;
        p += olen;

        switch (opt_num)
        {
        case COAP_OPT_URI_PATH:
            if (!seg_append(g_path, sizeof(g_path), &path_len, '/', val, olen))
                bad = true;
            break;
        case COAP_OPT_URI_QUERY:
            if (!seg_append(g_query, sizeof(g_query), &query_len, query_len ? '&' : '\0', val, olen))
                bad = true;
            break;
        case COAP_OPT_CONTENT_FORMAT:
            req_cf = (uint16_t)opt_uint(val, olen);
            break;
        default:
            break; // skip options we don't act on
        }
        if (bad)
            break;
    }

    if (bad)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_REQUEST, mid, token, tkl);

    if (path_len == 0)
    {
        g_path[0] = '/';
        g_path[1] = '\0';
    }

    // Only class-0 GET/POST/PUT/DELETE are supported request methods.
    if ((code >> 5) != 0 || code < COAP_GET || code > COAP_DELETE)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_NOT_IMPLEMENTED, mid, token, tkl);

    const CoapResource *r = find_resource(g_path);
    if (!r)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_NOT_FOUND, mid, token, tkl);
    if (!(r->methods & (1u << code)))
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_METHOD_NOT_ALLOWED, mid, token, tkl);

    CoapRequest creq;
    creq.method = code;
    creq.path = g_path;
    creq.query = g_query;
    creq.payload = payload;
    creq.payload_len = payload_len;
    creq.content_format = req_cf;

    CoapResponse cresp;
    cresp.code = COAP_RSP_CONTENT;
    cresp.content_format = COAP_CF_NONE;
    cresp.payload = g_pl;
    cresp.payload_cap = sizeof(g_pl);
    cresp.payload_len = 0;

    r->handler(&creq, &cresp);
    if (cresp.payload_len > sizeof(g_pl))
        cresp.payload_len = sizeof(g_pl); // defensive clamp

    // Build the response: header + token + Content-Format option + payload.
    size_t n = emit_header(resp, resp_cap, rsp_type, cresp.code, mid, token, tkl);
    if (n == 0)
        return 0;

    if (cresp.content_format != COAP_CF_NONE)
    {
        // Content-Format is option 12; first (and only) option, so delta == 12.
        uint8_t cfv[2];
        uint8_t cfn = 0;
        uint16_t cf = cresp.content_format;
        if (cf > 0xFF)
        {
            cfv[cfn++] = (uint8_t)(cf >> 8);
            cfv[cfn++] = (uint8_t)(cf & 0xFF);
        }
        else if (cf > 0)
        {
            cfv[cfn++] = (uint8_t)cf;
        }
        if (n + 1 + cfn > resp_cap)
            return n; // no room for the option; ship what we have
        resp[n++] = (uint8_t)((COAP_OPT_CONTENT_FORMAT << 4) | cfn);
        for (uint8_t i = 0; i < cfn; i++)
            resp[n++] = cfv[i];
    }

    if (cresp.payload_len)
    {
        if (n + 1 + cresp.payload_len > resp_cap)
            return n; // no room for the payload; ship the header
        resp[n++] = COAP_PAYLOAD_MARKER;
        memcpy(resp + n, cresp.payload, cresp.payload_len);
        n += cresp.payload_len;
    }

    return n;
}

// ---------------------------------------------------------------------------
// UDP transport (det_udp_listen is a host stub on non-Arduino builds)
// ---------------------------------------------------------------------------

static uint8_t g_coap_tx[DETWS_COAP_MSG_BUF_SIZE]; // response scratch (request is transport-owned)

static void coap_udp_handler(const uint8_t *data, size_t len, struct DetUdpPeer *peer, void *ctx)
{
    (void)ctx;
    size_t rn = coap_server_process(data, len, g_coap_tx, sizeof(g_coap_tx));
    if (rn)
        det_udp_send(peer, g_coap_tx, rn);
}

void coap_server_begin_udp(uint16_t port)
{
    det_udp_listen(port, coap_udp_handler, nullptr);
}

#endif // DETWS_ENABLE_COAP
