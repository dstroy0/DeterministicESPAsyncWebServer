// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_v3.cpp
 * @brief SNMPv3 USM: message framing, engine discovery, timeliness, auth, privacy.
 */

#include "services/snmp/snmp_v3.h"

#if DETWS_ENABLE_SNMP_V3

#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include "services/snmp/snmp_agent.h"
#include "services/snmp/snmp_ber.h"
#include "services/snmp/snmp_crypto.h"
#include <string.h>

#if defined(ARDUINO)
#include <Arduino.h>
static uint32_t snmp_v3_uptime_s()
{
    return (uint32_t)(millis() / 1000ULL);
}
#else
static uint32_t snmp_v3_uptime_s()
{
    return 0; // host build: tests drive boots/time via the discovery handshake
}
#endif

// usmStats... subtree: 1.3.6.1.6.3.15.1.1.<n>.0
static const uint32_t kUsmStatsBase[] = {1, 3, 6, 1, 6, 3, 15, 1, 1};
enum UsmStat
{
    USM_STAT_NOT_IN_TIME = 2,
    USM_STAT_UNKNOWN_USER = 3,
    USM_STAT_UNKNOWN_ENGINE = 4,
    USM_STAT_WRONG_DIGEST = 5,
    USM_STAT_DECRYPT = 6,
};

// ---------------------------------------------------------------------------
// Engine / user state
// ---------------------------------------------------------------------------

// All SNMPv3 USM engine state, owned by one instance (internal linkage): the engine id/boots,
// the configured user + localized auth/priv keys, the USM stats counters, and the per-request
// working buffers (staggered lifetimes; see snmp_v3_process), grouped so it is one named owner,
// unreachable cross-TU. Single-threaded (the lwIP callback or a test, never reentrant).
struct SnmpV3Ctx
{
    uint8_t engine_id[SNMP_V3_ENGINEID_MAX] = {0x80, 0x00, 0xC0, 0xDE, 0x05, 0x01, 0x02, 0x03, 0x04};
    size_t engine_id_len = 9;
    uint32_t boots = 1;

    char user[SNMP_V3_USER_MAX] = "";
    uint8_t auth_key[SNMP_USM_KEY_LEN];
    uint8_t priv_key[SNMP_USM_KEY_LEN];
    bool auth_set = false;
    bool priv_set = false;
    uint32_t salt_ctr = 0;

    // USM stats counters (reported in discovery / error Reports)
    uint32_t stat_unknown_engine = 0;
    uint32_t stat_unknown_user = 0;
    uint32_t stat_wrong_digest = 0;
    uint32_t stat_not_in_time = 0;
    uint32_t stat_decrypt = 0;

    // Working buffers; lifetimes staggered so they never alias within a single request.
    uint8_t v3_a[SNMP_MSG_BUF_SIZE]; // auth-verify copy / decrypted scopedPDU
    uint8_t v3_b[SNMP_MSG_BUF_SIZE]; // inner response PDU
    uint8_t v3_c[SNMP_MSG_BUF_SIZE]; // outgoing scopedPDU
    uint8_t v3_d[SNMP_MSG_BUF_SIZE]; // privacy ciphertext
    uint8_t v3_sec[256];             // msgSecurityParameters scratch
};
static SnmpV3Ctx s_v3;

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void snmp_v3_init(const uint8_t *engine_id, size_t engine_id_len)
{
    if (engine_id && engine_id_len >= 5 && engine_id_len <= SNMP_V3_ENGINEID_MAX)
    {
        memcpy(s_v3.engine_id, engine_id, engine_id_len);
        s_v3.engine_id_len = engine_id_len;
    }
    s_v3.user[0] = '\0';
    s_v3.auth_set = false;
    s_v3.priv_set = false;
}

void snmp_v3_set_user(const char *user, const char *auth_pass, const char *priv_pass)
{
    strncpy(s_v3.user, user ? user : "", sizeof(s_v3.user) - 1);
    s_v3.user[sizeof(s_v3.user) - 1] = '\0';
    s_v3.auth_set = auth_pass && auth_pass[0];
    s_v3.priv_set = priv_pass && priv_pass[0];
    if (s_v3.auth_set)
        snmp_usm_localize_key(auth_pass, s_v3.engine_id, s_v3.engine_id_len, s_v3.auth_key);
    if (s_v3.priv_set)
        snmp_usm_localize_key(priv_pass, s_v3.engine_id, s_v3.engine_id_len, s_v3.priv_key);
}

void snmp_v3_set_boots(uint32_t boots)
{
    s_v3.boots = boots;
}
uint32_t snmp_v3_get_boots()
{
    return s_v3.boots;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool ct_eq(const uint8_t *a, const uint8_t *b, size_t n)
{
    uint8_t r = 0;
    for (size_t i = 0; i < n; i++)
        r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

// Split a scopedPDU SEQUENCE into its contextName and inner PDU TLV.
static bool parse_scoped(const uint8_t *buf, size_t len, const uint8_t **ctxname, size_t *ctxname_len,
                         const uint8_t **pdu, size_t *pdu_len)
{
    BerDec d;
    ber_dec_init(&d, buf, len);
    uint8_t tag;
    size_t l;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE)
        return false;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_OCTET_STRING) // contextEngineID
        return false;
    d.pos += l;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_OCTET_STRING) // contextName
        return false;
    *ctxname = d.buf + d.pos;
    *ctxname_len = l;
    d.pos += l;
    size_t pdu_start = d.pos;
    if (!ber_read_header(&d, &tag, &l)) // PDU header (kept)
        return false;
    *pdu = buf + pdu_start;
    *pdu_len = (size_t)(d.pos - pdu_start) + l;
    return d.ok;
}

// Best-effort read of the inner request-id from a plaintext probe (for Reports).
static long inner_request_id(const uint8_t *mdata, size_t mdata_len, bool priv)
{
    if (priv)
        return 0;
    const uint8_t *ctxname, *pdu;
    size_t ctxname_len, pdu_len;
    if (!parse_scoped(mdata, mdata_len, &ctxname, &ctxname_len, &pdu, &pdu_len))
        return 0;
    BerDec d;
    ber_dec_init(&d, pdu, pdu_len);
    uint8_t tag;
    size_t l;
    long rid;
    if (!ber_read_header(&d, &tag, &l) || !ber_read_integer(&d, &rid))
        return 0;
    return rid;
}

// Build a complete v3 message wrapping the already-built scopedPDU bytes.
static size_t build_message(long msg_id, bool auth, bool priv, const uint8_t *scoped, size_t scoped_len, uint8_t *resp,
                            size_t resp_cap)
{
    uint32_t now = snmp_v3_uptime_s();
    const uint8_t *data_ptr = scoped;
    size_t data_len = scoped_len;
    uint8_t salt[SNMP_V3_PRIV_PARAM_LEN];

    if (priv)
    {
        put_be32(salt, s_v3.boots);
        put_be32(salt + 4, ++s_v3.salt_ctr);
        uint8_t iv[16];
        put_be32(iv, s_v3.boots);
        put_be32(iv + 4, now);
        memcpy(iv + 8, salt, SNMP_V3_PRIV_PARAM_LEN);
        if (scoped_len > sizeof(s_v3.v3_d))
            return 0; // GCOVR_EXCL_LINE  scoped is built in v3_c, the same size (SNMP_MSG_BUF_SIZE) as v3_d
        snmp_aes128_cfb(s_v3.priv_key, iv, scoped, s_v3.v3_d, scoped_len, true);
        data_ptr = s_v3.v3_d;
    }

    // msgSecurityParameters (a SEQUENCE, later wrapped in an OCTET STRING).
    BerEnc se;
    ber_enc_init(&se, s_v3.v3_sec, sizeof(s_v3.v3_sec));
    size_t ss = ber_seq_begin(&se, BER_SEQUENCE);
    ber_put_octet_string(&se, BER_OCTET_STRING, s_v3.engine_id, s_v3.engine_id_len);
    ber_put_integer(&se, (long)s_v3.boots);
    ber_put_integer(&se, (long)now);
    ber_put_octet_string(&se, BER_OCTET_STRING, (const uint8_t *)s_v3.user, strnlen(s_v3.user, SNMP_V3_USER_MAX));
    size_t auth_off = 0;
    if (auth)
    {
        auth_off = se.len + 2; // value follows tag + 1-byte length (24 < 128)
        uint8_t zeros[SNMP_V3_AUTH_PARAM_LEN];
        memset(zeros, 0, sizeof(zeros));
        ber_put_octet_string(&se, BER_OCTET_STRING, zeros, SNMP_V3_AUTH_PARAM_LEN);
    }
    else
    {
        ber_put_octet_string(&se, BER_OCTET_STRING, nullptr, 0);
    }
    if (priv)
        ber_put_octet_string(&se, BER_OCTET_STRING, salt, SNMP_V3_PRIV_PARAM_LEN);
    else
        ber_put_octet_string(&se, BER_OCTET_STRING, nullptr, 0);
    ber_seq_end(&se, ss);
    if (!se.ok)
        return 0; // GCOVR_EXCL_LINE  secParams (<=~120B: 32B engineID + 32B user + 24B auth + ints) never overflow
                  // v3_sec[256]
    size_t sec_len = se.len;

    // Full message.
    BerEnc e;
    ber_enc_init(&e, resp, resp_cap);
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, SNMP_V3);
    size_t hdr = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, msg_id);
    ber_put_integer(&e, 65507); // msgMaxSize
    uint8_t fl = (uint8_t)((auth ? 0x01 : 0) | (priv ? 0x02 : 0));
    ber_put_octet_string(&e, BER_OCTET_STRING, &fl, 1);
    ber_put_integer(&e, 3); // msgSecurityModel = USM
    ber_seq_end(&e, hdr);
    ber_put_octet_string(&e, BER_OCTET_STRING, s_v3.v3_sec, sec_len);
    size_t sec_value_pos = e.len - sec_len;
    if (priv)
        ber_put_octet_string(&e, BER_OCTET_STRING, data_ptr, data_len);
    else
        ber_put_raw(&e, data_ptr, data_len);
    ber_seq_end(&e, msg);
    if (!e.ok)
        return 0;
    size_t total = e.len;

    if (auth)
    {
        uint8_t mac[SSH_HMAC_SHA256_LEN];
        ssh_hmac_sha256(s_v3.auth_key, SNMP_USM_KEY_LEN, resp, total, mac);
        memcpy(resp + sec_value_pos + auth_off, mac, SNMP_V3_AUTH_PARAM_LEN);
    }
    return total;
}

// Build a Report PDU (usmStats<stat>.0 = Counter32) and wrap it in a v3 message.
static size_t build_report(long msg_id, bool auth, uint32_t stat, uint32_t count, long request_id, uint8_t *resp,
                           size_t resp_cap)
{
    uint32_t oid[11];
    for (int i = 0; i < 9; i++)
        oid[i] = kUsmStatsBase[i];
    oid[9] = stat;
    oid[10] = 0;

    BerEnc e;
    ber_enc_init(&e, s_v3.v3_b, sizeof(s_v3.v3_b));
    size_t pdu = ber_seq_begin(&e, SNMP_PDU_REPORT);
    ber_put_integer(&e, request_id);
    ber_put_integer(&e, 0);
    ber_put_integer(&e, 0);
    size_t vbl = ber_seq_begin(&e, BER_SEQUENCE);
    size_t vb = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_oid(&e, oid, 11);
    ber_put_uint(&e, SNMP_COUNTER32, count);
    ber_seq_end(&e, vb);
    ber_seq_end(&e, vbl);
    ber_seq_end(&e, pdu);
    if (!e.ok)
        return 0; // GCOVR_EXCL_LINE  the Report is one fixed usmStats varbind, far under v3_b (SNMP_MSG_BUF_SIZE)

    BerEnc sc;
    ber_enc_init(&sc, s_v3.v3_c, sizeof(s_v3.v3_c));
    size_t s = ber_seq_begin(&sc, BER_SEQUENCE);
    ber_put_octet_string(&sc, BER_OCTET_STRING, s_v3.engine_id, s_v3.engine_id_len);
    ber_put_octet_string(&sc, BER_OCTET_STRING, nullptr, 0);
    ber_put_raw(&sc, s_v3.v3_b, e.len);
    ber_seq_end(&sc, s);
    if (!sc.ok)
        return 0; // GCOVR_EXCL_LINE  fixed tiny Report scopedPDU never overflows v3_c (SNMP_MSG_BUF_SIZE)

    return build_message(msg_id, auth, false, s_v3.v3_c, sc.len, resp, resp_cap);
}

// ---------------------------------------------------------------------------
// Message processing
// ---------------------------------------------------------------------------

size_t snmp_v3_process(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap)
{
    BerDec d;
    ber_dec_init(&d, req, req_len);
    uint8_t tag;
    size_t l;
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE)
        return 0;
    long version;
    if (!ber_read_integer(&d, &version) || version != SNMP_V3)
        return 0;

    // msgGlobalData
    if (!ber_read_header(&d, &tag, &l) || tag != BER_SEQUENCE)
        return 0;
    long msg_id, msg_max, sec_model;
    if (!ber_read_integer(&d, &msg_id) || !ber_read_integer(&d, &msg_max))
        return 0;
    (void)msg_max;
    size_t flags_len;
    if (!ber_read_header(&d, &tag, &flags_len) || tag != BER_OCTET_STRING || flags_len < 1)
        return 0;
    uint8_t flags = d.buf[d.pos];
    d.pos += flags_len;
    if (!ber_read_integer(&d, &sec_model) || sec_model != 3) // USM only
        return 0;
    bool req_auth = flags & 0x01;
    bool req_priv = flags & 0x02;
    if (req_priv && !req_auth)
        return 0;

    // msgSecurityParameters: OCTET STRING wrapping a SEQUENCE.
    size_t sec_len;
    if (!ber_read_header(&d, &tag, &sec_len) || tag != BER_OCTET_STRING)
        return 0;
    const uint8_t *sec = d.buf + d.pos;
    d.pos += sec_len;

    BerDec sd;
    ber_dec_init(&sd, sec, sec_len);
    if (!ber_read_header(&sd, &tag, &l) || tag != BER_SEQUENCE)
        return 0;
    size_t eid_len;
    if (!ber_read_header(&sd, &tag, &eid_len) || tag != BER_OCTET_STRING)
        return 0;
    const uint8_t *eid = sd.buf + sd.pos;
    sd.pos += eid_len;
    long req_boots, req_time;
    if (!ber_read_integer(&sd, &req_boots) || !ber_read_integer(&sd, &req_time))
        return 0;
    size_t uname_len;
    if (!ber_read_header(&sd, &tag, &uname_len) || tag != BER_OCTET_STRING)
        return 0;
    const uint8_t *uname = sd.buf + sd.pos;
    sd.pos += uname_len;
    size_t aparm_len;
    if (!ber_read_header(&sd, &tag, &aparm_len) || tag != BER_OCTET_STRING)
        return 0;
    const uint8_t *aparm = sd.buf + sd.pos;
    size_t aparm_off = (size_t)(aparm - req);
    sd.pos += aparm_len;
    size_t pparm_len;
    if (!ber_read_header(&sd, &tag, &pparm_len) || tag != BER_OCTET_STRING)
        return 0;
    const uint8_t *pparm = sd.buf + sd.pos;
    sd.pos += pparm_len;
    if (!sd.ok)
        return 0; // GCOVR_EXCL_LINE  redundant: every secParams field read above returns on failure, so sd.ok holds
                  // here

    const uint8_t *mdata = d.buf + d.pos;
    size_t mdata_len = req_len - d.pos;

    // Engine discovery: unknown/empty authoritative engine ID.
    bool engine_match = (eid_len == s_v3.engine_id_len) && (memcmp(eid, s_v3.engine_id, eid_len) == 0);
    if (!engine_match)
    {
        s_v3.stat_unknown_engine++;
        return build_report(msg_id, false, USM_STAT_UNKNOWN_ENGINE, s_v3.stat_unknown_engine,
                            inner_request_id(mdata, mdata_len, req_priv), resp, resp_cap);
    }

    if (!req_auth) // non-discovery noAuthNoPriv is not supported
        return 0;

    // Known user?
    if (!s_v3.auth_set || !(uname_len == strnlen(s_v3.user, SNMP_V3_USER_MAX) && s_v3.user[0] &&
                            memcmp(uname, s_v3.user, uname_len) == 0))
    {
        s_v3.stat_unknown_user++;
        return build_report(msg_id, false, USM_STAT_UNKNOWN_USER, s_v3.stat_unknown_user, 0, resp, resp_cap);
    }

    // Authenticate: HMAC over the whole message with the auth field zeroed.
    if (aparm_len != SNMP_V3_AUTH_PARAM_LEN || req_len > sizeof(s_v3.v3_a))
    {
        s_v3.stat_wrong_digest++;
        return build_report(msg_id, false, USM_STAT_WRONG_DIGEST, s_v3.stat_wrong_digest, 0, resp, resp_cap);
    }
    memcpy(s_v3.v3_a, req, req_len);
    memset(s_v3.v3_a + aparm_off, 0, SNMP_V3_AUTH_PARAM_LEN);
    uint8_t mac[SSH_HMAC_SHA256_LEN];
    ssh_hmac_sha256(s_v3.auth_key, SNMP_USM_KEY_LEN, s_v3.v3_a, req_len, mac);
    if (!ct_eq(mac, aparm, SNMP_V3_AUTH_PARAM_LEN))
    {
        s_v3.stat_wrong_digest++;
        return build_report(msg_id, false, USM_STAT_WRONG_DIGEST, s_v3.stat_wrong_digest, 0, resp, resp_cap);
    }

    // Timeliness window (RFC 3414 §3.2): boots must match, time within +/-150s.
    uint32_t now = snmp_v3_uptime_s();
    long dt = (long)now - req_time;
    if (dt < 0)
        dt = -dt;
    if ((uint32_t)req_boots != s_v3.boots || dt > 150)
    {
        s_v3.stat_not_in_time++;
        return build_report(msg_id, true, USM_STAT_NOT_IN_TIME, s_v3.stat_not_in_time, 0, resp, resp_cap);
    }

    // Privacy: decrypt the scopedPDU if the priv flag is set.
    const uint8_t *scoped;
    size_t scoped_len;
    if (req_priv)
    {
        if (!s_v3.priv_set || pparm_len != SNMP_V3_PRIV_PARAM_LEN)
        {
            s_v3.stat_decrypt++;
            return build_report(msg_id, true, USM_STAT_DECRYPT, s_v3.stat_decrypt, 0, resp, resp_cap);
        }
        BerDec md;
        ber_dec_init(&md, mdata, mdata_len);
        if (!ber_read_header(&md, &tag, &l) || tag != BER_OCTET_STRING)
            return 0;
        const uint8_t *ct = md.buf + md.pos;
        size_t ct_len = l;
        if (ct_len > sizeof(s_v3.v3_a))
            return 0; // GCOVR_EXCL_LINE  ct_len <= mdata_len < req_len, and req_len<=sizeof(v3_a) was enforced at the
                      // digest step
        uint8_t iv[16];
        put_be32(iv, (uint32_t)req_boots);
        put_be32(iv + 4, (uint32_t)req_time);
        memcpy(iv + 8, pparm, SNMP_V3_PRIV_PARAM_LEN);
        snmp_aes128_cfb(s_v3.priv_key, iv, ct, s_v3.v3_a, ct_len, false);
        scoped = s_v3.v3_a;
        scoped_len = ct_len;
    }
    else
    {
        scoped = mdata;
        scoped_len = mdata_len;
    }

    // Inner PDU -> dispatch (authenticated: writes allowed; v2c-style semantics).
    const uint8_t *ctxname, *pdu;
    size_t ctxname_len, pdu_len;
    if (!parse_scoped(scoped, scoped_len, &ctxname, &ctxname_len, &pdu, &pdu_len))
        return 0;
    size_t rpdu = snmp_dispatch_pdu(pdu, pdu_len, true, true, s_v3.v3_b, sizeof(s_v3.v3_b));
    if (rpdu == 0)
        return 0;

    // Response scopedPDU { our engineID, echoed contextName, response PDU }.
    BerEnc sc;
    ber_enc_init(&sc, s_v3.v3_c, sizeof(s_v3.v3_c));
    size_t s = ber_seq_begin(&sc, BER_SEQUENCE);
    ber_put_octet_string(&sc, BER_OCTET_STRING, s_v3.engine_id, s_v3.engine_id_len);
    ber_put_octet_string(&sc, BER_OCTET_STRING, ctxname, ctxname_len);
    ber_put_raw(&sc, s_v3.v3_b, rpdu);
    ber_seq_end(&sc, s);
    if (!sc.ok)
        return 0;

    return build_message(msg_id, true, req_priv, s_v3.v3_c, sc.len, resp, resp_cap);
}

#if DETWS_ENABLE_SNMP_TRAP
#include "network_drivers/transport/udp.h"
#include "services/snmp/snmp_notify.h"

// Shared SNMPv3 USM notification path: authenticated, and encrypted when a
// privacy password is configured. Reuses the engine ID + localized keys from
// snmp_v3_init()/snmp_v3_set_user() and the same build_message() as responses.
// @p pdu_tag selects Trap (SNMP_PDU_TRAPV2) vs InformRequest (0xA6); @p request_id
// is the inner PDU request-id (the inform receiver echoes it in its Response).
static bool send_v3_notify(const char *dst_ip, uint16_t port, uint8_t pdu_tag, uint32_t request_id,
                           const uint32_t *trap_oid, size_t trap_oid_len, const SnmpVarbind *vbs, size_t n)
{
    if (!s_v3.auth_set) // a v3 notification must be authenticated
        return false;

    // Notification PDU into the inner-PDU scratch.
    BerEnc e;
    ber_enc_init(&e, s_v3.v3_b, sizeof(s_v3.v3_b));
    snmp_notify_build_pdu(&e, pdu_tag, request_id, trap_oid, trap_oid_len, snmp_v3_uptime_s() * 100, vbs, n);
    if (!e.ok)
        return false;

    // scopedPDU { contextEngineID = our engine, contextName = "", PDU }.
    BerEnc sc;
    ber_enc_init(&sc, s_v3.v3_c, sizeof(s_v3.v3_c));
    size_t s = ber_seq_begin(&sc, BER_SEQUENCE);
    ber_put_octet_string(&sc, BER_OCTET_STRING, s_v3.engine_id, s_v3.engine_id_len);
    ber_put_octet_string(&sc, BER_OCTET_STRING, nullptr, 0);
    ber_put_raw(&sc, s_v3.v3_b, e.len);
    ber_seq_end(&sc, s);
    if (!sc.ok)
        return false;

    uint8_t out[SNMP_MSG_BUF_SIZE];
    size_t len = build_message((long)request_id, s_v3.auth_set, s_v3.priv_set, s_v3.v3_c, sc.len, out, sizeof(out));
    return len && det_udp_sendto(dst_ip, port, out, len);
}

bool snmp_trap_v3(const char *dst_ip, uint16_t port, const uint32_t *trap_oid, size_t trap_oid_len,
                  const SnmpVarbind *vbs, size_t n)
{
    static uint32_t s_v3_trap_id = 1; // a trap is unconfirmed; the id is informational
    return send_v3_notify(dst_ip, port, SNMP_PDU_TRAPV2, s_v3_trap_id++, trap_oid, trap_oid_len, vbs, n);
}

// SNMPv3 USM InformRequest (confirmed notification, RFC 3416 4.2.7). Symmetric to
// snmp_inform_v2c: builds + sends the InformRequest. The caller owns the
// request_id and, for confirmed delivery, retransmits until the receiver's
// Response (echoing request_id) arrives.
bool snmp_inform_v3(const char *dst_ip, uint16_t port, uint32_t request_id, const uint32_t *trap_oid,
                    size_t trap_oid_len, const SnmpVarbind *vbs, size_t n)
{
    return send_v3_notify(dst_ip, port, 0xA6 /* InformRequest */, request_id, trap_oid, trap_oid_len, vbs, n);
}
#endif // DETWS_ENABLE_SNMP_TRAP

#endif // DETWS_ENABLE_SNMP_V3
