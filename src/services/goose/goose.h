// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file goose.h
 * @brief IEC 61850 GOOSE publisher codec (DETWS_ENABLE_GOOSE).
 *
 * GOOSE (Generic Object Oriented Substation Event) is the fast raw-L2 multicast publish IEC 61850 uses
 * for protection trips. A GOOSE frame is an Ethernet frame (ethertype 0x88B8, see services/rawl2) with
 * an 8-octet GOOSE header (APPID, length, two reserved) followed by the BER-encoded `IECGoosePdu`:
 *
 *   61 len  { 80 gocbRef  81 timeAllowedToLive  82 datSet  83 goID  84 t(UtcTime,8)  85 stNum
 *             86 sqNum  87 simulation(BOOL)  88 confRev  89 ndsCom(BOOL)  8A numDatSetEntries  AB allData }
 *
 * This builds the control PDU (all fields above) with `allData` supplied as a caller-encoded BER blob
 * (a SEQUENCE of Data elements), then wraps it in the GOOSE header + Ethernet frame. Pure, zero heap,
 * no stdlib, host-testable; the raw-L2 transmit (`esp_eth_transmit`) is the device step.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_GOOSE_H
#define DETERMINISTICESPASYNCWEBSERVER_GOOSE_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_GOOSE

/** @brief The GOOSE control fields (strings are borrowed, not copied). */
struct DetwsGoose
{
    const char *gocb_ref;          ///< the GOOSE control-block reference.
    uint32_t time_allowed_to_live; ///< ms the subscriber should keep this valid.
    const char *dat_set;           ///< the dataset reference.
    const char *go_id;             ///< the GOOSE id.
    const uint8_t *t;              ///< 8-octet UtcTime (seconds-since-epoch + fraction + quality).
    uint32_t st_num;               ///< state number (bumped on a data change).
    uint32_t sq_num;               ///< sequence number (bumped on each resend).
    bool simulation;               ///< test/simulation flag.
    uint32_t conf_rev;             ///< configuration revision.
    bool nds_com;                  ///< needs-commissioning flag.
    uint32_t num_entries;          ///< number of dataset entries in allData.
    const uint8_t *all_data;       ///< pre-encoded BER SEQUENCE-of-Data body (contents of the AB field).
    size_t all_data_len;
};

/**
 * @brief Build the BER IECGoosePdu (the `61 ...` structure) into @p out. @return length, or 0 on overflow.
 */
size_t detws_goose_pdu(const DetwsGoose *g, uint8_t *out, size_t cap);

/**
 * @brief Build a full GOOSE Ethernet frame: [dst][src][88B8][APPID len rsvd rsvd][IECGoosePdu].
 * @param appid the GOOSE APPID.
 * @return the frame length, or 0 on overflow. Requires DETWS_ENABLE_RAWL2 for the Ethernet framing.
 */
size_t detws_goose_frame(const uint8_t *dst, const uint8_t *src, uint16_t appid, const DetwsGoose *g, uint8_t *out,
                         size_t cap);

#endif // DETWS_ENABLE_GOOSE
#endif // DETERMINISTICESPASYNCWEBSERVER_GOOSE_H
