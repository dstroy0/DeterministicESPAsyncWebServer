// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sep2.cpp
 * @brief IEEE 2030.5 resource codec (see sep2.h).
 */

#include "services/sep2/sep2.h"

#if DETWS_ENABLE_SEP2

#include <string.h>

namespace
{
struct Buf
{
    char *p;
    size_t cap;
    size_t len;
    bool ok;
};

void put(Buf *b, const char *s)
{
    if (!b->ok)
        return;
    size_t sl = strlen(s);
    if (b->len + sl >= b->cap)
    {
        b->ok = false;
        return;
    }
    memcpy(b->p + b->len, s, sl);
    b->len += sl;
}

void put_escaped(Buf *b, const char *s)
{
    if (!b->ok || !s)
        return;
    for (; *s; s++)
    {
        const char *rep = nullptr;
        switch (*s)
        {
        case '&':
            rep = "&amp;";
            break;
        case '<':
            rep = "&lt;";
            break;
        case '>':
            rep = "&gt;";
            break;
        case '"':
            rep = "&quot;";
            break;
        default:
            break;
        }
        if (rep)
            put(b, rep);
        else
        {
            if (b->len + 1 >= b->cap)
            {
                b->ok = false;
                return;
            }
            b->p[b->len++] = *s;
        }
    }
}

void put_i64(Buf *b, int64_t v)
{
    if (!b->ok)
        return;
    char tmp[21];
    int n = 0;
    bool neg = v < 0;
    uint64_t u = neg ? (uint64_t)(-(v + 1)) + 1 : (uint64_t)v;
    do
    {
        tmp[n++] = (char)('0' + (int)(u % 10));
        u /= 10;
    } while (u);
    char out[22];
    int k = 0;
    if (neg)
        out[k++] = '-';
    for (int i = 0; i < n; i++)
        out[k++] = tmp[n - 1 - i];
    out[k] = '\0';
    put(b, out);
}

size_t finish(Buf *b)
{
    if (!b->ok)
        return 0;
    b->p[b->len] = '\0';
    return b->len;
}

const char *NS = " xmlns=\"urn:ieee:std:2030.5:ns\"";
const char *DECL = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
} // namespace

size_t detws_sep2_device_capability(uint32_t poll_rate, const char *edev_list_href, const char *derp_list_href,
                                    char *out, size_t cap)
{
    Buf b = {out, cap, 0, out != nullptr && cap > 0};
    put(&b, DECL);
    put(&b, "<DeviceCapability");
    put(&b, NS);
    put(&b, " pollRate=\"");
    put_i64(&b, poll_rate);
    put(&b, "\">");
    put(&b, "<EndDeviceListLink href=\"");
    put_escaped(&b, edev_list_href);
    put(&b, "\"/>");
    put(&b, "<DERProgramListLink href=\"");
    put_escaped(&b, derp_list_href);
    put(&b, "\"/>");
    put(&b, "</DeviceCapability>");
    return finish(&b);
}

size_t detws_sep2_end_device(uint64_t sfdi, const char *lfdi, const char *href, char *out, size_t cap)
{
    Buf b = {out, cap, 0, out != nullptr && cap > 0};
    put(&b, DECL);
    put(&b, "<EndDevice");
    put(&b, NS);
    put(&b, " href=\"");
    put_escaped(&b, href);
    put(&b, "\"><sFDI>");
    put_i64(&b, (int64_t)sfdi);
    put(&b, "</sFDI><lFDI>");
    put_escaped(&b, lfdi);
    put(&b, "</lFDI></EndDevice>");
    return finish(&b);
}

size_t detws_sep2_der_control(const char *mrid, uint32_t start, uint32_t duration, int32_t opmod_target_w, char *out,
                              size_t cap)
{
    Buf b = {out, cap, 0, out != nullptr && cap > 0};
    put(&b, DECL);
    put(&b, "<DERControl");
    put(&b, NS);
    put(&b, "><mRID>");
    put_escaped(&b, mrid);
    put(&b, "</mRID><interval><start>");
    put_i64(&b, start);
    put(&b, "</start><duration>");
    put_i64(&b, duration);
    put(&b, "</duration></interval><DERControlBase><opModFixedW>");
    put_i64(&b, opmod_target_w);
    put(&b, "</opModFixedW></DERControlBase></DERControl>");
    return finish(&b);
}

#endif // DETWS_ENABLE_SEP2
