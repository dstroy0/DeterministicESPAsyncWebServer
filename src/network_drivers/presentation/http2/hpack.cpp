// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hpack.cpp
 * @brief HPACK (RFC 7541) - implementation. See hpack.h.
 *
 * The static table, the Huffman code (Appendix B), and the canonical Huffman decode tables are
 * generated verbatim from RFC 7541 (see docs / the generator in the repo history). Header names
 * and values never touch the heap; the dynamic table is a fixed byte ring with FIFO eviction.
 */

#include "network_drivers/presentation/http2/hpack.h"

#if DETWS_ENABLE_HTTP2

#include "network_drivers/presentation/hpack_prim/hpack_prim.h" // shared prefix-int + Huffman
#include <string.h>

namespace
{
#define HPACK_BYTES DETWS_HPACK_TABLE_BYTES
#define HPACK_ENTS DETWS_HPACK_MAX_ENTRIES

// Static table (1-indexed; entry 0 is a placeholder). {name, value}. Generated from RFC 7541 App A.
const char *const STATIC[62][2] = {
    {"", ""},
    {":authority", ""},
    {":method", "GET"},
    {":method", "POST"},
    {":path", "/"},
    {":path", "/index.html"},
    {":scheme", "http"},
    {":scheme", "https"},
    {":status", "200"},
    {":status", "204"},
    {":status", "206"},
    {":status", "304"},
    {":status", "400"},
    {":status", "404"},
    {":status", "500"},
    {"accept-charset", ""},
    {"accept-encoding", "gzip, deflate"},
    {"accept-language", ""},
    {"accept-ranges", ""},
    {"accept", ""},
    {"access-control-allow-origin", ""},
    {"age", ""},
    {"allow", ""},
    {"authorization", ""},
    {"cache-control", ""},
    {"content-disposition", ""},
    {"content-encoding", ""},
    {"content-language", ""},
    {"content-length", ""},
    {"content-location", ""},
    {"content-range", ""},
    {"content-type", ""},
    {"cookie", ""},
    {"date", ""},
    {"etag", ""},
    {"expect", ""},
    {"expires", ""},
    {"from", ""},
    {"host", ""},
    {"if-match", ""},
    {"if-modified-since", ""},
    {"if-none-match", ""},
    {"if-range", ""},
    {"if-unmodified-since", ""},
    {"last-modified", ""},
    {"link", ""},
    {"location", ""},
    {"max-forwards", ""},
    {"proxy-authenticate", ""},
    {"proxy-authorization", ""},
    {"range", ""},
    {"referer", ""},
    {"refresh", ""},
    {"retry-after", ""},
    {"server", ""},
    {"set-cookie", ""},
    {"strict-transport-security", ""},
    {"transfer-encoding", ""},
    {"user-agent", ""},
    {"vary", ""},
    {"via", ""},
    {"www-authenticate", ""},
};

// --- Dynamic table byte-ring helpers ---------------------------------------------------------

void ring_write(HpackDynTable *t, uint16_t pos, const char *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        t->ring[(pos + i) % HPACK_BYTES] = src[i];
}
void ring_read(const HpackDynTable *t, uint16_t pos, char *dst, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = t->ring[(pos + i) % HPACK_BYTES];
}

// Descriptor for the k-th newest live entry (k = 1 is newest); null if out of range.
const HpackEntry *dyn_entry(const HpackDynTable *t, uint32_t k)
{
    if (k < 1 || k > t->ecount)
        return nullptr;
    uint16_t di = (uint16_t)((t->ehead + HPACK_ENTS - k) % HPACK_ENTS);
    return &t->ent[di];
}

void dyn_evict_oldest(HpackDynTable *t)
{
    if (t->ecount == 0)
        return;
    uint16_t oi = (uint16_t)((t->ehead + HPACK_ENTS - t->ecount) % HPACK_ENTS);
    const HpackEntry *e = &t->ent[oi];
    uint16_t bytes = (uint16_t)(e->name_len + e->val_len);
    t->rtail = (uint16_t)((t->rtail + bytes) % HPACK_BYTES);
    t->rused = (uint16_t)(t->rused - bytes);
    t->used -= (uint32_t)e->name_len + e->val_len + 32;
    t->ecount--;
}

void dyn_set_max(HpackDynTable *t, uint32_t new_max)
{
    if (new_max > HPACK_BYTES)
        new_max = HPACK_BYTES; // never exceed our advertised storage
    t->max_size = new_max;
    while (t->used > t->max_size && t->ecount > 0)
        dyn_evict_oldest(t);
}

void dyn_insert(HpackDynTable *t, const char *name, size_t nlen, const char *val, size_t vlen)
{
    uint32_t entry_size = (uint32_t)nlen + (uint32_t)vlen + 32;
    if (entry_size > t->max_size)
    { // RFC 7541 sec 4.4: clears the table, entry not added
        t->ecount = 0;
        t->used = 0;
        t->rused = 0;
        t->rtail = 0;
        t->ehead = 0;
        return;
    }
    while ((t->used + entry_size > t->max_size || t->ecount >= HPACK_ENTS) && t->ecount > 0)
        dyn_evict_oldest(t);
    uint16_t rpos = (uint16_t)((t->rtail + t->rused) % HPACK_BYTES);
    HpackEntry *e = &t->ent[t->ehead];
    e->name_len = (uint16_t)nlen;
    e->val_len = (uint16_t)vlen;
    e->ring_pos = rpos;
    ring_write(t, rpos, name, nlen);
    ring_write(t, (uint16_t)((rpos + nlen) % HPACK_BYTES), val, vlen);
    t->rused = (uint16_t)(t->rused + nlen + vlen);
    t->ehead = (uint16_t)((t->ehead + 1) % HPACK_ENTS);
    t->ecount++;
    t->used += entry_size;
}

// --- String coding ---------------------------------------------------------------------------

// Read a length-prefixed string at block[*pos] into out; advances *pos.
bool decode_string(const uint8_t *block, size_t len, size_t *pos, char *out, size_t cap, size_t *out_len)
{
    if (*pos >= len)
        return false;
    bool huff = (block[*pos] & 0x80) != 0;
    size_t c = 0;
    uint32_t slen = 0;
    if (!hpack_decode_int(block + *pos, len - *pos, 7, &c, &slen))
        return false;
    *pos += c;
    if (*pos + slen > len)
        return false;
    if (huff)
    {
        if (!hpack_huff_decode(block + *pos, slen, out, cap, out_len))
            return false;
    }
    else
    {
        if (slen > cap)
            return false;
        memcpy(out, block + *pos, slen);
        *out_len = slen;
    }
    *pos += slen;
    return true;
}

size_t encode_string(uint8_t *out, size_t cap, const char *s, size_t n)
{
    size_t hl = hpack_huff_len(s, n);
    if (hl < n)
    {
        size_t hdr = hpack_encode_int(out, cap, 7, 0x80, (uint32_t)hl);
        if (!hdr)
            return 0;
        size_t body = hpack_huff_encode(out + hdr, cap - hdr, s, n);
        if (body != hl)
            return 0;
        return hdr + body;
    }
    size_t hdr = hpack_encode_int(out, cap, 7, 0x00, (uint32_t)n);
    if (!hdr || hdr + n > cap)
        return 0;
    memcpy(out + hdr, s, n);
    return hdr + n;
}

// Copy an indexed header's name (idx>=1) into out; false if idx invalid / too big.
bool resolve_name(const HpackDynTable *t, uint32_t idx, char *out, size_t cap, size_t *out_len)
{
    if (idx >= 1 && idx <= 61)
    {
        size_t nl = strlen(STATIC[idx][0]);
        if (nl > cap)
            return false;
        memcpy(out, STATIC[idx][0], nl);
        *out_len = nl;
        return true;
    }
    const HpackEntry *e = dyn_entry(t, idx - 61);
    if (!e || e->name_len > cap)
        return false;
    ring_read(t, e->ring_pos, out, e->name_len);
    *out_len = e->name_len;
    return true;
}

// Emit a fully-indexed field (idx resolves to name+value); copies both into scratch.
bool emit_indexed(HpackDynTable *t, uint32_t idx, char *scratch, size_t cap, HpackEmitFn emit, void *ctx)
{
    size_t nl;
    size_t vl;
    if (idx >= 1 && idx <= 61)
    {
        nl = strlen(STATIC[idx][0]);
        vl = strlen(STATIC[idx][1]);
        if (nl + vl > cap)
            return false;
        memcpy(scratch, STATIC[idx][0], nl);
        memcpy(scratch + nl, STATIC[idx][1], vl);
    }
    else
    {
        const HpackEntry *e = dyn_entry(t, idx - 61);
        if (!e)
            return false;
        nl = e->name_len;
        vl = e->val_len;
        if (nl + vl > cap)
            return false;
        ring_read(t, e->ring_pos, scratch, nl);
        ring_read(t, (uint16_t)((e->ring_pos + nl) % HPACK_BYTES), scratch + nl, vl);
    }
    return emit(ctx, scratch, nl, scratch + nl, vl);
}

// Decode a literal representation (name via index or inline, value inline; optional indexing).
bool decode_literal(HpackDynTable *t, const uint8_t *block, size_t len, size_t *pos, uint8_t prefix_bits, bool do_index,
                    char *scratch, size_t cap, HpackEmitFn emit, void *ctx)
{
    size_t c = 0;
    uint32_t name_idx = 0;
    if (!hpack_decode_int(block + *pos, len - *pos, prefix_bits, &c, &name_idx))
        return false;
    *pos += c;
    size_t name_len = 0;
    if (name_idx == 0)
    {
        if (!decode_string(block, len, pos, scratch, cap, &name_len))
            return false;
    }
    else if (!resolve_name(t, name_idx, scratch, cap, &name_len))
    {
        return false;
    }
    size_t val_len = 0;
    if (!decode_string(block, len, pos, scratch + name_len, cap - name_len, &val_len))
        return false;
    if (do_index)
        dyn_insert(t, scratch, name_len, scratch + name_len, val_len);
    return emit(ctx, scratch, name_len, scratch + name_len, val_len);
}

} // namespace

// --- Public API ------------------------------------------------------------------------------

void hpack_dyn_init(HpackDynTable *t, uint32_t max_bytes)
{
    memset(t, 0, sizeof(*t));
    t->max_size = max_bytes ? max_bytes : (uint32_t)HPACK_BYTES;
    if (t->max_size > HPACK_BYTES)
        t->max_size = HPACK_BYTES;
}

bool hpack_decode(HpackDynTable *t, const uint8_t *block, size_t len, char *scratch, size_t scratch_cap,
                  HpackEmitFn emit, void *ctx)
{
    size_t pos = 0;
    while (pos < len)
    {
        uint8_t b = block[pos];
        if (b & 0x80)
        { // 6.1 Indexed Header Field
            size_t c = 0;
            uint32_t idx = 0;
            if (!hpack_decode_int(block + pos, len - pos, 7, &c, &idx) || idx == 0)
                return false;
            pos += c;
            if (!emit_indexed(t, idx, scratch, scratch_cap, emit, ctx))
                return false;
        }
        else if (b & 0x40)
        { // 6.2.1 Literal with incremental indexing (name prefix 6)
            if (!decode_literal(t, block, len, &pos, 6, true, scratch, scratch_cap, emit, ctx))
                return false;
        }
        else if ((b & 0xE0) == 0x20)
        { // 6.3 Dynamic table size update (prefix 5)
            size_t c = 0;
            uint32_t nm = 0;
            if (!hpack_decode_int(block + pos, len - pos, 5, &c, &nm))
                return false;
            pos += c;
            dyn_set_max(t, nm);
        }
        else
        { // 6.2.2 without / 6.2.3 never indexed (name prefix 4, no table insert)
            if (!decode_literal(t, block, len, &pos, 4, false, scratch, scratch_cap, emit, ctx))
                return false;
        }
    }
    return true;
}

size_t hpack_encode_header(uint8_t *out, size_t cap, const char *name, size_t name_len, const char *value,
                           size_t value_len)
{
    int name_idx = 0;
    int full_idx = 0;
    for (int i = 1; i <= 61; i++)
    {
        if (strlen(STATIC[i][0]) == name_len && memcmp(STATIC[i][0], name, name_len) == 0)
        {
            if (!name_idx)
                name_idx = i;
            if (strlen(STATIC[i][1]) == value_len && memcmp(STATIC[i][1], value, value_len) == 0)
            {
                full_idx = i;
                break;
            }
        }
    }
    if (full_idx)
        return hpack_encode_int(out, cap, 7, 0x80, (uint32_t)full_idx);
    // Literal without indexing (top nibble 0000), name prefix 4.
    size_t o = hpack_encode_int(out, cap, 4, 0x00, (uint32_t)name_idx);
    if (!o)
        return 0;
    if (name_idx == 0)
    {
        size_t ns = encode_string(out + o, cap - o, name, name_len);
        if (!ns)
            return 0;
        o += ns;
    }
    size_t vs = encode_string(out + o, cap - o, value, value_len);
    if (!vs)
        return 0;
    return o + vs;
}

#endif // DETWS_ENABLE_HTTP2
