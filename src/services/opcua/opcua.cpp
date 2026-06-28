// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file opcua.cpp
 * @brief OPC UA Binary server: handshake + SecureChannel + Session + Read + Browse.
 *
 * Pure little-endian codec, handshake, OpenSecureChannel, CreateSession/
 * ActivateSession, Read (Variant/DataValue), Browse (ReferenceDescription) and
 * CloseSession logic; the ESP32 section pumps the PROTO_OPCUA rx ring and answers
 * HEL with ACK, OPN with an OpenSecureChannelResponse, the Session/Read/Browse MSG
 * calls, and closes on CLO (SecurityPolicy None). No heap, no stdlib.
 */

#include "services/opcua/opcua.h"

#if DETWS_ENABLE_OPCUA

#include <string.h>

// ---------------------------------------------------------------------------
// Built-in type codec
// ---------------------------------------------------------------------------
static void w_bytes(UaWriter *w, const void *src, size_t n)
{
    if (!w->ok || w->n + n > w->cap)
    {
        w->ok = false;
        return;
    }
    memcpy(w->o + w->n, src, n);
    w->n += n;
}

void ua_w_u8(UaWriter *w, uint8_t v)
{
    w_bytes(w, &v, 1);
}
void ua_w_u16(UaWriter *w, uint16_t v)
{
    uint8_t b[2] = {(uint8_t)v, (uint8_t)(v >> 8)};
    w_bytes(w, b, 2);
}
void ua_w_u32(UaWriter *w, uint32_t v)
{
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)};
    w_bytes(w, b, 4);
}
void ua_w_u64(UaWriter *w, uint64_t v)
{
    uint8_t b[8];
    for (int i = 0; i < 8; i++)
        b[i] = (uint8_t)(v >> (8 * i));
    w_bytes(w, b, 8);
}
void ua_w_i32(UaWriter *w, int32_t v)
{
    ua_w_u32(w, (uint32_t)v);
}
void ua_w_f32(UaWriter *w, float v)
{
    uint32_t u;
    memcpy(&u, &v, 4);
    ua_w_u32(w, u);
}
void ua_w_f64(UaWriter *w, double v)
{
    uint64_t u;
    memcpy(&u, &v, 8);
    ua_w_u64(w, u);
}
void ua_w_bool(UaWriter *w, bool v)
{
    ua_w_u8(w, v ? 1 : 0);
}
void ua_w_string(UaWriter *w, const char *s, int32_t len)
{
    ua_w_i32(w, len);
    if (len > 0 && s)
        w_bytes(w, s, (size_t)len);
}

static bool r_take(UaReader *r, void *dst, size_t n)
{
    if (r->err || r->off + n > r->len)
    {
        r->err = true;
        return false;
    }
    memcpy(dst, r->p + r->off, n);
    r->off += n;
    return true;
}

uint8_t ua_r_u8(UaReader *r)
{
    uint8_t v = 0;
    r_take(r, &v, 1);
    return v;
}
uint16_t ua_r_u16(UaReader *r)
{
    uint8_t b[2] = {0, 0};
    r_take(r, b, 2);
    return (uint16_t)(b[0] | (b[1] << 8));
}
uint32_t ua_r_u32(UaReader *r)
{
    uint8_t b[4] = {0, 0, 0, 0};
    r_take(r, b, 4);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
uint64_t ua_r_u64(UaReader *r)
{
    uint8_t b[8];
    if (!r_take(r, b, 8))
        return 0;
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
        v |= (uint64_t)b[i] << (8 * i);
    return v;
}
int32_t ua_r_i32(UaReader *r)
{
    return (int32_t)ua_r_u32(r);
}
float ua_r_f32(UaReader *r)
{
    uint32_t u = ua_r_u32(r);
    float v;
    memcpy(&v, &u, 4);
    return v;
}
double ua_r_f64(UaReader *r)
{
    uint64_t u = ua_r_u64(r);
    double v;
    memcpy(&v, &u, 8);
    return v;
}
bool ua_r_bool(UaReader *r)
{
    return ua_r_u8(r) != 0;
}
bool ua_r_string(UaReader *r, char *out, size_t cap, int32_t *out_len)
{
    int32_t len = ua_r_i32(r);
    if (r->err)
        return false;
    if (out_len)
        *out_len = len;
    if (len < 0) // null string
    {
        if (cap)
            out[0] = '\0';
        return true;
    }
    if ((size_t)len + 1 > cap || r->off + (size_t)len > r->len)
    {
        r->err = true;
        return false;
    }
    memcpy(out, r->p + r->off, (size_t)len);
    out[len] = '\0';
    r->off += (size_t)len;
    return true;
}

static void r_skip(UaReader *r, size_t n)
{
    if (r->err || r->off + n > r->len)
    {
        r->err = true;
        return;
    }
    r->off += n;
}

// ---------------------------------------------------------------------------
// NodeId / ExtensionObject / DateTime
// ---------------------------------------------------------------------------
void ua_w_nodeid_numeric(UaWriter *w, uint16_t ns, uint32_t id)
{
    if (ns == 0 && id <= 0xFF) // TwoByte
    {
        ua_w_u8(w, 0x00);
        ua_w_u8(w, (uint8_t)id);
    }
    else if (ns <= 0xFF && id <= 0xFFFF) // FourByte
    {
        ua_w_u8(w, 0x01);
        ua_w_u8(w, (uint8_t)ns);
        ua_w_u16(w, (uint16_t)id);
    }
    else // Numeric
    {
        ua_w_u8(w, 0x02);
        ua_w_u16(w, ns);
        ua_w_u32(w, id);
    }
}

bool ua_r_nodeid(UaReader *r, UaNodeId *out)
{
    uint8_t enc = ua_r_u8(r);
    uint8_t kind = enc & 0x0F; // strip the NamespaceUri (0x80) / ServerIndex (0x40) flags
    out->ns = 0;
    out->id = 0;
    out->numeric = true;
    switch (kind)
    {
    case 0x00: // TwoByte
        out->id = ua_r_u8(r);
        break;
    case 0x01: // FourByte
        out->ns = ua_r_u8(r);
        out->id = ua_r_u16(r);
        break;
    case 0x02: // Numeric
        out->ns = ua_r_u16(r);
        out->id = ua_r_u32(r);
        break;
    case 0x03: // String
    case 0x05: // ByteString
    {
        out->ns = ua_r_u16(r);
        out->numeric = false;
        int32_t l = ua_r_i32(r);
        if (l > 0)
            r_skip(r, (size_t)l);
        break;
    }
    case 0x04: // Guid
        out->ns = ua_r_u16(r);
        out->numeric = false;
        r_skip(r, 16);
        break;
    default:
        r->err = true;
        return false;
    }
    if (enc & 0x80) // NamespaceUri (String)
    {
        int32_t l = ua_r_i32(r);
        if (l > 0)
            r_skip(r, (size_t)l);
    }
    if (enc & 0x40) // ServerIndex (UInt32)
        (void)ua_r_u32(r);
    return !r->err;
}

// Skip an ExtensionObject: NodeId TypeId + encoding byte (+ ByteString/XML body).
static bool r_ext_object_skip(UaReader *r)
{
    UaNodeId tid;
    if (!ua_r_nodeid(r, &tid))
        return false;
    uint8_t body_enc = ua_r_u8(r);
    if (body_enc == 0x00) // no body
        return !r->err;
    int32_t l = ua_r_i32(r); // ByteString (0x01) or XmlElement (0x02) body
    if (l > 0)
        r_skip(r, (size_t)l);
    return !r->err;
}

// Read a RequestHeader (the prefix of every service request), capturing the
// RequestHandle. The AuthenticationToken / Timestamp / diagnostics / audit id /
// timeout / AdditionalHeader are consumed and discarded.
static bool r_request_header(UaReader *r, uint32_t *request_handle)
{
    UaNodeId auth;
    ua_r_nodeid(r, &auth);     // AuthenticationToken
    (void)ua_r_u64(r);         // Timestamp (DateTime)
    uint32_t rh = ua_r_u32(r); // RequestHandle
    if (request_handle)
        *request_handle = rh;
    (void)ua_r_u32(r);         // ReturnDiagnostics
    int32_t aid = ua_r_i32(r); // AuditEntryId (String)
    if (aid > 0)
        r_skip(r, (size_t)aid);
    (void)ua_r_u32(r);           // TimeoutHint
    return r_ext_object_skip(r); // AdditionalHeader (ExtensionObject)
}

int64_t opcua_filetime_from_unix(int64_t unix_seconds)
{
    if (unix_seconds <= 0)
        return 0;
    return (unix_seconds + 11644473600LL) * 10000000LL; // 1601->1970 offset, seconds -> 100 ns ticks
}

// ---------------------------------------------------------------------------
// UACP framing + handshake
// ---------------------------------------------------------------------------
bool opcua_parse_header(const uint8_t *buf, size_t len, UaMsgHeader *h)
{
    if (!buf || len < 8 || !h)
        return false;
    h->type[0] = (char)buf[0];
    h->type[1] = (char)buf[1];
    h->type[2] = (char)buf[2];
    h->chunk = (char)buf[3];
    h->size = (uint32_t)buf[4] | ((uint32_t)buf[5] << 8) | ((uint32_t)buf[6] << 16) | ((uint32_t)buf[7] << 24);
    return true;
}

bool opcua_parse_hello(const uint8_t *msg, size_t len, OpcUaHello *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "HEL", 3) != 0)
        return false;
    if (h.size != len || h.size < 8 + 20) // 8-byte header + at least the five sizes
        return false;
    UaReader r = {msg + 8, len - 8, 0, false};
    out->protocol_version = ua_r_u32(&r);
    out->recv_buf_size = ua_r_u32(&r);
    out->send_buf_size = ua_r_u32(&r);
    out->max_msg_size = ua_r_u32(&r);
    out->max_chunk_count = ua_r_u32(&r);
    return !r.err; // EndpointUrl (a String) follows; not needed for negotiation
}

static uint32_t neg(uint32_t client, uint32_t server)
{
    if (client == 0)
        return server;
    return client < server ? client : server;
}

size_t opcua_build_ack(const OpcUaHello *hello, uint8_t *out, size_t cap)
{
    if (!hello || !out)
        return 0;
    const uint32_t total = 8 + 20; // header + 5 x UInt32
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'A');
    ua_w_u8(&w, 'C');
    ua_w_u8(&w, 'K');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, total);
    ua_w_u32(&w, 0);                                          // ProtocolVersion
    ua_w_u32(&w, neg(hello->send_buf_size, DETWS_OPCUA_BUF)); // our ReceiveBufferSize
    ua_w_u32(&w, neg(hello->recv_buf_size, DETWS_OPCUA_BUF)); // our SendBufferSize
    ua_w_u32(&w, neg(hello->max_msg_size, DETWS_OPCUA_BUF));  // MaxMessageSize
    ua_w_u32(&w, 1);                                          // MaxChunkCount (single-chunk)
    return w.ok ? w.n : 0;
}

// ---------------------------------------------------------------------------
// SecureChannel - OpenSecureChannel (OPN), SecurityPolicy None
// ---------------------------------------------------------------------------
bool opcua_parse_open(const uint8_t *msg, size_t len, OpcUaOpenChannel *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "OPN", 3) != 0)
        return false;
    if (h.size != len)
        return false;

    UaReader r = {msg + 8, len - 8, 0, false};

    // Asymmetric security header (SecurityPolicy None -> no certs).
    out->secure_channel_id = ua_r_u32(&r);
    int32_t pol = ua_r_i32(&r); // SecurityPolicyUri (String)
    if (pol > 0)
        r_skip(&r, (size_t)pol);
    int32_t sc = ua_r_i32(&r); // SenderCertificate (ByteString)
    if (sc > 0)
        r_skip(&r, (size_t)sc);
    int32_t rt = ua_r_i32(&r); // ReceiverCertificateThumbprint (ByteString)
    if (rt > 0)
        r_skip(&r, (size_t)rt);

    // Sequence header.
    out->sequence_number = ua_r_u32(&r);
    out->request_id = ua_r_u32(&r);

    // Body: TypeId NodeId (must be OpenSecureChannelRequest).
    UaNodeId tid;
    if (!ua_r_nodeid(&r, &tid))
        return false;
    if (!(tid.numeric && tid.ns == 0 && tid.id == OPCUA_ID_OPEN_REQ))
        return false;

    // RequestHeader.
    if (!r_request_header(&r, &out->request_handle))
        return false;

    // OpenSecureChannelRequest body.
    out->client_protocol_version = ua_r_u32(&r);
    out->security_token_request_type = ua_r_u32(&r);
    out->message_security_mode = ua_r_u32(&r);
    int32_t nonce = ua_r_i32(&r); // ClientNonce (ByteString)
    if (nonce > 0)
        r_skip(&r, (size_t)nonce);
    out->requested_lifetime = ua_r_u32(&r);
    return !r.err;
}

size_t opcua_build_open_response(const OpcUaOpenChannel *req, uint32_t channel_id, uint32_t token_id,
                                 uint32_t seq_number, int64_t now_ft, uint32_t lifetime, uint8_t *out, size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};

    // Message header (size patched after).
    ua_w_u8(&w, 'O');
    ua_w_u8(&w, 'P');
    ua_w_u8(&w, 'N');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, 0); // size placeholder

    // Asymmetric security header (SecurityPolicy None: null sender cert + thumbprint).
    ua_w_u32(&w, channel_id);
    ua_w_string(&w, OPCUA_POLICY_NONE_URI, (int32_t)strlen(OPCUA_POLICY_NONE_URI));
    ua_w_string(&w, nullptr, -1); // SenderCertificate
    ua_w_string(&w, nullptr, -1); // ReceiverCertificateThumbprint

    // Sequence header.
    ua_w_u32(&w, seq_number);
    ua_w_u32(&w, req->request_id); // RequestId echoed

    // Body: TypeId = OpenSecureChannelResponse.
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_OPEN_RESP);

    // ResponseHeader.
    ua_w_u64(&w, (uint64_t)now_ft);    // Timestamp
    ua_w_u32(&w, req->request_handle); // RequestHandle echoed
    ua_w_u32(&w, 0);                   // ServiceResult = Good
    ua_w_u8(&w, 0x00);                 // ServiceDiagnostics (DiagnosticInfo: no fields)
    ua_w_i32(&w, -1);                  // StringTable (null array)
    ua_w_nodeid_numeric(&w, 0, 0);     // AdditionalHeader: null NodeId ...
    ua_w_u8(&w, 0x00);                 // ... + ExtensionObject "no body"

    // OpenSecureChannelResponse body.
    ua_w_u32(&w, 0);                // ServerProtocolVersion
    ua_w_u32(&w, channel_id);       // ChannelSecurityToken.ChannelId
    ua_w_u32(&w, token_id);         // .TokenId
    ua_w_u64(&w, (uint64_t)now_ft); // .CreatedAt
    ua_w_u32(&w, lifetime);         // .RevisedLifetime
    ua_w_string(&w, nullptr, -1);   // ServerNonce (null for None)

    if (!w.ok)
        return 0;
    out[4] = (uint8_t)w.n;
    out[5] = (uint8_t)(w.n >> 8);
    out[6] = (uint8_t)(w.n >> 16);
    out[7] = (uint8_t)(w.n >> 24);
    return w.n;
}

// ---------------------------------------------------------------------------
// Session - CreateSession / ActivateSession (MSG service calls)
// ---------------------------------------------------------------------------

// Patch the 4-byte MessageSize field of a UACP message after the body is written.
static size_t patch_size(UaWriter *w)
{
    if (!w->ok)
        return 0;
    w->o[4] = (uint8_t)w->n;
    w->o[5] = (uint8_t)(w->n >> 8);
    w->o[6] = (uint8_t)(w->n >> 16);
    w->o[7] = (uint8_t)(w->n >> 24);
    return w->n;
}

// Write a MSG envelope prefix: UACP header (size placeholder) + SymmetricSecurityHeader
// (TokenId) + SequenceHeader (SequenceNumber, RequestId).
static void w_msg_prefix(UaWriter *w, uint32_t token_id, uint32_t seq, uint32_t request_id)
{
    ua_w_u8(w, 'M');
    ua_w_u8(w, 'S');
    ua_w_u8(w, 'G');
    ua_w_u8(w, 'F');
    ua_w_u32(w, 0);          // size placeholder
    ua_w_u32(w, token_id);   // SymmetricSecurityHeader.TokenId
    ua_w_u32(w, seq);        // SequenceHeader.SequenceNumber
    ua_w_u32(w, request_id); // SequenceHeader.RequestId
}

// Write a ResponseHeader (the prefix of every service response).
static void w_response_header(UaWriter *w, int64_t now_ft, uint32_t request_handle, uint32_t service_result)
{
    ua_w_u64(w, (uint64_t)now_ft); // Timestamp
    ua_w_u32(w, request_handle);   // RequestHandle echoed
    ua_w_u32(w, service_result);   // ServiceResult
    ua_w_u8(w, 0x00);              // ServiceDiagnostics (DiagnosticInfo: no fields)
    ua_w_i32(w, -1);               // StringTable (null array)
    ua_w_nodeid_numeric(w, 0, 0);  // AdditionalHeader: null NodeId ...
    ua_w_u8(w, 0x00);              // ... + ExtensionObject "no body"
}

bool opcua_parse_msg(const uint8_t *msg, size_t len, OpcUaMsg *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "MSG", 3) != 0)
        return false;
    if (h.size != len)
        return false;

    UaReader r = {msg + 8, len - 8, 0, false};
    out->token_id = ua_r_u32(&r);        // SymmetricSecurityHeader.TokenId
    out->sequence_number = ua_r_u32(&r); // SequenceHeader.SequenceNumber
    out->request_id = ua_r_u32(&r);      // SequenceHeader.RequestId

    UaNodeId tid;
    if (!ua_r_nodeid(&r, &tid)) // body TypeId
        return false;
    out->type_id = tid.numeric ? tid.id : 0;

    return r_request_header(&r, &out->request_handle);
}

size_t opcua_build_create_session_response(const OpcUaMsg *req, uint32_t session_id, uint32_t auth_token,
                                           double revised_timeout, uint32_t seq, int64_t now_ft, uint8_t *out,
                                           size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};
    w_msg_prefix(&w, req->token_id, seq, req->request_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_CREATE_SESSION_RESP);
    w_response_header(&w, now_ft, req->request_handle, 0);

    ua_w_nodeid_numeric(&w, 1, session_id); // SessionId (server-assigned)
    ua_w_nodeid_numeric(&w, 1, auth_token); // AuthenticationToken (server-assigned)
    ua_w_f64(&w, revised_timeout);          // RevisedSessionTimeout (ms)
    ua_w_string(&w, nullptr, -1);           // ServerNonce (none for SecurityPolicy None)
    ua_w_string(&w, nullptr, -1);           // ServerCertificate
    ua_w_i32(&w, 0);                        // ServerEndpoints[] (empty)
    ua_w_i32(&w, 0);                        // ServerSoftwareCertificates[] (empty)
    ua_w_string(&w, nullptr, -1);           // ServerSignature.Algorithm (null String)
    ua_w_string(&w, nullptr, -1);           // ServerSignature.Signature (null ByteString)
    ua_w_u32(&w, 0);                        // MaxRequestMessageSize (0 = no limit)
    return patch_size(&w);
}

size_t opcua_build_activate_session_response(const OpcUaMsg *req, uint32_t seq, int64_t now_ft, uint8_t *out,
                                             size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};
    w_msg_prefix(&w, req->token_id, seq, req->request_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_ACTIVATE_SESSION_RESP);
    w_response_header(&w, now_ft, req->request_handle, 0);

    ua_w_string(&w, nullptr, -1); // ServerNonce (none for SecurityPolicy None)
    ua_w_i32(&w, 0);              // Results[] (empty)
    ua_w_i32(&w, 0);              // DiagnosticInfos[] (empty)
    return patch_size(&w);
}

// ---------------------------------------------------------------------------
// Read service - Variant / DataValue encoding + ReadRequest/ReadResponse
// ---------------------------------------------------------------------------
void ua_w_variant(UaWriter *w, const OpcUaVariant *v)
{
    if (!v || v->type == OPCUA_VAR_NULL)
    {
        ua_w_u8(w, OPCUA_VAR_NULL); // Null Variant (encoding byte 0)
        return;
    }
    ua_w_u8(w, v->type); // encoding byte = built-in type id (scalar; no array bits)
    switch (v->type)
    {
    case OPCUA_VAR_BOOL:
        ua_w_bool(w, v->b);
        break;
    case OPCUA_VAR_INT32:
        ua_w_i32(w, v->i32);
        break;
    case OPCUA_VAR_UINT32:
        ua_w_u32(w, v->u32);
        break;
    case OPCUA_VAR_FLOAT:
        ua_w_f32(w, v->f32);
        break;
    case OPCUA_VAR_DOUBLE:
        ua_w_f64(w, v->f64);
        break;
    case OPCUA_VAR_STRING:
        ua_w_string(w, v->str, v->str_len);
        break;
    default:
        w->ok = false; // unsupported type id: fail closed
        break;
    }
}

void ua_w_datavalue(UaWriter *w, const OpcUaVariant *v, uint32_t status)
{
    bool has_value = v && v->type != OPCUA_VAR_NULL;
    uint8_t mask = 0;
    if (has_value)
        mask |= 0x01; // Value present
    if (status != OPCUA_STATUS_GOOD)
        mask |= 0x02; // StatusCode present
    ua_w_u8(w, mask);
    if (has_value)
        ua_w_variant(w, v);
    if (status != OPCUA_STATUS_GOOD)
        ua_w_u32(w, status);
}

bool opcua_parse_read(const uint8_t *msg, size_t len, OpcUaReadRequest *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "MSG", 3) != 0)
        return false;
    if (h.size != len)
        return false;

    UaReader r = {msg + 8, len - 8, 0, false};
    out->msg.token_id = ua_r_u32(&r);
    out->msg.sequence_number = ua_r_u32(&r);
    out->msg.request_id = ua_r_u32(&r);

    UaNodeId tid;
    if (!ua_r_nodeid(&r, &tid)) // body TypeId
        return false;
    out->msg.type_id = tid.numeric ? tid.id : 0;
    if (!r_request_header(&r, &out->msg.request_handle))
        return false;

    // ReadRequest body.
    (void)ua_r_f64(&r);         // MaxAge
    (void)ua_r_u32(&r);         // TimestampsToReturn (enum)
    int32_t cnt = ua_r_i32(&r); // NodesToRead array length
    out->total = (cnt < 0) ? 0 : (uint32_t)cnt;
    out->count = 0;
    for (int32_t i = 0; i < cnt; i++)
    {
        UaNodeId nid;
        if (!ua_r_nodeid(&r, &nid)) // ReadValueId.NodeId
            return false;
        uint32_t attr = ua_r_u32(&r); // AttributeId
        int32_t ir = ua_r_i32(&r);    // IndexRange (String)
        if (ir > 0)
            r_skip(&r, (size_t)ir);
        (void)ua_r_u16(&r);        // DataEncoding (QualifiedName) NamespaceIndex
        int32_t qn = ua_r_i32(&r); // QualifiedName.Name (String)
        if (qn > 0)
            r_skip(&r, (size_t)qn);
        if (out->count < DETWS_OPCUA_READ_MAX)
        {
            OpcUaReadItem *it = &out->items[out->count++];
            it->ns = nid.ns;
            it->id = nid.id;
            it->numeric = nid.numeric;
            it->attribute = attr;
        }
    }
    return !r.err;
}

size_t opcua_build_read_response(const OpcUaReadRequest *req, const OpcUaVariant *values, const uint32_t *statuses,
                                 uint32_t seq, int64_t now_ft, uint8_t *out, size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};
    w_msg_prefix(&w, req->msg.token_id, seq, req->msg.request_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_READ_RESP);
    w_response_header(&w, now_ft, req->msg.request_handle, 0);

    ua_w_i32(&w, (int32_t)req->count); // Results[] (one DataValue per captured node)
    for (uint32_t i = 0; i < req->count; i++)
        ua_w_datavalue(&w, values ? &values[i] : nullptr, statuses ? statuses[i] : OPCUA_STATUS_GOOD);
    ua_w_i32(&w, 0); // DiagnosticInfos[] (empty)
    return patch_size(&w);
}

// Application Read resolver (set via opcua_set_read_handler), used by opcua_rx.
static OpcUaReadHandler s_read_handler = nullptr;
void opcua_set_read_handler(OpcUaReadHandler fn)
{
    s_read_handler = fn;
}

// ---------------------------------------------------------------------------
// Browse service + CloseSession
// ---------------------------------------------------------------------------
void ua_w_qualifiedname(UaWriter *w, uint16_t ns, const char *name)
{
    ua_w_u16(w, ns);
    ua_w_string(w, name, name ? (int32_t)strlen(name) : -1);
}

void ua_w_localizedtext(UaWriter *w, const char *locale, const char *text)
{
    uint8_t mask = 0;
    if (locale)
        mask |= 0x01; // Locale present
    if (text)
        mask |= 0x02; // Text present
    ua_w_u8(w, mask);
    if (locale)
        ua_w_string(w, locale, (int32_t)strlen(locale));
    if (text)
        ua_w_string(w, text, (int32_t)strlen(text));
}

void ua_w_reference(UaWriter *w, const OpcUaReference *ref)
{
    if (!ref)
    {
        w->ok = false;
        return;
    }
    ua_w_nodeid_numeric(w, 0, ref->ref_type_id);                  // ReferenceTypeId
    ua_w_bool(w, ref->is_forward);                                // IsForward
    ua_w_nodeid_numeric(w, ref->target_ns, ref->target_id);       // NodeId (ExpandedNodeId, numeric, no flags)
    ua_w_qualifiedname(w, ref->browse_name_ns, ref->browse_name); // BrowseName
    ua_w_localizedtext(w, nullptr, ref->display_name);            // DisplayName
    ua_w_u32(w, ref->node_class);                                 // NodeClass
    ua_w_nodeid_numeric(w, 0, ref->type_def_id);                  // TypeDefinition (ExpandedNodeId, numeric)
}

bool opcua_parse_browse(const uint8_t *msg, size_t len, OpcUaBrowseRequest *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "MSG", 3) != 0)
        return false;
    if (h.size != len)
        return false;

    UaReader r = {msg + 8, len - 8, 0, false};
    out->msg.token_id = ua_r_u32(&r);
    out->msg.sequence_number = ua_r_u32(&r);
    out->msg.request_id = ua_r_u32(&r);

    UaNodeId tid;
    if (!ua_r_nodeid(&r, &tid)) // body TypeId
        return false;
    out->msg.type_id = tid.numeric ? tid.id : 0;
    if (!r_request_header(&r, &out->msg.request_handle))
        return false;

    // BrowseRequest body: View (ViewDescription) + RequestedMaxReferencesPerNode + NodesToBrowse.
    UaNodeId view;
    ua_r_nodeid(&r, &view); // View.ViewId
    (void)ua_r_u64(&r);     // View.Timestamp
    (void)ua_r_u32(&r);     // View.ViewVersion
    (void)ua_r_u32(&r);     // RequestedMaxReferencesPerNode

    int32_t cnt = ua_r_i32(&r); // NodesToBrowse array length
    out->total = (cnt < 0) ? 0 : (uint32_t)cnt;
    out->count = 0;
    for (int32_t i = 0; i < cnt; i++)
    {
        UaNodeId nid;
        if (!ua_r_nodeid(&r, &nid)) // BrowseDescription.NodeId
            return false;
        (void)ua_r_u32(&r); // BrowseDirection
        UaNodeId rt;
        ua_r_nodeid(&r, &rt); // ReferenceTypeId
        (void)ua_r_bool(&r);  // IncludeSubtypes
        (void)ua_r_u32(&r);   // NodeClassMask
        (void)ua_r_u32(&r);   // ResultMask
        if (out->count < DETWS_OPCUA_BROWSE_MAX)
        {
            OpcUaBrowseItem *it = &out->items[out->count++];
            it->ns = nid.ns;
            it->id = nid.id;
            it->numeric = nid.numeric;
        }
    }
    return !r.err;
}

size_t opcua_build_browse_response(const OpcUaBrowseRequest *req, OpcUaBrowseHandler handler, uint32_t seq,
                                   int64_t now_ft, uint8_t *out, size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};
    w_msg_prefix(&w, req->msg.token_id, seq, req->msg.request_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_BROWSE_RESP);
    w_response_header(&w, now_ft, req->msg.request_handle, 0);

    ua_w_i32(&w, (int32_t)req->count); // Results[] (one BrowseResult per browsed node)
    for (uint32_t i = 0; i < req->count; i++)
    {
        OpcUaReference refs[DETWS_OPCUA_REF_MAX];
        int32_t n = handler ? handler(req->items[i].ns, req->items[i].id, refs, DETWS_OPCUA_REF_MAX) : -1;
        uint32_t status = (n < 0) ? OPCUA_STATUS_BAD_NODE_ID_UNKNOWN : OPCUA_STATUS_GOOD;
        uint32_t nrefs = (n < 0) ? 0 : (uint32_t)n;

        // BrowseResult.
        ua_w_u32(&w, status);         // StatusCode
        ua_w_string(&w, nullptr, -1); // ContinuationPoint (ByteString, null)
        ua_w_i32(&w, (int32_t)nrefs); // References[]
        for (uint32_t j = 0; j < nrefs; j++)
            ua_w_reference(&w, &refs[j]);
    }
    ua_w_i32(&w, 0); // DiagnosticInfos[] (empty)
    return patch_size(&w);
}

size_t opcua_build_close_session_response(const OpcUaMsg *req, uint32_t seq, int64_t now_ft, uint8_t *out, size_t cap)
{
    if (!req || !out)
        return 0;
    UaWriter w = {out, cap, 0, true};
    w_msg_prefix(&w, req->token_id, seq, req->request_id);
    ua_w_nodeid_numeric(&w, 0, OPCUA_ID_CLOSE_SESSION_RESP);
    w_response_header(&w, now_ft, req->request_handle, 0); // ResponseHeader only
    return patch_size(&w);
}

// Application Browse resolver (set via opcua_set_browse_handler), used by opcua_rx.
static OpcUaBrowseHandler s_browse_handler = nullptr;
void opcua_set_browse_handler(OpcUaBrowseHandler fn)
{
    s_browse_handler = fn;
}

// ---------------------------------------------------------------------------
// ESP32 TCP server (PROTO_OPCUA)
// ---------------------------------------------------------------------------
#ifdef ARDUINO

#include "network_drivers/transport/transport.h"
#include <time.h>

namespace
{
size_t ring_avail(const TcpConn *c)
{
    return (size_t)((c->rx_head - c->rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE);
}
void ring_peek(const TcpConn *c, size_t off, uint8_t *dst, size_t n)
{
    size_t idx = (c->rx_tail + off) % RX_BUF_SIZE;
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = c->rx_buffer[idx];
        idx = (idx + 1) % RX_BUF_SIZE;
    }
}
void ring_consume(TcpConn *c, size_t n)
{
    c->rx_tail = (c->rx_tail + n) % RX_BUF_SIZE;
}
void raw_send(uint8_t slot, const void *data, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE || !c->pcb || n == 0)
        return;
    det_conn_send(c->id, c->pcb, data, (u16_t)n);
    det_conn_flush(c->id, c->pcb);
}
void close_conn(uint8_t slot)
{
    TcpConn *c = &conn_pool[slot];
    if (c->pcb)
    {
        struct tcp_pcb *p = c->pcb;
        det_conn_detach(p);
        c->state = CONN_FREE;
        c->pcb = nullptr;
        det_conn_close(slot, p);
    }
}

uint8_t s_msg[DETWS_OPCUA_BUF]; // single-accessor reassembly buffer
uint8_t s_resp[2048];           // single-accessor response buffer (ACK / OPN / MSG response)

// Per-channel SecureChannel + Session state (single client at a time).
uint32_t s_channel_id = 0;
uint32_t s_token_id = 0;
uint32_t s_seq = 0;
uint32_t s_session_id = 0;
uint32_t s_auth_token = 0;
} // namespace

void opcua_rx(uint8_t slot)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE)
        return;

    // Drain every complete UACP message currently in the rx ring (a client may
    // pipeline HEL then OPN; each arrives framed by an 8-byte header + MessageSize).
    for (;;)
    {
        if (ring_avail(c) < 8)
            return; // need the UACP header

        uint8_t hdr[8];
        ring_peek(c, 0, hdr, 8);
        UaMsgHeader h;
        if (!opcua_parse_header(hdr, 8, &h) || h.size < 8 || h.size > sizeof(s_msg))
        {
            close_conn(slot);
            return;
        }
        if (ring_avail(c) < h.size)
            return; // wait for the full message

        ring_peek(c, 0, s_msg, h.size);
        ring_consume(c, h.size);

        if (memcmp(h.type, "HEL", 3) == 0)
        {
            OpcUaHello hello;
            size_t n;
            if (opcua_parse_hello(s_msg, h.size, &hello) && (n = opcua_build_ack(&hello, s_resp, sizeof(s_resp))) > 0)
                raw_send(slot, s_resp, n);
            else
            {
                close_conn(slot);
                return;
            }
        }
        else if (memcmp(h.type, "OPN", 3) == 0)
        {
            OpcUaOpenChannel oc;
            if (!opcua_parse_open(s_msg, h.size, &oc))
            {
                close_conn(slot);
                return;
            }
            if (oc.secure_channel_id == 0) // fresh issue -> assign a channel id
                oc.secure_channel_id = ++s_channel_id;
            uint32_t token = ++s_token_id;
            uint32_t seq = ++s_seq;
            uint32_t lifetime = oc.requested_lifetime ? oc.requested_lifetime : 3600000u;
            int64_t now = opcua_filetime_from_unix((int64_t)time(nullptr));
            size_t n =
                opcua_build_open_response(&oc, oc.secure_channel_id, token, seq, now, lifetime, s_resp, sizeof(s_resp));
            if (n > 0)
                raw_send(slot, s_resp, n);
            else
            {
                close_conn(slot);
                return;
            }
        }
        else if (memcmp(h.type, "MSG", 3) == 0)
        {
            OpcUaMsg m;
            if (!opcua_parse_msg(s_msg, h.size, &m))
            {
                close_conn(slot);
                return;
            }
            int64_t now = opcua_filetime_from_unix((int64_t)time(nullptr));
            uint32_t seq = ++s_seq;
            size_t n = 0;
            if (m.type_id == OPCUA_ID_CREATE_SESSION_REQ)
                n = opcua_build_create_session_response(&m, ++s_session_id, ++s_auth_token, 1200000.0, seq, now, s_resp,
                                                        sizeof(s_resp));
            else if (m.type_id == OPCUA_ID_ACTIVATE_SESSION_REQ)
                n = opcua_build_activate_session_response(&m, seq, now, s_resp, sizeof(s_resp));
            else if (m.type_id == OPCUA_ID_READ_REQ)
            {
                OpcUaReadRequest rr;
                if (!opcua_parse_read(s_msg, h.size, &rr))
                {
                    close_conn(slot);
                    return;
                }
                OpcUaVariant vals[DETWS_OPCUA_READ_MAX];
                uint32_t sts[DETWS_OPCUA_READ_MAX];
                for (uint32_t i = 0; i < rr.count; i++)
                {
                    memset(&vals[i], 0, sizeof(vals[i]));
                    bool ok = s_read_handler &&
                              s_read_handler(rr.items[i].ns, rr.items[i].id, rr.items[i].attribute, &vals[i]);
                    sts[i] = ok ? OPCUA_STATUS_GOOD : OPCUA_STATUS_BAD_NODE_ID_UNKNOWN;
                }
                n = opcua_build_read_response(&rr, vals, sts, seq, now, s_resp, sizeof(s_resp));
            }
            else if (m.type_id == OPCUA_ID_BROWSE_REQ)
            {
                OpcUaBrowseRequest br;
                if (!opcua_parse_browse(s_msg, h.size, &br))
                {
                    close_conn(slot);
                    return;
                }
                n = opcua_build_browse_response(&br, s_browse_handler, seq, now, s_resp, sizeof(s_resp));
            }
            else if (m.type_id == OPCUA_ID_CLOSE_SESSION_REQ)
                n = opcua_build_close_session_response(&m, seq, now, s_resp, sizeof(s_resp));
            if (n > 0)
                raw_send(slot, s_resp, n);
            // Unhandled MSG services are consumed and ignored (a future increment may ServiceFault).
        }
        else if (memcmp(h.type, "CLO", 3) == 0)
        {
            close_conn(slot);
            return;
        }
    }
}

#else // host build: the codec/handshake are tested directly; rx is a no-op stub

void opcua_rx(uint8_t slot)
{
    (void)slot;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_OPCUA
