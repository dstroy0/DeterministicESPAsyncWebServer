// 18.ThreadGateway - an OpenThread RCP bridged to the gateway over spinel / HDLC-lite.
//
// A UART radio plugin (see 10.RadioGateway): an OpenThread radio co-processor (RCP - an
// nRF52840 / EFR32) speaks spinel over the HDLC-lite framing on a UART, so an 802.15.4 /
// Thread mesh is bridged to IP and the web. This decodes the HDLC-lite frames and bridges
// the spinel payload of each one northbound.
//
//   Thread RCP --UART--> spinel_frame_decode() --> spinel payload -> det_gw_uplink()
//                                                                         |
//                                                  envelope + topic  thread/0/<tid>
//                                                                         |
//                                                          northbound publish (MQTT/HTTP/WS)
//
// The HDLC-lite framing (byte-stuffing + CRC-16/X-25 FCS) is services/thread, host-tested in
// test/test_thread. Interpreting the spinel command inside a frame (the property get/set,
// an inbound IPv6 stream) is application work; this sketch bridges the raw spinel payload.
//
// Build flags (whole build): DETWS_ENABLE_THREAD=1 DETWS_ENABLE_GATEWAY=1

#include "dwserver.h" // discovers the library (adds src/ to the include path)
#include "services/gateway/gateway.h"
#include "services/thread/thread.h"
#include <Arduino.h>

static const uint8_t RADIO_PORT = 0;
static const int PIN_RX = 16, PIN_TX = 17; // UART2 to the Thread RCP

static uint8_t g_buf[1024];
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
    p.kind = det_gw_kind::DET_GW_THREAD;
    det_gw_add_port(&p);
    det_gw_set_uplink(northbound_publish, nullptr);
    det_gw_set_topic_prefix("thread");

    // Reset the NCP: spinel RESET command (header 0x80 | CMD_RESET 0x01).
    const uint8_t reset[2] = {0x80, 0x01};
    uint8_t frame[8];
    uint16_t n = spinel_frame_encode(reset, 2, frame, sizeof(frame));
    Serial2.write(frame, n);
    Serial.println("Thread gateway: spinel/HDLC -> codec -> publish (thread/0/<tid>)");
}

void loop()
{
    while (Serial2.available() && g_len < sizeof(g_buf))
        g_buf[g_len++] = (uint8_t)Serial2.read();

    for (;;)
    {
        if (g_len == 0)
            break;
        uint8_t payload[DETWS_THREAD_MAX_DATA];
        uint16_t plen = 0;
        int n = spinel_frame_decode(g_buf, g_len, payload, sizeof(payload), &plen);
        if (n == 0)
            break; // need more
        if (n < 0)
        {
            drop_front(1); // resync past a bad frame
            continue;
        }
        // Bridge the spinel payload; the header's low 4 bits are the transaction id.
        if (plen > 0)
        {
            uint16_t tid = payload[0] & 0x0F;
            det_gw_uplink(RADIO_PORT, tid, payload, plen, 0);
        }
        drop_front((uint16_t)n);
    }
}
