// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file iec60870.h
 * @brief IEC 60870-5-101 / -104 telecontrol (SCADA) codec (DETWS_ENABLE_IEC60870).
 *
 * The IEC 60870-5 utility-SCADA protocol in its two common transports:
 *  - **-104** over TCP: the APCI (`68 LEN` + 4 control octets) in its I / S / U formats,
 *    carrying an ASDU. I-format transfers numbered information (send/receive sequence
 *    numbers); S-format acknowledges; U-format does STARTDT / STOPDT / TESTFR.
 *  - **-101** over serial: the FT1.2 link frames - fixed length (`10 C A CS 16`) and variable
 *    length (`68 L L 68 C A <ASDU> CS 16`, 8-bit sum checksum), with a 1-octet link address.
 *  - The shared **ASDU** header (type id, variable structure qualifier, cause of transmission,
 *    common address) and the 3-octet Information Object Address used by both.
 *
 * The per-type-id information elements (single/double point, measured values, commands) are
 * the application's; this is the framing + ASDU-header layer, with named type-id / COT
 * constants for the common ones. Pure and host-tested. Bridge an RTU / outstation onto Wi-Fi:
 * run -104 over the shipped TCP stack, or -101 over a UART through an RS-232/485 transceiver.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_IEC60870_H
#define DETERMINISTICESPASYNCWEBSERVER_IEC60870_H

#include "ServerConfig.h"

#if DETWS_ENABLE_IEC60870

#include <stddef.h>
#include <stdint.h>

#define IEC_START_104 0x68u   ///< -104 APCI start octet (also the -101 variable-frame start)
#define IEC_START_FIXED 0x10u ///< -101 fixed-length frame start octet
#define IEC_STOP 0x16u        ///< -101 frame stop octet
#define IEC104_APCI_LEN 6u    ///< -104 control field: start + length + 4 control octets

// -104 U-format control commands (octet 1).
#define IEC104_STARTDT_ACT 0x07u
#define IEC104_STARTDT_CON 0x0Bu
#define IEC104_STOPDT_ACT 0x13u
#define IEC104_STOPDT_CON 0x23u
#define IEC104_TESTFR_ACT 0x43u
#define IEC104_TESTFR_CON 0x83u

// Common ASDU type identifications (IEC 60870-5-101 ch. 7.2.1).
#define IEC_TYPE_M_SP_NA_1 1   ///< single-point information
#define IEC_TYPE_M_DP_NA_1 3   ///< double-point information
#define IEC_TYPE_M_ME_NA_1 9   ///< measured value, normalized
#define IEC_TYPE_M_ME_NB_1 11  ///< measured value, scaled
#define IEC_TYPE_M_ME_NC_1 13  ///< measured value, short float
#define IEC_TYPE_C_SC_NA_1 45  ///< single command
#define IEC_TYPE_C_DC_NA_1 46  ///< double command
#define IEC_TYPE_M_EI_NA_1 70  ///< end of initialization
#define IEC_TYPE_C_IC_NA_1 100 ///< interrogation command
#define IEC_TYPE_C_CS_NA_1 103 ///< clock synchronization command

// Common causes of transmission (COT, low 6 bits).
#define IEC_COT_PERIODIC 1
#define IEC_COT_SPONTANEOUS 3
#define IEC_COT_REQUEST 5
#define IEC_COT_ACTIVATION 6
#define IEC_COT_ACT_CON 7
#define IEC_COT_DEACTIVATION 8
#define IEC_COT_ACT_TERM 10
#define IEC_COT_INTERROGATED 20

// -101 link-layer control-field function codes (low nibble; direction/FCB/FCV in the high bits).
#define IEC_FC_RESET_LINK 0x00        ///< reset of remote link (SEND/CONFIRM)
#define IEC_FC_TEST_LINK 0x02         ///< test function for link
#define IEC_FC_USER_DATA_CONFIRM 0x03 ///< user data, confirmed
#define IEC_FC_USER_DATA_NOREPLY 0x04 ///< user data, no reply
#define IEC_FC_REQUEST_CLASS1 0x0A    ///< request class 1 data
#define IEC_FC_REQUEST_CLASS2 0x0B    ///< request class 2 data

/** @brief -104 APCI frame formats. */
enum Iec104Format
{
    IEC104_I = 0, ///< information transfer (numbered; carries an ASDU)
    IEC104_S,     ///< supervisory (acknowledge only)
    IEC104_U,     ///< unnumbered (STARTDT / STOPDT / TESTFR)
};

/** @brief A parsed -104 APCI. */
struct Iec104Apci
{
    Iec104Format format;
    uint16_t ns;         ///< send sequence number (I-format)
    uint16_t nr;         ///< receive sequence number (I + S formats)
    uint8_t u_cmd;       ///< U-format command octet
    const uint8_t *asdu; ///< ASDU slice (I-format), or nullptr
    size_t asdu_len;
};

/** @brief A parsed ASDU header (the 6 octets before the information objects). */
struct IecAsduHeader
{
    uint8_t type_id;
    bool sq;              ///< structure qualifier: elements share one base IOA
    uint8_t count;        ///< number of information objects / elements (0..127)
    bool test;            ///< COT test bit
    bool negative;        ///< COT positive/negative confirm bit
    uint8_t cot;          ///< cause of transmission (low 6 bits)
    uint8_t orig_addr;    ///< originator address
    uint16_t common_addr; ///< common address of the ASDU
};

// --- IEC 60870-5-104 APCI (over TCP) ---

/** @brief Build an I-format APDU carrying @p asdu (numbered with @p ns / @p nr). */
size_t iec104_build_i(uint8_t *buf, size_t cap, uint16_t ns, uint16_t nr, const uint8_t *asdu, size_t asdu_len);

/** @brief Build an S-format (supervisory) APDU acknowledging up to @p nr. */
size_t iec104_build_s(uint8_t *buf, size_t cap, uint16_t nr);

/** @brief Build a U-format APDU with command @p u_cmd (IEC104_STARTDT_ACT, ...). */
size_t iec104_build_u(uint8_t *buf, size_t cap, uint8_t u_cmd);

/** @brief Parse one -104 APDU. Fills @p out and @p consumed (the whole APDU length). */
bool iec104_parse(const uint8_t *buf, size_t len, Iec104Apci *out, size_t *consumed);

// --- ASDU header + Information Object Address (shared by -101 and -104) ---

/** @brief Build the 6-octet ASDU header (type id, VSQ, COT[2], common address[2]). */
size_t iec_asdu_build_header(uint8_t *buf, size_t cap, const IecAsduHeader *h);

/** @brief Parse the 6-octet ASDU header; @p consumed is 6 on success. */
bool iec_asdu_parse_header(const uint8_t *buf, size_t len, IecAsduHeader *out, size_t *consumed);

/** @brief Write a 3-octet Information Object Address (little-endian). */
size_t iec_put_ioa(uint8_t *buf, size_t cap, uint32_t ioa);

/** @brief Read a 3-octet Information Object Address (little-endian). */
uint32_t iec_get_ioa(const uint8_t *p);

// --- IEC 60870-5-101 FT1.2 link frames (over serial) ---

/** @brief Build a fixed-length frame: 10 C A CS 16 (1-octet link address). */
size_t iec101_build_fixed(uint8_t *buf, size_t cap, uint8_t control, uint8_t addr);

/** @brief Build a variable-length frame: 68 L L 68 C A <asdu> CS 16 (1-octet link address). */
size_t iec101_build_variable(uint8_t *buf, size_t cap, uint8_t control, uint8_t addr, const uint8_t *asdu,
                             uint8_t asdu_len);

/** @brief A parsed -101 FT1.2 frame. */
struct Iec101Frame
{
    bool fixed;          ///< true => fixed-length frame (no ASDU)
    uint8_t control;     ///< link control field
    uint8_t addr;        ///< link address
    const uint8_t *asdu; ///< ASDU slice (variable frame), or nullptr
    uint8_t asdu_len;
};

/** @brief Parse one -101 FT1.2 frame (validates start/stop, doubled length, sum checksum). */
bool iec101_parse(const uint8_t *buf, size_t len, Iec101Frame *out, size_t *consumed);

#endif // DETWS_ENABLE_IEC60870
#endif // DETERMINISTICESPASYNCWEBSERVER_IEC60870_H
