// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mqtt.cpp
 * @brief MQTT 3.1.1 packet codec (host-testable) + the raw-lwIP / mbedTLS
 *        persistent client transport (ESP32 only).
 */

#include "services/mqtt/mqtt.h"

#if DETWS_ENABLE_MQTT

#include <string.h>

// ---------------------------------------------------------------------------
// Pure codec (host-testable)
// ---------------------------------------------------------------------------

// Big-endian 16-bit helpers and a length-prefixed UTF-8 string writer.
static inline void put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFF);
}
static inline uint16_t get_u16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}
// Write a 2-byte length + the bytes; returns total bytes written (2 + len).
static size_t put_field(uint8_t *p, const uint8_t *data, size_t len)
{
    put_u16(p, (uint16_t)len);
    if (len)
        memcpy(p + 2, data, len);
    return 2 + len;
}
static inline size_t put_str(uint8_t *p, const char *s)
{
    return put_field(p, (const uint8_t *)s, s ? strlen(s) : 0);
}

size_t mqtt_encode_remlen(uint8_t *out, uint32_t len)
{
    if (len > 268435455u) // 4 * 7 bits
        return 0;
    size_t n = 0;
    do
    {
        uint8_t byte = (uint8_t)(len % 128);
        len /= 128;
        if (len > 0)
            byte |= 0x80;
        out[n++] = byte;
    } while (len > 0);
    return n;
}

bool mqtt_decode_remlen(const uint8_t *buf, size_t avail, uint32_t *value, size_t *used)
{
    uint32_t v = 0;
    uint32_t mult = 1;
    size_t i = 0;
    for (; i < 4; i++)
    {
        if (i >= avail)
            return false; // incomplete
        uint8_t b = buf[i];
        v += (uint32_t)(b & 0x7F) * mult;
        if ((b & 0x80) == 0)
        {
            *value = v;
            *used = i + 1;
            return true;
        }
        mult *= 128;
    }
    return false; // malformed (5th continuation byte)
}

// Assemble a packet: fixed header byte0 + remaining-length, then a pre-built
// variable-header+payload body of vlen bytes already placed at out + (header).
// Returns total length or 0 if it will not fit cap. The body must be written by
// the caller into a scratch first; here we shift it after the header is known.
// To avoid a second buffer we build the body at a fixed offset (max 4-byte
// remlen) and memmove it tight - simpler: build body in `body`, then compose.
static size_t compose(uint8_t *out, size_t cap, uint8_t byte0, const uint8_t *body, size_t blen)
{
    uint8_t rl[4];
    size_t rln = mqtt_encode_remlen(rl, (uint32_t)blen);
    if (rln == 0)
        return 0;
    size_t total = 1 + rln + blen;
    if (total > cap)
        return 0;
    out[0] = byte0;
    memcpy(out + 1, rl, rln);
    if (blen)
        memcpy(out + 1 + rln, body, blen);
    return total;
}

size_t mqtt_build_connect(uint8_t *out, size_t cap, const MqttConnectOpts *opts)
{
    if (!out || !opts || !opts->client_id)
        return 0;
    uint8_t body[DETWS_MQTT_BUF_SIZE];
    size_t n = 0;
    // Variable header: protocol name + level + flags + keep-alive.
    n += put_str(body + n, "MQTT");
    body[n++] = 0x04; // protocol level 4 (MQTT 3.1.1)

    uint8_t flags = 0;
    if (opts->clean_session)
        flags |= 0x02;
    if (opts->will_topic)
    {
        flags |= 0x04;                                    // will flag
        flags |= (uint8_t)((opts->will_qos & 0x03) << 3); // will QoS
        if (opts->will_retain)
            flags |= 0x20;
    }
    if (opts->user)
        flags |= 0x80;
    if (opts->pass)
        flags |= 0x40;
    body[n++] = flags;
    put_u16(body + n, opts->keepalive_s);
    n += 2;

    // Payload: client id, [will topic, will msg], [user], [pass]. Bounds-check
    // each field against the body scratch as we go.
    size_t need = 2 + strlen(opts->client_id);
    if (opts->will_topic)
        need += 2 + strlen(opts->will_topic) + 2 + opts->will_len;
    if (opts->user)
        need += 2 + strlen(opts->user);
    if (opts->pass)
        need += 2 + strlen(opts->pass);
    if (n + need > sizeof(body))
        return 0;

    n += put_str(body + n, opts->client_id);
    if (opts->will_topic)
    {
        n += put_str(body + n, opts->will_topic);
        n += put_field(body + n, opts->will_msg, opts->will_len);
    }
    if (opts->user)
        n += put_str(body + n, opts->user);
    if (opts->pass)
        n += put_str(body + n, opts->pass);

    return compose(out, cap, (uint8_t)(MQTT_CONNECT << 4), body, n);
}

size_t mqtt_build_publish(uint8_t *out, size_t cap, const char *topic, const uint8_t *payload, size_t payload_len,
                          uint8_t qos, uint16_t packet_id, bool retain, bool dup)
{
    if (!out || !topic || qos > 2)
        return 0;
    size_t tlen = strlen(topic);
    size_t blen = 2 + tlen + (qos > 0 ? 2 : 0) + payload_len;
    uint8_t body[DETWS_MQTT_BUF_SIZE];
    if (blen > sizeof(body))
        return 0;
    size_t n = 0;
    n += put_field(body + n, (const uint8_t *)topic, tlen);
    if (qos > 0)
    {
        put_u16(body + n, packet_id);
        n += 2;
    }
    if (payload_len)
        memcpy(body + n, payload, payload_len);
    n += payload_len;

    uint8_t f = (uint8_t)((qos & 0x03) << 1);
    if (retain)
        f |= 0x01;
    if (dup)
        f |= 0x08;
    return compose(out, cap, (uint8_t)((MQTT_PUBLISH << 4) | f), body, n);
}

size_t mqtt_build_subscribe(uint8_t *out, size_t cap, uint16_t packet_id, const char *topic, uint8_t qos)
{
    if (!out || !topic || qos > 2)
        return 0;
    size_t tlen = strlen(topic);
    uint8_t body[DETWS_MQTT_BUF_SIZE];
    size_t blen = 2 + 2 + tlen + 1;
    if (blen > sizeof(body))
        return 0;
    size_t n = 0;
    put_u16(body + n, packet_id);
    n += 2;
    n += put_field(body + n, (const uint8_t *)topic, tlen);
    body[n++] = (uint8_t)(qos & 0x03);
    return compose(out, cap, (uint8_t)((MQTT_SUBSCRIBE << 4) | 0x02), body, n); // SUBSCRIBE flags = 0010
}

size_t mqtt_build_unsubscribe(uint8_t *out, size_t cap, uint16_t packet_id, const char *topic)
{
    if (!out || !topic)
        return 0;
    size_t tlen = strlen(topic);
    uint8_t body[DETWS_MQTT_BUF_SIZE];
    size_t blen = 2 + 2 + tlen;
    if (blen > sizeof(body))
        return 0;
    size_t n = 0;
    put_u16(body + n, packet_id);
    n += 2;
    n += put_field(body + n, (const uint8_t *)topic, tlen);
    return compose(out, cap, (uint8_t)((MQTT_UNSUBSCRIBE << 4) | 0x02), body, n); // UNSUBSCRIBE flags = 0010
}

size_t mqtt_build_ack(uint8_t *out, size_t cap, uint8_t type, uint16_t packet_id)
{
    if (!out || cap < 4)
        return 0;
    uint8_t f = (type == MQTT_PUBREL) ? 0x02 : 0x00; // PUBREL requires flags 0010
    out[0] = (uint8_t)((type << 4) | f);
    out[1] = 0x02;
    put_u16(out + 2, packet_id);
    return 4;
}

size_t mqtt_build_pingreq(uint8_t *out, size_t cap)
{
    if (!out || cap < 2)
        return 0;
    out[0] = (uint8_t)(MQTT_PINGREQ << 4);
    out[1] = 0x00;
    return 2;
}

size_t mqtt_build_disconnect(uint8_t *out, size_t cap)
{
    if (!out || cap < 2)
        return 0;
    out[0] = (uint8_t)(MQTT_DISCONNECT << 4);
    out[1] = 0x00;
    return 2;
}

bool mqtt_parse_fixed_header(const uint8_t *buf, size_t avail, uint8_t *type, uint8_t *flags, uint32_t *remaining_len,
                             size_t *header_len)
{
    if (avail < 2)
        return false;
    uint32_t rl;
    size_t used;
    if (!mqtt_decode_remlen(buf + 1, avail - 1, &rl, &used))
        return false;
    *type = (uint8_t)(buf[0] >> 4);
    *flags = (uint8_t)(buf[0] & 0x0F);
    *remaining_len = rl;
    *header_len = 1 + used;
    return true;
}

bool mqtt_parse_publish(const uint8_t *buf, uint32_t remaining_len, uint8_t flags, char *topic_out, size_t topic_cap,
                        size_t *topic_len, const uint8_t **payload, size_t *payload_len, uint16_t *packet_id)
{
    if (!buf || remaining_len < 2)
        return false;
    uint16_t tlen = get_u16(buf);
    size_t off = 2;
    if ((uint32_t)off + tlen > remaining_len)
        return false;
    if ((size_t)tlen + 1 > topic_cap)
        return false; // topic + NUL must fit
    memcpy(topic_out, buf + off, tlen);
    topic_out[tlen] = '\0';
    *topic_len = tlen;
    off += tlen;

    uint8_t qos = (uint8_t)((flags >> 1) & 0x03);
    if (qos == 3)
        return false; // MQTT-3.3.1-4: a PUBLISH MUST NOT have both QoS bits set (malformed)
    *packet_id = 0;
    if (qos > 0)
    {
        if ((uint32_t)off + 2 > remaining_len)
            return false;
        *packet_id = get_u16(buf + off);
        off += 2;
    }
    *payload = buf + off;
    *payload_len = remaining_len - off;
    return true;
}

uint16_t mqtt_parse_ack(const uint8_t *buf, uint32_t remaining_len)
{
    if (!buf || remaining_len < 2)
        return 0;
    return get_u16(buf);
}

int mqtt_parse_connack(const uint8_t *buf, uint32_t remaining_len, bool *session_present)
{
    if (!buf || remaining_len < 2)
        return -1;
    if (session_present)
        *session_present = (buf[0] & 0x01) != 0;
    return buf[1];
}

bool mqtt_parse_suback(const uint8_t *buf, uint32_t remaining_len, uint16_t *packet_id, uint8_t *return_code)
{
    if (!buf || remaining_len < 3)
        return false;
    if (packet_id)
        *packet_id = get_u16(buf);
    if (return_code)
        *return_code = buf[2];
    return true;
}

// ---------------------------------------------------------------------------
// Transport (ESP32 only): persistent raw-lwIP TCP client + QoS state machine,
// with mqtts:// over a persistent client TLS session (det_tls csess).
// ---------------------------------------------------------------------------
#if defined(ARDUINO)

#include "network_drivers/transport/det_client.h" // shared outbound TCP client (L4)
#include <Arduino.h>

#if DETWS_ENABLE_MQTT_TLS
#include "network_drivers/tls/det_tls.h" // persistent client TLS session (csess)
#include <mbedtls/ssl.h>                 // MBEDTLS_ERR_SSL_WANT_* for the BIO callbacks
#endif

#ifdef DETWS_MQTT_DEBUG
#define MQ_DBG(...) printf(__VA_ARGS__)
#else
#define MQ_DBG(...) ((void)0)
#endif

// --- connection state (one broker at a time; all static, no heap) ---
static MqttMessageCb s_cb;
static int s_cid = -1;         // outbound connection id (det_client pool)
static volatile bool s_closed; // peer closed / error (set when the pump sees it)

// Inbound plaintext byte ring (consumer = process_rx). It is fed by a pump in
// process_rx: for plain TCP from det_client_read, for MQTTS from the TLS session
// (det_tls_csess_read), whose BIO in turn reads ciphertext from det_client.
static uint8_t s_rx[DETWS_MQTT_BUF_SIZE];
static volatile size_t s_rx_head;
static volatile size_t s_rx_tail;

static uint8_t s_pkt[DETWS_MQTT_BUF_SIZE]; // contiguous scratch a packet is copied into to parse
static uint8_t s_tx[DETWS_MQTT_BUF_SIZE];  // outgoing packet scratch
static bool s_use_tls;                     // mqtts:// mode

static bool s_mqtt_up;
static uint16_t s_keepalive_s;
static uint32_t s_last_tx_ms;
static bool s_ping_pending;
static uint32_t s_ping_sent_ms;
static uint16_t s_next_pid = 1;
static int s_connack_code; // set by process_rx during the connect handshake

// Outbound QoS 1/2 in-flight (held for DUP retransmit until acknowledged).
struct MqttInflight
{
    uint16_t pid;
    uint8_t state; // 0 free, 1 awaiting PUBACK(qos1)/PUBREC(qos2), 2 awaiting PUBCOMP(qos2)
    uint32_t sent_ms;
    uint16_t len;
    uint8_t pkt[DETWS_MQTT_INFLIGHT_BUF];
};
static MqttInflight s_inflight[DETWS_MQTT_MAX_INFLIGHT];
// Inbound QoS 2 packet ids that have been PUBREC'd and await PUBREL (0 = empty).
static uint16_t s_rx_qos2[DETWS_MQTT_RX_QOS2_SLOTS];

static uint16_t next_pid()
{
    uint16_t p = s_next_pid++;
    if (s_next_pid == 0)
        s_next_pid = 1;
    return p;
}

// --- ring helpers (single-producer/single-consumer) ---
static inline size_t ring_avail()
{
    return (s_rx_head + sizeof(s_rx) - s_rx_tail) % sizeof(s_rx);
}
static inline uint8_t ring_peek(size_t i)
{
    return s_rx[(s_rx_tail + i) % sizeof(s_rx)];
}
static void ring_copy(uint8_t *dst, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = s_rx[(s_rx_tail + i) % sizeof(s_rx)];
}
static inline void ring_advance(size_t n)
{
    s_rx_tail = (s_rx_tail + n) % sizeof(s_rx);
}

// --- transport over the shared outbound client (det_client) ---

// Send raw plaintext bytes to the broker.
static bool mq_tx_plain(const uint8_t *data, size_t len)
{
    return det_client_send(s_cid, data, len);
}

// Drain plaintext wire bytes from the client into the s_rx ring (plain TCP).
// det_client's own ring applies lossless backpressure to the peer when s_rx is
// full and we stop draining.
static void mq_pump_plain()
{
    uint8_t tmp[256];
    for (;;)
    {
        size_t freey = (sizeof(s_rx) - 1) - ring_avail();
        if (freey == 0)
            break;
        size_t want = freey < sizeof(tmp) ? freey : sizeof(tmp);
        size_t n = det_client_read(s_cid, tmp, want);
        if (n == 0)
        {
            if (det_client_is_closed(s_cid))
                s_closed = true;
            break;
        }
        for (size_t i = 0; i < n; i++)
        {
            s_rx[s_rx_head] = tmp[i];
            s_rx_head = (s_rx_head + 1) % sizeof(s_rx);
        }
    }
}

#if DETWS_ENABLE_MQTT_TLS
// TLS BIO over the shared client: write ciphertext through the pool, read
// ciphertext by draining the client's wire ring.
static int mq_tls_send(void *ctx, const unsigned char *buf, size_t len)
{
    (void)ctx;
    size_t cap = len > 0xFFFF ? 0xFFFF : len;
    return det_client_send(s_cid, buf, cap) ? (int)cap : MBEDTLS_ERR_SSL_WANT_WRITE;
}
static int mq_tls_recv(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    size_t n = det_client_read(s_cid, buf, len);
    if (n == 0)
        return det_client_is_closed(s_cid) ? 0 : MBEDTLS_ERR_SSL_WANT_READ;
    return (int)n;
}
// Drain decrypted plaintext from the TLS session into the s_rx ring (main loop).
static void mq_pump_tls()
{
    uint8_t tmp[256];
    for (;;)
    {
        size_t freey = (sizeof(s_rx) - 1) - ring_avail();
        if (freey == 0)
            break;
        size_t want = freey < sizeof(tmp) ? freey : sizeof(tmp);
        int n = det_tls_csess_read(tmp, want);
        if (n <= 0)
        {
            if (n < 0)
                s_closed = true;
            break;
        }
        for (int i = 0; i < n; i++)
        {
            s_rx[s_rx_head] = tmp[i];
            s_rx_head = (s_rx_head + 1) % sizeof(s_rx);
        }
    }
}
#endif // DETWS_ENABLE_MQTT_TLS

// Send a complete MQTT packet (plaintext or TLS-encrypted per the mode).
static bool mq_tx(const uint8_t *data, size_t len)
{
    bool ok;
#if DETWS_ENABLE_MQTT_TLS
    if (s_use_tls)
        ok = det_tls_csess_write(data, len) == (int)len;
    else
#endif
        ok = mq_tx_plain(data, len);
    if (ok)
        s_last_tx_ms = millis();
    return ok;
}

static void mq_close()
{
#if DETWS_ENABLE_MQTT_TLS
    if (s_use_tls)
        det_tls_csess_end();
#endif
    if (s_cid >= 0)
        det_client_close(s_cid);
    s_cid = -1;
    s_mqtt_up = false;
}

static int inflight_find(uint16_t pid)
{
    for (int i = 0; i < DETWS_MQTT_MAX_INFLIGHT; i++)
        if (s_inflight[i].state != 0 && s_inflight[i].pid == pid)
            return i;
    return -1;
}

static void rxqos2_add(uint16_t pid)
{
    for (int i = 0; i < DETWS_MQTT_RX_QOS2_SLOTS; i++)
        if (s_rx_qos2[i] == 0)
        {
            s_rx_qos2[i] = pid;
            return;
        }
}
static bool rxqos2_has(uint16_t pid)
{
    for (int i = 0; i < DETWS_MQTT_RX_QOS2_SLOTS; i++)
        if (s_rx_qos2[i] == pid)
            return true;
    return false;
}
static void rxqos2_del(uint16_t pid)
{
    for (int i = 0; i < DETWS_MQTT_RX_QOS2_SLOTS; i++)
        if (s_rx_qos2[i] == pid)
            s_rx_qos2[i] = 0;
}

// Handle one fully-received packet sitting in s_pkt (length plen).
static void handle_packet(uint8_t type, uint8_t flags, const uint8_t *body, uint32_t rl)
{
    switch (type)
    {
    case MQTT_CONNACK:
        s_connack_code = mqtt_parse_connack(body, rl, nullptr);
        if (s_connack_code == 0)
            s_mqtt_up = true;
        MQ_DBG("[mqtt] CONNACK code=%d\n", s_connack_code);
        break;
    case MQTT_PUBLISH: {
        char topic[DETWS_MQTT_MAX_TOPIC];
        size_t tlen, plen;
        const uint8_t *payload;
        uint16_t pid;
        if (!mqtt_parse_publish(body, rl, flags, topic, sizeof(topic), &tlen, &payload, &plen, &pid))
        {
            mq_close(); // MQTT-4.8.0-1: a malformed PUBLISH (incl. QoS=3) MUST close the connection
            break;
        }
        uint8_t qos = (uint8_t)((flags >> 1) & 0x03);
        if (qos < 2)
        {
            if (s_cb)
                s_cb(topic, payload, plen);
            if (qos == 1)
            {
                size_t n = mqtt_build_ack(s_tx, sizeof(s_tx), MQTT_PUBACK, pid);
                mq_tx(s_tx, n);
            }
        }
        else // QoS 2: dispatch once, dedup by id until PUBREL completes
        {
            if (!rxqos2_has(pid))
            {
                if (s_cb)
                    s_cb(topic, payload, plen);
                rxqos2_add(pid);
            }
            size_t n = mqtt_build_ack(s_tx, sizeof(s_tx), MQTT_PUBREC, pid);
            mq_tx(s_tx, n);
        }
        break;
    }
    case MQTT_PUBACK:  // our QoS 1 publish acknowledged
    case MQTT_PUBCOMP: // our QoS 2 publish completed
    {
        int s = inflight_find(mqtt_parse_ack(body, rl));
        if (s >= 0)
            s_inflight[s].state = 0;
        break;
    }
    case MQTT_PUBREC: // our QoS 2 publish: reply PUBREL, await PUBCOMP
    {
        uint16_t pid = mqtt_parse_ack(body, rl);
        int s = inflight_find(pid);
        if (s >= 0)
        {
            s_inflight[s].state = 2;
            s_inflight[s].sent_ms = millis();
        }
        size_t n = mqtt_build_ack(s_tx, sizeof(s_tx), MQTT_PUBREL, pid);
        mq_tx(s_tx, n);
        break;
    }
    case MQTT_PUBREL: // broker releasing an inbound QoS 2 message: reply PUBCOMP
    {
        uint16_t pid = mqtt_parse_ack(body, rl);
        rxqos2_del(pid);
        size_t n = mqtt_build_ack(s_tx, sizeof(s_tx), MQTT_PUBCOMP, pid);
        mq_tx(s_tx, n);
        break;
    }
    case MQTT_PINGRESP:
        s_ping_pending = false;
        break;
    case MQTT_SUBACK:
    case MQTT_UNSUBACK:
    default:
        break; // acknowledgements we do not need to act on
    }
}

// Drain complete packets from the rx ring (copies each into s_pkt to parse).
static void process_rx()
{
#if DETWS_ENABLE_MQTT_TLS
    if (s_use_tls)
        mq_pump_tls(); // decrypt ciphertext into the plaintext ring first
    else
#endif
        mq_pump_plain(); // drain plaintext wire bytes into the ring
    for (;;)
    {
        size_t avail = ring_avail();
        if (avail < 2)
            return;
        // Peek the fixed header (byte0 + 1-4 remlen bytes) without advancing.
        uint8_t hdr[5];
        size_t hn = avail < 5 ? avail : 5;
        for (size_t i = 0; i < hn; i++)
            hdr[i] = ring_peek(i);
        uint8_t type, flags;
        uint32_t rl;
        size_t hl;
        if (!mqtt_parse_fixed_header(hdr, hn, &type, &flags, &rl, &hl))
            return; // incomplete header
        size_t total = hl + rl;
        if (avail < total)
            return; // packet not fully arrived yet
        if (total > sizeof(s_pkt))
        {
            ring_advance(total); // oversized: drop it
            continue;
        }
        ring_copy(s_pkt, total);
        ring_advance(total);
        handle_packet(type, flags, s_pkt + hl, rl);
    }
}

void mqtt_on_message(MqttMessageCb cb)
{
    s_cb = cb;
}

bool mqtt_connect(const char *host, uint16_t port, bool use_tls, const MqttConnectOpts *opts)
{
    if (!host || !opts)
        return false;
#if !DETWS_ENABLE_MQTT_TLS
    if (use_tls)
        return false; // built without MQTTS support
#endif

    // Reset all session state.
    memset(s_inflight, 0, sizeof(s_inflight));
    memset(s_rx_qos2, 0, sizeof(s_rx_qos2));
    s_rx_head = s_rx_tail = 0;
    s_closed = s_mqtt_up = s_ping_pending = false;
    s_connack_code = -1;
    s_keepalive_s = opts->keepalive_s;
    s_use_tls = use_tls;

    uint32_t deadline = millis() + 8000;

    // Open the TCP connection (DNS + connect) via the shared client transport.
    s_cid = det_client_open(host, port, 8000);
    if (s_cid < 0)
        return false;

#if DETWS_ENABLE_MQTT_TLS
    if (s_use_tls)
    {
        if (!det_tls_csess_begin(host, mq_tls_send, mq_tls_recv))
        {
            mq_close();
            return false;
        }
        int h;
        while ((h = det_tls_csess_handshake()) == 0 && !s_closed && (int32_t)(deadline - millis()) > 0)
            delay(5);
        if (h != 1)
        {
            MQ_DBG("[mqtt] TLS handshake failed (%d)\n", h);
            mq_close();
            return false;
        }
    }
#endif

    size_t n = mqtt_build_connect(s_tx, sizeof(s_tx), opts);
    if (n == 0 || !mq_tx(s_tx, n))
    {
        mq_close();
        return false;
    }

    // Wait for CONNACK.
    while (!s_mqtt_up && s_connack_code < 0 && !s_closed && (int32_t)(deadline - millis()) > 0)
    {
        process_rx();
        delay(5);
    }
    if (!s_mqtt_up)
    {
        mq_close();
        return false;
    }
    s_last_tx_ms = millis();
    return true;
}

bool mqtt_publish(const char *topic, const uint8_t *payload, size_t len, uint8_t qos, bool retain)
{
    if (!s_mqtt_up || qos > 2)
        return false;
    if (qos == 0)
    {
        size_t n = mqtt_build_publish(s_tx, sizeof(s_tx), topic, payload, len, 0, 0, retain, false);
        return n && mq_tx(s_tx, n);
    }
    // QoS 1/2: take an in-flight slot, store the serialized packet for retransmit.
    int slot = inflight_find(0);
    if (slot < 0)
        for (int i = 0; i < DETWS_MQTT_MAX_INFLIGHT; i++)
            if (s_inflight[i].state == 0)
            {
                slot = i;
                break;
            }
    if (slot < 0)
        return false; // in-flight window full
    uint16_t pid = next_pid();
    size_t n = mqtt_build_publish(s_inflight[slot].pkt, sizeof(s_inflight[slot].pkt), topic, payload, len, qos, pid,
                                  retain, false);
    if (n == 0)
        return false; // too large for an in-flight slot
    s_inflight[slot].pid = pid;
    s_inflight[slot].state = 1;
    s_inflight[slot].len = (uint16_t)n;
    s_inflight[slot].sent_ms = millis();
    return mq_tx(s_inflight[slot].pkt, n);
}

bool mqtt_subscribe(const char *topic, uint8_t qos)
{
    if (!s_mqtt_up)
        return false;
    size_t n = mqtt_build_subscribe(s_tx, sizeof(s_tx), next_pid(), topic, qos);
    return n && mq_tx(s_tx, n);
}

bool mqtt_unsubscribe(const char *topic)
{
    if (!s_mqtt_up)
        return false;
    size_t n = mqtt_build_unsubscribe(s_tx, sizeof(s_tx), next_pid(), topic);
    return n && mq_tx(s_tx, n);
}

bool mqtt_loop()
{
    if (!s_mqtt_up)
        return false;
    process_rx();
    if (s_closed)
    {
        mq_close();
        return false;
    }

    uint32_t now = millis();

    // Keep-alive: send PINGREQ when idle; drop the link if no PINGRESP comes back.
    if (s_keepalive_s)
    {
        uint32_t ka = (uint32_t)s_keepalive_s * 1000u;
        if (s_ping_pending && (now - s_ping_sent_ms) > ka)
        {
            mq_close();
            return false;
        }
        if (!s_ping_pending && (now - s_last_tx_ms) >= ka)
        {
            size_t n = mqtt_build_pingreq(s_tx, sizeof(s_tx));
            if (mq_tx(s_tx, n))
            {
                s_ping_pending = true;
                s_ping_sent_ms = now;
            }
        }
    }

    // Retransmit unacked in-flight QoS 1/2 messages.
    for (int i = 0; i < DETWS_MQTT_MAX_INFLIGHT; i++)
    {
        if (s_inflight[i].state == 0)
            continue;
        if ((now - s_inflight[i].sent_ms) < DETWS_MQTT_RETRANSMIT_MS)
            continue;
        if (s_inflight[i].state == 1)
        {
            s_inflight[i].pkt[0] |= 0x08; // set DUP on the stored PUBLISH
            mq_tx(s_inflight[i].pkt, s_inflight[i].len);
        }
        else // state 2: re-send PUBREL
        {
            size_t n = mqtt_build_ack(s_tx, sizeof(s_tx), MQTT_PUBREL, s_inflight[i].pid);
            mq_tx(s_tx, n);
        }
        s_inflight[i].sent_ms = now;
    }
    return true;
}

bool mqtt_connected()
{
    return s_mqtt_up;
}

void mqtt_disconnect()
{
    if (s_cid >= 0 && s_mqtt_up)
    {
        size_t n = mqtt_build_disconnect(s_tx, sizeof(s_tx));
        mq_tx(s_tx, n);
    }
    mq_close();
}

#else // host build: transport is a stub

void mqtt_on_message(MqttMessageCb)
{
}
bool mqtt_connect(const char *, uint16_t, bool, const MqttConnectOpts *)
{
    return false;
}
bool mqtt_publish(const char *, const uint8_t *, size_t, uint8_t, bool)
{
    return false;
}
bool mqtt_subscribe(const char *, uint8_t)
{
    return false;
}
bool mqtt_unsubscribe(const char *)
{
    return false;
}
bool mqtt_loop()
{
    return false;
}
bool mqtt_connected()
{
    return false;
}
void mqtt_disconnect()
{
}

#endif // ARDUINO

#endif // DETWS_ENABLE_MQTT
