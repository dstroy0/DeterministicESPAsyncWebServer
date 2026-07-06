// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mtconnect.h
 * @brief MTConnect agent response codec (DETWS_ENABLE_MTCONNECT).
 *
 * MTConnect (ANSI/MTC1.4) is the manufacturing-equipment read standard: an HTTP agent answers `probe`,
 * `current`, `sample`, and `asset` requests with XML documents. This builds the two most-used response
 * documents into a caller buffer, so the web server is an MTConnect agent over the existing HTTP stack:
 *
 *  - **MTConnectStreams** (the `current` / `sample` response): a header carrying the agent
 *    instanceId + nextSequence, then per-DataItem `<Samples>/<Events>/<Condition>` values.
 *  - **MTConnectError** (a request error): the header + an `<Errors><Error errorCode=..>` element.
 *
 * A streams document is assembled incrementally: open it, add each observation, close it. The instanceId
 * (an agent-boot id) + a monotonically increasing sequence number give a subscriber the from/count
 * long-poll semantics. Pure text framing, zero heap, no stdlib, host-testable; values are XML-escaped.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H
#define DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_MTCONNECT

/** @brief The MTConnect DataItem category (which stream element wraps the value). */
enum DetwsMtcCategory
{
    DETWS_MTC_SAMPLE,   ///< a measured value (<Samples>).
    DETWS_MTC_EVENT,    ///< a discrete state (<Events>).
    DETWS_MTC_CONDITION ///< a condition (<Condition>): value is the sub-element name (Normal/Warning/Fault).
};

/** @brief Incremental MTConnectStreams builder over a caller buffer. */
struct DetwsMtcStreams
{
    char *buf;
    size_t cap;
    size_t len;   ///< bytes written so far (excl NUL).
    bool ok;      ///< cleared on any overflow; the final length is 0 when not ok.
    bool in_comp; ///< a <ComponentStream> is open.
};

/**
 * @brief Begin an MTConnectStreams document: XML declaration + header + open <Streams>.
 * @param instance_id agent instanceId (boot id).
 * @param next_seq    the nextSequence the agent will assign.
 * @param device_name the single device's name/uuid for the ComponentStream.
 */
void detws_mtc_streams_begin(DetwsMtcStreams *s, char *buf, size_t cap, uint64_t instance_id, uint64_t next_seq,
                             const char *device_name);

/**
 * @brief Add one observation.
 * @param cat        the DataItem category.
 * @param type       the DataItem type element name (e.g. "Position", "Execution", "Availability").
 * @param data_id    the DataItem id attribute.
 * @param seq        this observation's sequence number.
 * @param timestamp  ISO-8601 timestamp string.
 * @param value      the value text (XML-escaped); for a CONDITION it is the sub-element (Normal/Fault/...).
 */
void detws_mtc_streams_add(DetwsMtcStreams *s, DetwsMtcCategory cat, const char *type, const char *data_id,
                           uint64_t seq, const char *timestamp, const char *value);

/** @brief Finish the document (close any open component + <Streams> + root). @return length, or 0 on overflow. */
size_t detws_mtc_streams_end(DetwsMtcStreams *s);

/**
 * @brief Build a complete MTConnectError document.
 * @return length written, or 0 on overflow.
 */
size_t detws_mtc_error(uint64_t instance_id, const char *error_code, const char *message, char *out, size_t cap);

#endif // DETWS_ENABLE_MTCONNECT
#endif // DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H
