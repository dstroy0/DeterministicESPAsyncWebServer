// 10.RadioGateway - bridge a southbound radio to the northbound stack (the capstone).
//
// The whole v5 pipeline as a wireless gateway: a radio (here a simulated LoRa module on a
// DMA channel) receives frames; the DMA-complete callback posts each onto the FORWARD lane;
// the lane's task runs a tiny per-radio codec (first two bytes are the source node address,
// the rest is the payload) and hands it to the gateway, which envelopes it (address / port /
// RSSI / seq) and PUBLISHES it northbound - wire that to MQTT / HTTP / WebSocket. A command
// runs the other way: det_gw_downlink() transmits on the radio.
//
//   radio RX --DMA--> callback --post--> FORWARD lane --> codec --> det_gw_uplink()
//                                                                        |
//                                                     envelope + topic <prefix>/<port>/<addr>
//                                                                        |
//                                                            northbound publish (MQTT/HTTP/WS)
//
// The radio TX and the northbound publish are callbacks, so this runs with no radio; a real
// build swaps the simulator feed for the module's SPI RX and the publish for an MQTT client.
//
// Build flags (whole build):
//   DETWS_ENABLE_DMA=1 DETWS_ENABLE_PREEMPT_QUEUE=1 DETWS_ENABLE_GATEWAY=1 DETWS_DMA_SIMULATE=1

#include "DeterministicESPAsyncWebServer.h" // discovers the library (adds src/ to the include path)
#include "services/dma/det_dma.h"
#include "services/gateway/det_gateway.h"
#include "services/preempt_queue/preempt_queue.h"
#include <Arduino.h>
#include <string.h>

static const uint8_t RADIO_PORT = 0; // DMA channel 0 == the LoRa module

// Northbound publish: a real build calls mqtt.publish(topic, payload, len). We format the
// routing key and print it.
static bool northbound_publish(const det_gw_msg *m, void *)
{
    char topic[48];
    det_gw_topic(m, topic, sizeof(topic));
    Serial.printf("  PUBLISH %s  (%u bytes, rssi %d, seq %u)\n", topic, m->len, m->rssi, (unsigned)m->seq);
    return true;
}

// Southbound transmit (downlink): a real build calls the radio's send(dst, payload).
static bool radio_tx(uint8_t port, uint16_t dst, const uint8_t *, uint16_t len, void *)
{
    Serial.printf("  TX port%u -> node %u (%u bytes)\n", port, dst, len);
    return true;
}

// FORWARD-lane item: a self-contained copy of the radio frame (07.DmaIngest explains why).
struct radio_frame
{
    uint16_t len;
    uint8_t port;
    uint8_t bytes[24];
};
union pq_item {
    radio_frame f;
    uint8_t raw[DETWS_PQ_ITEM_SIZE];
};

// FORWARD lane task (high priority): the per-radio codec + the northbound bridge, off the
// "ISR". Frame layout: [addr_hi][addr_lo][payload...].
static void on_forward(const void *item, void *)
{
    const radio_frame *f = &((const pq_item *)item)->f;
    if (f->len < 2)
        return; // need the 2-byte node address header
    uint16_t addr = ((uint16_t)f->bytes[0] << 8) | f->bytes[1];
    det_gw_uplink(f->port, addr, f->bytes + 2, (uint16_t)(f->len - 2), /*rssi*/ -60);
}

// DMA-complete on the radio port: copy the frame and post it onto the FORWARD lane.
static void on_dma_complete(const det_dma_event *ev, void *)
{
    if (ev->dir != DET_DMA_RX)
        return;
    pq_item it = {};
    it.f.port = RADIO_PORT;
    it.f.len = ev->len;
    uint16_t n = (ev->len < sizeof(it.f.bytes)) ? ev->len : sizeof(it.f.bytes);
    memcpy(it.f.bytes, ev->data, n);
    detws_pq_post_lane_from_isr(DETWS_PQ_LANE_FORWARD, &it);
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    DetwsPqConfig fwd = {};
    fwd.handler = on_forward;
    fwd.priority = 0; // FORWARD lane default (above the user lane)
    fwd.core = 1;
    fwd.name = "gw_rx";
    detws_pq_start_lane(DETWS_PQ_LANE_FORWARD, &fwd);

    det_dma_config a = {};
    a.channel = RADIO_PORT;
    a.periph = DET_DMA_SPI; // LoRa modules are SPI
    a.on_complete = on_dma_complete;
    det_dma_open(&a);

    // The gateway: one LoRa port, publishing under "lora/<port>/<addr>".
    det_gw_reset();
    det_gw_port_config p = {};
    p.port_id = RADIO_PORT;
    p.kind = DET_GW_LORA;
    p.tx = radio_tx;
    det_gw_add_port(&p);
    det_gw_set_uplink(northbound_publish, nullptr);
    det_gw_set_topic_prefix("lora");

    Serial.println("gateway: LoRa RX -> DMA -> FORWARD lane -> codec -> publish (lora/port/addr)");
}

static uint8_t g_seq = 0;

void loop()
{
    // A radio frame arrives from one of four nodes (0x40..0x43): [addr_hi][addr_lo][0xAB][seq].
    uint16_t addr = 0x0040 + (g_seq & 0x03);
    uint8_t frame[4] = {(uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF), 0xAB, g_seq};
    det_dma_sim_feed(RADIO_PORT, frame, sizeof(frame));
    det_dma_poll(); // completes RX -> FORWARD lane -> codec -> uplink publish
    g_seq++;

    if ((g_seq & 0x07) == 0)
    {
        uint8_t cmd[2] = {0x01, g_seq};
        det_gw_downlink(RADIO_PORT, 0x0040, cmd, sizeof(cmd)); // command node 0x40
        det_gw_stats st;
        det_gw_get_stats(&st);
        Serial.printf("stats: up_in=%lu published=%lu down_sent=%lu\n", (unsigned long)st.up_in,
                      (unsigned long)st.up_published, (unsigned long)st.down_sent);
    }
    delay(1000);
}
