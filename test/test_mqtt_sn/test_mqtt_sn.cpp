// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the MQTT-SN v1.2 wire codec (services/mqtt/mqtt_sn): the message
// builders, the Length+MsgType header parser, and the typed payload parsers. Pure
// host tests, with wire-byte assertions checked against the v1.2 spec layout.

#include "services/mqtt/mqtt_sn.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_make_flags()
{
    // DUP, QoS 2, retain, will, clean, short topic name.
    uint8_t f = mqttsn_make_flags(true, 2, true, true, true, MQTTSN_TOPIC_SHORT);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_FLAG_DUP, f & MQTTSN_FLAG_DUP);
    TEST_ASSERT_EQUAL_UINT8(2, (uint8_t)((f & MQTTSN_FLAG_QOS_MASK) >> MQTTSN_FLAG_QOS_SHIFT));
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_FLAG_RETAIN, f & MQTTSN_FLAG_RETAIN);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_FLAG_WILL, f & MQTTSN_FLAG_WILL);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_FLAG_CLEAN, f & MQTTSN_FLAG_CLEAN);
    TEST_ASSERT_EQUAL_UINT8(MQTTSN_TOPIC_SHORT, f & MQTTSN_FLAG_TOPICIDTYPE_MASK);
    // QoS -1 encodes as 0b11.
    TEST_ASSERT_EQUAL_UINT8(3, (uint8_t)((mqttsn_make_flags(false, 3, false, false, false, 0) & MQTTSN_FLAG_QOS_MASK) >>
                                         MQTTSN_FLAG_QOS_SHIFT));
}

// CONNECT bytes: Length, MsgType=0x04, Flags, ProtocolId=0x01, Duration(BE), ClientId.
void test_build_connect_bytes()
{
    uint8_t buf[32];
    uint8_t flags = mqttsn_make_flags(false, 0, false, false, true, MQTTSN_TOPIC_NORMAL); // clean session
    size_t n = mqttsn_build_connect(buf, sizeof(buf), flags, 30, "dev1");
    // total = 1(len) + 1(type) + 1(flags) + 1(protoid) + 2(duration) + 4(clientid) = 10
    TEST_ASSERT_EQUAL_size_t(10, n);
    const uint8_t expect[] = {10, MQTTSN_CONNECT, MQTTSN_FLAG_CLEAN, MQTTSN_PROTOCOL_ID, 0x00, 0x1E, 'd', 'e', 'v',
                              '1'};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_build_publish_bytes()
{
    uint8_t buf[32];
    uint8_t flags = mqttsn_make_flags(false, 1, false, false, false, MQTTSN_TOPIC_NORMAL);
    const uint8_t data[] = {0xDE, 0xAD};
    size_t n = mqttsn_build_publish(buf, sizeof(buf), flags, 0x0007, 0x0001, data, sizeof(data));
    // total = 1+1+1(flags)+2(topic)+2(msgid)+2(data) = 9
    TEST_ASSERT_EQUAL_size_t(9, n);
    const uint8_t expect[] = {9, MQTTSN_PUBLISH, (1 << MQTTSN_FLAG_QOS_SHIFT), 0x00, 0x07, 0x00, 0x01, 0xDE, 0xAD};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// REGISTER round-trips through the header parser + typed parser.
void test_register_round_trip()
{
    uint8_t buf[32];
    size_t n = mqttsn_build_register(buf, sizeof(buf), 0x0000, 0x0042, "sensors/temp");
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    MqttsnHeader h;
    size_t consumed;
    TEST_ASSERT_TRUE(mqttsn_parse_header(buf, n, &h, &consumed));
    TEST_ASSERT_EQUAL_size_t(n, consumed);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_REGISTER, h.msg_type);

    uint16_t topic_id, msg_id;
    const char *name;
    size_t name_len;
    TEST_ASSERT_TRUE(mqttsn_parse_register(h.payload, h.payload_len, &topic_id, &msg_id, &name, &name_len));
    TEST_ASSERT_EQUAL_HEX16(0x0000, topic_id);
    TEST_ASSERT_EQUAL_HEX16(0x0042, msg_id);
    TEST_ASSERT_EQUAL_size_t(12, name_len);
    TEST_ASSERT_EQUAL_MEMORY("sensors/temp", name, name_len);
}

void test_parse_connack_regack_suback_publish()
{
    uint8_t rc;
    const uint8_t connack[] = {0x03, MQTTSN_CONNACK, MQTTSN_RC_ACCEPTED};
    MqttsnHeader h;
    size_t c;
    TEST_ASSERT_TRUE(mqttsn_parse_header(connack, sizeof(connack), &h, &c));
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_CONNACK, h.msg_type);
    TEST_ASSERT_TRUE(mqttsn_parse_connack(h.payload, h.payload_len, &rc));
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_RC_ACCEPTED, rc);

    // REGACK: TopicId=0x0009, MsgId=0x0042, rc=accepted.
    const uint8_t regack[] = {0x07, MQTTSN_REGACK, 0x00, 0x09, 0x00, 0x42, MQTTSN_RC_ACCEPTED};
    uint16_t tid, mid;
    TEST_ASSERT_TRUE(mqttsn_parse_header(regack, sizeof(regack), &h, &c));
    TEST_ASSERT_TRUE(mqttsn_parse_regack(h.payload, h.payload_len, &tid, &mid, &rc));
    TEST_ASSERT_EQUAL_HEX16(0x0009, tid);
    TEST_ASSERT_EQUAL_HEX16(0x0042, mid);

    // SUBACK: flags, TopicId=0x0009, MsgId=0x0001, rc=accepted.
    const uint8_t suback[] = {0x08, MQTTSN_SUBACK, (1 << MQTTSN_FLAG_QOS_SHIFT), 0x00, 0x09, 0x00, 0x01, 0x00};
    uint8_t fl;
    TEST_ASSERT_TRUE(mqttsn_parse_header(suback, sizeof(suback), &h, &c));
    TEST_ASSERT_TRUE(mqttsn_parse_suback(h.payload, h.payload_len, &fl, &tid, &mid, &rc));
    TEST_ASSERT_EQUAL_HEX16(0x0009, tid);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_RC_ACCEPTED, rc);

    // Inbound PUBLISH: flags, TopicId=0x0009, MsgId=0x0000, data "hi".
    const uint8_t pub[] = {0x09, MQTTSN_PUBLISH, 0x00, 0x00, 0x09, 0x00, 0x00, 'h', 'i'};
    const uint8_t *data;
    size_t dlen;
    TEST_ASSERT_TRUE(mqttsn_parse_header(pub, sizeof(pub), &h, &c));
    TEST_ASSERT_TRUE(mqttsn_parse_publish(h.payload, h.payload_len, &fl, &tid, &mid, &data, &dlen));
    TEST_ASSERT_EQUAL_HEX16(0x0009, tid);
    TEST_ASSERT_EQUAL_size_t(2, dlen);
    TEST_ASSERT_EQUAL_MEMORY("hi", data, 2);
}

// The 3-octet Length form (0x01 + big-endian uint16) is used past 255 octets and parses back.
void test_three_octet_length()
{
    static uint8_t buf[600];
    static uint8_t payload[400];
    memset(payload, 0xAB, sizeof(payload));
    size_t n = mqttsn_build_publish(buf, sizeof(buf), 0, 0x0001, 0x0001, payload, sizeof(payload));
    // total = 3(len) + 1(type) + 1(flags) + 2 + 2 + 400 = 409
    TEST_ASSERT_EQUAL_size_t(409, n);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[1]); // 409 >> 8
    TEST_ASSERT_EQUAL_HEX8(0x99, buf[2]); // 409 & 0xFF
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_PUBLISH, buf[3]);

    MqttsnHeader h;
    size_t consumed;
    TEST_ASSERT_TRUE(mqttsn_parse_header(buf, n, &h, &consumed));
    TEST_ASSERT_EQUAL_size_t(409, consumed);
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_PUBLISH, h.msg_type);
    TEST_ASSERT_EQUAL_size_t(405, h.payload_len); // 400 data + 5 header fields
}

void test_optional_fields()
{
    uint8_t buf[16];
    // PINGREQ with no client id is a 2-byte keep-alive.
    TEST_ASSERT_EQUAL_size_t(2, mqttsn_build_pingreq(buf, sizeof(buf), nullptr));
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_PINGREQ, buf[1]);
    // DISCONNECT without a duration is 2 bytes; with one it is 4.
    TEST_ASSERT_EQUAL_size_t(2, mqttsn_build_disconnect(buf, sizeof(buf), false, 0));
    TEST_ASSERT_EQUAL_size_t(4, mqttsn_build_disconnect(buf, sizeof(buf), true, 60));
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x3C, buf[3]); // 60
    // SEARCHGW carries a 1-byte radius.
    TEST_ASSERT_EQUAL_size_t(3, mqttsn_build_searchgw(buf, sizeof(buf), 2));
    TEST_ASSERT_EQUAL_HEX8(MQTTSN_SEARCHGW, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(2, buf[2]);
}

void test_overflow_and_malformed()
{
    uint8_t small[4];
    TEST_ASSERT_EQUAL_size_t(0, mqttsn_build_connect(small, sizeof(small), 0, 0, "toolongclientid"));

    MqttsnHeader h;
    size_t c;
    TEST_ASSERT_FALSE(mqttsn_parse_header(nullptr, 0, &h, &c));
    const uint8_t partial[] = {0x09, MQTTSN_PUBLISH, 0x00}; // declares 9 octets, only 3 buffered
    TEST_ASSERT_FALSE(mqttsn_parse_header(partial, sizeof(partial), &h, &c));
    const uint8_t bad_three[] = {0x01, 0x00}; // 3-octet indicator but buffer too short
    TEST_ASSERT_FALSE(mqttsn_parse_header(bad_three, sizeof(bad_three), &h, &c));
    const uint8_t zero_len[] = {0x00, 0x00}; // total length 0 is impossible
    TEST_ASSERT_FALSE(mqttsn_parse_header(zero_len, sizeof(zero_len), &h, &c));
    // A payload too short for its typed parser is rejected.
    const uint8_t short_regack[] = {0x04, MQTTSN_REGACK, 0x00, 0x09};
    TEST_ASSERT_TRUE(mqttsn_parse_header(short_regack, sizeof(short_regack), &h, &c));
    TEST_ASSERT_FALSE(mqttsn_parse_regack(h.payload, h.payload_len, nullptr, nullptr, nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_make_flags);
    RUN_TEST(test_build_connect_bytes);
    RUN_TEST(test_build_publish_bytes);
    RUN_TEST(test_register_round_trip);
    RUN_TEST(test_parse_connack_regack_suback_publish);
    RUN_TEST(test_three_octet_length);
    RUN_TEST(test_optional_fields);
    RUN_TEST(test_overflow_and_malformed);
    return UNITY_END();
}
