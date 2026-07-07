// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file coap.cpp
 * @brief CoAP server (RFC 7252): resource table, message codec, and the UDP binding.
 */

#include "services/coap/coap.h"

#if DETWS_ENABLE_COAP

#include "network_drivers/transport/udp.h"
#include <string.h>
#if DETWS_ENABLE_COAP_OBSERVE
#include "services/clock.h" // detws_millis() for Observe notification message IDs / sequencing
#endif

// CoAP option numbers we understand (RFC 7252 §5.10, RFC 7959). Others are skipped.
enum
{
    COAP_OPT_OBSERVE = 6,
    COAP_OPT_URI_PATH = 11,
    COAP_OPT_CONTENT_FORMAT = 12,
    COAP_OPT_URI_QUERY = 15,
    COAP_OPT_BLOCK2 = 23, ///< block-wise responses (RFC 7959)
    COAP_OPT_BLOCK1 = 27, ///< block-wise request uploads (RFC 7959)
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

#if DETWS_ENABLE_COAP_OBSERVE
// An Observe registration (RFC 7641): a client awaiting notifications on a resource.
struct CoapObserver
{
    bool active;
    char ip[16];
    uint16_t port;
    uint8_t token[8];
    uint8_t tkl;
    int res_idx;
    uint32_t seq; // last Observe value sent
};
#endif

// All CoAP server state, owned by one instance. Grouping what were scattered file-scope
// mutables means state flows explicitly through the call graph (readers take a
// `const CoapCtx&` and cannot mutate the table), and nothing here is ambient.
struct CoapCtx
{
    CoapResource res[DETWS_COAP_MAX_RESOURCES]; // resource table: set before begin, read-only during dispatch
    size_t res_count = 0;

    char path[DETWS_COAP_MAX_PATH];      // scratch: reconstructed Uri-Path of the request in flight
    char query[DETWS_COAP_MAX_QUERY];    // scratch: reconstructed Uri-Query
    uint8_t pl[DETWS_COAP_MAX_PAYLOAD];  // scratch: handler response body
    uint8_t tx[DETWS_COAP_MSG_BUF_SIZE]; // scratch: outbound response (request buffer is transport-owned)

#if DETWS_ENABLE_COAP_OBSERVE
    uint16_t port = 5683; // UDP port the observe transport notifies from
    CoapObserver obs[DETWS_COAP_MAX_OBSERVERS];
    // Last-request fields recorded by coap_server_process_ex() for the Observe transport.
    int last_observe = -1;
    uint8_t last_method = 0;
    uint8_t last_token[8];
    uint8_t last_tkl = 0;
#endif

#if DETWS_ENABLE_COAP_BLOCK
    // Single in-flight Block1 (request upload) reassembly (RFC 7959); one transfer at a time.
    uint8_t b1[DETWS_COAP_BLOCK1_MAX];
    size_t b1_len = 0;  // bytes reassembled so far (also the next expected offset)
    uint8_t b1_szx = 0; // negotiated block-size exponent for this transfer
#endif
};

static CoapCtx s_coap;

void coap_server_init()
{
    s_coap.res_count = 0;
    memset(s_coap.res, 0, sizeof(s_coap.res));
#if DETWS_ENABLE_COAP_BLOCK
    s_coap.b1_len = 0;
    s_coap.b1_szx = 0;
#endif
}

bool coap_server_add_resource(const char *path, uint8_t methods, CoapHandler handler)
{
    if (s_coap.res_count >= DETWS_COAP_MAX_RESOURCES || !path || !handler)
        return false;
    s_coap.res[s_coap.res_count].path = path;
    s_coap.res[s_coap.res_count].methods = methods;
    s_coap.res[s_coap.res_count].handler = handler;
    s_coap.res_count++;
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

static const CoapResource *find_resource(const CoapCtx &c, const char *path)
{
    for (size_t i = 0; i < c.res_count; i++)
        if (strcmp(c.res[i].path, path) == 0)
            return &c.res[i];
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

// Encode an unsigned value into 0..3 big-endian bytes, dropping leading zeros
// (CoAP's minimal uint option encoding). Returns the byte count (0 when v == 0).
static uint8_t enc_uint_minimal(uint32_t v, uint8_t out[3])
{
    uint8_t k = 0;
    if (v & 0xFF0000)
        out[k++] = (uint8_t)(v >> 16);
    if (v & 0xFFFF00)
        out[k++] = (uint8_t)(v >> 8);
    if (v)
        out[k++] = (uint8_t)v;
    return k;
}

// Append one option whose number is @p opt_num (>= @p *last_opt) and whose value
// is @p vlen bytes. Handles an extended option delta (13..268, one extension
// byte); the value length is always < 13 for the options this server emits, so no
// extended length is needed. Stops without writing if it would overflow @p cap.
// Returns the new length.
static size_t append_opt(uint8_t *resp, size_t cap, size_t n, uint32_t *last_opt, uint32_t opt_num, const uint8_t *val,
                         uint8_t vlen)
{
    uint32_t delta = opt_num - *last_opt;
    uint8_t dn = (uint8_t)(delta < 13 ? delta : 13);
    bool ext = delta >= 13;
    if (n + 1u + (ext ? 1u : 0u) + vlen > cap)
        return n;
    resp[n++] = (uint8_t)((dn << 4) | vlen);
    if (ext)
        resp[n++] = (uint8_t)(delta - 13);
    for (uint8_t i = 0; i < vlen; i++)
        resp[n++] = val[i];
    *last_opt = opt_num;
    return n;
}

// Append options (Observe, Content-Format, Block2, Block1 - ascending order) and
// the payload to a message already holding @p n header+token bytes. @p observe_seq
// < 0 omits the Observe option (also omitted unless @p code is a 2.xx success);
// @p block2_val / @p block1_val < 0 omit the respective Block option (their value
// is the raw RFC 7959 option uint, (NUM<<4)|(M<<3)|SZX). Stops (shipping what fits)
// rather than overflowing @p cap. Returns the new length.
static size_t emit_options_payload(uint8_t *resp, size_t cap, size_t n, uint8_t code, int32_t observe_seq,
                                   uint16_t content_format, int32_t block2_val, int32_t block1_val,
                                   const uint8_t *payload, size_t payload_len)
{
    uint32_t last_opt = 0;
    uint8_t v[3];

    if (observe_seq >= 0 && (code >> 5) == 2)
        n = append_opt(resp, cap, n, &last_opt, COAP_OPT_OBSERVE, v,
                       enc_uint_minimal((uint32_t)observe_seq & 0xFFFFFF, v));

    if (content_format != COAP_CF_NONE)
        n = append_opt(resp, cap, n, &last_opt, COAP_OPT_CONTENT_FORMAT, v, enc_uint_minimal(content_format, v));

#if DETWS_ENABLE_COAP_BLOCK
    if (block2_val >= 0)
        n = append_opt(resp, cap, n, &last_opt, COAP_OPT_BLOCK2, v, enc_uint_minimal((uint32_t)block2_val, v));
    if (block1_val >= 0)
        n = append_opt(resp, cap, n, &last_opt, COAP_OPT_BLOCK1, v, enc_uint_minimal((uint32_t)block1_val, v));
#else
    (void)block2_val;
    (void)block1_val;
#endif

    if (payload_len)
    {
        if (n + 1 + payload_len > cap)
            return n;
        resp[n++] = COAP_PAYLOAD_MARKER;
        memcpy(resp + n, payload, payload_len);
        n += payload_len;
    }
    return n;
}

// ---------------------------------------------------------------------------
// Core processing (no sockets, no heap)
// ---------------------------------------------------------------------------

size_t coap_server_process_ex(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap, int32_t observe_seq)
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

#if DETWS_ENABLE_COAP_OBSERVE
    s_coap.last_observe = -1;
    s_coap.last_method = code;
    s_coap.last_tkl = tkl;
    if (tkl)
        memcpy(s_coap.last_token, token, tkl);
#endif

    // Decode options, reconstructing Uri-Path / Uri-Query and reading Content-Format.
    size_t path_len = 0, query_len = 0;
    s_coap.path[0] = '\0';
    s_coap.query[0] = '\0';
    uint16_t req_cf = COAP_CF_NONE;
    const uint8_t *payload = nullptr;
    size_t payload_len = 0;
    uint32_t opt_num = 0;
    bool bad = false;
    bool bad_option = false; // unrecognized critical (odd-numbered) option seen (RFC 7252 5.4.1)
#if DETWS_ENABLE_COAP_BLOCK
    int32_t req_block1 = -1; // request Block1 option value (RFC 7959), or -1 if absent
    int32_t req_block2 = -1; // request Block2 option value, or -1 if absent
#endif

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
            if (!seg_append(s_coap.path, sizeof(s_coap.path), &path_len, '/', val, olen))
                bad = true;
            break;
        case COAP_OPT_URI_QUERY:
            if (!seg_append(s_coap.query, sizeof(s_coap.query), &query_len, query_len ? '&' : '\0', val, olen))
                bad = true;
            break;
        case COAP_OPT_CONTENT_FORMAT:
            req_cf = (uint16_t)opt_uint(val, olen);
            break;
#if DETWS_ENABLE_COAP_OBSERVE
        case COAP_OPT_OBSERVE:
            s_coap.last_observe = (int)opt_uint(val, olen); // 0 = register, 1 = deregister
            break;
#endif
#if DETWS_ENABLE_COAP_BLOCK
        case COAP_OPT_BLOCK1: // block-wise request uploads (RFC 7959)
            if (olen > 3)
                bad = true;
            else
                req_block1 = (int32_t)opt_uint(val, olen);
            break;
        case COAP_OPT_BLOCK2: // block-wise responses (RFC 7959)
            if (olen > 3)
                bad = true;
            else
                req_block2 = (int32_t)opt_uint(val, olen);
            break;
#endif
        default:
            // RFC 7252 5.4.1: an unrecognized option of class "critical" (odd option
            // number) in a request must cause a 4.02 (Bad Option). Elective (even)
            // unknown options are silently ignored. Block1/Block2 are critical, so a
            // build without COAP_BLOCK correctly rejects them here.
            if (opt_num & 1)
                bad_option = true;
            break;
        }
        if (bad)
            break;
    }

    if (bad)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_REQUEST, mid, token, tkl);
    if (bad_option)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_OPTION, mid, token, tkl);

    if (path_len == 0)
    {
        s_coap.path[0] = '/';
        s_coap.path[1] = '\0';
    }

    // Only class-0 GET/POST/PUT/DELETE are supported request methods. RFC 7252 5.8:
    // "A request with an unrecognized or unsupported Method Code MUST generate a 4.05
    // (Method Not Allowed) piggybacked response."
    if ((code >> 5) != 0 || code < COAP_GET || code > COAP_DELETE)
        return emit_header(resp, resp_cap, rsp_type, COAP_RSP_METHOD_NOT_ALLOWED, mid, token, tkl);

    // The response the emit path below serializes (block-wise if large). Filled
    // either by the .well-known/core discovery listing or by a resource handler.
    CoapResponse cresp;
    cresp.code = COAP_RSP_CONTENT;
    cresp.content_format = COAP_CF_NONE;
    cresp.payload = s_coap.pl;
    cresp.payload_cap = sizeof(s_coap.pl);
    cresp.payload_len = 0;
    int32_t block1_echo = -1; // Block1 option to echo on the final-block response

    // RFC 6690: GET /.well-known/core returns the CoRE Link Format listing of the
    // registered resources, e.g. "</info>,</led>". Block2 (below) pages it if large.
    if (strcmp(s_coap.path, "/.well-known/core") == 0)
    {
        if (code != COAP_GET)
            return emit_header(resp, resp_cap, rsp_type, COAP_RSP_METHOD_NOT_ALLOWED, mid, token, tkl);
        size_t pl = 0;
        for (size_t i = 0; i < s_coap.res_count; i++)
        {
            const char *rpath = s_coap.res[i].path;
            size_t plen = strlen(rpath);
            size_t need = (pl ? 1u : 0u) + 2u + plen; // optional ',' + '<' + path + '>'
            if (pl + need > sizeof(s_coap.pl))
                break; // listing exceeds the payload buffer; truncate at a resource boundary
            if (pl)
                s_coap.pl[pl++] = ',';
            s_coap.pl[pl++] = '<';
            memcpy(s_coap.pl + pl, rpath, plen);
            pl += plen;
            s_coap.pl[pl++] = '>';
        }
        cresp.content_format = COAP_CF_LINK;
        cresp.payload_len = pl;
    }
    else
    {
        const CoapResource *r = find_resource(s_coap, s_coap.path);
        if (!r)
            return emit_header(resp, resp_cap, rsp_type, COAP_RSP_NOT_FOUND, mid, token, tkl);
        if (!(r->methods & (1u << code)))
            return emit_header(resp, resp_cap, rsp_type, COAP_RSP_METHOD_NOT_ALLOWED, mid, token, tkl);

        const uint8_t *eff_payload = payload;
        size_t eff_payload_len = payload_len;

#if DETWS_ENABLE_COAP_BLOCK
        // --- Block1: reassemble a chunked POST/PUT payload (RFC 7959 §2.5) ---
        if (req_block1 >= 0 && (code == COAP_POST || code == COAP_PUT))
        {
            uint32_t b = (uint32_t)req_block1;
            uint32_t num = b >> 4;
            uint8_t more = (uint8_t)((b >> 3) & 1);
            uint8_t szx = (uint8_t)(b & 7);
            if (szx == 7) // reserved block size
                return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_OPTION, mid, token, tkl);
            uint32_t bsize = 1u << (szx + 4);
            if (num == 0) // first block starts a fresh transfer
            {
                s_coap.b1_len = 0;
                s_coap.b1_szx = szx;
            }
            // The block size is fixed for a transfer; an offset gap means a lost or
            // reordered block. Either way the reassembly cannot continue: 4.08.
            if (szx != s_coap.b1_szx || (size_t)num * bsize != s_coap.b1_len)
            {
                s_coap.b1_len = 0;
                return emit_header(resp, resp_cap, rsp_type, COAP_RSP_REQUEST_INCOMPLETE, mid, token, tkl);
            }
            if (s_coap.b1_len + payload_len > sizeof(s_coap.b1))
            {
                s_coap.b1_len = 0;
                return emit_header(resp, resp_cap, rsp_type, COAP_RSP_REQUEST_TOO_LARGE, mid, token, tkl);
            }
            if (payload_len)
                memcpy(s_coap.b1 + s_coap.b1_len, payload, payload_len);
            s_coap.b1_len += payload_len;

            if (more)
            {
                // More blocks coming: acknowledge with 2.31 Continue + Block1 echo
                // (no representation yet; the handler runs only on the final block).
                size_t cn = emit_header(resp, resp_cap, rsp_type, COAP_RSP_CONTINUE, mid, token, tkl);
                if (cn == 0)
                    return 0;
                return emit_options_payload(resp, resp_cap, cn, COAP_RSP_CONTINUE, -1, COAP_CF_NONE, -1,
                                            (int32_t)((num << 4) | (1u << 3) | szx), nullptr, 0);
            }
            // Final block: hand the whole reassembled payload to the handler.
            eff_payload = s_coap.b1;
            eff_payload_len = s_coap.b1_len;
            block1_echo = (int32_t)((num << 4) | szx); // More = 0
        }
#endif

        CoapRequest creq;
        creq.method = code;
        creq.path = s_coap.path;
        creq.query = s_coap.query;
        creq.payload = eff_payload;
        creq.payload_len = eff_payload_len;
        creq.content_format = req_cf;

        r->handler(&creq, &cresp);
        if (cresp.payload_len > sizeof(s_coap.pl))
            cresp.payload_len = sizeof(s_coap.pl); // defensive clamp
    }

    int32_t block2_echo = -1;

#if DETWS_ENABLE_COAP_BLOCK
    if (block1_echo >= 0)
        s_coap.b1_len = 0; // the reassembled upload has been handed to the handler; clear it

    // --- Block2: serve a (large or explicitly requested) representation one
    // block at a time (RFC 7959 §2.4). Applies only to a successful body. ---
    if ((cresp.code >> 5) == 2)
    {
        uint8_t szx = DETWS_COAP_BLOCK_SZX_MAX;
        uint32_t num = 0;
        bool block_wise = false;
        if (req_block2 >= 0)
        {
            uint32_t b = (uint32_t)req_block2;
            num = b >> 4;
            szx = (uint8_t)(b & 7);
            if (szx == 7)
                return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_OPTION, mid, token, tkl);
            if (szx > DETWS_COAP_BLOCK_SZX_MAX)
                szx = DETWS_COAP_BLOCK_SZX_MAX;
            block_wise = true; // the client asked for block-wise transfer
        }
        else if (cresp.payload_len > (size_t)(1u << (szx + 4)))
        {
            block_wise = true; // body too large for one block -> start at block 0
        }
        if (block_wise)
        {
            uint32_t bsize = 1u << (szx + 4);
            size_t off = (size_t)num * bsize;
            // A block number past the end of the representation is a bad request.
            if (off > cresp.payload_len || (off == cresp.payload_len && num > 0))
                return emit_header(resp, resp_cap, rsp_type, COAP_RSP_BAD_REQUEST, mid, token, tkl);
            size_t this_len = cresp.payload_len - off;
            uint8_t more = 0;
            if (this_len > bsize)
            {
                this_len = bsize;
                more = 1;
            }
            block2_echo = (int32_t)((num << 4) | ((uint32_t)more << 3) | szx);
            cresp.payload += off;
            cresp.payload_len = this_len;
        }
    }
#endif

    // Build the response: header + token + options (ascending order) + payload.
    size_t n = emit_header(resp, resp_cap, rsp_type, cresp.code, mid, token, tkl);
    if (n == 0)
        return 0;
    return emit_options_payload(resp, resp_cap, n, cresp.code, observe_seq, cresp.content_format, block2_echo,
                                block1_echo, cresp.payload, cresp.payload_len);
}

size_t coap_server_process(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap)
{
    return coap_server_process_ex(req, req_len, resp, resp_cap, -1); // no Observe option
}

// ---------------------------------------------------------------------------
// UDP transport (det_udp_listen is a host stub on non-Arduino builds)
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_COAP_OBSERVE
// ---------------------------------------------------------------------------
// Observe registry + notifications (RFC 7641)
// ---------------------------------------------------------------------------

static int find_resource_index(const CoapCtx &c, const char *path)
{
    for (size_t i = 0; i < c.res_count; i++)
        if (strcmp(c.res[i].path, path) == 0)
            return (int)i;
    return -1;
}

static bool same_token(const CoapObserver *o, const uint8_t *token, uint8_t tkl)
{
    return o->tkl == tkl && (tkl == 0 || memcmp(o->token, token, tkl) == 0);
}

// Find/refresh an observer; create one on first registration. Returns its slot or -1.
static int obs_register(const char *ip, uint16_t port, const uint8_t *token, uint8_t tkl, int res_idx)
{
    for (int i = 0; i < DETWS_COAP_MAX_OBSERVERS; i++)
        if (s_coap.obs[i].active && s_coap.obs[i].res_idx == res_idx && s_coap.obs[i].port == port &&
            same_token(&s_coap.obs[i], token, tkl) && strcmp(s_coap.obs[i].ip, ip) == 0)
            return i; // already observing
    for (int i = 0; i < DETWS_COAP_MAX_OBSERVERS; i++)
        if (!s_coap.obs[i].active)
        {
            s_coap.obs[i].active = true;
            strncpy(s_coap.obs[i].ip, ip, sizeof(s_coap.obs[i].ip) - 1);
            s_coap.obs[i].ip[sizeof(s_coap.obs[i].ip) - 1] = '\0';
            s_coap.obs[i].port = port;
            s_coap.obs[i].tkl = tkl;
            if (tkl)
                memcpy(s_coap.obs[i].token, token, tkl);
            s_coap.obs[i].res_idx = res_idx;
            s_coap.obs[i].seq = 1;
            return i;
        }
    return -1; // registry full -> observation declined (resource still returned)
}

// Remove a specific observation (deregister GET) or every observation from a peer
// (a Reset). Pass token=nullptr to drop all of @p ip:@p port.
static void obs_remove(const char *ip, uint16_t port, const uint8_t *token, uint8_t tkl)
{
    for (int i = 0; i < DETWS_COAP_MAX_OBSERVERS; i++)
        if (s_coap.obs[i].active && s_coap.obs[i].port == port && strcmp(s_coap.obs[i].ip, ip) == 0 &&
            (token == nullptr || same_token(&s_coap.obs[i], token, tkl)))
            s_coap.obs[i].active = false;
}

void coap_notify(const char *path)
{
    int ridx = find_resource_index(s_coap, path);
    if (ridx < 0)
        return;
    for (int i = 0; i < DETWS_COAP_MAX_OBSERVERS; i++)
    {
        if (!s_coap.obs[i].active || s_coap.obs[i].res_idx != ridx)
            continue;
        // Re-render the resource via its GET handler.
        CoapRequest creq;
        creq.method = COAP_GET;
        creq.path = s_coap.res[ridx].path;
        creq.query = "";
        creq.payload = nullptr;
        creq.payload_len = 0;
        creq.content_format = COAP_CF_NONE;
        CoapResponse cresp;
        cresp.code = COAP_RSP_CONTENT;
        cresp.content_format = COAP_CF_NONE;
        cresp.payload = s_coap.pl;
        cresp.payload_cap = sizeof(s_coap.pl);
        cresp.payload_len = 0;
        s_coap.res[ridx].handler(&creq, &cresp);
        if (cresp.payload_len > sizeof(s_coap.pl))
            cresp.payload_len = sizeof(s_coap.pl);

        // Build a NON notification: header + token + Observe(seq) + body.
        uint16_t mid = (uint16_t)detws_millis();
        s_coap.obs[i].seq = (s_coap.obs[i].seq + 1) & 0xFFFFFF;
        size_t n = emit_header(s_coap.tx, sizeof(s_coap.tx), COAP_TYPE_NON, cresp.code, mid, s_coap.obs[i].token,
                               s_coap.obs[i].tkl);
        if (n)
            n = emit_options_payload(s_coap.tx, sizeof(s_coap.tx), n, cresp.code, (int32_t)s_coap.obs[i].seq,
                                     cresp.content_format, -1, -1, cresp.payload, cresp.payload_len);
        if (!n || !det_udp_listener_sendto(s_coap.port, s_coap.obs[i].ip, s_coap.obs[i].port, s_coap.tx, n))
            s_coap.obs[i].active = false; // unreachable -> drop the observer
    }
}

static void coap_udp_handler(const uint8_t *data, size_t len, struct DetUdpPeer *peer, void *ctx)
{
    (void)ctx;
    char ip[16];
    uint16_t pport = 0;
    bool have_peer = det_udp_peer_addr(peer, ip, sizeof(ip), &pport);

    // A Reset from a client rejects our notification -> drop its observations.
    if (len >= 1 && ((data[0] >> 4) & 0x03) == COAP_TYPE_RST)
    {
        if (have_peer)
            obs_remove(ip, pport, nullptr, 0);
        return;
    }

    size_t rn = coap_server_process_ex(data, len, s_coap.tx, sizeof(s_coap.tx), -1);
    if (!rn)
        return;

    if (s_coap.last_method == COAP_GET && s_coap.last_observe == 0 && have_peer)
    {
        int ridx = find_resource_index(s_coap, s_coap.path);
        if (ridx >= 0)
        {
            int slot = obs_register(ip, pport, s_coap.last_token, s_coap.last_tkl, ridx);
            if (slot >= 0)
            {
                // Re-encode the response carrying the Observe option (registration ack).
                size_t rn2 =
                    coap_server_process_ex(data, len, s_coap.tx, sizeof(s_coap.tx), (int32_t)s_coap.obs[slot].seq);
                if (rn2)
                    rn = rn2;
            }
        }
    }
    else if (s_coap.last_observe == 1 && have_peer)
    {
        obs_remove(ip, pport, s_coap.last_token, s_coap.last_tkl);
    }

    det_udp_send(peer, s_coap.tx, rn);
}

void coap_server_begin_udp(uint16_t port)
{
    s_coap.port = port;
    for (int i = 0; i < DETWS_COAP_MAX_OBSERVERS; i++)
        s_coap.obs[i].active = false;
    det_udp_listen(port, coap_udp_handler, nullptr);
}

#else // Observe disabled: the basic request/response handler

static void coap_udp_handler(const uint8_t *data, size_t len, struct DetUdpPeer *peer, void *ctx)
{
    (void)ctx;
    size_t rn = coap_server_process(data, len, s_coap.tx, sizeof(s_coap.tx));
    if (rn)
        det_udp_send(peer, s_coap.tx, rn);
}

void coap_server_begin_udp(uint16_t port)
{
    det_udp_listen(port, coap_udp_handler, nullptr);
}

#endif // DETWS_ENABLE_COAP_OBSERVE

#endif // DETWS_ENABLE_COAP
