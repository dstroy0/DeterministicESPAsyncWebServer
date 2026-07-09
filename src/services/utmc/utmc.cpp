// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file utmc.cpp
 * @brief UTMC common-database codec (see utmc.h).
 */

#include "services/utmc/utmc.h"

#if DETWS_ENABLE_UTMC

#include <string.h>

#include "shared_primitives/strbuf.h"

namespace
{
void put_u(DetSb *b, uint32_t v)
{
    char tmp[11];
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + (int)(v % 10));
        v /= 10;
    } while (v);
    char out[12];
    for (int i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    det_sb_put(b, out);
}
} // namespace

size_t detws_utmc_request(const char *object_id, char *out, size_t cap)
{
    DetSb b = {out, cap, 0, out != nullptr && cap > 0};
    det_sb_put(&b, "<?xml version=\"1.0\"?><UTMCRequest><object id=\"");
    det_sb_xml(&b, object_id);
    det_sb_put(&b, "\"/></UTMCRequest>");
    return det_sb_finish(&b);
}

size_t detws_utmc_response(const char *object_id, const char *value, uint8_t quality, const char *timestamp, char *out,
                           size_t cap)
{
    DetSb b = {out, cap, 0, out != nullptr && cap > 0};
    det_sb_put(&b, "<?xml version=\"1.0\"?><UTMCResponse><object id=\"");
    det_sb_xml(&b, object_id);
    det_sb_put(&b, "\" value=\"");
    det_sb_xml(&b, value);
    det_sb_put(&b, "\" quality=\"");
    put_u(&b, quality);
    det_sb_put(&b, "\" timestamp=\"");
    det_sb_xml(&b, timestamp);
    det_sb_put(&b, "\"/></UTMCResponse>");
    return det_sb_finish(&b);
}

size_t detws_utmc_parse_request(const char *xml, size_t len, char *out, size_t cap)
{
    if (!xml || !out || cap == 0)
        return 0;
    // Find `id="` and copy up to the next quote.
    const char *key = "id=\"";
    size_t kl = 4;
    for (size_t i = 0; i + kl < len; i++)
    {
        if (memcmp(xml + i, key, kl) == 0)
        {
            size_t j = i + kl, k = 0;
            while (j < len && xml[j] != '"')
            {
                if (k + 1 >= cap)
                    return 0;
                out[k++] = xml[j++];
            }
            if (j >= len) // unterminated
                return 0;
            out[k] = '\0';
            return k;
        }
    }
    return 0;
}

#endif // DETWS_ENABLE_UTMC
