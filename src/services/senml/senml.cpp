// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file senml.cpp
 * @brief SenML-JSON + SenML-CBOR pack builders (pure, host-tested).
 */

#include "services/senml/senml.h"

#if DWS_ENABLE_SENML

#include "network_drivers/presentation/cbor/cbor.h"
#include "network_drivers/presentation/json/json.h"
#include <stdio.h> // snprintf for the JSON number formatting

// SenML-CBOR integer labels (RFC 8428).
#define SENML_LBL_BN (-2)
#define SENML_LBL_BT (-3)
#define SENML_LBL_N 0
#define SENML_LBL_U 1
#define SENML_LBL_V 2
#define SENML_LBL_VS 3
#define SENML_LBL_VB 4
#define SENML_LBL_T 6

// True when @p d is an integer that fits in int64 (so it can be emitted losslessly as one).
static bool is_integral(double d)
{
    return d >= -9.2e18 && d <= 9.2e18 && d == (double)(int64_t)d;
}

// Emit a SenML number into a JSON value position: an integer when integral (keeps timestamp
// precision), otherwise a %g float.
static void json_num(JsonWriter &w, double d)
{
    char tmp[32];
    if (is_integral(d))
        snprintf(tmp, sizeof(tmp), "%lld", (long long)d);
    else
        snprintf(tmp, sizeof(tmp), "%g", d);
    w.raw(tmp);
}

size_t dws_senml_json_build(char *buf, size_t cap, const SenmlRecord *records, size_t count)
{
    if (!buf || (count && !records))
        return 0;
    JsonWriter w(buf, cap);
    w.begin_array();
    for (size_t i = 0; i < count; i++)
    {
        const SenmlRecord &r = records[i];
        w.begin_object();
        if (r.base_name)
            w.kv_str("bn", r.base_name);
        if (r.has_base_time)
        {
            w.key("bt");
            json_num(w, r.base_time);
        }
        if (r.name)
            w.kv_str("n", r.name);
        if (r.unit)
            w.kv_str("u", r.unit);
        // Every SenmlValueKind enumerator has a case below, so the default edge the compiler
        // emits for the uint8_t-backed enum is unreachable for any value the API admits.
        switch (r.value_kind) // GCOVR_EXCL_LINE  exhaustive enum switch; the default edge is dead
        {
        case SenmlValueKind::SENML_V_FLOAT:
            w.key("v");
            json_num(w, r.value);
            break;
        case SenmlValueKind::SENML_V_STRING:
            if (r.value_str)
                w.kv_str("vs", r.value_str);
            break;
        case SenmlValueKind::SENML_V_BOOL:
            w.kv_bool("vb", r.value_bool);
            break;
        case SenmlValueKind::SENML_V_NONE:
            break;
        }
        if (r.has_time)
        {
            w.key("t");
            json_num(w, r.time);
        }
        w.end_object();
    }
    w.end_array();
    return w.ok() ? w.length() : 0;
}

// Emit a SenML number into a CBOR value: an integer when integral, else a float.
static void dws_cbor_num(CborWriter *w, double d)
{
    if (is_integral(d))
        dws_cbor_int(w, (int64_t)d);
    else
        dws_cbor_float(w, (float)d);
}

static size_t record_fields(const SenmlRecord &r)
{
    size_t n = 0;
    if (r.base_name)
        n++;
    if (r.has_base_time)
        n++;
    if (r.name)
        n++;
    if (r.unit)
        n++;
    if (r.value_kind != SenmlValueKind::SENML_V_NONE &&
        !(r.value_kind == SenmlValueKind::SENML_V_STRING && !r.value_str))
        n++;
    if (r.has_time)
        n++;
    return n;
}

size_t dws_senml_cbor_build(uint8_t *buf, size_t cap, const SenmlRecord *records, size_t count)
{
    if (!buf || (count && !records))
        return 0;
    CborWriter w;
    dws_cbor_init(&w, buf, cap);
    dws_cbor_array(&w, count);
    for (size_t i = 0; i < count; i++)
    {
        const SenmlRecord &r = records[i];
        dws_cbor_map(&w, record_fields(r));
        if (r.base_name)
        {
            dws_cbor_int(&w, SENML_LBL_BN);
            dws_cbor_text(&w, r.base_name);
        }
        if (r.has_base_time)
        {
            dws_cbor_int(&w, SENML_LBL_BT);
            dws_cbor_num(&w, r.base_time);
        }
        if (r.name)
        {
            dws_cbor_int(&w, SENML_LBL_N);
            dws_cbor_text(&w, r.name);
        }
        if (r.unit)
        {
            dws_cbor_int(&w, SENML_LBL_U);
            dws_cbor_text(&w, r.unit);
        }
        // Exhaustive over SenmlValueKind, as in the JSON builder above: the compiler's default
        // edge for the uint8_t-backed enum is unreachable, and record_fields() above is written
        // against the same four kinds so the declared field count always matches what is emitted.
        switch (r.value_kind) // GCOVR_EXCL_LINE  exhaustive enum switch; the default edge is dead
        {
        case SenmlValueKind::SENML_V_FLOAT:
            dws_cbor_int(&w, SENML_LBL_V);
            dws_cbor_num(&w, r.value);
            break;
        case SenmlValueKind::SENML_V_STRING:
            if (r.value_str)
            {
                dws_cbor_int(&w, SENML_LBL_VS);
                dws_cbor_text(&w, r.value_str);
            }
            break;
        case SenmlValueKind::SENML_V_BOOL:
            dws_cbor_int(&w, SENML_LBL_VB);
            dws_cbor_bool(&w, r.value_bool);
            break;
        case SenmlValueKind::SENML_V_NONE:
            break;
        }
        if (r.has_time)
        {
            dws_cbor_int(&w, SENML_LBL_T);
            dws_cbor_num(&w, r.time);
        }
    }
    return dws_cbor_ok(&w) ? dws_cbor_len(&w) : 0;
}

// --- resolution (RFC 8428 §4.6) ---

size_t dws_senml_resolve(const SenmlRecord *in, size_t n, SenmlResolved *out, size_t max)
{
    if (!in || !out)
        return 0;
    const char *base_name = nullptr; // the active base name (bn), carried forward
    bool base_time_set = false;
    double base_time = 0.0; // the active base time (bt)

    size_t count = n < max ? n : max;
    for (size_t i = 0; i < count; i++)
    {
        const SenmlRecord *r = &in[i];
        if (r->base_name) // a base field becomes active for this record and the ones after it
            base_name = r->base_name;
        if (r->has_base_time)
        {
            base_time = r->base_time;
            base_time_set = true;
        }

        SenmlResolved *o = &out[i];
        // Resolved name = active base name + record name (either part may be absent).
        int w = snprintf(o->name, sizeof(o->name), "%s%s", base_name ? base_name : "", r->name ? r->name : "");
        if (w < 0)
            o->name[0] = '\0'; // GCOVR_EXCL_LINE  snprintf on "%s%s" cannot encode-error

        // Resolved time = base time + record time (each defaults to 0); absent only if neither is present.
        o->has_time = base_time_set || r->has_time;
        o->time = (base_time_set ? base_time : 0.0) + (r->has_time ? r->time : 0.0);

        o->unit = r->unit;
        o->value_kind = r->value_kind;
        o->value = r->value;
        o->value_str = r->value_str;
        o->value_bool = r->value_bool;
    }
    return count;
}

#endif // DWS_ENABLE_SENML
