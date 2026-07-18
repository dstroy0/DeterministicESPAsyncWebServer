// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file senml.h
 * @brief SenML (RFC 8428) sensor-measurement pack builder (DWS_ENABLE_SENML) - zero-heap
 *        SenML-JSON and SenML-CBOR encoders over the shipped JSON / CBOR codecs. SenML is
 *        the standard measurement format for CoAP / LwM2M / HTTP telemetry.
 *
 * A SenML pack is an array of records. Each record carries an optional base name / base time
 * (applied to the records that follow), a name, a unit, and exactly one value (a number, a
 * string, or a boolean), plus an optional time:
 * @code
 *   [{"bn":"urn:dev:ow:10e2073a01080063","u":"Cel","v":23.1}]
 * @endcode
 * SenML-JSON uses the text labels (bn/bt/n/u/v/vs/vb/t); SenML-CBOR uses the integer labels
 * (n=0 u=1 v=2 vs=3 vb=4 t=6, bn=-2 bt=-3) in a CBOR map per record. Numbers that are
 * integral are emitted as integers (so timestamps keep full precision); otherwise as floats.
 *
 * The caller fills a @ref SenmlRecord array and the builder emits the whole pack into a
 * caller buffer (fail-closed on overflow). Verified against the RFC 8428 example.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SENML_H
#define DETERMINISTICESPASYNCWEBSERVER_SENML_H

#include "ServerConfig.h"

#if DWS_ENABLE_SENML

#include <stddef.h>
#include <stdint.h>

/** @brief Which value field a record carries. */
enum class SenmlValueKind : uint8_t
{
    SENML_V_NONE,   ///< no value (e.g. a base-only record)
    SENML_V_FLOAT,  ///< numeric value (v) - emitted as int when integral, else float
    SENML_V_STRING, ///< string value (vs)
    SENML_V_BOOL    ///< boolean value (vb)
};

/** @brief One SenML record. String pointers are borrowed (not copied); nullptr fields are skipped. */
struct SenmlRecord
{
    const char *base_name; ///< bn (optional)
    bool has_base_time;
    double base_time; ///< bt (optional)
    const char *name; ///< n (optional)
    const char *unit; ///< u (optional)
    SenmlValueKind value_kind;
    double value;          ///< v (when value_kind == SenmlValueKind::SENML_V_FLOAT)
    const char *value_str; ///< vs (when value_kind == SenmlValueKind::SENML_V_STRING)
    bool value_bool;       ///< vb (when value_kind == SenmlValueKind::SENML_V_BOOL)
    bool has_time;
    double time; ///< t (optional)
};

/** @brief Build a SenML-JSON pack. Returns bytes written (excluding NUL), or 0 on overflow. */
size_t senml_json_build(char *buf, size_t cap, const SenmlRecord *records, size_t count);

/** @brief Build a SenML-CBOR pack (a CBOR array of integer-keyed maps). Returns bytes, or 0. */
size_t senml_cbor_build(uint8_t *buf, size_t cap, const SenmlRecord *records, size_t count);

#endif // DWS_ENABLE_SENML

#endif // DETERMINISTICESPASYNCWEBSERVER_SENML_H
