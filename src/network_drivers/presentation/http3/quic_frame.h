// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_frame.h
 * @brief QUIC frame parsing and building (RFC 9000 sec 19).
 *
 * The payload of a QUIC packet is a sequence of frames, each `Frame Type (i)` followed by
 * type-specific fields coded with QUIC varints. This module reads one frame at a time into a
 * tagged QuicFrame and builds the frames a server sends. It covers the frames a minimal HTTP/3
 * server needs - PADDING, PING, ACK, CRYPTO, STREAM, MAX_DATA, CONNECTION_CLOSE, HANDSHAKE_DONE -
 * and reports the frame type for anything else so the caller can decide.
 *
 * Data-bearing frames (CRYPTO / STREAM / CONNECTION_CLOSE reason) point into the caller's packet
 * buffer; nothing is copied. Pure, zero heap, host-tested.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_QUIC_FRAME_H
#define DETERMINISTICESPASYNCWEBSERVER_QUIC_FRAME_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_HTTP3

#include <stddef.h>
#include <stdint.h>

/** @brief Frame types (RFC 9000 sec 19 / Table 3). STREAM is the range 0x08..0x0f. */
enum
{
    QUIC_FT_PADDING = 0x00,
    QUIC_FT_PING = 0x01,
    QUIC_FT_ACK = 0x02, ///< 0x02 (no ECN) .. 0x03 (with ECN counts)
    QUIC_FT_ACK_ECN = 0x03,
    QUIC_FT_CRYPTO = 0x06,
    QUIC_FT_STREAM = 0x08, ///< 0x08..0x0f; low 3 bits are OFF (0x04) / LEN (0x02) / FIN (0x01)
    QUIC_FT_MAX_DATA = 0x10,
    QUIC_FT_CONNECTION_CLOSE = 0x1c,     ///< transport-level close (carries the triggering frame type)
    QUIC_FT_CONNECTION_CLOSE_APP = 0x1d, ///< application-level close
    QUIC_FT_HANDSHAKE_DONE = 0x1e,
};

/** @brief STREAM frame type bits. */
enum
{
    QUIC_STREAM_FIN = 0x01,
    QUIC_STREAM_LEN = 0x02,
    QUIC_STREAM_OFF = 0x04,
};

/** @brief Transport error codes for CONNECTION_CLOSE (RFC 9000 sec 20.1). */
enum
{
    QUIC_ERR_NO_ERROR = 0x00,
    QUIC_ERR_INTERNAL = 0x01,
    QUIC_ERR_FLOW_CONTROL = 0x03,
    QUIC_ERR_STREAM_LIMIT = 0x04,
    QUIC_ERR_FRAME_ENCODING = 0x07,     ///< a frame could not be decoded
    QUIC_ERR_PROTOCOL_VIOLATION = 0x0a, ///< a frame/packet violated the protocol
    QUIC_ERR_CRYPTO_BASE = 0x0100,      ///< 0x0100 + the TLS alert code (RFC 9001 sec 4.8)
};

/** @brief One parsed frame. Pointer fields alias the input buffer (not copied). */
struct QuicFrame
{
    uint64_t type; ///< the frame type (STREAM reported as its exact 0x08..0x0f value)
    union {
        struct
        {
            uint64_t largest;     ///< Largest Acknowledged
            uint64_t delay;       ///< ACK Delay (encoded units)
            uint64_t range_count; ///< number of additional ACK Ranges (skipped, but counted)
            uint64_t first_range; ///< First ACK Range
        } ack;
        struct
        {
            uint64_t offset;
            uint64_t length;
            const uint8_t *data;
        } crypto;
        struct
        {
            uint64_t id;
            uint64_t offset; ///< 0 when the OFF bit is clear
            uint64_t length;
            const uint8_t *data;
            uint8_t fin;
        } stream;
        struct
        {
            uint64_t max;
        } max_data;
        struct
        {
            uint64_t error_code;
            uint64_t frame_type; ///< 0 for the application-level variant (0x1d)
            uint64_t reason_len;
            const uint8_t *reason;
            uint8_t app; ///< 1 if this was the application-level close (0x1d)
        } close;
    };
};

/** @brief Parse one frame at @p buf. @return bytes consumed, or 0 on malformed / truncated input. */
size_t quic_frame_parse(const uint8_t *buf, size_t len, QuicFrame *out);

// --- Builders (server side) ------------------------------------------------------------------

/** @brief @p n PADDING frames (n zero bytes). @return n, or 0 if it does not fit. */
size_t quic_build_padding(uint8_t *out, size_t cap, size_t n);
/** @brief A PING frame. */
size_t quic_build_ping(uint8_t *out, size_t cap);
/** @brief A HANDSHAKE_DONE frame. */
size_t quic_build_handshake_done(uint8_t *out, size_t cap);
/** @brief A single-range ACK frame (ACK Range Count 0): Largest, ACK Delay, First ACK Range. */
size_t quic_build_ack(uint8_t *out, size_t cap, uint64_t largest, uint64_t delay, uint64_t first_range);
/** @brief A CRYPTO frame carrying @p len bytes at stream @p offset. */
size_t quic_build_crypto(uint8_t *out, size_t cap, uint64_t offset, const uint8_t *data, size_t len);
/** @brief A STREAM frame (LEN always set; OFF set when @p offset > 0; FIN per @p fin). */
size_t quic_build_stream(uint8_t *out, size_t cap, uint64_t id, uint64_t offset, const uint8_t *data, size_t len,
                         bool fin);
/** @brief A MAX_DATA frame. */
size_t quic_build_max_data(uint8_t *out, size_t cap, uint64_t max);
/** @brief A transport CONNECTION_CLOSE (0x1c) with the triggering @p frame_type and a reason phrase. */
size_t quic_build_connection_close(uint8_t *out, size_t cap, uint64_t error_code, uint64_t frame_type,
                                   const char *reason, size_t reason_len);

#endif // DETWS_ENABLE_HTTP3
#endif // DETERMINISTICESPASYNCWEBSERVER_QUIC_FRAME_H
