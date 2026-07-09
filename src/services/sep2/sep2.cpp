// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sep2.cpp
 * @brief IEEE 2030.5 resource codec (see sep2.h).
 */

#include "services/sep2/sep2.h"

#if DETWS_ENABLE_SEP2

#include <string.h>

#include "shared_primitives/strbuf.h"

namespace
{
void put_i64(DetSb *b, int64_t v)
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
    det_sb_put(b, out);
}

const char *NS = " xmlns=\"urn:ieee:std:2030.5:ns\"";
const char *DECL = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
} // namespace

size_t detws_sep2_device_capability(uint32_t poll_rate, const char *edev_list_href, const char *derp_list_href,
                                    char *out, size_t cap)
{
    DetSb b = {out, cap, 0, out != nullptr && cap > 0};
    det_sb_put(&b, DECL);
    det_sb_put(&b, "<DeviceCapability");
    det_sb_put(&b, NS);
    det_sb_put(&b, " pollRate=\"");
    put_i64(&b, poll_rate);
    det_sb_put(&b, "\">");
    det_sb_put(&b, "<EndDeviceListLink href=\"");
    det_sb_xml(&b, edev_list_href);
    det_sb_put(&b, "\"/>");
    det_sb_put(&b, "<DERProgramListLink href=\"");
    det_sb_xml(&b, derp_list_href);
    det_sb_put(&b, "\"/>");
    det_sb_put(&b, "</DeviceCapability>");
    return det_sb_finish(&b);
}

size_t detws_sep2_end_device(uint64_t sfdi, const char *lfdi, const char *href, char *out, size_t cap)
{
    DetSb b = {out, cap, 0, out != nullptr && cap > 0};
    det_sb_put(&b, DECL);
    det_sb_put(&b, "<EndDevice");
    det_sb_put(&b, NS);
    det_sb_put(&b, " href=\"");
    det_sb_xml(&b, href);
    det_sb_put(&b, "\"><sFDI>");
    put_i64(&b, (int64_t)sfdi);
    det_sb_put(&b, "</sFDI><lFDI>");
    det_sb_xml(&b, lfdi);
    det_sb_put(&b, "</lFDI></EndDevice>");
    return det_sb_finish(&b);
}

size_t detws_sep2_der_control(const char *mrid, uint32_t start, uint32_t duration, int32_t opmod_target_w, char *out,
                              size_t cap)
{
    DetSb b = {out, cap, 0, out != nullptr && cap > 0};
    det_sb_put(&b, DECL);
    det_sb_put(&b, "<DERControl");
    det_sb_put(&b, NS);
    det_sb_put(&b, "><mRID>");
    det_sb_xml(&b, mrid);
    det_sb_put(&b, "</mRID><interval><start>");
    put_i64(&b, start);
    det_sb_put(&b, "</start><duration>");
    put_i64(&b, duration);
    det_sb_put(&b, "</duration></interval><DERControlBase><opModFixedW>");
    put_i64(&b, opmod_target_w);
    det_sb_put(&b, "</opModFixedW></DERControlBase></DERControl>");
    return det_sb_finish(&b);
}

#endif // DETWS_ENABLE_SEP2
