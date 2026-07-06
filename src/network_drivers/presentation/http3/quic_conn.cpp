// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_conn.cpp
 * @brief Stateful QUIC v1 server connection engine (see quic_conn.h).
 */

#include "network_drivers/presentation/http3/quic_conn.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_aead.h" // QUIC_AEAD_TAG_LEN
#include "network_drivers/presentation/http3/quic_frame.h"
#include "network_drivers/presentation/http3/quic_packet.h"
#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>

namespace
{
// The open (decrypt) keys for an encryption level: Initial keys come from the DCID, Handshake and
// 1-RTT keys from the TLS handshake. Returns NULL if that level's keys are not available yet.
const QuicPacketKeys *open_keys(QuicConn *qc, int level)
{
    if (level == QUIC_ENC_INITIAL)
        return &qc->initial.client;
    return quic_tls_keys(&qc->tls, level, /*is_server=*/false);
}
const QuicPacketKeys *seal_keys(QuicConn *qc, int level)
{
    if (level == QUIC_ENC_INITIAL)
        return &qc->initial.server;
    return quic_tls_keys(&qc->tls, level, /*is_server=*/true);
}

// Find a stream slot by id, or allocate one; NULL if the table is full.
QuicStream *stream_get(QuicConn *qc, uint64_t id, bool create)
{
    QuicStream *free_slot = nullptr;
    for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
    {
        if (qc->streams[i].id == id && qc->streams[i].id != UINT64_MAX)
            return &qc->streams[i];
        if (!free_slot && qc->streams[i].id == UINT64_MAX)
            free_slot = &qc->streams[i];
    }
    if (!create || !free_slot)
        return nullptr;
    memset(free_slot, 0, sizeof(*free_slot));
    free_slot->id = id;
    return free_slot;
}
} // namespace

void quic_conn_init(QuicConn *qc, const QuicTlsConfig *cfg, const uint8_t *odcid, uint8_t odcid_len,
                    const uint8_t *peer_scid, uint8_t peer_scid_len, const uint8_t *our_scid, uint8_t our_scid_len,
                    const QuicConnCallbacks *cb)
{
    memset(qc, 0, sizeof(*qc));
    memcpy(qc->odcid, odcid, odcid_len);
    qc->odcid_len = odcid_len;
    memcpy(qc->dcid, peer_scid, peer_scid_len);
    qc->dcid_len = peer_scid_len;
    memcpy(qc->scid, our_scid, our_scid_len);
    qc->scid_len = our_scid_len;
    if (cb)
        qc->cb = *cb;

    quic_derive_initial_secrets(odcid, odcid_len, &qc->initial);

    for (int i = 0; i < 3; i++)
    {
        qc->space[i].largest_acked = -1;
        qc->space[i].last_ae_pn = -1;
    }
    for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
        qc->streams[i].id = UINT64_MAX;

    // The server's transport parameters carry the connection IDs the handshake must echo.
    QuicTlsConfig c = *cfg;
    c.params.has_original_dcid = true;
    memcpy(c.params.original_dcid, odcid, odcid_len);
    c.params.original_dcid_len = odcid_len;
    c.params.has_initial_scid = true;
    memcpy(c.params.initial_scid, our_scid, our_scid_len);
    c.params.initial_scid_len = our_scid_len;
    quic_tls_server_init(&qc->tls, &c);
}

// --- Frame handling --------------------------------------------------------------------------
namespace
{
// Queue a transport CONNECTION_CLOSE for a fatal error at @p level; the first error wins (RFC 9000 sec
// 10.2.3). Sending at the level the error was seen on guarantees the peer holds keys to read it.
void queue_close(QuicConn *qc, uint64_t error_code, uint64_t frame_type, int level)
{
    if (qc->close_queued || qc->closed)
        return;
    qc->close_queued = true;
    qc->close_error = error_code;
    qc->close_frame_type = frame_type;
    qc->close_level = (uint8_t)level;
}

void handle_crypto(QuicConn *qc, int level, const QuicFrame *f)
{
    QuicPnSpace *s = &qc->space[level];
    uint64_t want = s->crypto_rx_off;
    if (f->crypto.offset > want)
        return; // out-of-order beyond our window; the peer will retransmit
    if (f->crypto.offset + f->crypto.length <= want)
        return; // wholly duplicate
    size_t skip = (size_t)(want - f->crypto.offset);
    const uint8_t *nd = f->crypto.data + skip;
    size_t nl = (size_t)(f->crypto.length - skip);
    if (s->crypto_rx_have + nl > sizeof(s->crypto_rx))
        nl = sizeof(s->crypto_rx) - s->crypto_rx_have; // clamp to the reassembly window
    memcpy(s->crypto_rx + s->crypto_rx_have, nd, nl);
    s->crypto_rx_have += nl;
    s->crypto_rx_off += nl;

    size_t used = quic_tls_recv_crypto(&qc->tls, level, s->crypto_rx, s->crypto_rx_have);
    if (used)
    {
        memmove(s->crypto_rx, s->crypto_rx + used, s->crypto_rx_have - used);
        s->crypto_rx_have -= used;
    }
    // A fatal TLS error (bad Finished, unsupported handshake) becomes a QUIC CRYPTO_ERROR: report it
    // to the client with the TLS alert in the low byte (RFC 9001 sec 4.8) instead of stalling.
    if (qc->tls.state == QTLS_FAILED)
    {
        queue_close(qc, QUIC_ERR_CRYPTO_BASE + qc->tls.alert, QUIC_FT_CRYPTO, level);
        return;
    }
    // Completing the handshake opens 1-RTT and lets us send HANDSHAKE_DONE. We keep the Handshake
    // space live so the same outbound datagram still ACKs the client's Finished; HANDSHAKE_DONE (at
    // 1-RTT) is what tells the client the handshake is confirmed (RFC 9001 sec 4.1.2).
    if (qc->tls.state == QTLS_DONE && !qc->handshake_done_sent && !qc->handshake_done_queued)
    {
        qc->handshake_done_queued = true;
        qc->address_validated = true;
        if (qc->cb.on_handshake_done)
            qc->cb.on_handshake_done(qc->cb.app, qc);
    }
}

void handle_stream(QuicConn *qc, const QuicFrame *f)
{
    QuicStream *st = stream_get(qc, f->stream.id, true);
    if (!st)
        return;
    uint64_t want = st->rx_off;
    if (f->stream.offset > want)
        return; // out-of-order beyond window
    if (f->stream.offset + f->stream.length > want)
    {
        size_t skip = (size_t)(want - f->stream.offset);
        const uint8_t *nd = f->stream.data + skip;
        size_t nl = (size_t)(f->stream.length - skip);
        if (nl > sizeof(st->rx))
            nl = sizeof(st->rx);
        // Deliver in place (we hand the callback the contiguous new bytes directly).
        st->rx_off += nl;
        if (f->stream.fin)
            st->rx_fin = true;
        if (qc->cb.on_stream_data)
            qc->cb.on_stream_data(qc->cb.app, qc, st->id, nd, nl, st->rx_fin);
        return;
    }
    if (f->stream.fin && f->stream.offset + f->stream.length == want && !st->rx_fin)
    {
        st->rx_fin = true;
        if (qc->cb.on_stream_data)
            qc->cb.on_stream_data(qc->cb.app, qc, st->id, nullptr, 0, true);
    }
}

// Process the frames in one decrypted packet. Returns false on a fatal connection error.
bool process_frames(QuicConn *qc, int level, const uint8_t *p, size_t len, bool *ack_eliciting)
{
    size_t off = 0;
    while (off < len)
    {
        if (p[off] == QUIC_FT_PADDING)
        {
            off++;
            continue;
        }
        QuicFrame f;
        size_t n = quic_frame_parse(p + off, len - off, &f);
        if (!n)
        {
            // Undecodable frame: a transport FRAME_ENCODING_ERROR (RFC 9000 sec 20.1). Report it.
            queue_close(qc, QUIC_ERR_FRAME_ENCODING, 0, level);
            return false;
        }
        off += n;

        if (f.type != QUIC_FT_ACK && f.type != QUIC_FT_ACK_ECN && f.type != QUIC_FT_CONNECTION_CLOSE &&
            f.type != QUIC_FT_CONNECTION_CLOSE_APP)
            *ack_eliciting = true;

        switch (f.type)
        {
        case QUIC_FT_CRYPTO:
            handle_crypto(qc, level, &f);
            break;
        case QUIC_FT_ACK:
        case QUIC_FT_ACK_ECN:
            if ((int64_t)f.ack.largest > qc->space[level].largest_acked)
            {
                qc->space[level].largest_acked = (int64_t)f.ack.largest;
                // Forward progress: reset the PTO backoff and re-evaluate the timer (RFC 9002 sec 6.2).
                qc->pto_count = 0;
                qc->pto_armed = false;
            }
            break;
        case QUIC_FT_CONNECTION_CLOSE:
        case QUIC_FT_CONNECTION_CLOSE_APP:
            qc->draining = true;
            qc->closed = true;
            break;
        case QUIC_FT_HANDSHAKE_DONE:
            break; // server-only frame; ignore if a peer sends it
        case QUIC_FT_MAX_DATA:
        case QUIC_FT_PING:
            break; // no per-frame state to keep for a minimal server
        default:
            if (f.type >= QUIC_FT_STREAM && f.type <= QUIC_FT_STREAM + 7)
                handle_stream(qc, &f);
            break;
        }
    }
    return true;
}

// Decrypt and process one packet at datagram offset; returns bytes consumed (0 to stop the datagram).
size_t recv_packet(QuicConn *qc, const uint8_t *dg, size_t len)
{
    if (len < 1)
        return 0;
    bool is_long = quic_is_long_header(dg[0]);

    int level;
    size_t pn_offset;
    size_t pkt_len; // total on-wire bytes of this packet
    uint64_t payload_length;

    if (is_long)
    {
        QuicLongHeader h;
        if (!quic_parse_long_header(dg, len, &h))
            return 0;
        if (h.version == 0 || h.version != QUIC_VERSION_1)
            return 0; // Version Negotiation is a client concern; unknown versions are dropped
        if (h.type == QUIC_LP_INITIAL)
            level = QUIC_ENC_INITIAL;
        else if (h.type == QUIC_LP_HANDSHAKE)
            level = QUIC_ENC_HANDSHAKE;
        else
            return 0; // 0-RTT / Retry not supported

        size_t off = h.hdr_len;
        if (level == QUIC_ENC_INITIAL)
        {
            uint64_t tok_len = 0;
            size_t c = 0;
            if (!quic_varint_decode(dg + off, len - off, &tok_len, &c))
                return 0;
            off += c + (size_t)tok_len; // skip the token
            if (off > len)
                return 0;
        }
        size_t c = 0;
        if (!quic_varint_decode(dg + off, len - off, &payload_length, &c))
            return 0;
        off += c;
        pn_offset = off;
        pkt_len = pn_offset + (size_t)payload_length;
        if (pkt_len > len)
            return 0;
    }
    else
    {
        // Short header: DCID length is our locally chosen scid_len; the packet runs to datagram end.
        level = QUIC_ENC_APP;
        pn_offset = 1 + qc->scid_len;
        if (pn_offset >= len)
            return 0;
        payload_length = len - pn_offset;
        pkt_len = len;
    }

    const QuicPacketKeys *keys = open_keys(qc, level);
    if (!keys)
        return 0; // keys for this level are not available yet

    // Unprotect on a copy so a failed decrypt does not corrupt following coalesced packets. The
    // engine runs sequentially on one task, so the scratch is a shared static (not reentrant).
    static uint8_t work[DETWS_QUIC_MAX_DATAGRAM];
    static uint8_t plain[DETWS_QUIC_MAX_DATAGRAM];
    if (pkt_len > sizeof(work))
        return 0;
    memcpy(work, dg, pkt_len);
    uint64_t pn = 0;
    size_t pt = quic_packet_unprotect(work, pn_offset, (size_t)payload_length, qc->space[level].largest_rx, keys,
                                      is_long, plain, &pn);
    if (pt == (size_t)-1)
        return is_long ? pkt_len : 0; // drop this packet, keep parsing later coalesced ones

    if (!qc->space[level].have_rx || pn > qc->space[level].largest_rx)
        qc->space[level].largest_rx = pn;
    qc->space[level].have_rx = true;

    // Receiving a Handshake packet validates the client's address (lifts anti-amplification).
    if (level == QUIC_ENC_HANDSHAKE)
    {
        qc->address_validated = true;
        qc->space[QUIC_ENC_INITIAL].discarded = true;
    }

    bool ack_eliciting = false;
    process_frames(qc, level, plain, pt, &ack_eliciting);
    if (ack_eliciting)
        qc->space[level].ack_eliciting_rx = true;

    return pkt_len;
}
} // namespace

bool quic_conn_recv(QuicConn *qc, const uint8_t *datagram, size_t len)
{
    if (qc->closed)
        return false;
    qc->recv_bytes += len;
    size_t off = 0;
    bool any = false;
    while (off < len)
    {
        size_t n = recv_packet(qc, datagram + off, len - off);
        if (!n)
            break;
        any = true;
        off += n;
    }
    return any;
}

// --- Sending ---------------------------------------------------------------------------------
namespace
{
// Build the frame payload for one encryption level into buf; returns its length (0 = nothing to send).
// @p ae is set true if the payload carries an ack-eliciting frame (CRYPTO / STREAM / HANDSHAKE_DONE),
// which arms loss recovery for this space.
size_t build_frames(QuicConn *qc, int level, uint8_t *buf, size_t cap, bool *ae)
{
    QuicPnSpace *s = &qc->space[level];
    size_t p = 0;
    *ae = false;

    // While closing, the only frame we send is the transport CONNECTION_CLOSE (RFC 9000 sec 10.2.3).
    // It is not ack-eliciting (*ae stays false), so no PTO is armed for it. quic_conn_send() invokes
    // this for a single level when a close is queued, so it is emitted exactly once.
    if (qc->close_queued && !qc->close_sent)
        return quic_build_connection_close(buf, cap, qc->close_error, qc->close_frame_type, nullptr, 0);

    // ACK first, if we owe one.
    if (s->ack_eliciting_rx && s->have_rx)
    {
        size_t n = quic_build_ack(buf + p, cap - p, s->largest_rx, 0, s->largest_rx);
        if (n)
        {
            p += n;
            s->ack_eliciting_rx = false;
        }
    }

    // CRYPTO flight for this level (Initial = ServerHello, Handshake = EE..Finished).
    if (level == QUIC_ENC_INITIAL || level == QUIC_ENC_HANDSHAKE)
    {
        size_t flen = 0;
        const uint8_t *flight = quic_tls_flight(&qc->tls, level, &flen);
        if (flight && s->crypto_tx_off < flen)
        {
            size_t remain = flen - (size_t)s->crypto_tx_off;
            // Leave room for the CRYPTO frame header (type + offset + length varints, <= 1+8+8).
            size_t room = (cap - p > 20) ? (cap - p - 20) : 0;
            size_t take = remain < room ? remain : room;
            if (take)
            {
                size_t n = quic_build_crypto(buf + p, cap - p, s->crypto_tx_off, flight + s->crypto_tx_off, take);
                if (n)
                {
                    p += n;
                    s->crypto_tx_off += take;
                    *ae = true;
                }
            }
        }
    }

    // 1-RTT extras: HANDSHAKE_DONE and stream data.
    if (level == QUIC_ENC_APP)
    {
        if (qc->handshake_done_queued)
        {
            size_t n = quic_build_handshake_done(buf + p, cap - p);
            if (n)
            {
                p += n;
                qc->handshake_done_queued = false;
                qc->handshake_done_sent = true;
                *ae = true;
            }
        }
        for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
        {
            QuicStream *st = &qc->streams[i];
            if (st->id == UINT64_MAX)
                continue;
            bool more = st->tx_sent < st->tx_have;
            bool fin_due = st->tx_fin && !st->tx_fin_sent && st->tx_sent == st->tx_have;
            if (!more && !fin_due)
                continue;
            size_t room = (cap - p > 24) ? (cap - p - 24) : 0;
            size_t remain = st->tx_have - st->tx_sent;
            size_t take = remain < room ? remain : room;
            bool fin = st->tx_fin && (st->tx_sent + take == st->tx_have);
            size_t n = quic_build_stream(buf + p, cap - p, st->id, st->tx_off, st->tx + st->tx_sent, take, fin);
            if (n)
            {
                p += n;
                st->tx_off += take;
                st->tx_sent += take;
                if (fin)
                    st->tx_fin_sent = true;
                *ae = true;
            }
        }
    }
    return p;
}

// Long-header packet type for an encryption level.
uint8_t level_lp_type(int level)
{
    return level == QUIC_ENC_INITIAL ? QUIC_LP_INITIAL : QUIC_LP_HANDSHAKE;
}

// Build one protected packet for a level into out; returns its length (0 = nothing to send).
size_t build_packet(QuicConn *qc, int level, uint8_t *out, size_t cap)
{
    QuicPnSpace *s = &qc->space[level];
    if (s->discarded)
        return 0;
    const QuicPacketKeys *keys = seal_keys(qc, level);
    if (!keys)
        return 0;

    uint8_t frames[DETWS_QUIC_MAX_DATAGRAM];
    bool ae = false;
    size_t frame_len = build_frames(qc, level, frames, sizeof(frames), &ae);
    if (frame_len == 0)
        return 0;

    uint64_t pn = s->next_pn;
    uint8_t pn_len = quic_pn_length(pn, s->largest_acked);
    bool is_long = (level != QUIC_ENC_APP);

    // Header protection samples 16 bytes at (packet number + 4), so the packet number and payload
    // together must be at least 4 bytes (RFC 9001 sec 5.4.2). Pad tiny packets (e.g. a lone
    // HANDSHAKE_DONE or ACK) with PADDING frames (zero bytes) to reach that minimum.
    if ((size_t)pn_len + frame_len < 4)
    {
        size_t pad = 4 - pn_len - frame_len;
        memset(frames + frame_len, 0, pad);
        frame_len += pad;
    }

    size_t p = 0;
    if (is_long)
    {
        // Invariant header fields, then the type-specific token (Initial only) + Length + PN.
        size_t hn = quic_build_long_header(out, cap, level_lp_type(level), QUIC_VERSION_1, qc->dcid, qc->dcid_len,
                                           qc->scid, qc->scid_len, pn_len);
        if (!hn)
            return 0;
        p = hn;
        if (level == QUIC_ENC_INITIAL)
        {
            size_t n = quic_varint_encode(out + p, cap - p, 0); // empty token
            if (!n)
                return 0;
            p += n;
        }
        uint64_t length = (uint64_t)pn_len + frame_len + QUIC_AEAD_TAG_LEN;
        size_t n = quic_varint_encode(out + p, cap - p, length);
        if (!n)
            return 0;
        p += n;
    }
    else
    {
        // Short header: 0x40 fixed bit | pn_len-1; then the peer's DCID (no length on the wire).
        if (1 + qc->dcid_len > cap)
            return 0;
        out[0] = (uint8_t)(0x40 | (pn_len - 1));
        memcpy(out + 1, qc->dcid, qc->dcid_len);
        p = 1 + qc->dcid_len;
    }

    size_t pn_offset = p;
    // Bound the whole packet up front (p <= cap here): the header builders checked their own writes
    // but not the packet number + payload + tag that follow, so verify the remainder fits before
    // writing it (avoids a size_t addition wrap in the bounds check, cpp:S3519).
    if (cap - p < (size_t)pn_len + frame_len + QUIC_AEAD_TAG_LEN)
        return 0;
    // Write the (unprotected) truncated packet number.
    for (uint8_t i = 0; i < pn_len; i++)
        out[pn_offset + i] = (uint8_t)(pn >> (8 * (pn_len - 1 - i)));
    p += pn_len;

    memcpy(out + p, frames, frame_len);

    size_t total = quic_packet_protect(out, cap, pn_offset, pn_len, pn, frame_len, keys, is_long);
    if (!total)
        return 0;
    if (ae)
        s->last_ae_pn = (int64_t)pn; // this space now has ack-eliciting data outstanding
    s->next_pn++;
    return total;
}
} // namespace

size_t quic_conn_send(QuicConn *qc, uint8_t *out, size_t cap)
{
    if (qc->closed && !qc->draining)
        return 0;
    // Anti-amplification (RFC 9000 sec 8.1): until the client's address is validated, send nothing
    // once we have already put 3x the received bytes on the wire. Checked BEFORE building the packets
    // so a blocked send never advances packet-number / CRYPTO / stream state (which would desync the
    // flight); a build then discard was the bug. Being at-most one datagram approximate here is fine.
    if (!qc->address_validated && qc->sent_bytes >= 3 * qc->recv_bytes)
        return 0;
    if (cap > DETWS_QUIC_MAX_DATAGRAM)
        cap = DETWS_QUIC_MAX_DATAGRAM;

    // A queued transport CONNECTION_CLOSE is sent once - at the highest encryption level we still hold
    // keys for (so the peer can decrypt it) - and then the connection is closed. It replaces the normal
    // frame build (a closing endpoint sends nothing else) and is bounded by the amplification limit above.
    if (qc->close_queued && !qc->close_sent)
    {
        // Send at the level the error was seen on (the peer holds those keys); if that space has since
        // been discarded, fall back to the highest level we still hold keys for.
        int level = qc->close_level;
        if (level < QUIC_ENC_INITIAL || level > QUIC_ENC_APP || qc->space[level].discarded || !seal_keys(qc, level))
        {
            level = QUIC_ENC_INITIAL;
            for (int l = QUIC_ENC_APP; l >= QUIC_ENC_INITIAL; l--)
                if (!qc->space[l].discarded && seal_keys(qc, l))
                {
                    level = l;
                    break;
                }
        }
        size_t n = build_packet(qc, level, out, cap);
        if (n)
        {
            qc->close_sent = true;
            qc->closed = true;
            qc->sent_bytes += n;
        }
        return n;
    }

    size_t dg = 0;
    // Coalesce Initial, then Handshake, then 1-RTT into one datagram.
    for (int level = QUIC_ENC_INITIAL; level <= QUIC_ENC_APP; level++)
    {
        size_t n = build_packet(qc, level, out + dg, cap - dg);
        dg += n;
    }
    if (dg == 0)
        return 0;
    qc->sent_bytes += dg;
    return dg;
}

namespace
{
// PTO period with exponential backoff, capped so the shift cannot overflow (RFC 9002 sec 6.2.1).
uint32_t pto_period(uint8_t count)
{
    uint32_t p = DETWS_QUIC_PTO_MS;
    for (uint8_t i = 0; i < count && p < (1u << 30); i++)
        p <<= 1;
    return p;
}
// A space has unacknowledged ack-eliciting data outstanding: it sent an ack-eliciting packet the peer
// has not yet acknowledged, and its keys are still live.
bool space_outstanding(const QuicPnSpace *s)
{
    return !s->discarded && s->last_ae_pn >= 0 && s->largest_acked < s->last_ae_pn;
}
} // namespace

void quic_conn_on_timeout(QuicConn *qc, uint32_t now_ms)
{
    if (qc->closed)
        return;
    // Loss recovery (RFC 9002): retransmission is driven by a Probe Timeout, because a lost server
    // packet is not re-triggered by the peer (a duplicate ClientHello re-delivers no CRYPTO; a lost
    // 1-RTT response is never re-requested). Anything the peer has not acknowledged in a live space -
    // the handshake CRYPTO flight, HANDSHAKE_DONE, or the 1-RTT response - is outstanding.
    bool outstanding = space_outstanding(&qc->space[QUIC_ENC_INITIAL]) ||
                       space_outstanding(&qc->space[QUIC_ENC_HANDSHAKE]) || space_outstanding(&qc->space[QUIC_ENC_APP]);
    if (!outstanding)
    {
        qc->pto_armed = false; // everything acknowledged: nothing to retransmit
        qc->pto_count = 0;
        return;
    }
    if (!qc->pto_armed)
    {
        qc->pto_armed = true;
        qc->pto_deadline_ms = now_ms + pto_period(qc->pto_count);
        return;
    }
    if ((int32_t)(now_ms - qc->pto_deadline_ms) < 0)
        return; // not yet (wrap-safe compare)

    // PTO fired: mark the unacknowledged data in each outstanding space for retransmission so the next
    // quic_conn_send() re-sends it, then back the timer off.
    for (int level = QUIC_ENC_INITIAL; level <= QUIC_ENC_HANDSHAKE; level++)
        if (space_outstanding(&qc->space[level]))
            qc->space[level].crypto_tx_off = 0; // re-send the CRYPTO flight for this level
    if (space_outstanding(&qc->space[QUIC_ENC_APP]))
    {
        // Re-send 1-RTT data. The peer dedups STREAM data by offset, so rewinding each stream to 0
        // recovers a lost response and is a no-op for data already received.
        if (qc->handshake_done_sent)
        {
            qc->handshake_done_queued = true;
            qc->handshake_done_sent = false;
        }
        for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
        {
            QuicStream *st = &qc->streams[i];
            if (st->id == UINT64_MAX)
                continue;
            st->tx_off = 0;
            st->tx_sent = 0;
            st->tx_fin_sent = false;
        }
    }
    if (qc->pto_count < 8)
        qc->pto_count++;
    qc->pto_deadline_ms = now_ms + pto_period(qc->pto_count);
}

size_t quic_conn_stream_send(QuicConn *qc, uint64_t stream_id, const uint8_t *data, size_t len, bool fin)
{
    QuicStream *st = stream_get(qc, stream_id, true);
    if (!st)
        return 0;
    size_t room = sizeof(st->tx) - st->tx_have;
    size_t take = len < room ? len : room;
    memcpy(st->tx + st->tx_have, data, take);
    st->tx_have += take;
    if (fin && take == len)
        st->tx_fin = true;
    return take;
}

void quic_conn_close(QuicConn *qc, uint64_t error_code)
{
    // Application-initiated close: send at the highest level we still hold keys for.
    int level = QUIC_ENC_INITIAL;
    for (int l = QUIC_ENC_APP; l >= QUIC_ENC_INITIAL; l--)
        if (!qc->space[l].discarded && seal_keys(qc, l))
        {
            level = l;
            break;
        }
    queue_close(qc, error_code, 0, level);
}

bool quic_conn_established(const QuicConn *qc)
{
    return qc->tls.state == QTLS_DONE;
}

bool quic_conn_is_closed(const QuicConn *qc)
{
    return qc->closed || qc->draining;
}

#endif // DETWS_ENABLE_HTTP3
