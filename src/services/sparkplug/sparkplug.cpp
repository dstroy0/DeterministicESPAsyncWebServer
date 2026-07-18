// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sparkplug.cpp
 * @brief Sparkplug B payload + topic builder over the Protobuf codec (pure, host-tested).
 */

#include "services/sparkplug/sparkplug.h"

#if DWS_ENABLE_SPARKPLUG

#include "services/protobuf/protobuf.h"
#include <string.h>

// Tahu Payload field numbers.
#define SPB_PL_TIMESTAMP 1
#define SPB_PL_METRICS 2
#define SPB_PL_SEQ 3

// Tahu Metric field numbers.
#define SPB_MET_NAME 1
#define SPB_MET_ALIAS 2
#define SPB_MET_TIMESTAMP 3
#define SPB_MET_DATATYPE 4
#define SPB_MET_INT 10
#define SPB_MET_LONG 11
#define SPB_MET_FLOAT 12
#define SPB_MET_DOUBLE 13
#define SPB_MET_BOOL 14
#define SPB_MET_STRING 15

size_t dws_spb_build_topic(char *buf, size_t cap, const char *group, const char *message_type, const char *edge_node,
                           const char *device)
{
    if (!buf || !group || !message_type || !edge_node)
        return 0;
    // spBv1.0/<group>/<message_type>/<edge_node>[/<device>]
    size_t need = 8 /*"spBv1.0/"*/ + strnlen(group, cap) + 1 + strnlen(message_type, cap) + 1 + strnlen(edge_node, cap);
    if (device)
        need += 1 + strnlen(device, cap);
    if (need + 1 > cap) // + NUL
        return 0;
    size_t p = 0;
    const char *prefix = "spBv1.0/";
    memcpy(buf + p, prefix, 8);
    p += 8;
    size_t n = strnlen(group, cap);
    memcpy(buf + p, group, n);
    p += n;
    buf[p++] = '/';
    n = strnlen(message_type, cap);
    memcpy(buf + p, message_type, n);
    p += n;
    buf[p++] = '/';
    n = strnlen(edge_node, cap);
    memcpy(buf + p, edge_node, n);
    p += n;
    if (device)
    {
        buf[p++] = '/';
        n = strnlen(device, cap);
        memcpy(buf + p, device, n);
        p += n;
    }
    buf[p] = '\0';
    return p;
}

size_t dws_spb_build_metric(uint8_t *buf, size_t cap, const SpbMetric *m)
{
    if (!buf || !m)
        return 0;
    PbWriter w;
    dws_pb_writer_init(&w, buf, cap);
    if (m->name)
        dws_pb_string(&w, SPB_MET_NAME, m->name);
    if (m->has_alias)
        dws_pb_uint64(&w, SPB_MET_ALIAS, m->alias);
    if (m->has_timestamp)
        dws_pb_uint64(&w, SPB_MET_TIMESTAMP, m->timestamp);
    dws_pb_uint64(&w, SPB_MET_DATATYPE, m->datatype); // datatype is a uint32; the varint covers it
    switch (m->kind)
    {
    case SpbMetricKind::SPB_M_INT:
        dws_pb_uint64(&w, SPB_MET_INT, m->int_value);
        break;
    case SpbMetricKind::SPB_M_LONG:
        dws_pb_uint64(&w, SPB_MET_LONG, m->long_value);
        break;
    case SpbMetricKind::SPB_M_FLOAT:
        dws_pb_float(&w, SPB_MET_FLOAT, m->float_value);
        break;
    case SpbMetricKind::SPB_M_DOUBLE:
        dws_pb_double(&w, SPB_MET_DOUBLE, m->double_value);
        break;
    case SpbMetricKind::SPB_M_BOOL:
        dws_pb_bool(&w, SPB_MET_BOOL, m->bool_value);
        break;
    case SpbMetricKind::SPB_M_STRING:
        if (m->string_value)
            dws_pb_string(&w, SPB_MET_STRING, m->string_value);
        break;
    }
    return dws_pb_writer_finish(&w);
}

size_t dws_spb_build_payload(uint8_t *buf, size_t cap, uint64_t timestamp, uint64_t seq, const SpbMetric *metrics,
                             size_t n)
{
    if (!buf || (n && !metrics))
        return 0;
    PbWriter w;
    dws_pb_writer_init(&w, buf, cap);
    dws_pb_uint64(&w, SPB_PL_TIMESTAMP, timestamp);
    for (size_t i = 0; i < n; i++)
    {
        // Serialize each Metric submessage into a bounded temp, then add it as a
        // length-delimited field (Payload.metrics). A metric stays well under this bound
        // unless it carries a large string, in which case the build fails closed.
        uint8_t metric[DWS_SPB_METRIC_MAX];
        size_t mlen = dws_spb_build_metric(metric, sizeof(metric), &metrics[i]);
        if (!mlen)
            return 0;
        dws_pb_bytes(&w, SPB_PL_METRICS, metric, mlen);
    }
    dws_pb_uint64(&w, SPB_PL_SEQ, seq);
    return dws_pb_writer_finish(&w);
}

#endif // DWS_ENABLE_SPARKPLUG
