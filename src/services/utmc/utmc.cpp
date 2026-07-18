// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file utmc.cpp
 * @brief UTMC common-database codec (see utmc.h).
 */

#include "services/utmc/utmc.h"

#if DWS_ENABLE_UTMC

#include <string.h>

#include "shared_primitives/strbuf.h"

namespace
{
void put_u(DWSSb *b, uint32_t v)
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
    dws_sb_put(b, out);
}
} // namespace

size_t dws_utmc_request(const char *object_id, char *out, size_t cap)
{
    DWSSb b = {out, cap, 0, out != nullptr && cap > 0};
    dws_sb_put(&b, "<?xml version=\"1.0\"?><UTMCRequest><object id=\"");
    dws_sb_xml(&b, object_id);
    dws_sb_put(&b, "\"/></UTMCRequest>");
    return dws_sb_finish(&b);
}

size_t dws_utmc_response(const char *object_id, const char *value, uint8_t quality, const char *timestamp, char *out,
                         size_t cap)
{
    DWSSb b = {out, cap, 0, out != nullptr && cap > 0};
    dws_sb_put(&b, "<?xml version=\"1.0\"?><UTMCResponse><object id=\"");
    dws_sb_xml(&b, object_id);
    dws_sb_put(&b, "\" value=\"");
    dws_sb_xml(&b, value);
    dws_sb_put(&b, "\" quality=\"");
    put_u(&b, quality);
    dws_sb_put(&b, "\" timestamp=\"");
    dws_sb_xml(&b, timestamp);
    dws_sb_put(&b, "\"/></UTMCResponse>");
    return dws_sb_finish(&b);
}

size_t dws_utmc_parse_request(const char *xml, size_t len, char *out, size_t cap)
{
    if (!xml || !out || cap == 0)
        return 0;
    // Find `id="` and copy up to the next quote.
    const char *key = "id=\"";
    size_t kl = 4;
    for (size_t i = 0; i + kl < len; i++)
    {
        if (memcmp(xml + i, key, kl) != 0)
            continue;
        size_t j = i + kl;
        size_t k = 0;
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
    return 0;
}

#endif // DWS_ENABLE_UTMC
