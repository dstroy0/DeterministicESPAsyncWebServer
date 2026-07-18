// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file coaps.cpp
 * @brief CoAP over DTLS (CoAPs, RFC 7252 §9). See coaps.h.
 */

#include "services/coap/coaps.h"

#if DETWS_ENABLE_DTLS && DETWS_ENABLE_COAP

#include "services/coap/coap.h"

// Largest CoAP request/response carried in one DTLS application record. CoAP messages are expected to
// fit a single datagram (RFC 7252 §4.6); anything larger is dropped rather than fragmented here.
static constexpr size_t COAPS_MSG_CAP = 1152;

int det_coaps_process(DtlsConn *c, const uint8_t *dgram, size_t len, uint8_t *out, size_t out_cap)
{
    if (!det_dtls_conn_established(c))
        return det_dtls_conn_process(c, dgram, len, out, out_cap); // still handshaking (or -1 on fatal error)

    // Established. A DTLSCiphertext unified header is 0b001CSLEE; the low two bits are the epoch mod 4,
    // so epoch 3 (application data) is 0b001xxx11. Route application data through CoAP; route anything
    // else (a retransmitted epoch-2 client Finished) back to the state machine to be re-acknowledged.
    if (len >= 1 && (dgram[0] & 0xE0) == 0x20 && (dgram[0] & 0x03) == 3)
    {
        uint8_t req[COAPS_MSG_CAP];
        size_t req_len = 0;
        if (!det_dtls_conn_open_app(c, dgram, len, req, sizeof(req), &req_len))
            return 0; // replay, truncated, or not application data
        uint8_t resp[COAPS_MSG_CAP];
        size_t resp_len = det_coap_server_process(req, req_len, resp, sizeof(resp));
        if (!resp_len)
            return 0; // no response (e.g. a Non-confirmable message with no resource match)
        return (int)det_dtls_conn_seal_app(c, resp, resp_len, out, out_cap);
    }
    return det_dtls_conn_process(c, dgram, len, out, out_cap);
}

#endif // DETWS_ENABLE_DTLS && DETWS_ENABLE_COAP
