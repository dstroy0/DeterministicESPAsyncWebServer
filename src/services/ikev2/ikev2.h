// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ikev2.h
 * @brief IKEv2 (RFC 7296) message + payload codec (DWS_ENABLE_IKEV2) - a zero-heap builder / parser for
 *        the Internet Key Exchange v2 wire format that negotiates IPsec security associations over UDP
 *        500 / 4500 (NAT-T). This is the "secure machine bridge over untrusted networks" southbound: the
 *        framing tier of an IKEv2/IPsec stack.
 *
 * Scope (tier 1 of the IPsec roadmap item): the pure wire codec only - build / parse into caller
 * buffers, no sockets and no crypto. It frames the 28-octet IKE header and the generic payload chain
 * (SA -> proposals -> transforms, KE, Ni/Nr, IDi/IDr, CERT/CERTREQ, AUTH, N notify, D delete, TSi/TSr,
 * and the SK encrypted-payload envelope). The Diffie-Hellman math, the SKEYSEED / SK_* key derivation,
 * the AEAD encrypt/decrypt of the SK payload, and the IKE_SA_INIT -> IKE_AUTH state machine are the
 * later tiers (they reuse the crypto the library already ships); this file just lays out and reads the
 * bytes so those tiers have a tested framing seam.
 *
 * Wire framing (byte-exact, network byte order): the IKE header is 8-byte Initiator SPI, 8-byte
 * Responder SPI, 1-byte Next Payload, 1-byte Version (0x20 = MjVer 2 / MnVer 0), 1-byte Exchange Type,
 * 1-byte Flags, 4-byte Message ID, 4-byte Length (whole message). Every payload starts with the same
 * 4-byte generic header: Next Payload (the type of the FOLLOWING payload, so the chain is walked
 * forward from the header's Next Payload), a Critical bit + 7 reserved bits, and a 2-byte Payload Length
 * (this payload incl. the generic header). Multi-byte fields are big-endian throughout.
 *
 * Reference: RFC 7296 (IKEv2) + the IANA "Internet Key Exchange Version 2 (IKEv2) Parameters" registry.
 * Every structure was cross-checked byte-for-byte against scapy's IKEv2 contrib (the header, KE, Nonce,
 * Notify, Delete, and the SA -> proposal -> transform tree, including the key-length transform attribute
 * verified against scapy's decoder).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_IKEV2_H
#define DETERMINISTICESPASYNCWEBSERVER_IKEV2_H

#include "ServerConfig.h"

#if DWS_ENABLE_IKEV2

#include <stddef.h>
#include <stdint.h>

/** @brief IKEv2 UDP port (IKE_SA_INIT / IKE_AUTH before NAT is detected). */
#define DWS_IKEV2_PORT 500
/** @brief IKEv2 UDP port after NAT traversal is negotiated (NAT-T, RFC 3948). */
#define DWS_IKEV2_NAT_PORT 4500
/** @brief Fixed IKE header size. */
#define DWS_IKE_HDR_LEN 28
/** @brief IKE SPI size (Initiator / Responder). */
#define DWS_IKE_SPI_LEN 8
/** @brief Generic payload header size (next-payload + critical/reserved + 2-byte length). */
#define DWS_IKE_PAYLOAD_HDR_LEN 4
/** @brief IKE version byte value: major 2, minor 0. */
#define DWS_IKE_VERSION 0x20
/** @brief Critical bit in a payload's second header byte. */
#define DWS_IKE_CRITICAL 0x80

/** @brief IKE header flag bits (RFC 7296 3.1). */
#define DWS_IKE_FLAG_INITIATOR 0x08 ///< set in messages from the original initiator
#define DWS_IKE_FLAG_VERSION 0x10   ///< set if a higher minor version is supported
#define DWS_IKE_FLAG_RESPONSE 0x20  ///< set in a response (clear in a request)

/** @brief Exchange types (RFC 7296 3.1). */
enum class IkeExchange : uint8_t
{
    IKE_SA_INIT = 34,
    IKE_AUTH = 35,
    IKE_CREATE_CHILD_SA = 36,
    IKE_INFORMATIONAL = 37,
};

/** @brief Payload types / Next Payload values (RFC 7296 3.2); 0 terminates a chain. */
enum class IkePayloadType : uint8_t
{
    IKE_PL_NONE = 0,
    IKE_PL_SA = 33,      ///< Security Association (proposals / transforms)
    IKE_PL_KE = 34,      ///< Key Exchange
    IKE_PL_IDI = 35,     ///< Identification - Initiator
    IKE_PL_IDR = 36,     ///< Identification - Responder
    IKE_PL_CERT = 37,    ///< Certificate
    IKE_PL_CERTREQ = 38, ///< Certificate Request
    IKE_PL_AUTH = 39,    ///< Authentication
    IKE_PL_NONCE = 40,   ///< Nonce (Ni / Nr)
    IKE_PL_NOTIFY = 41,  ///< Notify
    IKE_PL_DELETE = 42,  ///< Delete
    IKE_PL_VENDOR = 43,  ///< Vendor ID
    IKE_PL_TSI = 44,     ///< Traffic Selector - Initiator
    IKE_PL_TSR = 45,     ///< Traffic Selector - Responder
    IKE_PL_SK = 46,      ///< Encrypted and Authenticated
    IKE_PL_CP = 47,      ///< Configuration
    IKE_PL_EAP = 48,     ///< Extensible Authentication
};

/** @brief Transform types (RFC 7296 3.3.2). */
enum class IkeTransformType : uint8_t
{
    IKE_TRANSFORM_ENCR = 1,  ///< Encryption Algorithm
    IKE_TRANSFORM_PRF = 2,   ///< Pseudo-random Function
    IKE_TRANSFORM_INTEG = 3, ///< Integrity Algorithm
    IKE_TRANSFORM_DH = 4,    ///< Diffie-Hellman / Key Exchange Method
    IKE_TRANSFORM_ESN = 5,   ///< Extended Sequence Numbers
};

/** @brief A few common Transform IDs (IANA), for convenience; any 16-bit id is accepted on the wire. */
#define IKE_ENCR_AES_CBC 12
#define IKE_ENCR_AES_GCM_16 20
#define IKE_ENCR_CHACHA20_POLY1305 28
#define IKE_PRF_HMAC_SHA2_256 5
#define IKE_INTEG_HMAC_SHA2_256_128 12
#define IKE_DH_MODP2048 14
#define IKE_DH_ECP256 19
#define IKE_DH_CURVE25519 31

/** @brief Transform attribute type: Key Length (RFC 7296 3.3.5), encoded TV (AF bit set). */
#define IKE_ATTR_KEY_LENGTH 14

/** @brief Protocol IDs (RFC 7296 3.3.1 / 3.10 / 3.11). */
enum class IkeProtocol : uint8_t
{
    IKE_PROTO_NONE = 0, ///< notify/delete not concerning an existing SA (RFC 7296 3.10)
    IKE_PROTO_IKE = 1,
    IKE_PROTO_AH = 2,
    IKE_PROTO_ESP = 3,
};

/** @brief Identification payload ID types (RFC 7296 3.5). */
enum class IkeIdType : uint8_t
{
    IKE_ID_RESERVED = 0, ///< IANA-reserved; the value an out-param holds before a parse succeeds
    IKE_ID_IPV4_ADDR = 1,
    IKE_ID_FQDN = 2,
    IKE_ID_RFC822_ADDR = 3,
    IKE_ID_IPV6_ADDR = 5,
    IKE_ID_KEY_ID = 11,
};

/** @brief Authentication methods (RFC 7296 3.8 / IANA). */
enum class IkeAuthMethod : uint8_t
{
    IKE_AUTH_RESERVED = 0,    ///< IANA-reserved; the value an out-param holds before a parse succeeds
    IKE_AUTH_RSA_SIG = 1,     ///< RSA Digital Signature
    IKE_AUTH_PSK = 2,         ///< Shared Key Message Integrity Code (pre-shared key)
    IKE_AUTH_DSS_SIG = 3,     ///< DSS Digital Signature
    IKE_AUTH_DIGITAL_SIG = 14 ///< Generic Digital Signature (RFC 7427)
};

/** @brief Traffic selector types (RFC 7296 3.13.1). */
enum class IkeTsType : uint8_t
{
    IKE_TS_IPV4_ADDR_RANGE = 7,
    IKE_TS_IPV6_ADDR_RANGE = 8,
};

/** @brief A decoded / to-be-encoded IKE header. */
struct IkeHeader
{
    uint8_t init_spi[DWS_IKE_SPI_LEN];
    uint8_t resp_spi[DWS_IKE_SPI_LEN];
    IkePayloadType next_payload; ///< type of the first payload in the message
    uint8_t version;             ///< 0x20 for IKEv2
    IkeExchange exchange;        ///< @ref IkeExchange
    uint8_t flags;               ///< OR of DWS_IKE_FLAG_*
    uint32_t message_id;
    uint32_t length; ///< whole-message length (built value; on parse, the value read off the wire)
};

/** @brief A parsed generic payload: its type (from the chain), the following type, and the body slice
 *  (the bytes after the 4-byte generic header), pointing INTO the caller's buffer. */
struct IkePayload
{
    IkePayloadType type;         ///< this payload's type
    IkePayloadType next_payload; ///< type of the next payload (IKE_PL_NONE = last)
    bool critical;               ///< the Critical bit
    const uint8_t *body;
    size_t body_len;
};

/** @brief Forward-walks the payload chain of a message. */
struct IkePayloadIter
{
    const uint8_t *area;      ///< payload area (message + DWS_IKE_HDR_LEN)
    size_t len;               ///< bytes remaining in the area
    size_t off;               ///< current offset into @ref area
    IkePayloadType next_type; ///< type of the payload at @ref off (IKE_PL_NONE = done)
};

/** @brief One transform to encode inside a proposal (@ref dws_ike_sa_build). */
struct IkeTransform
{
    IkeTransformType type; ///< which transform slot this fills
    uint16_t id;           ///< transform id (algorithm)
    int32_t key_length;    ///< key-length attribute in bits, or < 0 for none
};

/** @brief A parsed transform (from @ref dws_ike_transform_next). */
struct IkeTransformRef
{
    IkeTransformType type;
    uint16_t id;
    int32_t key_length; ///< decoded key-length attribute, or < 0 if absent
    bool last;          ///< true if this is the last transform in the proposal
};

/** @brief A parsed proposal (from @ref dws_ike_sa_first_proposal). */
struct IkeProposalRef
{
    uint8_t proposal_num;
    IkeProtocol protocol_id; ///< which SA this proposal is for
    uint8_t spi_size;
    uint8_t num_transforms;
    const uint8_t *spi; ///< SPI bytes (spi_size long), or nullptr
    const uint8_t *transforms;
    size_t transforms_len;
    bool last; ///< true if this is the only / last proposal
};

/** @brief Iterates the transforms within a proposal's transform area. */
struct IkeTransformIter
{
    const uint8_t *area;
    size_t len;
    size_t off;
};

/** @brief One traffic selector (RFC 7296 3.13.1). */
struct IkeTrafficSelector
{
    IkeTsType ts_type;   ///< selector address family
    uint8_t ip_protocol; ///< 0 = any
    uint16_t start_port;
    uint16_t end_port;
    const uint8_t *start_addr; ///< 4 bytes (IPv4) or 16 bytes (IPv6)
    const uint8_t *end_addr;
    size_t addr_len; ///< 4 or 16
};

// ── IKE header ──────────────────────────────────────────────────────────────────────────────────

/**
 * @brief Build the 28-byte IKE header from @p h (writing @p h->length verbatim - use
 *        @ref dws_ike_set_length to patch it once the payloads are appended).
 * @return DWS_IKE_HDR_LEN (28) on success, or 0 on overflow / bad input.
 */
size_t dws_ike_hdr_build(uint8_t *buf, size_t cap, const IkeHeader *h);

/**
 * @brief Parse the 28-byte IKE header into @p out.
 * @return true if at least 28 bytes are present; false otherwise. (The version byte is surfaced in
 *         @p out->version but not rejected, so a caller can decide how to handle a mismatch.)
 */
bool dws_ike_hdr_parse(const uint8_t *buf, size_t len, IkeHeader *out);

/**
 * @brief Patch the 4-byte Length field of an already-built header (bytes 24..27) to @p total_len.
 * @return true if @p buf holds at least a header.
 */
bool dws_ike_set_length(uint8_t *buf, size_t buf_cap, uint32_t total_len);

// ── generic payload chain ─────────────────────────────────────────────────────────────────────

/**
 * @brief Start walking the payload chain: @p first_type is the header's Next Payload and @p area /
 *        @p area_len is the message body after the header (message + DWS_IKE_HDR_LEN).
 */
void dws_ike_payload_iter_init(IkePayloadIter *it, IkePayloadType first_type, const uint8_t *area, size_t area_len);

/**
 * @brief Read the next payload's generic header + body slice into @p out and advance.
 * @return true if a well-formed payload was produced; false at end of chain or on a malformed length
 *         (payload length < 4 or running past the area).
 */
bool dws_ike_payload_next(IkePayloadIter *it, IkePayload *out);

/**
 * @brief Build a raw payload: the 4-byte generic header (@p next_payload + optional @p critical +
 *        length) followed by @p body. Most callers use the typed builders below instead.
 * @return total bytes written (4 + body_len), or 0 on overflow.
 */
size_t dws_ike_payload_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, bool critical, const uint8_t *body,
                             size_t body_len);

// ── typed payload builders (each writes a full payload incl. the generic header) ──────────────

/** @brief Build an SA payload carrying ONE proposal with @p num_transforms transforms. */
size_t dws_ike_sa_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint8_t proposal_num,
                        IkeProtocol protocol_id, const uint8_t *spi, uint8_t spi_size, const IkeTransform *transforms,
                        uint8_t num_transforms);

/** @brief Build a KE payload: DH group + key-exchange data. */
size_t dws_ike_ke_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint16_t dh_group, const uint8_t *data,
                        size_t data_len);

/** @brief Build a Nonce payload (Ni / Nr): the raw nonce bytes. */
size_t dws_ike_nonce_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const uint8_t *nonce,
                           size_t nonce_len);

/** @brief Build an Identification payload (IDi or IDr - the type is set by the previous Next Payload). */
size_t dws_ike_id_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeIdType id_type, const uint8_t *data,
                        size_t data_len);

/** @brief Build an AUTH payload: auth method + authentication data. */
size_t dws_ike_auth_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeAuthMethod auth_method,
                          const uint8_t *data, size_t data_len);

/** @brief Build a CERT or CERTREQ payload (same layout): cert encoding + data. The payload type is set
 *  by the previous Next Payload; this only writes the body. */
size_t dws_ike_cert_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint8_t cert_encoding,
                          const uint8_t *data, size_t data_len);

/** @brief Build a Notify payload: protocol + optional SPI + 16-bit type + notification data. */
size_t dws_ike_notify_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeProtocol protocol_id,
                            const uint8_t *spi, uint8_t spi_size, uint16_t notify_type, const uint8_t *data,
                            size_t data_len);

/** @brief Build a Delete payload: protocol + a list of @p num_spis SPIs each @p spi_size bytes. */
size_t dws_ike_delete_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeProtocol protocol_id,
                            uint8_t spi_size, const uint8_t *spis, uint16_t num_spis);

/** @brief Build a Traffic Selector payload (TSi or TSr) from @p num selectors. */
size_t dws_ike_ts_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const IkeTrafficSelector *sels,
                        uint8_t num);

/**
 * @brief Frame an SK (encrypted) payload envelope: the generic header then @p iv, @p ciphertext, and
 *        @p icv laid end to end. The AEAD that produces @p ciphertext + @p icv is the caller's (a later
 *        tier) - this only lays out the bytes.
 * @return total bytes written, or 0 on overflow.
 */
size_t dws_ike_sk_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const uint8_t *iv, size_t iv_len,
                        const uint8_t *ciphertext, size_t ct_len, const uint8_t *icv, size_t icv_len);

// ── typed payload parsers (each takes a payload BODY from dws_ike_payload_next) ────────────────

/** @brief Decode a KE body into DH group + key data. */
bool dws_ike_ke_parse(const uint8_t *body, size_t body_len, uint16_t *dh_group, const uint8_t **data, size_t *data_len);

/** @brief Decode an Identification body into id type + data. */
bool dws_ike_id_parse(const uint8_t *body, size_t body_len, IkeIdType *id_type, const uint8_t **data, size_t *data_len);

/** @brief Decode an AUTH body into method + data. */
bool dws_ike_auth_parse(const uint8_t *body, size_t body_len, IkeAuthMethod *auth_method, const uint8_t **data,
                        size_t *data_len);

/** @brief Decode a Notify body into protocol, type, optional SPI, and notification data. */
bool dws_ike_notify_parse(const uint8_t *body, size_t body_len, IkeProtocol *protocol_id, uint16_t *notify_type,
                          const uint8_t **spi, uint8_t *spi_size, const uint8_t **data, size_t *data_len);

/** @brief Decode a Delete body into protocol, SPI size, count, and the SPI list. */
bool dws_ike_delete_parse(const uint8_t *body, size_t body_len, IkeProtocol *protocol_id, uint8_t *spi_size,
                          uint16_t *num_spis, const uint8_t **spis);

/**
 * @brief Slice an SK body into IV / ciphertext / ICV given the negotiated @p iv_len and @p icv_len.
 * @return true if the body is at least @p iv_len + @p icv_len bytes.
 */
bool dws_ike_sk_parse(const uint8_t *body, size_t body_len, size_t iv_len, size_t icv_len, const uint8_t **iv,
                      const uint8_t **ciphertext, size_t *ct_len, const uint8_t **icv);

// ── SA / proposal / transform parsing ─────────────────────────────────────────────────────────

/** @brief Read the first proposal out of an SA payload body. */
bool dws_ike_sa_first_proposal(const uint8_t *body, size_t body_len, IkeProposalRef *out);

/** @brief Start iterating the transforms of a parsed proposal. */
void dws_ike_transform_iter_init(IkeTransformIter *it, const IkeProposalRef *p);

/** @brief Read the next transform (decoding the key-length attribute if present). */
bool dws_ike_transform_next(IkeTransformIter *it, IkeTransformRef *out);

// ── traffic selector parsing ──────────────────────────────────────────────────────────────────

/** @brief The number of traffic selectors in a TS payload body (0 on malformed). */
uint8_t dws_ike_ts_count(const uint8_t *body, size_t body_len);

/** @brief Decode selector @p index (0-based) from a TS payload body. */
bool dws_ike_ts_get(const uint8_t *body, size_t body_len, uint8_t index, IkeTrafficSelector *out);

#endif // DWS_ENABLE_IKEV2

#endif // DETERMINISTICESPASYNCWEBSERVER_IKEV2_H
