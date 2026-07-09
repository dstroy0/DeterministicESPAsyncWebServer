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
 *  - **MTConnectDevices** (the `probe` response): the device model - a `<Device>` with its
 *    `<DataItems>` (each `category`/`id`/`type`, optional `name`/`units`) - that a client
 *    discovers before it streams.
 *  - **MTConnectAssets** (the `asset` response): the tool/fixture inventory - a `<CuttingTool>`
 *    with its `<CuttingToolLifeCycle>` (`<ToolLife>` remaining minutes / part count) - that a
 *    client reads out of band from the observation stream.
 *  - **MTConnectError** (a request error): the header + an `<Errors><Error errorCode=..>` element.
 *
 * A streams document is assembled incrementally: open it, add each observation, close it. The instanceId
 * (an agent-boot id) + a monotonically increasing sequence number give a subscriber the from/count
 * long-poll semantics. Pure text framing, zero heap, no stdlib, host-testable; values are XML-escaped.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H
#define DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H

#include "ServerConfig.h"
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

/**
 * @brief Begin an MTConnectDevices (`probe`) document: XML declaration + header + open one `<Device>`.
 * @param instance_id agent instanceId (boot id).
 * @param device_id   the Device `id` attribute.
 * @param device_name the Device `name` attribute.
 * @param uuid        the Device `uuid` attribute.
 *
 * Reuses ::DetwsMtcStreams as the incremental buffer builder (as ::detws_mtc_error does).
 */
void detws_mtc_devices_begin(DetwsMtcStreams *s, char *buf, size_t cap, uint64_t instance_id, const char *device_id,
                             const char *device_name, const char *uuid);

/**
 * @brief Add one `<DataItem>` to the device model.
 * @param cat   the DataItem category (SAMPLE / EVENT / CONDITION).
 * @param id    the DataItem `id` attribute.
 * @param type  the DataItem `type` attribute (e.g. "Position", "Execution", "Availability").
 * @param name  optional `name` attribute (omitted when null/empty).
 * @param units optional `units` attribute (omitted when null/empty).
 */
void detws_mtc_devices_add_item(DetwsMtcStreams *s, DetwsMtcCategory cat, const char *id, const char *type,
                                const char *name, const char *units);

/** @brief Finish the probe document (close `<DataItems>` + `<Device>` + root). @return length, or 0 on overflow. */
size_t detws_mtc_devices_end(DetwsMtcStreams *s);

/**
 * @brief Begin an MTConnectAssets (`asset`) document: XML declaration + header + open `<Assets>`.
 * @param instance_id       agent instanceId (boot id).
 * @param asset_count       the number of assets in this response (the Header `assetCount`).
 * @param asset_buffer_size the agent's total asset capacity (the Header `assetBufferSize`).
 *
 * Reuses ::DetwsMtcStreams as the incremental buffer builder (as ::detws_mtc_devices_begin does).
 */
void detws_mtc_assets_begin(DetwsMtcStreams *s, char *buf, size_t cap, uint64_t instance_id, uint32_t asset_count,
                            uint32_t asset_buffer_size);

/**
 * @brief Open one `<CuttingTool>` asset and its `<CuttingToolLifeCycle>`.
 * @param asset_id      the CuttingTool `assetId` (required).
 * @param serial_number optional `serialNumber` attribute (omitted when null/empty).
 * @param tool_id       optional `toolId` attribute (omitted when null/empty).
 * @param device_uuid   optional `deviceUuid` attribute (omitted when null/empty).
 * @param timestamp     optional ISO-8601 `timestamp` attribute (omitted when null/empty).
 */
void detws_mtc_assets_cutting_tool_begin(DetwsMtcStreams *s, const char *asset_id, const char *serial_number,
                                         const char *tool_id, const char *device_uuid, const char *timestamp);

/**
 * @brief Add one `<ToolLife>` element to the open cutting tool's life cycle.
 * @param type            the life kind: "MINUTES", "PART_COUNT", or "WEAR".
 * @param count_direction "UP" (accumulating) or "DOWN" (remaining).
 * @param limit           optional `limit` attribute (the max/threshold; omitted when null/empty).
 * @param value           the current life value text (XML-escaped).
 */
void detws_mtc_assets_tool_life(DetwsMtcStreams *s, const char *type, const char *count_direction, const char *limit,
                                const char *value);

/** @brief Close the open `<CuttingToolLifeCycle>` + `<CuttingTool>`. */
void detws_mtc_assets_cutting_tool_end(DetwsMtcStreams *s);

/** @brief Finish the asset document (close `<Assets>` + root). @return length, or 0 on overflow. */
size_t detws_mtc_assets_end(DetwsMtcStreams *s);

#endif // DETWS_ENABLE_MTCONNECT
#endif // DETERMINISTICESPASYNCWEBSERVER_MTCONNECT_H
