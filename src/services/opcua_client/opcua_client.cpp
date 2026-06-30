// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file opcua_client.cpp
 * @brief OPC UA Binary client - request builders + response parsers (implementation).
 *
 * Pure byte-buffer logic reusing the opcua.h codec; no transport, no heap, no stdlib.
 */

#include "services/opcua_client/opcua_client.h"

#if DETWS_ENABLE_OPCUA_CLIENT

void opcua_client_init(OpcUaClient *c)
{
    memset(c, 0, sizeof(*c));
}

// ---------------------------------------------------------------------------
// Builder helpers
// ---------------------------------------------------------------------------
static size_t cw_patch(UaWriter *w)
{
    if (!w->ok)
        return 0;
    w->o[4] = (uint8_t)w->n;
    w->o[5] = (uint8_t)(w->n >> 8);
    w->o[6] = (uint8_t)(w->n >> 16);
    w->o[7] = (uint8_t)(w->n >> 24);
    return w->n;
}

// RequestHeader: the session AuthenticationToken (if available and requested) or a
// null NodeId, then the fixed trailer. RequestHandle increments per request.
static void cw_request_header(OpcUaClient *c, UaWriter *w, bool with_session_token)
{
    if (with_session_token && c->session_auth_id)
        ua_w_nodeid_numeric(w, c->session_auth_ns, c->session_auth_id);
    else
        ua_w_nodeid_numeric(w, 0, 0);
    ua_w_u64(w, 0);                   // Timestamp (unset)
    ua_w_u32(w, ++c->request_handle); // RequestHandle
    ua_w_u32(w, 0);                   // ReturnDiagnostics
    ua_w_string(w, nullptr, -1);      // AuditEntryId
    ua_w_u32(w, 0);                   // TimeoutHint
    ua_w_nodeid_numeric(w, 0, 0);     // AdditionalHeader NodeId (null)
    ua_w_u8(w, 0x00);                 // ... ExtensionObject "no body"
}

// MSG envelope prefix: header (size placeholder) + SymmetricSecurityHeader (TokenId)
// + SequenceHeader (SequenceNumber, RequestId) + body TypeId.
static void cw_msg(OpcUaClient *c, UaWriter *w, uint32_t type_id)
{
    ua_w_u8(w, 'M');
    ua_w_u8(w, 'S');
    ua_w_u8(w, 'G');
    ua_w_u8(w, 'F');
    ua_w_u32(w, 0);               // size placeholder
    ua_w_u32(w, c->channel_id);   // SecureChannelId
    ua_w_u32(w, c->token_id);     // SymmetricSecurityHeader.TokenId
    ua_w_u32(w, ++c->seq);        // SequenceNumber
    ua_w_u32(w, ++c->request_id); // RequestId
    ua_w_nodeid_numeric(w, 0, type_id);
}

// ---------------------------------------------------------------------------
// Request builders
// ---------------------------------------------------------------------------
size_t opcua_client_hello(const char *endpoint_url, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'H');
    ua_w_u8(&w, 'E');
    ua_w_u8(&w, 'L');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0);               // size placeholder
    ua_w_u32(&w, 0);               // ProtocolVersion
    ua_w_u32(&w, DETWS_OPCUA_BUF); // ReceiveBufferSize
    ua_w_u32(&w, DETWS_OPCUA_BUF); // SendBufferSize
    ua_w_u32(&w, 0);               // MaxMessageSize (no limit)
    ua_w_u32(&w, 0);               // MaxChunkCount
    ua_w_string(&w, endpoint_url, endpoint_url ? (int32_t)strlen(endpoint_url) : -1);
    return cw_patch(&w);
}

size_t opcua_client_open(OpcUaClient *c, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'P');
    ua_w_u8(&w, 'N');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder
    ua_w_u32(&w, 0); // SecureChannelId (0 = request a new channel)
    ua_w_string(&w, OPCUA_POLICY_NONE_URI, (int32_t)strlen(OPCUA_POLICY_NONE_URI));
    ua_w_string(&w, nullptr, -1);  // SenderCertificate
    ua_w_string(&w, nullptr, -1);  // ReceiverCertificateThumbprint
    ua_w_u32(&w, ++c->seq);        // SequenceNumber
    ua_w_u32(&w, ++c->request_id); // RequestId
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_REQ);
    cw_request_header(c, &w, false);
    ua_w_u32(&w, 0);              // ClientProtocolVersion
    ua_w_u32(&w, 0);              // RequestType = Issue
    ua_w_u32(&w, 1);              // MessageSecurityMode = None
    ua_w_string(&w, nullptr, -1); // ClientNonce
    ua_w_u32(&w, 3600000);        // RequestedLifetime (ms)
    return cw_patch(&w);
}

size_t opcua_client_get_endpoints(OpcUaClient *c, const char *endpoint_url, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_GET_ENDPOINTS_REQ);
    cw_request_header(c, &w, false);                                                  // GetEndpoints needs no session
    ua_w_string(&w, endpoint_url, endpoint_url ? (int32_t)strlen(endpoint_url) : -1); // EndpointUrl
    ua_w_i32(&w, -1);                                                                 // LocaleIds[] (null)
    ua_w_i32(&w, -1);                                                                 // ProfileUris[] (null)
    return cw_patch(&w);
}

size_t opcua_client_create_session(OpcUaClient *c, const char *session_name, const char *endpoint_url, uint8_t *out,
                                   size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_CREATE_SESSION_REQ);
    cw_request_header(c, &w, false);
    // ClientDescription (ApplicationDescription).
    ua_w_string(&w, "urn:det:opcua:client", 20);                                      // ApplicationUri
    ua_w_string(&w, "urn:det:opcua", 13);                                             // ProductUri
    ua_w_localizedtext(&w, nullptr, "DetOpcUaClient");                                // ApplicationName
    ua_w_u32(&w, 1);                                                                  // ApplicationType = Client
    ua_w_string(&w, nullptr, -1);                                                     // GatewayServerUri
    ua_w_string(&w, nullptr, -1);                                                     // DiscoveryProfileUri
    ua_w_i32(&w, -1);                                                                 // DiscoveryUrls[] (null)
    ua_w_string(&w, nullptr, -1);                                                     // ServerUri
    ua_w_string(&w, endpoint_url, endpoint_url ? (int32_t)strlen(endpoint_url) : -1); // EndpointUrl
    ua_w_string(&w, session_name, session_name ? (int32_t)strlen(session_name) : -1); // SessionName
    ua_w_string(&w, nullptr, -1);                                                     // ClientNonce (ByteString)
    ua_w_string(&w, nullptr, -1);                                                     // ClientCertificate
    ua_w_f64(&w, 1200000.0);                                                          // RequestedSessionTimeout
    ua_w_u32(&w, 0);                                                                  // MaxResponseMessageSize
    return cw_patch(&w);
}

size_t opcua_client_activate_session(OpcUaClient *c, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_ACTIVATE_SESSION_REQ);
    cw_request_header(c, &w, true);
    ua_w_string(&w, nullptr, -1); // ClientSignature.Algorithm
    ua_w_string(&w, nullptr, -1); // ClientSignature.Signature
    ua_w_i32(&w, -1);             // ClientSoftwareCertificates[] (null)
    ua_w_i32(&w, -1);             // LocaleIds[] (null)
    // UserIdentityToken: ExtensionObject(AnonymousIdentityToken i=321) with a String PolicyId body.
    ua_w_nodeid_numeric(&w, 0, 321); // AnonymousIdentityToken_Encoding_DefaultBinary
    ua_w_u8(&w, 0x01);               // ExtensionObject body = ByteString
    ua_w_i32(&w, 4 + 9);             // body length = encoded String "anonymous" (4-byte len + 9 chars)
    ua_w_string(&w, "anonymous", 9); // PolicyId
    ua_w_string(&w, nullptr, -1);    // UserTokenSignature.Algorithm
    ua_w_string(&w, nullptr, -1);    // UserTokenSignature.Signature
    return cw_patch(&w);
}

size_t opcua_client_read(OpcUaClient *c, const OpcUaReadItem *items, uint32_t n, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_READ_REQ);
    cw_request_header(c, &w, true);
    ua_w_f64(&w, 0.0);        // MaxAge
    ua_w_u32(&w, 0);          // TimestampsToReturn (Source)
    ua_w_i32(&w, (int32_t)n); // NodesToRead count
    for (uint32_t i = 0; i < n; i++)
    {
        ua_w_nodeid_numeric(&w, items[i].ns, items[i].id);
        ua_w_u32(&w, items[i].attribute);
        ua_w_string(&w, nullptr, -1); // IndexRange
        ua_w_u16(&w, 0);              // DataEncoding QualifiedName.ns
        ua_w_string(&w, nullptr, -1); // QualifiedName.name
    }
    return cw_patch(&w);
}

size_t opcua_client_browse(OpcUaClient *c, uint16_t ns, uint32_t id, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_BROWSE_REQ);
    cw_request_header(c, &w, true);
    ua_w_nodeid_numeric(&w, 0, 0);   // View.ViewId (null)
    ua_w_u64(&w, 0);                 // View.Timestamp
    ua_w_u32(&w, 0);                 // View.ViewVersion
    ua_w_u32(&w, 0);                 // RequestedMaxReferencesPerNode
    ua_w_i32(&w, 1);                 // NodesToBrowse count
    ua_w_nodeid_numeric(&w, ns, id); // BrowseDescription.NodeId
    ua_w_u32(&w, 0);                 // BrowseDirection (Forward)
    ua_w_nodeid_numeric(&w, 0, 0);   // ReferenceTypeId (null = all)
    ua_w_bool(&w, true);             // IncludeSubtypes
    ua_w_u32(&w, 0);                 // NodeClassMask
    ua_w_u32(&w, 0x3F);              // ResultMask
    return cw_patch(&w);
}

size_t opcua_client_write(OpcUaClient *c, const OpcUaWriteItem *items, uint32_t n, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_WRITE_REQ);
    cw_request_header(c, &w, true);
    ua_w_i32(&w, (int32_t)n); // NodesToWrite count
    for (uint32_t i = 0; i < n; i++)
    {
        ua_w_nodeid_numeric(&w, items[i].ns, items[i].id);      // NodeId
        ua_w_u32(&w, items[i].attribute);                       // AttributeId
        ua_w_string(&w, nullptr, -1);                           // IndexRange
        ua_w_datavalue(&w, &items[i].value, OPCUA_STATUS_GOOD); // Value (DataValue, value-only)
    }
    return cw_patch(&w);
}

size_t opcua_client_close_session(OpcUaClient *c, uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    cw_msg(c, &w, OPCUA_ID_CLOSE_SESSION_REQ);
    cw_request_header(c, &w, true);
    ua_w_bool(&w, true); // DeleteSubscriptions
    return cw_patch(&w);
}

size_t opcua_client_close_channel(uint8_t *out, size_t cap)
{
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'C');
    ua_w_u8(&w, 'L');
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder (patched to 8)
    return cw_patch(&w);
}

// ---------------------------------------------------------------------------
// Parser helpers
// ---------------------------------------------------------------------------
static void cr_skip(UaReader *r, size_t n)
{
    if (r->err || r->off + n > r->len)
    {
        r->err = true;
        return;
    }
    r->off += n;
}

static void cr_skip_string(UaReader *r)
{
    int32_t l = ua_r_i32(r);
    if (l > 0)
        cr_skip(r, (size_t)l);
}

static void cr_skip_string_array(UaReader *r)
{
    int32_t n = ua_r_i32(r);
    for (int32_t i = 0; i < n; i++)
        cr_skip_string(r);
}

// Consume a ResponseHeader (SecurityPolicy None encoding: empty diagnostics, no body
// AdditionalHeader). Captures the ServiceResult.
static bool cr_response_header(UaReader *r, uint32_t *service_result)
{
    (void)ua_r_u64(r); // Timestamp
    (void)ua_r_u32(r); // RequestHandle
    uint32_t svc = ua_r_u32(r);
    if (service_result)
        *service_result = svc;
    (void)ua_r_u8(r);        // ServiceDiagnostics (DiagnosticInfo, empty)
    cr_skip_string_array(r); // StringTable
    UaNodeId ah;
    ua_r_nodeid(r, &ah); // AdditionalHeader NodeId
    (void)ua_r_u8(r);    // AdditionalHeader ExtensionObject (no body)
    return !r->err;
}

// Open a MSG response: validate type + body TypeId, then consume the ResponseHeader.
static bool cr_msg_open(const uint8_t *msg, size_t len, UaReader *r, uint32_t expect_type, uint32_t *service_result)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "MSG", 3) != 0 || h.size != len)
        return false;
    *r = UaReader{msg + 8, len - 8, 0, false};
    (void)ua_r_u32(r); // SecureChannelId
    (void)ua_r_u32(r); // TokenId
    (void)ua_r_u32(r); // SequenceNumber
    (void)ua_r_u32(r); // RequestId
    UaNodeId tid;
    if (!ua_r_nodeid(r, &tid))
        return false;
    if (!(tid.numeric && tid.id == expect_type))
        return false;
    return cr_response_header(r, service_result);
}

// ---------------------------------------------------------------------------
// Response parsers
// ---------------------------------------------------------------------------
bool opcua_client_on_ack(const uint8_t *msg, size_t len, OpcUaAckInfo *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "ACK", 3) != 0)
        return false;
    if (h.size != len || len < 8 + 20)
        return false;
    UaReader r = {msg + 8, len - 8, 0, false};
    (void)ua_r_u32(&r); // ProtocolVersion
    out->recv_buf_size = ua_r_u32(&r);
    out->send_buf_size = ua_r_u32(&r);
    out->max_msg_size = ua_r_u32(&r);
    out->max_chunk_count = ua_r_u32(&r);
    return !r.err;
}

bool opcua_client_on_open(OpcUaClient *c, const uint8_t *msg, size_t len)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "OPN", 3) != 0 || h.size != len)
        return false;
    UaReader r = {msg + 8, len - 8, 0, false};
    (void)ua_r_u32(&r); // SecureChannelId (asymmetric security header)
    cr_skip_string(&r); // SecurityPolicyUri
    cr_skip_string(&r); // SenderCertificate
    cr_skip_string(&r); // ReceiverCertificateThumbprint
    (void)ua_r_u32(&r); // SequenceNumber
    (void)ua_r_u32(&r); // RequestId
    UaNodeId tid;
    if (!ua_r_nodeid(&r, &tid) || !(tid.numeric && tid.id == OPCUA_ID_OPEN_RESP))
        return false;
    uint32_t svc = 0;
    if (!cr_response_header(&r, &svc))
        return false;
    (void)ua_r_u32(&r);           // ServerProtocolVersion
    c->channel_id = ua_r_u32(&r); // SecurityToken.ChannelId
    c->token_id = ua_r_u32(&r);   // SecurityToken.TokenId
    (void)ua_r_u64(&r);           // CreatedAt
    (void)ua_r_u32(&r);           // RevisedLifetime
    return !r.err && svc == OPCUA_STATUS_GOOD;
}

int32_t opcua_client_on_get_endpoints(const uint8_t *msg, size_t len)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_GET_ENDPOINTS_RESP, &svc) || svc != OPCUA_STATUS_GOOD)
        return -1;
    int32_t cnt = ua_r_i32(&r); // Endpoints[] count
    return (cnt < 0 || r.err) ? -1 : cnt;
}

bool opcua_client_on_create_session(OpcUaClient *c, const uint8_t *msg, size_t len)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_CREATE_SESSION_RESP, &svc))
        return false;
    UaNodeId sid;
    ua_r_nodeid(&r, &sid); // SessionId
    UaNodeId atok;
    ua_r_nodeid(&r, &atok); // AuthenticationToken
    c->session_auth_ns = atok.ns;
    c->session_auth_id = atok.id;
    c->session_auth_numeric = atok.numeric;
    return !r.err && svc == OPCUA_STATUS_GOOD;
}

bool opcua_client_on_activate_session(const uint8_t *msg, size_t len)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_ACTIVATE_SESSION_RESP, &svc))
        return false;
    return svc == OPCUA_STATUS_GOOD;
}

int32_t opcua_client_on_read(const uint8_t *msg, size_t len, OpcUaVariant *vals, uint32_t *statuses, uint32_t max)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_READ_RESP, &svc) || svc != OPCUA_STATUS_GOOD)
        return -1;
    int32_t cnt = ua_r_i32(&r); // Results count
    if (cnt < 0)
        return -1;
    uint32_t out_n = 0;
    for (int32_t i = 0; i < cnt; i++)
    {
        uint8_t mask = ua_r_u8(&r);
        OpcUaVariant v;
        memset(&v, 0, sizeof(v));
        uint32_t st = OPCUA_STATUS_GOOD;
        if (mask & 0x01) // Value (Variant) present
        {
            uint8_t vt = ua_r_u8(&r);
            v.type = vt;
            switch (vt)
            {
            case OPCUA_VAR_BOOL:
                v.b = ua_r_bool(&r);
                break;
            case OPCUA_VAR_INT32:
                v.i32 = ua_r_i32(&r);
                break;
            case OPCUA_VAR_UINT32:
                v.u32 = ua_r_u32(&r);
                break;
            case OPCUA_VAR_FLOAT:
                v.f32 = ua_r_f32(&r);
                break;
            case OPCUA_VAR_DOUBLE:
                v.f64 = ua_r_f64(&r);
                break;
            case OPCUA_VAR_STRING: {
                int32_t sl = ua_r_i32(&r);
                v.str_len = sl;
                if (sl > 0)
                {
                    v.str = (const char *)(r.p + r.off); // points into msg
                    cr_skip(&r, (size_t)sl);
                }
                break;
            }
            default:
                r.err = true; // unsupported Variant type
                break;
            }
        }
        if (mask & 0x02) // StatusCode present
            st = ua_r_u32(&r);
        if (out_n < max)
        {
            if (vals)
                vals[out_n] = v;
            if (statuses)
                statuses[out_n] = st;
            out_n++;
        }
    }
    return r.err ? -1 : (int32_t)out_n;
}

int32_t opcua_client_on_write(const uint8_t *msg, size_t len, uint32_t *results, uint32_t max)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_WRITE_RESP, &svc) || svc != OPCUA_STATUS_GOOD)
        return -1;
    int32_t cnt = ua_r_i32(&r); // Results count
    if (cnt < 0)
        return -1;
    uint32_t out_n = 0;
    for (int32_t i = 0; i < cnt; i++)
    {
        uint32_t st = ua_r_u32(&r);
        if (out_n < max)
        {
            if (results)
                results[out_n] = st;
            out_n++;
        }
    }
    return r.err ? -1 : (int32_t)out_n;
}

int32_t opcua_client_on_browse(const uint8_t *msg, size_t len, OpcUaClientRef *refs, uint32_t max)
{
    UaReader r;
    uint32_t svc = 0;
    if (!cr_msg_open(msg, len, &r, OPCUA_ID_BROWSE_RESP, &svc) || svc != OPCUA_STATUS_GOOD)
        return -1;
    int32_t nres = ua_r_i32(&r); // Results count
    if (nres < 0)
        return -1;
    uint32_t out_n = 0;
    for (int32_t ri = 0; ri < nres; ri++)
    {
        (void)ua_r_u32(&r); // BrowseResult.StatusCode
        cr_skip_string(&r); // ContinuationPoint (ByteString)
        int32_t nrefs = ua_r_i32(&r);
        for (int32_t j = 0; j < nrefs; j++)
        {
            OpcUaClientRef ref;
            memset(&ref, 0, sizeof(ref));
            UaNodeId rt;
            ua_r_nodeid(&r, &rt);
            ref.ref_type_id = rt.id;
            ref.is_forward = ua_r_bool(&r);
            UaNodeId tgt;
            ua_r_nodeid(&r, &tgt);
            ref.target_ns = tgt.ns;
            ref.target_id = tgt.id;
            (void)ua_r_u16(&r); // BrowseName.NamespaceIndex
            int32_t bnl = 0;
            ua_r_string(&r, ref.browse_name, sizeof(ref.browse_name), &bnl);
            uint8_t m = ua_r_u8(&r); // DisplayName LocalizedText mask
            if (m & 0x01)
                cr_skip_string(&r); // Locale
            if (m & 0x02)
                cr_skip_string(&r); // Text
            ref.node_class = ua_r_u32(&r);
            UaNodeId td;
            ua_r_nodeid(&r, &td); // TypeDefinition
            if (out_n < max)
            {
                if (refs)
                    refs[out_n] = ref;
                out_n++;
            }
        }
    }
    return r.err ? -1 : (int32_t)out_n;
}

#endif // DETWS_ENABLE_OPCUA_CLIENT
