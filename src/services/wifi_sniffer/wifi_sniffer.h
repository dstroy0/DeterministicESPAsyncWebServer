// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file wifi_sniffer.h
 * @brief 802.11 frame decode + traffic tally + RSSI roaming decision (DETWS_ENABLE_WIFI_SNIFFER).
 *
 * The ESP32 can run its WiFi MAC in promiscuous mode and hand raw 802.11 frames to a callback. Turning
 * those into a useful sniffer / traffic analyzer / RF-diagnostics panel means decoding the 802.11 MAC
 * header (frame control type/subtype + flags, and the three addresses whose roles - receiver /
 * transmitter / BSSID - depend on the ToDS/FromDS bits), tallying frames by type, and, for
 * channel-agility roaming, deciding when a candidate AP is enough stronger than the current one to switch.
 *
 * This is that pure decode + decision layer; the promiscuous-mode radio callback belongs to the app. No
 * heap, no stdlib, host-testable against captured frame bytes.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_WIFI_SNIFFER_H
#define DETERMINISTICESPASYNCWEBSERVER_WIFI_SNIFFER_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_WIFI_SNIFFER

/** @brief 802.11 frame type (Frame Control bits 2-3). */
struct WifiType
{
    static constexpr uint8_t WIFI_TYPE_MGMT = 0; ///< management (beacon, probe, auth, assoc, ...).
    static constexpr uint8_t WIFI_TYPE_CTRL = 1; ///< control (RTS/CTS/ACK, ...).
    static constexpr uint8_t WIFI_TYPE_DATA = 2; ///< data.
    static constexpr uint8_t WIFI_TYPE_EXT = 3;  ///< extension.
};

/** @brief A decoded 802.11 MAC header. Addresses not present for the frame's length are left zeroed. */
struct WifiFrame
{
    uint8_t version;      ///< protocol version (FC bits 0-1).
    uint8_t type;         ///< WIFI_TYPE_*.
    uint8_t subtype;      ///< FC bits 4-7.
    bool to_ds;           ///< to the distribution system.
    bool from_ds;         ///< from the distribution system.
    bool retry;           ///< retransmission.
    bool protected_frame; ///< the Protected Frame (WEP/WPA) flag.
    uint8_t naddr;        ///< number of addresses decoded (1..3).
    uint8_t addr1[6];     ///< receiver / destination (role varies by DS bits).
    uint8_t addr2[6];     ///< transmitter / source (present when naddr >= 2).
    uint8_t addr3[6];     ///< BSSID / source / dest (present when naddr >= 3).
};

/**
 * @brief Decode the 802.11 MAC header of a captured frame.
 *
 * Requires at least the Frame Control + Duration + Address1 (10 bytes); Address2 is decoded at >= 16
 * bytes and Address3 at >= 24 bytes, with @p out->naddr reporting how many were present.
 * @return true if @p len >= 10 and @p frame is non-null; false otherwise.
 */
bool detws_wifi_parse(const uint8_t *frame, size_t len, WifiFrame *out);

/** @brief Running per-type frame tally. */
struct WifiStats
{
    uint32_t mgmt;
    uint32_t ctrl;
    uint32_t data;
    uint32_t other;
    uint32_t total;
};

/** @brief Zero a tally. */
void detws_wifi_stats_reset(WifiStats *s);

/** @brief Fold one decoded frame into the tally. */
void detws_wifi_stats_add(WifiStats *s, const WifiFrame *f);

/**
 * @brief Channel-agility roaming decision.
 * @return true if @p cand_rssi exceeds @p cur_rssi by more than @p hysteresis_db (so a switch is worth it).
 */
bool detws_wifi_should_roam(int8_t cur_rssi, int8_t cand_rssi, uint8_t hysteresis_db);

#endif // DETWS_ENABLE_WIFI_SNIFFER
#endif // DETERMINISTICESPASYNCWEBSERVER_WIFI_SNIFFER_H
