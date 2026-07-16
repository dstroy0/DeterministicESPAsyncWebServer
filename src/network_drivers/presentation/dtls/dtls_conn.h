// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dtls_conn.h
 * @brief DTLS 1.3 server handshake state machine (RFC 9147 §5-6).
 *
 * The transport-neutral core that drives one DTLS 1.3 server handshake: it consumes inbound
 * datagrams and produces the outbound flight, wiring the reused TLS 1.3 message builders and key
 * schedule (tls13_msg, tls13_kdf) through the DTLS record layer (dtls_record) and handshake framing
 * (dtls_handshake). Like coap_server_process it has no sockets - the UDP glue (a later CoAPs
 * front-end) feeds it datagrams and sends whatever it emits.
 *
 * Profile: the single spec-valid suite the whole hand-rolled TLS 1.3 stack uses -
 * TLS_AES_128_GCM_SHA256, X25519 key exchange, an Ed25519 server certificate. The handshake is the
 * one-round-trip full handshake (no PSK, no 0-RTT, no client auth):
 *
 *   epoch 0  ClientHello ->
 *            <- ServerHello                                          (epoch 0, DTLSPlaintext)
 *   epoch 2  <- EncryptedExtensions, Certificate, CertificateVerify, Finished  (DTLSCiphertext)
 *   epoch 2  Finished ->
 *   epoch 3  application data (CoAP) protected with the app-traffic keys
 *
 * Each handshake message fits one record in this profile. When the client does not offer an X25519
 * key_share up front, the server answers the first ClientHello with a HelloRetryRequest carrying a
 * stateless, address-bound cookie and renegotiates the group to X25519 (RFC 9147 §5.1); the second
 * ClientHello must echo the cookie before any asymmetric crypto is spent. Full ACK/timeout
 * retransmission (§5.8, §7) beyond the Finished acknowledgement is a follow-on increment; the framing
 * it needs already exists in dtls_handshake.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DTLS_CONN_H
#define DETERMINISTICESPASYNCWEBSERVER_DTLS_CONN_H

#include "ServerConfig.h"

#if DETWS_ENABLE_DTLS

#include "network_drivers/presentation/dtls/dtls_handshake.h"
#include "network_drivers/presentation/dtls/dtls_record.h"
#include "network_drivers/presentation/http3/tls13_kdf.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <stddef.h>
#include <stdint.h>

/** @brief Largest inbound handshake message body reassembled (ClientHello / client Finished). */
#define DTLS_CONN_REASM_CAP 1024

/** @brief Largest single outbound handshake message (Certificate-dominated; one record per message
 *         in this phase, so the certificate plus framing must fit one record). */
#define DTLS_CONN_MSG_CAP 1024

/** @brief Largest serialized peer address the HelloRetryRequest cookie binds (IPv6 16 + port 2). */
#define DTLS_PEER_ADDR_MAX 18

/** @brief Handshake progress. */
enum class DtlsConnState : uint8_t
{
    START,         ///< awaiting ClientHello
    WAIT_FINISHED, ///< server flight sent; awaiting client Finished
    DONE,          ///< handshake complete; application keys installed
    FAILED         ///< fatal error (see @ref dtls_conn_alert)
};

/**
 * @brief The server's long-lived identity plus this handshake's fresh randomness.
 *
 * @c cert_der / @c ed25519_seed are the server's certificate and matching signing key (long-lived).
 * @c ephemeral_priv and @c server_random must be freshly generated per connection by the caller
 * (from a CSPRNG); they are the X25519 ephemeral private key and the ServerHello random.
 */
struct DtlsServerConfig
{
    const uint8_t *cert_der; ///< Ed25519 leaf certificate, DER
    size_t cert_len;
    const uint8_t *ed25519_seed;   ///< 32-byte Ed25519 signing seed (matches @c cert_der)
    const uint8_t *ephemeral_priv; ///< 32-byte X25519 server ephemeral private key (fresh per handshake)
    const uint8_t *server_random;  ///< 32-byte ServerHello random (fresh per handshake)
    const uint8_t *cookie_key;     ///< 32-byte server-wide secret keying the HelloRetryRequest cookie MAC (§5.1)
};

/** @brief One DTLS 1.3 server handshake. Owns all per-connection state; no heap. */
struct DtlsConn
{
    DtlsServerConfig cfg;
    DtlsConnState state;
    uint8_t alert; ///< RFC 8446 §6 alert code when @c state is FAILED (0 otherwise)

    SshSha256Ctx transcript;                         ///< running Transcript-Hash over the TLS handshake messages
    Tls13KeySchedule ks;                             ///< TLS 1.3 key schedule
    DtlsRecordKeys ep2_srv;                          ///< epoch 2 server write keys (handshake traffic)
    DtlsRecordKeys ep2_cli;                          ///< epoch 2 client read keys
    DtlsRecordKeys ep3_srv;                          ///< epoch 3 server write keys (application traffic)
    DtlsRecordKeys ep3_cli;                          ///< epoch 3 client read keys
    bool ep2_ready;                                  ///< epoch 2 keys installed
    bool ep3_ready;                                  ///< epoch 3 keys installed
    uint8_t hs_finished_hash[SSH_SHA256_DIGEST_LEN]; ///< Transcript-Hash(CH..server Finished)

    uint64_t tx_seq_ep0;                   ///< next outbound record sequence number, epoch 0
    uint64_t tx_seq_ep2;                   ///< next outbound record sequence number, epoch 2
    uint64_t tx_seq_ep3;                   ///< next outbound record sequence number, epoch 3
    uint16_t tx_msg_seq;                   ///< next outbound handshake message_seq (advances across an optional HRR)
    bool hrr_sent;                         ///< a HelloRetryRequest was sent; the next ClientHello is the retry (§5.1)
    uint16_t next_recv_msg_seq;            ///< handshake message_seq expected next from the client
    DtlsReplayWindow replay_ep2;           ///< anti-replay window for inbound epoch-2 records
    uint64_t rx_ep2_seq;                   ///< sequence number of the last inbound epoch-2 record (the client Finished)
    bool hs_ack_sent;                      ///< the client Finished has been acknowledged (RFC 9147 §5.8.3 / §7)
    uint8_t peer_addr[DTLS_PEER_ADDR_MAX]; ///< serialized peer address the HRR cookie is bound to (§5.1)
    uint8_t peer_addr_len;                 ///< bytes of @ref peer_addr in use (0 = no address bound)

    DtlsHsReasm reasm;                                    ///< inbound handshake reassembler
    uint8_t reasm_buf[4 + DTLS_CONN_REASM_CAP];           ///< TLS message = 4-byte header [0..3] + body [4..]
    uint8_t msgbuf[DTLS_CONN_MSG_CAP];                    ///< scratch for one outbound TLS message
    uint8_t fragbuf[DTLS_CONN_MSG_CAP + DTLS_HS_HDR_LEN]; ///< scratch for its DTLS handshake fragment
};

/**
 * @brief Initialize a connection for a new handshake. @p cfg is copied (its pointers must outlive @p c).
 *
 * @param peer_addr      serialized peer address (e.g. IP || port) the HelloRetryRequest cookie binds,
 *                       so a cookie minted for one peer is worthless to another (RFC 9147 §5.1). May
 *                       be NULL / 0 when no HRR is expected (the one-round-trip happy path).
 * @param peer_addr_len  length of @p peer_addr; clamped to @ref DTLS_PEER_ADDR_MAX.
 */
void dtls_conn_init(DtlsConn *c, const DtlsServerConfig *cfg, const uint8_t *peer_addr, size_t peer_addr_len);

/**
 * @brief Feed one inbound datagram; append any response records to @p out.
 *
 * Demultiplexes the datagram's records (DTLSPlaintext for epoch 0, DTLSCiphertext for epoch 2),
 * unprotects and reassembles the handshake, and drives the state machine, writing the server's
 * response flight to @p out.
 *
 * @return the number of bytes written to @p out (0 if nothing to send), or -1 on a fatal error
 *         (then @c state is FAILED and @ref dtls_conn_alert gives the reason).
 */
int dtls_conn_process(DtlsConn *c, const uint8_t *dgram, size_t len, uint8_t *out, size_t out_cap);

/** @brief True once the handshake has completed and the application-traffic keys are installed. */
bool dtls_conn_established(const DtlsConn *c);

/** @brief The alert code (RFC 8446 §6) set when the handshake failed, or 0. */
uint8_t dtls_conn_alert(const DtlsConn *c);

/** @brief Application-epoch (epoch 3) server write keys - protect outbound application records. */
const DtlsRecordKeys *dtls_conn_app_write_keys(const DtlsConn *c);

/** @brief Application-epoch (epoch 3) client read keys - unprotect inbound application records. */
const DtlsRecordKeys *dtls_conn_app_read_keys(const DtlsConn *c);

#endif // DETWS_ENABLE_DTLS
#endif // DETERMINISTICESPASYNCWEBSERVER_DTLS_CONN_H
