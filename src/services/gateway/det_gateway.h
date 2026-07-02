// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_gateway.h
 * @brief Radio / wireless gateway bridge (DETWS_ENABLE_GATEWAY) - the v5 southbound-to-
 *        northbound bridge.
 *
 * The generic gateway pattern that ties the hardware-ingest pipeline to the web stack. A
 * southbound radio (LoRa / nRF24 / CC1101 / Zigbee / Z-Wave / ... reached over SPI / I2C /
 * UART) is a **port**. When it receives a frame - the data-ready ISR reads it over DMA
 * (services/dma), posts it onto the FORWARD lane (services/preempt_queue), and a per-radio
 * codec extracts the source node address and payload - you call det_gw_uplink(). The
 * gateway **envelopes** the frame (source address, port, RSSI, a sequence number) and
 * **publishes it northbound** through the uplink callback, which you wire to MQTT / HTTP /
 * WebSocket / UDP. A northbound command runs the other way through det_gw_downlink() to the
 * port's transmit callback (the radio's SPI / UART write).
 *
 * The radio transmit and the northbound publish are **callbacks** - the seam a real radio
 * driver and a real protocol binding plug into - so the bridge is fully host- and
 * device-testable with no radio hardware (the tests / example supply capturing callbacks
 * and feed simulated frames). This is the northbound half; the DMA + FORWARD lane carry the
 * bytes, and each radio's frame format is its own codec.
 *
 * Per-port uplink rate cap (fail-closed), a routing-key helper (det_gw_topic() formats
 * `<prefix>/<port>/<addr>`), and static tables (zero heap): DETWS_GW_MAX_PORTS ports.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_GATEWAY_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_GATEWAY_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_GATEWAY

#include <stddef.h>
#include <stdint.h>

/** @brief Southbound radio / bus kind a port bridges (informational + topic hint). */
enum det_gw_kind
{
    DET_GW_OTHER = 0,
    DET_GW_LORA,
    DET_GW_NRF24,
    DET_GW_CC1101,
    DET_GW_THREAD,
    DET_GW_ZIGBEE,
    DET_GW_ZWAVE,
    DET_GW_ENOCEAN,
    DET_GW_SIGFOX,
    DET_GW_WISUN,
    DET_GW_NFC,
    DET_GW_BLE,
};

/**
 * @brief A northbound message: a southbound frame enveloped with its routing metadata.
 *        @ref payload points at the caller's bytes and is valid only for the duration of
 *        the uplink callback - copy what you publish asynchronously.
 */
struct det_gw_msg
{
    const uint8_t *payload; ///< frame payload bytes
    uint32_t seq;           ///< per-gateway uplink sequence (wraps)
    uint16_t len;           ///< payload length
    uint16_t src_addr;      ///< source node address on the radio
    int16_t rssi;           ///< received signal strength (0 if the driver has none)
    uint8_t port_id;        ///< the port the frame arrived on
    uint8_t kind;           ///< det_gw_kind of that port
};

/**
 * @brief Northbound publish: emit @p msg to MQTT / HTTP / WebSocket / UDP.
 * @return true if the northbound stack accepted it; false drops (counted).
 */
typedef bool (*det_gw_uplink_fn)(const det_gw_msg *msg, void *ctx);

/**
 * @brief Southbound transmit (downlink): send @p payload to @p dst_addr on @p port_id.
 * @return true if the radio accepted the frame; false drops (counted).
 */
typedef bool (*det_gw_tx_fn)(uint8_t port_id, uint16_t dst_addr, const uint8_t *payload, uint16_t len, void *ctx);

/** @brief Southbound port (radio / bus) configuration passed to det_gw_add_port(). */
struct det_gw_port_config
{
    uint8_t port_id;   ///< caller-assigned id (used in topics and up/down-link calls).
    uint8_t kind;      ///< det_gw_kind.
    det_gw_tx_fn tx;   ///< downlink transmit (may be null for a receive-only port).
    void *ctx;         ///< opaque, forwarded to @ref tx.
    uint16_t rate_cap; ///< max uplink frames/second from this port (0 = unlimited).
};

/** @brief Gateway counters (monotonic since the last det_gw_reset()). */
struct det_gw_stats
{
    uint32_t up_in;        ///< det_gw_uplink() calls
    uint32_t up_published; ///< frames the uplink callback accepted
    uint32_t up_dropped;   ///< uplinks dropped (rate cap / no sink / refused / bad port)
    uint32_t down_in;      ///< det_gw_downlink() calls
    uint32_t down_sent;    ///< downlinks the port transmit accepted
    uint32_t down_dropped; ///< downlinks dropped (bad port / no tx / refused)
};

/** @brief Clear all ports, the uplink sink, the topic prefix, and stats. */
void det_gw_reset(void);

/**
 * @brief Register a southbound port.
 * @return true; false if @p cfg is null, the id is already registered, or the table is
 *         full (DETWS_GW_MAX_PORTS).
 */
bool det_gw_add_port(const det_gw_port_config *cfg);

/** @brief Install the northbound publish callback (required to publish anything). */
void det_gw_set_uplink(det_gw_uplink_fn fn, void *ctx);

/** @brief Set the topic prefix used by det_gw_topic() (caller-owned string; default "gw"). */
void det_gw_set_topic_prefix(const char *prefix);

/**
 * @brief Bridge a received southbound frame northbound: envelope it and publish.
 *
 * Applies the port's uplink rate cap, then calls the uplink callback. Fail-closed: an
 * unknown port, no installed sink, an exceeded cap, or a callback returning false drops
 * the frame (counted), never blocks.
 * @return true if published.
 */
bool det_gw_uplink(uint8_t port_id, uint16_t src_addr, const uint8_t *payload, uint16_t len, int16_t rssi);

/**
 * @brief Bridge a northbound command southbound: transmit it on @p port_id's radio.
 * @return true if the port's transmit callback accepted it; false drops (counted).
 */
bool det_gw_downlink(uint8_t port_id, uint16_t dst_addr, const uint8_t *payload, uint16_t len);

/**
 * @brief Format a northbound routing key `<prefix>/<port>/<addr>` for @p msg into @p buf.
 * @return the string length written (excluding the NUL), or 0 if @p buf is too small.
 */
uint16_t det_gw_topic(const det_gw_msg *msg, char *buf, uint16_t buflen);

/** @brief Copy the current gateway counters into @p out. */
void det_gw_get_stats(det_gw_stats *out);

#if !defined(ARDUINO)
/** @brief Host only: set the millisecond clock the rate cap uses (tests drive the window). */
void det_gw_test_set_now(uint32_t ms);
#endif

#endif // DETWS_ENABLE_GATEWAY

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_GATEWAY_H
