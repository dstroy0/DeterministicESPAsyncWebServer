// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea0183.h
 * @brief NMEA 0183 sentence codec (DETWS_ENABLE_NMEA0183) - the marine / GPS ASCII protocol.
 *
 * NMEA 0183 sentences look like `$GPGGA,123519,4807.038,N,...*47<CR><LF>`: a `$` (or `!` for
 * AIS-encapsulated), a comma-separated field list whose first field is the 5-char address
 * (2-char talker id + 3-char sentence type), a `*`, and a two-hex-digit XOR checksum. This
 * codec builds a sentence (adding the `$`, checksum, and CR/LF) and parses one (validating the
 * checksum and splitting the fields), with `det_strtof` / `det_strtol` field-value helpers.
 *
 * GPS / marine receivers are cheap UART breakouts, so on an ESP32 this is a plain
 * `HardwareSerial` link (commonly 4800 or 9600 baud); the UART is the application's. Pure
 * codec, host-tested. Bridge position / wind / depth data onto Wi-Fi.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_NMEA0183_H
#define DETERMINISTICESPASYNCWEBSERVER_NMEA0183_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_NMEA0183

#include <stddef.h>
#include <stdint.h>

/** @brief A parsed NMEA 0183 sentence. Field pointers reference the caller's buffer. */
struct Nmea0183
{
    char talker[3];                                ///< 2-char talker id (e.g. "GP") + NUL
    char type[4];                                  ///< 3-char sentence type (e.g. "GGA") + NUL
    uint8_t field_count;                           ///< number of fields, including field 0 (the address)
    const char *fields[DETWS_NMEA0183_MAX_FIELDS]; ///< field 0 is the address; data is 1..n
    uint8_t field_len[DETWS_NMEA0183_MAX_FIELDS];  ///< each field's length (0 for an empty field)
};

/** @brief XOR checksum over @p len octets (the sentence body between `$` and `*`). */
uint8_t nmea0183_checksum(const char *s, size_t len);

/**
 * @brief Build a sentence from @p body (e.g. "GPGGA,123519,..."): writes `$<body>*HH\r\n` and
 * NUL-terminates. Returns the length (excluding the NUL) or 0 on overflow.
 */
size_t nmea0183_build(char *buf, size_t cap, const char *body);

/**
 * @brief Parse a sentence: requires a leading `$`/`!`, validates the `*HH` XOR checksum, and
 * splits the comma-separated fields. Fills @p out (field 0 is the address; talker / type are
 * derived from it). Returns false on a bad frame or checksum.
 */
bool nmea0183_parse(const char *s, size_t len, Nmea0183 *out);

/** @brief Decode field @p idx as a float (false if absent / empty / non-numeric). */
bool nmea0183_field_float(const Nmea0183 *m, uint8_t idx, float *out);

/** @brief Decode field @p idx as a long integer (false if absent / empty / non-numeric). */
bool nmea0183_field_int(const Nmea0183 *m, uint8_t idx, long *out);

#endif // DETWS_ENABLE_NMEA0183
#endif // DETERMINISTICESPASYNCWEBSERVER_NMEA0183_H
