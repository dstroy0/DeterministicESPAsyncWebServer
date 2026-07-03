// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_ip.cpp
 * @brief DetIp implementation: RFC 4291 text parsing, RFC 5952 canonical formatting, scope
 *        classification. Pure, hand-rolled (no stdlib parsing), host-identical.
 */

#include "network_drivers/network/det_ip.h"
#include <string.h>

namespace
{
int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

// Parse a dotted-quad from s[0, len) into out[4].
bool parse_v4(const char *s, size_t len, uint8_t out[4])
{
    int oct = 0, val = -1, digits = 0;
    for (size_t i = 0; i < len; i++)
    {
        char c = s[i];
        if (c == '.')
        {
            if (digits == 0 || oct >= 3)
                return false;
            out[oct++] = (uint8_t)val;
            val = -1;
            digits = 0;
        }
        else if (c >= '0' && c <= '9')
        {
            if (digits >= 3)
                return false;
            val = (val < 0 ? 0 : val) * 10 + (c - '0');
            if (val > 255)
                return false;
            digits++;
        }
        else
            return false;
    }
    if (oct != 3 || digits == 0)
        return false;
    out[3] = (uint8_t)val;
    return true;
}

// Parse an IPv6 text form (with :: compression and an optional embedded v4 tail) into out[16].
bool parse_v6(const char *s, size_t len, uint8_t out[16])
{
    uint16_t head[8], tail[8];
    int nhead = 0, ntail = 0;
    bool seen_dc = false;
    uint16_t *cur = head;
    int *ncur = &nhead;

    size_t i = 0;
    if (len >= 1 && s[0] == ':')
    {
        if (len < 2 || s[1] != ':')
            return false; // a single leading ':' is illegal
        seen_dc = true;
        cur = tail;
        ncur = &ntail;
        i = 2;
        if (i == len)
        {
            memset(out, 0, 16); // "::"
            return true;
        }
    }

    while (i < len)
    {
        size_t j = i;
        bool has_dot = false;
        while (j < len && s[j] != ':')
        {
            if (s[j] == '.')
                has_dot = true;
            j++;
        }
        size_t tlen = j - i;
        if (has_dot)
        {
            uint8_t q[4];
            if (!parse_v4(s + i, tlen, q))
                return false;
            if (*ncur > 6)
                return false; // no room for the two embedded hextets
            cur[(*ncur)++] = (uint16_t)((q[0] << 8) | q[1]);
            cur[(*ncur)++] = (uint16_t)((q[2] << 8) | q[3]);
            i = j;
            if (i != len)
                return false; // an embedded v4 must be the final token
            break;
        }
        if (tlen == 0 || tlen > 4)
            return false;
        int v = 0;
        for (size_t k = i; k < j; k++)
        {
            int h = hexval(s[k]);
            if (h < 0)
                return false;
            v = v * 16 + h;
        }
        if (*ncur >= 8)
            return false;
        cur[(*ncur)++] = (uint16_t)v;
        i = j;
        if (i < len)
        {
            if (i + 1 < len && s[i + 1] == ':')
            {
                if (seen_dc)
                    return false; // only one "::" allowed
                seen_dc = true;
                cur = tail;
                ncur = &ntail;
                i += 2;
                if (i == len)
                    break; // trailing "::"
            }
            else
            {
                i += 1;
                if (i == len)
                    return false; // trailing single ':'
            }
        }
    }

    int total = nhead + ntail;
    if (seen_dc ? (total > 7) : (total != 8))
        return false;

    uint16_t g[8];
    memset(g, 0, sizeof(g));
    for (int k = 0; k < nhead; k++)
        g[k] = head[k];
    for (int k = 0; k < ntail; k++)
        g[8 - ntail + k] = tail[k];
    for (int k = 0; k < 8; k++)
    {
        out[2 * k] = (uint8_t)(g[k] >> 8);
        out[2 * k + 1] = (uint8_t)(g[k] & 0xFF);
    }
    return true;
}

// Append a decimal byte (0-255). Returns chars written.
size_t put_u8(uint8_t v, char *o)
{
    size_t n = 0;
    if (v >= 100)
        o[n++] = (char)('0' + v / 100);
    if (v >= 10)
        o[n++] = (char)('0' + (v / 10) % 10);
    o[n++] = (char)('0' + v % 10);
    return n;
}

size_t format_v4(const uint8_t *b, char *out, size_t cap)
{
    char tmp[16];
    size_t n = 0;
    for (int k = 0; k < 4; k++)
    {
        if (k)
            tmp[n++] = '.';
        n += put_u8(b[k], tmp + n);
    }
    if (n + 1 > cap)
        return 0;
    memcpy(out, tmp, n);
    out[n] = '\0';
    return n;
}

size_t put_hex16(uint16_t v, char *o)
{
    const char *H = "0123456789abcdef";
    char t[4];
    int n = 0;
    do
    {
        t[n++] = H[v & 0xF];
        v >>= 4;
    } while (v);
    for (int k = 0; k < n; k++)
        o[k] = t[n - 1 - k];
    return (size_t)n;
}
} // namespace

bool det_ip_parse(const char *s, DetIp *out)
{
    if (!s || !out)
        return false;
    size_t len = 0;
    bool colon = false, dot = false;
    while (s[len])
    {
        if (s[len] == ':')
            colon = true;
        else if (s[len] == '.')
            dot = true;
        len++;
        if (len > 45)
            return false; // longer than any legal address
    }
    if (len == 0)
        return false;
    memset(out->bytes, 0, 16);
    if (colon)
    {
        if (!parse_v6(s, len, out->bytes))
            return false;
        out->family = DET_IP_V6;
        return true;
    }
    if (dot)
    {
        if (!parse_v4(s, len, out->bytes))
            return false;
        out->family = DET_IP_V4;
        return true;
    }
    return false;
}

size_t det_ip_format(const DetIp *ip, char *out, size_t cap)
{
    if (!ip || !out || cap == 0)
        return 0;
    if (ip->family == DET_IP_V4)
        return format_v4(ip->bytes, out, cap);
    if (ip->family != DET_IP_V6)
        return 0;

    // IPv4-mapped: ::ffff:a.b.c.d (RFC 5952 §5).
    if (det_ip_is_v4_mapped(ip))
    {
        char tail[16];
        size_t tn = format_v4(ip->bytes + 12, tail, sizeof(tail));
        if (tn == 0 || 7 + tn + 1 > cap)
            return 0;
        memcpy(out, "::ffff:", 7);
        memcpy(out + 7, tail, tn);
        out[7 + tn] = '\0';
        return 7 + tn;
    }

    uint16_t g[8];
    for (int k = 0; k < 8; k++)
        g[k] = (uint16_t)((ip->bytes[2 * k] << 8) | ip->bytes[2 * k + 1]);

    // Longest run of >= 2 zero groups -> "::" (leftmost on a tie, RFC 5952 §4.2.3).
    int best_start = -1, best_len = 0, cs = -1, cl = 0;
    for (int k = 0; k < 8; k++)
    {
        if (g[k] == 0)
        {
            if (cs < 0)
                cs = k;
            cl++;
            if (cl > best_len)
            {
                best_len = cl;
                best_start = cs;
            }
        }
        else
        {
            cs = -1;
            cl = 0;
        }
    }
    if (best_len < 2)
        best_start = -1;

    char tmp[DET_IP_STR_MAX];
    size_t n = 0;
    for (int k = 0; k < 8; k++)
    {
        if (best_start != -1 && k >= best_start && k < best_start + best_len)
        {
            if (k == best_start)
                tmp[n++] = ':'; // one colon here; the pair forms "::"
            continue;
        }
        if (k != 0)
            tmp[n++] = ':';
        n += put_hex16(g[k], tmp + n);
    }
    if (best_start != -1 && best_start + best_len == 8)
        tmp[n++] = ':'; // trailing run: close the "::"
    if (n + 1 > cap)
        return 0;
    memcpy(out, tmp, n);
    out[n] = '\0';
    return n;
}

bool det_ip_is_v4_mapped(const DetIp *ip)
{
    if (!ip || ip->family != DET_IP_V6)
        return false;
    for (int k = 0; k < 10; k++)
        if (ip->bytes[k] != 0)
            return false;
    return ip->bytes[10] == 0xff && ip->bytes[11] == 0xff;
}

DetIpScope det_ip_classify(const DetIp *ip)
{
    if (!ip)
        return DET_IP_SCOPE_UNSPECIFIED;
    if (ip->family == DET_IP_V4)
    {
        const uint8_t *b = ip->bytes;
        if (b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 0)
            return DET_IP_SCOPE_UNSPECIFIED;
        if (b[0] == 127)
            return DET_IP_SCOPE_LOOPBACK;
        if (b[0] == 169 && b[1] == 254)
            return DET_IP_SCOPE_LINK_LOCAL;
        if (b[0] == 10)
            return DET_IP_SCOPE_PRIVATE;
        if (b[0] == 172 && b[1] >= 16 && b[1] <= 31)
            return DET_IP_SCOPE_PRIVATE;
        if (b[0] == 192 && b[1] == 168)
            return DET_IP_SCOPE_PRIVATE;
        if (b[0] >= 224 && b[0] <= 239)
            return DET_IP_SCOPE_MULTICAST;
        return DET_IP_SCOPE_GLOBAL;
    }
    if (ip->family == DET_IP_V6)
    {
        const uint8_t *b = ip->bytes;
        bool allzero = true;
        for (int k = 0; k < 16; k++)
            if (b[k])
            {
                allzero = false;
                break;
            }
        if (allzero)
            return DET_IP_SCOPE_UNSPECIFIED;
        bool loop = (b[15] == 1);
        for (int k = 0; k < 15 && loop; k++)
            if (b[k])
                loop = false;
        if (loop)
            return DET_IP_SCOPE_LOOPBACK;
        if (det_ip_is_v4_mapped(ip))
        {
            DetIp v4 = det_ip_from_v4_octets(b[12], b[13], b[14], b[15]);
            return det_ip_classify(&v4);
        }
        if (b[0] == 0xff)
            return DET_IP_SCOPE_MULTICAST; // ff00::/8
        if (b[0] == 0xfe && (b[1] & 0xc0) == 0x80)
            return DET_IP_SCOPE_LINK_LOCAL; // fe80::/10
        if ((b[0] & 0xfe) == 0xfc)
            return DET_IP_SCOPE_PRIVATE; // fc00::/7 (ULA)
        return DET_IP_SCOPE_GLOBAL;
    }
    return DET_IP_SCOPE_UNSPECIFIED;
}

bool det_ip_equal(const DetIp *a, const DetIp *b)
{
    if (!a || !b || a->family != b->family)
        return false;
    int n = (a->family == DET_IP_V4) ? 4 : (a->family == DET_IP_V6 ? 16 : 0);
    if (n == 0)
        return a->family == b->family; // both NONE
    return memcmp(a->bytes, b->bytes, (size_t)n) == 0;
}

DetIp det_ip_from_v4_octets(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    DetIp ip;
    memset(&ip, 0, sizeof(ip));
    ip.family = DET_IP_V4;
    ip.bytes[0] = a;
    ip.bytes[1] = b;
    ip.bytes[2] = c;
    ip.bytes[3] = d;
    return ip;
}

uint32_t det_ip_to_v4_be(const DetIp *ip)
{
    if (!ip)
        return 0;
    const uint8_t *b = ip->bytes;
    if (ip->family == DET_IP_V4)
        return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | b[3];
    if (det_ip_is_v4_mapped(ip))
        return ((uint32_t)b[12] << 24) | ((uint32_t)b[13] << 16) | ((uint32_t)b[14] << 8) | b[15];
    return 0;
}
