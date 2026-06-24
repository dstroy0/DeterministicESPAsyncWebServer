// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_agent.cpp
 * @brief SNMP v1/v2c agent: MIB table, PDU dispatch, and the lwIP UDP socket.
 */

#include "services/snmp/snmp_agent.h"

#if DETWS_ENABLE_SNMP

#include <string.h>

#if DETWS_ENABLE_SNMP_V3
#include "services/snmp/snmp_v3.h"
#endif

#if defined(ARDUINO)
#include <Arduino.h>
static uint32_t snmp_uptime_cs()
{
    return (uint32_t)(millis() / 10ULL); // hundredths of a second since boot
}
#else
static uint32_t snmp_uptime_cs()
{
    return 0; // host build has no clock; tests assert type, not value
}
#endif

// ---------------------------------------------------------------------------
// MIB table + community configuration (all in BSS - no heap)
// ---------------------------------------------------------------------------

struct SnmpMibEntry
{
    uint32_t oid[SNMP_MAX_OID_LEN];
    size_t oid_len;
    SnmpValue val;    // static value (used when getter == nullptr)
    SnmpGetFn getter; // dynamic value provider (optional)
    SnmpSetFn setter; // writable hook (optional)
};

static SnmpMibEntry g_mib[SNMP_MAX_MIB_ENTRIES];
static size_t g_mib_count = 0;

static char g_ro[SNMP_COMMUNITY_MAX] = "public";
static char g_rw[SNMP_COMMUNITY_MAX] = "";
static bool g_rw_set = false;

// sysObjectID value (private enterprise placeholder: 1.3.6.1.4.1.49374).
static const uint32_t g_sys_object_id[] = {1, 3, 6, 1, 4, 1, 49374};

// ---------------------------------------------------------------------------
// OID helpers
// ---------------------------------------------------------------------------

static int oid_cmp(const uint32_t *a, size_t an, const uint32_t *b, size_t bn)
{
    size_t n = an < bn ? an : bn;
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] < b[i])
            return -1;
        if (a[i] > b[i])
            return 1;
    }
    if (an < bn)
        return -1;
    if (an > bn)
        return 1;
    return 0;
}

static const SnmpMibEntry *mib_find_exact(const uint32_t *oid, size_t n)
{
    for (size_t i = 0; i < g_mib_count; i++)
        if (oid_cmp(g_mib[i].oid, g_mib[i].oid_len, oid, n) == 0)
            return &g_mib[i];
    return nullptr;
}

// Smallest registered OID that is strictly greater than (oid,n), or nullptr.
static const SnmpMibEntry *mib_find_next(const uint32_t *oid, size_t n)
{
    const SnmpMibEntry *best = nullptr;
    for (size_t i = 0; i < g_mib_count; i++)
    {
        if (oid_cmp(g_mib[i].oid, g_mib[i].oid_len, oid, n) > 0)
        {
            if (!best || oid_cmp(g_mib[i].oid, g_mib[i].oid_len, best->oid, best->oid_len) < 0)
                best = &g_mib[i];
        }
    }
    return best;
}

static bool fetch_value(const SnmpMibEntry *en, SnmpValue *out)
{
    if (en->getter)
        return en->getter(out);
    *out = en->val;
    return true;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

static SnmpMibEntry *mib_alloc(const uint32_t *oid, size_t n)
{
    if (g_mib_count >= SNMP_MAX_MIB_ENTRIES || n < 2 || n > SNMP_MAX_OID_LEN)
        return nullptr;
    SnmpMibEntry *e = &g_mib[g_mib_count++];
    memset(e, 0, sizeof(*e));
    for (size_t i = 0; i < n; i++)
        e->oid[i] = oid[i];
    e->oid_len = n;
    return e;
}

void snmp_agent_init(const char *ro_community)
{
    g_mib_count = 0;
    g_rw_set = false;
    g_rw[0] = '\0';
    const char *ro = (ro_community && ro_community[0]) ? ro_community : "public";
    strncpy(g_ro, ro, sizeof(g_ro) - 1);
    g_ro[sizeof(g_ro) - 1] = '\0';
}

void snmp_agent_set_rw_community(const char *rw_community)
{
    if (!rw_community || !rw_community[0])
    {
        g_rw_set = false;
        g_rw[0] = '\0';
        return;
    }
    strncpy(g_rw, rw_community, sizeof(g_rw) - 1);
    g_rw[sizeof(g_rw) - 1] = '\0';
    g_rw_set = true;
}

bool snmp_agent_add_string(const uint32_t *oid, size_t oid_len, const char *value, SnmpSetFn setter)
{
    SnmpMibEntry *e = mib_alloc(oid, oid_len);
    if (!e)
        return false;
    e->val.type = BER_OCTET_STRING;
    e->val.str = value;
    e->val.str_len = value ? strlen(value) : 0;
    e->setter = setter;
    return true;
}

bool snmp_agent_add_integer(const uint32_t *oid, size_t oid_len, long value, SnmpSetFn setter)
{
    SnmpMibEntry *e = mib_alloc(oid, oid_len);
    if (!e)
        return false;
    e->val.type = BER_INTEGER;
    e->val.ival = value;
    e->setter = setter;
    return true;
}

bool snmp_agent_add_dynamic(const uint32_t *oid, size_t oid_len, uint8_t type, SnmpGetFn getter)
{
    SnmpMibEntry *e = mib_alloc(oid, oid_len);
    if (!e)
        return false;
    e->val.type = type;
    e->getter = getter;
    return true;
}

static bool sys_uptime_get(SnmpValue *out)
{
    out->type = SNMP_TIMETICKS;
    out->uval = snmp_uptime_cs();
    return true;
}

void snmp_agent_set_system(const char *descr, const char *contact, const char *name, const char *location,
                           long services)
{
    const uint32_t o_descr[] = {1, 3, 6, 1, 2, 1, 1, 1, 0};
    const uint32_t o_oid[] = {1, 3, 6, 1, 2, 1, 1, 2, 0};
    const uint32_t o_uptime[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
    const uint32_t o_contact[] = {1, 3, 6, 1, 2, 1, 1, 4, 0};
    const uint32_t o_name[] = {1, 3, 6, 1, 2, 1, 1, 5, 0};
    const uint32_t o_loc[] = {1, 3, 6, 1, 2, 1, 1, 6, 0};
    const uint32_t o_svc[] = {1, 3, 6, 1, 2, 1, 1, 7, 0};

    snmp_agent_add_string(o_descr, 9, descr);

    SnmpMibEntry *e = mib_alloc(o_oid, 9);
    if (e)
    {
        e->val.type = BER_OID;
        e->val.oid = g_sys_object_id;
        e->val.oid_len = sizeof(g_sys_object_id) / sizeof(g_sys_object_id[0]);
    }

    snmp_agent_add_dynamic(o_uptime, 9, SNMP_TIMETICKS, sys_uptime_get);
    snmp_agent_add_string(o_contact, 9, contact);
    snmp_agent_add_string(o_name, 9, name);
    snmp_agent_add_string(o_loc, 9, location);
    snmp_agent_add_integer(o_svc, 9, services);
}

// ---------------------------------------------------------------------------
// Value encode / decode
// ---------------------------------------------------------------------------

static void enc_value(BerEnc *e, const SnmpValue *v)
{
    switch (v->type)
    {
    case BER_INTEGER:
        ber_put_integer(e, v->ival);
        break;
    case BER_OCTET_STRING:
    case SNMP_OPAQUE:
        ber_put_octet_string(e, v->type, (const uint8_t *)v->str, v->str_len);
        break;
    case BER_OID:
        ber_put_oid(e, v->oid, v->oid_len);
        break;
    case SNMP_TIMETICKS:
    case SNMP_COUNTER32:
    case SNMP_GAUGE32:
        ber_put_uint(e, v->type, v->uval);
        break;
    case SNMP_IPADDRESS: {
        uint8_t ip[4] = {(uint8_t)(v->uval >> 24), (uint8_t)(v->uval >> 16), (uint8_t)(v->uval >> 8),
                         (uint8_t)(v->uval)};
        ber_put_octet_string(e, SNMP_IPADDRESS, ip, 4);
        break;
    }
    case BER_NULL:
        ber_put_null(e);
        break;
    default:
        // Exception markers (noSuchObject / noSuchInstance / endOfMibView): the
        // tag with a zero-length, no-value encoding.
        ber_put_tlv(e, v->type, nullptr, 0);
        break;
    }
}

// Read one varbind value TLV at the decoder cursor into @p v (OID values decode
// into @p oidbuf). Advances the cursor past the value.
static bool dec_value(BerDec *d, SnmpValue *v, uint32_t *oidbuf)
{
    memset(v, 0, sizeof(*v));
    size_t save = d->pos;
    uint8_t tag;
    size_t len;
    if (!ber_read_header(d, &tag, &len))
        return false;
    v->type = tag;
    switch (tag)
    {
    case BER_INTEGER:
        d->pos = save;
        return ber_read_integer(d, &v->ival);
    case SNMP_TIMETICKS:
    case SNMP_COUNTER32:
    case SNMP_GAUGE32:
    case SNMP_IPADDRESS: {
        uint32_t acc = 0;
        for (size_t i = 0; i < len; i++)
            acc = (acc << 8) | d->buf[d->pos + i];
        v->uval = acc;
        d->pos += len;
        return true;
    }
    case BER_OCTET_STRING:
    case SNMP_OPAQUE:
        v->str = (const char *)(d->buf + d->pos);
        v->str_len = len;
        d->pos += len;
        return true;
    case BER_OID:
        d->pos = save;
        if (!ber_read_oid(d, oidbuf, SNMP_MAX_OID_LEN, &v->oid_len))
            return false;
        v->oid = oidbuf;
        return true;
    default: // NULL (GET requests) and anything else: skip the value bytes
        d->pos += len;
        return true;
    }
}

// ---------------------------------------------------------------------------
// Per-request scratch (single-threaded: lwIP callback or test, never reentrant)
// ---------------------------------------------------------------------------

struct InVb
{
    uint32_t oid[SNMP_MAX_OID_LEN];
    size_t oid_len;
    SnmpValue val;
    uint32_t valoid[SNMP_MAX_OID_LEN]; // backing store if the value is an OID
};

struct OutVb
{
    const uint32_t *oid;
    size_t oid_len;
    SnmpValue val;
};

static InVb g_in[SNMP_MAX_VARBINDS];
static OutVb g_out[SNMP_MAX_VARBINDS];

static bool community_eq(const char *stored, const char *p, size_t len)
{
    return stored[0] != '\0' && strlen(stored) == len && memcmp(stored, p, len) == 0;
}

static size_t encode_pdu(long request_id, long err_status, long err_index, const OutVb *out, size_t nout, uint8_t *buf,
                         size_t cap)
{
    BerEnc e;
    ber_enc_init(&e, buf, cap);
    size_t pdu = ber_seq_begin(&e, SNMP_PDU_RESPONSE);
    ber_put_integer(&e, request_id);
    ber_put_integer(&e, err_status);
    ber_put_integer(&e, err_index);
    size_t vbl = ber_seq_begin(&e, BER_SEQUENCE);
    for (size_t i = 0; i < nout; i++)
    {
        size_t vb = ber_seq_begin(&e, BER_SEQUENCE);
        ber_put_oid(&e, out[i].oid, out[i].oid_len);
        enc_value(&e, &out[i].val);
        ber_seq_end(&e, vb);
    }
    ber_seq_end(&e, vbl);
    ber_seq_end(&e, pdu);
    return e.ok ? e.len : 0;
}

// Process one request PDU (a Get/GetNext/GetBulk/Set TLV) against the MIB and
// emit one GetResponse PDU. Shared by the v1/v2c community framing and the v3
// USM layer. @p v2c selects v2c-style per-varbind exceptions over v1
// error-status; @p allow_write authorizes Set.
size_t snmp_dispatch_pdu(const uint8_t *pdu, size_t pdu_len, bool allow_write, bool v2c, uint8_t *out, size_t out_cap)
{
    BerDec d;
    ber_dec_init(&d, pdu, pdu_len);

    uint8_t pdu_tag;
    size_t pdu_clen;
    if (!ber_read_header(&d, &pdu_tag, &pdu_clen))
        return 0;

    long request_id, field2, field3;
    if (!ber_read_integer(&d, &request_id) || !ber_read_integer(&d, &field2) || !ber_read_integer(&d, &field3))
        return 0;

    uint8_t vbl_tag;
    size_t vbl_len;
    if (!ber_read_header(&d, &vbl_tag, &vbl_len) || vbl_tag != BER_SEQUENCE)
        return 0;
    size_t vbl_end = d.pos + vbl_len;

    // Decode the request varbind list.
    size_t nvb = 0;
    while (d.pos < vbl_end && d.ok)
    {
        if (nvb >= SNMP_MAX_VARBINDS)
            return 0;
        uint8_t vt;
        size_t vlen;
        if (!ber_read_header(&d, &vt, &vlen) || vt != BER_SEQUENCE)
            return 0;
        if (!ber_read_oid(&d, g_in[nvb].oid, SNMP_MAX_OID_LEN, &g_in[nvb].oid_len))
            return 0;
        if (!dec_value(&d, &g_in[nvb].val, g_in[nvb].valoid))
            return 0;
        nvb++;
    }
    if (!d.ok)
        return 0;

    long err_status = SNMP_ERR_NO_ERROR;
    long err_index = 0;
    size_t nout = 0;

    if (pdu_tag == SNMP_PDU_GET || pdu_tag == SNMP_PDU_GETNEXT)
    {
        for (size_t i = 0; i < nvb; i++)
        {
            const SnmpMibEntry *en = (pdu_tag == SNMP_PDU_GET) ? mib_find_exact(g_in[i].oid, g_in[i].oid_len)
                                                               : mib_find_next(g_in[i].oid, g_in[i].oid_len);
            SnmpValue val;
            bool ok = en && fetch_value(en, &val);
            if (!ok)
            {
                if (!v2c)
                {
                    // Classic error reporting: echo the request varbinds.
                    err_status = SNMP_ERR_NO_SUCH_NAME;
                    err_index = (long)(i + 1);
                    nout = nvb;
                    for (size_t k = 0; k < nvb; k++)
                    {
                        g_out[k].oid = g_in[k].oid;
                        g_out[k].oid_len = g_in[k].oid_len;
                        g_out[k].val = g_in[k].val;
                    }
                    break;
                }
                // v2c: per-varbind exception.
                val.type = (pdu_tag == SNMP_PDU_GET) ? (uint8_t)SNMP_NO_SUCH_OBJECT : (uint8_t)SNMP_END_OF_MIB_VIEW;
                g_out[nout].oid = en ? en->oid : g_in[i].oid;
                g_out[nout].oid_len = en ? en->oid_len : g_in[i].oid_len;
                g_out[nout].val = val;
                nout++;
                continue;
            }
            g_out[nout].oid = (pdu_tag == SNMP_PDU_GETNEXT) ? en->oid : g_in[i].oid;
            g_out[nout].oid_len = (pdu_tag == SNMP_PDU_GETNEXT) ? en->oid_len : g_in[i].oid_len;
            g_out[nout].val = val;
            nout++;
        }
    }
    else if (pdu_tag == SNMP_PDU_GETBULK)
    {
        if (!v2c) // GetBulk is v2c+
            return 0;
        long non_rep = field2 < 0 ? 0 : field2;
        long max_rep = field3 < 0 ? 0 : field3;
        if ((size_t)non_rep > nvb)
            non_rep = (long)nvb;

        // Non-repeaters: one GetNext each.
        for (long i = 0; i < non_rep && nout < SNMP_MAX_VARBINDS; i++)
        {
            const SnmpMibEntry *en = mib_find_next(g_in[i].oid, g_in[i].oid_len);
            if (en)
            {
                g_out[nout].oid = en->oid;
                g_out[nout].oid_len = en->oid_len;
                fetch_value(en, &g_out[nout].val);
            }
            else
            {
                g_out[nout].oid = g_in[i].oid;
                g_out[nout].oid_len = g_in[i].oid_len;
                g_out[nout].val.type = SNMP_END_OF_MIB_VIEW;
            }
            nout++;
        }

        // Repeaters: walk each remaining varbind up to max_rep successors.
        size_t nrep = nvb - (size_t)non_rep;
        const uint32_t *cur_oid[SNMP_MAX_VARBINDS];
        size_t cur_len[SNMP_MAX_VARBINDS];
        bool ended[SNMP_MAX_VARBINDS];
        for (size_t r = 0; r < nrep; r++)
        {
            cur_oid[r] = g_in[non_rep + r].oid;
            cur_len[r] = g_in[non_rep + r].oid_len;
            ended[r] = false;
        }
        for (long rep = 0; rep < max_rep && nout < SNMP_MAX_VARBINDS; rep++)
        {
            for (size_t r = 0; r < nrep && nout < SNMP_MAX_VARBINDS; r++)
            {
                if (ended[r])
                {
                    g_out[nout].oid = cur_oid[r];
                    g_out[nout].oid_len = cur_len[r];
                    g_out[nout].val.type = SNMP_END_OF_MIB_VIEW;
                    nout++;
                    continue;
                }
                const SnmpMibEntry *en = mib_find_next(cur_oid[r], cur_len[r]);
                if (en)
                {
                    g_out[nout].oid = en->oid;
                    g_out[nout].oid_len = en->oid_len;
                    fetch_value(en, &g_out[nout].val);
                    cur_oid[r] = en->oid;
                    cur_len[r] = en->oid_len;
                }
                else
                {
                    g_out[nout].oid = cur_oid[r];
                    g_out[nout].oid_len = cur_len[r];
                    g_out[nout].val.type = SNMP_END_OF_MIB_VIEW;
                    ended[r] = true;
                }
                nout++;
            }
        }
    }
    else if (pdu_tag == SNMP_PDU_SET)
    {
        // SET uses error-status/error-index in both v1 and v2c; echo the varbinds.
        nout = nvb;
        for (size_t k = 0; k < nvb; k++)
        {
            g_out[k].oid = g_in[k].oid;
            g_out[k].oid_len = g_in[k].oid_len;
            g_out[k].val = g_in[k].val;
        }
        if (!allow_write)
        {
            err_status = (!v2c) ? SNMP_ERR_NO_SUCH_NAME : SNMP_ERR_NO_ACCESS;
            err_index = 1;
        }
        else
        {
            for (size_t i = 0; i < nvb; i++)
            {
                const SnmpMibEntry *en = mib_find_exact(g_in[i].oid, g_in[i].oid_len);
                if (!en)
                {
                    err_status = SNMP_ERR_NO_SUCH_NAME;
                    err_index = (long)(i + 1);
                    break;
                }
                if (!en->setter)
                {
                    err_status = (!v2c) ? SNMP_ERR_READ_ONLY : SNMP_ERR_NOT_WRITABLE;
                    err_index = (long)(i + 1);
                    break;
                }
                if (!en->setter(&g_in[i].val))
                {
                    err_status = (!v2c) ? SNMP_ERR_BAD_VALUE : SNMP_ERR_WRONG_TYPE;
                    err_index = (long)(i + 1);
                    break;
                }
            }
        }
    }
    else
    {
        return 0; // unsupported PDU (Trap, Response, etc.)
    }

    size_t n = encode_pdu(request_id, err_status, err_index, g_out, nout, out, out_cap);
    if (n == 0 && nout > 0)
    {
        // Response didn't fit: report tooBig with an empty varbind list.
        n = encode_pdu(request_id, SNMP_ERR_TOO_BIG, 0, g_out, 0, out, out_cap);
    }
    return n;
}

// ---------------------------------------------------------------------------
// Message wrapper: v1/v2c community framing (v3 delegates to the USM layer)
// ---------------------------------------------------------------------------

size_t snmp_agent_process(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap)
{
    BerDec d;
    ber_dec_init(&d, req, req_len);

    uint8_t tag;
    size_t len;
    if (!ber_read_header(&d, &tag, &len) || tag != BER_SEQUENCE) // message wrapper
        return 0;

    long version;
    if (!ber_read_integer(&d, &version))
        return 0;

    if (version == SNMP_V3)
    {
#if DETWS_ENABLE_SNMP_V3
        return snmp_v3_process(req, req_len, resp, resp_cap);
#else
        return 0; // v3 needs the gated USM layer (DETWS_ENABLE_SNMP_V3)
#endif
    }
    if (version != SNMP_V1 && version != SNMP_V2C)
        return 0;

    uint8_t ctag;
    size_t clen;
    if (!ber_read_header(&d, &ctag, &clen) || ctag != BER_OCTET_STRING)
        return 0;
    const char *community = (const char *)(d.buf + d.pos);
    size_t community_len = clen;
    d.pos += clen;

    bool is_rw = g_rw_set && community_eq(g_rw, community, community_len);
    bool is_ro = is_rw || community_eq(g_ro, community, community_len);
    if (!is_ro) // unknown community: silently drop, like a standard agent
        return 0;

    // The PDU is the remaining bytes of the datagram; dispatch and re-wrap.
    static uint8_t pdubuf[SNMP_MSG_BUF_SIZE];
    size_t pn = snmp_dispatch_pdu(req + d.pos, req_len - d.pos, is_rw, version == SNMP_V2C, pdubuf, sizeof(pdubuf));
    if (pn == 0)
        return 0;

    BerEnc e;
    ber_enc_init(&e, resp, resp_cap);
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, version);
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)community, community_len);
    ber_put_raw(&e, pdubuf, pn);
    ber_seq_end(&e, msg);
    return e.ok ? e.len : 0;
}

// ---------------------------------------------------------------------------
// UDP transport (ESP32 only)
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "lwip/pbuf.h"
#include "lwip/udp.h"

static struct udp_pcb *g_snmp_pcb = nullptr;
static uint8_t g_snmp_rx[SNMP_MSG_BUF_SIZE];
static uint8_t g_snmp_tx[SNMP_MSG_BUF_SIZE];

static void snmp_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg;
    if (!p)
        return;
    u16_t n = (p->tot_len < sizeof(g_snmp_rx)) ? p->tot_len : (u16_t)sizeof(g_snmp_rx);
    pbuf_copy_partial(p, g_snmp_rx, n, 0);
    pbuf_free(p);

    size_t rn = snmp_agent_process(g_snmp_rx, n, g_snmp_tx, sizeof(g_snmp_tx));
    if (rn == 0)
        return;

    struct pbuf *out = pbuf_alloc(PBUF_TRANSPORT, (u16_t)rn, PBUF_RAM);
    if (out)
    {
        memcpy(out->payload, g_snmp_tx, rn);
        udp_sendto(pcb, out, addr, port);
        pbuf_free(out);
    }
}

void snmp_agent_begin_udp(uint16_t port)
{
    if (g_snmp_pcb)
        return;
    g_snmp_pcb = udp_new();
    if (g_snmp_pcb)
    {
        udp_bind(g_snmp_pcb, IP_ANY_TYPE, port);
        udp_recv(g_snmp_pcb, snmp_udp_recv, nullptr);
    }
}

#else // non-Arduino: the core stays host-testable; no socket layer

void snmp_agent_begin_udp(uint16_t port)
{
    (void)port;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_SNMP
