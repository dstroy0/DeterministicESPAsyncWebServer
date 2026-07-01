// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file redis_resp.h
 * @brief Redis RESP2 wire codec (DETWS_ENABLE_REDIS) - zero-heap command encoder
 *        + reply parser, so a device can talk to a Redis server with the shipped
 *        outbound client transport.
 *
 * RESP2 (https://redis.io/docs/latest/develop/reference/protocol-spec):
 *  - A command is an array of bulk strings: `*<n>\r\n$<len>\r\n<arg>\r\n...`.
 *  - A reply is one of: simple string `+OK\r\n`, error `-ERR ...\r\n`, integer
 *    `:<n>\r\n`, bulk string `$<len>\r\n<bytes>\r\n` (`$-1\r\n` = nil), or array
 *    `*<n>\r\n<elements>` (`*-1\r\n` = nil).
 *
 * The parser is a cursor: it decodes one value at the buffer head and reports how
 * many bytes it consumed; for an array it reports the element count and the caller
 * parses each element from the remaining bytes (no recursion state, no heap).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_REDIS_RESP_H
#define DETERMINISTICESPASYNCWEBSERVER_REDIS_RESP_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_REDIS

#include "shared_primitives/shim.h"

/** @brief RESP2 reply value types. */
enum RespType
{
    RESP_SIMPLE,  ///< simple string (+); value in str/str_len
    RESP_ERROR,   ///< error (-); message in str/str_len
    RESP_INTEGER, ///< integer (:); value in ival
    RESP_BULK,    ///< bulk string ($); bytes in str/str_len
    RESP_ARRAY,   ///< array (*); element count in count (parse each from the remainder)
    RESP_NIL,     ///< null bulk string ($-1) or null array (*-1)
};

/** @brief One decoded RESP value. String fields point INTO the source buffer (not copied). */
struct RespReply
{
    RespType type;
    int64_t ival;    ///< value for RESP_INTEGER; element count for RESP_ARRAY
    const char *str; ///< bytes for RESP_SIMPLE / RESP_ERROR / RESP_BULK (not NUL-terminated)
    size_t str_len;  ///< length of @ref str
    int64_t count;   ///< element count for RESP_ARRAY (also mirrored in ival)
};

/**
 * @brief Encode a command (array of bulk strings) into @p buf.
 *
 * @param args     argument byte pointers (argv).
 * @param arg_lens per-arg byte lengths, or nullptr to use strlen() on each arg.
 * @param argc     number of arguments.
 * @return bytes written (excluding any NUL), or 0 on overflow / bad input.
 */
size_t resp_encode_command(char *buf, size_t cap, const char *const *args, const size_t *arg_lens, size_t argc);

/**
 * @brief Parse one RESP value at the head of [buf, buf+len).
 *
 * @param consumed receives the number of bytes the value occupied (so the caller
 *                 can advance to the next value / array element).
 * @return true on a complete value; false if the buffer holds an incomplete or
 *         malformed value (then @p out / @p consumed are unspecified).
 */
bool resp_parse(const uint8_t *buf, size_t len, RespReply *out, size_t *consumed);

#endif // DETWS_ENABLE_REDIS

#endif // DETERMINISTICESPASYNCWEBSERVER_REDIS_RESP_H
