// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dmx.h
 * @brief DMX512 framing + RDM (ANSI E1.20) management codec (DWS_ENABLE_DMX).
 *
 * DMX512 (lighting / stage control over RS-485) is positional: after a break, a start code
 * octet (0x00 for dimmer data) is followed by up to 512 channel slots, with no checksum or
 * in-frame addressing. This codec assembles / reads that slot array, and implements **RDM**
 * (Remote Device Management, ANSI E1.20) - the addressed management layer that shares the
 * DMX wire: a real packet with 48-bit source / destination UIDs, a command class + parameter
 * id, and a 16-bit additive checksum.
 *
 * The break + RS-485 direction are the application's (a `MAX485`-class transceiver on a UART
 * at 250 kbit/s, 8N2). This is the byte-level framing layer. Pure and host-tested. Bridge a
 * lighting rig onto Wi-Fi: drive DMX slots or discover / configure RDM fixtures from the web.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DMX_H
#define DETERMINISTICESPASYNCWEBSERVER_DMX_H

#include "ServerConfig.h"

#if DWS_ENABLE_DMX

#include <stddef.h>
#include <stdint.h>

#define DMX_MAX_CHANNELS 512u ///< slots per DMX512 universe
#define DMX_SC_DIMMER 0x00u   ///< start code for standard dimmer data

#define RDM_SC 0xCCu     ///< RDM start code (SC_RDM)
#define RDM_SUB_SC 0x01u ///< RDM sub-start code (SC_SUB_MESSAGE)
#define RDM_OVERHEAD 26u ///< full packet octets with PDL 0 (24-octet message + 2 checksum)

// RDM command classes.
#define RDM_CC_DISCOVERY 0x10u
#define RDM_CC_DISCOVERY_RESPONSE 0x11u
#define RDM_CC_GET 0x20u
#define RDM_CC_GET_RESPONSE 0x21u
#define RDM_CC_SET 0x30u
#define RDM_CC_SET_RESPONSE 0x31u

// RDM response types (carried in the port-id / response-type octet of a response).
#define RDM_RESPONSE_ACK 0x00u
#define RDM_RESPONSE_ACK_TIMER 0x01u
#define RDM_RESPONSE_NACK_REASON 0x02u
#define RDM_RESPONSE_ACK_OVERFLOW 0x03u

// A few common RDM parameter ids (PIDs).
#define RDM_PID_DISC_UNIQUE_BRANCH 0x0001u
#define RDM_PID_DISC_MUTE 0x0002u
#define RDM_PID_DISC_UN_MUTE 0x0003u
#define RDM_PID_SUPPORTED_PARAMETERS 0x0050u
#define RDM_PID_DEVICE_INFO 0x0060u
#define RDM_PID_DMX_START_ADDRESS 0x00F0u
#define RDM_PID_IDENTIFY_DEVICE 0x1000u

// --- DMX512 ---

/**
 * @brief Assemble a DMX512 packet body: [start code][channel slots]. @p n <= 512.
 * Returns the byte count (1 + n) or 0 on overflow. The break is the transport's job.
 */
size_t dws_dmx_build(uint8_t *buf, size_t cap, uint8_t start_code, const uint8_t *channels, uint16_t n);

/**
 * @brief Read channel @p ch (1-based, per DMX convention) from a received packet body.
 * Returns the slot value, or 0 if @p ch is out of range / not present.
 */
uint8_t dws_dmx_get_channel(const uint8_t *buf, size_t len, uint16_t ch);

// --- RDM (ANSI E1.20) ---

/** @brief A parsed / to-be-built RDM packet. UIDs are 48-bit (manufacturer<<32 | device). */
struct RdmPacket
{
    uint64_t dest_uid;
    uint64_t src_uid;
    uint8_t tn;           ///< transaction number
    uint8_t port_id;      ///< port id (request) / response type (response)
    uint8_t msg_count;    ///< queued message count
    uint16_t sub_device;  ///< sub-device (0 = root)
    uint8_t cc;           ///< command class (RDM_CC_*)
    uint16_t pid;         ///< parameter id
    uint8_t pdl;          ///< parameter data length
    const uint8_t *pdata; ///< parameter data (points into the parsed buffer); nullptr when pdl 0
};

/** @brief Compose a 48-bit RDM UID from a manufacturer id and a device id. */
uint64_t dws_rdm_uid(uint16_t manufacturer, uint32_t device);

/** @brief 16-bit additive checksum over @p len octets (RDM message block). */
uint16_t dws_rdm_checksum(const uint8_t *buf, size_t len);

/**
 * @brief Build a full RDM packet (incl. the trailing 16-bit checksum) from @p p and its
 * parameter data. Returns the total length (26 + pdl) or 0 on overflow.
 */
size_t dws_rdm_build(uint8_t *buf, size_t cap, const RdmPacket *p, const uint8_t *pdata, uint8_t pdl);

/**
 * @brief Parse an RDM packet: validates the start codes, the message length vs PDL, and the
 * checksum. Fills @p out and @p consumed (the whole packet length).
 */
bool dws_rdm_parse(const uint8_t *buf, size_t len, RdmPacket *out, size_t *consumed);

#endif // DWS_ENABLE_DMX
#endif // DETERMINISTICESPASYNCWEBSERVER_DMX_H
