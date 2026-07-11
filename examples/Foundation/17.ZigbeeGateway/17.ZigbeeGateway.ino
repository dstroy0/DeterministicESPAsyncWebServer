// 17.ZigbeeGateway - a Zigbee (EZSP/ASH) NCP bridged to the gateway over UART.
//
// A UART radio plugin (see 10.RadioGateway): a Silicon Labs EmberZNet network co-processor
// speaks EZSP over the ASH data-link protocol on a UART. This resets the NCP and decodes
// the ASH frames it sends; a DATA frame carrying an EZSP callback (e.g. an incoming Zigbee
// message) is bridged northbound.
//
//   Zigbee NCP --UART--> ash_frame_decode() --> EZSP payload -> det_gw_uplink()
//                                                                     |
//                                              envelope + topic  zigbee/0/<node>
//                                                                     |
//                                                      northbound publish (MQTT/HTTP/WS)
//
// The ASH framing (byte-stuffing + CRC16) is services/zigbee, host-tested in
// test/test_zigbee. Interpreting the EZSP command inside a DATA frame (version negotiation,
// sequence numbers, the incomingMessageHandler fields) is application work; this sketch
// bridges the raw EZSP payload to show the transport path.
//
// Build flags (whole build): DETWS_ENABLE_ZIGBEE=1 DETWS_ENABLE_GATEWAY=1

#include "dwserver.h" // discovers the library (adds src/ to the include path)
#include "services/gateway/gateway.h"
#include "services/zigbee/zigbee.h"
#include <Arduino.h>

static const uint8_t RADIO_PORT = 0;
static const int PIN_RX = 16, PIN_TX = 17; // UART2 to the Zigbee NCP

static uint8_t g_buf[512];
static uint16_t g_len = 0;

static bool northbound_publish(const det_gw_msg *m, void *)
{
    char topic[48];
    det_gw_topic(m, topic, sizeof(topic));
    Serial.printf("PUBLISH %s  (%u bytes)\n", topic, m->len);
    return true;
}

static void drop_front(uint16_t n)
{
    memmove(g_buf, g_buf + n, g_len - n);
    g_len = (uint16_t)(g_len - n);
}

void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
    delay(300);

    det_gw_reset();
    det_gw_port_config p = {};
    p.port_id = RADIO_PORT;
    p.kind = det_gw_kind::DET_GW_ZIGBEE;
    det_gw_add_port(&p);
    det_gw_set_uplink(northbound_publish, nullptr);
    det_gw_set_topic_prefix("zigbee");

    uint8_t rst[8];
    uint16_t n = ash_frame_encode(Ash::ASH_RST, nullptr, 0, rst, sizeof(rst)); // reset the NCP
    Serial2.write(rst, n);
    Serial.println("Zigbee gateway: EZSP/ASH -> codec -> publish (zigbee/0/<node>)");
}

void loop()
{
    while (Serial2.available() && g_len < sizeof(g_buf))
        g_buf[g_len++] = (uint8_t)Serial2.read();

    for (;;)
    {
        if (g_len == 0)
            break;
        uint8_t control = 0, payload[DETWS_ZIGBEE_MAX_DATA];
        uint16_t plen = 0;
        int n = ash_frame_decode(g_buf, g_len, &control, payload, sizeof(payload), &plen);
        if (n == 0)
            break; // need more
        if (n < 0)
        {
            drop_front(1); // resync past a bad frame
            continue;
        }
        // A DATA frame (control bit 7 clear) carries an EZSP command; ACK/RSTACK/ERROR are
        // control frames. Bridge the EZSP payload of a DATA frame northbound.
        if ((control & 0x80) == 0 && plen > 0)
        {
            uint16_t node = plen >= 2 ? (uint16_t)((payload[0] << 8) | payload[1]) : payload[0];
            det_gw_uplink(RADIO_PORT, node, payload, plen, 0);
        }
        drop_front((uint16_t)n);
    }
}
