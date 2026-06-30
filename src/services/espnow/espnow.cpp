// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file espnow.cpp
 * @brief ESP-NOW envelope codec + peer registry (pure) and esp_now binding (ESP32).
 */

#include "services/espnow/espnow.h"

#if DETWS_ENABLE_ESPNOW

const uint8_t DETWS_ESPNOW_BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------------------------------------------------------------------
// Envelope codec
// ---------------------------------------------------------------------------
size_t detws_espnow_encode(uint8_t type, const uint8_t *payload, size_t len, uint8_t *out, size_t cap)
{
    if (!out || len > DETWS_ESPNOW_MAX_PAYLOAD || cap < len + DETWS_ESPNOW_HDR)
        return 0;
    out[0] = DETWS_ESPNOW_MAGIC;
    out[1] = type;
    out[2] = (uint8_t)len;
    if (len && payload)
        memcpy(out + DETWS_ESPNOW_HDR, payload, len);
    return len + DETWS_ESPNOW_HDR;
}

bool detws_espnow_decode(const uint8_t *buf, size_t len, uint8_t *type, const uint8_t **payload, size_t *plen)
{
    if (!buf || len < DETWS_ESPNOW_HDR || buf[0] != DETWS_ESPNOW_MAGIC)
        return false;
    size_t declared = buf[2];
    if (declared + DETWS_ESPNOW_HDR != len) // length must match exactly (no trailing/short)
        return false;
    if (type)
        *type = buf[1];
    if (payload)
        *payload = buf + DETWS_ESPNOW_HDR;
    if (plen)
        *plen = declared;
    return true;
}

// ---------------------------------------------------------------------------
// Peer registry
// ---------------------------------------------------------------------------
namespace
{
struct Peer
{
    uint8_t mac[6];
    bool used;
};
Peer s_peers[DETWS_ESPNOW_MAX_PEERS];

int peer_find(const uint8_t mac[6])
{
    for (int i = 0; i < DETWS_ESPNOW_MAX_PEERS; i++)
        if (s_peers[i].used && memcmp(s_peers[i].mac, mac, 6) == 0)
            return i;
    return -1;
}
} // namespace

void detws_espnow_peers_reset(void)
{
    for (int i = 0; i < DETWS_ESPNOW_MAX_PEERS; i++)
        s_peers[i].used = false;
}

bool detws_espnow_peer_add(const uint8_t mac[6])
{
    if (!mac)
        return false;
    if (peer_find(mac) >= 0)
        return true; // idempotent
    for (int i = 0; i < DETWS_ESPNOW_MAX_PEERS; i++)
        if (!s_peers[i].used)
        {
            memcpy(s_peers[i].mac, mac, 6);
            s_peers[i].used = true;
            return true;
        }
    return false; // table full
}

bool detws_espnow_peer_has(const uint8_t mac[6])
{
    return mac && peer_find(mac) >= 0;
}

bool detws_espnow_peer_remove(const uint8_t mac[6])
{
    int i = mac ? peer_find(mac) : -1;
    if (i < 0)
        return false;
    s_peers[i].used = false;
    return true;
}

int detws_espnow_peer_count(void)
{
    int n = 0;
    for (int i = 0; i < DETWS_ESPNOW_MAX_PEERS; i++)
        if (s_peers[i].used)
            n++;
    return n;
}

// ---------------------------------------------------------------------------
// ESP32 radio binding
// ---------------------------------------------------------------------------
#ifdef ARDUINO

#include <esp_now.h>
#include <esp_wifi.h>

namespace
{
detws_espnow_recv_fn s_recv = nullptr;

void on_recv(const uint8_t *mac, const uint8_t *data, int len)
{
    if (!s_recv || len < 0)
        return;
    uint8_t type;
    const uint8_t *payload;
    size_t plen;
    if (detws_espnow_decode(data, (size_t)len, &type, &payload, &plen))
        s_recv(mac, type, payload, plen);
}

bool radio_add_peer(const uint8_t mac[6], uint8_t channel)
{
    esp_now_peer_info_t p;
    memset(&p, 0, sizeof(p));
    memcpy(p.peer_addr, mac, 6);
    p.channel = channel;
    p.encrypt = false;
    if (esp_now_is_peer_exist(mac))
        return true;
    return esp_now_add_peer(&p) == ESP_OK;
}

uint8_t s_channel = 0;
} // namespace

bool detws_espnow_begin(uint8_t channel, detws_espnow_recv_fn cb)
{
    s_channel = channel;
    s_recv = cb;
    if (esp_now_init() != ESP_OK)
        return false;
    esp_now_register_recv_cb(on_recv);
    detws_espnow_peers_reset();
    return radio_add_peer(DETWS_ESPNOW_BROADCAST, channel); // broadcast is always a peer
}

bool detws_espnow_add_peer(const uint8_t mac[6])
{
    if (!detws_espnow_peer_add(mac))
        return false;
    return radio_add_peer(mac, s_channel);
}

bool detws_espnow_send(const uint8_t mac[6], uint8_t type, const uint8_t *payload, size_t len)
{
    uint8_t frame[DETWS_ESPNOW_HDR + DETWS_ESPNOW_MAX_PAYLOAD];
    size_t n = detws_espnow_encode(type, payload, len, frame, sizeof(frame));
    if (n == 0)
        return false;
    return esp_now_send(mac, frame, n) == ESP_OK;
}

bool detws_espnow_broadcast(uint8_t type, const uint8_t *payload, size_t len)
{
    return detws_espnow_send(DETWS_ESPNOW_BROADCAST, type, payload, len);
}

#else // host build - no radio

bool detws_espnow_begin(uint8_t, detws_espnow_recv_fn)
{
    return false;
}
bool detws_espnow_add_peer(const uint8_t mac[6])
{
    return detws_espnow_peer_add(mac);
}
bool detws_espnow_send(const uint8_t *, uint8_t, const uint8_t *, size_t)
{
    return false;
}
bool detws_espnow_broadcast(uint8_t, const uint8_t *, size_t)
{
    return false;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_ESPNOW
