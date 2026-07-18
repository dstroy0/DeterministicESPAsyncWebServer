// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file enocean.h
 * @brief EnOcean ESP3 serial codec (DWS_ENABLE_ENOCEAN) - energy-harvesting 868 MHz.
 *
 * A UART telegram codec for EnOcean Serial Protocol 3 (ESP3), the framing every USB /
 * serial EnOcean gateway (TCM 310 / USB 300) speaks. A telegram is:
 *
 *   0x55 | data-len (2, big-endian) | opt-len (1) | packet-type (1) | CRC8H
 *        | data[data-len] | opt[opt-len] | CRC8D
 *
 * where CRC8H protects the 4 header bytes and CRC8D protects the data + optional data (both
 * CRC-8, polynomial 0x07, init 0). esp3_parse() frames one telegram out of a byte stream,
 * resynchronising on a bad sync / CRC, and esp3_build() assembles one. This is the radio-
 * plugin codec for the gateway: an inbound RADIO_ERP1 telegram carries a sender id (its
 * source address) and payload; bridge it northbound with dws_gateway_uplink(). Pure - you feed
 * it the UART bytes - so it is fully host-testable. See example 13.EnOceanGateway.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_ENOCEAN_H
#define DETERMINISTICESPASYNCWEBSERVER_ENOCEAN_H

#include "ServerConfig.h"

#if DWS_ENABLE_ENOCEAN

#include <stddef.h>
#include <stdint.h>

/** @brief ESP3 sync byte that starts every telegram. */
#define ESP3_SYNC 0x55

/** @brief ESP3 packet types (the common ones). */
enum class esp3_type : uint8_t
{
    ESP3_RADIO_ERP1 = 0x01,
    ESP3_RESPONSE = 0x02,
    ESP3_RADIO_SUB_TEL = 0x03,
    ESP3_EVENT = 0x04,
    ESP3_COMMON_COMMAND = 0x05,
    ESP3_SMART_ACK = 0x06,
    ESP3_REMOTE_MAN = 0x07,
    ESP3_RADIO_ERP2 = 0x0A,
};

/** @brief A parsed ESP3 telegram (pointers alias the caller's buffer). */
struct esp3_packet
{
    const uint8_t *data; ///< data field
    const uint8_t *opt;  ///< optional-data field
    uint16_t data_len;   ///< data length
    uint8_t opt_len;     ///< optional-data length
    esp3_type type;      ///< packet type (esp3_type)
};

/** @brief CRC-8 used by ESP3 (polynomial 0x07, MSB-first, init 0x00). */
uint8_t esp3_crc8(const uint8_t *buf, uint16_t len);

/**
 * @brief Frame one ESP3 telegram from the front of @p raw.
 * @return the telegram length consumed (> 0, fields in @p out) if a valid telegram is at
 *         @p raw[0]; 0 if more bytes are needed to complete it; -1 if @p raw[0] is not a
 *         valid telegram start (wrong sync, header CRC, data CRC, or an over-length data
 *         field) - the caller should drop one byte and retry to resynchronise.
 */
int esp3_parse(const uint8_t *raw, uint16_t len, esp3_packet *out);

/**
 * @brief Assemble an ESP3 telegram into @p out.
 * @return the total telegram length, or 0 if it would not fit @p cap or @p data_len exceeds
 *         DWS_ENOCEAN_MAX_DATA.
 */
uint16_t esp3_build(esp3_type type, const uint8_t *data, uint16_t data_len, const uint8_t *opt, uint8_t opt_len,
                    uint8_t *out, uint16_t cap);

#endif // DWS_ENABLE_ENOCEAN

#endif // DETERMINISTICESPASYNCWEBSERVER_ENOCEAN_H
