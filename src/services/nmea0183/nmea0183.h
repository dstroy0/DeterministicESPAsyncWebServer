// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nmea0183.h
 * @brief NMEA 0183 sentence codec (DWS_ENABLE_NMEA0183) - the marine / GPS ASCII protocol.
 *
 * NMEA 0183 sentences look like `$GPGGA,123519,4807.038,N,...*47<CR><LF>`: a `$` (or `!` for
 * AIS-encapsulated), a comma-separated field list whose first field is the 5-char address
 * (2-char talker id + 3-char sentence type), a `*`, and a two-hex-digit XOR checksum. This
 * codec builds a sentence (adding the `$`, checksum, and CR/LF) and parses one (validating the
 * checksum and splitting the fields), with `dws_strtof` / `dws_strtol` field-value helpers.
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

#include "ServerConfig.h"

#if DWS_ENABLE_NMEA0183

#include <stddef.h>
#include <stdint.h>

/** @brief A parsed NMEA 0183 sentence. Field pointers reference the caller's buffer. */
struct Nmea0183
{
    char talker[3];                              ///< 2-char talker id (e.g. "GP") + NUL
    char type[4];                                ///< 3-char sentence type (e.g. "GGA") + NUL
    uint8_t field_count;                         ///< number of fields, including field 0 (the address)
    const char *fields[DWS_NMEA0183_MAX_FIELDS]; ///< field 0 is the address; data is 1..n
    uint8_t field_len[DWS_NMEA0183_MAX_FIELDS];  ///< each field's length (0 for an empty field)
};

/** @brief XOR checksum over @p len octets (the sentence body between `$` and `*`). */
uint8_t dws_nmea0183_checksum(const char *s, size_t len);

/**
 * @brief Build a sentence from @p body (e.g. "GPGGA,123519,..."): writes `$<body>*HH\r\n` and
 * NUL-terminates. Returns the length (excluding the NUL) or 0 on overflow.
 */
size_t dws_nmea0183_build(char *buf, size_t cap, const char *body);

/**
 * @brief Parse a sentence: requires a leading `$`/`!`, validates the `*HH` XOR checksum, and
 * splits the comma-separated fields. Fills @p out (field 0 is the address; talker / type are
 * derived from it). Returns false on a bad frame or checksum.
 */
bool dws_nmea0183_parse(const char *s, size_t len, Nmea0183 *out);

/** @brief Decode field @p idx as a float (false if absent / empty / non-numeric). */
bool dws_nmea0183_field_float(const Nmea0183 *m, uint8_t idx, float *out);

/** @brief Decode field @p idx as a long integer (false if absent / empty / non-numeric). */
bool dws_nmea0183_field_int(const Nmea0183 *m, uint8_t idx, long *out);

// -- typed decoders for the two common GPS position sentences --
//
// These lift a parsed sentence (dws_nmea0183_parse) into a position struct: the ddmm.mmmm coordinates
// become signed decimal degrees (hemisphere-adjusted), and the hhmmss.ss / ddmmyy fields split into their
// components. Coordinates are read through the float field helper, so their precision is float's (~1 m).

/** @brief Decoded GGA (fix data). */
struct DwsNmeaGga
{
    uint8_t hour, minute; ///< UTC time of the fix
    float second;
    double lat_deg;      ///< latitude in signed decimal degrees (+ N, - S); 0 if absent
    double lon_deg;      ///< longitude in signed decimal degrees (+ E, - W); 0 if absent
    uint8_t fix_quality; ///< 0 = no fix, 1 = GPS, 2 = DGPS, ... (field 6)
    uint8_t num_sats;    ///< satellites used in the fix
    float hdop;          ///< horizontal dilution of precision
    float alt_m;         ///< altitude above mean sea level (metres)
};

/** @brief Decoded RMC (recommended minimum: position + velocity + date). */
struct DwsNmeaRmc
{
    bool valid;           ///< true when the status field is 'A' (data valid), false for 'V' (warning)
    uint8_t hour, minute; ///< UTC time
    float second;
    uint8_t day, month, year; ///< UTC date (year is the 2-digit field value, 0..99)
    double lat_deg;           ///< latitude in signed decimal degrees
    double lon_deg;           ///< longitude in signed decimal degrees
    float speed_knots;        ///< speed over ground (knots)
    float course_deg;         ///< course over ground (degrees true)
};

/**
 * @brief Decode a parsed GGA sentence into @p out. @return true iff @p m is a GGA sentence with enough
 *        fields; false (and @p out untouched) otherwise. Empty optional fields read back as 0.
 */
bool dws_nmea0183_parse_gga(const Nmea0183 *m, DwsNmeaGga *out);

/**
 * @brief Decode a parsed RMC sentence into @p out. @return true iff @p m is an RMC sentence with enough
 *        fields; false otherwise. @c valid reflects the A/V status field (a 'V' sentence still decodes).
 */
bool dws_nmea0183_parse_rmc(const Nmea0183 *m, DwsNmeaRmc *out);

/** @brief One satellite record from a GSV sentence. */
struct DwsNmeaGsvSat
{
    uint8_t prn;      ///< satellite PRN / id
    int16_t elev_deg; ///< elevation (degrees, 0..90)
    int16_t azim_deg; ///< azimuth (degrees true, 0..359)
    uint8_t snr_db;   ///< signal-to-noise ratio (dB-Hz), valid only when @ref snr_valid
    bool snr_valid;   ///< false when the SNR field is blank (the satellite is not being tracked)
};

/** @brief Decoded GSV (satellites in view). One sentence carries up to four satellite records; a full sky
 *  view spans @ref total_msgs sentences. */
struct DwsNmeaGsv
{
    uint8_t total_msgs;   ///< total GSV sentences in this cycle
    uint8_t msg_num;      ///< this sentence's number (1-based)
    uint8_t sats_in_view; ///< total satellites in view across the cycle
    uint8_t sat_count;    ///< satellite records present in THIS sentence (0..4)
    DwsNmeaGsvSat sats[4];
};

/**
 * @brief Decode a parsed GSV sentence into @p out. @return true iff @p m is a GSV sentence with at least
 *        the 3-field header; the per-satellite records present in this sentence are filled (0..4).
 */
bool dws_nmea0183_parse_gsv(const Nmea0183 *m, DwsNmeaGsv *out);

/** @brief Decoded ZDA (UTC time + calendar date + local zone offset). Unlike RMC this carries the full
 *  4-digit year, so it is the sentence to read for wall-clock time sync. */
struct DwsNmeaZda
{
    uint8_t hour, minute; ///< UTC time
    float second;
    uint8_t day, month;   ///< UTC date
    uint16_t year;        ///< UTC year (4-digit)
    int8_t zone_hours;    ///< local zone offset hours (-13..+13); 0 if the field is absent
    uint8_t zone_minutes; ///< local zone offset minutes (0..59); 0 if the field is absent
};

/**
 * @brief Decode a parsed ZDA sentence into @p out. @return true iff @p m is a ZDA sentence with at least
 *        the time / day / month / year fields; false otherwise. The zone offset reads back 0 when absent.
 */
bool dws_nmea0183_parse_zda(const Nmea0183 *m, DwsNmeaZda *out);

/** @brief Decoded VTG (course over ground + ground speed). The course-over-ground vector complements the
 *  RMC/GGA position - it is the sentence to read for heading and speed. */
struct DwsNmeaVtg
{
    float course_true_deg; ///< course over ground, degrees true (0 if the field is absent)
    float course_mag_deg;  ///< course over ground, degrees magnetic (0 if absent)
    float speed_knots;     ///< speed over ground in knots (0 if absent)
    float speed_kmh;       ///< speed over ground in km/h (0 if absent)
    char mode;             ///< NMEA 2.3+ mode indicator ('A'/'D'/'E'/'N'), or '\0' when the field is absent
};

/**
 * @brief Decode a parsed VTG sentence into @p out. @return true iff @p m is a VTG sentence with at least the
 *        course / speed fields (through the km/h unit); false otherwise. The mode reads back '\0' when absent.
 */
bool dws_nmea0183_parse_vtg(const Nmea0183 *m, DwsNmeaVtg *out);

/** @brief Decoded GSA (GPS DOP + the satellites active in the fix). Where GSV lists satellites in view and
 *  GGA gives the fix quality, GSA gives the 2D/3D fix mode, which satellites were used, and all three DOPs. */
struct DwsNmeaGsa
{
    char mode;         ///< selection mode: 'M' manual, 'A' automatic 2D/3D
    uint8_t fix_type;  ///< 1 = no fix, 2 = 2D, 3 = 3D
    uint8_t sat_count; ///< number of satellite PRNs present (0..12)
    uint8_t sats[12];  ///< PRNs of the satellites used in the fix
    float pdop;        ///< position (3D) dilution of precision
    float hdop;        ///< horizontal dilution of precision
    float vdop;        ///< vertical dilution of precision
};

/**
 * @brief Decode a parsed GSA sentence into @p out. @return true iff @p m is a GSA sentence with the full
 *        field set (through VDOP); false otherwise. Empty PRN slots are skipped (not counted).
 */
bool dws_nmea0183_parse_gsa(const Nmea0183 *m, DwsNmeaGsa *out);

/** @brief Decoded MWV (wind speed + angle): the standard wind-instrument sentence. */
struct DwsNmeaMwv
{
    float wind_angle_deg; ///< wind angle (0..359 degrees)
    char reference;       ///< 'R' relative (apparent) or 'T' true, or '\0' if the field is absent
    float wind_speed;     ///< wind speed, in the units given by @ref speed_units
    char speed_units;     ///< 'K' km/h, 'M' m/s, 'N' knots, or '\0' if absent
    bool valid;           ///< status field: true for 'A' (valid), false for 'V'
};

/**
 * @brief Decode a parsed MWV sentence into @p out. @return true iff @p m is an MWV sentence with the angle /
 *        reference / speed / units / status fields; false otherwise.
 */
bool dws_nmea0183_parse_mwv(const Nmea0183 *m, DwsNmeaMwv *out);

/** @brief Decoded DPT (depth of water): depth relative to the transducer plus the transducer offset. */
struct DwsNmeaDpt
{
    float depth_m;  ///< water depth relative to the transducer (meters)
    float offset_m; ///< transducer offset (meters): + = transducer to waterline, - = transducer to keel
    bool has_range; ///< true when the optional maximum-range-scale field is present
    float range_m;  ///< maximum range scale in use (meters), valid only when @ref has_range
};

/**
 * @brief Decode a parsed DPT sentence into @p out. @return true iff @p m is a DPT sentence with the depth
 *        and offset fields; false otherwise. The optional range scale sets @ref DwsNmeaDpt::has_range.
 */
bool dws_nmea0183_parse_dpt(const Nmea0183 *m, DwsNmeaDpt *out);

#endif // DWS_ENABLE_NMEA0183
#endif // DETERMINISTICESPASYNCWEBSERVER_NMEA0183_H
