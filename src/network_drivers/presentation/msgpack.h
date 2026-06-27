// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file msgpack.h
 * @brief Layer 6 (Presentation) - zero-heap MessagePack encoder.
 *
 * A streaming encoder that writes directly into a caller-provided buffer (no
 * heap), the MessagePack-format sibling of the CBOR / JSON writers. Each value is
 * emitted in the shortest MessagePack form (fixint / fixstr / fixarray / fixmap
 * where possible). Emit definite-length arrays and maps by writing the header
 * (msgpack_array / msgpack_map with the item count) then that many items (twice
 * that for a map: key, value, key, value, ...).
 *
 * Overflow is tracked, not crashed on: writes past the buffer set the overflow
 * flag and stop, while msgpack_len() keeps counting the bytes the full payload
 * would need, so a caller can size the buffer and check msgpack_ok().
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MSGPACK_H
#define DETERMINISTICESPASYNCWEBSERVER_MSGPACK_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_MSGPACK

/** @brief MessagePack encoder state over a caller-provided buffer. */
struct MsgpackWriter
{
    uint8_t *buf;  ///< destination buffer.
    size_t cap;    ///< buffer capacity in bytes.
    size_t pos;    ///< bytes the payload needs so far (may exceed cap on overflow).
    bool overflow; ///< true once a write did not fit.
};

/** @brief Bind a writer to @p buf (capacity @p cap) and reset it. */
void msgpack_init(MsgpackWriter *w, uint8_t *buf, size_t cap);

/** @brief Encoded length in bytes (what the full payload needs; compare to cap). */
size_t msgpack_len(const MsgpackWriter *w);

/** @brief True if every value fit in the buffer. */
bool msgpack_ok(const MsgpackWriter *w);

void msgpack_uint(MsgpackWriter *w, uint64_t v);                       ///< unsigned integer
void msgpack_int(MsgpackWriter *w, int64_t v);                         ///< signed integer
void msgpack_bytes(MsgpackWriter *w, const uint8_t *data, size_t len); ///< binary (bin family)
void msgpack_str(MsgpackWriter *w, const char *s);                     ///< UTF-8 string (null-terminated)
void msgpack_str_n(MsgpackWriter *w, const char *s, size_t len);       ///< UTF-8 string (explicit length)
void msgpack_bool(MsgpackWriter *w, bool b);                           ///< true / false
void msgpack_nil(MsgpackWriter *w);                                    ///< nil
void msgpack_float(MsgpackWriter *w, float f);                         ///< IEEE-754 single (float32, 0xca)
void msgpack_array(MsgpackWriter *w, size_t count);                    ///< array header
void msgpack_map(MsgpackWriter *w, size_t count);                      ///< map header

#endif // DETWS_ENABLE_MSGPACK
#endif // DETERMINISTICESPASYNCWEBSERVER_MSGPACK_H
