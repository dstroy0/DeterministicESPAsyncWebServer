// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file webhook.cpp
 * @brief IFTTT URL / payload builders (pure) + fire over the http_client.
 */

#include "services/webhook/webhook.h"

#if DETWS_ENABLE_WEBHOOK

#include "shared_primitives/mime.h"

#include <stdio.h>
#include <string.h>

namespace
{
bool put(char *out, size_t cap, size_t *pos, const char *s)
{
    size_t n = strlen(s);
    if (*pos + n >= cap)
        return false;
    memcpy(out + *pos, s, n);
    *pos += n;
    out[*pos] = '\0';
    return true;
}

// Append a JSON string value, escaping '"' and '\'.
bool put_escaped(char *out, size_t cap, size_t *pos, const char *s)
{
    for (; *s; s++)
    {
        char c = *s;
        if (c == '"' || c == '\\')
        {
            if (*pos + 2 >= cap)
                return false;
            out[(*pos)++] = '\\';
            out[(*pos)++] = c;
        }
        else
        {
            if (*pos + 1 >= cap)
                return false;
            out[(*pos)++] = c;
        }
        out[*pos] = '\0';
    }
    return true;
}
} // namespace

int detws_ifttt_url(const char *event, const char *key, char *out, size_t cap)
{
    if (!out || cap == 0 || !event || !key)
    {
        if (out && cap)
            out[0] = '\0';
        return 0;
    }
    int w = snprintf(out, cap, "https://maker.ifttt.com/trigger/%s/with/key/%s", event, key);
    if (w < 0 || (size_t)w >= cap)
    {
        out[0] = '\0';
        return 0;
    }
    return w;
}

int detws_ifttt_payload(const char *v1, const char *v2, const char *v3, char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    out[0] = '\0';
    size_t pos = 0;
    bool ok = put(out, cap, &pos, "{");
    const char *names[3] = {"value1", "value2", "value3"};
    const char *vals[3] = {v1, v2, v3};
    bool first = true;
    for (int i = 0; i < 3 && ok; i++)
    {
        if (!vals[i])
            continue;
        if (!first)
            ok = ok && put(out, cap, &pos, ",");
        first = false;
        ok = ok && put(out, cap, &pos, "\"");
        ok = ok && put(out, cap, &pos, names[i]);
        ok = ok && put(out, cap, &pos, "\":\"");
        ok = ok && put_escaped(out, cap, &pos, vals[i]);
        ok = ok && put(out, cap, &pos, "\"");
    }
    ok = ok && put(out, cap, &pos, "}");
    if (!ok)
    {
        out[0] = '\0';
        return 0;
    }
    return (int)pos;
}

#if DETWS_ENABLE_HTTP_CLIENT

#include "services/http_client/http_client.h"

int detws_webhook_post(const char *url, const char *json)
{
    if (!url || !json)
        return HTTP_CLIENT_ERR_URL;
    HttpClientResult r;
    return http_post(url, DET_MIME_JSON, (const uint8_t *)json, strlen(json), &r);
}

#else // http_client not enabled in this build

int detws_webhook_post(const char *, const char *)
{
    return -1;
}

#endif // DETWS_ENABLE_HTTP_CLIENT

int detws_ifttt_trigger(const char *event, const char *key, const char *v1, const char *v2, const char *v3)
{
    char url[160];
    char body[256];
    if (detws_ifttt_url(event, key, url, sizeof(url)) == 0)
        return -1;
    if (detws_ifttt_payload(v1, v2, v3, body, sizeof(body)) == 0)
        return -1;
    return detws_webhook_post(url, body);
}

#endif // DETWS_ENABLE_WEBHOOK
