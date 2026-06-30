// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_ber.h
 * @brief Zero-heap ASN.1 BER encoder/decoder for the SNMP agent (DETWS_ENABLE_SNMP).
 *
 * A minimal, bounded TLV codec covering exactly the types SNMP uses: INTEGER,
 * OCTET STRING, NULL, OBJECT IDENTIFIER, SEQUENCE, and the SNMP application
 * types (Counter32/Gauge32/TimeTicks/IpAddress/Counter64) and PDU context tags.
 * Encoder and decoder both operate over caller-provided fixed buffers - no heap.
 * This is the shared base for SNMP v1/v2c and (later) v3; it is unit-tested on
 * its own (env:native_snmp) since it needs no lwIP.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SNMP_BER_H
#define DETERMINISTICESPASYNCWEBSERVER_SNMP_BER_H

#include "DetWebServerConfig.h"
#include "shared_primitives/shim.h"

#if DETWS_ENABLE_SNMP

// ASN.1 / SNMP tags
enum SnmpTag
{
    BER_INTEGER = 0x02,
    BER_OCTET_STRING = 0x04,
    BER_NULL = 0x05,
    BER_OID = 0x06,
    BER_SEQUENCE = 0x30,
    // SNMP application types (RFC 2578)
    SNMP_IPADDRESS = 0x40,
    SNMP_COUNTER32 = 0x41,
    SNMP_GAUGE32 = 0x42,
    SNMP_TIMETICKS = 0x43,
    SNMP_OPAQUE = 0x44,
    SNMP_COUNTER64 = 0x46,
    // VarBind exception markers (RFC 3416)
    SNMP_NO_SUCH_OBJECT = 0x80,
    SNMP_NO_SUCH_INSTANCE = 0x81,
    SNMP_END_OF_MIB_VIEW = 0x82,
    // PDU tags (context-specific, constructed)
    SNMP_PDU_GET = 0xA0,
    SNMP_PDU_GETNEXT = 0xA1,
    SNMP_PDU_RESPONSE = 0xA2,
    SNMP_PDU_SET = 0xA3,
    SNMP_PDU_GETBULK = 0xA5,
    SNMP_PDU_TRAPV2 = 0xA7,
    SNMP_PDU_REPORT = 0xA8,
};

// ---------------------------------------------------------------------------
// Encoder - forward writer over a caller buffer. Constructed types reserve a
// 3-byte long-form length that is back-patched at close (valid BER; accepted by
// net-snmp etc.), so no buffering or shifting is needed.
// ---------------------------------------------------------------------------
struct BerEnc
{
    uint8_t *buf;
    size_t cap;
    size_t len;
    bool ok;
};

void ber_enc_init(BerEnc *e, uint8_t *buf, size_t cap);

bool ber_put_integer(BerEnc *e, long v);                                       ///< INTEGER (signed, minimal)
bool ber_put_uint(BerEnc *e, uint8_t tag, uint32_t v);                         ///< non-negative int with @p tag
bool ber_put_octet_string(BerEnc *e, uint8_t tag, const uint8_t *d, size_t n); ///< OCTET STRING / IpAddress / Opaque
bool ber_put_null(BerEnc *e);                                                  ///< NULL
bool ber_put_oid(BerEnc *e, const uint32_t *arcs, size_t n);                   ///< OBJECT IDENTIFIER (n >= 2)
bool ber_put_tlv(BerEnc *e, uint8_t tag, const uint8_t *val, size_t n);        ///< raw primitive TLV
bool ber_put_raw(BerEnc *e, const uint8_t *bytes, size_t n);                   ///< append pre-encoded bytes verbatim

size_t ber_seq_begin(BerEnc *e, uint8_t tag); ///< open a constructed type; returns a token
void ber_seq_end(BerEnc *e, size_t token);    ///< close it (back-patch the length)

// ---------------------------------------------------------------------------
// Decoder - forward reader over a buffer.
// ---------------------------------------------------------------------------
struct BerDec
{
    const uint8_t *buf;
    size_t len;
    size_t pos;
    bool ok;
};

void ber_dec_init(BerDec *d, const uint8_t *buf, size_t len);

/** @brief Read a tag + length; on success @p d->pos is left at the value. */
bool ber_read_header(BerDec *d, uint8_t *tag, size_t *length);
/** @brief Read an INTEGER into @p out. */
bool ber_read_integer(BerDec *d, long *out);
/** @brief Read an OBJECT IDENTIFIER into @p arcs (capacity @p max); count in @p n. */
bool ber_read_oid(BerDec *d, uint32_t *arcs, size_t max, size_t *n);
/** @brief Advance the cursor past @p length value bytes. */
bool ber_skip(BerDec *d, size_t length);

#endif // DETWS_ENABLE_SNMP

#endif // DETERMINISTICESPASYNCWEBSERVER_SNMP_BER_H
