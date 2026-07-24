// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ubx.h
 * @brief u-blox UBX binary protocol codec (DWS_ENABLE_UBX) - the GNSS receiver control/nav protocol.
 *
 * A UBX frame is `B5 62 <class> <id> <len-LE:2> <payload...> <CK_A> <CK_B>`: two sync chars, a
 * class/id message selector, a little-endian payload length, the payload, and an 8-bit Fletcher
 * checksum computed over everything between the sync chars and the checksum (class..payload end).
 * This codec builds a frame (adding the sync chars, length, and checksum), builds a zero-length
 * poll request (how UBX asks a receiver to emit a message), parses one complete frame (validating
 * the length and checksum), and - because a u-blox receiver multiplexes UBX with ASCII NMEA on the
 * same UART - provides a streaming demultiplexer that pulls UBX frames out of a byte stream and
 * hands every non-UBX byte back to the caller (for an NMEA line assembler). Little-endian payload
 * readers and an ACK helper decode the common replies. Pure codec, host-tested; the UART is the
 * application's (a plain HardwareSerial link, commonly 9600 baud).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_UBX_H
#define DETERMINISTICESPASYNCWEBSERVER_UBX_H

#include "ServerConfig.h"

#if DWS_ENABLE_UBX

#include <stddef.h>
#include <stdint.h>

#define DWS_UBX_SYNC1 0xB5u ///< first sync char
#define DWS_UBX_SYNC2 0x62u ///< second sync char

/** @brief A parsed UBX frame. @p payload aliases the caller's / stream's buffer (@p len octets). */
struct DwsUbx
{
    uint8_t cls;            ///< message class
    uint8_t id;             ///< message id
    uint16_t len;           ///< payload length
    const uint8_t *payload; ///< payload (len octets); references an external buffer
};

/**
 * @brief 8-bit Fletcher checksum over @p body (the class..payload-end span - everything the frame
 * checksums). Writes @p ck_a / @p ck_b.
 */
void dws_ubx_checksum(const uint8_t *body, size_t len, uint8_t *ck_a, uint8_t *ck_b);

/**
 * @brief Build a UBX frame into @p buf: `B5 62 cls id len(LE) payload CK_A CK_B`.
 * @return total frame length (8 + @p len) or 0 on overflow / bad args (@p payload may be NULL
 *         only when @p len is 0).
 */
size_t dws_ubx_build(uint8_t *buf, size_t cap, uint8_t cls, uint8_t id, const uint8_t *payload, uint16_t len);

/** @brief Build a zero-length poll request for (@p cls, @p id) - how UBX asks for a message. */
size_t dws_ubx_build_poll(uint8_t *buf, size_t cap, uint8_t cls, uint8_t id);

/**
 * @brief Parse exactly one complete UBX frame at the front of @p s. Validates the sync chars, the
 * declared length against @p len, and the checksum; fills @p out (payload aliases @p s). Returns
 * false on a short / malformed / bad-checksum frame.
 */
bool dws_ubx_parse(const uint8_t *s, size_t len, DwsUbx *out);

/**
 * @brief ACK helper. Returns 1 for UBX-ACK-ACK, 0 for UBX-ACK-NAK, -1 if @p m is not an ACK frame.
 * When it is an ACK, writes the acknowledged class/id (the payload's two octets) to @p acked_cls /
 * @p acked_id.
 */
int dws_ubx_ack(const DwsUbx *m, uint8_t *acked_cls, uint8_t *acked_id);

/** @brief Little-endian readers for a payload at byte offset @p off (caller bounds-checks @p off). */
uint16_t dws_ubx_u16(const uint8_t *p, size_t off);
uint32_t dws_ubx_u32(const uint8_t *p, size_t off);
int32_t dws_ubx_i32(const uint8_t *p, size_t off);

/** @brief Result of feeding one byte to the streaming demultiplexer. */
enum DwsUbxFeed
{
    DWS_UBX_NONE = 0,        ///< byte consumed inside a (partial or discarded) UBX frame
    DWS_UBX_FRAME = 1,       ///< a complete, checksum-valid frame is in @p out
    DWS_UBX_PASSTHROUGH = 2, ///< byte is not part of any UBX frame (returned in @p passthrough)
    DWS_UBX_OVERFLOW = 3     ///< a frame whose declared length exceeds DWS_UBX_MAX_PAYLOAD; skipped
};

/** @brief Streaming demultiplexer state for a mixed NMEA + UBX byte stream. Zero-initialize / init. */
struct DwsUbxStream
{
    uint8_t state;                    ///< internal parser state
    uint8_t cls;                      ///< class of the frame in progress
    uint8_t id;                       ///< id of the frame in progress
    uint16_t len;                     ///< declared payload length of the frame in progress
    uint16_t idx;                     ///< payload bytes received so far
    uint32_t skip;                    ///< bytes left to discard for an over-long frame
    uint8_t ck_a;                     ///< running checksum A
    uint8_t ck_b;                     ///< running checksum B
    uint8_t rx_ck_a;                  ///< received checksum A (awaiting B to compare)
    uint8_t buf[DWS_UBX_MAX_PAYLOAD]; ///< payload accumulator
};

/** @brief Reset a demux to the idle (hunting-for-sync) state. */
void dws_ubx_stream_init(DwsUbxStream *st);

/**
 * @brief Feed one byte. Returns a ::DwsUbxFeed code: DWS_UBX_FRAME (@p out filled, payload aliases
 * the stream buffer, valid until the next feed), DWS_UBX_PASSTHROUGH (@p passthrough set to a
 * non-UBX byte - hand it to your NMEA assembler), DWS_UBX_OVERFLOW (an over-long frame was skipped),
 * or DWS_UBX_NONE.
 */
int dws_ubx_stream_feed(DwsUbxStream *st, uint8_t b, DwsUbx *out, uint8_t *passthrough);

#endif // DWS_ENABLE_UBX
#endif // DETERMINISTICESPASYNCWEBSERVER_UBX_H
