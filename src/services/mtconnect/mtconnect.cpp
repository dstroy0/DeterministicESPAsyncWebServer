// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mtconnect.cpp
 * @brief MTConnect agent response codec (see mtconnect.h).
 */

#include "services/mtconnect/mtconnect.h"

#if DETWS_ENABLE_MTCONNECT

#include <string.h>

namespace
{
void put(DetwsMtcStreams *s, const char *text)
{
    if (!s->ok)
        return;
    size_t tl = strlen(text);
    if (s->len + tl >= s->cap)
    {
        s->ok = false;
        return;
    }
    memcpy(s->buf + s->len, text, tl);
    s->len += tl;
}

void put_escaped(DetwsMtcStreams *s, const char *text)
{
    if (!s->ok)
        return;
    for (const char *p = text; *p; p++)
    {
        const char *rep = nullptr;
        switch (*p)
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
            put(s, rep);
        else
        {
            if (s->len + 1 >= s->cap)
            {
                s->ok = false;
                return;
            }
            s->buf[s->len++] = *p;
        }
    }
}

// A minimal unsigned -> decimal directly into the stream.
void put_u64(DetwsMtcStreams *s, uint64_t v)
{
    char tmp[20];
    int n = 0;
    do
    {
        tmp[n++] = (char)('0' + (int)(v % 10));
        v /= 10;
    } while (v);
    char out[21];
    for (int i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    put(s, out);
}

const char *mtc_cat_str(DetwsMtcCategory cat)
{
    if (cat == DETWS_MTC_SAMPLE)
        return "SAMPLE";
    if (cat == DETWS_MTC_EVENT)
        return "EVENT";
    return "CONDITION";
}
} // namespace

void detws_mtc_streams_begin(DetwsMtcStreams *s, char *buf, size_t cap, uint64_t instance_id, uint64_t next_seq,
                             const char *device_name)
{
    s->buf = buf;
    s->cap = cap;
    s->len = 0;
    s->ok = (buf != nullptr && cap > 0);
    s->in_comp = false;
    put(s, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    put(s, "<MTConnectStreams xmlns=\"urn:mtconnect.org:MTConnectStreams:1.4\">");
    put(s, "<Header instanceId=\"");
    put_u64(s, instance_id);
    put(s, "\" version=\"1.4\" nextSequence=\"");
    put_u64(s, next_seq);
    put(s, "\"/>");
    put(s, "<Streams><DeviceStream name=\"");
    put_escaped(s, device_name ? device_name : "");
    put(s, "\">");
}

void detws_mtc_streams_add(DetwsMtcStreams *s, DetwsMtcCategory cat, const char *type, const char *data_id,
                           uint64_t seq, const char *timestamp, const char *value)
{
    if (!s->ok)
        return;
    if (!s->in_comp)
    {
        put(s, "<ComponentStream component=\"Device\">");
        s->in_comp = true;
    }
    const char *wrap = "Condition";
    if (cat == DETWS_MTC_SAMPLE)
        wrap = "Samples";
    else if (cat == DETWS_MTC_EVENT)
        wrap = "Events";
    put(s, "<");
    put(s, wrap);
    put(s, ">");
    if (cat == DETWS_MTC_CONDITION)
    {
        // <Condition><Normal type="TYPE" dataItemId="ID" sequence="SEQ" timestamp="TS"/></Condition>
        const char *sub = value ? value : "Normal"; // Normal / Warning / Fault / Unavailable
        put(s, "<");
        put(s, sub);
        put(s, " type=\"");
        put_escaped(s, type ? type : "");
        put(s, "\" dataItemId=\"");
        put_escaped(s, data_id ? data_id : "");
        put(s, "\" sequence=\"");
        put_u64(s, seq);
        put(s, "\" timestamp=\"");
        put_escaped(s, timestamp ? timestamp : "");
        put(s, "\"/>");
    }
    else
    {
        // <Type dataItemId="ID" sequence="SEQ" timestamp="TS">VALUE</Type>
        put(s, "<");
        put(s, type ? type : "");
        put(s, " dataItemId=\"");
        put_escaped(s, data_id ? data_id : "");
        put(s, "\" sequence=\"");
        put_u64(s, seq);
        put(s, "\" timestamp=\"");
        put_escaped(s, timestamp ? timestamp : "");
        put(s, "\">");
        put_escaped(s, value ? value : "");
        put(s, "</");
        put(s, type ? type : "");
        put(s, ">");
    }
    put(s, "</");
    put(s, wrap);
    put(s, ">");
}

size_t detws_mtc_streams_end(DetwsMtcStreams *s)
{
    if (s->in_comp)
        put(s, "</ComponentStream>");
    put(s, "</DeviceStream></Streams></MTConnectStreams>");
    if (!s->ok)
        return 0;
    s->buf[s->len] = '\0';
    return s->len;
}

size_t detws_mtc_error(uint64_t instance_id, const char *error_code, const char *message, char *out, size_t cap)
{
    DetwsMtcStreams s;
    s.buf = out;
    s.cap = cap;
    s.len = 0;
    s.ok = (out != nullptr && cap > 0);
    s.in_comp = false;
    put(&s, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    put(&s, "<MTConnectError xmlns=\"urn:mtconnect.org:MTConnectError:1.4\">");
    put(&s, "<Header instanceId=\"");
    put_u64(&s, instance_id);
    put(&s, "\" version=\"1.4\"/>");
    put(&s, "<Errors><Error errorCode=\"");
    put_escaped(&s, error_code ? error_code : "");
    put(&s, "\">");
    put_escaped(&s, message ? message : "");
    put(&s, "</Error></Errors></MTConnectError>");
    if (!s.ok)
        return 0;
    out[s.len] = '\0';
    return s.len;
}

// --- probe (MTConnectDevices): the device model a client discovers before streaming ---

void detws_mtc_devices_begin(DetwsMtcStreams *s, char *buf, size_t cap, uint64_t instance_id, const char *device_id,
                             const char *device_name, const char *uuid)
{
    s->buf = buf;
    s->cap = cap;
    s->len = 0;
    s->ok = (buf != nullptr && cap > 0);
    s->in_comp = false;
    put(s, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    put(s, "<MTConnectDevices xmlns=\"urn:mtconnect.org:MTConnectDevices:1.4\">");
    put(s, "<Header instanceId=\"");
    put_u64(s, instance_id);
    put(s, "\" version=\"1.4\"/>");
    put(s, "<Devices><Device id=\"");
    put_escaped(s, device_id ? device_id : "");
    put(s, "\" name=\"");
    put_escaped(s, device_name ? device_name : "");
    put(s, "\" uuid=\"");
    put_escaped(s, uuid ? uuid : "");
    put(s, "\"><DataItems>");
}

void detws_mtc_devices_add_item(DetwsMtcStreams *s, DetwsMtcCategory cat, const char *id, const char *type,
                                const char *name, const char *units)
{
    if (!s->ok)
        return;
    put(s, "<DataItem category=\"");
    put(s, mtc_cat_str(cat));
    put(s, "\" id=\"");
    put_escaped(s, id ? id : "");
    put(s, "\" type=\"");
    put_escaped(s, type ? type : "");
    put(s, "\"");
    if (name && name[0])
    {
        put(s, " name=\"");
        put_escaped(s, name);
        put(s, "\"");
    }
    if (units && units[0])
    {
        put(s, " units=\"");
        put_escaped(s, units);
        put(s, "\"");
    }
    put(s, "/>");
}

size_t detws_mtc_devices_end(DetwsMtcStreams *s)
{
    put(s, "</DataItems></Device></Devices></MTConnectDevices>");
    if (!s->ok)
        return 0;
    s->buf[s->len] = '\0';
    return s->len;
}

#endif // DETWS_ENABLE_MTCONNECT
