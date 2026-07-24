// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ptp.h
 * @brief PTP / IEEE 1588-2008 (PTPv2) message codec + slave clock math (DWS_ENABLE_PTP).
 *
 * The Precision Time Protocol synchronizes clocks across a LAN to sub-microsecond accuracy by
 * exchanging timestamped messages. This codec builds and parses the PTPv2 wire format - the 34-octet
 * common header, the 10-octet (48-bit seconds + 32-bit nanoseconds) timestamp, and the Sync /
 * Delay_Req / Follow_Up / Delay_Resp / Announce messages - and computes an ordinary-clock **slave**'s
 * offset-from-master and mean-path-delay from the four transfer timestamps (t1..t4). All multi-octet
 * fields are big-endian (network order), per IEEE 1588-2008 clause 13. Pure and host-tested; the UDP
 * transport (event port 319, general port 320, multicast 224.0.1.129) and the local timestamping are
 * the application's - see the Ptp example for the ordinary-clock slave that drives this codec.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PTP_H
#define DETERMINISTICESPASYNCWEBSERVER_PTP_H

#include "ServerConfig.h"

#if DWS_ENABLE_PTP

#include <stddef.h>
#include <stdint.h>

/** @brief PTPv2 messageType values (low nibble of octet 0). */
enum DwsPtpMsgType
{
    DWS_PTP_SYNC = 0x0,
    DWS_PTP_DELAY_REQ = 0x1,
    DWS_PTP_FOLLOW_UP = 0x8,
    DWS_PTP_DELAY_RESP = 0x9,
    DWS_PTP_ANNOUNCE = 0xB
};

#define DWS_PTP_HEADER_LEN 34    ///< common header length
#define DWS_PTP_TS_LEN 10        ///< on-wire timestamp length (6-octet seconds + 4-octet nanoseconds)
#define DWS_PTP_EVENT_PORT 319   ///< UDP port for event messages (Sync, Delay_Req)
#define DWS_PTP_GENERAL_PORT 320 ///< UDP port for general messages (Follow_Up, Delay_Resp, Announce)

/** @brief A PTP timestamp: 48-bit seconds + 32-bit nanoseconds. */
struct DwsPtpTimestamp
{
    uint64_t seconds;     ///< seconds (only the low 48 bits are on the wire)
    uint32_t nanoseconds; ///< nanoseconds within the second (0 .. 999999999)
};

/** @brief The 34-octet PTPv2 common header. */
struct DwsPtpHeader
{
    uint8_t message_type;       ///< DwsPtpMsgType (octet 0 low nibble)
    uint8_t transport_specific; ///< octet 0 high nibble (majorSdoId)
    uint8_t version;            ///< PTP version (octet 1 low nibble); 2 for PTPv2
    uint16_t message_length;    ///< total message length in octets
    uint8_t domain;             ///< domainNumber
    uint16_t flags;             ///< flagField
    int64_t correction;         ///< correctionField (nanoseconds scaled by 2^16)
    uint8_t clock_identity[8];  ///< sourcePortIdentity clockIdentity
    uint16_t port_number;       ///< sourcePortIdentity portNumber
    uint16_t sequence_id;       ///< sequenceId
    uint8_t control;            ///< controlField
    int8_t log_interval;        ///< logMessageInterval
};

/** @brief Parsed Delay_Resp body. */
struct DwsPtpDelayResp
{
    DwsPtpTimestamp receive; ///< receiveTimestamp (t4, when the master got our Delay_Req)
    uint8_t req_clock_id[8]; ///< requestingPortIdentity clockIdentity (echoes our clock id)
    uint16_t req_port;       ///< requestingPortIdentity portNumber
};

/** @brief Parsed Announce body (the master's quality, for best-master selection / display). */
struct DwsPtpAnnounce
{
    DwsPtpTimestamp origin;    ///< originTimestamp
    int16_t utc_offset;        ///< currentUtcOffset (TAI - UTC, seconds)
    uint8_t gm_priority1;      ///< grandmasterPriority1
    uint8_t gm_clock_class;    ///< grandmasterClockQuality.clockClass
    uint8_t gm_clock_accuracy; ///< grandmasterClockQuality.clockAccuracy
    uint16_t gm_variance;      ///< grandmasterClockQuality.offsetScaledLogVariance
    uint8_t gm_priority2;      ///< grandmasterPriority2
    uint8_t gm_identity[8];    ///< grandmasterIdentity
    uint16_t steps_removed;    ///< stepsRemoved
    uint8_t time_source;       ///< timeSource
};

/** @brief Slave sync result: offset from master and mean path delay, in nanoseconds. */
struct DwsPtpSync
{
    int64_t offset_ns; ///< offsetFromMaster (local - master); subtract to correct the local clock
    int64_t delay_ns;  ///< meanPathDelay
};

// -- timestamp helpers --

/** @brief Write @p ts to @p p as the 10-octet on-wire form (big-endian). */
void dws_ptp_ts_write(uint8_t *p, const DwsPtpTimestamp *ts);
/** @brief Read a 10-octet on-wire timestamp from @p p into @p ts. */
void dws_ptp_ts_read(const uint8_t *p, DwsPtpTimestamp *ts);
/** @brief Convert @p ts to signed nanoseconds since its epoch (fits current epochs in int64). */
int64_t dws_ptp_ts_to_ns(const DwsPtpTimestamp *ts);
/** @brief Convert signed-nanoseconds @p ns to a timestamp. */
void dws_ptp_ts_from_ns(int64_t ns, DwsPtpTimestamp *ts);

// -- header --

/**
 * @brief Build the 34-octet common header into @p buf, stamping messageLength = 34 + @p body_len.
 * @return DWS_PTP_HEADER_LEN or 0 on overflow / bad args.
 */
size_t dws_ptp_build_header(uint8_t *buf, size_t cap, const DwsPtpHeader *h, uint16_t body_len);
/** @brief Parse the common header from @p s (@p len octets). Returns false if too short. */
bool dws_ptp_parse_header(const uint8_t *s, size_t len, DwsPtpHeader *h);

// -- messages (build stamps the type-specific messageType / control / length for you) --

/** @brief Build a Sync (@p origin is the originTimestamp; 0 for a two-step Sync). */
size_t dws_ptp_build_sync(uint8_t *buf, size_t cap, const DwsPtpHeader *h, const DwsPtpTimestamp *origin);
/** @brief Build a Delay_Req (@p origin is the originTimestamp; usually 0). */
size_t dws_ptp_build_delay_req(uint8_t *buf, size_t cap, const DwsPtpHeader *h, const DwsPtpTimestamp *origin);
/** @brief Build a Follow_Up carrying the precise Sync egress time @p precise (t1). */
size_t dws_ptp_build_follow_up(uint8_t *buf, size_t cap, const DwsPtpHeader *h, const DwsPtpTimestamp *precise);
/** @brief Build a Delay_Resp carrying t4 @p recv and the requester's port identity. */
size_t dws_ptp_build_delay_resp(uint8_t *buf, size_t cap, const DwsPtpHeader *h, const DwsPtpTimestamp *recv,
                                const uint8_t *req_clock_id, uint16_t req_port);
/** @brief Build an Announce from @p a - master mode: advertise this clock's quality + origin time. */
size_t dws_ptp_build_announce(uint8_t *buf, size_t cap, const DwsPtpHeader *h, const DwsPtpAnnounce *a);

/**
 * @brief Parse a Sync / Delay_Req / Follow_Up message: fills @p h and its single timestamp @p ts.
 * Returns false on a short frame or a non-timestamp message type.
 */
bool dws_ptp_parse_timestamp_msg(const uint8_t *s, size_t len, DwsPtpHeader *h, DwsPtpTimestamp *ts);
/** @brief Parse a Delay_Resp into @p h + @p out. Returns false on a short / wrong-type frame. */
bool dws_ptp_parse_delay_resp(const uint8_t *s, size_t len, DwsPtpHeader *h, DwsPtpDelayResp *out);
/** @brief Parse an Announce into @p h + @p out. Returns false on a short / wrong-type frame. */
bool dws_ptp_parse_announce(const uint8_t *s, size_t len, DwsPtpHeader *h, DwsPtpAnnounce *out);

// -- slave clock math --

/**
 * @brief Compute offsetFromMaster and meanPathDelay from the four PTP transfer timestamps, in
 * nanoseconds: t1 = Sync egress (master), t2 = Sync ingress (slave), t3 = Delay_Req egress (slave),
 * t4 = Delay_Req ingress (master). offset = ((t2-t1) - (t4-t3)) / 2, delay = ((t2-t1) + (t4-t3)) / 2.
 * Fold any correctionField into t1..t4 before calling.
 */
void dws_ptp_compute(int64_t t1, int64_t t2, int64_t t3, int64_t t4, DwsPtpSync *out);

#endif // DWS_ENABLE_PTP
#endif // DETERMINISTICESPASYNCWEBSERVER_PTP_H
