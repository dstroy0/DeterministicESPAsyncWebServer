// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_notify.cpp
 * @brief Outbound SNMP Trap / Inform PDU builder (host-testable) + the UDP send
 *        (ESP32 only). SNMPv3 USM notifications live in snmp_v3.cpp.
 */

#include "services/snmp/snmp_notify.h"

#if DETWS_ENABLE_SNMP_TRAP

#include "services/snmp/snmp_ber.h"
#include <string.h>

// The two mandatory bindings of any v2c/v3 notification (RFC 3416 4.2.6).
static const uint32_t OID_SYSUPTIME_0[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
static const uint32_t OID_SNMPTRAPOID_0[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};

// Encode one caller varbind: SEQUENCE { OID, typed-value }.
static void put_varbind(BerEnc *e, const SnmpVarbind *vb)
{
    size_t t = ber_seq_begin(e, BER_SEQUENCE);
    ber_put_oid(e, vb->oid, vb->oid_len);
    switch (vb->type)
    {
    case SNMP_VB_INT:
        ber_put_integer(e, vb->ival);
        break;
    case SNMP_VB_STRING:
        ber_put_octet_string(e, BER_OCTET_STRING, vb->bytes, vb->blen);
        break;
    case SNMP_VB_OID:
        ber_put_oid(e, vb->oid_val, vb->oid_val_len);
        break;
    case SNMP_VB_COUNTER32:
        ber_put_uint(e, SNMP_COUNTER32, (uint32_t)vb->ival);
        break;
    case SNMP_VB_GAUGE32:
        ber_put_uint(e, SNMP_GAUGE32, (uint32_t)vb->ival);
        break;
    case SNMP_VB_TIMETICKS:
        ber_put_uint(e, SNMP_TIMETICKS, (uint32_t)vb->ival);
        break;
    case SNMP_VB_IPADDR:
        ber_put_octet_string(e, SNMP_IPADDRESS, vb->bytes, vb->blen);
        break;
    default:
        e->ok = false;
        break;
    }
    ber_seq_end(e, t);
}

// Build the notification PDU (request-id, 0, 0, varbinds) under @p pdu_tag. The
// varbinds begin with sysUpTime.0 + snmpTrapOID.0. Shared with the v3 builder
// (which wraps this PDU in a scopedPDU); see snmp_notify_build_pdu() in the header.
size_t snmp_notify_build_pdu(BerEnc *e, uint8_t pdu_tag, uint32_t request_id, const uint32_t *trap_oid,
                             size_t trap_oid_len, uint32_t uptime_ticks, const SnmpVarbind *vbs, size_t n)
{
    size_t pdu = ber_seq_begin(e, pdu_tag);
    ber_put_integer(e, (long)request_id); // request-id
    ber_put_integer(e, 0);                // error-status
    ber_put_integer(e, 0);                // error-index
    size_t vbl = ber_seq_begin(e, BER_SEQUENCE);
    // sysUpTime.0 = TimeTicks
    {
        size_t t = ber_seq_begin(e, BER_SEQUENCE);
        ber_put_oid(e, OID_SYSUPTIME_0, sizeof(OID_SYSUPTIME_0) / sizeof(uint32_t));
        ber_put_uint(e, SNMP_TIMETICKS, uptime_ticks);
        ber_seq_end(e, t);
    }
    // snmpTrapOID.0 = OID
    {
        size_t t = ber_seq_begin(e, BER_SEQUENCE);
        ber_put_oid(e, OID_SNMPTRAPOID_0, sizeof(OID_SNMPTRAPOID_0) / sizeof(uint32_t));
        ber_put_oid(e, trap_oid, trap_oid_len);
        ber_seq_end(e, t);
    }
    for (size_t i = 0; i < n; i++)
        put_varbind(e, &vbs[i]);
    ber_seq_end(e, vbl);
    ber_seq_end(e, pdu);
    return e->len;
}

size_t snmp_notify_build_v2c(uint8_t *out, size_t cap, const char *community, uint8_t pdu_tag, uint32_t request_id,
                             const uint32_t *trap_oid, size_t trap_oid_len, uint32_t uptime_ticks,
                             const SnmpVarbind *vbs, size_t n)
{
    if (!out || !community || !trap_oid)
        return 0;
    BerEnc e;
    ber_enc_init(&e, out, cap);
    size_t msg = ber_seq_begin(&e, BER_SEQUENCE);
    ber_put_integer(&e, 1); // version: SNMPv2c
    ber_put_octet_string(&e, BER_OCTET_STRING, (const uint8_t *)community, strlen(community));
    snmp_notify_build_pdu(&e, pdu_tag, request_id, trap_oid, trap_oid_len, uptime_ticks, vbs, n);
    ber_seq_end(&e, msg);
    return e.ok ? e.len : 0;
}

// ---------------------------------------------------------------------------
// Transport (ESP32 only)
// ---------------------------------------------------------------------------
#if defined(ARDUINO)

#include "network_drivers/transport/udp_transport.h"
#include <Arduino.h>

// All SNMP-notify transport state, owned by one instance (internal linkage): the trap
// request-id counter, so it is one named owner, unreachable from any other translation unit.
namespace
{
struct SnmpNotifyCtx
{
    uint32_t trap_reqid = 1;
};
SnmpNotifyCtx s_notify;
} // namespace

bool snmp_trap_v2c(const char *dst_ip, uint16_t port, const char *community, const uint32_t *trap_oid,
                   size_t trap_oid_len, const SnmpVarbind *vbs, size_t n)
{
    uint8_t buf[DETWS_SNMP_TRAP_BUF_SIZE];
    uint32_t up = (uint32_t)(millis() / 10); // TimeTicks = hundredths of a second
    size_t len = snmp_notify_build_v2c(buf, sizeof(buf), community, SNMP_PDU_TRAPV2, s_notify.trap_reqid++, trap_oid,
                                       trap_oid_len, up, vbs, n);
    return len && det_udp_sendto(dst_ip, port, buf, len);
}

bool snmp_inform_v2c(const char *dst_ip, uint16_t port, const char *community, uint32_t request_id,
                     const uint32_t *trap_oid, size_t trap_oid_len, const SnmpVarbind *vbs, size_t n)
{
    uint8_t buf[DETWS_SNMP_TRAP_BUF_SIZE];
    uint32_t up = (uint32_t)(millis() / 10);
    size_t len = snmp_notify_build_v2c(buf, sizeof(buf), community, 0xA6 /* InformRequest */, request_id, trap_oid,
                                       trap_oid_len, up, vbs, n);
    return len && det_udp_sendto(dst_ip, port, buf, len);
}

#else // host build: transport is a stub

bool snmp_trap_v2c(const char *, uint16_t, const char *, const uint32_t *, size_t, const SnmpVarbind *, size_t)
{
    return false;
}
bool snmp_inform_v2c(const char *, uint16_t, const char *, uint32_t, const uint32_t *, size_t, const SnmpVarbind *,
                     size_t)
{
    return false;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_SNMP_TRAP
