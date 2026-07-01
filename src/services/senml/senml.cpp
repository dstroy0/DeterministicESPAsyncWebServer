// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file senml.cpp
 * @brief SenML-JSON + SenML-CBOR pack builders (pure, host-tested).
 */

#include "services/senml/senml.h"

#if DETWS_ENABLE_SENML

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

size_t senml_json_build(char *buf, size_t cap, const SenmlRecord *records, size_t count)
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
        switch (r.value_kind)
        {
        case SENML_V_FLOAT:
            w.key("v");
            json_num(w, r.value);
            break;
        case SENML_V_STRING:
            if (r.value_str)
                w.kv_str("vs", r.value_str);
            break;
        case SENML_V_BOOL:
            w.kv_bool("vb", r.value_bool);
            break;
        case SENML_V_NONE:
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
static void cbor_num(CborWriter *w, double d)
{
    if (is_integral(d))
        cbor_int(w, (int64_t)d);
    else
        cbor_float(w, (float)d);
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
    if (r.value_kind != SENML_V_NONE && !(r.value_kind == SENML_V_STRING && !r.value_str))
        n++;
    if (r.has_time)
        n++;
    return n;
}

size_t senml_cbor_build(uint8_t *buf, size_t cap, const SenmlRecord *records, size_t count)
{
    if (!buf || (count && !records))
        return 0;
    CborWriter w;
    cbor_init(&w, buf, cap);
    cbor_array(&w, count);
    for (size_t i = 0; i < count; i++)
    {
        const SenmlRecord &r = records[i];
        cbor_map(&w, record_fields(r));
        if (r.base_name)
        {
            cbor_int(&w, SENML_LBL_BN);
            cbor_text(&w, r.base_name);
        }
        if (r.has_base_time)
        {
            cbor_int(&w, SENML_LBL_BT);
            cbor_num(&w, r.base_time);
        }
        if (r.name)
        {
            cbor_int(&w, SENML_LBL_N);
            cbor_text(&w, r.name);
        }
        if (r.unit)
        {
            cbor_int(&w, SENML_LBL_U);
            cbor_text(&w, r.unit);
        }
        switch (r.value_kind)
        {
        case SENML_V_FLOAT:
            cbor_int(&w, SENML_LBL_V);
            cbor_num(&w, r.value);
            break;
        case SENML_V_STRING:
            if (r.value_str)
            {
                cbor_int(&w, SENML_LBL_VS);
                cbor_text(&w, r.value_str);
            }
            break;
        case SENML_V_BOOL:
            cbor_int(&w, SENML_LBL_VB);
            cbor_bool(&w, r.value_bool);
            break;
        case SENML_V_NONE:
            break;
        }
        if (r.has_time)
        {
            cbor_int(&w, SENML_LBL_T);
            cbor_num(&w, r.time);
        }
    }
    return cbor_ok(&w) ? cbor_len(&w) : 0;
}

#endif // DETWS_ENABLE_SENML
