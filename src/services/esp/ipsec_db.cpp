// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ipsec_db.cpp
 * @brief IPsec SPD + SAD (RFC 4301) - see ipsec_db.h.
 */

#include "services/esp/ipsec_db.h"

#if DWS_ENABLE_IKEV2

#include <string.h>

namespace
{
// addr is within the inclusive [lo, hi] range. Addresses are big-endian, so a byte-wise unsigned compare
// (memcmp) is the numeric compare.
bool in_range(const uint8_t *addr, const uint8_t *lo, const uint8_t *hi, uint8_t len)
{
    return memcmp(addr, lo, len) >= 0 && memcmp(addr, hi, len) <= 0;
}
bool port_in(uint16_t p, uint16_t lo, uint16_t hi)
{
    return p >= lo && p <= hi;
}
} // namespace

// ── SPD ─────────────────────────────────────────────────────────────────────────────────────────

void dws_ipsec_spd_init(IpsecSpd *spd)
{
    if (!spd)
        return;
    spd->count = 0;
}

bool dws_ipsec_spd_add(IpsecSpd *spd, const IpsecSelector *sel, IpsecAction action, uint32_t sa_spi)
{
    if (!spd || !sel || spd->count >= DWS_IPSEC_SPD_MAX)
        return false;
    IpsecPolicy *p = &spd->entries[spd->count];
    p->sel = *sel;
    p->action = action;
    p->sa_spi = (action == IpsecAction::PROTECT) ? sa_spi : 0;
    spd->count++;
    return true;
}

bool dws_ipsec_selector_match(const IpsecSelector *sel, const IpsecFlow *flow)
{
    if (!sel || !flow || !flow->src || !flow->dst)
        return false;
    if (sel->addr_len != flow->addr_len) // different address family
        return false;
    if (sel->addr_len != 4 && sel->addr_len != 16)
        return false;
    if (sel->ip_protocol != 0 && sel->ip_protocol != flow->ip_protocol)
        return false;
    if (!in_range(flow->src, sel->src_lo, sel->src_hi, sel->addr_len))
        return false;
    if (!in_range(flow->dst, sel->dst_lo, sel->dst_hi, sel->addr_len))
        return false;
    if (!port_in(flow->src_port, sel->src_port_lo, sel->src_port_hi))
        return false;
    if (!port_in(flow->dst_port, sel->dst_port_lo, sel->dst_port_hi))
        return false;
    return true;
}

const IpsecPolicy *dws_ipsec_spd_lookup(const IpsecSpd *spd, const IpsecFlow *flow)
{
    if (!spd || !flow)
        return nullptr;
    for (size_t i = 0; i < spd->count; i++) // first match wins (order is significant)
        if (dws_ipsec_selector_match(&spd->entries[i].sel, flow))
            return &spd->entries[i];
    return nullptr;
}

bool dws_ipsec_selector_from_ts(IpsecSelector *out, const IkeTrafficSelector *ts_src, const IkeTrafficSelector *ts_dst)
{
    if (!out || !ts_src || !ts_dst)
        return false;
    if (ts_src->ts_type != ts_dst->ts_type)
        return false;
    if (ts_src->addr_len != ts_dst->addr_len || (ts_src->addr_len != 4 && ts_src->addr_len != 16))
        return false;
    if (!ts_src->start_addr || !ts_src->end_addr || !ts_dst->start_addr || !ts_dst->end_addr)
        return false;
    // Protocol: honor "any" (0) on either side; if both name a protocol they must agree.
    if (ts_src->ip_protocol != 0 && ts_dst->ip_protocol != 0 && ts_src->ip_protocol != ts_dst->ip_protocol)
        return false;

    memset(out, 0, sizeof(*out));
    uint8_t len = ts_src->addr_len;
    out->addr_len = len;
    out->ip_protocol = ts_src->ip_protocol ? ts_src->ip_protocol : ts_dst->ip_protocol;
    memcpy(out->src_lo, ts_src->start_addr, len);
    memcpy(out->src_hi, ts_src->end_addr, len);
    memcpy(out->dst_lo, ts_dst->start_addr, len);
    memcpy(out->dst_hi, ts_dst->end_addr, len);
    out->src_port_lo = ts_src->start_port;
    out->src_port_hi = ts_src->end_port;
    out->dst_port_lo = ts_dst->start_port;
    out->dst_port_hi = ts_dst->end_port;
    return true;
}

// ── SAD ─────────────────────────────────────────────────────────────────────────────────────────

void dws_ipsec_sad_init(IpsecSad *sad)
{
    if (!sad)
        return;
    sad->count = 0;
    for (size_t i = 0; i < DWS_IPSEC_SAD_MAX; i++)
        sad->entries[i].valid = false;
}

IpsecSaEntry *dws_ipsec_sad_add(IpsecSad *sad, uint32_t spi, const uint8_t *dst, uint8_t addr_len,
                                const uint8_t key[DWS_ESP_KEY_LEN], const uint8_t salt[DWS_ESP_SALT_LEN], bool inbound)
{
    if (!sad || !dst || !key || !salt || (addr_len != 4 && addr_len != 16))
        return nullptr;
    if (dws_ipsec_sad_find(sad, spi)) // SPIs are unique within a SAD
        return nullptr;
    IpsecSaEntry *e = nullptr;
    for (size_t i = 0; i < DWS_IPSEC_SAD_MAX; i++)
        if (!sad->entries[i].valid)
        {
            e = &sad->entries[i];
            break;
        }
    if (!e) // full
        return nullptr;

    memset(e, 0, sizeof(*e));
    e->spi = spi;
    e->addr_len = addr_len;
    memcpy(e->dst, dst, addr_len);
    memcpy(e->key, key, DWS_ESP_KEY_LEN);
    memcpy(e->salt, salt, DWS_ESP_SALT_LEN);
    e->seq = 0;
    e->inbound = inbound;
    if (inbound)
        dws_esp_replay_init(&e->replay);
    e->valid = true;
    sad->count++;
    return e;
}

IpsecSaEntry *dws_ipsec_sad_find(IpsecSad *sad, uint32_t spi)
{
    if (!sad)
        return nullptr;
    for (size_t i = 0; i < DWS_IPSEC_SAD_MAX; i++)
        if (sad->entries[i].valid && sad->entries[i].spi == spi)
            return &sad->entries[i];
    return nullptr;
}

bool dws_ipsec_sad_remove(IpsecSad *sad, uint32_t spi)
{
    IpsecSaEntry *e = dws_ipsec_sad_find(sad, spi);
    if (!e)
        return false;
    memset(e, 0, sizeof(*e)); // wipe the key material with the slot
    e->valid = false;
    if (sad->count)
        sad->count--;
    return true;
}

bool dws_ipsec_sad_next_seq(IpsecSaEntry *sa, uint32_t *seq_out)
{
    if (!sa || !seq_out)
        return false;
    if (sa->seq == 0xFFFFFFFFu) // counter exhausted - must rekey before sending more (RFC 4303 §3.3.3)
        return false;
    sa->seq++; // pre-increment: the first packet uses sequence number 1
    *seq_out = sa->seq;
    return true;
}

#endif // DWS_ENABLE_IKEV2
