// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file cbor.h
 * @brief Layer 6 (Presentation) - zero-heap CBOR (RFC 8949) encoder.
 *
 * A streaming encoder that writes directly into a caller-provided buffer (no
 * heap), the binary counterpart to the JSON writer. Emit definite-length arrays
 * and maps by writing the header (cbor_array / cbor_map with the item count) then
 * that many items (twice that for a map: key, value, key, value, ...).
 *
 * Overflow is tracked, not crashed on: writes past the buffer set the overflow
 * flag and stop, while cbor_len() keeps counting the bytes the full payload would
 * need, so a caller can size the buffer and check cbor_ok().
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CBOR_H
#define DETERMINISTICESPASYNCWEBSERVER_CBOR_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_CBOR

/** @brief CBOR encoder state over a caller-provided buffer. */
struct CborWriter
{
    uint8_t *buf;  ///< destination buffer.
    size_t cap;    ///< buffer capacity in bytes.
    size_t pos;    ///< bytes the payload needs so far (may exceed cap on overflow).
    bool overflow; ///< true once a write did not fit.
};

/** @brief Bind a writer to @p buf (capacity @p cap) and reset it. */
void cbor_init(CborWriter *w, uint8_t *buf, size_t cap);

/** @brief Encoded length in bytes (what the full payload needs; compare to cap). */
size_t cbor_len(const CborWriter *w);

/** @brief True if every value fit in the buffer. */
bool cbor_ok(const CborWriter *w);

void cbor_uint(CborWriter *w, uint64_t v);                       ///< unsigned integer
void cbor_int(CborWriter *w, int64_t v);                         ///< signed integer
void cbor_bytes(CborWriter *w, const uint8_t *data, size_t len); ///< byte string
void cbor_text(CborWriter *w, const char *s);                    ///< text string (null-terminated)
void cbor_text_n(CborWriter *w, const char *s, size_t len);      ///< text string (explicit length)
void cbor_bool(CborWriter *w, bool b);                           ///< true / false
void cbor_null(CborWriter *w);                                   ///< null
void cbor_float(CborWriter *w, float f);                         ///< IEEE-754 single (major 7, 0xfa)
void cbor_array(CborWriter *w, size_t count);                    ///< definite-length array header
void cbor_map(CborWriter *w, size_t count);                      ///< definite-length map header

#endif // DETWS_ENABLE_CBOR
#endif // DETERMINISTICESPASYNCWEBSERVER_CBOR_H
