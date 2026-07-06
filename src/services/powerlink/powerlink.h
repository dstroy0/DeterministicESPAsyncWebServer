// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file powerlink.h
 * @brief Ethernet POWERLINK (EPSG) basic frame codec (DETWS_ENABLE_POWERLINK).
 *
 * Ethernet POWERLINK is the EPSG real-time managed-node bus over raw L2 (ethertype 0x88AB, on the
 * shipped services/rawl2). The Managing Node (MN) runs an isochronous cycle: it multicasts a **SoC**
 * (Start of Cycle), unicasts a **PReq** (Poll Request) to each Controlled Node (CN), each CN answers with
 * a **PRes** (Poll Response) carrying its process data, then an **SoA** (Start of Async) opens the async
 * phase. Every EPL basic frame is:
 *
 *     [messageType : 1][destination node : 1][source node : 1][payload...]
 *
 * This builds and parses those frames (the four cyclic message types + the node addressing), so the MN
 * schedules the cycle and a CN answers with its PRes process image. Pure, zero heap, no stdlib,
 * host-testable; the raw-L2 transmit + the isochronous timing (the preempting-task model) are the device
 * step.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_POWERLINK_H
#define DETERMINISTICESPASYNCWEBSERVER_POWERLINK_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_POWERLINK

/** @brief EPL message types (EPSG DS 301). */
enum
{
    EPL_MSG_SOC = 0x01,        ///< Start of Cycle (MN -> all, multicast).
    EPL_MSG_PREQ = 0x03,       ///< Poll Request (MN -> CN, unicast).
    EPL_MSG_PRES = 0x04,       ///< Poll Response (CN -> all, multicast, carries process data).
    EPL_MSG_SOA = 0x05,        ///< Start of Async (MN -> all).
    EPL_MSG_ASND = 0x06,       ///< Async Send.
    EPL_NODE_BROADCAST = 0xFF, ///< broadcast node id (SoC/SoA destination).
    EPL_NODE_MN = 0xF0         ///< the Managing Node id (240).
};

/**
 * @brief Build an EPL basic frame: [messageType][dest][source][payload...].
 * @return the frame length (3 + payload_len), or 0 on overflow / bad args.
 */
size_t detws_epl_build(uint8_t msg_type, uint8_t dest, uint8_t source, const uint8_t *payload, size_t payload_len,
                       uint8_t *out, size_t cap);

/** @brief Convenience: build an SoC (MN -> broadcast, no payload). */
size_t detws_epl_soc(uint8_t source, uint8_t *out, size_t cap);

/** @brief Convenience: build a PReq to a CN carrying its output process image. */
size_t detws_epl_preq(uint8_t dest_cn, uint8_t source, const uint8_t *pdo, size_t pdo_len, uint8_t *out, size_t cap);

/** @brief Convenience: build a PRes from a CN carrying its input process image (multicast). */
size_t detws_epl_pres(uint8_t source_cn, const uint8_t *pdo, size_t pdo_len, uint8_t *out, size_t cap);

/** @brief A parsed EPL basic frame (payload points into the input). */
struct EplFrame
{
    uint8_t msg_type;
    uint8_t dest;
    uint8_t source;
    const uint8_t *payload;
    size_t payload_len;
};

/** @brief Parse an EPL basic frame. @return true if @p len >= 3 and the message type is known. */
bool detws_epl_parse(const uint8_t *frame, size_t len, EplFrame *out);

#endif // DETWS_ENABLE_POWERLINK
#endif // DETERMINISTICESPASYNCWEBSERVER_POWERLINK_H
