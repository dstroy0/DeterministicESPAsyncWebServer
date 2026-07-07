// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file promisc.cpp
 * @brief promisc implementation: the pure 802.11 header parser + PCAP framing, and the ESP32
 *        esp_wifi promiscuous binding. The parser / PCAP builders are host-identical.
 */

#include "services/promisc/promisc.h"

#if DETWS_ENABLE_PROMISC

#include <string.h>

bool wifi_frame_parse(const uint8_t *frame, uint16_t len, WifiFrameInfo *out)
{
    if (!frame || !out || len < 10) // FC(2) + Duration(2) + Addr1(6) - the shortest control frame
        return false;
    memset(out, 0, sizeof(*out));

    const uint8_t fc0 = frame[0], fc1 = frame[1];
    out->type = (uint8_t)((fc0 >> 2) & 0x3);
    out->subtype = (uint8_t)((fc0 >> 4) & 0xF);
    out->to_ds = (fc1 & 0x01) != 0;
    out->from_ds = (fc1 & 0x02) != 0;
    out->protected_frame = (fc1 & 0x40) != 0;

    if (out->type == WIFI_FT_CTRL)
    {
        // Control frames carry only Addr1 (the receiver); the rest vary by subtype.
        out->dst = frame + 4;
        out->hdr_len = 10;
        return true;
    }

    // Management / data / extension frames carry the full 3-address header + sequence control.
    if (len < 24)
        return false;
    out->seq = (uint16_t)(((uint16_t)frame[22] | ((uint16_t)frame[23] << 8)) >> 4);
    out->is_qos = (out->type == WIFI_FT_DATA) && (out->subtype & 0x08) != 0;

    const bool has_addr4 = out->to_ds && out->from_ds; // WDS
    uint16_t hlen = 24;
    if (has_addr4)
        hlen += 6;
    if (out->is_qos)
        hlen += 2;
    if (out->is_qos && (fc1 & 0x80)) // Order bit on a QoS data frame -> HT Control field
        hlen += 4;
    if (len < hlen)
        return false;
    out->hdr_len = hlen;

    const uint8_t *a1 = frame + 4, *a2 = frame + 10, *a3 = frame + 16;
    if (!out->to_ds && !out->from_ds) // IBSS / management
    {
        out->dst = a1;
        out->src = a2;
        out->bssid = a3;
    }
    else if (!out->to_ds && out->from_ds) // from the AP
    {
        out->dst = a1;
        out->bssid = a2;
        out->src = a3;
    }
    else if (out->to_ds && !out->from_ds) // to the AP
    {
        out->bssid = a1;
        out->src = a2;
        out->dst = a3;
    }
    else // WDS (4-address): A1=RA, A2=TA, A3=DA, A4=SA
    {
        out->dst = a3;
        out->src = frame + 24;
        out->bssid = nullptr;
    }
    return true;
}

// libpcap framing (det_pcap_global_header / det_pcap_record_header) is in
// shared_primitives/det_pcap.h - shared with the other capture features.

// --- ESP32 radio binding -----------------------------------------------------------------
#ifdef ARDUINO

#include <esp_wifi.h>

namespace
{
// All promiscuous-capture state, owned by one instance (internal linkage): the frame sink.
// One named owner, unreachable from any other translation unit.
struct PromiscCtx
{
    promisc_sink_fn sink = nullptr;
};
PromiscCtx s_promisc;

void promisc_rx(void *buf, wifi_promiscuous_pkt_type_t)
{
    if (!s_promisc.sink || !buf)
        return;
    const wifi_promiscuous_pkt_t *pkt = (const wifi_promiscuous_pkt_t *)buf;
    uint16_t len = (uint16_t)pkt->rx_ctrl.sig_len; // includes the 4-byte FCS
    if (len < 4)
        return;
    s_promisc.sink(pkt->payload, (uint16_t)(len - 4), (int8_t)pkt->rx_ctrl.rssi, (uint8_t)pkt->rx_ctrl.channel);
}
} // namespace

bool promisc_begin(uint8_t channel, promisc_sink_fn sink)
{
    if (!sink)
        return false;
    s_promisc.sink = sink;
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promisc_rx);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    return true;
}

void promisc_set_channel(uint8_t channel)
{
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void promisc_end(void)
{
    esp_wifi_set_promiscuous(false);
    s_promisc.sink = nullptr;
}

#else // host build - no radio

bool promisc_begin(uint8_t, promisc_sink_fn)
{
    return false;
}
void promisc_set_channel(uint8_t)
{
    // host build: no radio, no channel to set
}
void promisc_end(void)
{
    // host build: no radio, nothing to stop
}

#endif // ARDUINO

#endif // DETWS_ENABLE_PROMISC
